// VCapGraph.cpp : Implementation of CV4Lstream

#include "v4l2stream.h"

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if defined(__UNIX__)

/////////////////////////////////////////////////////////////////////////////

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

// PHILIPS WEBCAM STUFF
#include "pwc-ioctl.h"

#include <asm/types.h>
#include <libv4l1-videodev.h>
//#include <linux/videodev.h>
#include <linux/videodev2.h>

#include "iv4l2wcfCtrl.h"
#include "v4l2wcfilter.h"

#include "../GenericDebHelp.h"


// Helper functions
static int xioctl(int device, int request, void* arg)
{
   // repeat the call to ioctl as long as the previous call was interrupted prematurely
   int r = 0;
   int tryx=10;
   do {
     r = ::ioctl(device,request,arg); tryx--;
   } while ( -1 == r && EINTR == errno && tryx );

   return r;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief sleep command; take milliseconds as argument
//static
static void Sleep_ms(unsigned short  mseconds )
{
   // Windows Sleep uses miliseconds
   // linux usleep uses microsecond
   // the argument is   coming in millisecond.
   long useconds = long(mseconds) * 1000;
   // usleep can only handle <1000000 usec
   if (useconds>=1000000) {
      long seconds = useconds / 1000000;
      sleep(seconds);
      useconds %=1000000;
   }
   if (useconds>20) usleep(useconds);
}

#define CLEAR(x) memset(&(x), 0, sizeof(x))

#ifdef __UNIX__
    #define EnterCScb pOwner->m_cb.CrSection.Enter()
    #define LeaveCScb pOwner->m_cb.CrSection.Leave()
    #define EnterCS m_cb.CrSection.Enter()
    #define LeaveCS m_cb.CrSection.Leave()
#else // Win
    #define EnterCScb EnterCriticalSection(&pOwner->m_cb.CrSection)
    #define LeaveCScb LeaveCriticalSection(&pOwner->m_cb.CrSection)
    #define EnterCS EnterCriticalSection(&m_cb.CrSection)
    #define LeaveCS LeaveCriticalSection(&m_cb.CrSection)
#endif

#ifndef BI_RGB
   // defined biCompression of BITMAPINFOHEADER
   #define BI_RGB  0
#endif



/// ///////////////////////////////////////////////////////////////////
/// Trivial Thread Wrapper
/// ///////////////////////////////////////////////////////////////////

#include <pthread.h>
struct V4L2Thread
{
private:
   CapWorker&     m_capWorker; // generic worker object
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
   V4L2Thread(CapWorker& p_work);
   ~V4L2Thread();

   //!  Task entry
   int Run();
   //! pthread Join exposed
   int Join();
};

//! @brief  Implements a c style function to be sent as callback ptr to pthread_create
///   we have to submit this in pthread_create so it is sent as obj (context) here
///   so this is generic up to the point it casts to V4L2Thread somehow
extern "C" void* V4L2thread_routine(void* obj) throw() {
   try {
      static_cast<V4L2Thread*>(obj)->go(); // cast and call member from context given
      return obj; //or whatever else you might need
   }
   catch(...) {
      // return error code or just exit
      return NULL;
   }
}//externC


//! @brief  Start function wrapped in thread_routine()
void V4L2Thread::go() { m_capWorker(); } // call operand () of our thread argument

// constructor
V4L2Thread::V4L2Thread(CapWorker& p_work)
: m_capWorker(p_work)
{
   routine.address = V4L2thread_routine; // assign callback address
   pthread_attr_init(&attr);
}

// destructor
V4L2Thread::~V4L2Thread() {
   pthread_attr_destroy(&attr);
}

//! @brief  Thread entry (pthread_create exposed)
int V4L2Thread::Run() {
   return pthread_create (&id, &attr, routine.address, this);
   // this one callsback above and therefore calls our go()
   // which in turn runs the real instance of AtikWorker submitted
   // while creating this AtikThread object
}

//! @brief  Thread Join (pthread_join exposed)
int V4L2Thread::Join() {
   return pthread_join(id, NULL);
}

/////////////////////////////////////////////////////////////////////////////
// CV4Lstream


/*
 CaptureState sequence goes:

 CSampleGrabberCB::BUF_IDLE:
	idle
	caller.CaptureStream ON       -> CSampleGrabberCB::BUF_READY_FORSAMPLE (start loop)

 CSampleGrabberCB::BUF_READY_FORSAMPLE:
	Capture
	Callback                      -> CSampleGrabberCB::BUF_SAMPLE_ARRIVED

 CSampleGrabberCB::BUF_SAMPLE_ARRIVED:
	OnCapture                     -> CSampleGrabberCB::BUF_SAMPLE_PREPARED

 CSampleGrabberCB::BUF_SAMPLE_PREPARED:
	Fire_Capture                  -> back to caller
		caller.get_CapturedImage   -> CSampleGrabberCB::BUF_READY_FORSAMPLE (loop)
		caller.CaptureStream OFF   -> CSampleGrabberCB::BUF_IDLE (end loop)
*/


/////////////////////////////////////////////////////////////////////////////
// Callback copied from StillCap Sample and modified heavily
CSampleGrabberCB::CSampleGrabberCB( )
: pOwner(NULL)
{
}

bool  CSampleGrabberCB::IsSampleCBXready()
{
   // we don't need sync with reading a status variable only

   return  (pOwner->m_cb.bufferStatus == CSampleGrabberCB::BUF_READY_FORSAMPLE);
}

// The sample grabber is calling us back on its deliver thread.
// This is potentially NOT the main app thread!
// As a workaround, copy the bitmap data during the callback,
// post a message to our app, and write the data later.
//
bool  CSampleGrabberCB::SampleCBX(wxUint8 *pSampleNative, size_t lengthNative,
                                  wxUint8 *pSampleBgr, size_t lengthBgr,
                                  EVideoConversion conversion)
{
wxUint8 *bPtrNative = pSampleNative;  // buffer pointer
wxUint8 *bPtrBgr    = pSampleBgr;  // buffer pointer
bool     post = false;

//NOTE - if capturing is off this one is called with NULL ptrs for the image buffers
//       just ticking the stream pace here

   EnterCScb;

   pOwner->m_cb.capTick=true;

	// we maintain a buffer status to synchronize access
	// if access is not allowed just bail out
	if (pOwner->m_cb.bufferStatus != CSampleGrabberCB::BUF_READY_FORSAMPLE) {
      LeaveCScb;
		return true;
	}

	// reset to avoid trouble if it fails
	pOwner->m_cb.validBufferSize = 0;

   // if it is only an empty tick - bail out here with capture true
   if (pOwner->m_cb.countdown) {
       pOwner->m_cb.onCaptureFlag=true;       // Set flag if counting down
       if (pOwner->m_cb.countdown<=1) {
          pOwner->put_CaptureEnabled(true);  // enable capture in wcfilter for the last run
       }
      pOwner->m_cb.bufferStatus = CSampleGrabberCB::BUF_SAMPLE_ARRIVED; // change status
      LeaveCScb;
      return true;
   }

	// it seems we have a valid sample

	wxUint8*pSrc = NULL;
	wxUint8*pDst = pOwner->m_cb.pImageBgr;
	long w = pOwner->m_cb.bmih.biWidth;     // src image width
	long h = pOwner->m_cb.bmih.biHeight;    // src image height

	long sx = pOwner->m_cb.startX;     // dst coords
	long sy = pOwner->m_cb.startY;     // dst coords
	long sw = pOwner->m_cb.width;      // dst coords
	long sh = pOwner->m_cb.height;     // dst coords
	long scan_sw = (sw*3+3) & ~3;      // dst coords scan width in bytes WORD aligned
	long iSize =  scan_sw*sh;        // BMP has 3 bytes per pixel - i.e. use scanwidth
	// some sanity checks here
	if ( (pOwner->m_cb.pImageBgr == NULL) || (iSize > pOwner->m_cb.sizeBGR) ) {
		// if the buffer to copy is larger than our local one, just bail out
		LeaveCScb;
		::fprintf(stderr, "CSampleGrabberCB::SampleCBX() - error exit - iSize %ld - sizeBGR %ld \n", iSize, pOwner->m_cb.sizeBGR);
		return false;
	}


   // we return an RGB image for wxAstroCapture
   // BGRPlane w*h*3 pixels
   for (long y=sy; y<(sy+sh); y++){
      pSrc = bPtrBgr +(y*w + sx)*3; // still assuming our src does not need scanline alignment
      ::memcpy(pDst, pSrc, sw*3); pDst+=scan_sw; // but the dst might use it
   }
	pOwner->m_cb.validBufferSize = iSize;

	// copy the native bitmap only now that we know about it
	pOwner->m_cb.nativeRGB = (conversion==EVC_RGB24_RGB24);
	switch (conversion) {
      case EVC_RGB24_RGB24: {
		// Native RGB provides no YUV plane at all
		// so leave it alone
		break;
      }
      case EVC_I420_RGB24: {
         // must handle subframes - so get ptrs into the full frame
         // get target size BMP
         long iYSize =  sw * sh;
         long iUVSize = (sw>>1) * (sh>>1);
         iSize =  iYSize + 2*iUVSize;
         // some sanity checks here
         if ( (pOwner->m_cb.pImageYuv == NULL) || (iSize > pOwner->m_cb.sizeYUV) ) {
            // if the buffer to copy is larger than our local one, just bail out
            LeaveCScb;
            ::fprintf(stderr, "CSampleGrabberCB::SampleCBX() - error exit - iSize %ld - sizeYUV %ld \n", iSize, pOwner->m_cb.sizeYUV);
            return false;
         }

         const wxUint8*pU = bPtrNative + w*h;  // Y is one plane
         const wxUint8*pV = pU+ (w*h / 4);  // U is w/2*h/2
         // copy the image content
         const wxUint8*pSrc=NULL;
         wxUint8*pDst=pOwner->m_cb.pImageYuv;

         // YPlane w*h pixels
         for (long y=0; y<sh; y++){
            pSrc = bPtrNative + (sy+y)*w + sx; // line/col start in original Y plane
            ::memcpy(pDst, pSrc, sw); pDst+=sw;
         }
         w/=2; h/=2;  // src image U and V is only half width, half height
         // UPlane w/2*h/2 pixels
         for (long y=0; y<(sh/2); y++){
            pSrc = pU + (sy/2+y)*w + sx/2; // line start/col start in original U plane
            ::memcpy(pDst, pSrc, sw/2); pDst+=sw/2;
         }
         // VPlane w/2*h/2 pixels
         for (long y=0; y<(sh/2); y++){
            pSrc = pV + (sy/2+y)*w + sx/2; // line start/col start in original V plane
            ::memcpy(pDst, pSrc, sw/2); pDst+=sw/2;
         }
         break;
      }// copy YUV 4:2:0

      case EVC_YUY2_RGB24:
      case EVC_YVYU_RGB24:
      case EVC_UYVY_RGB24: {
         // must handle subframes - so get ptrs into the full frame
         // get target size BMP
         long iYSize =  sw * sh;
         iSize =  iYSize * 2;    //16bits/pix
         // some sanity checks here
         if ( (pOwner->m_cb.pImageYuv == NULL) || (iSize > pOwner->m_cb.sizeYUV) ) {
            // if the buffer to copy is larger than our local one, just bail out
            LeaveCScb;
            ::fprintf(stderr, "CSampleGrabberCB::SampleCBX() - error exit - iSize %ld - sizeYUV %ld \n", iSize, pOwner->m_cb.sizeYUV);
            return false;
         }

         // copy the image content
         const wxUint8*pSrc=NULL;
         wxUint8*pDst=pOwner->m_cb.pImageYuv;

         // YPlane w*h pixels
         // the YUV 4:2:2 format is interleaved 4 bytes form 2 pixels
         for (long y=0; y<sh; y++){
            pSrc = bPtrNative + (sy+y)*w*2 + sx*2; // line/col start in original Y plane
            ::memcpy(pDst, pSrc, sw*2); pDst+=sw*2;
         }
         break;
      }// copy YUV 4:2:2
      default: ;
	}//switch conversion

	pOwner->m_cb.bufferStatus = CSampleGrabberCB::BUF_SAMPLE_ARRIVED; // change status
   pOwner->m_cb.onCaptureFlag=true;       // Set flag
   post = (pOwner->m_cap.ghWnd!=0);       // true if we have a win handle to post

   LeaveCScb;

   return true;

}//SampleCB

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// PUBLIC METHODS

/////////////////////////////////////////////////////////////////////////////
// Returns the HWND handle
//
const long  CV4Lstream::EventSink() const
{
   return (long)m_cap.ghWnd;
}//get_EventSink


/////////////////////////////////////////////////////////////////////////////
// Set the HWND handle and subclass the window to receive USB (dis)connections
//
// returns:
// true
// E_HANDLE		if the handle does not belong to a valid window
//
bool  CV4Lstream::put_EventSink(long newVal)
{
	if (m_cap.ghWnd)
	{// unsubclass if we already had a handle
		m_cap.ghWnd = 0;
	}

	if (newVal==0)
	{
		return true;  // don't resubclass with handle 0 !!
	}

   return false;
}//put_EventSink


//---------------------------------------------------------------------------
//

// control the capture filters behavior

const EVideoPixelOrder CV4Lstream::PixelOrder()
{
   if (m_cap.pSGrab) {
      m_cap.pSGrab->get_V4LwcfPixOrder( m_cap.pixOrder );
   }
   return m_cap.pixOrder;
}//PixelOrder

void CV4Lstream::put_PixelOrder(EVideoPixelOrder newVal)
{
   if (m_cap.pSGrab) {
      m_cap.pixOrder = newVal;
      m_cap.pSGrab->put_V4LwcfPixOrder( m_cap.pixOrder );
   }
}//put_PixelOrder


const EVideoConversion CV4Lstream::WCFConversion()
{
   if (m_cap.pSGrab) {
      m_cap.pSGrab->get_V4LwcfConversion( m_cap.conversion );
   }
   return m_cap.conversion;
}//get_InputFormat

void CV4Lstream::put_WCFConversion(EVideoConversion newVal)
{
   if (m_cap.pSGrab) {
      m_cap.conversion = newVal;
      m_cap.pSGrab->put_V4LwcfConversion( m_cap.conversion );
}
}//InputFormat


// query the current capture status of the filter
bool CV4Lstream::CaptureEnabled()
{
   if (m_cap.pSGrab) {
      m_cap.pSGrab->get_V4LwcfCapture( m_cap.captureFlag );
   }
   return m_cap.captureFlag;
}

// set the new capture status of the filter
void CV4Lstream::put_CaptureEnabled(bool newVal)
{
 	if (m_cap.pSGrab) {
		m_cap.captureFlag = newVal;
		m_cap.pSGrab->put_V4LwcfCapture( m_cap.captureFlag );
	}
}

//!@brief put the new FPS values
void CV4Lstream::put_Fps(wxUint32 num, wxUint32 denom)
{
 	if (m_cap.pSGrab) {
		m_cap.pSGrab->put_V4LwcfFps(num, denom);
	}
}

//!@brief get the current FPS/DPS values  //20100702:BM - added
bool CV4Lstream::GetRates(float& fps, float& dps)
{
 	if (m_cap.pSGrab) {
		return m_cap.pSGrab->GetRates(fps, dps);
	}
	else return false;
}


//---------------------------------------------------------------------------
//
// Init Streaming Capture
//   gathering the n'th image from the capture filter
//
// returns:
// true
// false    if no HWND of the event handler was given
// false    if direct capturing via buffer is running
// false    if sample is not available
//
bool  CV4Lstream::CaptureStream(bool enabled, size_t countdown)
{
bool hr = true;

	if (!enabled) {
      // capture stream or collect OFF
      EnterCS;
         put_CaptureEnabled(false); // do not longer capture in wcFilter
         m_cb.captureFlag = false;
         m_cb.bufferStatus = CSampleGrabberCB::BUF_IDLE;
         m_cb.countdown = 0;
         m_cb.validBufferSize = 0;
      LeaveCS;
	}
	else {
	   // ON
      EnterCS;
         // if we wait for an image we switch off capture
         // it is enabled by the streamer itself when the countdown gets 0
         put_CaptureEnabled( bool(countdown==0) ); // do capture in wcFilter only while continous streaming
         m_cb.captureFlag = true;
         m_cb.bufferStatus = CSampleGrabberCB::BUF_READY_FORSAMPLE;
         m_cb.countdown = countdown; // set
         m_cb.validBufferSize = 0;
         // Adjust the sample Bitmap
         m_cb.bmih.biBitCount = 24;
         m_cb.bmih.biCompression = 0;
         m_cb.bmih.biSizeImage = m_cb.bmih.biWidth* m_cb.bmih.biHeight*m_cb.bmih.biBitCount/8;
      LeaveCS;
	}

	return hr;
}//CaptureStream

bool CV4Lstream::SetSubframe(long& startX, long& startY, long&width, long& height)
{
	EnterCS;
		if (m_cb.captureFlag) return false; // ERROR EXIT - not while capture in progress

		m_cb.startX = ((startX + 3) & ~3); // DWORD align left for BMPs
		m_cb.startY = ((startY + 3) & ~3); // DWORD align top for BMPs
		m_cb.width  = ((width + 7) & ~7);  // QWORD align width for BMPs (the YUV UV images are div 2)
		m_cb.height = ((height + 3) & ~3); // DWORD align height for BMPs
		// return new dims
		startX = m_cb.startX; startY = m_cb.startY;
		width  = m_cb.width;  height = m_cb.height;
	LeaveCS;
	return true;
}

bool CV4Lstream::SetImageBuffer(void* pBGR, long sizeBGR, void* pYUV, long sizeYUV)
{
	EnterCS;
		if (m_cb.captureFlag) return false; // ERROR EXIT - not while capture in progress

		m_cb.pImageBgr = reinterpret_cast<wxUint8*>(pBGR);
		m_cb.sizeBGR = sizeBGR;
		m_cb.pImageYuv = reinterpret_cast<wxUint8*>(pYUV);
		m_cb.sizeYUV = sizeYUV;
	LeaveCS;
	return true;
}

//---------------------------------------------------------------------------
//
// Return the current capture status
//
// returns:
// true
//
const CV4Lstream::GraphStreamingType  CV4Lstream::CaptureStatus() const
{
	GraphStreamingType pVal = GST_Off;
	if (m_cb.captureFlag)
	{
		pVal = GST_Stream;
	}

	return pVal;
}//GraphStreamingType



//---------------------------------------------------------------------------
//
// just ignores the pending capture an resets the state machine
void CV4Lstream::DropCapturedSample()
{
    EnterCS;
	{
		m_cb.bufferStatus = CSampleGrabberCB::BUF_IDLE; // back to idle
		m_cb.captureFlag = false; // not longer
	}
    LeaveCS;
}//DropCapturedSample


//---------------------------------------------------------------------------
//
// returns the captured image type and memptr
// NOTE: the sample shall be copied for further usage
//
// returns:
// true
// false    if sample is not available
// false    if not in streaming mode
//
bool CV4Lstream::CapturedSample(CapSampleType &sample)
{
bool hr = true;

   EnterCS;
	{
		if (m_cb.bufferStatus!=CSampleGrabberCB::BUF_SAMPLE_PREPARED)
		{
			hr = false;
		}
		else
		{
		   sample.nativeIsRGB = m_cb.nativeRGB;

			sample.startX = m_cb.startX;
			sample.startY = m_cb.startY;
			sample.width = m_cb.width;
			sample.height = m_cb.height;
			sample.fullFrame = ( (m_cb.width==m_cb.bmih.biWidth) && (m_cb.height==m_cb.bmih.biHeight) );

			m_cb.bufferStatus = CSampleGrabberCB::BUF_IDLE; // back to idle
			m_cb.captureFlag = false;  // not longer
         put_CaptureEnabled(false); // do not longer capture in wcFilter

		}
	}
   LeaveCS;

	return hr;
}//CapturedSample


/////////////////////////////////////////////////////////////////////////////
//
// Returns the captured frame dimension
// (Video Source)
//
// returns:
// true
void  CV4Lstream::FrameDimension(wxUint16 &width, wxUint16 &height) const
{
	width = m_cb.bmih.biWidth;
	height = m_cb.bmih.biHeight;
}//FrameDimension


//---------------------------------------------------------------------------
//
// Set the Format of the Capture OutPin (if supported) according to a binning level
// i.e. level 1 = 640x480; 2 = 320x240; 4 = 160x120; 5 = 352x288 cropped
//  must dis- and reconncet the graph because most format changes
//  require new filters with changed properties to be inserted by the graph manager
//  will maintain the current state of the graph
//
// returns:
// true
// false    if capturing
// false    if the binnig parameter is out of range
// false    if reconnect failed
//
bool  CV4Lstream::SetFormat(wxUint16 binning)
{

   if ( binning>1 ) return false; // ERROR EXIT
   return true;

/*
THIS IS NOT LONGER SUPPORTED...
bool hr = true;
bool fs;

   if (m_cb.captureFlag)
      return   false; // not while capturing !!!


    // Prepare the sample Bitmap
//   m_cb.bmih.biWidth=640;
//   m_cb.bmih.biHeight=480;

	// ** here we go on - bail out through LoadFormat_Final to reconnect the graph

	// must stop the graph and reconnect later in case anything has changed
	fs=Status();
	if ( fs )
		Stop();


	// if we have a new format the graph must be completely rebuilt!!
	// all filters have to agree the new format
	TeardownCaptureGraph();

   // *** finally set the desired format of the capture
   {
      v4l2_format fmt;
      // cropping ignored here
      CLEAR(fmt);
      fmt.type                      = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      fmt.fmt.pix.width             = m_cb.bmih.biWidth;
      fmt.fmt.pix.height            = m_cb.bmih.biHeight;
      fmt.fmt.pix.pixelformat       = V4L2_PIX_FMT_YUV420;
      fmt.fmt.pix.field             = V4L2_FIELD_ANY;
   }

	// re-connect filters through any conversion needed
	hr = SetupCaptureGraph();

	// maintain previous state of the graph
	if (hr && fs)
	{
		return Run();
	}
	return hr;
*/

}//SetFormat


//---------------------------------------------------------------------------
//
// setup the video window and the video output
// according to the client window dimensions
void CV4Lstream::ScaleVideoOutput()
{
/* TODO
RECT rcDest;

	if (m_cap.hWndVideo && m_cap.pVideo && m_cap.pVWindow )
	{
		::GetClientRect (m_cap.hWndVideo, &rcDest);
		// make the video window matching the target control window
		m_cap.pVWindow->SetWindowPosition(rcDest.left,rcDest.top, rcDest.right, rcDest.bottom);

		m_cap.pVideo->SetDefaultSourcePosition();
		// scale it to the dest aspect ratio
		m_cap.pVideo->SetDestinationPosition(rcDest.left,rcDest.top, rcDest.right, rcDest.bottom);
	}
*/
}//ScaleVideoOutput

void CV4Lstream::VideoOutputVisible(bool enable)
{
}

//---------------------------------------------------------------------------
//
// Return the status of our graph
//
// returns:
// true
// false    if no Media Control object is available
// bool from GetState
//
long  CV4Lstream::Status() const
{
	return (long) m_cap.fPreviewing;
}//Status



//---------------------------------------------------------------------------
//
// Run our graph
//
// returns:
// true
// false    if no Media Control object is available
// bool from Run
//
bool  CV4Lstream::Run()
{
   if ( !m_cap.fCaptureGraphBuilt ) return false;  // no interface

   // 1st check the current state
   if ( m_cap.fPreviewing ) {
      // seems OK - return so
      m_cap.fGraphRunning = true;
      m_cap.fGraphRendering = true;
      // already ready to sample
      return true;
   }

   // make it run
   if (m_cap.pSGrab) {
      m_cap.pSGrab->StreamON();
      if ( ! m_cap.pSGrab->IsStreaming() ) {
         m_cap.fGraphRunning = false;
         m_cap.fGraphRendering = false;
         ::fprintf(stderr, "CV4Lstream::Run() - error exit - StreamOn() failed\n");
         return false;
      }
   }
   else {
      m_cap.fGraphRunning = false;
      m_cap.fGraphRendering = false;
      ::fprintf(stderr, "CV4Lstream::Run() - sw error - m_cap.pSGrab is NULL\n");
      return false;
   }


   m_cap.fPreviewing = true;
   m_cap.fGraphRunning = true;
   m_cap.fGraphRendering = true;
   m_cb.bufferStatus = (m_cb.captureFlag)?CSampleGrabberCB::BUF_READY_FORSAMPLE:CSampleGrabberCB::BUF_IDLE;
   return true;
}//Run


//---------------------------------------------------------------------------
//
// Pause our graph
//
// returns:
// true
// E_NOINTERFACE  if no Media Control object is available
// bool from Pause
//
bool  CV4Lstream::Pause()
{

    return Stop();
}//Pause


//---------------------------------------------------------------------------
//
// Stop our graph
//
// returns:
// true
// E_NOINTERFACE		if no Media Control object is available
// bool from Stop
//
bool  CV4Lstream::Stop()
{
   _V_ ::fprintf(stderr, "CV4Lstream::Stop() - entry \n");

	if ( !m_cap.fCaptureGraphBuilt ) return false;	// no interface


	m_cb.bufferStatus = CSampleGrabberCB::BUF_IDLE; // not longer sampling

	// 1st check the current state
	if ( m_cap.fPreviewing )
    {
        _V_ ::fprintf(stderr, "CV4Lstream::Stop() - disable capture \n");
        this->put_CaptureEnabled(false);
        _V_ ::fprintf(stderr, "CV4Lstream::Stop() - calling StreamOFF \n");
        // make it stop
         if (m_cap.pSGrab) {
            m_cap.pSGrab->StreamOFF();
            ::Sleep_ms(500); // wait some time to allow the thread to catch streamoff
            if ( m_cap.pSGrab->IsStreaming() ) {
               m_cap.fGraphRunning = false;
               m_cap.fGraphRendering = false;
               ::fprintf(stderr, "CV4Lstream::Run() - error exit - StreamOFF() failed\n");
               return false;
            }
           else
           {
               m_cap.fGraphRunning = false;
               m_cap.fGraphRendering = false;
               return true;
           }
         }
         else {
            m_cap.fGraphRunning = false;
            m_cap.fGraphRendering = false;
            ::fprintf(stderr, "CV4Lstream::Run() - sw error - m_cap.pSGrab is NULL\n");
            return false;
         }
    }
    else
    {
        m_cap.fGraphRunning = false;
        m_cap.fGraphRendering = false;
        return true;
    }

}//Stop


////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// PRIVATE METHODS


////////////////////////////////////////////////////////////////
// Build Graph Methods

////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
// External Entry Point to create a capture graph
//
// receives a capture filter (IVCapFilter) and create a capture graph from it
// NOTE: this is a one shoot interface, it will fail if it is called a second time
//       without killing all
// returns:
// true
// false    if the given filter was not to be used
// false    if something else went wrong
// false    if called more than once
bool  CV4Lstream::CreateNew(const CV4LcapFilterItem& capEntry)
{
   _V_ ::fprintf(stderr, "CV4Lstream::CreateNew() - entry \n");

    // we dont have a valid device from constructor
   if(m_cap.device==-1)
	{
      _V_ ::fprintf(stderr, "CV4Lstream::CreateNew() - error exit - no device (-1) \n");
      return false;
	}

   DropFilterThread(); // remove the filter thread if it existed before

	// create the 'Grabber Filter' performs the stream to image conversion
	// supply the image plane to write to and an eventhandler to post when an image is updated
   _V_ ::fprintf(stderr, "CV4Lstream::CreateNew() - create stream objects \n");
   m_cap.pWcFilter = new CV4Lwcfilter(this, m_cap.device, capEntry);
   m_cap.pSGrab = dynamic_cast<IV4LwcfCtrl*>(m_cap.pWcFilter); // this is just the sample grabber interface

   _V_ ::fprintf(stderr, "CV4Lstream::CreateNew() - create streaming thread \n");
   m_cap.pThread = new V4L2Thread(*m_cap.pWcFilter);

   // Prepare the sample Bitmap
   _V_ ::fprintf(stderr, "CV4Lstream::CreateNew() - prepare receiving bitmap \n");
   m_cb.bmih.biWidth  = capEntry.FrameSize_Width();
   m_cb.bmih.biHeight = capEntry.FrameSize_Height();
   m_cb.bmih.biBitCount=24;
   m_cb.bmih.biCompression=0;

   // start the builder chain

   if( !SetupBuilderGraph() ) {
      ::fprintf(stderr, "CV4Lstream::CreateNew() - error exit - SetupBuilderGraph failed \n");
      DropFilterThread(); // remove the filter thread if it existed before
      return false;
   }
   if ( !SetupCapDevice() ) {
      ::fprintf(stderr, "CV4Lstream::CreateNew() - error exit - SetupCapDevice failed \n");
      DropFilterThread(); // remove the filter thread if it existed before
      return false;
   }
   if ( !SetupSampleGrabber() ) {
      ::fprintf(stderr, "CV4Lstream::CreateNew() - error exit - SetupSampleGrabber failed \n");
      DropFilterThread(); // remove the filter thread if it existed before
      DropSampleGrabber();
      return false;
   }
   if ( !SetupVideo() ) {
      ::fprintf(stderr, "CV4Lstream::CreateNew() - error exit - SetupVideo failed \n");
      DropFilterThread(); // remove the filter thread if it existed before
      DropVideo(); DropSampleGrabber();
      return false;
   }
   if ( !SetupCaptureGraph() ) {
      ::fprintf(stderr, "CV4Lstream::CreateNew() - error exit - SetupCaptureGraph failed \n");
      DropFilterThread(); // remove the filter thread if it existed before
      DropCaptureGraph();
      DropVideo(); DropSampleGrabber();
      return false;
   }

   _V_ ::fprintf(stderr, "CV4Lstream::CreateNew() - starting driver thread \n");
   m_cap.pThread->Run(); // run the thread

   ::fprintf(stderr, "CV4Lstream::CreateNew() - driver thread started\n");

   _V_ ::fprintf(stderr, "CV4Lstream::CreateNew() - exit \n");
   return true;
}//CreateNew


void CV4Lstream::DropFilterThread()
{
   _V_ ::fprintf(stderr, "CV4Lstream::DropFilterThread() - entry\n");

   if (m_cap.pWcFilter) {
      _V_ ::fprintf(stderr, "CV4Lstream::DropFilterThread() - killing driver thread now\n");
      DropSampleGrabber();
      m_cap.pWcFilter->AbortThread();
      // first wait for the thread proc to terminate
      assert(m_cap.pWcFilter);
      if (m_cap.pThread) {
         _V_ ::fprintf(stderr, "CV4Lstream::DropFilterThread() - wait for driver thread end\n");
         m_cap.pThread->Join();  // waits until it is gone
         delete m_cap.pThread; m_cap.pThread = NULL;
         _V_ ::fprintf(stderr, "CV4Lstream::DropFilterThread() - driver thread has ended, reclaiming resources\n");
      }

      // kill worker obj
      if (m_cap.pWcFilter) delete m_cap.pWcFilter;
      m_cap.pWcFilter = NULL; m_cap.pSGrab=NULL;
   }
   _V_ ::fprintf(stderr, "CV4Lstream::DropFilterThread() - exit\n");
}


////////////////////////////////////////////////////////////////
// Create a Filtergraph and a Graphbuilder2 object including a MediaControl IF
// NOTE: you must call Drop<method> if the return value is not true
bool CV4Lstream::SetupBuilderGraph()
{
   v4l2_capability cap;
   if(-1 == xioctl(m_cap.device,VIDIOC_QUERYCAP,&cap)) {
      ::fprintf(stderr, "CV4Lstream::SetupBuilderGraph() - error exit - VIDIOC_QUERYCAP failed - errno: %d\n", errno);
      return false;
   }

//   m_name = wxString::FromAscii((char*)cap.card);

   if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
      ::fprintf(stderr, "CV4Lstream::SetupBuilderGraph() - error exit - Not a video device\n");
      return false;
   }

   return true;
}//SetupBuilderGraph

