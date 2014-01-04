
#include "uxcapture.h"
#include "GenericImageMem.h"

#if defined(__UNIX__)

#include <memory.h>
#include <errno.h>

#include "v4l2wcprop.h"
#include "v4l2capfilter.h"
#include "v4l2stream.h"


#include "../GenericDebHelp.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// This is the constructor of a class that has been exported.
// see dscapture.h for the class definition
CUXcapture::CUXcapture()
: m_pDsCapFilterItem(NULL)
, m_streamUp(false)
, m_pProp(NULL)
, m_pStream(NULL)
, m_lastHResult(0)
, m_device(-1)
, m_settingCacheValid(false)
, m_timedivider(2)
, m_timetic(0)
, m_pBGR(NULL)
, m_pYUV(NULL)
, m_subF(wxRect(0,0,0,0))
, m_subF_fullFrame(false)
{
	return;
}


CUXcapture::~CUXcapture()
{
   m_streamUp=false;
   ControlDevice(-1);  // drop controlled items
	if (m_pBGR) delete m_pBGR;
	if (m_pYUV) delete m_pYUV;
}


//*** DScapfilter interface ***

	// error tracking: return the last HRESULT from a call
long CUXcapture::LastHResult() const
{
	return m_lastHResult;
}


// Get a generic device descriptor of device cfIndex
const CapDeviceEntry& CUXcapture::CapDevEntry(size_t cfIndex)
{
	// use the enumerator
	CAPTURE_ENUMERATOR( cFilter );
	const CV4LcapFilterItem& dsCapFilterItem = cFilter.GetCapFilter(cfIndex);

	m_pCapDeviceEntry.Init(dsCapFilterItem.FIndex(), dsCapFilterItem.FName(),
							dsCapFilterItem.DevSerial(), dsCapFilterItem.DevPath());

	return m_pCapDeviceEntry;
}//CapDeviceEntry


// Re-Enumerate all devices
bool CUXcapture::EnumerateCapFilter()
{
	CAPTURE_ENUMERATOR( cFilter );
	return ( (cFilter.EnumerateCapFilter()==0) ? false : true);
}


// return the number of devices found
size_t CUXcapture::NumItems()
{
	CAPTURE_ENUMERATOR( cFilter );
	return cFilter.NumItems();
}

//*** DSwcprop interface ***

// Enable Access to the device with cfIndex
// set to -1,0..n
//  0 .. n enable access to device m
bool CUXcapture::ControlDevice(long cfIndex)
{
   _V_ ::fprintf(stderr, "CUXcapture::ControlDevice() - entry with device index: %ld \n", cfIndex);

   // if we had one controlled before, delete it
   DropControlledDevice();

   if (cfIndex<0) return false;  // bail out if we drop only

   // use the enumerator to access the filters
   _V_ ::fprintf(stderr, "CUXcapture::ControlDevice() - use enumerator to find the device\n");
   CAPTURE_ENUMERATOR( cFilter );

   // get the item to control
   _V_ ::fprintf(stderr, "CUXcapture::ControlDevice() - create cap filter from device \n");
   m_pDsCapFilterItem = new CV4LcapFilterItem( cFilter.GetCapFilter(cfIndex));

   // open the device to control
   _V_ ::fprintf(stderr, "CUXcapture::ControlDevice() - trying to open(v4l2dev) %S\n", m_pDsCapFilterItem->DevPath().c_str());
   m_device = ::open(m_pDsCapFilterItem->DevPath().fn_str(),O_RDWR /*required*/ | O_NONBLOCK,0);

   if(-1 == m_device) {
      ::fprintf(stderr, "CUXcapture::ControlDevice() - error exit - open(v4l2dev) failed - might be OK - errno:%d\n", errno);
      if (m_pDsCapFilterItem) delete m_pDsCapFilterItem; m_pDsCapFilterItem = NULL;
      return false;
   }
   _V_ ::fprintf(stderr, "CUXcapture::ControlDevice() - opened(v4l2dev) \n");

	// create a property object
   _V_ ::fprintf(stderr, "CUXcapture::ControlDevice() - create property object \n");
	m_pProp = new CV4LwcProp();

	// make use of it by submitting the controlled item
   _V_ ::fprintf(stderr, "CUXcapture::ControlDevice() - use device to capture \n");
	m_pProp->put_CaptureDevice(m_device);

    // use the controlled Item and not the enumerator
    if (m_pDsCapFilterItem) {
        // cache the friendly device entries for user retrieval
        m_pCapDeviceEntryControlled.Init(m_pDsCapFilterItem->FIndex(), m_pDsCapFilterItem->FName(),
                                            m_pDsCapFilterItem->DevSerial(), m_pDsCapFilterItem->DevPath());
    }

    m_settingCacheValid=false; // re-read settings of this device

    _V_ ::fprintf(stderr, "CUXcapture::ControlDevice() - regular exit \n");
    return true;
}//ControlDevice


