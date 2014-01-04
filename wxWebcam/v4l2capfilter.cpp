// CV4LcapFilter.cpp : Implementation of CV4LcapFilter
//
//---------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if defined(__UNIX__)

/////////////////////////////////////////////////////////////////////////////

#include "v4l2capfilter.h"

#include <errno.h>
#include <sys/ioctl.h>

#include <libv4l1-videodev.h>
//#include <linux/videodev.h>
#include <linux/videodev2.h>

#include "wxCameraTypes.h"  // fourCCs

#include "../GenericDebHelp.h"


#define CLEAR(x) memset(&(x), 0, sizeof(x))



/////////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////
// CV4LcapFilter

// Singleton
CV4LcapFilter& CV4LcapFilter::Instance()
{
	static CV4LcapFilter instance;
	return instance;
}


CV4LcapFilter::CV4LcapFilter()
: m_numFilter(0)
{
	for (int i=0; i<MAXITEM; i++)
	{
		m_pFilters[i] = NULL;
	}

	// just start with all enumerated
	EnumerateCapFilter();
}


CV4LcapFilter::~CV4LcapFilter()
{
	DropAll();
}

void CV4LcapFilter::DropAll()
{
	for (int i=0; i<MAXITEM; i++)
	{
		if (m_pFilters[i])
			delete m_pFilters[i]; m_pFilters[i] = NULL;
	}
	m_numFilter = 0;
}


const CV4LcapFilterItem& CV4LcapFilter::GetCapFilter(long cfIndex) const
{

	if (cfIndex>=m_numFilter || cfIndex<0)
	{
		return m_nullFilter;
	}

	return *(m_pFilters[cfIndex]);
}//GetCapFilter

//---------------------------------------------------------------------------
// return a collection of IVCapFilter of the give category
//
// returns:
// S_OK
//from CLSIDFromString
// E_INVALIDARG
// CO_E_CLASSTRING		The class string was improperly formatted.
// REGDB_E_WRITEREGDB	The CLSID corresponding to the class string was not found in the registry.
//from CreateClassEnumerator
// E_OUTOFMEMORY		There is not enough memory available to create a class enumerator.
// E_POINTER			NULL pointer argument to classenum
//
bool CV4LcapFilter::EnumerateCapFilter()
{
   _V_ ::fprintf(stderr, "CV4LcapFilter::EnumerateCapFilter: entry\n");

	// first cleanup
	DropAll();

   wxString base_path = wxT("/dev/video");

    for(int i=0; i<MAXITEM; i++) {

        m_actualDevPath = base_path;
        m_actualDevPath << i;

        struct stat st;
        if(-1 == stat(m_actualDevPath.fn_str(),&st)) continue;  //  cannot identify
        if(!S_ISCHR(st.st_mode)) continue;  //  not a device
        int device = open(m_actualDevPath.fn_str(),O_RDWR /*required*/ | O_NONBLOCK,0);
        if(-1 == device) continue; // could not be opened


        CLEAR(m_actualCap);
        m_actualCap.capabilities =  V4L2_CAP_VIDEO_CAPTURE;
        if(-1 == xioctl(device,VIDIOC_QUERYCAP,&m_actualCap)) {
            // Not a v4l2 device (or other problem
            close(device);
            continue;
        }
        if ( (m_actualCap.capabilities | V4L2_CAP_VIDEO_CAPTURE) == 0 ) {
            // Not a v4l2 capture device
            close(device);
            continue;
        }
        _V_ ::fprintf(stderr, "CV4LcapFilter::EnumerateCapFilter: Probing %ls\n", m_actualDevPath.c_str());
        EnumPixFormat(device);

        close(device);
    }//for
    m_actualDevPath=wxT("");
    CLEAR(m_actualCap);

   if (m_numFilter<1) {
      ::fprintf(stderr, "CV4LcapFilter::EnumerateCapFilter: could not find any supported device\n");
   }

   _V_ ::fprintf(stderr, "CV4LcapFilter::EnumerateCapFilter: exit\n");
    return (m_numFilter>0);
}


