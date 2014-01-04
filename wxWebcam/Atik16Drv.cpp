/**

  Astronomy USB Appliance - Control Library

Copyright (C) 2009 Martin Burri  (bm@burri-web.org)

This program is free software licensed under LGPL;
you can redistribute it and/or modify it under the terms of
the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License,
or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/**
 * @file           Atik16Drv.cpp
 *****************************************************************************
 * class Atik16Drv
 *
 * Astronomy USB Appliance driver for Linux and Win32
 * The basic driver for Artemis/ATIK cameras with USB 1.1 interface
 * supported cameras are: see AtikVidPid.h
 *
 * This is part of the Driver Library of wxAstroCapture
 *
 * Copyright (C) 2009 Martin Burri  (bm98@burri-web.org)
 *
 *<hr>
 *
 * @b Project      wxAstroCapture<br>
 *
 * @author         M. Burri
 * @date           01-Feb-2009
 *
 *****************************************************************************
 *<hr>
 * @b Updates
 * - dd-mmm-yyyy V. Name: Description
 *
 *****************************************************************************/

#include "Atik16Drv.h"

/// There is a derived version used for the INDI framework
///  which has some files in another place than wxAstroCapture
#ifdef INDI
   #include "GenericDebHelp.h"
#else
   #include "../GenericDebHelp.h"
#endif

// defines a helper usually found in wxWidgets for INDI usage
#ifndef WXUNUSED
   #define WXUNUSED(identifier) /* identifier */
#endif
/// end INDI compatibility


#include "Atik16Thread.h"
#include "Atik16Cmd.h"

//! ////////////////////////////////////////
// defines the usage of either the Linux ftdi_sio or libusb driver
// for Artemis/ATIK cameras
// undef USE_LIBUSB  for the ftdi_sio driver
#define USE_LIBUSB

#ifdef USE_LIBUSB
   #define FTD245DRIVER Ftd245Lib
   #define FTD245TECH EAV_ArtLibUsb
   #include "Ftd245Lib.h"
#else
   #define FTD245DRIVER Ftd245IOUX
   #define FTD245TECH EAV_ArtSio
   #include "Ftd245IOUX.h"
#endif
//! end USE_LIBUSB ///////////////////////////

#include <assert.h>

#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

// local shortcuts
typedef unsigned char  _byte;
typedef unsigned short _word;
typedef unsigned long  _dword;


/// ///////////////////////////////////////////////////////////////////
/// Singleton handler
/// ///////////////////////////////////////////////////////////////////

Atik16Drv* Atik16Drv::pinstance = NULL;// initialize pointer
Atik16Drv* Atik16Drv::Instance ()
{
   if (pinstance == 0) {  // is it the first call?
      pinstance = new Atik16Drv(); // create sole instance
   }
   return pinstance; // address of sole instance
}


/// ///////////////////////////////////////////////////////////////////
/// Trivial Thread Wrapper
/// ///////////////////////////////////////////////////////////////////

#include <pthread.h>
struct AtikThread
{
private:
   ArtWorker&     m_artWorker; // generic worker object
   pthread_t      id;
   pthread_attr_t attr;

   // holds c asked callback routine
   union call_back {
      void * (*address)(void *);    // pthread expected callback
   } routine; // local var holds and translates the callback

public:
   // only public for the callback use !!
   void go();

public:
   AtikThread(ArtWorker& p_work);
   ~AtikThread();

   //!  Task entry
   int Run();
   //! pthread Join exposed
   int Join();
};

//! @brief  Implements a c style function to be sent as callback ptr to pthread_create
///   we have to submit this in pthread_create so it is sent as obj (context) here
///   so this is generic up to the point it casts to AtikThread somehow
extern "C" void* ARTthread_routine(void* obj) throw() {
   try {
      static_cast<AtikThread*>(obj)->go(); // cast and call member from context given
      return obj; //or whatever else you might need
   }
   catch(...) {
      // return error code or just exit
      return NULL;
   }
}//externC


//! @brief  Start function wrapped in thread_routine()
void AtikThread::go() { m_artWorker(); } // call operand () of our thread argument

// constructor
AtikThread::AtikThread(ArtWorker& p_work)
: m_artWorker(p_work)
{
   routine.address = ARTthread_routine; // assign callback address
   pthread_attr_init(&attr);
}

// destructor
AtikThread::~AtikThread() {
   pthread_attr_destroy(&attr);
}

//! @brief  Thread entry (pthread_create exposed)
int AtikThread::Run() {
   return pthread_create (&id, &attr, routine.address, this);
   // this one callsback above and therefore calls our go()
   // which in turn runs the real instance of AtikWorker submitted
   // while creating this AtikThread object
}

//! @brief  Thread Join (pthread_join exposed)
int AtikThread::Join() {
   return pthread_join(id, NULL);
}


/// ///////////////////////////////////////////////////////////////////
/// USB Enum Driver Interface
/// ///////////////////////////////////////////////////////////////////

//! @brief  Return true if Nth USB device exists and is a camera.
bool Atik16Drv::DeviceIsCamera(unsigned short Device)
{
   return Ftd245IO::DeviceIsCCDCamera(Device);
}

//! @brief  Get USB Identifier of Nth USB device. Return false if no such device.
//! @brief  buffer provided with pName must be at least 40 chars long.
bool Atik16Drv::DeviceName(unsigned short Device, char pName[])
{
   return Ftd245IO::DeviceName(Device, pName);
}

//! @brief  Get USB Serial number of Nth USB device. Return false if no such device.
//! @brief  buffer provided with pSerial must be at least 40 chars long.
bool Atik16Drv::DeviceSerial(unsigned short Device, char pSerial[])
{
   return Ftd245IO::DeviceSerial(Device, pSerial);
}


/// ///////////////////////////////////////////////////////////////////
/// Construction/Destruction
/// ///////////////////////////////////////////////////////////////////

