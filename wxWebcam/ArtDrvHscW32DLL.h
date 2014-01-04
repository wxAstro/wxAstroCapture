

#ifndef _ARTDRVHSCW32DLL_H_
#define _ARTDRVHSCW32DLL_H_

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if defined(WIN32)

#include "ArtDrv.h"


//! implements a wrapper for the Artemis HSC DLL

// Implemented as Singleton
class ArtDrvHscW32DLL : public ArtDrv
{
public:
   static ArtDrvHscW32DLL* Instance();

protected:
   // hide
   ArtDrvHscW32DLL();
   ArtDrvHscW32DLL(const ArtDrvHscW32DLL&);
   ArtDrvHscW32DLL& operator= (const ArtDrvHscW32DLL&);

private:
   static ArtDrvHscW32DLL* pinstance;

public:
   virtual ~ArtDrvHscW32DLL();

public:
   EArtVideoTechnology VideoTechnology() const {return EAV_ArtHsc_Dll;};

   // The following methods accept a Device Index which is used to differentiate
   // multiple cameras attached to the computer

   // Get USB Identifier of Nth USB device. Return false if no such device.
   // pName must be at least 40 chars long.
   bool DeviceName(unsigned short Device, char *pName);

   // Get USB Serial number of Nth USB device. Return false if no such device.
   // pName must be at least 40 chars long.
   bool DeviceSerial(unsigned short Device, char *pName);

   // Return true if Nth USB device exists and is a camera.
   bool DeviceIsCamera(unsigned short Device);

   // Connect to given device. If Device=-1, connect to first available
   // Sets the internal handle if connected as requested
   // returns true when successfull otherwise false
   bool Connect(unsigned short Device);



   // All methods below are using the handle created with Connect

   ARTEMISERROR ReconnectUSB();

   // Return the last FT USB error condition seen
   // Calling this function clears the internal error state (FT_OK=0)
   int DiagnosticUSBError();

   // Ping the camera, return the result. -1 on error.
   int DiagnosticPing(int send);

   // Disconnect from given device.
   // Returns true if disconnected as requested
   bool Disconnect();

   // Returns TRUE if currently connected to a device
   bool IsConnected() const;

   // Retrieve the current camera state
   ARTEMISCAMERASTATE CameraState() const;

   // Fills in pProp with camera properties
   ARTEMISERROR Properties(ARTEMISPROPERTIES& pProp);

   // Set download thread to high or normal priority
   ARTEMISERROR HighPriority(bool bHigh);


   // Get the pos and size of imaging subframe
   void  Subframe(unsigned short& x, unsigned short& y, unsigned short& w, unsigned short& h) const;
   // set the pos and size of imaging subframe inunbinned coords
   ARTEMISERROR SetSubframe(unsigned short x, unsigned short y, unsigned short w, unsigned short h);
   // Set the start x,y coords for imaging subframe., X,Y in unbinned coordinates
   ARTEMISERROR SetSubframePos(unsigned short x, unsigned short y);
   // Set the width and height of imaging subframe, W,H in unbinned coordinates
   ARTEMISERROR SetSubframeSize(unsigned short w, unsigned short h);



   // Get the x,y binning factors
   void  Bin(unsigned short& x, unsigned short& y) const;
   // Set the x,y binning factors
   ARTEMISERROR SetBin(unsigned short x, unsigned short y);
   // Get the max x,y binning factors
   void MaxBin(unsigned short& x, unsigned short& y);


   // Return true if amp switched off during exposures
   bool AmplifierSwitched() const;
   // Set whether amp is switched off during exposures
   ARTEMISERROR SetAmplifierSwitched(bool bSwitched);

   // Set the CCD amplifier on or off
   ARTEMISERROR SetAmp(bool bOn);

   // Return duration of last exposure, in seconds
   float LastExposureDuration() const;

   // Start an exposure
   ARTEMISERROR StartExposure(float Seconds);

   // Return time remaining in current exposure, in seconds
   float ExposureTimeRemaining() const;

   // Percentage downloaded
   int DownloadPercent() const;

   // Prematurely end an exposure, collecting image data.
   ARTEMISERROR StopExposure();

   // Abort exposure, if one is in progress
   ARTEMISERROR AbortExposure();


   // Return true if an image is ready to be retrieved
   bool ImageReady() const;

   // Retrieve image dimensions and binning factors.
   // x,y are actual CCD locations. w,h are pixel dimensions of image
   void  ImageData(unsigned short& x, unsigned short& y,
                   unsigned short& w, unsigned short& h,
                   unsigned short& binx, unsigned short& biny);

   // Return pointer to internal image buffer (actually wxUint16s)
   const unsigned short* ImageBuffer() const;

   // cooling support
   void TemperatureSensorInfo(unsigned short sensor, int& temperature); // °C *100

   void CoolingInfo(ECamCoolingInfo& flags, int& level, int& minlvl, int& maxlvl, int& setpoint);

   ARTEMISERROR SetCooling(int setpoint);
   ARTEMISERROR SetCooling(double temperature);

   ARTEMISERROR CoolerWarmUp();


private:
	bool           m_dllLoaded;
	ArtemisHandle  m_hCam;
	// we have to workaround a DLL bug (DL percent is faulty)
	long           m_msDLtimeExpected;
	wxLongLong     m_msStartDLtime;
};

#endif  // #if defined(WIN32)

#endif  // _ARTDRVHSCW32DLL_H_