//!@brief Add a found configuration to the list of supported device modes
void CV4LcapFilter::AddSupport(int v4l2_dev, const v4l2_frmivalenum& fival)
{
   wxString friendly_name = wxString::FromAscii((char*)m_actualCap.card);
   wxString dName;


   if ( friendly_name.Len()>0 ) {
      CV4LcapFilterItem* item = new CV4LcapFilterItem;// create a new VCapFilter
      item->put_DevPath(m_actualDevPath);
      item->put_PixFormat(fival.pixel_format);
      item->put_FrameSize(fival.width, fival.height);

      item->put_Fps(fival.discrete.numerator, fival.discrete.denominator);
      dName = friendly_name + wxString::Format(wxT(" %ux%u (%c%c%c%c)"),
            fival.width, fival.height,
            char(fival.pixel_format&0xff), char((fival.pixel_format>>8)&0xff),
            char((fival.pixel_format>>16)&0xff), char((fival.pixel_format>>24)&0xff));
      item->put_FName(dName);
      // TODO - do we have a serial number ???
      // m_pFilters[m_numFilter]->put_DevSerial(m_devSerial);

      // look if we have the same then Merge the FPS value and leave it alone
      bool found = false;
      for (int i=m_numFilter-1; i>=0; i--) {
         ;
         if ( found = (m_pFilters[i]->IsSimilar(*item) ) ) {
            m_pFilters[i]->MergeFps(item->Fps_Num(), item->Fps_Denom());
            break;
         }
      }
      if (!found) {
         ::fprintf(stderr, "CV4LcapFilter::AddSupport: %ls\n", item->FName().c_str());
         m_pFilters[m_numFilter] = item;
         // add an index to the filter
         m_pFilters[m_numFilter]->put_FIndex(m_numFilter);
         m_numFilter++;
      }


   }
}


//!@brief Enumerate the FourCC codes the device supports
/// Only discrete types are supported
void CV4LcapFilter::EnumPixFormat(int v4l2_dev)
{
   v4l2_fmtdesc fmt; CLEAR(fmt);
   fmt.index = 0;
   fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   while(xioctl(v4l2_dev, VIDIOC_ENUM_FMT, &fmt) == 0) {
      _V_ ::fprintf(stderr, "EnumPixFormat: fourCC  0x%X (%c%c%c%c)\n", fmt.pixelformat,
         char(fmt.pixelformat&0xff), char((fmt.pixelformat>>8)&0xff),
         char((fmt.pixelformat>>16)&0xff), char((fmt.pixelformat>>24)&0xff));
      if ( IsSupportedPixFormat(fmt) ) {
         EnumFrameSize(v4l2_dev, fmt);
      }
      else {
         ::fprintf(stderr, "CV4LcapFilter::EnumPixFormat: fourCC  0x%X (%c%c%c%c) not supported\n", fmt.pixelformat,
                                    char(fmt.pixelformat&0xff), char((fmt.pixelformat>>8)&0xff),
                                    char((fmt.pixelformat>>16)&0xff), char((fmt.pixelformat>>24)&0xff));
      }
		fmt.index++;
   }
}