Atik16Drv::Atik16Drv()
: m_ezCmd(NULL)
, m_thread(NULL)
, m_receiver(NULL)
, m_camState(CAMERA_ERROR)
, m_connected(false)
, m_ampSwitched(true)
, m_imageAquisition(false)
, m_timedWaiting(false)
, m_downloadPercent(0)
, m_downloadBps(0)
{
	m_artSample.imageMem = NULL;
	m_artSample.binX = 1; m_artSample.binY = 1;
	m_artSample.dataLength = 0;
	m_artSample.ready = false;
}

Atik16Drv::~Atik16Drv()
{
   Disconnect();
	if (m_artSample.imageMem) delete m_artSample.imageMem; m_artSample.imageMem=NULL;

   if (m_receiver) delete m_receiver;
	if (m_thread) delete m_thread;
}

void Atik16Drv::SetVerbose(bool verbose)
{
   DebHelp::SetVerboseLog();
}


//! @brief  Exposes the video technology used by the driver
Atik16Drv::EArtVideoTechnology Atik16Drv::VideoTechnology() const
{
   return FTD245TECH;  // defined in the header section (depends on libusb or ftd_sio usage)
}

//! @brief  Connect to the first available device
//! @return bool: true if successful
bool Atik16Drv::Connect()
{
   // InitBoard with all known cams, test some functions
   bool retVal=false;
   for (int i=0; i<10;i++) {
      retVal = Connect(i); // tries to connect the cam board
      if (retVal) break;  // exit if true
   }
   return retVal;
}

//! @brief  Connect if we know the deviceIndex
//! @param  int deviceIndex: the device to attempt to connect
//! @return bool: true if successful
bool Atik16Drv::Connect(unsigned short deviceIndex)
{
   _V_ ::fprintf(stderr, "Atik16Drv::Connect(%hd) \n", deviceIndex);

   if (m_connected) {
      if (m_ezCmd) if (deviceIndex==m_ezCmd->DeviceIndex()) return true; // already connected
      ::fprintf(stderr, "Atik16Drv::Connect() - already connected - disconnect first\n");
      return false; // cannot connect - disconnect first
   }

   // create the driver
   m_ezCmd = new FTD245DRIVER;   // defined in the header - usage depends on USE_LIBUSB  !!

   // InitBoard, test some functions
   bool retVal=false;
   retVal = m_ezCmd->InitBoard(deviceIndex); // tries to connect the cam board
   if (!retVal) {
      // did not work....
      if (m_ezCmd) delete m_ezCmd;  m_ezCmd = NULL; // drop the driver

      m_camState = CAMERA_ERROR;
      ::fprintf(stderr, "Atik16Drv::Connect() - InitBoard failed\n");
      return false; // ERROR EXIT - cannot connect
   }

   m_dev.ftd = m_ezCmd ;   // we use m_dev as handle to the IO system

   m_expoRecord.Init();
   m_caps.Init();

   // cleanup any data that is still there
   ClearCamIO();

   // read serial no
   _byte serBuffer[20]; // actual get only 6 + 1
   retVal = retVal && Atik16Cmd::CMD_GetSerialNo(m_dev, serBuffer);
   if (!retVal) ::fprintf(stderr, "Atik16Drv::Connect() - GetSerialNo failed\n");
   for (unsigned i=0;i<sizeof(m_camSerial);i++) {
      m_camSerial[i] = serBuffer[i];
   }
   m_hwRevIndex = serBuffer[sizeof(m_camSerial)]; // next byte is hw rev index: 0 = beta

   // first thing is to read caps (and init cam accord. to manual)
   retVal = retVal && RequestCAPS();
   if (!retVal) ::fprintf(stderr, "Atik16Drv::Connect() - RequestCAPS failed\n");
   if (retVal) {

      /*
      // send byte to expansion port (taken from ArtCapture Dump) ?!
      serBuffer[0] = 0xc9; serBuffer[1] = 0xf4; serBuffer[2] = 0x61; serBuffer[3] = 0xb1;
      retVal = retVal && Atik16Cmd::CMD_SendExtPort(m_dev, serBuffer, 4);
      if (!retVal) ::fprintf(stderr, "Atik16Drv::Connect() - SendExtPort failed - is ok for some cams\n");
      */

      m_connected = true;
      m_camState = CAMERA_WAITING;
      // create the receiver worker object - drop old ones first
      if (m_receiver) delete m_receiver;
      m_receiver = new Atik16Thread(this, m_dev);

      // create the thread helper - drop old ones first
      if (m_thread) delete m_thread;
      m_thread = new AtikThread(*m_receiver);
      m_thread->Run(); // run the thread
      ::fprintf(stderr, "Atik16Drv::Connect() - driver thread started\n");
   }
   else {
      //cannot get camera CAPS!!
      ::fprintf(stderr, "Atik16Drv::Connect() - Shutdown camera connection, device.closed!\n");
      m_ezCmd->ShutConnection();
      m_camState = CAMERA_ERROR;
   }

   return (m_connected && m_thread && m_receiver);
}


//! @brief  Disconnect from given device.
//! @brief  Returns true if disconnected as requested
bool Atik16Drv::Disconnect()
{
   _V_ ::fprintf(stderr, "Atik16Drv::Disconnect() \n");

   if (m_connected) {
      m_dev.ftd->Sleep_ms(500);  // allow some time to settle
      m_connected = false;       // not longer
      _V_ ::fprintf(stderr, "Atik16Drv::Disconnect() - Issue camera reset\n");
      Atik16Cmd::CMD_RESET(m_dev); // FULL reset
   }
   if (m_receiver) {
      _V_ ::fprintf(stderr, "Atik16Drv::Disconnect() - killing driver thread now\n");
      m_receiver->AbortThread();
      // first wait for the thread proc to terminate
      assert(m_thread);
      if (m_thread) {
         _V_ ::fprintf(stderr, "Atik16Drv::Disconnect() - wait for driver thread end\n");
         m_thread->Join();  // waits until it is gone
         delete m_thread; m_thread = NULL;
         _V_ ::fprintf(stderr, "Atik16Drv::Disconnect() - driver thread has ended, reclaiming resources\n");
      }

      // kill worker obj
      delete m_receiver; m_receiver = NULL;
   }
   return true;
}

