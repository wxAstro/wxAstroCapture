// dscapture.cpp : Defines the entry point for the DLL application.
//

#include "dscapture.h"
#include "GenericImageMem.h"

#if defined(_WXMSW_)

#include <memory.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ensure that we link against the DScaplib DLL import library
#pragma comment (lib,"dscaplib.lib")

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
DScapLib& CDScapture::m_pDsCapLib = DScapLib();

// This is the constructor of a class that has been exported.
// see dscapture.h for the class definition
CDScapture::CDScapture()
: m_settingCacheValid(false)
, m_streamUp(false)
, m_pDsCapFilterItem(NULL)
, m_lastHResult(true)
, m_pBGR(NULL)
, m_pYUV(NULL)
, m_subF(wxRect(0,0,0,0))
, m_subF_fullFrame(false)
{
	return;
}


CDScapture::~CDScapture()
{
   m_streamUp=false;
	if (m_pDsCapFilterItem) delete m_pDsCapFilterItem;
	if (m_pBGR) delete m_pBGR;
	if (m_pYUV) delete m_pYUV;
}


//*** DScapfilter interface ***

	// error tracking: return the last HRESULT from a call
long CDScapture::LastHResult() const
{
	return m_lastHResult;
}


// Get a generic device descriptor of device cfIndex
const CapDeviceEntry& CDScapture::CapDevEntry(size_t cfIndex)
{
	// use the enumerator
	const CDScapFilterItem& dsCapFilterItem = m_pDsCapLib.GetCapFilter((long)cfIndex);

#ifdef _UNICODE
	m_pCapDeviceEntry.Init(dsCapFilterItem.FIndex(), dsCapFilterItem.FName(),
                        dsCapFilterItem.DevSerial(), dsCapFilterItem.DeviceVidPid());
#else // ANSI
	m_pCapDeviceEntry.Init(dsCapFilterItem.FIndex(), dsCapFilterItem.FNameA(),
                        dsCapFilterItem.DevSerialA(), dsCapFilterItem.DeviceVidPidA());
#endif
	return m_pCapDeviceEntry;
}//CapDeviceEntry


// Re-Enumerate all devices
bool CDScapture::EnumerateCapFilter()
{
   return ( m_pDsCapLib.EnumerateCapFilter() );
}


// return the number of devices found
size_t CDScapture::NumItems()
{
   return (size_t)m_pDsCapLib.NumItems();
}

//*** DSwcprop interface ***

// Enable Access to the device with cfIndex
// set to -1,0..n
//  0 .. n enable access to device m
bool CDScapture::ControlDevice(long cfIndex)
{
	// if we had one controlled before, delete it
   DropControlledDevice();

	// get the item to control
	m_pDsCapFilterItem = new CDScapFilterItem(m_pDsCapLib.GetCapFilter(cfIndex));
	// create a property object

	// make use of it by submitting the controlled item
	if ( !(m_pDsCapLib.PROP().put_CaptureFilter(m_pDsCapFilterItem)) ) {
		// if someting goes wrong, drop control
		if (m_pDsCapFilterItem) delete m_pDsCapFilterItem; m_pDsCapFilterItem = NULL;
		return false;
	}
	else
	{
		// use the controlled Item and not the enumerator
		if (m_pDsCapFilterItem) {
			// cache the friendly device entries for user retrieval
#ifdef _UNICODE
			m_pCapDeviceEntryControlled.Init(m_pDsCapFilterItem->FIndex(), m_pDsCapFilterItem->FName(),
                        m_pDsCapFilterItem->DevSerial(), m_pDsCapFilterItem->DeviceVidPid());
#else // ANSI
			m_pCapDeviceEntryControlled.Init(m_pDsCapFilterItem->FIndex(), m_pDsCapFilterItem->FNameA(),
                        m_pDsCapFilterItem->DevSerialA(), m_pDsCapFilterItem->DeviceVidPidA());
#endif
		}

		m_settingCacheValid=false; // re-read settings of this device
		return true;
	}
}//ControlDevice

void CDScapture::DropControlledDevice()
{
	m_streamUp = false;
	// if we had one controlled before, delete it
	if (m_pDsCapFilterItem) delete m_pDsCapFilterItem; m_pDsCapFilterItem = NULL;
//	if (m_pBGR) delete m_pBGR; m_pBGR=NULL;
//	if (m_pYUV) delete m_pYUV; m_pYUV=NULL;

	m_pDsCapLib.PROP_INIT(); // reinitialize
}