//!@brief Enumerate the Framesizes the device supports
/// Only discrete types are supported
void CV4LcapFilter::EnumFrameSize(int v4l2_dev, const v4l2_fmtdesc& fmt)
{
	v4l2_frmsizeenum fsize; CLEAR(fsize);
	fsize.index = 0;
	fsize.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fsize.pixel_format = fmt.pixelformat;

	while(xioctl(v4l2_dev, VIDIOC_ENUM_FRAMESIZES, &fsize) == 0) {

		// Copy the frame size attributes
		if(fsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
         _V_ ::fprintf(stderr, "    size discrete: %d x %d (width x height)\n", fsize.discrete.width, fsize.discrete.height);
         if ( IsSupportedFrameSize(fsize) ) {
            EnumFPS(v4l2_dev, fsize);
         }
         else {
            ::fprintf(stderr, "CV4LcapFilter::EnumFrameSize: size discrete: %d x %d (width x height) not supported\n",
                              fsize.discrete.width, fsize.discrete.height);
         }
		}
		else if(fsize.type == V4L2_FRMSIZE_TYPE_CONTINUOUS) {
         ::fprintf(stderr, "CV4LcapFilter::EnumFrameSize: size cont:     %d x %d (width x height) not supported\n",
                              fsize.stepwise.min_width, fsize.stepwise.min_height);
		}
		else if(fsize.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
         ::fprintf(stderr, "CV4LcapFilter::EnumFrameSize: size step:     %d x %d (width x height) not supported\n",
                              fsize.stepwise.min_width, fsize.stepwise.min_height);
		}
		fsize.index++;
	}//while

   // we are missing any framesize for this device
	if (fsize.index==0) {
      ::fprintf(stderr, "CV4LcapFilter::EnumFrameSize: device does not support enumerate framsize\n");
      ::fprintf(stderr, "  . trying default resolution of 640x480\n");
      fsize.type = V4L2_FRMSIZE_TYPE_DISCRETE;
      fsize.discrete.width=640; fsize.discrete.height=480;
      EnumFPS(v4l2_dev, fsize);
	}
}


//!@brief Enumerate the Framerates the device supports
/// Only discrete types are supported
void CV4LcapFilter::EnumFPS(int v4l2_dev, const v4l2_frmsizeenum& fsize)
{
   if ( fsize.type != V4L2_FRMSIZE_TYPE_DISCRETE ) return;

	v4l2_frmivalenum fival; CLEAR(fival);
	fival.index = 0;
	fival.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fival.pixel_format = fsize.pixel_format;
	fival.width = fsize.discrete.width; fival.height = fsize.discrete.height;
	while(ioctl(v4l2_dev, VIDIOC_ENUM_FRAMEINTERVALS, &fival) == 0) {
		if(fival.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
         _V_ ::fprintf(stderr, "        interval discrete: %d / %d seconds\n", fival.discrete.numerator, fival.discrete.denominator);
         if ( IsSupportedFps(fival) ) {
            AddSupport(v4l2_dev, fival);
         }
         else {
            ::fprintf(stderr, "CV4LcapFilter::EnumFPS: interval discrete: %d / %d seconds not supported\n",
                              fival.discrete.numerator, fival.discrete.denominator);
         }
		}
		else if(fival.type == V4L2_FRMIVAL_TYPE_CONTINUOUS) {
         ::fprintf(stderr, "CV4LcapFilter::EnumFPS: interval cont:     %d / %d seconds not supported\n",
                              fival.stepwise.min.numerator, fival.stepwise.min.denominator);
		}
		else if(fival.type == V4L2_FRMIVAL_TYPE_STEPWISE) {
         ::fprintf(stderr, "CV4LcapFilter::EnumFPS: interval step:     %d / %d seconds not supported\n",
                              fival.stepwise.min.numerator, fival.stepwise.min.denominator);
		}
		fival.index++;
	}
}


//!@brief Returns if a PixelFormat (fourCC) is supported or not
//!@return true is supported, otherwise false
bool CV4LcapFilter::IsSupportedPixFormat(const v4l2_fmtdesc& fmt)
{
   return ( (fmt.pixelformat==wxAC_FOURCC_YUY2)
         || (fmt.pixelformat==wxAC_FOURCC_YUYV)
         || (fmt.pixelformat==wxAC_FOURCC_YVYU)
         || (fmt.pixelformat==wxAC_FOURCC_UYVY)
         || (fmt.pixelformat==wxAC_FOURCC_I420)
         || (fmt.pixelformat==wxAC_FOURCC_IYUV)
         || (fmt.pixelformat==wxAC_FOURCC_YU12)
         || (fmt.pixelformat==wxAC_FOURCC_Y800)
         || (fmt.pixelformat==wxAC_FOURCC_GREY)
         || (fmt.pixelformat==wxAC_FOURCC_BY8)
         || (fmt.pixelformat==wxAC_FOURCC_Y16)
         || (fmt.pixelformat==wxAC_FOURCC_YP16)
         );
}