//! @brief  Fills in pProp with camera properties
ARTEMISERROR Atik16Drv::Properties(ARTEMISPROPERTIES& pProp)
{
   if (!m_connected) return ARTEMIS_NOT_CONNECTED;

   if (m_caps.ProtocolRevMaj==0) {
      // must gather those properties again
      if (Atik16Cmd::CMD_RequestCAPS(m_dev, m_caps))
         m_connected = true;
   }
   // convert from DrvCmd Format to ARTEMISPROPERTIES
   pProp.Protocol = m_caps.ProtocolRevMaj*100 + m_caps.ProtocolRevMin;
   pProp.nPixelsX = m_caps.TotalPixelsX;
   pProp.nPixelsY = m_caps.TotalPixelsY;
   pProp.PixelMicronsX = m_caps.PixelSizeX;
   pProp.PixelMicronsY = m_caps.PixelSizeY;
   pProp.cameraflags = m_caps.FIFOfitted ? ARTEMIS_PROPERTIES_CAMERAFLAGS_FIFO : ARTEMIS_PROPERTIES_CAMERAFLAGS_NULL;
   pProp.ccdflags = m_caps.Interlaced ? ARTEMIS_PROPERTIES_CCDFLAGS_INTERLACED : ARTEMIS_PROPERTIES_CCDFLAGS_NULL;
   strncpy(pProp.Description, m_caps.ID_String.c_str(), 39); pProp.Description[39]=0;
   strncpy(pProp.Manufacturer, m_caps.UserID_String.c_str(), 39);pProp.Manufacturer[39]=0;

   if (m_connected)
      return ARTEMIS_OK;
   else
      return ARTEMIS_NO_RESPONSE;
}

//! @brief  Set download thread to high or normal priority
ARTEMISERROR Atik16Drv::HighPriority(bool bHigh)
{
   if (!m_connected) return ARTEMIS_NOT_CONNECTED;
   return ARTEMIS_NOT_IMPLEMENTED;
}


/// All coords are unbinned camera pixels

//! @brief  Get the pos and size of imaging subframe
void  Atik16Drv::Subframe(unsigned short& x, unsigned short& y, unsigned short& w, unsigned short& h) const
{
   x = m_expoRecord.XStart; y = m_expoRecord.YStart;
   w = m_expoRecord.NumX;   h = m_expoRecord.NumY;
}

//! @brief  set the pos and size of imaging subframe inunbinned coords
ARTEMISERROR Atik16Drv::SetSubframe(unsigned short x, unsigned short y, unsigned short w, unsigned short h)
{
   if (!m_connected) return ARTEMIS_NOT_CONNECTED;

   if ( (x+w) > m_caps.TotalPixelsX ) return ARTEMIS_INVALID_PARAMETER;
   if ( (y+h) > m_caps.TotalPixelsY ) return ARTEMIS_INVALID_PARAMETER;

   m_expoRecord.XStart = x; m_expoRecord.YStart = y;
   m_expoRecord.SetNum(w, h);
   return ARTEMIS_OK;
}

//! @brief  Set the start x,y coords for imaging subframe., X,Y in unbinned coordinates
ARTEMISERROR Atik16Drv::SetSubframePos(unsigned short x, unsigned short y)
{
   if (!m_connected) return ARTEMIS_NOT_CONNECTED;
   if ( (x+m_expoRecord.NumX) > m_caps.TotalPixelsX ) return ARTEMIS_INVALID_PARAMETER;
   if ( (y+m_expoRecord.NumY) > m_caps.TotalPixelsY ) return ARTEMIS_INVALID_PARAMETER;

   m_expoRecord.XStart = x; m_expoRecord.YStart = y;
   return ARTEMIS_OK;
}

//! @brief  Set the width and height of imaging subframe, W,H in unbinned coordinates
ARTEMISERROR Atik16Drv::SetSubframeSize(unsigned short w, unsigned short h)
{
   if (!m_connected) return ARTEMIS_NOT_CONNECTED;
   if ( (m_expoRecord.XStart+w) > m_caps.TotalPixelsX ) return ARTEMIS_INVALID_PARAMETER;
   if ( (m_expoRecord.YStart+h) > m_caps.TotalPixelsY ) return ARTEMIS_INVALID_PARAMETER;

   m_expoRecord.SetNum(w, h);
   return ARTEMIS_OK;
}

//! @brief  Get the x,y binning factors
void  Atik16Drv::Bin(unsigned short&x, unsigned short& y) const
{
   x = m_expoRecord.XBin; y = m_expoRecord.YBin;
}

//! @brief  Set the x,y binning factors
ARTEMISERROR Atik16Drv::SetBin(unsigned short x, unsigned short y)
{
   if (!m_connected) return ARTEMIS_NOT_CONNECTED;
   if ( x > m_caps.MaxBinX ) return ARTEMIS_INVALID_PARAMETER;
   if ( y > m_caps.MaxBinY ) return ARTEMIS_INVALID_PARAMETER;
   if (m_caps.SyncBinning) {
      m_expoRecord.XBin = x; m_expoRecord.YBin = x; // BinY=X
   }
   else {
      m_expoRecord.XBin = x; m_expoRecord.YBin = y;
   }
   return ARTEMIS_OK;
}

//! @brief  Get the max x,y binning factors
void Atik16Drv::MaxBin(unsigned short& x, unsigned short& y)
{
   x = m_caps.MaxBinX;
   if (m_caps.SyncBinning) {
      y = m_caps.MaxBinX;
   }
   else {
      y = m_caps.MaxBinY;
   }
}

//! @brief  Set whether amp is switched off during exposures
ARTEMISERROR Atik16Drv::SetAmplifierSwitched(bool bSwitched)
{
   if (!m_connected) return ARTEMIS_NOT_CONNECTED;

   m_ampSwitched = bSwitched;
   // this is for short expo only
   if (m_ampSwitched)
      m_expoRecord.ImageConfig |= EICB_AmpOn;
   else
      m_expoRecord.ImageConfig &= ~EICB_AmpOn;
   return ARTEMIS_OK;
}

