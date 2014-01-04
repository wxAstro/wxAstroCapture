//------------------------------------------------------------------------------
// File: V4Lwcfilter.h
//
// Desc: DirectShow sample code - special effects filter header file.
//
// Adopted to make a SampleGrabber with special capabilities by BM

#ifndef __v4l2wcfilter__
#define __v4l2wcfilter__


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if defined(__UNIX__)


#include "iv4l2wcfCtrl.h"


//!@brief A webcam media filter that implements a calling interface and a thread interface
class CV4Lstream;
class CV4LcapFilterItem;
class GenericFps;

class CV4Lwcfilter
    : public CapWorker
    , public IV4LwcfCtrl
{

public:
    CV4Lwcfilter(CV4Lstream* pCaller, int m_device, const CV4LcapFilterItem& capEntry);

    virtual ~CV4Lwcfilter();

   //! Prematurely stop thread execution
   void AbortThread();

    // These implement the custom CapWorker interface
   void Entry();

    // These implement the custom IV4LwcfCtrl interface

   //!@brief set the call back for captured images
   bool SetCallback( CSampleCB * Callback );

   //!@brief Return the current pixel order selected
   bool get_V4LwcfPixOrder(EVideoPixelOrder &pixOrder);
   //!@brief Set the required pixel order
   bool put_V4LwcfPixOrder(EVideoPixelOrder pixOrder);

   //!@brief Return the current conversion format
   bool get_V4LwcfConversion(EVideoConversion &conversion);
   //!@brief Set the required conversion format
   bool put_V4LwcfConversion(EVideoConversion conversion);

   //!@brief get/put flag for enable/disable converting and streaming
   //!@brief disable streaming allows to save some resources while not used
   bool get_V4LwcfCapture(bool &enabled);
   bool put_V4LwcfCapture(bool enabled);

   //!@brief gut the current FPS/DPS values  //20100702:BM - added
   bool GetRates(float& fps, float& dps);

   //!@brief put the new FPS values
   bool put_V4LwcfFps(wxUint32 num, wxUint32 denom);

   //!@brief return the number of used buffers
   size_t get_NBuffers() {return m_n_buffers;};

   //!@brief initialize internal structs
   bool InitCapFilter();

   //!@brief Handle the stream in the task to be in sync  //20100702:BM - added
   bool StreamON();
   bool StreamOFF();
   bool IsStreaming() {return m_streaming;};

private:
//   CV4Lwcfilter() {}; // hide the empty constructor

    // Look after doing the special effect
   bool CopyConvert(wxUint8 *pSource, size_t slength);
	void CvtIYUV2BGR (long width, long height,
							   wxUint8 *ySrc, wxUint8 *uSrc,
							   wxUint8 *vSrc, wxUint8 *bgrDst);
	void CvtYUYV2BGR (long width, long height, wxUint8 *ySrc, wxUint8 *bgrDst);
	void CvtYVYU2BGR (long width, long height, wxUint8 *ySrc, wxUint8 *bgrDst);
	void CvtUYVY2BGR (long width, long height, wxUint8 *ySrc, wxUint8 *bgrDst);
	void CvtY8002BGR (long width, long height, wxUint8 *ySrc, wxUint8 *bgrDst);

	void CvtBayerGB2BGR (long bayerType, long width, long height,
                            wxUint8 *ySrc, wxUint8 *bgrDst);
   EVideoConversion GetFromPixFormat(wxUint32 fourCC);

   void DropCapFilter();
   bool AllocateCapBuffers();
   void DropCapBuffers();

   void LocalChangeFps();
   void LocalStreamON();
   void LocalStreamOFF();

   // initialize internal structs

   CV4Lstream*     m_pCaller;
   int             m_device;
   wxUint8*        m_bmpData;  // points to data start

	wxUint32        m_width;
	wxUint32        m_height;
	wxUint32        m_fourCC;
	wxUint32        m_num;     // fps numerator
	wxUint32        m_denom;   // fps denominator
	long            m_size;
	bool            m_isTypeRGB;  // Native source is RGB map

   //v4l2 buffer type
   struct buffer {
      void* start;
      size_t length;
   };
   typedef buffer   *pbuffer;

   pbuffer           m_pBuffers;
   unsigned int      m_n_buffers;

   CSampleCB        *m_callback;

   EVideoConversion  m_conversion;        // Which conversion to apply
   EVideoPixelOrder  m_pixelOrder;        // Which pixel order is in place
   bool              m_captureFlag;       // conversion and stream enabled if true
   const long        m_lBufferRequest;    // The number of buffers to use

   bool              m_streaming;
   bool              m_streamOffNeeded;   // true if requested
   bool              m_fpsChangeNeeded;   // true if requested
   wxUint32          m_newNum;   // FPS numerator
   wxUint32          m_newDenom; // FPS denominator
   GenericFps        *m_genFps;

   bool              m_abort;
   int               m_exitCode;

}; // V4Lwcfilter

#endif //__UNIX__
#endif //__v4l2wcfilter__


