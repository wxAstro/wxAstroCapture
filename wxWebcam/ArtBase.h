
#ifndef _ARTBASE_H_
#define _ARTBASE_H_

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


#include "wxCameraTypes.h" // enums and structs of the wxCamera environment
#include "ArtTypes.h" // enums and structs of the Artemis environment

#include "ArtDevice.h"
#include "wxArtSample.h"

// The base classes to implement Artemis  CCD and HSC type cameras
// CCD = Art285/429/255 and Atik branded ones
// HSC = HighRes Artemis cameras (not supported yet)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  ArtBase     - an class that defines one of the cameras supported
//
class ArtBase
{
public:
   // Static members which to enumeration stuff
   static EOSKind Implementation() {return m_implementation;};

	// The number of device entries that can be retrieved - the index is 0-based
	// e.g.  for (i=0; i<NumItems(); i++) {}
	static size_t NumItems() {return m_numCameras;};

	// (Re)-Enumerate the artemis cameras
	static bool EnumerateArtCameras();

	// Get the camera entry with the index (0..NumItems-1)
	static const ArtDevice& ArtDevEntry(size_t cfIndex);


public:
   // Non static members which provide the camera behavior
	ArtBase();  // create a certain camera
	virtual ~ArtBase();

	bool LastHResult();

	// get access to the camera device with Index
	bool ControlDevice(long cfIndex);
	// release access
	void DropControlledDevice();

   wxString VideoTechnology();

	// Get the controlled camera device (which holds the driver access too)
	ArtDevice& ControlledArtDevEntry();

	// Get the Chipset of the controlled device
	const EKnownChipsetCat Chipset() const;

   // some Artemis cam and CCD properties
   bool HasFifo();
   bool HasExternalTrigger();
   bool HasCCDInterlaced();
   // cooling support
   bool HasCooling();
   bool  HasWarmup();
   ECoolingStatus CoolingStatus();
   float CoolingSetpoint();
   float Temperature();
   bool  SetTemperature(float temp);
   bool  SetWarmup();

	// query the camera video ccd original dimension (BIN=1)
	void CcdDimension(unsigned short& width, unsigned short& height) const;

	// query the camera video frame original dimension (BIN=1)
	void CamDimension(unsigned short& width, unsigned short& height) const;

	// query the current video frame dimension
	void FrameDimension(unsigned short& width, unsigned short& height) const;
	// Set the binning
	bool SetFormat(unsigned short binning);
	unsigned short MaxFormat();

	// return current binning
	unsigned short BinFormat();

   // set the subframe format (binning factor 1 assumed)
   //  needs two points of a rectangle that makes the subframe in pixel coords
   //  the subframe might get adjusted if binning changes
   //  switching back to the max frame is done by setting all points to 1
   bool SetSubframe(unsigned short startX, unsigned short startY,
                            unsigned short endX, unsigned short endY);

	// returns  if a an OnCapture call is expected
	// supports non event driven apps i.e. EventSink is not provided
	ESYNMode OnCaptureRequired();
	// must be called to gather the sampled image
	// either via WM_CAPTURE handler or polled via OnCaptureRequired
	//
	// OnCapture returns 0 if a new sample must be collected
	// positive numbers are capture events left until finished
	// negative numbers are error indications
	long OnCapture();
	long OnTimer();

   // Abort exposure, if one is in progress
   void AbortExposure();

	// fills the submitted sample object with data
	// NOTE: the sample is allocated and will be released with the object
	void CapturedSample(wxArtSample& sample);
	// just reply to ignore that sample and continue
	void DropCapturedSample();

	// init and capture images - calls back counting down # images
	bool CaptureImage(bool enabled, unsigned long milliseconds);
	// query the current type of streaming
	const ARTEMISCAMERASTATE CaptureStatus() const;

	// Return time remaining in current exposure, in seconds
   void TimeRemaining(bool& exposing, float& timeRemaining);

private:
   // Static member vars and stuff
	static void DropAll();

   static EOSKind          m_implementation;
   // the enumeration store
	static const size_t     MAXITEM = ACT_Art_LAST;  // cam enumerator holds so many items
	static ArtDevice*       m_pCameras[MAXITEM];    // enumerated list of cameras
	static size_t           m_numCameras;           // number of found cameras

	static const ArtDevice  m_nullCamera;           // a dummy cam entry

	// a generic device
	static ArtDevice        m_artCamera;            // the exported entry

	// we have potentially 3 different drivers to load
	static ArtDrv*          m_camCcdDrv;            // the camera driver
	static ArtDrv*          m_camHscDrv;            // the camera driver
	static ArtDrv*          m_camSimDrv;            // the camera driver


private:
   // instance specific items
   bool              m_lastHResult;  // the latest reults from a call (may be dropped later)
   int               m_lastTemp;
   long              m_lastTempCall;
   ECamCoolingInfo   m_lastInfo;
   int               m_lastSetpoint;
   long              m_lastInfoCall;

	// The controlled device
	ArtDevice         m_cam;      // the controlled camera
	ARTEMISPROPERTIES m_artProps; // cam prop record
   ArtSampleType     m_lSample;  // image transfer object (drv to base)
   EKnownChipsetCat  m_chipset;

};

#endif // _ARTBASE_H_