// Drop all objects from Setup
void CV4Lstream::DropBuilderGraph()
{
}//DropBuilderGraph


////////////////////////////////////////////////////////////////
// create the capture filters of the graph.  We need to keep them loaded from
// the beginning, so we can set parameters on them and have them remembered
// NOTE: you must call Drop<method> if the return value is not true
bool CV4Lstream::SetupCapDevice()
{

   v4l2_format fmt;

   // Get the current format
   CLEAR(fmt);
   fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // must define what we want to receive
   if(-1 == xioctl(m_cap.device,VIDIOC_G_FMT,&fmt)) {
      ::fprintf(stderr, "CV4Lstream::SetupCapDevice() - error exit - VIDIOC_G_FMT failed - errno: %d\n", errno);
      return false;
   }
   m_cb.bmih.biWidth  = fmt.fmt.pix.width;
   m_cb.bmih.biHeight = fmt.fmt.pix.height;

   return true;
}//SetupCapDevice

// Drop all objects from Setup
void CV4Lstream::DropCapDevice()
{
}//DropCapDevice


////////////////////////////////////////////////////////////////
// Get the SampleGrabber and add it to the graph
// the SampleGrabber is set to accept only YUV data on its input
// so captures images should always have this bitmap compatible format
// NOTE: you must call Drop<method> if the return value is not true
bool CV4Lstream::SetupSampleGrabber()
{
    if (m_cap.pSGrab) {
        m_cap.pSGrab->SetCallback(m_SGrabCallback);
        return true;
    }
    ::fprintf(stderr, "CV4Lstream::SetupSampleGrabber() - error exit - cannot set callback \n");
    return false;
}// SetupSampleGrabber

