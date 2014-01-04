// V4Lstream.h : Declaration of the CV4Lstream
//

#ifndef __v4l2stream_H_
#define __v4l2stream_H_

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if defined(__UNIX__)

#include "wx/wfstream.h"
#include "wx/zipstrm.h"

#include "wx/thread.h"

/////////////////////////////////////////////////////////////////////////////

#include "captypes.h"
#include "v4l2capfilter.h"
#include "DoubleMap.h"
#include "iv4l2wcfCtrl.h"   // defines the baseclass CSampleCB


/// ////////////////////////////////////////////////////////////////////////
//
class CV4Lstream; // forward declare

//!@brief The communication item of the stream and the filter thread
//!@brief Derives from the generic Sample Callback
//!@brief Implements access regulation through a crtitical section
//!@brief Maintains the sample buffer status machine
class CSampleGrabberCB : public CSampleCB
{
friend class CV4Lstream; // grant unlimited access

public:
    CSampleGrabberCB( );
    bool  SampleCBX(wxUint8* pSampleNative, size_t lengthNative,
                    wxUint8* pSampleBgr, size_t lengthBgr,
                    EVideoConversion conversion);
   //20100701:BM - implement framedropping in the filter without processing the image
   bool  IsSampleCBXready();

private:
	// maintains the capture state machine see cpp file for details
	enum ECaptureState {
		BUF_IDLE             = -1,
		BUF_READY_FORSAMPLE  =  0,
		BUF_SAMPLE_ARRIVED   =  1,
		BUF_SAMPLE_PREPARED  =  2
	};

	// structure of callback communication with Sample Grabber
	// this shall be allocated per instance of the CV4Lstream object
	static const long ImageBufferSize;
	struct CALLBACKINFO
	{
		CALLBACKINFO() : pImageYuv(NULL), sizeYUV(0), pImageBgr(NULL), sizeBGR(0) { Init();}
		void Init(){// set all values to 0 or default
			onCaptureFlag=false; capTick=false; nativeRGB=false;
			captureFlag=false; countdown=0;
			bufferStatus=BUF_IDLE; validBufferSize=0;
			memset(&bmih, 0, sizeof(BITMAPINFOHEADER));  bmih.biSize=sizeof(BITMAPINFOHEADER);
		}

		wxCriticalSection    CrSection;
		bool                 onCaptureFlag;  // true if we expect such a call
      bool                 capTick;

		BITMAPINFOHEADER     bmih;     // maintain this one while reconnecting the cam
      bool                 nativeRGB;
		// capture mode
		bool                 captureFlag;    // TRUE if capturing enabled
		long                 countdown;      // a sync element - get the bitmap only if 0 (zero)

		// any mode
		long                 startX, startY;
		long                 width, height;

		ECaptureState        bufferStatus;   // BUF_XY consts
		wxUint8*             pImageYuv;      // image buffer for the YUV bitmap (if available)
		long                 sizeYUV;        // length of the buffer submitted
		wxUint8*             pImageBgr;      // image buffer for the BGR bitmap
		long                 sizeBGR;        // length of the buffer submitted
		size_t               validBufferSize;// how much of the above BGR buffer is filled with valid data
	};

	CV4Lstream*  pOwner;  // receives the this pointer of the owning object

};//class CSampleGrabberCB


/// /////////////////////////////////////////////////////////////////////////
// CV4Lstream
class CV4Lwcfilter;
class V4L2Thread;

//!@brief The video4Linux2 stream class
class CV4Lstream
{
public:
	CV4Lstream(int p_device);
	~CV4Lstream();


// Window Subclassing class to catch some events
//BEGIN_MSG_MAP(CV4Lstream)
//	  MESSAGE_HANDLER( WM_TIMER, OnTimer )
//	  MESSAGE_HANDLER( WM_FGNOTIFY, OnNotify )
//	  MESSAGE_HANDLER( WM_DEVICECHANGE, OnDevChange )
//	  MESSAGE_HANDLER( WM_CAPTURE, OnCapture )
//END_MSG_MAP()

friend class CSampleGrabberCB; // uses ours


   enum io_method { IO_METHOD_READ, IO_METHOD_MMAP, IO_METHOD_USERPTR};

	enum GraphStreamingType
	{
		GST_Off         = 0,
		GST_Stream      = 1,
		GST_Collect     = 2,
		GST_Classify    = 3
	};

public:

	//! Interface

	//!@brief create a new capture graph connecting to the capture device submitted
	//!@brief submit a window handle which receives the video output
	bool CreateNew(const CV4LcapFilterItem& capEntry);

	//!@brief query the current video frame dimension
	void FrameDimension(wxUint16 &width, wxUint16 &height) const;

	//!@brief Set the Format of the Capture OutPin (if supported) according to a binning level
	//!@brief i.e. level 1 = 640x480; 2 = 320x240; 4 = 160x120; 5 = 352x288 cropped
	bool SetFormat(wxUint16 binning);

	//!@brief make the video output to match the hWnd size of the used window (from CreateNew)
	//!@brief call it whenever the video should be readjusted within the control window
	void ScaleVideoOutput();
   void VideoOutputVisible(bool enable);


	//! *** capture graph control

	//!@brief query the status of the object
	//!@return the GetState() result of the graph
	long Status() const;

