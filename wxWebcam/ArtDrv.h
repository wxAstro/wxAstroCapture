

#ifndef _ARTDRV_H_
#define _ARTDRV_H_

//#include "wxCameraTypes.h" // enums and structs of the Camera environment
#include "ArtTypes.h"      // enums and structs of the Artemis environment

//! implements an abstract Artemis Driver (similar to the Artemis DLL)

class ArtDrv
{

public:
   virtual ~ArtDrv(){};

   enum EArtVideoTechnology {
      EAV_ArtCcd_Dll=0,    // original Win CCD DLL
      EAV_ArtHsc_Dll,      // original Win HSC DLL
      EAV_ArtSio,          // Linux SIO driver
      EAV_ArtLibUsb,       // Linux LibUsb driver
      EAV_ArtSim,          // camera simulation
   };

   //! @brief  Exposes the video technology used by the driver
   virtual EArtVideoTechnology VideoTechnology() const =0;

   // The following methods accept a Device Index which is used to differentiate
   // multiple cameras attached to the computer

   //! @brief  Return true if Nth USB device exists and is a camera.
   virtual bool DeviceIsCamera(unsigned short Device) =0;

   //! @brief  Get USB Identifier of Nth USB device. Return false if no such device.
   //! @brief  pName must be at least 40 chars long.
   virtual bool DeviceName(unsigned short Device, char *pName) =0;

   //! @brief  Get USB Serial number of Nth USB device. Return false if no such device.
   //! @brief  pName must be at least 40 chars long.
   virtual bool DeviceSerial(unsigned short Device, char *pName) =0;


   // Connect to given device.
   // Sets the internal handle if connected as requested
   // returns true when successfull otherwise false
   virtual bool Connect(unsigned short Device) =0;




   // All methods below are using the handle created with Connect

   virtual ARTEMISERROR ReconnectUSB() =0;

   // Return the last FT USB error condition seen
   // Calling this function clears the internal error state (FT_OK=0)
   virtual int DiagnosticUSBError() =0;

   // Ping the camera, return the result. -1 on error.
   virtual int DiagnosticPing(int send) =0;

   // Disconnect from given device.
   // Returns true if disconnected as requested
   virtual bool Disconnect() =0;

   // Returns TRUE if currently connected to a device
   virtual bool IsConnected() const =0;

   // Retrieve the current camera state
   virtual ARTEMISCAMERASTATE CameraState() const =0;

   // Fills in pProp with camera properties
   virtual ARTEMISERROR Properties(ARTEMISPROPERTIES &pProp) =0;

   // Set download thread to high or normal priority
   virtual ARTEMISERROR HighPriority(bool bHigh) =0;


   // Get the pos and size of imaging subframe
   virtual void  Subframe(unsigned short& x, unsigned short& y, unsigned short& w, unsigned short& h) const =0;
   // set the pos and size of imaging subframe inunbinned coords
   virtual ARTEMISERROR SetSubframe(unsigned short x, unsigned short y, unsigned short w, unsigned short h) =0;
   // Set the start x,y coords for imaging subframe., X,Y in unbinned coordinates
   virtual ARTEMISERROR SetSubframePos(unsigned short x, unsigned short y) =0;
   // Set the width and height of imaging subframe, W,H in unbinned coordinates
   virtual ARTEMISERROR SetSubframeSize(unsigned short w, unsigned short h) =0;



   // Get the x,y binning factors
   virtual void  Bin(unsigned short& x, unsigned short& y) const =0;
   // Set the x,y binning factors
   virtual ARTEMISERROR SetBin(unsigned short x, unsigned short y) =0;
   // Get the max x,y binning factors
   virtual void MaxBin(unsigned short& x, unsigned short& y) =0;

   // Return true if amp switched off during exposures
   virtual bool AmplifierSwitched() const =0;
   // Set whether amp is switched off during exposures
   virtual ARTEMISERROR SetAmplifierSwitched(bool bSwitched) =0;

   // Set the CCD amplifier on or off
   virtual ARTEMISERROR SetAmp(bool bOn) =0;


   // Return duration of last exposure, in seconds
   virtual float LastExposureDuration() const =0;

   // Start an exposure
   virtual ARTEMISERROR StartExposure(float Seconds) =0;

   // Return time remaining in current exposure, in seconds
   virtual float ExposureTimeRemaining() const =0;

   // Percentage downloaded
   virtual int DownloadPercent() const =0;

   // Prematurely end an exposure, collecting image data.
   virtual ARTEMISERROR StopExposure() =0;

   // Abort exposure, if one is in progress
   virtual ARTEMISERROR AbortExposure() =0;


   // Return true if an image is ready to be retrieved
   virtual bool ImageReady() const =0;

   // Retrieve image dimensions and binning factors.
   // x,y are actual CCD locations. w,h are pixel dimensions of image
   virtual void  ImageData(unsigned short& x, unsigned short& y,
                           unsigned short& w, unsigned short& h,
                           unsigned short& binx, unsigned short& biny) =0;

   // Return pointer to internal image buffer
   virtual const unsigned short* ImageBuffer() const =0;

   // cooling support
   virtual void TemperatureSensorInfo(unsigned short sensor, int& temperature) =0; // °C *100
   virtual void CoolingInfo(ECamCoolingInfo& flags, int& level, int& minlvl, int& maxlvl, int& setpoint) =0;
   virtual ARTEMISERROR SetCooling(int setpoint) =0;
   virtual ARTEMISERROR SetCooling(double temperature) =0;
   virtual ARTEMISERROR CoolerWarmUp() =0;


};

#endif  // _ARTDRV_H_