// Drop all objects from Setup
void CV4Lstream::DropSampleGrabber()
{
    if (m_cap.pSGrab) m_cap.pSGrab->SetCallback(NULL);
}//DropSampleGrabber


////////////////////////////////////////////////////////////////
// get the video interfaces and connect things to make it a child
// window of the control we got and also that is will receive the
// events generated by the filter graph
bool CV4Lstream::SetupVideo()
{
	if (m_cap.hWndVideo) // if we have a control window
	{
	}


	return true;
}//SetupVideo

// Drop all objects from Setup
void CV4Lstream::DropVideo()
{
}//DropVideo



////////////////////////////////////////////////////////////////
// build the capture graph
// can be called whenever the graph is not longer built (flag fCaptureGraphBuilt)
// cannot be called while we have a running graph (bug false)
// failes if no capture device is available (bug false)
// NOTE: you must call Drop<method> if the return value is not true
bool CV4Lstream::SetupCaptureGraph()
{
   if (m_cap.pSGrab) m_cap.pSGrab->InitCapFilter();

   // recalc image size here
   m_cb.bmih.biSizeImage = (m_cb.bmih.biBitCount
                              * m_cb.bmih.biWidth * m_cb.bmih.biHeight) / 8;

   // All done.
   m_cap.fCaptureGraphBuilt = true;

   return true;

}//SetupCaptureGraph