void CUXcapture::DropControlledDevice()
{
   _V_ ::fprintf(stderr, "CUXcapture::DropControlledDevice() - entry \n");

	m_streamUp = false;
   // if we had one controlled before, delete it
   _V_ ::fprintf(stderr, "CUXcapture::DropControlledDevice() - dropping objects \n");
   if (m_pStream) delete m_pStream; m_pStream=NULL;
   if (m_pProp) delete m_pProp; m_pProp = NULL;
   if (m_pDsCapFilterItem) delete m_pDsCapFilterItem; m_pDsCapFilterItem = NULL;
   // kill grabber after stream otherwise stream cannot stop v4l stuff
   if (m_device != -1) {
       close(m_device);
       _V_ ::fprintf(stderr, "CUXcapture::DropControlledDevice() - closed (v4l2dev)\n");
   }
   m_device = -1;
	if (m_pBGR) delete m_pBGR; m_pBGR=NULL;
	if (m_pYUV) delete m_pYUV; m_pYUV=NULL;

   _V_ ::fprintf(stderr, "CUXcapture::DropControlledDevice() - exit \n");
}


// Get a device descriptor of the controlled device
CapDeviceEntry& CUXcapture::ControlledCapDevEntry()
{
	return m_pCapDeviceEntryControlled;
}//ControlledCapDeviceEntry


// return a property of the controlled device
long CUXcapture::Property(EVidCapProperty prop, bool percent)
{
    long retVal;

	if (!m_pProp) return -999999;

	if (!m_settingCacheValid) RenewSettingCache();
	if (percent) {
		return m_settingCache[prop].Val2Pct(m_pProp->Prop((EVidCapProperty)prop));
	}
	else {
		retVal = m_pProp->Prop((EVidCapProperty)prop);
		// hook to get the most recent FPS setting - 0=5,1=10,2=15,3=20,4=25,5=30,6=60
		// set the divider to 3 for 5 fps and 2 for 10 fs else 1, assuming a 66ms rate
		if (prop==VCP_FPS) {
            m_timedivider=(retVal==0)?3:((retVal==1)?2:1);
		}
		return retVal;
	}
}


// set a property of the controlled device
void CUXcapture::SetProperty(EVidCapProperty prop, long p_propValue, bool percent)
{
	if (!m_pProp) return;

	if (!m_settingCacheValid) RenewSettingCache();
	if (percent)
		m_pProp->put_Prop((EVidCapProperty)prop, m_settingCache[prop].Pct2Val(p_propValue));
	else
		m_pProp->put_Prop((EVidCapProperty)prop, p_propValue);

   // nasty special treatment of the FPS property when we need to
   //  switch it in the filter rather than as property
   if ( (prop==VCP_FPS) && (m_pProp->HasCache((EVidCapProperty)prop)) ) {
      // we know that the FPS setting is cached as num / denom
      wxUint32 num = wxUint32(m_pProp->CachedValue((EVidCapProperty)prop, 0));
      wxUint32 denom = wxUint32(m_pProp->CachedValue((EVidCapProperty)prop, 1));
      m_pStream->put_Fps(num, denom);
   }
}