//!@brief Returns if a Frame Size (type, width) is supported or not
//!@return true is supported, otherwise false
bool CV4LcapFilter::IsSupportedFrameSize(const v4l2_frmsizeenum& fsize)
{
   return ( (fsize.type == V4L2_FRMSIZE_TYPE_DISCRETE)  // must be DISCRETE
         && ( fsize.discrete.width>=320  )              // must be larger or equal 320
         && ( fsize.discrete.width<=1280 )              // must be smaller or equal 1280
         );
}

//!@brief Returns if a Frame Period (fps) is supported or not
//!@return true is supported, otherwise false
bool CV4LcapFilter::IsSupportedFps(const v4l2_frmivalenum& fival)
{
   if (fival.discrete.numerator>0) {
      return ( (fival.type == V4L2_FRMIVAL_TYPE_DISCRETE)  // must be DISCRETE
   //         && ( (fival.discrete.denominator/fival.discrete.numerator) <=15.0  ) // must be smaller or equal 15fps
            && ( (fival.discrete.denominator/fival.discrete.numerator) <=60.0  ) // 20100701:BM - try to get most rates
            );
   }
   else
   {
      ::fprintf(stderr, "CV4LcapFilter::IsSupportedFps: device reported FPS numerator==0 (div0) ??\n");
      return false;
   }
}



/////////////////////////////////////////////////////////////////////////////
// CV4LcapFilterItem

// a filter class used as collection item for system enumeration
// used as input for the current capture device of the VCapGraph class
//
CV4LcapFilterItem::CV4LcapFilterItem()
: m_FIndex(-1)
, m_friendlyName(wxT(""))
, m_devPath(wxT(""))
, m_devSerial(wxT(""))
{
}

CV4LcapFilterItem::~CV4LcapFilterItem()
{
	this->Clear();
}

void CV4LcapFilterItem::Clear()
{
	m_friendlyName.Clear();
	m_devPath.Clear();
	m_devSerial.Clear();
	m_FIndex = -1;
}


CV4LcapFilterItem::CV4LcapFilterItem(const CV4LcapFilterItem& other)
: m_FIndex(-1)
, m_friendlyName()
, m_devPath()
, m_devSerial()
{
	this->Clear();
	*this = other;
}


CV4LcapFilterItem& CV4LcapFilterItem::operator= (const CV4LcapFilterItem& other)
{
	this->Clear();

	m_FIndex = other.m_FIndex;
	if (other.m_FIndex<0) return  *this;

	m_friendlyName = other.m_friendlyName;
	m_devSerial = other.m_devSerial;
	m_devPath = other.m_devPath;
	m_fourCC = other.m_fourCC;
	m_width = other.m_width;
	m_height = other.m_height;
	m_num = other.m_num;
	m_denom = other.m_denom;

	return *this;
}


bool CV4LcapFilterItem::operator== (const CV4LcapFilterItem& other) const
{
   // identical is all but the own index
   bool retVal = true;
//	retVal= retVal && (m_FIndex == other.m_FIndex);
	retVal= retVal && (m_friendlyName == other.m_friendlyName);
	retVal= retVal && (m_devSerial == other.m_devSerial);
	retVal= retVal && (m_devPath == other.m_devPath);
	retVal= retVal && (m_fourCC == other.m_fourCC);
	retVal= retVal && (m_width == other.m_width);
	retVal= retVal && (m_height == other.m_height);
	retVal= retVal && (m_num == other.m_num);
	retVal= retVal && (m_denom == other.m_denom);

	return retVal;
}


bool CV4LcapFilterItem::IsSimilar(const CV4LcapFilterItem& other) const
{
   // similar is all but the own index and the fps setting
   bool retVal = true;
//	retVal= retVal && (m_FIndex == other.m_FIndex);
	retVal= retVal && (m_friendlyName == other.m_friendlyName);
	retVal= retVal && (m_devSerial == other.m_devSerial);
	retVal= retVal && (m_devPath == other.m_devPath);
	retVal= retVal && (m_fourCC == other.m_fourCC);
	retVal= retVal && (m_width == other.m_width);
	retVal= retVal && (m_height == other.m_height);

	return retVal;
}