//! @brief  Set the CCD amplifier on or off
ARTEMISERROR Atik16Drv::SetAmp(bool bOn)
{
   if (!m_connected) return ARTEMIS_NOT_CONNECTED;

   return (Atik16Cmd::CMD_Amplifier(m_dev, bOn))?ARTEMIS_OK:ARTEMIS_NO_RESPONSE;
}

//! @brief  Return duration of last exposure, in seconds
float Atik16Drv::LastExposureDuration() const
{
   return (float)m_artSample.expotime / 1000.0;
}

//! @brief  Start an exposure
ARTEMISERROR Atik16Drv::StartExposure(float Seconds)
{
   _V_ ::fprintf(stderr, "Atik16Drv::StartExposure() \n");

   if (!m_connected) return ARTEMIS_NOT_CONNECTED;

   if (m_camState!=CAMERA_IDLE) return ARTEMIS_NO_RESPONSE;
   // get this data as late as possible as it may have changed from setup
   // due to pixel adjustment for interlace and binning (cam does not like odd sizes)
   _V_ ::fprintf(stderr, "Atik16Drv::StartExposure() - ready\n");
   m_expoRecord.Exposure = (_dword)(Seconds *1000.0); // used in ms steps
   m_artSample.ready    = false;
   m_artSample.expotime = m_expoRecord.Exposure;

   if (m_artSample.expotime<Atik16Cmd::SHORTEXPOLIMIT) {
      _V_ ::fprintf(stderr, "Atik16Drv::StartExposure() - short exposure \n");
      m_receiver->TimedWait(0); // dummy wait
   }
   else {
      _V_ ::fprintf(stderr, "Atik16Drv::StartExposure() - regular exposure \n");
//      if (m_ampSwitched) Atik16Cmd::CMD_Amplifier(m_dev, 0);
      // do it like the others do ?!
      Atik16Cmd::CMD_GuideOff(m_dev);
      Atik16Cmd::CMD_Amplifier(m_dev, false);
      Atik16Cmd::CMD_ClearCCD(m_dev);
      Atik16Cmd::CMD_Shutter(m_dev);
      m_receiver->TimedWait(m_artSample.expotime-300); // start the expo cycle; sub fudge time 300ms for now
     // return ( TIMER(m_artSample.expotime-300) )?ARTEMIS_OK:ARTEMIS_NO_RESPONSE; // sub fudge time 300ms for now
   }
   return ARTEMIS_OK;
}

//! @brief  Return time remaining in current exposure, in seconds
float Atik16Drv::ExposureTimeRemaining() const
{
   if (!m_connected) return 0.0;
   float tr = m_receiver->WaitTimeRemaining();
   return tr / 1000.0;
}

//! @brief  Percentage downloaded
int Atik16Drv::DownloadPercent() const
{
   return m_downloadPercent;
}

//! @brief  Prematurely end an exposure, collecting image data.
ARTEMISERROR Atik16Drv::StopExposure()
{
   _V_ ::fprintf(stderr, "Atik16Drv::StopExposure() \n");

   if (!m_connected) return ARTEMIS_NOT_CONNECTED;

   m_receiver->StopExposure();
   return ARTEMIS_OK;
}

//! @brief  Abort exposure, if one is in progress
ARTEMISERROR Atik16Drv::AbortExposure()
{
   _V_ ::fprintf(stderr, "Atik16Drv::AbortExposure() \n");

   if (!m_connected) return ARTEMIS_NOT_CONNECTED;

   m_receiver->AbortExposure();
   return ARTEMIS_OK;
}


//! @brief  Return true if an image is ready to be retrieved
bool Atik16Drv::ImageReady() const
{
   if (!m_connected) return false;

   return m_artSample.ready;
}

//! @brief  Retrieve image dimensions and binning factors.
//! @brief  x,y are actual CCD locations. w,h are pixel dimensions of image
void  Atik16Drv::ImageData(unsigned short& x, unsigned short& y,
                               unsigned short& w, unsigned short& h,
                               unsigned short& binx, unsigned short& biny)
{
   x=0;y=0;w=0;h=0;
   if (m_artSample.imageMem) m_artSample.imageMem->GetFrameRect(x,y,w,h);
   binx = m_artSample.binX; biny = m_artSample.binY;
}

//! @brief  Return pointer to internal image buffer (actually unsigned shorts)
const unsigned short* Atik16Drv::ImageBuffer() const
{
   if (!m_connected) return NULL;

   if (m_artSample.ready && m_artSample.imageMem)
      return m_artSample.imageMem->WordMemPtr();
   else
      return NULL;
}


//! @brief  cooling support

void Atik16Drv::TemperatureSensorInfo(unsigned short WXUNUSED(sensor), int& temperature) // °C *100
{
   temperature=-990;
}

void Atik16Drv::CoolingInfo(ECamCoolingInfo& flags, int& WXUNUSED(level), int& WXUNUSED(minlvl), int& WXUNUSED(maxlvl), int& WXUNUSED(setpoint))
{
   flags=(ECamCoolingInfo)0; // returns no cooling
}

ARTEMISERROR Atik16Drv::SetCooling(int WXUNUSED(setpoint))
{
   return ARTEMIS_NOT_IMPLEMENTED;
}

ARTEMISERROR Atik16Drv::SetCooling(double WXUNUSED(temperature))
{
   return ARTEMIS_NOT_IMPLEMENTED;
}

ARTEMISERROR Atik16Drv::CoolerWarmUp()
{
   return ARTEMIS_NOT_IMPLEMENTED;
}


/// Tools

ARTEMISERROR Atik16Drv::ReconnectUSB()
{
   // connecting a cam resets the USB interface
   // so we will not do anything here
   if (!m_connected) return ARTEMIS_NOT_CONNECTED;
   return ARTEMIS_OK;
}

//! @brief  Return the last FT USB error condition seen
//! @brief  Calling this function clears the internal error state (FT_OK=0)
int Atik16Drv::DiagnosticUSBError()
{
   if (m_dev.DevNULL()) return Ftd245IO::FTS_INVALID_HANDLE;
   if (!m_connected) return Ftd245IO::FTS_INVALID_HANDLE; //FT_INVALID_HANDLE
   if (m_receiver) {
      if (m_receiver->IsReadError()) return Ftd245IO::FTS_IO_ERROR; //FT_IO_ERROR
   }
   if (!m_dev.ftd->BoardAvailable()) return Ftd245IO::FTS_IO_ERROR; //FT_IO_ERROR

   return Ftd245IO::FTS_OK; // FT_OK
}