// returns the access for a property of the controlled device
EVidCapPropertySupport CUXcapture::PropertyHas(EVidCapProperty prop)
{
	if (!m_pProp) return VCS_no;

	if (!m_settingCacheValid) RenewSettingCache();
	return m_settingCache[prop].has;
}


// returns the min prop range of a property of the controlled device
long CUXcapture::PropertyMin(EVidCapProperty prop)
{
	if (!m_pProp) return -999999;

	if (!m_settingCacheValid) RenewSettingCache();
	return m_settingCache[prop].min;
}


// returns the max prop range of a property of the controlled device
long CUXcapture::PropertyMax(EVidCapProperty prop)
{
	if (!m_pProp) return -999999;

	if (!m_settingCacheValid) RenewSettingCache();
	return m_settingCache[prop].max;
}


// returns the prop range step of a property of the controlled device
long CUXcapture::PropertyStep(EVidCapProperty prop)
{
	if (!m_pProp) return -999999;

	if (!m_settingCacheValid) RenewSettingCache();
	return m_settingCache[prop].step;
}


//LOCAL:  re-read the propery attributes of the controlled device into a cache
void CUXcapture::RenewSettingCache()
{
	if (!m_pProp) return;

	for (int i = (int)VCP_FirstElement; i<=(int)VCP_LastElement; i++) {
		m_settingCache[i].has = (EVidCapPropertySupport)m_pProp->Has((EVidCapProperty)i);
		if (m_settingCache[i].has!=VCS_no) {
			m_pProp->Range((EVidCapProperty)i,
				m_settingCache[i].min, m_settingCache[i].max, m_settingCache[i].step);
			m_settingCache[i].SetConversion();
		}
	}
	m_settingCacheValid = true;
}



// *** Camera Streaming ***


const long CUXcapture::EventSink() const
{
	if (!m_pStream) return 0;

	return m_pStream->EventSink();
}

bool CUXcapture::put_EventSink(long newVal)
{
	if (!m_pStream) return false;

	return ( m_pStream->put_EventSink(newVal) );
}


// create a new stream into the window
bool CUXcapture::CreateNew(long WXUNUSED(hWnd))
{
   _V_ ::fprintf(stderr, "CUXcapture::CreateNew() - entry \n");

	m_lastHResult= true;

	// if we had one, throw it away
	if (m_pStream) {
      _V_ ::fprintf(stderr, "CUXcapture::CreateNew() - drop existing stream \n");
		m_pStream->Stop(); // just in case...

		delete m_pStream; m_pStream = NULL;
	}
	if (m_pBGR) delete m_pBGR; m_pBGR=NULL;
	if (m_pYUV) delete m_pYUV; m_pYUV=NULL;

	if (!m_pDsCapFilterItem) {
      ::fprintf(stderr, "CUXcapture::CreateNew() - error exit - missing device context \n");
		m_lastHResult= false;
		return false;   // we dont have the device entry
	}

   // create a stream object that connects the device to stream from
   _V_ ::fprintf(stderr, "CUXcapture::CreateNew() - create a stream \n");
	m_pStream = new CV4Lstream(m_device);

	// now create the stream (graph)
	// submit the sample grabber handle which later adds a callback
	// to be informed when a new sample can be processed
   _V_ ::fprintf(stderr, "CUXcapture::CreateNew() - init the stream \n");
	m_lastHResult = m_pStream->CreateNew(*m_pDsCapFilterItem);
	if ( !m_lastHResult ) {
      ::fprintf(stderr, "CUXcapture::CreateNew() - error exit - init failed \n");
		delete m_pStream; m_pStream = NULL;
      if (m_pBGR) delete m_pBGR; m_pBGR=NULL;
      if (m_pYUV) delete m_pYUV; m_pYUV=NULL;
		return false;
	}

	// set subframe defaults to fullframe
   _V_ ::fprintf(stderr, "CUXcapture::CreateNew() - set subframe defaults (fullframe) \n");
   wxUint16 fDimWidth, fDimHeight;
   m_pStream->FrameDimension(fDimWidth, fDimHeight);
   m_subF.SetPosition(wxPoint(0,0));
   m_subF.SetSize(wxSize(fDimWidth, fDimHeight));
   m_subF_fullFrame = true;

	m_streamUp = m_lastHResult;

   _V_ ::fprintf(stderr, "CUXcapture::CreateNew() - exit \n");
	return m_lastHResult;

}//CreateNew


