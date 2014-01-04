

#ifndef __ARTEMISDRVSIM_H__
#define __ARTEMISDRVSIM_H__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/filename.h>

#include "ArtDrv.h"

// Implements an Artemis Driver Proxy that uses the Serial IO

// Implemented as Singleton

class ArtSimThread;
class ArtDrvSim : public ArtDrv
{
public:
   static ArtDrvSim* Instance();

protected:
   // hide
   ArtDrvSim();
   ArtDrvSim(const ArtDrvSim&);
   ArtDrvSim& operator= (const ArtDrvSim&);

private:
   static ArtDrvSim* pinstance;

public:
   virtual ~ArtDrvSim();


public:
   EArtVideoTechnology VideoTechnology() const {return EAV_ArtSim;};

   //! @brief  The following methods accept a Device Index which is used to differentiate
   //! @brief  multiple cameras attached to the computer

   //! @brief  Get USB Identifier of Nth USB device. Return false if no such device.
   //! @brief  pName must be at least 40 chars long.
   bool DeviceName(unsigned short Device, char *pName);

   //! @brief  Get USB Serial number of Nth USB device. Return false if no such device.
   //! @brief  pName must be at least 40 chars long.
   bool DeviceSerial(unsigned short Device, char *pName);

   //! @brief  Return true if Nth USB device exists and is a camera.
   bool DeviceIsCamera(unsigned short Device);

   //! @brief  Connect to given device.
   //! @brief  Sets the internal handle if connected as requested
   //! @brief  returns true when successfull otherwise false
   bool Connect(unsigned short Device);


   //! @brief  All methods below are using the handle created with Connect

   ARTEMISERROR ReconnectUSB();

   //! @brief  Return the last FT USB error condition seen
   //! @brief  Calling this function clears the internal error state (FT_OK=0)
   int DiagnosticUSBError();

   //! @brief  Ping the camera, return the result. -1 on error.
   int DiagnosticPing(int send);

   //! @brief  Disconnect from given device.
   //! @brief  Returns true if disconnected as requested
   bool Disconnect();

   //! @brief  Returns true if currently connected to a device
   bool IsConnected() const;

   //! @brief  Retrieve the current camera state
   ARTEMISCAMERASTATE CameraState() const;

   //! @brief  Fills in pProp with camera properties
   ARTEMISERROR Properties(ARTEMISPROPERTIES& pProp);

   //! @brief  Set download thread to high or normal priority
   ARTEMISERROR HighPriority(bool bHigh);


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
   bool AmplifierSwitched() const;
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

   //! @brief  cooling support
   void TemperatureSensorInfo(unsigned short sensor, int& temperature); // °C *100
   void CoolingInfo(ECamCoolingInfo& flags, int& level, int& minlvl, int& maxlvl, int& setpoint);
   ARTEMISERROR SetCooling(int setpoint);
   ARTEMISERROR SetCooling(double temperature);
   ARTEMISERROR CoolerWarmUp();


private:
   friend class ArtSimThread; // allow access to members from thread

   bool ReadImage(const wxFileName &simName);

   void SetSimProperties(EArtCameraType camType);
   bool CheckSimFile(wxFileName &simName, unsigned short Device);

   wxString           m_driverName;

   bool               m_connected;
   bool               m_abortExposure;
   ARTEMISCAMERASTATE m_camState;
   ARTEMISPROPERTIES  m_camProps;

// simulation context
   ArtSimThread*  m_pSimThread; // the receiver thread class
   bool           m_aborted;
   wxFileName     m_simFilename;

   unsigned short m_ccdsx, m_ccdsy, m_ccdw, m_ccdh;
   unsigned short m_binx, m_biny;
   unsigned short m_maxBin;
   bool           m_ampSwitched;
   float          m_lastExpoDuration;
   unsigned long  m_expoDuration;
   wxLongLong     m_startExpoTime;
   wxLongLong     m_endExpoTime;
   ArtSampleType  m_artSample;
   int            m_dlPercent;
   unsigned long  m_dlRate;
   unsigned short*      m_imageData;

   // cooling
   ECamCoolingInfo m_coolingBaseFlags;
   int            m_actualTemp; // °C *100
   int            m_setpoint; // °C *100
   bool           m_warmingUp;
   bool           m_coolingOn;
   bool           m_setpCtrl;

};

#endif  // __ARTEMISDRVSIM_H__