// Sets the friendly name of the filter
void CV4LcapFilterItem::put_FName(const wxString &newVal)
{
	m_friendlyName = newVal;
}

// Sets the device name of the filter
void CV4LcapFilterItem::put_DevPath(const wxString &newVal)
{
	m_devPath = newVal;
}


void CV4LcapFilterItem::put_DevSerial(const wxString &newVal)
{
	m_devSerial = newVal;
}

void CV4LcapFilterItem::put_FIndex(long newVal)
{
	m_FIndex = newVal;
}

void CV4LcapFilterItem::put_PixFormat (wxUint32 fourCC)
{
   m_fourCC=fourCC;
}

void CV4LcapFilterItem::put_FrameSize (wxUint32 width, wxUint32 height)
{
   m_width=width; m_height=height;
}

void CV4LcapFilterItem::put_Fps (wxUint32 num, wxUint32 denom)
{
   m_num=num; m_denom=denom;
}

//!@brief Merge means to overwrite the FPS if it is lower than the existing one
void CV4LcapFilterItem::MergeFps (wxUint32 num, wxUint32 denom)
{
   // paranoid.. if values returned from the device are crazy we get div0 exceptions
   if (m_num<=0) {
      m_num=num; m_denom=denom;
   }
   else if (num<=0) {
      ; // ignore
   }
   else {
      // find lower and return with the lower one i.e.  1/30 vs  1/5 -> 5<30
      float myFps = m_denom/m_num;
      float otherFps = denom/num;
      if (otherFps<myFps) {
         m_num=num; m_denom=denom;
      }
   }
}

/*

Logitech S7500 reports:

EnmuPixFormat: fourCC  0x47504A4D (MJPG)
EnmuPixFormat: fourCC  0x56595559 (YUYV)
    size discrete: 160 x 120 (width x height)
    size discrete: 176 x 144 (width x height)
    size discrete: 320 x 240 (width x height)
        interval discrete: 1 / 30 seconds
        interval discrete: 1 / 25 seconds
        interval discrete: 1 / 20 seconds
        interval discrete: 1 / 15 seconds
        interval discrete: 1 / 10 seconds
        interval discrete: 1 / 5 seconds
    size discrete: 352 x 288 (width x height)
        interval discrete: 1 / 30 seconds
        interval discrete: 1 / 25 seconds
        interval discrete: 1 / 20 seconds
        interval discrete: 1 / 15 seconds
        interval discrete: 1 / 10 seconds
        interval discrete: 1 / 5 seconds
    size discrete: 640 x 480 (width x height)
        interval discrete: 1 / 30 seconds
        interval discrete: 1 / 25 seconds
        interval discrete: 1 / 20 seconds
        interval discrete: 1 / 15 seconds
        interval discrete: 1 / 10 seconds
        interval discrete: 1 / 5 seconds
    size discrete: 960 x 720 (width x height)
        interval discrete: 1 / 10 seconds
        interval discrete: 1 / 5 seconds
    size discrete: 1280 x 960 (width x height)
        interval discrete: 2 / 15 seconds
        interval discrete: 1 / 5 seconds


Philips SPC900NC reports:

EnmuPixFormat: fourCC  0x32435750 (PWC2)
EnmuPixFormat: fourCC  0x32315559 (YU12)
    size discrete: 160 x 120 (width x height)
    size discrete: 320 x 240 (width x height)
        interval discrete: 1 / 5 seconds
        interval discrete: 1 / 10 seconds
        interval discrete: 1 / 15 seconds
        interval discrete: 1 / 20 seconds
        interval discrete: 1 / 25 seconds
        interval discrete: 1 / 30 seconds
    size discrete: 640 x 480 (width x height)
        interval discrete: 1 / 5 seconds
        interval discrete: 1 / 10 seconds
        interval discrete: 1 / 15 seconds

*/

#endif // defined __UNIX__