long CUXcapture::Status() const
{
	if (!m_pStream) return 0;

	return m_pStream->Status();
}

bool CUXcapture::Pause()
{
	m_lastHResult = true; // init with OK

	if (!m_pStream) {
		m_lastHResult= false;
		return false;
	}

	m_lastHResult = m_pStream->Pause();
	return m_lastHResult;
}

bool CUXcapture::Stop()
{
   _V_ ::fprintf(stderr, "CUXcapture::Stop() - entry \n");

	m_lastHResult = true; // init with OK

	if (!m_pStream) {
      ::fprintf(stderr, "CUXcapture::Stop() - error exit - have no stream \n");
		m_lastHResult= false;
		return false;
	}

	m_lastHResult = m_pStream->Stop();
   _V_ ::fprintf(stderr, "CUXcapture::Stop() - exit \n");
	return m_lastHResult;
}

bool CUXcapture::Run()
{
   _V_ ::fprintf(stderr, "CUXcapture::Run() - entry \n");

	m_lastHResult = true; // init with OK

	if (!m_pStream) {
		m_lastHResult= false;
      ::fprintf(stderr, "CUXcapture::Run() - error exit - have no stream \n");
		return false;
	}

	m_lastHResult = m_pStream->Run();

   _V_ ::fprintf(stderr, "CUXcapture::Run() - exit \n");
	return m_lastHResult;
}


// init and capture images - calls back after countdown # images
bool CUXcapture::CaptureStream(bool enabled, size_t countdown)
{
	m_lastHResult = true; // init with OK

	if (!m_pStream) {
      ::fprintf(stderr, "CUXcapture::CaptureStream() - error exit - have no stream \n");
		m_lastHResult= false;
		return false;
	}
	if (enabled) {
	   // setup the image tx area as we don't copy but move ptrs around
      long x(m_subF.x), y(m_subF.y), w(m_subF.width), h(m_subF.height);
      m_pStream->SetSubframe(x, y, w, h);
      // note SetSubframe() may change values due to alingnment requirements
      m_subF.SetPosition(wxPoint(x,y));
      m_subF.SetSize(wxSize(w,h));

      //! One and only place to create the new Image Memory
      //20100626:BM - fix bug with RGB to RGB conversion (has no YUV !!!)
      if (m_pBGR) delete m_pBGR; if (m_pYUV) delete m_pYUV;
      m_pBGR = new ByteImageMem(m_subF.width*m_subF.height * 24 / 8); // BGR type 24 bits per pel

      EVideoConversion testConv = m_pStream->WCFConversion();
      if (testConv==EVC_I420_RGB24) {
         m_pYUV = new ByteImageMem(m_subF.width*m_subF.height * 12 / 8); // YUV4:2:0 type 12 bits per pel
         // store dimensions
         m_pBGR->SetFrameRect(m_subF); m_pYUV->SetFrameRect(m_subF);
         m_pStream->SetImageBuffer(m_pBGR->ByteMemPtrRef(), m_pBGR->ByteSize(),
                                    m_pYUV->ByteMemPtrRef(), m_pYUV->ByteSize());
      }
      else if ( (testConv==EVC_YUY2_RGB24)
               || (testConv==EVC_YVYU_RGB24)
               || (testConv==EVC_UYVY_RGB24)  ) {
         m_pYUV = new ByteImageMem(m_subF.width*m_subF.height * 16 / 8); // YUV4:2:2 type 16 bits per pel
         // store dimensions
         m_pBGR->SetFrameRect(m_subF); m_pYUV->SetFrameRect(m_subF);
         m_pStream->SetImageBuffer(m_pBGR->ByteMemPtrRef(), m_pBGR->ByteSize(),
                                    m_pYUV->ByteMemPtrRef(), m_pYUV->ByteSize());
      }
      else if (testConv==EVC_RGB24_RGB24) {
         // native was RGB24 - we have no YUV type
         // store dimensions
         m_pBGR->SetFrameRect(m_subF);
         m_pStream->SetImageBuffer(m_pBGR->ByteMemPtrRef(), m_pBGR->ByteSize(),
                                    NULL, 0);
      }
      else {
         // unhandled conversion in wxWebcam
         assert(false);
      }
	}
	m_lastHResult = m_pStream->CaptureStream(enabled, countdown);

	return m_lastHResult;
}