//! @brief  Send a diagnostic message to the camera
//! @param  int send:  the message
//! @return int: returns the sent message if OK
int Atik16Drv::DiagnosticPing(int send)
{
   if ( m_dev.DevNULL() ) return -1;
   if (!m_connected) return -1;
   if (!m_dev.ftd->BoardAvailable()) return -1;

   _byte c = send & 0xff;
   bool retVal = Atik16Cmd::CMD_AppPing(m_dev, c);
   return (retVal && (c==(send & 0xff))?send:-1);
}



/// ///////////////////////////////////////////////////////////////////
/// Cam Commands
/// ///////////////////////////////////////////////////////////////////

//! @brief  Query the camera capabilities into the drivers own store
bool Atik16Drv::RequestCAPS()
{
   _V_ ::fprintf(stderr, "Atik16Drv::RequestCAPS() \n");

   bool retVal = Atik16Cmd::CMD_RequestCAPS(m_dev, m_caps);
   if (!retVal) {
      ::fprintf(stderr, "Atik16Drv::RequestCAPS() - Error in CMD_RequestCAPS()\n");
      m_camState = CAMERA_ERROR;
      m_caps.Init();
      m_expoRecord.CamCaps = m_caps;   // contents are used !!
      return false;
   }
   _V_ ::fprintf(stderr, "Atik16Drv::RequestCAPS() - CMD_RequestCAPS() OK \n");
   // initialize the expo record for full image capture
   m_expoRecord.CamCaps = m_caps;   // contents are used !!
   m_expoRecord.XStart = 0; m_expoRecord.YStart = 0;
   m_expoRecord.SetNum(m_caps.TotalPixelsX, m_caps.TotalPixelsY);
   m_expoRecord.XBin = 1; m_expoRecord.YBin = 1;
   // cam dependent flags
   m_expoRecord.ImageConfig |=  (m_caps.Interlaced)?EICB_DeInterlace:0;
   if (m_hwRevIndex>0) {
      m_expoRecord.ImageConfig |=  EICB_UseFIFO;
      _V_ ::fprintf(stderr, "Atik16Drv::RequestCAPS() - hwIndex forces use of FIFO \n");
   }
   else {
      // FIFO is in question only for beta cams ??
      if (! m_caps.FIFOfitted) {
         // if we don't get it from the caps - we try the query
         bool fifo;
         retVal = Atik16Cmd::CMD_FIFO_Fitted(m_dev, fifo);
         if (!retVal) {
            ::fprintf(stderr, "Atik16Drv::RequestCAPS() - Error in CMD_FIFO_Fitted()\n");
            fifo = false; // still no FIFO
         }
         m_expoRecord.ImageConfig |=  (fifo)?EICB_UseFIFO:0;
         _V_ ::fprintf(stderr, "Atik16Drv::RequestCAPS() - FIFO query defines use of FIFO \n");
      }
      else {
         m_expoRecord.ImageConfig |= EICB_UseFIFO;
         _V_ ::fprintf(stderr, "Atik16Drv::RequestCAPS() - caps defines use of FIFO \n");
      }
   }
   _V_ ::fprintf(stderr, "Atik16Drv::RequestCAPS() - ImageConfig: 0x%04x \n", m_expoRecord.ImageConfig);

	if (m_artSample.imageMem)
	{
		delete m_artSample.imageMem;
		m_artSample.imageMem = NULL;
		m_artSample.dataLength = 0;
	}

   //! the one and only place to create the image mem for the Art Driver
   //! creates memory to hold a full image (and a helper for deinterlaceing if needed)
	// make a new image buffer if we know the ccd i.e. call this before reading the ccd
	// this is not going to change while connected to a cam
   m_artSample.dataLength = m_caps.TotalPixelsX * m_caps.TotalPixelsY; // WORDS full cam size
   if (m_caps.Interlaced) {
      m_artSampleHelper = new WordImageMem(m_artSample.dataLength);    // read sample is in the helper space
      m_artSample.imageMem = new WordImageMem(m_artSample.dataLength); // will get deinterlaced to here
   }
   else {
      m_artSampleHelper = new WordImageMem(m_artSample.dataLength);
      m_artSample.imageMem = m_artSampleHelper;// read sample is already the proper one
   }
   _V_ ::fprintf(stderr, "Atik16Drv::RequestCAPS() - mem alloc for image sample OK length %d \n", (int)m_artSample.dataLength);
   m_artSample.dataLength *= 2; // make BYTES (this is what is needed to be read

   m_camState = CAMERA_IDLE;

	return true;
}


//! @brief  Prepare reading out the CCD
bool Atik16Drv::PrepareReadCCD()
{
   _V_ ::fprintf(stderr, "Atik16Drv::PrepareReadCCD() \n");

   Atik16Cmd::PrepReadCCD(m_expoRecord, m_rccdBuf);

   m_artSample.expotime = m_expoRecord.Exposure;
   // get this data as late as possible as it may have changed from setup
   // due to pixel adjustment for interlace and binning (cam does not like odd sizes)
   // m_artSample carries positive binned CCD coords i.e. mirrored but positive oriented realworld coords
   // m_artSample carries positive binned CCD coords i.e. mirrored but positive oriented realworld coords
   m_artSample.imageMem->SetFrameRect(m_expoRecord.XStart, m_expoRecord.YStart,
                              m_expoRecord.NumX/m_expoRecord.XBin, m_expoRecord.NumY/m_expoRecord.YBin);
   m_artSample.binX       = m_expoRecord.XBin;
   m_artSample.binY       = m_expoRecord.YBin;
   m_artSample.dataLength = m_expoRecord.ImageBytes();
	if (m_caps.Interlaced) {
      // interlaced CCD
      m_artSample.toDeinterlace = true; // must be deinterlaced before usage
	}
	// ready now

   _V_ ::fprintf(stderr, "Atik16Drv::PrepareReadCCD() - width / height = %u / %u \n",
                  m_artSample.imageMem->width(), m_artSample.imageMem->height());
	// let the thread start capturing - command execution is in thread routine
   m_receiver->Aquire(m_artSample);
   m_imageAquisition = true;

	// once the thread is ready it will trigger the READCCD with the
	// parameters set up above  i.e. sending rccdBuf
	return true;
}