// Get a device descriptor of the controlled device
CapDeviceEntry& CDScapture::ControlledCapDevEntry()
{
	return m_pCapDeviceEntryControlled;
}//ControlledCapDeviceEntry


// return the chipset of the controlled device
const EKnownChipsetCat CDScapture::Chipset() const
{
   return m_pDsCapLib.PROP().Chipset();
}//Chipset


// return a property of the controlled device
long CDScapture::Property(EVidCapProperty prop, bool percent)
{
	if (!m_settingCacheValid) RenewSettingCache();
	if (percent)
		return m_settingCache[prop].Val2Pct(m_pDsCapLib.PROP().Prop((EVidCapProperty)prop));
	else
		return m_pDsCapLib.PROP().Prop((EVidCapProperty)prop);
}


// set a property of the controlled device
void CDScapture::SetProperty(EVidCapProperty prop, long p_propValue, bool percent)
{
	if (!m_settingCacheValid) RenewSettingCache();
	if (percent)
		m_pDsCapLib.PROP().put_Prop((EVidCapProperty)prop, m_settingCache[prop].Pct2Val(p_propValue));
	else
		m_pDsCapLib.PROP().put_Prop((EVidCapProperty)prop, p_propValue);
}


// returns the access for a property of the controlled device
EVidCapPropertySupport CDScapture::PropertyHas(EVidCapProperty prop)
{
	if (!m_settingCacheValid) RenewSettingCache();
	return m_settingCache[prop].has;
}


// returns the min prop range of a property of the controlled device
long CDScapture::PropertyMin(EVidCapProperty prop)
{
	if (!m_settingCacheValid) RenewSettingCache();
	return m_settingCache[prop].min;
}


// returns the max prop range of a property of the controlled device
long CDScapture::PropertyMax(EVidCapProperty prop)
{
	if (!m_settingCacheValid) RenewSettingCache();
	return m_settingCache[prop].max;
}


// returns the prop range step of a property of the controlled device
long CDScapture::PropertyStep(EVidCapProperty prop)
{
	if (!m_settingCacheValid) RenewSettingCache();
	return m_settingCache[prop].step;
}


//LOCAL:  re-read the propery attributes of the controlled device into a cache
void CDScapture::RenewSettingCache()
{
	for (int i = (int)VCP_FirstElement; i<=(int)VCP_LastElement; i++) {
		m_settingCache[i].has = (EVidCapPropertySupport)m_pDsCapLib.PROP().Has((EVidCapProperty)i);
		if (m_settingCache[i].has!=VCS_no) {
			m_pDsCapLib.PROP().Range((EVidCapProperty)i,
				m_settingCache[i].min, m_settingCache[i].max, m_settingCache[i].step);
			m_settingCache[i].SetConversion();
		}
	}
	m_settingCacheValid = true;
}



// *** Camera Streaming ***


const long CDScapture::EventSink() const
{
	return (long)m_pDsCapLib.STREAM().EventSink();
}

bool CDScapture::put_EventSink(long newVal)
{
	return ( SUCCEEDED(m_pDsCapLib.STREAM().put_EventSink((HANDLE)newVal)) );
}


// create a new stream into the window
// drop all if hWnd is -1
bool CDScapture::CreateNew(long hWnd)
{

	m_lastHResult= true;

	// if we had one, throw it away
	m_pDsCapLib.STREAM().Stop(); // just in case...

		//delete m_pStream; m_pStream = NULL;

	if (m_pBGR) delete m_pBGR; m_pBGR=NULL;
	if (m_pYUV) delete m_pYUV; m_pYUV=NULL;

	if (hWnd==-1) {
		return true;
	}
	if (!m_pDsCapFilterItem) {
		m_lastHResult= false;
		return m_lastHResult;
	}

	m_pDsCapLib.STREAM_INIT(); // reinitialize

	// now create the stream (graph)
	// submit the window handle (this window will receive events from CDSstream)
	m_lastHResult = m_pDsCapLib.STREAM().CreateNew((HANDLE)hWnd, m_pDsCapFilterItem);
	if ( m_lastHResult ) {
      // set subframe defaults to fullframe
      long fDimWidth, fDimHeight;
      m_pDsCapLib.STREAM().FrameDimension(fDimWidth, fDimHeight);
      m_subF.SetPosition(wxPoint(0,0));
      m_subF.SetSize(wxSize(fDimWidth, fDimHeight));
      m_subF_fullFrame = true;
	}
	else {
	   // cannot - drop everything
   	m_pDsCapLib.STREAM_INIT(); // reinitialize
   	DropControlledDevice();
	}
	m_streamUp = m_lastHResult;
	return m_lastHResult;

}//CreateNew


