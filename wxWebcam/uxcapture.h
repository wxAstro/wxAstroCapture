
#ifndef _uxcapture_H_
#define _uxcapture_H_

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if defined(__UNIX__)

#include "captypes.h" // enums and structs of the capture environment
#include "capturebase.h" // abstract webcam capture classes


// Forwards
class CV4Lstream;
class CV4LwcProp;
class CV4LcapFilterItem;
class ByteImageMem;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This class serves as unified interface for the implemented DirectShow capabilities
// Once instantiated it enumerates the capture devices a first time (the enumerator is a singleton)
// Each enumeration may return a different list as devices may have plugged in or out
// So make sure to use CapDeviceEntries only while they are valid
class CUXcapture :public CaptureBase
{

public:
	 CUXcapture();
	~ CUXcapture();

   EOSKind Implementation() const {return EOSK_Linux;};

	// error tracking: return the last HRESULT from a call if it returns false
	long LastHResult() const;

	// (Re)-Enumerate the video capture devices
	bool EnumerateCapFilter();

	// The number of device entries that can be retrieved - the index is 0-based
	// e.g.  for (i=0; i<NumItems(); i++) {}
	size_t NumItems();

	// Get the filter entry with the index (0..NumItems-1)
	const CapDeviceEntry& CapDevEntry(size_t cfIndex);


	// *** Camera Properties ***

	// Allows access to capture device properties
	// some devices are supported with extended properties, other only with the basic
	// ones that are exposed to standard interfaces such as VideoProcAmp etc. (see DS docs)

	// Property Interfaces

	// get property access to the capture device with Index
	bool ControlDevice(long cfIndex);
	// drop property access to the capture device
	void DropControlledDevice();

	// Get the filter attributes of the controlled device
	CapDeviceEntry& ControlledCapDevEntry();

	// Get the Chipset of the controlled device
	const EKnownChipsetCat Chipset() const {return CSX_SAA8116;}; //TODO make it right

	// Get the Property 'prop' - value range native or percent 0..100
    long Property(EVidCapProperty prop, bool percent = false);

	// Set the Property 'prop' - value range native or percent 0..100
    void SetProperty(EVidCapProperty prop, long p_propValue, bool percent = false);

	// Get the supported access for Property 'prop'
	EVidCapPropertySupport PropertyHas(EVidCapProperty prop);

	// Get the range values for Property 'prop'
	long PropertyMin(EVidCapProperty prop);
	long PropertyMax(EVidCapProperty prop);
	long PropertyStep(EVidCapProperty prop);



	// *** Camera Streaming ***

	// create a new capture graph connecting to the controlled device
	// submit a valid window handle which receives the video output
	bool CreateNew(long hWnd);

	// query the current video frame dimension
	void FrameDimension(wxUint16 &width, wxUint16 &height) const;
	// Set the Format of the Capture OutPin (if supported) according to a binning level
	// only bin=1 is supported
	bool SetFormat(wxUint16 binning);
   wxUint16 MaxFormat();

   // set the subframe format (binning factor 1 assumed)
   //  needs two points of a rectangle that makes the subframe in pixel coords
   //  the subframe might get adjusted if binning changes
   //  switching back to the max frame is done by setting all points to 1
   bool SetSubframe(wxUint16 startX, wxUint16 startY,
                                wxUint16 endX, wxUint16 endY);

	// make the video output to match the hWnd size of the used window (from CreateNew)
	// call it whenever the video should be readjusted within the control window
	void ScaleVideoOutput();
   void VideoOutputVisible(bool enable);

	// query the status of the object
	// returns the GetState() result of the graph
	long Status() const;

	// set graph to pause
	bool Pause();
	// set graph to stop
	bool Stop();
	// set graph to run
	bool Run();

	// The app model is either to provide a Wnd Handle that receives WM_CAPTURE
	// events which in turn must trigger the call of OnCapture()
	// If no EventSink is given, the app must poll OnCaptureRequired() and
	// call OnCapture() when it returns true.
	// This flow is needed because the DS Filter must not do any dataprocessing but
	// return asap - so any further capture handling is performed in the context
	// of the application when calling OnCapture()


	// submit a window handle to receive the Post message WM_CAPTURE
	// usually the dialog hWnd is submitted here
	const long EventSink() const;
	// query the used window handle
	bool put_EventSink(long newVal);

	// returns if a an OnCapture call is expected
	// supports non event driven apps i.e. EventSink is not provided
	ESYNMode OnCaptureRequired();
	// must be called to gather the sampled image
	// either via WM_CAPTURE handler or polled via OnCaptureRequired
	//
	// OnCapture returns 0 if a new sample must be collected
	// positive numbers are capture events left until finished
	// negative numbers are error indications
	long OnCapture();

   long OnTimer() {return 0;}; // does nothing

	// fills the submitted sample object with data
	// NOTE: the sample is allocated and will be released with the object
	void CapturedSample(wxWebSample& sample);
	// just reply to ignore that sample and continue
	void DropCapturedSample();

	// init and capture images - calls back counting down # images
	bool CaptureStream(bool enabled, size_t countdown);

	// query the current type of streaming
	const EGraphStreamingType CaptureStatus() const;


	// control the capture filters behavior
	// query the current conversion type
	const EVideoConversion WCFConversion();
	// set a new conversion type
	void put_WCFConversion(EVideoConversion newVal);
	// query the current pixel order (for raw only)
	const EVideoPixelOrder PixelOrder();
	// set the new pixel order (for raw only)
	void put_PixelOrder(EVideoPixelOrder newVal);
   // get the current FPS/DPS values  //20100702:BM - added
   bool GetRates(float& fps, float& dps);

	bool ShowSourceDlg(long hwndParent) {return false;}; // not supported
	bool ShowFormatDlg(long hwndParent) {return false;}; // not supported


private:
	// a generic device
	CapDeviceEntry     m_pCapDeviceEntry;          // the exported entry
	// The controlled device
	CapDeviceEntry     m_pCapDeviceEntryControlled;// the exported entry
	CV4LcapFilterItem*  m_pDsCapFilterItem;         // a device context

	// the internal objects used to make it work
	bool              m_streamUp;
	CV4LwcProp*       m_pProp;                   // a property object
	CV4Lstream*       m_pStream;                 // a stream object

	long              m_lastHResult;              // the latest HRESULT from a call

   int               m_device;   // the Linux device handle
   wxImage*          m_videoWindow;  // ref to image that receives the content

	// Helpers
	void            RenewSettingCache();
	bool            m_settingCacheValid;

    // downsample the timer tic according to the current FPS setting
    // avoids event overruns on slower framerates
	int             m_timedivider;  // range 1..3
	int             m_timetic;      // fire at 0

	struct Settings
	{
		long Val2Pct(long val){return (long)((float)(val-min)*cnv2Pct);}
		long Pct2Val(long pct){return (long)((float)pct/cnv2Pct+min);}
		void SetConversion(){cnv2Pct = (long)100.0F / (float)(( (max-min)==0 )?100000.0F:(max-min) );} // avoid zerodiv
		// vars
		EVidCapPropertySupport has;
		long min; long max; long step;
		float cnv2Pct; //value * cnv2Pct = percent
	};
	Settings        m_settingCache[VCP_LastElement+1];

   ByteImageMem*   m_pBGR;
   ByteImageMem*   m_pYUV;

   wxRect         m_subF;
	bool           m_subF_fullFrame;

};

#endif // defined __UNIX__
#endif // _uxcapture_H_