//! @brief  read all Input from the camera that might still be there
void Atik16Drv::ClearCamIO()
{
   _V_ ::fprintf(stderr, "Atik16Drv::ClearCamIO - IN ... ");

   unsigned long rt,wt;
	m_dev.ftd->GetTimeouts(rt, wt);

	m_dev.ftd->SetTimeouts(500, wt); // wait only little here
	m_dev.ftd->SetMaxBuffer();
   size_t readErr = 5; // try some to be sure to empty the cam
   size_t read = 0;
   do {
      m_dev.ftd->ReadBYTEsNIL(read);
      if (read==0)
         readErr--;
      else
         readErr = 5; // reset end detector

   } while (readErr>0);
	m_dev.ftd->SetDefaultBuffer();
	m_dev.ftd->SetTimeouts(rt, wt); // reset to what it was before
   _V_ ::fprintf(stderr, " - OUT \n");

   Atik16Cmd::CMD_RESET(m_dev);
}


/// ///////////////////////////////////////////////////////////////////
/// Thread Callback implementation
/// ///////////////////////////////////////////////////////////////////

//! @brief  Callback from the ReceiverThread when timer expires with success
void Atik16Drv::ProcessTimeElapsed()
{
   _V_ ::fprintf(stderr, "Atik16Drv::ProcessTimeElapsed()\n");
// REMARK: beware of the context ... this is a callback of the HighPrio Thread
	m_timedWaiting = false;
   PrepareReadCCD();
}

//! @brief  Callback from the ReceiverThread to start the camera readout
//! @brief    the command must be setup with PrepareReadCCD before
void Atik16Drv::ProcessTriggerRDCCD()
{
   _V_ ::fprintf(stderr, "Atik16Drv::ProcessTriggerRDCCD()\n");
// REMARK: beware of the context ... this is a callback of the HighPrio Thread
   Atik16Cmd::CMD_GuideOff(m_dev);
   Atik16Cmd::CMD_Amplifier(m_dev, true);
   Atik16Cmd::CMD_ReadCCD(m_dev, m_rccdBuf); // either complete timing (short expo) or only readout
}


//! @brief  Callback from the ReceiverThread while downloading data
//! @param  long downloadPercent: download completion % value 0.. 100
void Atik16Drv::ProcessTriggerDownload(long downloadPercent)
{
// REMARK: beware of the context ... this is a callback of the HighPrio Thread

	m_downloadPercent = downloadPercent;
}

//! @brief  Callback from the ReceiverThread when DL is finished
//! @param  bool readError: true if a read error occured
//! @param  long realExpotime: the measured exposure time
//! @param  long downloadBps: the measured download rate in bytes/sec
// note: this is called in any case
//       readerror could be set or camState FLUSHING or DOWNLOADING (which is OK)
void Atik16Drv::ProcessImageRead(bool readError, long realExpotime, long downloadBps)
{
   _V_ ::fprintf(stderr, "Atik16Drv::ProcessImageRead() \n");
// REMARK: beware of the context ... this is a callback of the HighPrio Thread

   m_downloadPercent = 100;
   m_downloadBps = downloadBps;

   ::fprintf(stderr, "DL Rate is: %d Bytes/sec \n", (int)m_downloadBps);
   if (m_artSample.expotime>=Atik16Cmd::SHORTEXPOLIMIT) m_artSample.expotime = realExpotime;  // gather the real exposuretime

   ::fprintf(stderr, "Expotime is: %d millisec \n", (int)m_artSample.expotime);
   m_imageAquisition = false;
   m_artSample.ready = ( (m_camState==CAMERA_DOWNLOADING) && (!readError) );

   if (m_artSample.ready && m_artSample.toDeinterlace) {
      //! we have to deinterlace the image

      // the sample is collected in artSample but we need to deinterlace it
      // so the artSample.imageMem is switched with the helper
      m_artSampleHelper->SetFrameRect(m_artSample.imageMem); // copy frame info from master
      WordImageMem* tmp = m_artSample.imageMem;
      m_artSample.imageMem = m_artSampleHelper;
      m_artSampleHelper = tmp;

      // Deinterlace by combining the two single frames that arrived one after the other
      // must use binned data here
      _word ccdw = m_artSample.imageMem->width();
      _word ccdh = m_artSample.imageMem->height();

      _word* s1Ptr = m_artSampleHelper->WordMemPtrRef() + ((ccdh/2)*ccdw);    // source lower (1st) half
      _word* s2Ptr = m_artSampleHelper->WordMemPtrRef();                      // source upper half

      // the cam does gather exposures <2.46 sec in two rounds
      // to get rid of the differences we calculate the ratio of the two interlaced images
      // and equalize frames using the first 7 lines as scale (or 1 if no 7 are avail)
      size_t nPix = (ccdh>2)?ccdw*7:ccdw;
      double mean1=0.0; for (size_t x=0;x<nPix; x++, s1Ptr++) {mean1 += *s1Ptr;}; mean1 /= nPix;
      double mean2=0.0; for (size_t x=0;x<nPix; x++, s2Ptr++) {mean2 += *s2Ptr;}; mean2 /= nPix;
      double eq = mean2/mean1;

      s1Ptr = m_artSampleHelper->WordMemPtrRef() + ((ccdh/2)*ccdw);    // source lower (1st) half
      s2Ptr = m_artSampleHelper->WordMemPtrRef();                      // source upper half
      _word* dPtr = m_artSample.imageMem->WordMemPtrRef();         // dest start
      for (_word y=0; y<(ccdh/2); y++) {
         for (_word x=0; x<ccdw; x++) {*dPtr++=(_word)(eq *  (*s1Ptr++));};
         for (_word x=0; x<ccdw; x++) {*dPtr++=*s2Ptr++;};
         /*
         memcpy(dPtr, s1Ptr, ccdw*2); // copy a line in BYTE size
         dPtr += ccdw; s1Ptr += ccdw;   // src is next line
         memcpy(dPtr, s2Ptr, ccdw*2); // copy a line in BYTE size
         dPtr += ccdw; s2Ptr += ccdw;   // src is next line
         */
      }
      m_artSample.toDeinterlace = false; // not longer
   }
   m_artSample.ready =true; // force to true in the end (state behavior)
}


