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
 * @file           Atik16Drv.h
 *****************************************************************************
 * class Atik16Drv
 *
 * Artemis/ATIK USB camera driver for Linux
 * The basic driver for Artemis/ARTEMIS cameras with USB 1.1 interface
 * supported cameras are: see AtikVidPid.h
 *
 * This is part of the Driver Library of wxAstroCapture
 *
 * Implements a driver for Art16 type cameras as singleton
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


#ifndef _ATIK16DRV_H_
#define _ATIK16DRV_H_

#include "ArtTypes.h"         // enums and structs of the Artemis environment
#include "Atik16Cmd.h"         // enums and structs of the Artemis driver
#include "Atik16ThreadCB.h"    // prototype of callbacks used by thread

//! A class which implements the Atik camera commands
//! the commands are sent to the IO driver

class Atik16Thread;
class AtikThread;
class Ftd245IO;

#ifdef INDI
// driver class is only derived from callback
class Atik16Drv : public Atik16ThreadCB
{
public:
   // must be defined here - for non INDI it is in ArtDrv
   enum EArtVideoTechnology {
      EAV_ArtCcd_Dll=0,    // original Win CCD DLL
      EAV_ArtHsc_Dll,      // original Win HSC DLL
      EAV_ArtSio,          // Linux SIO driver
      EAV_ArtLibUsb,       // Linux LibUsb driver
      EAV_ArtSim,          // camera simulation
   };

#else  //! wxAstroCapture
#include "ArtDrv.h"        // abstract Artemis/ATIK driver of wxAstroCapture
// driver class is derived from callback and from the generic Artemis/ATIk driver
class Atik16Drv : public ArtDrv, public Atik16ThreadCB
{
#endif


friend class Atik16Thread; // grant all access to the thread proc

public:
   //! @brief  Get the single instance of the driver through this call
   static Atik16Drv* Instance();

protected:
	//! class construction / destruction (hidden)
   Atik16Drv();
   Atik16Drv(const Atik16Drv&);
   Atik16Drv& operator= (const Atik16Drv&);

private:
   static Atik16Drv* pinstance;  // singleton holder

public:
   virtual ~Atik16Drv();

public:
   //! @brief  Exposes the video technology used by the driver
   EArtVideoTechnology VideoTechnology() const;


   //! @brief  The following methods accept a Device Index which is used to differentiate
   //! @brief  multiple cameras attached to the computer

   //! @brief  Return true if Nth USB device exists and is a camera.
   bool DeviceIsCamera(unsigned short Device);

   //! @brief  Get USB Identifier of Nth USB device. Return false if no such device.
   //! @brief  buffer provided with pName must be at least 40 chars long.
   bool DeviceName(unsigned short Device, char pName[]);

   //! @brief  Get USB Serial number of Nth USB device. Return false if no such device.
   //! @brief  buffer provided with pSerial must be at least 40 chars long.
   bool DeviceSerial(unsigned short Device, char pSerial[]);

   //! @brief  Connect to the first available device
   //! @return bool: true if successful
   bool Connect();

   //! @brief  Connect if we know the deviceIndex
   //! @param  int deviceIndex: the device to attempt to connect
   //! @return bool: true if successful
   bool Connect(unsigned short Device);

   bool IsConnected() const {return m_connected;};

   //! @brief  All methods below are using the handle created with Connect


   //! @brief  Disconnect from given device.
   //! @brief  Returns true if disconnected as requested
   bool Disconnect();

   //! @brief  Retrieve the current camera state
   ARTEMISCAMERASTATE CameraState() const {return m_camState;};

   //! @brief  Fills in pProp with camera properties
   ARTEMISERROR Properties(ARTEMISPROPERTIES& pProp);

   //! @brief  Set download thread to high or normal priority
   ARTEMISERROR HighPriority(bool bHigh);


   //! All coords are unbinned camera pixels

   //! @brief  Get the pos and size of imaging subframe
   void  Subframe(unsigned short& x, unsigned short& y, unsigned short& w, unsigned short& h) const;
   //! @brief  set the pos and size of imaging subframe inunbinned coords
   ARTEMISERROR SetSubframe(unsigned short x, unsigned short y, unsigned short w, unsigned short h);
   //! @brief  Set the start x,y coords for imaging subframe., X,Y in unbinned coordinates
   ARTEMISERROR SetSubframePos(unsigned short x, unsigned short y);
   //! @brief  Set the width and height of imaging subframe, W,H in unbinned coordinates
   ARTEMISERROR SetSubframeSize(unsigned short w, unsigned short h);

   //! @brief  Get the x,y binning factors
   void  Bin(unsigned short& x, unsigned short& y) const;
   //! @brief  Set the x,y binning factors
   ARTEMISERROR SetBin(unsigned short x, unsigned short y);
   //! @brief  Get the max x,y binning factors
   void MaxBin(unsigned short& x, unsigned short& y);

   //! @brief  Return true if amp switched off during exposures
   bool AmplifierSwitched() const {return m_ampSwitched;};
   //! @brief  Set whether amp is switched off during exposures
   ARTEMISERROR SetAmplifierSwitched(bool bSwitched);

   //! @brief  Set the CCD amplifier on or off
   ARTEMISERROR SetAmp(bool bOn);

   //! @brief  Return duration of last exposure, in seconds
   float LastExposureDuration() const;

   //! @brief  Start an exposure
   ARTEMISERROR StartExposure(float Seconds);

   //! @brief  Return time remaining in current exposure, in seconds
   float ExposureTimeRemaining() const;

   //! @brief  Percentage downloaded
   int DownloadPercent() const;

   //! @brief  Prematurely end an exposure, collecting image data.
   ARTEMISERROR StopExposure();

