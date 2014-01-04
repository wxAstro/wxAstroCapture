//
//

#include "v4l2wcprop.h"

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


#include "../GenericDebHelp.h"


#include <map>
#include <iostream>
using namespace std;

#define CLEAR(x) memset(&(x), 0, sizeof(x))

#ifdef _DEBUG_SWITCHED_OFF
    #define EDUMP(x , n) cerr << (x) << (n) << endl
#else
    #define EDUMP(x , n)
#endif
//P11(0=5,1=10,2=15,3=20,4=25,5=30,6=60)
static int frames[7] =  { 5, 10, 15, 20, 25, 30, 60 };
//static int frames[7] =  { 5, 6, 7, 8, 10, 15, 20 }; // test low FPS

// Helper functions
static int xioctl(int device, __u32 request, void* arg)
{
   // repeat the call to ioctl as long as the previous call was interrupted prematurely
   int r = 0, e = 0;
   do {
     r = ioctl(device,request,arg);  e = errno;
   } while (-1 == r && EINTR == e);

   return r;
}

//!@brief Makes an underscore string from a comma list
static std::string name2var(unsigned char *name)
{
	std::string s;
	int add_underscore = 0;

	while (*name) {
		if (isalnum(*name)) {
			if (add_underscore)
				s += '_';
			add_underscore = 0;
			s += std::string(1, tolower(*name));
		}
		else if (s.length()) add_underscore = 1;
		name++;
	}
	return s;
}