//! @brief  Callback from the ReceiverThread when the worker is about to start its main loop
// setup the infrastructure
void Atik16Drv::ProcessThreadStart()
{
   _V_ ::fprintf(stderr, "Atik16Drv::ProcessThreadStart - IN\n");
// REMARK: beware of the context ... this is a callback of the HighPrio Thread

   assert( m_dev.DevOK() );

	m_camState = CAMERA_IDLE;
}


//! @brief  Callback from the ReceiverThread when the worker is about to exit
// clean up the infrastructure
void Atik16Drv::ProcessThreadEnd(int exitCode)
{
   _V_ ::fprintf(stderr,"Atik16Drv::ProcessThreadEnd - exitCode %d\n", exitCode);
// REMARK: beware of the context ... this is a callback of the HighPrio Thread

   m_connected = false;
	m_timedWaiting = false;
	m_imageAquisition = false;

	m_camState = CAMERA_WAITING;

   assert( m_dev.DevOK() );
   ClearCamIO();

   if ( m_dev.DevOK() ) {
      if (m_ezCmd) delete m_ezCmd; m_ezCmd = NULL;
   }
   m_dev.Init();  // reset this too
}


/// ///////////////////////////////////////////////////////////////////
/// Trivial FITS Writer
/// ///////////////////////////////////////////////////////////////////

/// byte order
#define  xBIG_ENDIAN     4321
#define  xLITTLE_ENDIAN  1234

#ifdef WORDS_BIGENDIAN
#define  xBYTE_ORDER  xBIG_ENDIAN
#else
#define  xBYTE_ORDER  xLITTLE_ENDIAN
#endif

#define xUINT16_SWAP_ALWAYS(val) \
   ((_word) ( \
    (((_word) (val) & (_word) 0x00ffU) << 8) | \
    (((_word) (val) & (_word) 0xff00U) >> 8)))

#define xINT16_SWAP_ALWAYS(val) \
   ((_word) ( \
    (((_word) (val) & (_word) 0x00ffU) << 8) | \
    (((_word) (val) & (_word) 0xff00U) >> 8)))

#define xUINT32_SWAP_ALWAYS(val) \
   ((_dword) ( \
    (((_dword) (val) & (_dword) 0x000000ffU) << 24) | \
    (((_dword) (val) & (_dword) 0x0000ff00U) <<  8) | \
    (((_dword) (val) & (_dword) 0x00ff0000U) >>  8) | \
    (((_dword) (val) & (_dword) 0xff000000U) >> 24)))

#define xINT32_SWAP_ALWAYS(val) \
   ((_dword) ( \
    (((_dword) (val) & (_dword) 0x000000ffU) << 24) | \
    (((_dword) (val) & (_dword) 0x0000ff00U) <<  8) | \
    (((_dword) (val) & (_dword) 0x00ff0000U) >>  8) | \
    (((_dword) (val) & (_dword) 0xff000000U) >> 24)))

#ifdef WORDS_BIGENDIAN
  #define xUINT16_SWAP_ON_BE(val)  xUINT16_SWAP_ALWAYS(val)
  #define xINT16_SWAP_ON_BE(val)   xINT16_SWAP_ALWAYS(val)
  #define xUINT16_SWAP_ON_LE(val)  (val)
  #define xINT16_SWAP_ON_LE(val)   (val)
  #define xUINT32_SWAP_ON_BE(val)  xUINT32_SWAP_ALWAYS(val)
  #define xINT32_SWAP_ON_BE(val)   xINT32_SWAP_ALWAYS(val)
  #define xUINT32_SWAP_ON_LE(val)  (val)
  #define xINT32_SWAP_ON_LE(val)   (val)
#else
  #define xUINT16_SWAP_ON_LE(val)  xUINT16_SWAP_ALWAYS(val)
  #define xINT16_SWAP_ON_LE(val)   xINT16_SWAP_ALWAYS(val)
  #define xUINT16_SWAP_ON_BE(val)  (val)
  #define xINT16_SWAP_ON_BE(val)   (val)
  #define xUINT32_SWAP_ON_LE(val)  xUINT32_SWAP_ALWAYS(val)
  #define xINT32_SWAP_ON_LE(val)   xINT32_SWAP_ALWAYS(val)
  #define xUINT32_SWAP_ON_BE(val)  (val)
  #define xINT32_SWAP_ON_BE(val)   (val)
#endif
/// END byte order

// Trivial FITS write functions
static char fx1[81] = "SIMPLE  =                    T                                                  ";
static char fx2[81] = "BITPIX  =                   16 / # bits storing pix values                      ";
static char fx3[81] = "NAXIS   =                    2 / 2d plane image                                 ";
static char fx4[81] = "NAXIS1  =                 %4hu / # of pixels/row                                ";
static char fx5[81] = "NAXIS2  =                 %4hu / # of rows (also # of scan lines)               ";
static char fx6[81] = "BZERO   =              32768.0 / real = fits-value*BSCALE+BZERO                 ";
static char fx7[81] = "BSCALE  =                  1.0 / real = fits-value*BSCALE+BZERO                 ";
static char fx8[81] = "DATE-OBS= %20s / UTC Date the file was written                                  ";
static char fx9[81] = "TIMESYS =                  UTC / Time system used                               ";
static char fxa[81] = "INSTRUME= %20s                                                                  ";
static char fxb[81] = "PSZX    = %#20.6G / detector X cell size [microns]                              ";
static char fxc[81] = "PSZY    = %#20.6G / detector Y cell size [microns]                              ";
static char fxd[81] = "SWCREATE=      Atik16 INDI Driver                                               ";
static char fxe[81] = "IMAGETYP= %20s                                                                  ";
static char fxf[81] = "XBINNING=                 %4hu / X binning factor                               ";
static char fxg[81] = "YBINNING=                 %4hu / Y binning factor                               ";
static char fxh[81] = "EXPTIME = %#20.6G / Total integration time (s)                                  ";
//static char fxi[81] = "CCD_TEMP= %#20.6G / temperature deg C                                           ";
static char fxx[81] = "END                                                                             ";
static char fxX[81] = "                                                                                ";