   //! @brief  Abort exposure, if one is in progress
   ARTEMISERROR AbortExposure();


   //! @brief  Return true if an image is ready to be retrieved
   bool ImageReady() const;

   //! @brief  Retrieve image dimensions and binning factors.
   //! @brief  x,y are actual CCD locations. w,h are pixel dimensions of image
   void  ImageData(unsigned short& x, unsigned short& y,
                   unsigned short& w, unsigned short& h,
                   unsigned short& binx, unsigned short& biny);

   //! @brief  Return pointer to internal image buffer (actually unsigned shorts)
   const unsigned short* ImageBuffer() const;

   bool ProcessImageRead();

   //! @brief  cooling support
   void TemperatureSensorInfo(unsigned short sensor, int& temperature); // °C *100
   void CoolingInfo(ECamCoolingInfo& flags, int& level, int& minlvl, int& maxlvl, int& setpoint);
   ARTEMISERROR SetCooling(int setpoint);
   ARTEMISERROR SetCooling(double temperature);
   ARTEMISERROR CoolerWarmUp();


   //! additional commands not in the original Artemis/ARTEMIS API
   // Trivial FITS
   //! @brief  Write a trivial FITS image to a file
   //! @param  const char fName[]: the output filename
   //! @param  char frameTypeS[]: a string used as filetype keyword
   //! @return bool: returns true on success, otherwise false
   bool WriteTmpFits(const char fName[], char frameTypeS[]);

   // Tools
   ARTEMISERROR ReconnectUSB();

   //! @brief  Return the last FT USB error condition seen
   //! @brief  Calling this function clears the internal error state (FT_OK=0)
   int DiagnosticUSBError();

   //! @brief  Send a diagnostic message to the camera
   //! @param  int send:  the message
   //! @return int: returns the sent message if OK
   int DiagnosticPing(int send);

   //! @brief  Set the debug helper to verbosity level output on stderr
   //! @param  bool verbose:  enable / disable verbosity
   void SetVerbose(bool verbose);

private:
   //! @brief  Set the internal camera status
   //! @param  ARTEMISCAMERASTATE camState:  the camera status
   void SetCameraState(ARTEMISCAMERASTATE camState) {m_camState=camState;};

   //! @brief  Query the camera capabilities into the drivers own store
   //! @return - true if successful
   bool RequestCAPS();

   //! @brief  Prepare reading out the CCD
   //! @return - true if successful
   bool PrepareReadCCD();

   //! @brief  read all Input from the camera that might still be there
   void ClearCamIO();

   //! Defines and starts the exposure timeout
   //! @param waitMillisec  wait time in milliseconds
   //! @return - true if the timer could start
   bool TIMER(unsigned long waitMillisec);

   //! Query the timer part
   //! @return - true if the timer is running
   bool TimedWait() {return m_timedWaiting;};

   //! @brief  Write the FITS Header data to a file
   //! @param  FILE* of: the output file
   //! @param  char frameTypeS[]: a string used as filetype keyword
   void WriteFitsHeader(FILE* of, char frameTypeS[]);

   //! @brief  Write the FITS image data to a file
   //! @param  FILE* of: the output file
   //! @param  unsigned short buf[]: the image buffer
   //! @param  size_t bSize: the image buffer size [bytes]
   void WriteFitsData(FILE* of, const unsigned short buf[], size_t bSize);

private:
   //! implements thread callbacks - don't use !!!!
   ArtSampleType* GetImageBuffer() {return &m_artSample;};

   //! @brief  Callback from the ReceiverThread when DL is finished
   //! @param  bool readError: true if a read error occured
   //! @param  long realExpotime: the measured exposure time
   //! @param  long downloadBps: the measured download rate in bytes/sec
   void ProcessImageRead(bool readError, long realExpotime, long downloadBps);

   //! @brief  Callback from the ReceiverThread when timer expires with success
   void ProcessTimeElapsed();

   //! @brief  Callback from the ReceiverThread to start the camera readout
   //! @brief    the command must be setup with PrepareReadCCD before
   void ProcessTriggerRDCCD();

   //! @brief  Callback from the ReceiverThread while downloading data
   //! @param  long downloadPercent: download completion % value 0.. 100
   void ProcessTriggerDownload(long downloadPercent);

   //! @brief  Callback from the ReceiverThread when the worker is about to start its main loop
   void ProcessThreadStart();
   //! @brief  Callback from the ReceiverThread when the worker is about to exit
   //! @param  int exitCode:  the thread exit code
   void ProcessThreadEnd(int exitCode);


private:
   Ftd245IO*          m_ezCmd;     // ptr to the CMD driver library
   AtikThread*        m_thread;
   Atik16Thread*      m_receiver;

   unsigned char      m_ezFWmajor, m_ezFWminor;
   unsigned char      m_camSerial[6];
   unsigned char      m_hwRevIndex;

   ARTEMISCAMERASTATE m_camState;
   bool               m_connected;
   bool               m_ampSwitched;
   bool               m_imageAquisition;
   bool               m_timedWaiting;
   long               m_downloadPercent;
   long               m_downloadBps;
   timeval            m_expoStartClock;

   ArtSampleType      m_artSample;        // a dataobj to transfer imagedata
   WordImageMem*      m_artSampleHelper;  // helper space for deinterlacing

   Atik16Cmd::AtikCamCapsRec     m_caps;   // camera caps (read once)
   Atik16Cmd::AtikCamExposureRec m_expoRecord;
   Atik16Cmd::AtikDevDescRec     m_dev;    // device descriptor
   unsigned char      m_rccdBuf[20];      // return buffer

};

#endif  // _ATIK16DRV_H_