// remove filter connections of the graph without killing the
// filters needed - use SetupCaptureGraph to rebuild it
void CV4Lstream::TeardownCaptureGraph()
{
   if (m_cap.pSGrab) m_cap.pSGrab->StreamOFF();
   m_cap.fPreviewing = false;
   m_cap.fCaptureGraphBuilt = false;
}//TeardownCaptureGraph


// Drop all objects from Setup
void CV4Lstream::DropCaptureGraph()
{
   if (m_cap.device!=-1) {
      if (m_cap.pSGrab) m_cap.pSGrab->StreamOFF();
   }
   m_cap.fPreviewing = false;
}//DropCaptureGraph


////////////////////////////////////////////////////////////////
// Remove Graph Methods


////////////////////////////////////////////////////////////////
// Drop all DS object references
void CV4Lstream::DropDSobjects()
{
   _V_ ::fprintf(stderr, "CV4Lstream::DropDSobjects() - entry \n");
	// should call Release through CComPtr class
	DropVideo();
	DropCaptureGraph();
	DropSampleGrabber();
	DropCapDevice();
	DropBuilderGraph();

   _V_ ::fprintf(stderr, "CV4Lstream::DropDSobjects() - exit \n");
}//END DropDSobjects



////////////////////////////////////////////////////////////////
// Tear down everything downstream of the capture filters, so we can build
// a different capture graph.  Notice that we never destroy the capture filters
// and WDM filters upstream of them, because then all the capture settings
// we've set would be lost.
//
void CV4Lstream::TearDownGraph()
{
   _V_ ::fprintf(stderr, "CV4Lstream::TearDownGraph() - entry \n");

	// kill references along the way
   DropFilterThread(); // kill filter and stream

	DropVideo();
	DropCaptureGraph();
	DropSampleGrabber();

	// maintain status about the graph
    m_cap.fCaptureGraphBuilt = false;

   _V_ ::fprintf(stderr, "CV4Lstream::TearDownGraph() - exit \n");
}//TearDownGraph