// query the current type of streaming
const EGraphStreamingType CUXcapture::CaptureStatus() const
{
	if (!m_pStream) return GST_Off;

	return (EGraphStreamingType)m_pStream->CaptureStatus();
}


// fills the submitted sample object with data
void CUXcapture::CapturedSample(wxWebSample& sample)
{
	// clear
	sample.Init(false, EVC_UNKNOWN, NULL, NULL);

	if (!m_pStream) return;

	CapSampleType lSample;
	if ( m_pStream->CapturedSample(lSample) ) {
	   //20080518:BM native can be RGB so we don't have a YUV at hand
	   if (lSample.nativeIsRGB) {
         sample.Init(m_subF_fullFrame, EVC_RGB24_RGB24, NULL, m_pBGR);
         delete m_pYUV; // no YUV image, delete image instance here
	   }
      else {
         sample.Init(m_subF_fullFrame, m_pStream->WCFConversion(),m_pYUV, m_pBGR);
      }
      // NOTE image mem is taken over by WebSample - so NULLify ptrs here
      m_pYUV=NULL; m_pBGR=NULL;
	}

	return;
}


// just reply to ignore that sample and continue
void CUXcapture::DropCapturedSample()
{
   if (!m_streamUp) return;
	if (!m_pStream)  return;

	m_pStream->DropCapturedSample();
}


void CUXcapture::FrameDimension(wxUint16 &width, wxUint16 &height) const
{
	width = 0, height = 0;
   if (!m_streamUp) return;
	if (!m_pStream) return;

	m_pStream->FrameDimension(width, height);
}

bool CUXcapture::SetFormat(wxUint16 binning)
{
	m_lastHResult = true; // init with OK

	if (!m_pStream) {
		m_lastHResult= false;
		return false;
	}

	m_lastHResult = m_pStream->SetFormat(binning);
	// set subframe defaults to fullframe
   wxUint16 fDimWidth, fDimHeight;
   m_pStream->FrameDimension(fDimWidth, fDimHeight);
   m_subF.SetPosition(wxPoint(0,0));
   m_subF.SetSize(wxSize(fDimWidth, fDimHeight));
   m_subF_fullFrame = true;

	return m_lastHResult;
}

wxUint16 CUXcapture::MaxFormat()
{
   // i.e. level 1 = 640x480; 2 = 320x240; 4 = 160x120; 5 = 352x288 cropped
   return 1; // TODO see how to support SetFormat()
}

