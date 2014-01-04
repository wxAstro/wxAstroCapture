
#ifndef _CAPTUREBASE_H__
#define _CAPTUREBASE_H__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


#include "captypes.h" // enums and structs of the capture environment

#include "wxWebSample.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declares
class CDScapture;
class CUXcapture;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Declare a CapDeviceEntry
//
class CapDeviceEntry
{
// Allow initialization of content by owner class
friend class CDScapture;
friend class CUXcapture;


public:
    // constructor inits the content as an invalid device (index = -1)
	CapDeviceEntry();
	virtual ~CapDeviceEntry(){};

	// The device index
	size_t DevIndex(void) const;
	// The friendly device name (any two cams of the same brand have the same name)
	const wxString& DevName(void) const;
	// The MS serial assigned (serves to distinguis between cams of the same brand)
	const wxString& DevSerial(void) const;
	// The device path such as /dev/videoN
	const wxString& DevPath(void) const;

private:
	// hidden from general access, only the provider can use it
	void Init(size_t pFIndex, const wxString& pFName, const wxString& pDevSerial, const wxString& pDevPath);
	// Capture Device attribute storage
	struct SDevEntry
	{
		SDevEntry(){ Init();}
		void Init(){
			FIndex=0;
			FName.Clear(); DevSerial.Clear(); DevPath.Clear();
		}

		size_t FIndex;         // The index of the entry
		wxString FName;      // The cameras friendly name - show this one
		wxString DevSerial;  // A serial number if we have to cams with the same name
		wxString DevPath;    // The path /dev/videoN
	};

	SDevEntry   m_DevEntry; // the real storage
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Abstract capture classes
//
class CaptureBase
{
public:
	virtual ~CaptureBase(){};


   virtual EOSKind Implementation() const =0;

	virtual long LastHResult() const =0;

	// (Re)-Enumerate the video capture devices
	virtual bool EnumerateCapFilter() =0;

	// The number of device entries that can be retrieved - the index is 0-based
	// e.g.  for (i=0; i<NumItems(); i++) {}
	virtual size_t NumItems() =0;

	// Get the filter entry with the index (0..NumItems-1)
	virtual const CapDeviceEntry& CapDevEntry(size_t cfIndex) =0;

	// *** Camera Properties ***

	// Allows access to capture device properties
	// some devices are supported with extended properties, other only with the basic
	// ones that are exposed to standard interfaces such as VideoProcAmp etc. (see DS docs)

	// Property Interfaces

	// get property access to the capture device with Index
	virtual bool ControlDevice(long cfIndex) =0;
	// get property access to the capture device with Index
	virtual void DropControlledDevice() =0;

	// Get the filter attributes of the controlled device
	virtual CapDeviceEntry& ControlledCapDevEntry() =0;

	// Get the Chipset of the controlled device
	virtual const EKnownChipsetCat Chipset() const =0;

	// Get the Property 'prop' - value range native or percent 0..100
    virtual long Property(EVidCapProperty prop, bool percent = false) =0;

	// Set the Property 'prop' - value range native or percent 0..100
    virtual void SetProperty(EVidCapProperty prop, long p_propValue, bool percent = false) =0;

	// Get the supported access for Property 'prop'
	virtual EVidCapPropertySupport PropertyHas(EVidCapProperty prop) =0;

	// Get the range values for Property 'prop'
	virtual long PropertyMin(EVidCapProperty prop) =0;
	virtual long PropertyMax(EVidCapProperty prop) =0;
	virtual long PropertyStep(EVidCapProperty prop) =0;



	// *** Camera Streaming ***

	// create a new capture graph connecting to the controlled device
	// submit a valid window handle which receives the video output
	// and a hander that receives the posted events
//	virtual bool CreateNew(wxImage *hWnd, wxEvtHandler *p_eventHandler) =0;

	// create a new capture graph connecting to the controlled device
	// submit a valid window handle which receives the video output
	virtual bool CreateNew(long hWnd) =0;

	// query the current video frame dimension
	virtual void FrameDimension(unsigned short& width, unsigned short& height) const =0;
	// Set the Format of the Capture OutPin (if supported) according to a binning level
	// i.e. level 1 = 640x480; 2 = 320x240; 4 = 160x120; 5 = 352x288 cropped
	virtual bool SetFormat(unsigned short binning) =0;
	virtual unsigned short MaxFormat() =0;

   // set the subframe format (binning factor 1 assumed)
   //  needs two points of a rectangle that makes the subframe in pixel coords
   //  the subframe might get adjusted if binning changes
   //  switching back to the max frame is done by setting all points to 1
   virtual bool SetSubframe(unsigned short startX, unsigned short startY,
                            unsigned short endX, unsigned short endY) =0;

	// make the video output to match the hWnd size of the used window (from CreateNew)
	// call it whenever the video should be readjusted within the control window
	virtual void ScaleVideoOutput() =0;
   virtual void VideoOutputVisible(bool enable) =0;

	// query the status of the object
	// returns the GetState() result of the graph
	virtual long Status() const =0;

	// set graph to pause
	virtual bool Pause() =0;
	// set graph to stop
	virtual bool Stop() =0;
	// set graph to run
	virtual bool Run() =0;

	// The app model is either to provide a Wnd Handle that receives WM_CAPTURE
	// events which in turn must trigger the call of OnCapture()
	// If no EventSink is given, the app must poll OnCaptureRequired() and
	// call OnCapture() when it returns true.
	// This flow is needed because the DS Filter must not do any dataprocessing but
	// return asap - so any further capture handling is performed in the context
	// of the application when calling OnCapture()


	// submit a window handle to receive the Post message WM_CAPTURE
	// usually the dialog hWnd is submitted here
	virtual const long EventSink() const =0;
	// query the used window handle
	virtual bool put_EventSink(long newVal) =0;

	// returns  if a an OnCapture call is expected
	// supports non event driven apps i.e. EventSink is not provided
	virtual ESYNMode OnCaptureRequired() =0;
	// must be called to gather the sampled image
	// either via WM_CAPTURE handler or polled via OnCaptureRequired
	//
	// OnCapture returns 0 if a new sample must be collected
	// positive numbers are capture events left until finished
	// negative numbers are error indications
	virtual long OnCapture() =0;
	virtual long OnTimer() =0;

	// fills the submitted sample object with data
	// NOTE: the sample is allocated and will be released with the object
	virtual void CapturedSample(wxWebSample& sample) =0;
	// just reply to ignore that sample and continue
	virtual void DropCapturedSample() =0;

	// init and capture images - calls back counting down # images
	virtual bool CaptureStream(bool enabled, size_t countdown) =0;
	// query the current type of streaming
	virtual const EGraphStreamingType CaptureStatus() const =0;


	// control the capture filters behavior
	// query the current conversion type
	virtual const EVideoConversion WCFConversion() =0;
	// set a new conversion type
	virtual void put_WCFConversion(EVideoConversion newVal) =0;
	// query the current pixel order (for raw only)
	virtual const EVideoPixelOrder PixelOrder() =0;
	// set the new pixel order (for raw only)
	virtual void put_PixelOrder(EVideoPixelOrder newVal) =0;
   // get the current FPS/DPS values  //20100702:BM - added
   virtual bool GetRates(float& fps, float& dps) =0;


	// provide access to the manufacturers dialogs
	virtual bool ShowSourceDlg(long hwndParent) =0;
	virtual bool ShowFormatDlg(long hwndParent) =0;

protected:
	CaptureBase(){};
};

#endif // _CAPTUREBASE_H__