////////////////////////////////////////////////////////////////
// called by Graph destructor - clean up any existing DS objects
//
void CV4Lstream::KillGraph()
{
   _V_ ::fprintf(stderr, "CV4Lstream::KillGraph() - entry \n");

	if (m_cap.device != -1) // having a capture filter
	{
		TearDownGraph();
		DropDSobjects();
	}

   _V_ ::fprintf(stderr, "CV4Lstream::KillGraph() - exit \n");
}//KillGraph


//////////////////////////////////////////////////////////////////////
// Callbacks
//////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// returns true if we expect an OnCapture call
// supporting non event driven apps
ESYNMode CV4Lstream::OnCaptureRequired()
{
ESYNMode retVal;

    EnterCS;
		retVal = (m_cb.capTick)?ESYN_Tick:ESYN_None;
		retVal = (m_cb.onCaptureFlag)?( (retVal)?ESYN_All:ESYN_Capture ):retVal;
		m_cb.capTick=false; // one shoot flag
	LeaveCS;

	return retVal;
}//OnCaptureRequired


/////////////////////////////////////////////////////////////////////////////
// called via Post Message when the SampleGrabber got a new Sample
// or polled when OnCaptureRequired() returns true
//
// returns a positive number indicating the number of events left until
//  a capture is ready
// returns a 0 (zero) if a captured sample is to collect
// returns a -1 if an unexpected state is encountered but processing goes on
// returns a negative number below -1 on error
long CV4Lstream::OnCapture()
{
long d;

   EnterCS;
	m_cb.onCaptureFlag=false;   // reset flag

   // someone stopped the streamer
   if ( !m_cb.captureFlag )
   {
      LeaveCS;
      _V_ ::fprintf(stderr, "CV4Lstream::OnCapture() - exit - capturing stopped \n");
      return -1;
   }

   // check the status - must be CSampleGrabberCB::BUF_SAMPLE_ARRIVED
   if (m_cb.bufferStatus!=CSampleGrabberCB::BUF_SAMPLE_ARRIVED) {
      m_cb.bufferStatus = CSampleGrabberCB::BUF_READY_FORSAMPLE; // ready for next capture
      LeaveCS;
      _V_ ::fprintf(stderr, "CV4Lstream::OnCapture() - exit - old sample not reclaimed so far \n");
      return -1;
   }


   if ( m_cb.captureFlag ) {
      // this is the mode where one waits a number of samples to get one
      if (m_cb.countdown>0) {
         // just decrement, and retrigger capture
         d = m_cb.countdown;
         m_cb.bufferStatus = CSampleGrabberCB::BUF_READY_FORSAMPLE; // ready for next capture
         m_cb.countdown--;
         LeaveCS;
         return d;
      }//if still counting
      else {
         // Finished
         if (m_cb.validBufferSize<=0) {
            // invalid buffer
            m_cb.bufferStatus = CSampleGrabberCB::BUF_READY_FORSAMPLE; // ready for next capture
            LeaveCS;
            return -1;
         }
         m_cb.bufferStatus = CSampleGrabberCB::BUF_SAMPLE_PREPARED; // ready for next step...
         LeaveCS;
         return 0;
      }
   }// if capture
   else {
         // ??? bug
      m_cb.bufferStatus = CSampleGrabberCB::BUF_READY_FORSAMPLE; // ready for next capture
      LeaveCS;
      _V_ ::fprintf(stderr, "CV4Lstream::OnCapture() - exit - try again \n");
      return -2;
   }

}//OnCapture


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//
// Class Constructor / Destructor (FinalRelease)
//
CV4Lstream::CV4Lstream(int p_device)
{
   m_cap.device =p_device;

   // create an instance of the grabber class
   m_SGrabCallback = new CSampleGrabberCB;
   m_SGrabCallback->pOwner = this; // set the pointer to allow access to cb structure

}


CV4Lstream::~CV4Lstream()
{
	// remove subclassing
	if (m_cap.ghWnd != 0)
	{
	}

	// tear down graph
	KillGraph();

	delete m_SGrabCallback;  // deallocate callback class

    // Release resources used by the critical section object.
}

#endif  // __UNIX__