// set the subframe format (binning factor 1 assumed)
//  needs two points of a rectangle that makes the subframe in pixel coords
//  the subframe might get adjusted if binning changes
//  switching back to the max frame is done by setting all points to 1
bool CUXcapture::SetSubframe(wxUint16 startX, wxUint16 startY,
                              wxUint16 endX, wxUint16 endY)
{
	m_lastHResult = true; // init with OK

	if (!m_streamUp) {
		m_lastHResult = false;
		return false;
	}
	if (!m_pStream) {
		m_lastHResult = false;
		return false;
	}

   m_subF_fullFrame = false;
   wxUint16 ccdsx, ccdsy, ccdw, ccdh;
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
   wxUint16 fDimWidth, fDimHeight;
   m_pStream->FrameDimension(fDimWidth, fDimHeight);
   if (ccdw==1 && ccdh==1) {
      ccdsx=0; ccdsy=0;
      ccdw  = fDimWidth;
      ccdh = fDimHeight;
      m_subF_fullFrame = true;
   }
   // handle too small size boxes
   if (ccdw<10 || ccdh<10) {
      m_lastHResult = false;
   }
   else {
      // seems OK - send values
      m_subF.SetPosition(wxPoint(ccdsx, ccdsy));
      m_subF.SetSize(wxSize(ccdw, ccdh));
      m_subF_fullFrame = (m_subF_fullFrame || ( (ccdw==fDimWidth) && (ccdh==fDimHeight) ));
      m_lastHResult = true;
   }

	return m_lastHResult;
}

void CUXcapture::ScaleVideoOutput()
{
	m_lastHResult = true; // init with OK

   if (!m_streamUp) return;
	if (!m_pStream) {
		m_lastHResult= false;
		return;
	}

	m_pStream->ScaleVideoOutput();
}


void CUXcapture::VideoOutputVisible(bool enable)
{
	m_lastHResult = true; // init with OK

   if (!m_streamUp) return;
	if (!m_pStream) {
		m_lastHResult= false;
		return;
	}

	m_pStream->VideoOutputVisible(enable);
}

// query the current conversion type

const EVideoConversion CUXcapture::WCFConversion()
{
   if (!m_streamUp) return EVC_UNKNOWN;
	if (!m_pStream)  return EVC_UNKNOWN;

	return m_pStream->WCFConversion();
}

// set a new conversion type
void CUXcapture::put_WCFConversion(EVideoConversion newVal)
{
   if (!m_streamUp) return;
	if (!m_pStream) return;

	m_pStream->put_WCFConversion( newVal );
}


// query the current pixel order (for raw only)
const EVideoPixelOrder CUXcapture::PixelOrder()
{
   if (!m_streamUp) return EVO_GB;
	if (!m_pStream)  return EVO_GB;

   return m_pStream->PixelOrder();
}

// set the new pixel order (for raw only)
void CUXcapture::put_PixelOrder(EVideoPixelOrder newVal)
{
   if (!m_streamUp) return;
	if (!m_pStream) return;

   m_pStream->put_PixelOrder( newVal );
}

// get the current FPS/DPS values  //20100702:BM - added
bool CUXcapture::GetRates(float& fps, float& dps)
{
   if (!m_streamUp) return false;
	if (!m_pStream) return false;

   return m_pStream->GetRates(fps, dps);
}

// returns true if a an OnCapture call is expected
// supports non event driven apps i.e. EventSink is not provided
ESYNMode CUXcapture::OnCaptureRequired()
{
   if (!m_streamUp) return ESYN_None;
	if (!m_pStream)  return ESYN_None;

   return m_pStream->OnCaptureRequired();
}


// must be called to gather the sampled image
// either via WM_CAPTURE handler or polled via OnCaptureRequired
// Note: Linux version does not (yet) provide the message into the wx handler - so use this one
//
// OnCapture returns 0 if a new sample must be collected
// positive numbers are capture events left until finished
// negative numbers are error indications
long CUXcapture::OnCapture()
{
   if (!m_streamUp) return -3;
	if (!m_pStream)  return -3;

   return m_pStream->OnCapture();
}


#endif // defined __UNIX__