long CDScapture::Status() const
{
	return m_pDsCapLib.STREAM().Status();
}

bool CDScapture::Pause()
{
	m_lastHResult = m_pDsCapLib.STREAM().Pause();
	return m_lastHResult;
}

bool CDScapture::Stop()
{
	m_lastHResult = m_pDsCapLib.STREAM().Stop();
	return m_lastHResult;
}

bool CDScapture::Run()
{
	m_lastHResult = m_pDsCapLib.STREAM().Run();
	return m_lastHResult;
}

// init and capture images - calls back after countdown # images
bool CDScapture::CaptureStream(bool enabled, size_t countdown)
{
	m_lastHResult = true; // init with OK

	if (enabled) {
	   // setup the image tx area as we don't copy but move ptrs around
      long x(m_subF.x), y(m_subF.y), w(m_subF.width), h(m_subF.height);
      m_pDsCapLib.STREAM().SetSubframe(x, y, w, h);
      // note SetSubframe() may change values due to alingnment requirements
      m_subF.SetPosition(wxPoint(x,y));
      m_subF.SetSize(wxSize(w,h));

      //! One and only place to create the new Image Memory
      if (m_pBGR) delete m_pBGR; if (m_pYUV) delete m_pYUV;
      m_pBGR = new ByteImageMem(m_subF.width*m_subF.height * 24 / 8); // BGR type 24 bits per pel

      //20100613:BM - fix bug with RGB to RGB conversion (has no YUV !!!)
      EVideoConversion testConv = m_pDsCapLib.STREAM().WCFConversion();
      if (testConv==EVC_I420_RGB24) {
         m_pYUV = new ByteImageMem(m_subF.width*m_subF.height * 12 / 8); // YUV4:2:0 type 12 bits per pel
         // store dimensions
         m_pBGR->SetFrameRect(m_subF); m_pYUV->SetFrameRect(m_subF);
         m_pDsCapLib.STREAM().SetImageBuffer(m_pBGR->ByteMemPtrRef(), m_pBGR->ByteSize(),
                                    m_pYUV->ByteMemPtrRef(), m_pYUV->ByteSize());
      }
      else if (   (testConv==EVC_YUY2_RGB24)
               || (testConv==EVC_YVYU_RGB24)
               || (testConv==EVC_UYVY_RGB24)  ) {
         m_pYUV = new ByteImageMem(m_subF.width*m_subF.height * 16 / 8); // YUV4:2:2 type 16 bits per pel
         // store dimensions
         m_pBGR->SetFrameRect(m_subF); m_pYUV->SetFrameRect(m_subF);
         m_pDsCapLib.STREAM().SetImageBuffer(m_pBGR->ByteMemPtrRef(), m_pBGR->ByteSize(),
                                    m_pYUV->ByteMemPtrRef(), m_pYUV->ByteSize());
      }
      else if (testConv==EVC_RGB24_RGB24) {
         // native was RGB24 - we have no YUV type
         // store dimensions
         m_pBGR->SetFrameRect(m_subF);
         m_pDsCapLib.STREAM().SetImageBuffer(m_pBGR->ByteMemPtrRef(), m_pBGR->ByteSize(),
                                    NULL, 0);
      }
      else {
         // unhandled conversion in wxWebcam
         assert(false);
      }
	}
	m_lastHResult = m_pDsCapLib.STREAM().CaptureStream(enabled, (long)countdown);

	return m_lastHResult;
}


// query the current type of streaming
const EGraphStreamingType CDScapture::CaptureStatus() const
{
	return (EGraphStreamingType)m_pDsCapLib.STREAM().CaptureStatus();
}