	//!@brief set graph to pause
	bool Pause();
	//!@brief set graph to stop
	bool Stop();
	//!@brief set graph to run
	bool Run();


	//! *** capture handling

	//!@brief init and capture images - calls back after countdown # images
	bool CaptureStream(bool enabled, size_t countdown);
   bool SetSubframe(long& startX, long& startY, long&width, long& height);
   bool SetImageBuffer(void* pBGR, long sizeBGR, void* pYUV, long sizeYUV);

	//!@brief query the current type of streaming
	const GraphStreamingType CaptureStatus() const;


	// The app model is either to provide a Wnd Handle that receives WM_CAPTURE
	// events which in turn must trigger the call of OnCapture()
	// If no EventSink is given, the app must poll OnCaptureRequired() and
	// call OnCapture() when it returns true.
	// This flow is needed because the DS Filter must not do any dataprocessing but
	// return asap - so any further capture handling is performed in the context
	// of the application when calling OnCapture()
	// OnCapture returns 0 if a new sample must be collected
	// positive numbers are capture events left until finished
	// negative numbers are error indications

	//!@brief submit a window handle to receive the Post message WM_CAPTURE
	bool put_EventSink(long newVal);
	//!@brief query the used window handle
	const long EventSink() const;

	//!@brief returns if a an OnCapture call is expected
	//!@brief supports non event driven apps i.e. EventSink is not provided
	ESYNMode OnCaptureRequired();

	// must be called to gather the sampled image
	// either via WM_CAPTURE handler or polled via OnCaptureRequired
	//
	//!@brief OnCapture returns 0 if a new sample must be collected
	//!@brief positive numbers are capture events left until finished
	//!@brief negative numbers are error indications
	long OnCapture();

	//!@brief returns the captured image type and memptr
	//!@brief NOTE: the sample is allocated and will be released with the object
	bool CapturedSample(CapSampleType &sample);
	//!@brief just reply to ignore that sample and continue
	void DropCapturedSample();
	//!@brief returns the last classify numbers
	void ClassifyNumbers(long &min, long &max);

	//! *** control the DS capture filters behavior

	//!@brief query the current conversion type
	const EVideoConversion WCFConversion();
	//!@brief set a new conversion type
	void put_WCFConversion(EVideoConversion newVal);
	//!@brief query the current pixel order (for raw only)
	const EVideoPixelOrder PixelOrder();
	//!@brief set the new pixel order (for raw only)
	void put_PixelOrder(EVideoPixelOrder newVal);
   //!@brief gut the current FPS/DPS values  //20100702:BM - added
   bool GetRates(float& fps, float& dps);

	//!@brief query the current capture status of the filter
	bool CaptureEnabled();
	//!@brief set the new capture status of the filter
	void put_CaptureEnabled(bool newVal);

   //!@brief put the new FPS values
   void put_Fps(wxUint32 num, wxUint32 denom);


private:
	// Member Methods
//	long OnTimer(unsigned int uMsg, wxUint16 wParam, long lParam, bool& bHandled);
//	long OnNotify(unsigned int uMsg, wxUint16 wParam, long lParam, bool& bHandled);
//	long OnDevChange(unsigned int uMsg, wxUint16 wParam, long lParam, bool& bHandled);

	void RegisterDeviceNotification();

   void DropFilterThread();
	bool SetupBuilderGraph();       void DropBuilderGraph();
	bool SetupCapDevice();          void DropCapDevice();
	bool SetupSampleGrabber();      void DropSampleGrabber();
	bool SetupCaptureGraph();       void TeardownCaptureGraph();    void DropCaptureGraph();

	bool SetupVideo();               void DropVideo();


	void    TearDownGraph();
	void    KillGraph();
	void    DropDSobjects();

	// Member Variables

 	struct _capstuff {
		_capstuff(){ Init();}
		void Init(){// set all values to 0 or default
			device=-1;pThread=NULL; pWcFilter=NULL; pSGrab=NULL;
			hWndVideo=NULL; ghWnd=0;
			fCaptureGraphBuilt=false; fGraphRunning=false; fGraphRendering=false; fPreviewing=false;
			FrameRate=0; fUseTimeLimit=false; fUseFrameRate=false; dwTimeLimit=0;
			pixOrder=EVO_GB; conversion=EVC_UNKNOWN; captureFlag=false;
		}

		// Builder object
		int             device;

		V4L2Thread      *pThread;
      CV4Lwcfilter    *pWcFilter; // the filter instance
      IV4LwcfCtrl     *pSGrab;    // interface to Sgrabber of the filter

		wxImage         *hWndVideo;  // the video capture window handle
		long            ghWnd;  // controls global window handle (event sink)

		bool            fCaptureGraphBuilt;
		bool            fGraphRunning;      // either running or paused
		bool            fGraphRendering;    // running
		bool            fPreviewing;

		double          FrameRate;
		bool            fUseTimeLimit;
		bool            fUseFrameRate;
		unsigned long   dwTimeLimit;

		EVideoPixelOrder    pixOrder;
		EVideoConversion    conversion;
		bool                captureFlag;
	}m_cap;

	CSampleGrabberCB::CALLBACKINFO m_cb;    // callback communication structure
	CSampleGrabberCB *m_SGrabCallback; // this is the p_instance of the callback class

};



#endif //__UNIX__
#endif //__v4l2stream_H_