//!@brief Enumerates the supported FPS of the device with its curent state (framsize that is)
static void V4L2EnumFPS(int v4l2_dev, wxUint32 num[], wxUint32 denom[], int& numItems)
{
   wxUint32 tNum[20], tDenom[20];
   numItems=0;
   // retrieve current format, and report to stderr
   v4l2_format fmt; CLEAR(fmt);
   fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // must define what we want to receive
   if(-1 == xioctl(v4l2_dev,VIDIOC_G_FMT,&fmt)) {
      return;
   }

 	v4l2_frmivalenum fival; CLEAR(fival);
	fival.index = 0;
	fival.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fival.pixel_format = fmt.fmt.pix.pixelformat;
	fival.width = fmt.fmt.pix.width; fival.height = fmt.fmt.pix.height;
	while(ioctl(v4l2_dev, VIDIOC_ENUM_FRAMEINTERVALS, &fival) == 0) {
		if(fival.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
         tNum[numItems]=fival.discrete.numerator ; tDenom[numItems]=fival.discrete.denominator;
         numItems++;
		}
	   fival.index++;
	}

	// FPS are delivered shortest first - we expect it slowest firs...
	for (int i=0,j=numItems-1; i<numItems; i++,j--) {
	   num[i]=tNum[j];
	   denom[i]=tDenom[j];
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////

long CProperty::CachedValue(short cindex) const
{
   if ( (cindex<0) || (cindex>=m_cacheNum) ) return 0;
   return m_cache[cindex];
}


//////////////////////////////////////////////////////////////////////////////////////////////////

//! a default Property which returns Nope
class CP_Default : public CProperty
{
public:
    CP_Default(int p_device, EKnownCams p_camType) : CProperty(p_device, p_camType){};
    const EVidCapPropertySupport Has()
    {
        return VCS_no;
    }
	const long Prop()
	{
	    return -999999;
	}
	bool Range(long &pMin, long &pMax, long &pDelta)
	{
	    return false;
	}
	bool put_Prop(long pVal)
	{
	    return false;
	}
};


/// //////////////////////////////////////////////////////////////////////////////////////////////
/// Generic Video4Linux2 Support
/// //////////////////////////////////////////////////////////////////////////////////////////////


//! a partially abstract default Video4Linux2 Property which initializes the V4L2 property
class CV4L2Prop : public CProperty
{
public:
   CV4L2Prop(long V4L2_ID, int p_device, EKnownCams p_camType)
      : CProperty(p_device, p_camType)
   {
      m_isDisabled=false;

      CLEAR(m_qctrl);
      m_qctrl.id = V4L2_ID;
      if( xioctl(m_device,VIDIOC_QUERYCTRL,&m_qctrl) ) {
         m_qctrl.flags |=V4L2_CTRL_FLAG_DISABLED;
         _V_ ::fprintf(stderr, "Unsupported ctrl %d\n", m_qctrl.id);
         m_isDisabled=true;
         return;
      }
      else {
         m_isDisabled=false;
         if (m_qctrl.flags & V4L2_CTRL_FLAG_DISABLED) m_isDisabled=true;
         if (m_qctrl.flags & V4L2_CTRL_FLAG_INACTIVE) m_isDisabled=true;
         ReportCaps();
      }
   };

   virtual const EVidCapPropertySupport Has()=0;
   virtual const long Prop() =0;
   virtual bool Range(long &pMin, long &pMax, long &pDelta)=0;
   virtual bool put_Prop(long pVal)=0;


protected:
   bool           m_isDisabled;

   v4l2_queryctrl m_qctrl; // save the query values

   void ReportCaps()
   {
      _V_ ::fprintf(stderr, "Supported ctrl   %s - id %u - \n",m_qctrl.name, m_qctrl.id);
      switch (m_qctrl.type) {
         case V4L2_CTRL_TYPE_INTEGER:     _V_ ::fprintf(stderr, " - type :  INTEGER \n"); break;
         case V4L2_CTRL_TYPE_BOOLEAN:     _V_ ::fprintf(stderr, " - type :  BOOLEAN \n"); break;
         case V4L2_CTRL_TYPE_MENU:
            _V_ ::fprintf(stderr, " - type :  MENU \n");
            // we should be able to get the range from the control - but this seems not the case...
            v4l2_querymenu qmenu; CLEAR(qmenu);
            qmenu.id=m_qctrl.id;
            qmenu.index=0;
            // so we loop until we bail out
            for (qmenu.index=0; qmenu.index<10; qmenu.index++) {
               if( ! xioctl(m_device,VIDIOC_QUERYMENU,&qmenu) ) {
                  _V_ ::fprintf(stderr, "   - menu item %d  -  %s\n", qmenu.index, qmenu.name);
               }
            }
         break;
         case V4L2_CTRL_TYPE_BUTTON:      _V_ ::fprintf(stderr, " - type :  BUTTON \n"); break;
         case V4L2_CTRL_TYPE_INTEGER64:   _V_ ::fprintf(stderr, " - type :  INTEGER64 \n"); break;
         case V4L2_CTRL_TYPE_CTRL_CLASS:  _V_ ::fprintf(stderr, " - type :  CTRL_CLASS \n"); break;
         default: _V_ ::fprintf(stderr, " - type :  unknown \n");
      }
      _V_ ::fprintf(stderr, " - min %d; max %d; step %d; default %d \n",
               m_qctrl.minimum, m_qctrl.maximum, m_qctrl.step, m_qctrl.default_value);

      _V_ ::fprintf(stderr, " - flags (0x%04x): ", m_qctrl.flags);
      if (m_qctrl.flags & V4L2_CTRL_FLAG_DISABLED) _V_ ::fprintf(stderr, "disabled, ");
      if (m_qctrl.flags & V4L2_CTRL_FLAG_GRABBED) _V_ ::fprintf(stderr, "grabbed, ");
      if (m_qctrl.flags & V4L2_CTRL_FLAG_READ_ONLY) _V_ ::fprintf(stderr, "readonly, ");
      if (m_qctrl.flags & V4L2_CTRL_FLAG_UPDATE) _V_ ::fprintf(stderr, "update, ");
      if (m_qctrl.flags & V4L2_CTRL_FLAG_INACTIVE) _V_ ::fprintf(stderr, "inactive, ");
      if (m_qctrl.flags & V4L2_CTRL_FLAG_SLIDER) _V_ ::fprintf(stderr, "slider, ");
      _V_ ::fprintf(stderr, "\n");


   }

};


/// //////////////////////////////////////////////////////////////////////////////////////////////

//!@brief a generic v4l2 property that can be used with those that are implemented properly ...
class CP_v4l2 : public CV4L2Prop
{
public:
   CP_v4l2(long V4L2_ID, int p_device, EKnownCams p_camType)
      : CV4L2Prop(V4L2_ID, p_device, p_camType){}

   const EVidCapPropertySupport Has()
   {
      if (m_isDisabled)
         return VCS_no;
      else if (m_qctrl.flags & V4L2_CTRL_FLAG_READ_ONLY)
         return VCS_get;
      else if (m_qctrl.flags & V4L2_CTRL_FLAG_GRABBED)
         return VCS_get;
      else {
         if (m_qctrl.type==V4L2_CTRL_TYPE_BUTTON)
            return VCS_set;
         else
            return VCS_both; // finally
      }
   }//Has

   const long Prop()
   {
      if (m_isDisabled) return 1;

      m_lastGet = 0;
      v4l2_control ctrl;            CLEAR(ctrl);
      v4l2_ext_control ext_ctrl;    CLEAR(ext_ctrl);
      v4l2_ext_controls ctrls;      CLEAR(ctrls);

      ext_ctrl.id = m_qctrl.id;
      ctrls.ctrl_class = V4L2_CTRL_ID2CLASS(m_qctrl.id);
      ctrls.count = 1; ctrls.controls = &ext_ctrl;

      if ( (V4L2_CTRL_ID2CLASS(m_qctrl.id) != V4L2_CTRL_CLASS_USER) && (m_qctrl.id < V4L2_CID_PRIVATE_BASE) ) {
         if (xioctl(m_device, VIDIOC_G_EXT_CTRLS, &ctrls)) {
            _V_ ::fprintf(stderr, "getting ext_ctrl %s failed- errno %d %s\n", m_qctrl.name, errno, strerror(errno)); return 0;
         }
      }
      else {
         ctrl.id = m_qctrl.id;
         if (xioctl(m_device, VIDIOC_G_CTRL, &ctrl)) {
            _V_ ::fprintf(stderr, "getting ctrl %s failed- errno %d %s\n", m_qctrl.name, errno, strerror(errno)); return 0;
         }
         ext_ctrl.value = ctrl.value;  // save in extctrl
      }
      m_lastGet=ctrl.value; // save the last retrieved value
      return m_lastGet;

   }//Prop

   bool Range(long &pMin, long &pMax, long &pDelta)
   {
      if (m_isDisabled) return false;

      switch (m_qctrl.type) {
         case V4L2_CTRL_TYPE_INTEGER:
            pMin = m_qctrl.minimum; pMax = m_qctrl.maximum; pDelta= m_qctrl.step;
            break;
         case V4L2_CTRL_TYPE_MENU:
            pMin = m_qctrl.minimum; pMax = m_qctrl.maximum; pDelta= 1;
            break;
         case V4L2_CTRL_TYPE_BOOLEAN:
            pMin = 0; pMax = 1; pDelta= 1;
            break;
         default:
            return false;
      }//switch
      return true;
   }//Range

   bool put_Prop(long pVal)
   {
      if (m_isDisabled) return false;

      v4l2_control ctrl; CLEAR(ctrl);
      v4l2_ext_control ext_ctrl; CLEAR(ext_ctrl);
      v4l2_ext_controls ctrls; CLEAR(ctrls);

      ext_ctrl.id = m_qctrl.id; ext_ctrl.value = pVal;
      ctrls.ctrl_class = V4L2_CTRL_ID2CLASS(m_qctrl.id);
      ctrls.count = 1; ctrls.controls = &ext_ctrl;

      if ( (V4L2_CTRL_ID2CLASS(m_qctrl.id) != V4L2_CTRL_CLASS_USER) && (m_qctrl.id < V4L2_CID_PRIVATE_BASE) ) {
         if (xioctl(m_device, VIDIOC_S_EXT_CTRLS, &ctrls)) {
            ::fprintf(stderr, "setting ext_ctrl %s failed- errno %d %s\n", m_qctrl.name, errno, strerror(errno)); return false;
         }
      }
      else {
         ctrl.id = m_qctrl.id; ctrl.value = ext_ctrl.value;
         if (xioctl(m_device, VIDIOC_S_CTRL, &ctrl)) {
            ::fprintf(stderr, "setting ctrl %s failed- errno %d %s\n", m_qctrl.name, errno, strerror(errno)); return false;
         }
      }
      m_lastSet=pVal; // save the last set value
      return true;
   }//put_prop
};//CP_v4l2


//! the Ext Property AutoExposure
/*  See V4L2 Manual
exposure_auto (menu) : min=0 max=255 default=8 value=8
   1: Manual Mode
   2: Auto Mode
   4: Shutter Priority Mode
   8: Aperture Priority Mode

// So we return 0, 1 as range and translate
    0 into Manual mode, 1 and anything else into Auto Priority Mode

    Pitty is that Manual is sometimes 0 as well....
    So we have to find Manual as well
*/
class CP_V4L2AutoExposure : public CV4L2Prop
{
private:
   long m_Manual;
   long m_Auto;

public:
    CP_V4L2AutoExposure(long V4L2_ID, int p_device, EKnownCams p_camType)
      : CV4L2Prop(V4L2_ID, p_device, p_camType), m_Manual(0), m_Auto(0) {

      m_Auto=m_qctrl.default_value; // first guess

      if ( m_qctrl.type==V4L2_CTRL_TYPE_MENU) {
         // get the menu
         v4l2_querymenu qmenu; CLEAR(qmenu);
         qmenu.id=m_qctrl.id; qmenu.index=0;
         // so we loop until we bail out as we cannot rely on proper driver implementation...
         for (qmenu.index=0; qmenu.index<10; qmenu.index++) {
            if( ! xioctl(m_device,VIDIOC_QUERYMENU,&qmenu) ) {
               // see if we can derive from the name
               std::string mi = name2var(qmenu.name); // returns lowercase
               size_t found = mi.find("manual");
               if (found!=string::npos) {
                  m_Manual = qmenu.index;
                  _V_ ::fprintf(stderr, "CP_V4L2AutoExposure() - using  %d  -  %s as Manual Expo\n", qmenu.index, qmenu.name);
               }
               found = mi.find("aperture");
               if (found!=string::npos) {
                  m_Auto = qmenu.index;
                  _V_ ::fprintf(stderr, "CP_V4L2AutoExposure() - using  %d  -  %s as Auto Expo\n", qmenu.index, qmenu.name);
               }

            }
         }//for
      }
   }

   const EVidCapPropertySupport Has()
   {
      if (m_isDisabled)
         return VCS_no;
      else
         return VCS_both;
   }//Has

   const long Prop()
   {
      if (m_isDisabled) return 1;

      m_lastGet = 0;
      v4l2_control ctrl; CLEAR(ctrl);
      ctrl.id = m_qctrl.id;
      if (xioctl(m_device, VIDIOC_G_CTRL, &ctrl)) {
         ::fprintf(stderr, "getting ctrl %s failed- errno %d %s\n", m_qctrl.name, errno, strerror(errno)); return 0;
      }
      m_lastGet= (ctrl.value==m_Manual) ? 0 : 1;
      return m_lastGet;

   }//Prop

   bool Range(long &pMin, long &pMax, long &pDelta)
   {
      if (m_isDisabled) return false;

      pMin = 0; pMax = 1; pDelta= 1;
      return true;
   }//Range

   bool put_Prop(long pVal)
   {
      if (m_isDisabled) return false;

      v4l2_control ctrl; CLEAR(ctrl);

      ctrl.id = m_qctrl.id;
      ctrl.value = (pVal==0) ? m_Manual : m_Auto;
      if (xioctl(m_device, VIDIOC_S_CTRL, &ctrl)) {
         ::fprintf(stderr, "setting ctrl %s failed- errno %d %s\n", m_qctrl.name, errno, strerror(errno)); return false;
      }
      m_lastSet=pVal; // save the last set value
      return true;
   }//put_prop


};

// the WhiteBalanceMode Property VIDIOCPWC[SG]AWB
// the v4l2 WB property is only implemented poorly so we use PWC specifc
class CP_V4L2WhiteBalanceMode : public CV4L2Prop
{
public:
    CP_V4L2WhiteBalanceMode(long V4L2_ID, int p_device, EKnownCams p_camType)
      : CV4L2Prop(V4L2_ID, p_device, p_camType) {}

   const EVidCapPropertySupport Has()
   {
      if (m_isDisabled)
         return VCS_no;
      else
         return VCS_both;
   }//Has

   const long Prop()
   {
      if (m_isDisabled) return 1;

      m_lastGet = 0;
      v4l2_control ctrl; CLEAR(ctrl);
      ctrl.id = m_qctrl.id;
      if (xioctl(m_device, VIDIOC_G_CTRL, &ctrl)) {
         ::fprintf(stderr, "getting ctrl %s failed- errno %d %s\n", m_qctrl.name, errno, strerror(errno)); return 0;
      }
      m_lastGet= (ctrl.value==0) ? 3 : 4;
      return m_lastGet;

   }//Prop

   bool Range(long &pMin, long &pMax, long &pDelta)
   {
      if (m_isDisabled) return false;

      pMin = 3; pMax = 4; pDelta= 1;  // Manual / Auto
      return true;
   }//Range

   bool put_Prop(long pVal)
   {
      if (m_isDisabled) return false;

      v4l2_control ctrl; CLEAR(ctrl);

      ctrl.id = m_qctrl.id;
      ctrl.value = (pVal==3) ? 0 : 1; // set Auto pVal(1)
      if (xioctl(m_device, VIDIOC_S_CTRL, &ctrl)) {
         ::fprintf(stderr, "setting ctrl %s failed- errno %d %s\n", m_qctrl.name, errno, strerror(errno)); return false;
      }
      m_lastSet=pVal; // save the last set value
      return true;
   }//put_prop
};


//! Map the FPS stream setting into a property
class CP_V4L2FPS : public CProperty
{
private:
   bool m_has; int m_numItems; wxUint32 m_num[20], m_denom[20];
public:
   CP_V4L2FPS(int p_device, EKnownCams p_camType)
   : CProperty(p_device, p_camType)
   , m_has(false)
   , m_numItems(0)
   {
      v4l2_streamparm sPara; CLEAR(sPara);
      sPara.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      if (xioctl(m_device, VIDIOC_G_PARM, &sPara) < 0) {
         ::fprintf(stderr, "VIDIOC_G_PARM failed - errno %d %s\n", errno, strerror(errno));
      }
      else {
         if (sPara.parm.capture.capability & V4L2_CAP_TIMEPERFRAME) {
            m_has=true;
            _V_ ::fprintf(stderr, "CP_V4L2FPS supported\n");
            wxUint32 num   = sPara.parm.capture.timeperframe.numerator;
            wxUint32 denom = sPara.parm.capture.timeperframe.denominator;
            _V_ ::fprintf(stderr, "CP_V4L2FPS actual frame periode %d / %d  seconds\n", num, denom);
            // get a list of supported FPS for the current stream format and size
            V4L2EnumFPS(m_device, m_num, m_denom, m_numItems);
         }
         else {
            _V_ ::fprintf(stderr, "CP_V4L2FPS not supported\n");
         }
      }
   }

   const EVidCapPropertySupport Has()
   {
      return (m_has) ? VCS_both : VCS_no;
   }//Has

	const long Prop()
	{
	   if (!m_has) return 0;
	   if (m_numItems<=0) return 0;

      v4l2_streamparm sPara; CLEAR(sPara);
      sPara.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      if (xioctl(m_device, VIDIOC_G_PARM, &sPara) < 0) {
         ::fprintf(stderr, "VIDIOC_G_PARM failed - errno %d %s\n", errno, strerror(errno));
         return 0;
      }
      wxUint32 num   = sPara.parm.capture.timeperframe.numerator;
      wxUint32 denom = sPara.parm.capture.timeperframe.denominator;
      int fs=-1;
      for (int i=0; i<m_numItems; i++) {
         if (num==m_num[i]) {
            if (denom==m_denom[i]) {
               fs = i;
               break; // for
            }
         }
      }//for
      if (fs != -1) {
          m_lastGet=fs; // save the last retrieved value
          return m_lastGet; //PROPER EXIT
      }
      return 0;
	}//Prop

	bool Range(long &pMin, long &pMax, long &pDelta)
	{
	   if (!m_has) return false;
	   if (m_numItems<=0) return false;

      pMin = 0; pMax = m_numItems-1; pDelta= 1;
      return true;
	}//Range

	bool put_Prop(long pVal)
	{
	   if (!m_has) return false;
	   if (m_numItems<=0) return false;

      pVal=(pVal<0)?0: ((pVal>=m_numItems) ? m_numItems-1 : pVal);

      /// SEND A MSG TO THE CAP FILTER TO CHANGE FPS...
      m_cache[0] = m_num[pVal];
      m_cache[1] = m_denom[pVal];
      m_cacheNum=2;
      // frames[pVal];
      return true;
	}//put_prop
};



/// //////////////////////////////////////////////////////////////////////////////////////////////
//! Specific PWC driver support
//! Some PWC properties handled through the ioctl of PWC
/// //////////////////////////////////////////////////////////////////////////////////////////////

//! this one works around a bug in PWC
class CP_PWCSaturation : public CProperty
{
public:
   CP_PWCSaturation(int p_device, EKnownCams p_camType) : CProperty(p_device, p_camType){};
   const EVidCapPropertySupport Has()
   {
      EVidCapPropertySupport vhas(VCS_no);
      v4l2_queryctrl qctrl;
      qctrl.id = V4L2_CID_SATURATION;
      if(0 == xioctl(m_device,VIDIOC_QUERYCTRL,&qctrl)) {
         vhas=VCS_both;
         if(qctrl.flags&V4L2_CTRL_FLAG_DISABLED)
            vhas =VCS_no;
         return vhas;
      }
      return vhas;
   }//Has
   const long Prop()
   {
      m_lastGet = 0;
      v4l2_control ctrl = { V4L2_CID_SATURATION, 0 };
      if(0 == xioctl(m_device,VIDIOC_G_CTRL,&ctrl)) {
         m_lastGet=ctrl.value; // save the last retrieved value
      }
      return m_lastGet;
   }//Prop
   bool Range(long &pMin, long &pMax, long &pDelta)
   {
      v4l2_queryctrl qctrl;
      qctrl.id = V4L2_CID_SATURATION;
      if(0 == xioctl(m_device,VIDIOC_QUERYCTRL,&qctrl)) {
         pMin = qctrl.minimum; pMax = qctrl.maximum; pDelta= qctrl.step;
         return true;
      }
      return false;
   }//Range
   bool put_Prop(long pVal)
   {
      v4l2_control ctrl = { V4L2_CID_SATURATION, pVal };
      if (0 == xioctl(m_device,VIDIOC_S_CTRL,&ctrl)) {
         m_lastSet=pVal; // save the last set value
         return true;
      }
      return false;
   }//put_prop
};//CP_Saturation


//////////////////////////////////////////////////////////////////////////////////////////////////
// a set of PWC driver specific properties

// the WhiteBalanceMode Property VIDIOCPWC[SG]AWB
// the v4l2 WB property is only implemented poorly so we use PWC specifc
class CP_PWCWhiteBalanceMode : public CProperty
{
public:
    CP_PWCWhiteBalanceMode(int p_device, EKnownCams p_camType) : CProperty(p_device, p_camType){};
    const EVidCapPropertySupport Has()
    {
       EVidCapPropertySupport vhas(VCS_no);
       pwc_whitebalance qctrl;
       if(0 == xioctl(m_device,VIDIOCPWCGAWB,&qctrl)) {
           vhas=VCS_both; // assuming if we can read it it is supported...
       }
	   return vhas;
    }//Has
	const long Prop()
	{
       pwc_whitebalance ctrl;
       if(0 == xioctl(m_device,VIDIOCPWCGAWB,&ctrl)) {
          m_lastGet=ctrl.mode; // save the last retrieved value
          return m_lastGet;
       }
       return 0;
	}//Prop
	bool Range(long &pMin, long &pMax, long &pDelta)
	{
        pMin = PWC_WB_INDOOR; pMax = PWC_WB_AUTO; pDelta= 1;
        return true;
	}//Range
	bool put_Prop(long pVal)
	{
       pVal=(pVal<PWC_WB_INDOOR)? PWC_WB_INDOOR : ((pVal>PWC_WB_AUTO)?PWC_WB_AUTO : pVal); // sanity
       pwc_whitebalance ctrl;
       if(0 == xioctl(m_device,VIDIOCPWCGAWB,&ctrl)) {
           if (ctrl.mode==pVal) {
               return true;  // already there
           }
           if (pVal==PWC_WB_MANUAL) {
               // we should have access to the manual settings
               // to apply them - we got only meaningful values when
               // switching from Auto to Manual
               // so as workaround we switch to auto shortly then to manual
               ctrl.mode = PWC_WB_AUTO;
               xioctl(m_device,VIDIOCPWCSAWB,&ctrl);
               // get the auto replied values
               xioctl(m_device,VIDIOCPWCGAWB,&ctrl);
               ctrl.manual_red=ctrl.read_red;
               ctrl.manual_blue=ctrl.read_blue;
           }
           // now set the desired mode
           ctrl.mode=pVal;
           if(0 == xioctl(m_device,VIDIOCPWCSAWB,&ctrl)) {
              m_lastSet = pVal;
              return true;
           }
       }
       return false;
	}//put_prop
};//CP_WhiteBalanceMode


// the WhiteBalRed Property VVIDIOCPWC[SG]AWB range 0..65535
class CP_PWCWhiteBalRed : public CProperty
{
public:
    CP_PWCWhiteBalRed(int p_device, EKnownCams p_camType) : CProperty(p_device, p_camType){};
    const EVidCapPropertySupport Has()
    {
       EVidCapPropertySupport vhas(VCS_no);
       pwc_whitebalance qctrl;
       if(0 == xioctl(m_device,VIDIOCPWCGAWB,&qctrl)) {
           vhas=VCS_both; // assuming if we can read it it is supported...
       }
	   return vhas;
    }//Has
	const long Prop()
	{
       pwc_whitebalance ctrl;
       if(0 == xioctl(m_device,VIDIOCPWCGAWB,&ctrl)) {
          m_lastGet=(ctrl.mode==PWC_WB_MANUAL) ? ctrl.manual_red : ctrl.read_red; // save the last retrieved value
          return m_lastGet;
       }
       return 0;
	}//Prop
	bool Range(long &pMin, long &pMax, long &pDelta)
	{
        pMin = 0; pMax = 65535; pDelta= 1;
        return true;
	}//Range
	bool put_Prop(long pVal)
	{
       pVal=(pVal<0)?0:pVal; // sanity
       pwc_whitebalance ctrl;
       if(0 == xioctl(m_device,VIDIOCPWCGAWB,&ctrl)) {
            if (ctrl.mode ==PWC_WB_MANUAL)
            {// can set only in manual mode
                ctrl.manual_red = pVal;
                if(0 == xioctl(m_device,VIDIOCPWCSAWB,&ctrl)) {
                    m_lastSet = pVal;
                    return true;
                }
            }
       }
       return false;
	}//put_prop
};//CP_WhiteBalRed


// the WhiteBalBlue Property VIDIOCPWC[SG]AWB range 0..65535
class CP_PWCWhiteBalBlue : public CProperty
{
public:
    CP_PWCWhiteBalBlue(int p_device, EKnownCams p_camType) : CProperty(p_device, p_camType){};
    const EVidCapPropertySupport Has()
    {
       EVidCapPropertySupport vhas(VCS_no);
       pwc_whitebalance qctrl;
       if(0 == xioctl(m_device,VIDIOCPWCGAWB,&qctrl)) {
           vhas=VCS_both; // assuming if we can read it it is supported...
       }
	   return vhas;
    }//Has
	const long Prop()
	{
       pwc_whitebalance ctrl;
       if(0 == xioctl(m_device,VIDIOCPWCGAWB,&ctrl)) {
          m_lastGet=(ctrl.mode==PWC_WB_MANUAL) ? ctrl.manual_blue : ctrl.read_blue; // save the last retrieved value
          return m_lastGet;
       }
       return 0;
	}//Prop
	bool Range(long &pMin, long &pMax, long &pDelta)
	{
        pMin = 0; pMax = 65535; pDelta= 1;
        return true;
	}//Range
	bool put_Prop(long pVal)
	{
       pVal=(pVal<0)?0:pVal; // sanity
       pwc_whitebalance ctrl;
       if(0 == xioctl(m_device,VIDIOCPWCGAWB,&ctrl)) {
            if (ctrl.mode ==PWC_WB_MANUAL)
            {// can set only in manual mode
                ctrl.manual_blue = pVal;
                if(0 == xioctl(m_device,VIDIOCPWCSAWB,&ctrl)) {
                    m_lastSet = pVal;
                    return true;
                }
            }
       }
       return false;
	}//put_prop
};


// the WhiteBalBlue Property VIDIOCPWC[SG]AWB range 0..65535
class CP_Contour : public CProperty
{
public:
    CP_Contour(int p_device, EKnownCams p_camType) : CProperty(p_device, p_camType){};
    const EVidCapPropertySupport Has()
    {
       EVidCapPropertySupport vhas(VCS_no);
       int qctrl;
       if(0 == xioctl(m_device,VIDIOCPWCGCONTOUR,&qctrl)) {
           vhas=VCS_both; // assuming if we can read it it is supported...
       }
	   return vhas;
    }//Has
	const long Prop()
	{
       int ctrl;
       if(0 == xioctl(m_device,VIDIOCPWCGCONTOUR,&ctrl)) {
           // replies neg numbers if in auto mode
          m_lastGet=(ctrl<0) ? 0 : ctrl ; // save the last retrieved value
          return m_lastGet;
       }
       return 0;
	}//Prop
	bool Range(long &pMin, long &pMax, long &pDelta)
	{
        pMin = 0; pMax = 65535; pDelta= 1;
        return true;
	}//Range
	bool put_Prop(long pVal)
	{
       int ctrl = (pVal<0) ? 0:pVal;
       if (0 == xioctl(m_device,VIDIOCPWCSCONTOUR,&ctrl)) {
           m_lastSet=(pVal<0) ? 0:pVal;
           return true;
       }
       return false;
	}//put_prop
};//CP_Contour


// the Exposure Property V4L2_CID_EXPOSURE  or VIDIOCPWCSSHUTTER
// read is not supported according to driver code
class CP_PWCExposure : public CProperty
{
public:
    CP_PWCExposure(int p_device, EKnownCams p_camType) : CProperty(p_device, p_camType){};
    const EVidCapPropertySupport Has()
    {
       EVidCapPropertySupport vhas(VCS_no);
       v4l2_queryctrl qctrl;
       qctrl.id = V4L2_CID_EXPOSURE;
       if(0 == xioctl(m_device,VIDIOC_QUERYCTRL,&qctrl)) {
           vhas=VCS_both;
           if(qctrl.flags&V4L2_CTRL_FLAG_DISABLED)vhas =VCS_no;
           return vhas;
       }
	   return vhas;
    }//Has
	const long Prop()
	{
	    // does not support read, we send the last known back
       return m_lastSet;
	}//Prop
	bool Range(long &pMin, long &pMax, long &pDelta)
	{
	    //P08(0..10 / 0=1/5, 1=1/10, 2=1/25...10=1/10000)  Win driver compatible
        pMin = 0; pMax = 10; pDelta= 1;
        return true;
	}//Range
	bool put_Prop(long pVal)
	{
       int ctrl = (pVal<0) ? 0 : (pVal>10) ? 10 : pVal;
       int speed = (65535 / 10) * (10-pVal);
       EDUMP("set EXPO: ", speed);
       if (0 == xioctl(m_device,VIDIOCPWCSSHUTTER,&speed)) {
           m_lastSet=ctrl;
           return true;
       }
       return false;
	}//put_prop
};


// the AutoExposure Property -- Exposure <0 AND Gain<0 !!??
// read is not supported according to driver code
class CP_PWCAutoExposure : public CProperty
{
public:
    CP_PWCAutoExposure(int p_device, EKnownCams p_camType) : CProperty(p_device, p_camType){};
    const EVidCapPropertySupport Has()
    {
       EVidCapPropertySupport vhas(VCS_no);
       v4l2_queryctrl qctrl;
       qctrl.id = V4L2_CID_EXPOSURE;
       if(0 == xioctl(m_device,VIDIOC_QUERYCTRL,&qctrl)) {
           vhas=VCS_both;
           if(qctrl.flags&V4L2_CTRL_FLAG_DISABLED) vhas =VCS_no;
       }
	   return vhas;
    }//Has
	const long Prop()
	{
	    // we just reply wether gain is in auto mode (expo is WO)
       int ctrl;
       if(0 == xioctl(m_device,VIDIOCPWCGAGC,&ctrl)) {
          m_lastGet=(ctrl<0) ? 1 : 0 ;
          return m_lastGet;
       }
       return 0;
	}//Prop
	bool Range(long &pMin, long &pMax, long &pDelta)
	{
       v4l2_queryctrl qctrl;
       qctrl.id = V4L2_CID_EXPOSURE;
       if(0 == xioctl(m_device,VIDIOC_QUERYCTRL,&qctrl)) {
          pMin = 0; pMax = 1; pDelta= 1;
          return true;
       }
	    return false;
	}//Range
	bool put_Prop(long pVal)
	{
	    // we should now the Gain and Expo setting before Auto mode hmmm
	    // set Gain
       int ctrl = 0;
       // read Gain
       if(0 == xioctl(m_device,VIDIOCPWCGAGC,&ctrl)) {
           if (m_cameraType==EKC_PWC_ToUCam) {
                ctrl = (ctrl<0)? 65535+ctrl : ctrl; // make positive
                ctrl = ((pVal>0) ? ctrl-65535 : ctrl); // set negative if Auto is requested
           }
           else {
                ctrl = (ctrl<0)? -1*ctrl : ctrl; // make positive
                ctrl = ((pVal>0) ? -1*ctrl-1 : ctrl); // set negative if Auto is requested
           }
           if (0 == xioctl(m_device,VIDIOCPWCSAGC,&ctrl)) {
               // the set Exposure (we dont know the prev. value
               ctrl = ((pVal>0) ? -1 :65535); // set to slowest
               if (0 == xioctl(m_device,VIDIOCPWCSSHUTTER,&ctrl)) {
                   m_lastSet=(pVal>0) ? 1 : 0;
                   return true;
               }
           }
       }
       return false;
	}//put_prop
};



// the Gain Property V4L2_CID_GAIN
class CP_PWCGain : public CProperty
{
public:
    CP_PWCGain(int p_device, EKnownCams p_camType) : CProperty(p_device, p_camType){};
    const EVidCapPropertySupport Has()
    {
       EVidCapPropertySupport vhas(VCS_no);
       v4l2_queryctrl qctrl;
       qctrl.id = V4L2_CID_GAIN;
       if(0 == xioctl(m_device,VIDIOC_QUERYCTRL,&qctrl)) {
           vhas=VCS_both;
           if(qctrl.flags&V4L2_CTRL_FLAG_DISABLED)vhas =VCS_no;
           return vhas;
       }
	   return vhas;
    }//Has
	const long Prop()
	{
       int ctrl;
       if(0 == xioctl(m_device,VIDIOCPWCGAGC,&ctrl)) {
           // replies neg numbers if in auto mode
          if (m_cameraType==EKC_PWC_ToUCam) {
            m_lastGet=(ctrl<0) ? 65535+ctrl : ctrl ; // save the last retrieved value
          }
          else {
            m_lastGet=(ctrl<0) ? -1*ctrl : ctrl ; // save the last retrieved value
          }
          return m_lastGet;
       }
       return 0;
	}//Prop
	bool Range(long &pMin, long &pMax, long &pDelta)
	{
         pMin = 0; pMax = 65535; pDelta= 1; // eff 65500
         return true;
	}//Range
	bool put_Prop(long pVal)
	{
       int ctrl = (pVal<0) ? 0:pVal; // make sure we do not switch to auto
       EDUMP("set GAIN: ", ctrl);
       if (0 == xioctl(m_device,VIDIOCPWCSAGC,&ctrl)) {
           m_lastSet=(pVal<0) ? 0:pVal;
           return true;
       }
       return false;
	}//put_prop
};



// TODO the FPS Property VIDIOC[SG]WIN
class CP_PWCFPS : public CProperty
{
public:
    CP_PWCFPS(int p_device, EKnownCams p_camType) : CProperty(p_device, p_camType){};
    const EVidCapPropertySupport Has()
    {
       EVidCapPropertySupport vhas(VCS_no);
       struct video_window vwin;
       memset(&vwin, 0, sizeof(vwin)); // clear structure
       if (xioctl(m_device, VIDIOCGWIN, &vwin) < 0) {
       }
       else {
           vhas = VCS_both;
       }
	   return vhas;
    }//Has
	const long Prop()
	{
        struct video_window vwin; int i, fs, nfs, diff, dev;
        memset(&vwin, 0, sizeof(vwin)); // clear structure
        if (xioctl(m_device, VIDIOCGWIN, &vwin) < 0) {
            return -1;
        }
        if (vwin.flags & PWC_FPS_FRMASK) {
            int ifs = (vwin.flags & PWC_FPS_FRMASK) >> PWC_FPS_SHIFT;
            fs = -1; nfs = -1; dev = 9999;
            for (i = 0; i < 7; i++) {
                diff = abs(frames[i] - ifs);
                if (diff < dev) { // Closest so far...
                    dev = diff; nfs = frames[i]; fs = i;
                }
            }
            if (fs != -1) {
                m_lastGet=fs; // save the last retrieved value
                return m_lastGet; //PROPER EXIT
            }
        }//if
        return 0;
	}//Prop
	bool Range(long &pMin, long &pMax, long &pDelta)
	{
        pMin = 0; pMax = 5; pDelta= 1;
//        pMin = 0; pMax = 2; pDelta= 1;  //20080601:BM  limit FPS to 5,10,15
        return true;
	}//Range
	bool put_Prop(long pVal)
	{
	    bool retVal = true;
	    pVal=(pVal<0)?0: ((pVal>5)?5:pVal); // sanity
//	    pVal=(pVal<0)?0: ((pVal>1)?2:pVal);  //20080601:BM  limit FPS to 5,10,15
        struct video_window vwin;
        memset(&vwin, 0, sizeof(vwin)); // clear structure
        if (xioctl(m_device, VIDIOCGWIN, &vwin) < 0) {
           return false;
        }
        vwin.flags &= ~PWC_FPS_MASK; // clear FPS bits
        vwin.flags |= ((frames[pVal] << PWC_FPS_SHIFT) & PWC_FPS_FRMASK);

        if (xioctl(m_device, VIDIOCSWIN, &vwin) < 0) {
            retVal=false;
        }
        return retVal;
	}//put_prop
};


// the ActualFrameRate Property VIDIOCGWIN (vwin.flags & PWC_FPS_FRMASK) >> PWC_FPS_SHIFT
class CP_PWCActualFrameRate : public CProperty
{
public:
    CP_PWCActualFrameRate(int p_device, EKnownCams p_camType) : CProperty(p_device, p_camType){};
    const EVidCapPropertySupport Has()
    {
       EVidCapPropertySupport vhas(VCS_no);
       struct video_window vwin;
       memset(&vwin, 0, sizeof(vwin)); // clear structure
       if (xioctl(m_device, VIDIOCGWIN, &vwin) < 0) {
       }
       else {
           vhas = VCS_get;
       }
	   return vhas;
    }//Has
	const long Prop()
	{
       struct video_window vwin;

        memset(&vwin, 0, sizeof(vwin)); // clear structure
        if (xioctl(m_device, VIDIOCGWIN, &vwin) < 0) {
            return -1;
        }
        else {
            m_lastGet=(vwin.flags & PWC_FPS_FRMASK) >> PWC_FPS_SHIFT;
            return m_lastGet;
        }//if
        return 0;
	}//Prop
	bool Range(long &pMin, long &pMax, long &pDelta)
	{
        pMin = 0; pMax = 0; pDelta= 0;
        return true;
	}//Range
	bool put_Prop(long pVal)
	{
       return false;
	}//put_prop
};


// the LED Property VIDIOCPWCxLED range = 0..25000
class CP_PWCLED : public CProperty
{
public:
    CP_PWCLED(int p_device, EKnownCams p_camType) : CProperty(p_device, p_camType){};
    const EVidCapPropertySupport Has()
    {
       EVidCapPropertySupport vhas(VCS_no);
       pwc_leds qctrl = {0,0};
       if(0 == xioctl(m_device,VIDIOCPWCGLED,&qctrl)) {
           vhas=VCS_set; // assuming if we can read it it is supported...
       }
	   return vhas;
    }//Has
	const long Prop()
	{
       return m_lastGet; // RO
	}//Prop
	bool Range(long &pMin, long &pMax, long &pDelta)
	{
        pMin = 16; pMax = 18; pDelta= 1;
        //16 on, =17 slow blink, =18 fast blink --> Win compatible
        return true;
	}//Range
	bool put_Prop(long pVal)
	{
       pwc_leds ctrl = { 0, 0 };
       switch (pVal) {
           case 17: ctrl.led_on=25000; ctrl.led_off=25000; break; // slow
           case 18: ctrl.led_on=10000; ctrl.led_off=10000; break; // fast
           case 16:
           default: ctrl.led_on=25000; ctrl.led_off=0; break; // on
       }//switch
       if (0 == xioctl(m_device,VIDIOC_S_CTRL,&ctrl)){
           m_lastSet=pVal;
           return true;
       }
       return false;
	}//put_prop
};


// the Setting Property V4L2_CID_PRIVATE_SAVE_USER, RESTORE_USER, RESTORE_FACTORY
class CP_PWCSetting : public CProperty
{
public:
    CP_PWCSetting(int p_device, EKnownCams p_camType) : CProperty(p_device, p_camType){};
    const EVidCapPropertySupport Has()
    {
	   return VCS_set; // may be we have to check for the cam type
    }//Has
	const long Prop()
	{
       return m_lastGet; // RO
	}//Prop
	bool Range(long &pMin, long &pMax, long &pDelta)
	{
        pMin = 0; pMax = 2; pDelta= 1;
        // =0 Load User, =1 Save User, =2 Load Factory  - Win compatible
        return true;
	}//Range
	bool put_Prop(long pVal)
	{
	    v4l2_control ctrl = {0,1};
	    switch (pVal) {
	        case 0: ctrl.id = V4L2_CID_PRIVATE_RESTORE_USER; break;
	        case 1: ctrl.id = V4L2_CID_PRIVATE_SAVE_USER; break;
	        case 2: ctrl.id = V4L2_CID_PRIVATE_RESTORE_FACTORY; break;
	        default:
                return false;
	    }//switch
       if (0 == xioctl(m_device,VIDIOC_S_CTRL,&ctrl)) {
           m_lastSet=pVal;
           return true;
       }
       return false;
	}//put_prop
};

// END of PWC specific
//////////////////////////////////////////////////////////////////////////////////////////////////






///////////////////////////////////////////////////////////////////////////////////////
// class  CV4LwcProp  Implementation
//
CV4LwcProp::CV4LwcProp()
: m_device(0)
, m_cameraType(CProperty::EKC_NoCam)
{
    // proper init
    for (int i=VCP_FirstElement; i<=VCP_LastElement; i++)
    {
        m_propSet[i].propObj=NULL;
        m_propSet[i].propID =VCP_LastElement1; // invalid ID
    }
}

CV4LwcProp::~CV4LwcProp()
{
    KillPropSet();
}

// Deletes the propObjs which were instantiated
void CV4LwcProp::KillPropSet()
{
    for (int i=VCP_FirstElement; i<=VCP_LastElement; i++)
    {
        if (m_propSet[i].propObj)
            delete m_propSet[i].propObj;
        m_propSet[i].propObj=NULL;
        m_propSet[i].propID =VCP_LastElement1; // invalid ID
    }
}



void CV4LwcProp::put_CaptureDevice(int p_device)
{
   // first get rid of the past
   KillPropSet();
   // set the current device
   m_device = p_device;
   m_cameraType = CProperty::EKC_NoCam;

   if (m_device!=-1) {
      // query the controls available - this usually works for UVC but not for PWC devices :-(
      v4l2_queryctrl qctrl; CLEAR(qctrl);
      qctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;

      typedef map<string, unsigned> ctrl_strmap;
      ctrl_strmap ctrl_str2id;

      while (xioctl(m_device, VIDIOC_QUERYCTRL, &qctrl) == 0) {
         ctrl_str2id[name2var(qctrl.name)] = qctrl.id; // add to map
         _V_ ::fprintf(stderr, "query ctrl %d: %s \n", qctrl.id, name2var(qctrl.name).c_str());
         qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
      }

      // second add specific stuff from devices we know more about
      // check if this is a Philips chip based camera
      v4l2_capability cap;
      if(0 == xioctl(m_device,VIDIOC_QUERYCAP,&cap)) {
         wxString cap_name((const char*)cap.driver, wxConvLibc);
         if(cap_name.Find(wxT("pwc"))!=-1) {
            // PWC driver confirmed !
            /*
               A Philips SPC900 (PWC driver) returns about this property set
               user ctrl 9963776:                      brightness (int)  : min=0 max=128 step=1 default=64 value=63
               user ctrl 9963777:                        contrast (int)  : min=0 max=64 step=1 default=0 value=29
               user ctrl 9963778:                      saturation (int)  : min=-100 max=100 step=1 default=0 value=0
               user ctrl 9963788:              auto_white_balance (bool) : default=0 value=0
               user ctrl 9963790:                        red_gain (int)  : min=0 max=256 step=1 default=0 value=63
               user ctrl 9963791:                       blue_gain (int)  : min=0 max=256 step=1 default=0 value=127
               user ctrl 9963792:                           gamma (int)  : min=0 max=32 step=1 default=0 value=0
               user ctrl 9963793:          shutter_speed_exposure (int)  : min=0 max=256 step=1 default=200 value=236
               user ctrl 9963794:               auto_gain_enabled (bool) : default=1 value=1
               user ctrl 9963795:                      gain_level (int)  : min=0 max=256 step=1 default=0 value=-9
               priv ctrl 134217731:                     colour_mode (bool) : default=0 value=1
               priv ctrl 134217732:                    auto_contour (bool) : default=0 value=0
               priv ctrl 134217733:                         contour (int)  : min=0 max=63 step=1 default=0 value=0
               priv ctrl 134217734:          backlight_compensation (bool) : default=0 value=0
               priv ctrl 134217735:                     flickerless (bool) : default=0 value=0
               priv ctrl 134217736:                 noise_reduction (int)  : min=0 max=3 step=1 default=0 value=0
            */
            /// The Philips cam does not enumerate the controls
            ///  so we have to assign each one by ID

            m_cameraType = CProperty::EKC_PWC_Generic;
            pwc_probe probe;
            if(0 == xioctl(m_device,VIDIOCPWCPROBE,&probe)) {
               wxString probe_name((const char*)probe.name, wxConvLibc);
               switch (probe.type) {
                  case 675:
                  case 680:
                  case 690:
                      m_cameraType = CProperty::EKC_PWC_Vesta;
                      break;
                  case 720:
                      if(probe_name.Find(wxT("SPC 900"))!=-1) {
                       // Old Driver bug -SPC900 is a 740 NOT 720 type
                          m_cameraType = CProperty::EKC_PWC_ToUCam;
                      }
                      break;
                  case 740:
                  case 750:
                      m_cameraType = CProperty::EKC_PWC_ToUCam;
                      break;
                  default: ;
               }//switch
            }//if PWC probe

            // add PWC specifics
            m_propSet[VCP_Brightness].propObj=new CP_v4l2(V4L2_CID_BRIGHTNESS, m_device, m_cameraType);
            m_propSet[VCP_Contrast].propObj=new CP_v4l2(V4L2_CID_CONTRAST, m_device, m_cameraType);
            m_propSet[VCP_Hue].propObj=new CP_v4l2(V4L2_CID_HUE, m_device, m_cameraType);
            m_propSet[VCP_Gamma].propObj=new CP_v4l2(V4L2_CID_GAMMA, m_device, m_cameraType);
            m_propSet[VCP_FlipHorizontal].propObj=new CP_v4l2(V4L2_CID_HFLIP, m_device, m_cameraType);
            m_propSet[VCP_FlipVertical].propObj=new CP_v4l2(V4L2_CID_VFLIP, m_device, m_cameraType);
            // this one goes around a bug in PWC
            m_propSet[VCP_Saturation].propObj=new CP_PWCSaturation(m_device, m_cameraType);
            m_propSet[VCP_ColorEnable].propObj=new CP_v4l2(V4L2_CID_PRIVATE_COLOUR_MODE, m_device, m_cameraType);
            m_propSet[VCP_Sharpness].propObj=new CP_Contour(m_device, m_cameraType);
            // the WhiteBalance Property n.a.
            m_propSet[VCP_BacklightCompensation].propObj=new CP_v4l2(V4L2_CID_PRIVATE_BACKLIGHT, m_device, m_cameraType);
            m_propSet[VCP_WhiteBalanceMode].propObj=new CP_PWCWhiteBalanceMode(m_device, m_cameraType);
            m_propSet[VCP_WhiteBalRed].propObj=new CP_PWCWhiteBalRed(m_device, m_cameraType);
            m_propSet[VCP_WhiteBalBlue].propObj=new CP_PWCWhiteBalBlue(m_device, m_cameraType);
            m_propSet[VCP_FlickerFree].propObj=new CP_v4l2(V4L2_CID_PRIVATE_FLICKERLESS, m_device, m_cameraType);
            // Flicker50_60 n.a.
            m_propSet[VCP_FPS].propObj=new CP_PWCFPS(m_device, m_cameraType);
            m_propSet[VCP_Exposure].propObj=new CP_PWCExposure(m_device, m_cameraType);
            m_propSet[VCP_Gain].propObj=new CP_PWCGain(m_device, m_cameraType);
            m_propSet[VCP_AutoExposure].propObj=new CP_PWCAutoExposure(m_device, m_cameraType);
            // the AutoAll Property n.a.
            m_propSet[VCP_ActualFrameRate].propObj=new CP_PWCActualFrameRate(m_device, m_cameraType);
            m_propSet[VCP_LED].propObj=new CP_PWCLED(m_device, m_cameraType);
            m_propSet[VCP_Setting].propObj=new CP_PWCSetting(m_device, m_cameraType);
            // the ExtTrigger Property n.a.
            // the SWTrigger Property n.a.
            // .. orbit etc. probes may come later
            // the Focus Property n.a.
         }//if PWC driver
         else {
            // other driver tbd
             /*
               A Logitec S7500 (UVC driver) returns about this property set
               query ctrl 9963776:                      brightness (int)  : min=0 max=255 step=1 default=144 value=140
               query ctrl 9963777:                        contrast (int)  : min=0 max=255 step=1 default=27 value=25
               query ctrl 9963778:                      saturation (int)  : min=0 max=255 step=1 default=28 value=28
               query ctrl 9963795:                            gain (int)  : min=0 max=255 step=1 default=0 value=0
               query ctrl 134217728:          backlight_compensation (int)  : min=0 max=2 step=1 default=1 value=1
               query ctrl 134217729:            power_line_frequency (menu) : min=0 max=2 default=2 value=2
                                               0: Disabled
                                               1: 50 Hz
                                               2: 60 Hz
               query ctrl 134217730:                       sharpness (int)  : min=0 max=255 step=1 default=180 value=180
               query ctrl 134217738:                   exposure_auto (menu) : min=0 max=255 default=8 value=8
                                               1: Manual Mode
                                               2: Auto Mode
                                               4: Shutter Priority Mode
                                               8: Aperture Priority Mode
               query ctrl 134217739:               exposure_absolute (int)  : min=1 max=10000 step=1 default=166 value=166
               query ctrl 134217740:  white_balance_temperature_auto (bool) : default=1 value=0
               query ctrl 134217741:       white_balance_temperature (int)  : min=0 max=10000 step=10 default=4000 value=2519
               query ctrl 134217742:          exposure_auto_priority (bool) : default=4000 value=1
            */

            // Assign the V4L2 props that are cam independent
            // for EKC_Generic and all others
            m_cameraType = CProperty::EKC_Generic;

            // assign all known props
            ctrl_strmap::const_iterator ctrl_strmapCIT;
            ctrl_strmapCIT = ctrl_str2id.find(string("brightness"));
            if (ctrl_strmapCIT != ctrl_str2id.end())
               m_propSet[VCP_Brightness].propObj=new CP_v4l2(ctrl_strmapCIT->second, m_device, m_cameraType);

            ctrl_strmapCIT = ctrl_str2id.find(string("contrast"));
            if (ctrl_strmapCIT != ctrl_str2id.end())
               m_propSet[VCP_Contrast].propObj=new CP_v4l2(ctrl_strmapCIT->second, m_device, m_cameraType);

            ctrl_strmapCIT = ctrl_str2id.find(string("hue"));
            if (ctrl_strmapCIT != ctrl_str2id.end())
               m_propSet[VCP_Hue].propObj=new CP_v4l2(ctrl_strmapCIT->second, m_device, m_cameraType);

            ctrl_strmapCIT = ctrl_str2id.find(string("gamma"));
            if (ctrl_strmapCIT != ctrl_str2id.end())
               m_propSet[VCP_Gamma].propObj=new CP_v4l2(ctrl_strmapCIT->second, m_device, m_cameraType);

            m_propSet[VCP_FlipHorizontal].propObj=new CP_v4l2(V4L2_CID_HFLIP, m_device, m_cameraType);
            m_propSet[VCP_FlipVertical].propObj=new CP_v4l2(V4L2_CID_VFLIP, m_device, m_cameraType);
            ctrl_strmapCIT = ctrl_str2id.find(string("sharpness"));
            if (ctrl_strmapCIT != ctrl_str2id.end())
               m_propSet[VCP_Sharpness].propObj=new CP_v4l2(ctrl_strmapCIT->second, m_device, m_cameraType);

            ctrl_strmapCIT = ctrl_str2id.find(string("saturation"));
            if (ctrl_strmapCIT != ctrl_str2id.end())
               m_propSet[VCP_Saturation].propObj=new CP_v4l2(ctrl_strmapCIT->second, m_device, m_cameraType);

            ctrl_strmapCIT = ctrl_str2id.find(string("gain"));
            if (ctrl_strmapCIT != ctrl_str2id.end())
               m_propSet[VCP_Gain].propObj=new CP_v4l2(ctrl_strmapCIT->second, m_device, m_cameraType);

            ctrl_strmapCIT = ctrl_str2id.find(string("backlight_compensation"));
            if (ctrl_strmapCIT != ctrl_str2id.end())
               m_propSet[VCP_BacklightCompensation].propObj=new CP_v4l2(ctrl_strmapCIT->second, m_device, m_cameraType);

            ctrl_strmapCIT = ctrl_str2id.find(string("exposure_auto"));
            if (ctrl_strmapCIT != ctrl_str2id.end())
               m_propSet[VCP_AutoExposure].propObj=new CP_V4L2AutoExposure(ctrl_strmapCIT->second, m_device, m_cameraType);

            ctrl_strmapCIT = ctrl_str2id.find(string("exposure_absolute"));
            if (ctrl_strmapCIT != ctrl_str2id.end())
               m_propSet[VCP_Exposure].propObj=new CP_v4l2(ctrl_strmapCIT->second, m_device, m_cameraType);

            ctrl_strmapCIT = ctrl_str2id.find(string("white_balance_temperature_auto"));
            if (ctrl_strmapCIT != ctrl_str2id.end())
               m_propSet[VCP_WhiteBalanceMode].propObj=new CP_V4L2WhiteBalanceMode(ctrl_strmapCIT->second, m_device, m_cameraType);

            ctrl_strmapCIT = ctrl_str2id.find(string("white_balance_temperature"));
            if (ctrl_strmapCIT != ctrl_str2id.end())
               m_propSet[VCP_WhiteBalRed].propObj=new CP_v4l2(ctrl_strmapCIT->second, m_device, m_cameraType);

            m_propSet[VCP_FPS].propObj=new CP_V4L2FPS(m_device, m_cameraType);


//            ctrl_strmapCIT = ctrl_str2id.find(string("exposure_auto_priority"));
//            if (ctrl_strmapCIT != ctrl_str2id.end())
//               m_propSet[VCP_BacklightCompensation].propObj=new CP_v4l2(ctrl_strmapCIT->second, m_device, m_cameraType);
         }
      }//if having v4l2 capsS

   }//if having a valid device

   // set all unoccupied slots to CP_Default and set all propIDs
   for (int i=VCP_FirstElement; i<VCP_LastElement1; i++) {
      if (!m_propSet[i].propObj) m_propSet[i].propObj = new CP_Default(m_device, CProperty::EKC_NoCam);
      m_propSet[i].propID = (EVidCapProperty)i;
   }
}//put_CaptureDevice


// replies about the capabilities of a Property
const EVidCapPropertySupport CV4LwcProp::Has(EVidCapProperty Property)
{
    if ( (Property<0) || (Property>=VCP_LastElement1) )
        return VCS_no;
    return m_propSet[Property].propObj->Has();
}//Has


const long CV4LwcProp::Prop(EVidCapProperty Property) const
{
    if ( (Property<0) || (Property>=VCP_LastElement1) )
        return -99999;
    return m_propSet[Property].propObj->Prop();
}//Prop


bool CV4LwcProp::Range(EVidCapProperty Property, long &pMin, long &pMax, long &pDelta)
{
    if ( (Property<0) || (Property>=VCP_LastElement1) )
        return false;
    return m_propSet[Property].propObj->Range(pMin, pMax, pDelta);
}//Range



bool CV4LwcProp::put_Prop(EVidCapProperty Property, long pVal)
{
    if ( (Property<0) || (Property>=VCP_LastElement1) )
        return false;
    return m_propSet[Property].propObj->put_Prop(pVal);
}//put_Prop

long CV4LwcProp::CachedValue(EVidCapProperty Property, short cindex) const
{
    if ( (Property<0) || (Property>=VCP_LastElement1) )
        return 0;
    return m_propSet[Property].propObj->CachedValue(cindex);
}

bool CV4LwcProp::HasCache(EVidCapProperty Property) const
{
    if ( (Property<0) || (Property>=VCP_LastElement1) )
        return false;
    return m_propSet[Property].propObj->HasCache();
}


#endif