// fills the submitted sample object with data
void CDScapture::CapturedSample(wxWebSample& sample)
{
	// clear
	sample.Init(false, EVC_UNKNOWN, NULL, NULL);

	CapSampleType lSample;
	if ( m_pDsCapLib.STREAM().CapturedSample(lSample) ) {
	   //20080518:BM native can be RGB so we don't have a YUV at hand
	   if (lSample.nativeIsRGB) {
         sample.Init(m_subF_fullFrame, EVC_RGB24_RGB24, NULL, m_pBGR);
         delete m_pYUV; // no YUV image, delete image instance here
	   }
      else {
         sample.Init(m_subF_fullFrame, m_pDsCapLib.STREAM().WCFConversion(), m_pYUV, m_pBGR);
      }
      // NOTE image mem is taken over by WebSample - so NULLify ptrs here
      m_pYUV=NULL; m_pBGR=NULL;
	}

	return;
}


// just reply to ignore that sample and continue
void CDScapture::DropCapturedSample()
{
   if (!m_streamUp) return;
	m_pDsCapLib.STREAM().DropCapturedSample();
}

// query the current video frame dimension
void CDScapture::FrameDimension(unsigned short& width, unsigned short& height) const
{
	width = 0, height = 0;
   if (!m_streamUp) return;

	long lwidth = width;
	long lheight = height;

	m_pDsCapLib.STREAM().FrameDimension(lwidth,lheight);

	width  = (unsigned short)(lwidth);
	height = (unsigned short)(lheight);
}

// Set the Format of the Capture OutPin (if supported) according to a binning level
// i.e. level 1 = 640x480; 2 = 320x240; 4 = 160x120; 5 = 352x288 cropped
bool CDScapture::SetFormat(unsigned short binning)
{
	m_lastHResult = m_pDsCapLib.STREAM().SetFormat((long)binning);
	if (m_lastHResult) {
      // set subframe defaults to fullframe
      long fDimWidth, fDimHeight;
      m_pDsCapLib.STREAM().FrameDimension(fDimWidth, fDimHeight);
      m_subF.SetPosition(wxPoint(0,0));
      m_subF.SetSize(wxSize(fDimWidth, fDimHeight));
      m_subF_fullFrame = true;
	}

	return m_lastHResult;
}

unsigned short CDScapture::MaxFormat()
{
   // i.e. level 1 = 640x480; 2 = 320x240; 4 = 160x120
   return 1; // we dont support any kind of binning
}

// set the subframe format (binning factor 1 assumed)
//  needs two points of a rectangle that makes the subframe in pixel coords
//  the subframe might get adjusted if binning changes
//  switching back to the max frame is done by setting all points to 1
bool CDScapture::SetSubframe(unsigned short startX, unsigned short startY,
                              unsigned short endX, unsigned short endY)
{
	m_lastHResult = true; // init with OK

	if (!m_streamUp) {
		m_lastHResult = false;
		return m_lastHResult;
	}

   m_subF_fullFrame = false;
   unsigned short ccdsx, ccdsy, ccdw, ccdh;
   // handle reversed boxes
   if (startX<=endX) {
      ccdsx = startX; ccdw = endX - startX +1; // pt inclusive
   }
   else {
      ccdsx = endX; ccdw = startX - endX +1; // pt inclusive
      // this happens sometimes...
      ccdw=2; // bail out with too small rectangle
   }
   if (startY<=endY) {
      // we take care of it when returning the bitmap in wxWebSample
      ccdsy = startY; ccdh = endY - startY + 1; // pt inclusive
   }
   else {
      ccdsy = endY; ccdh = startY - endY + 1; // pt inclusive
      // this happens sometimes...
      ccdh=2; // bail out with too small rectangle
   }
   // handle max format box (all dims are equal == 1 pixel image)
   long fDimWidth, fDimHeight;
   m_pDsCapLib.STREAM().FrameDimension(fDimWidth, fDimHeight);
   if (ccdw==1 && ccdh==1) {
      ccdsx=0; ccdsy=0;
      ccdw  = (unsigned short)(fDimWidth);
      ccdh = (unsigned short)(fDimHeight);
      m_subF_fullFrame = true;
   }
   // handle too small size boxes
   if (ccdw<10 || ccdh<10) {
      m_lastHResult = false;
   }
   else {
      // seems OK -use values
      m_subF.SetPosition(wxPoint(ccdsx, ccdsy));
      m_subF.SetSize(wxSize(ccdw, ccdh));
      m_subF_fullFrame = (m_subF_fullFrame || ( (ccdw==fDimWidth) && (ccdh==fDimHeight) ));
      m_lastHResult = true;
   }

	return m_lastHResult;
}


void CDScapture::ScaleVideoOutput()
{
	m_lastHResult = true; // init with OK
   if (!m_streamUp) return;
	m_pDsCapLib.STREAM().ScaleVideoOutput();
}

