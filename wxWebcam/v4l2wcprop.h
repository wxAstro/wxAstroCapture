//
//

#ifndef _v4l2wcprop_H_
#define _v4l2wcprop_H_

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if defined(__UNIX__)

#include "captypes.h"


//!@brief Abstract class carries one property
class CProperty
{
public:
   enum EKnownCams{
      EKC_NoCam  = 0,     // not a valid capture device
      EKC_Generic,        // a generic v4l2 capture device
      EKC_PWC_Generic,    // All of PWC driver
      EKC_PWC_ToUCam,     // ToUCam, Logitech etc
      EKC_PWC_Vesta,      // Vesta Range
      // more to come ...
   };

   CProperty(int p_device, EKnownCams p_camType) : m_device(p_device), m_cameraType(p_camType),
                                              m_lastSet(0), m_lastGet(0), m_cacheNum(0) {};
   virtual ~CProperty() {};

   virtual const EVidCapPropertySupport Has()=0;
   virtual const long Prop() =0;
   virtual bool Range(long &pMin, long &pMax, long &pDelta)=0;
   virtual bool put_Prop(long pVal)=0;

   //!@brief Returns a number of cached values for the specific property
   // this is used to deal with that pesky FPS setting as prop
   long CachedValue(short cindex) const;
   bool HasCache() const { return (m_cacheNum>0);}


protected:
   int        m_device;
   EKnownCams m_cameraType;
   long       m_lastSet;
   long       m_lastGet;
   short      m_cacheNum; // number of valid cached values
   long       m_cache[20];
private:
    CProperty() {};
};



//!@brief Provides all properties of a V4L2 device
class CV4LwcProp
{
public:
   CV4LwcProp();
   virtual ~CV4LwcProp();


public:
   // Assign a device with this object
   void put_CaptureDevice(int p_device);

   // the property interface
   const EVidCapPropertySupport Has(EVidCapProperty Property);
   const long Prop(EVidCapProperty Property) const;
   bool Range(EVidCapProperty Property, long &pMin, long &pMax, long &pDelta);
   bool put_Prop(EVidCapProperty Property, long pVal);

   //!@brief Returns a number of cached values for the specific property
   // this is used to deal with that pesky FPS setting as prop
   long CachedValue(EVidCapProperty Property, short cindex) const;
   bool HasCache(EVidCapProperty Property) const;

private:

   void KillPropSet(); // cleaning up helper
   struct SProps   // describes a property
   {
      EVidCapProperty  propID;     // my own ID (not yet used)
      CProperty       *propObj;   // the instance
   };
   SProps                  m_propSet[VCP_LastElement+1]; // all properties
   int                     m_device;                // the controlled device
   CProperty::EKnownCams   m_cameraType;

};




#endif // defined UX
#endif  // _v4l2wcprop_H_
