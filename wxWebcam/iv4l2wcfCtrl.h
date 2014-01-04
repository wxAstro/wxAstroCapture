//------------------------------------------------------------------------------
// File: iV4Lwcfilter.h
//
// Desc: DirectShow sample code - custom interface to allow the user to
//       perform image special effects.
//
// Adopted to make a SampleGrabber with special capabilities by BM


#ifndef __iv4l2wcfCtrl__
#define __iv4l2wcfCtrl__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if defined(__UNIX__)

#include "captypes.h"



// We define a callback class template for this example.
// Normally, you would make the BM_Grabber support a COM interface,
// and in one of its methods you would pass in a pointer to a COM interface
// used for calling back. See the DirectX documentation for the BM_Grabber
// for more information.


//! @brief abstract interface which owner of the webcam media filter must implement
class CSampleCB
{
public:
    virtual ~CSampleCB() {};
	virtual bool SampleCBX(unsigned char *pSampleNative, size_t lengthNative,
                          unsigned char *pSampleBgr, size_t lengthBgr,
                          EVideoConversion conversion)=0;

   //20100701:BM - implement framedropping in the filter without processing the image
   virtual bool IsSampleCBXready()=0;

};//CSampleCB


// Abstract classes where the wcFilter is derived from

//! @brief generic Webcam camera worker thread prototype
class CapWorker
{
public:
 virtual ~CapWorker(){};
 void operator () () { Entry(); };
 virtual void Entry() = 0;
};

// uses the common types defined in captypes.h
//! @brief generic webcam media filter interface
class IV4LwcfCtrl
{
public:
   virtual ~IV4LwcfCtrl(){};

   //!@brief set the call back for captured images
   virtual bool SetCallback(CSampleCB *Callback)=0;

   //!@brief Return the current pixel order selected
   virtual bool get_V4LwcfPixOrder(EVideoPixelOrder& pixOrder)=0;
   //!@brief Set the required pixel order
   virtual bool put_V4LwcfPixOrder(EVideoPixelOrder pixOrder)=0;

   //!@brief Return the current conversion format
   virtual bool get_V4LwcfConversion(EVideoConversion& conversion)=0;
   //!@brief Set the required conversion format
   virtual bool put_V4LwcfConversion(EVideoConversion conversion)=0;

   //!@brief get/put flag for enable/disable converting and streaming
   //!@brief disable streaming allows to save some resources while not used
   virtual bool get_V4LwcfCapture(bool &enabled)=0;
   virtual bool put_V4LwcfCapture(bool enabled)=0;

   //!@brief get the current FPS/DPS values  //20100702:BM - added
   virtual bool GetRates(float& fps, float& dps)=0;

   //!@brief put the new FPS values
   virtual bool put_V4LwcfFps(wxUint32 num, wxUint32 denom)=0;

   //!@brief Handle the stream in the task to be in sync  //20100702:BM - added
   virtual bool StreamON()=0;
   virtual bool StreamOFF()=0;
   virtual bool IsStreaming()=0;



   //!@brief return the number of used buffers
   virtual size_t get_NBuffers()=0;
   //!@brief initialize internal structs
   virtual bool InitCapFilter()=0;

};

#endif //__UNIX__
#endif // __iv4l2wcfCtrl__