//! @brief  Write the FITS Header data to a file
//! @param  FILE* of: the output file
//! @param  char frameTypeS[]: a string used as filetype keyword
void Atik16Drv::WriteFitsHeader(FILE* of, char frameTypeS[])
{
   _V_ ::fprintf(stderr, "Atik16Drv::WriteFitsHeader() \n");
   char buf[81];
   char tmpBuf[40];
   size_t hSize = 36;    // 2880/80 = 36 header lines to write in total
   // write some lines with some meaningful content
   ::fwrite(fx1, 80, 1, of); hSize--;
   ::fwrite(fx2, 80, 1, of); hSize--;
   // naxis
   ::fwrite(fx3, 80, 1, of); hSize--;
   ::sprintf(buf, fx4, m_artSample.imageMem->width());
   ::fwrite(buf, 80, 1, of); hSize--;
   ::sprintf(buf, fx5, m_artSample.imageMem->height());
   ::fwrite(buf, 80, 1, of); hSize--;
   // bscale/bzero
   ::fwrite(fx6, 80, 1, of); hSize--;
   ::fwrite(fx7, 80, 1, of); hSize--;
   // time date
   {
      time_t t; tm* pTm;
      ::time(&t);// THIS IS THE REPORTED ACTUAL TIME
      pTm = ::gmtime(&t);
      ::strftime(tmpBuf, 20, "%Y-%m-%dT%H:%M:%S", pTm); tmpBuf[20]=0;
      ::sprintf(buf, fx8, tmpBuf);
      ::fwrite(buf, 80, 1, of); hSize--;
   }
   ::fwrite(fx9, 80, 1, of); hSize--;
   // cam string
   ::sprintf(tmpBuf, "%20s", m_caps.CamModel.c_str()); tmpBuf[20]=0;
   ::sprintf(buf, fxa, tmpBuf);
   ::fwrite(buf, 80, 1, of); hSize--;
   // pixel size
   ::sprintf(buf, fxb, m_caps.PixelSizeX);
   ::fwrite(buf, 80, 1, of); hSize--;
   ::sprintf(buf, fxc, m_caps.PixelSizeY);
   ::fwrite(buf, 80, 1, of); hSize--;
   // swcreate
   ::fwrite(fxd, 80, 1, of); hSize--;
   // imagetype
   ::sprintf(buf, fxe, frameTypeS);
   ::fwrite(buf, 80, 1, of); hSize--;
   // binning
   ::sprintf(buf, fxf, m_artSample.binX);
   ::fwrite(buf, 80, 1, of); hSize--;
   ::sprintf(buf, fxg, m_artSample.binY);
   ::fwrite(buf, 80, 1, of); hSize--;
   // exptime
   float et = float(m_artSample.expotime) / 1000.0;
   ::sprintf(buf, fxh, et);
   ::fwrite(buf, 80, 1, of); hSize--;

   // end
   ::fwrite(fxx, 80, 1, of); hSize--;

   // fill the rest of the 2880 bytes header area
   for (size_t i=0; i<hSize; i++) {
      ::fwrite(fxX, 80, 1, of);
   }
}


//! @brief  Write the FITS image data to a file
//! @param  FILE* of: the output file
//! @param  unsigned short buf[]: the image buffer
//! @param  size_t bSize: the image buffer size [bytes]
void Atik16Drv::WriteFitsData(FILE* of, const unsigned short buf[], size_t bSize)
{
   _V_ ::fprintf(stderr, "Atik16Drv::WriteFitsData() \n");
   const _word* pB = buf;
   const size_t Chunk = 2048;
   _word xBuf[Chunk];

   size_t       toWrite = bSize/2; // writing words
   size_t       numChunks = toWrite/Chunk;

   // we write chunks until we have only little or nothing left
   for (size_t i=0; i<numChunks; i++) {
      ::memcpy(xBuf, pB, Chunk*2);
      //have to adjust bytes of word for FITS format which is big endian
      for (size_t x=0; x<Chunk; x++) {long t=long(xBuf[x])-32768L; xBuf[x] = xINT16_SWAP_ON_LE( _word(t) );};
      ::fwrite(xBuf, Chunk, sizeof(_word), of);
      pB+=Chunk; toWrite-=Chunk;
   }
   // still something left - do it now
   if (toWrite) {
      ::memcpy(xBuf, pB, toWrite*2);
      //have to adjust bytes of word for FITS format which is big endian
      for (size_t x=0; x<toWrite; x++) {long t=long(xBuf[x])-32768L; xBuf[x] = xINT16_SWAP_ON_LE( _word(t) );};
      ::fwrite(xBuf, toWrite, sizeof(_word), of);
   }
}


//! @brief  Write a trivial FITS image to a file
//! @param  const char fName[]: the output filename
//! @param  char frameTypeS[]: a string used as filetype keyword
//! @return bool: returns true on success, otherwise false
bool Atik16Drv::WriteTmpFits(const char fName[], char frameTypeS[])
{
   _V_ ::fprintf(stderr, "Atik16Drv::WriteTmpFits() \n");
   if (m_artSample.ready) {
      // write a trivial FITS file from the data (no checks at all here)
      FILE* of = ::fopen(fName, "wb");
      WriteFitsHeader(of, frameTypeS);
      WriteFitsData(of, m_artSample.imageMem->WordMemPtr(), m_artSample.dataLength);
      ::fclose(of);
      return true;
   }
   return false;
}