void CDScapture::VideoOutputVisible(bool enable)
{
	m_lastHResult = true; // init with OK

   if (!m_streamUp) return;

	m_pDsCapLib.STREAM().VideoOutputVisible(enable);
}


// query the current conversion type
const EVideoConversion CDScapture::WCFConversion()
{
   if (!m_streamUp) return EVC_UNKNOWN;

	return m_pDsCapLib.STREAM().WCFConversion();
}

// set a new conversion type
void CDScapture::put_WCFConversion(EVideoConversion newVal)
{
   if (!m_streamUp) return;

	m_pDsCapLib.STREAM().put_WCFConversion( newVal );
}


// query the current pixel order (for raw only)
const EVideoPixelOrder CDScapture::PixelOrder()
{
   if (!m_streamUp) return EVO_GB;

	return m_pDsCapLib.STREAM().PixelOrder();
}

// set the new pixel order (for raw only)
void CDScapture::put_PixelOrder(EVideoPixelOrder newVal)
{
   if (!m_streamUp) return;

	m_pDsCapLib.STREAM().put_PixelOrder( newVal );
}

// get the current FPS/DPS values  //20100702:BM - added
bool CDScapture::GetRates(float& fps, float& dps)
{
   if (!m_streamUp) return false;

   return m_pDsCapLib.STREAM().GetRates(fps, dps);
}

// Drawing into the stream image

// clear the graphics items
void CDScapture::Clear()
{
   if (!m_streamUp) return;

	m_pDsCapLib.STREAM().Clear();
}
// Sets the current pen for drawing (primitive pen...)
void CDScapture::SetPen(unsigned long rgbColor, unsigned short width)
{
   if (!m_streamUp) return;

	m_pDsCapLib.STREAM().SetPen(rgbColor,  width);
}
// Draws a circle with the given centre and radius.
bool CDScapture::DrawCircle(short x, short y, short radius)
{
   if (!m_streamUp) return false;

	return m_pDsCapLib.STREAM().DrawCircle(x, y, radius);
}
// Draws a rectangle with the given top left corner, and with the given size.
// The current pen is used for the outline (no filling)
bool CDScapture::DrawRectangle(short x, short y, short width, short height)
{
   if (!m_streamUp) return false;

	return m_pDsCapLib.STREAM().DrawRectangle(x, y, width, height);
}
// Draws a line from the first point to the second.
// The current pen is used for drawing the line.
// Note that the point (x2, y2) is not part of the line and is not drawn by this function
bool CDScapture::DrawLine(short x1, short y1, short x2, short y2)
{
   if (!m_streamUp) return false;

	return m_pDsCapLib.STREAM().DrawLine(x1, y1, x2, y2);
}

// END Drawing into the stream image




// returns true if a an OnCapture call is expected
// supports non event driven apps i.e. EventSink is not provided
ESYNMode CDScapture::OnCaptureRequired()
{
   if (!m_streamUp) return ESYN_None;

	return m_pDsCapLib.STREAM().OnCaptureRequired();
}


// must be called to gather the sampled image
// either via WM_CAPTURE handler or polled via OnCaptureRequired
//
// OnCapture returns 0 if a new sample must be collected
// positive numbers are capture events left until finished
// negative numbers are error indications
long CDScapture::OnCapture()
{
   if (!m_streamUp) return -3;

	return m_pDsCapLib.STREAM().OnCapture();
}



bool CDScapture::ShowSourceDlg(long hwndParent)
{
	m_lastHResult = true; // init with OK

   if (!m_streamUp) return false;

	m_lastHResult = m_pDsCapLib.STREAM().ShowSourceDlg(hwndParent);
	return m_lastHResult;
}

bool CDScapture::ShowFormatDlg(long hwndParent)
{
	m_lastHResult = true; // init with OK

   if (!m_streamUp) return false;

	m_lastHResult = m_pDsCapLib.STREAM().ShowFormatDlg(hwndParent);
	if (m_lastHResult) {
      // set subframe defaults to fullframe
      long fDimWidth, fDimHeight;
      m_pDsCapLib.STREAM().FrameDimension(fDimWidth, fDimHeight);
      m_subF.SetPosition(wxPoint(0,0));
      m_subF.SetSize(wxSize(fDimWidth, fDimHeight));
      m_subF_fullFrame = true;
	}

	return m_lastHResult;
}

#endif  // _WXMSW_
