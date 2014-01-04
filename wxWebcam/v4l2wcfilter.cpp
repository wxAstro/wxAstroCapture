//------------------------------------------------------------------------------
// File: V4Lwcfilter.cpp
//

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if defined(__UNIX__)


#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <poll.h>

#include <libv4l1-videodev.h>
//#include <linux/videodev.h>
#include <linux/videodev2.h>

#include "v4l2wcfilter.h"
#include "v4l2capfilter.h"
#include "v4l2stream.h"
#include "GenericFps.h"

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



static long long NowMs()
{
   timeval _now; ::gettimeofday(&_now, NULL);
   // must take care of data type (tv_sec is large already)
   return (long long)(_now.tv_sec) * (long long)(1000) + _now.tv_usec/1000;
}

// return the elapsed milliseconds
static unsigned long ElapsedMSec(long long startMs)
{
   long long _now = NowMs();
   if (_now>startMs) {
      long long diff = _now - startMs;
      return ( unsigned long)(diff);
   }
   return 0;
}
static pollfd s_pollCam;

#define CLEAR(x) memset(&(x), 0, sizeof(x))


// Win type BGR plane
#define  CV_BayerBG2BGR 40
#define  CV_BayerGB2BGR 41
#define  CV_BayerRG2BGR 42
#define  CV_BayerGR2BGR 43

// Linux type RGB plane
#define  CV_BayerBG2RGB CV_BayerRG2BGR
#define  CV_BayerGB2RGB CV_BayerGR2BGR
#define  CV_BayerRG2RGB CV_BayerBG2BGR
#define  CV_BayerGR2RGB CV_BayerGB2BGR

#define XTIME(vh, vl)    s = ::wxGetElapsedTime(); (vh) = (s> (vh) )?s: (vh) ; (vl) = (s< (vl) )?s: (vl) ;

//
// Constructor
//
CV4Lwcfilter::CV4Lwcfilter(CV4Lstream* pCaller, int p_device, const CV4LcapFilterItem& capEntry)
: m_pCaller(pCaller)
, m_device(p_device)
, m_bmpData(NULL)
, m_width(0)
, m_height(0)
, m_fourCC(0)
, m_num(1)
, m_denom(1)
, m_size(0)
, m_isTypeRGB(false)
, m_pBuffers(NULL)
, m_n_buffers(0)
, m_callback(NULL)
, m_conversion(EVC_UNKNOWN)
, m_pixelOrder(EVO_GB)
, m_captureFlag(false)
, m_lBufferRequest(1)
, m_streaming(false)
, m_streamOffNeeded(false)
, m_fpsChangeNeeded(false)
, m_newNum(1)
, m_newDenom(1)
, m_genFps( new GenericFps(23, 1) )
, m_abort(false)
, m_exitCode(0)
{
   // setup the poll structures
   CLEAR(s_pollCam);
   s_pollCam.fd = p_device;
   s_pollCam.events = POLLIN;

   // save default formats
   m_width  = capEntry.FrameSize_Width();
   m_height = capEntry.FrameSize_Height();
   m_fourCC = capEntry.PixFormat();
   m_num    = capEntry.Fps_Num();
   m_denom  = capEntry.Fps_Denom();

   // set reasonable capabilities
   InitCapFilter();

} // (Constructor)

CV4Lwcfilter::~CV4Lwcfilter()
{
   m_abort=true;
   DropCapFilter();
   delete m_genFps;
}



//!@brief release resources
void CV4Lwcfilter::DropCapFilter()
{
   // free allocated request buffers - if not already done
   DropCapBuffers();
   if (m_bmpData)  delete [] m_bmpData;  m_bmpData=NULL;  // in case it was not collected
}



bool CV4Lwcfilter::AllocateCapBuffers()
{
   _V_ ::fprintf(stderr, "CV4Lwcfilter::AllocateCapBuffers() - entry\n");

   // request buffers for capture
   v4l2_requestbuffers req;
   CLEAR(req);
   req.count  = 3; // try to get 3 buffers (we always only get 2 by the PWC driver ??)
   req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   req.memory = V4L2_MEMORY_MMAP;

   _V_ ::fprintf(stderr, "CV4Lwcfilter::AllocateCapBuffers() - Request framebuffers\n");

   _V_ ::fprintf(stderr, "CV4Lwcfilter::AllocateCapBuffers() -VIDIOC_REQBUFS - request %d buffers \n", req.count);
   if(-1 == xioctl(m_device,VIDIOC_REQBUFS, &req)) {
      ::fprintf(stderr, "CV4Lwcfilter::AllocateCapBuffers() - VIDIOC_REQBUFS failed - errno: %d %s \n", errno, strerror(errno));
      return false;
   }

   if(req.count < 2) {
      ::fprintf(stderr, "CV4Lwcfilter::AllocateCapBuffers() - double buffer failed - errno: %d %s \n", errno, strerror(errno));
      return false;
   }

   // allocate buffer descriptor memory
   m_pBuffers = new buffer[req.count];
   if(!m_pBuffers) {
      ::fprintf(stderr, "CV4Lwcfilter::AllocateCapBuffers() - Out of memory\n");
      return false;
   }

   _V_ ::fprintf(stderr, "CV4Lwcfilter::AllocateCapBuffers() - %d buffers granted\n", req.count);


   // using memory mapped buffers here
   _V_ ::fprintf(stderr, "CV4Lwcfilter::AllocateCapBuffers() - Get and map framebuffers\n");
   for(m_n_buffers = 0; m_n_buffers < req.count ; ++m_n_buffers) {
      v4l2_buffer buf; CLEAR(buf);

      buf.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory  = V4L2_MEMORY_MMAP;
      buf.index   = m_n_buffers;

      _V_ ::fprintf(stderr, "CV4Lwcfilter::AllocateCapBuffers() - VIDIOC_QUERYBUF - index: %u\n", m_n_buffers);
      if(-1 == xioctl(m_device, VIDIOC_QUERYBUF,&buf)) {
         ::fprintf(stderr, "CV4Lwcfilter::AllocateCapBuffers() - VIDIOC_QUERYBUF failed EXIT- errno: %d %s\n", errno, strerror(errno));
         delete [] m_pBuffers;
         m_pBuffers=NULL;
         return false;
      }

      _V_ ::fprintf(stderr, "CV4Lwcfilter::AllocateCapBuffers() - mmap buffer\n");
      m_pBuffers[m_n_buffers].length = buf.length;
      m_pBuffers[m_n_buffers].start
             = mmap(NULL,buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, m_device, buf.m.offset);
      if(MAP_FAILED == m_pBuffers[m_n_buffers].start) {
         ::fprintf(stderr, "CV4Lwcfilter::AllocateCapBuffers() - mmap failed EXIT- errno: %d %s\n", errno, strerror(errno));
         return false;
      }
   }//for


   _V_ ::fprintf(stderr, "CV4Lwcfilter::AllocateCapBuffers() - exit\n");
   return true;
}



void CV4Lwcfilter::DropCapBuffers()
{
   _V_ ::fprintf(stderr, "CV4Lwcfilter::DropCapBuffers() - entry\n");

   // free allocated request buffers - if not already done
   LocalStreamOFF();

   // free ressources
   if (m_pBuffers) {
      for(size_t i=0; i< m_n_buffers; ++i) {
         if(-1 == munmap(m_pBuffers[i].start, m_pBuffers[i].length)) {
            ::fprintf(stderr, "CV4Lwcfilter::DropCapBuffers() - munmap failed - errno: %d %s\n", errno, strerror(errno));
         }
      }
      delete [] m_pBuffers; m_pBuffers=NULL;  // v4l2 capture buffers
   }
   _V_ ::fprintf(stderr, "CV4Lwcfilter::DropCapBuffers() - exit\n");
}


//! initialize internal structs
bool CV4Lwcfilter::InitCapFilter()
{
   _V_ ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() - entry\n");

   // kill things first
   _V_ ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() - Drop existing capture items first\n");
   DropCapFilter();

   {//block
      _V_ ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() - Prepare new items now \n");
      // retrieve current format
      v4l2_format fmt; CLEAR(fmt);
      fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // must define what we want to receive
      _V_ ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() - VIDIOC_G_FMT - get current setting \n");
      if(-1 == xioctl(m_device,VIDIOC_G_FMT,&fmt)) {
         ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() - VIDIOC_G_FMT failed EXIT- errno: %d %s\n", errno, strerror(errno));
         return false;
      }

      // set the desired caps
      fmt.fmt.pix.pixelformat = m_fourCC;
      fmt.fmt.pix.width       = m_width;
      fmt.fmt.pix.height      = m_height;
      fmt.fmt.pix.field       = V4L2_FIELD_ANY;

      _V_ ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() - wxh requested: %ux%u \n", m_width, m_height);

      _V_ ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() - VIDIOC_S_FMT \n");
      if(-1 == xioctl(m_device,VIDIOC_S_FMT,&fmt)) {
         ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() - VIDIOC_S_FMT failed - errno: %d %s\n", errno, strerror(errno));
       //  return false;
      }

      if (m_num>0) {
         _V_ ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() - Setting desired fps - lowest rate \n");
         // set the desired fps
         v4l2_streamparm sPara; CLEAR(sPara);
         sPara.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
         _V_ ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() - VIDIOC_G_PARM - get current setting\n");
         if( -1 == xioctl(m_device, VIDIOC_G_PARM, &sPara) ) {
            ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() - VIDIOC_G_PARM failed - errno: %d %s\n", errno, strerror(errno));
         }
         else {
            if (sPara.parm.capture.capability & V4L2_CAP_TIMEPERFRAME) {
               sPara.parm.capture.capturemode = V4L2_CAP_TIMEPERFRAME;
               sPara.parm.capture.timeperframe.numerator = m_num;
               sPara.parm.capture.timeperframe.denominator = m_denom;
               _V_ ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() - VIDIOC_S_PARM - set %u / %u sec\n", m_num, m_denom);
               if( -1 == xioctl(m_device, VIDIOC_S_PARM, &sPara) ) {
                  ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() - VIDIOC_S_PARM failed - errno: %d %s\n", errno, strerror(errno));
               }
            }
         }
      }
      else {
         ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() fps with V4L2_CAP_TIMEPERFRAME not supported\n");
      }
   }//block

   // then establish the capture buffers
   if ( ! AllocateCapBuffers() ) return false;

   {//block
      _V_ ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() - Get and adjust the final settings \n");
      // retrieve current format, and report to stderr
      v4l2_format fmt; CLEAR(fmt);
      fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // must define what we want to receive
        _V_ ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() - VIDIOC_G_FMT - get final frame setting \n");
       if(-1 == xioctl(m_device,VIDIOC_G_FMT,&fmt)) {
         ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() - VIDIOC_G_FMT failed - errno: %d %s\n", errno, strerror(errno));
         return false;
      }
      // we want to get the desired back
      if (m_width  != fmt.fmt.pix.width) {
         ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() - width wanted: %u - we get: %u\n", m_width, fmt.fmt.pix.width);
      }
      if (m_height  != fmt.fmt.pix.height) {
         ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() - height wanted: %u - we get: %u\n", m_height, fmt.fmt.pix.height);
      }

      // just in case - so it will not break to heavily
      m_width = fmt.fmt.pix.width;
      m_height = fmt.fmt.pix.height;
      m_size   = m_width*m_height*3; // make sure we have enough for BMPs
      ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() vidsize:   %u x %u\n", m_width, m_height);

      if ( m_fourCC  != fmt.fmt.pix.pixelformat ) {
         ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() - fourCC wanted: %c%c%c%c - we get: %c%c%c%c\n",
               char(m_fourCC&0xff), char((m_fourCC>>8)&0xff), char((m_fourCC>>16)&0xff), char((m_fourCC>>24)&0xff),
               char(fmt.fmt.pix.pixelformat&0xff), char((fmt.fmt.pix.pixelformat>>8)&0xff),
               char((fmt.fmt.pix.pixelformat>>16)&0xff), char((fmt.fmt.pix.pixelformat>>24)&0xff));
      }
      m_fourCC = fmt.fmt.pix.pixelformat;
      m_conversion = GetFromPixFormat(m_fourCC);

      ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() pixformat: %c%c%c%c\n",
               char(m_fourCC&0xff), char((m_fourCC>>8)&0xff), char((m_fourCC>>16)&0xff), char((m_fourCC>>24)&0xff));

      if (m_num>0) {
         _V_ ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() - Get the final fps setting \n");
         // get the actual fps if fps setting is available
         v4l2_streamparm sPara; CLEAR(sPara);
         sPara.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
         _V_ ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() - VIDIOC_G_PARM - get current setting\n");
         if( -1 == xioctl(m_device, VIDIOC_G_PARM, &sPara) ) {
            ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() - VIDIOC_G_PARM failed - errno: %d %s\n", errno, strerror(errno));
         }
         else {
            if (sPara.parm.capture.capability & V4L2_CAP_TIMEPERFRAME) {
               m_num   = sPara.parm.capture.timeperframe.numerator;
               m_denom = sPara.parm.capture.timeperframe.denominator;
               ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() fps: %d / %d sec\n", m_num, m_denom);
            }
         }

      }

      _V_ ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() - allocate bitmap mem: (%ld bytes) \n", m_size);
      m_bmpData = new wxUint8[m_size];
      if ( m_bmpData == NULL ) {
         ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() no memory to allocate bitmap EXIT \n");
         return false;
      }
   }//block

   _V_ ::fprintf(stderr, "CV4Lwcfilter::InitCapFilter() - exit\n");

   return true;
}


void CV4Lwcfilter::LocalStreamON()
{
   _V_ ::fprintf(stderr, "CV4Lwcfilter::LocalStreamON()\n");

   if ( m_streaming ) return;  // already
   if ( m_device == -1) return; // nope

   // ask to queue all buffers
   for(size_t i= 0; i < m_n_buffers; i++) {
      v4l2_buffer buf; CLEAR(buf);
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; buf.memory = V4L2_MEMORY_MMAP;
      buf.index = i;
      _V_ ::fprintf(stderr, "CV4Lwcfilter::LocalStreamON() - VIDIOC_QBUF - index: %u\n", i);
      if(-1 == xioctl(m_device,VIDIOC_QBUF,&buf)) {
         ::fprintf(stderr, "CV4Lwcfilter::LocalStreamON() - VIDIOC_QBUF failed - errno: %d %s\n", errno, strerror(errno));
         return;
      }
   }

   // make it start
   v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   _V_ ::fprintf(stderr, "CV4Lwcfilter::LocalStreamON() - VIDIOC_STREAMON \n");
   if(-1 == xioctl(m_device,VIDIOC_STREAMON,&type)) {
      ::fprintf(stderr, "CV4Lwcfilter::LocalStreamON() - VIDIOC_STREAMON failed - errno: %d\n", errno);
      return;
   }
   else {
      m_streaming=true;
   }
}

void CV4Lwcfilter::LocalStreamOFF()
{
   _V_ ::fprintf(stderr, "CV4Lwcfilter::LocalStreamOFF()\n");

   if ( ! m_streaming ) return;  // already
   if ( m_device == -1) return; // nope

   // make it stop
   m_streamOffNeeded=false; // not longer
   v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   _V_ ::fprintf(stderr, "CV4Lwcfilter::LocalStreamOFF() - VIDIOC_STREAMOFF \n");
   if(-1 == xioctl(m_device,VIDIOC_STREAMOFF,&type)) {
      ::fprintf(stderr, "CV4Lwcfilter::LocalStreamOFF() - VIDIOC_STREAMOFF failed - errno: %d\n", errno);
   }
   else {
      // buffers are de-queued now !
      m_streaming=false;
   }
}


void CV4Lwcfilter::LocalChangeFps()
{
   _V_ ::fprintf(stderr, "CV4Lwcfilter::LocalChangeFps() - entry\n");

   v4l2_streamparm sPara; CLEAR(sPara);
   if ( (m_newNum>0) && (m_newDenom>0) ) {
      // set the desired fps
      sPara.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      xioctl(m_device, VIDIOC_G_PARM, &sPara);
      if (sPara.parm.capture.capability & V4L2_CAP_TIMEPERFRAME) {
         _V_ ::fprintf(stderr, "CV4Lwcfilter::ChangeFps() fps: was %d / %d seconds\n", m_num, m_denom);

         // to change FPS make the stream stop
         DropCapBuffers();

         // change the FPS setting
         sPara.parm.capture.capturemode = V4L2_CAP_TIMEPERFRAME;
         sPara.parm.capture.timeperframe.numerator = m_newNum;
         sPara.parm.capture.timeperframe.denominator = m_newDenom;
         if (-1 == xioctl(m_device, VIDIOC_S_PARM, &sPara) ) {
            ::fprintf(stderr, "CV4Lwcfilter::ChangeFps() VIDIOC_S_PARM failed - errno: %d %s\n", errno, strerror(errno));
         }

         AllocateCapBuffers(); // get, allocate and queue buffers

         // make it run again
         LocalStreamON();

         // now reread it again to see what it does have now
         CLEAR(sPara);
         sPara.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
         xioctl(m_device, VIDIOC_G_PARM, &sPara);
         if (sPara.parm.capture.capability & V4L2_CAP_TIMEPERFRAME) {
            // we want to get the newly accepted back
            m_num   = sPara.parm.capture.timeperframe.numerator;
            m_denom = sPara.parm.capture.timeperframe.denominator;
            ::fprintf(stderr, "CV4Lwcfilter::ChangeFps() fps: now %d / %d seconds\n", m_num, m_denom);
         }
      }
      else {
         ::fprintf(stderr, "CV4Lwcfilter::ChangeFps() fps with V4L2_CAP_TIMEPERFRAME not supported\n");
      }
   }

   _V_ ::fprintf(stderr, "CV4Lwcfilter::LocalChangeFps() - exit\n");
}

//! Task execution comes here
void CV4Lwcfilter::Entry()
{
   _V_ ::fprintf(stderr, "CV4Lwcfilter::Entry() - entry\n");
   // Sanity check
   if ( !m_pCaller ) {
      ::fprintf(stderr, "CV4Lwcfilter::Entry() - sw error caller not assigned\n");
      return; // immediately - this is messy - we cannot even reach our creator
   }

	m_exitCode = 0;         // task result

   long long startMs = NowMs(); // debug helper for time tracking
   long      capImages = 0;
   long      droppedImages = 0;
   bool      reported = false;

   bool cRet;
   bool done = false;
   while (!done) {
      if (m_abort) {
         _V_ ::fprintf(stderr, "CV4Lwcfilter::Entry() - must abort now - thread will end\n");
         done=true;
      }
      else {
         // do something useful here

         // poll the cam device to receive the image stream
         int r = poll(&s_pollCam, 1, 250);  // wait up to 250ms if no data corunmes in

         if (r>0 && ( (s_pollCam.revents | POLLIN)==POLLIN) ) {

            if (m_streaming) {
               v4l2_buffer buf; CLEAR(buf);
               buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; buf.memory = V4L2_MEMORY_MMAP;
               // take a sample out of the queued buffers
               _D_ ::fprintf(stderr, "CV4Lwcfilter::Entry() - VIDIOC_DQBUF\n");
               cRet = (-1 != xioctl(m_device,VIDIOC_DQBUF,&buf));
               if (cRet) {
                  _D_ ::fprintf(stderr, "CV4Lwcfilter::Entry() - got buffer: %u; length %u\n", buf.index, buf.length);
                  // check if we should process a sample or just drop it
                  _D_ ::fprintf(stderr, "CV4Lwcfilter::Entry() - check callback\n");
                  if ( m_callback ) {
                     _D_ ::fprintf(stderr, "CV4Lwcfilter::Entry() - check if sample is allowed\n");
                     if ( m_callback->IsSampleCBXready() ) {
                        // process the received image
                        _D_ ::fprintf(stderr, "CV4Lwcfilter::Entry() - yes, allowed\n");
                        cRet= CopyConvert((wxUint8*)m_pBuffers[buf.index].start, buf.bytesused);
                        capImages++;
                        m_genFps->AddSentFrame(ElapsedMSec(startMs));
                     }
                     else {
                        // dropped frame goes here
                        _D_ ::fprintf(stderr, "CV4Lwcfilter::Entry() - no, dump it\n");
                        droppedImages++;
                        m_genFps->AddDroppedFrame(ElapsedMSec(startMs));
                        m_callback->SampleCBX(NULL, 0, NULL, 0, EVC_UNKNOWN); // return an empty tick
                     }

                     // tell about the current FPS (but only every 10th image received
                     if ( (capImages % 10) ==0 ) {
                        // avoid multiple reports on the same number of frames taken
                        if ( reported==false ) {
                           _V_ ::fprintf(stderr, "CV4Lwcfilter::Entry() images taken: %ld - fps: %f - dropped frames %ld - dps: %f\n",
                                                   capImages, m_genFps->FpsRate(), droppedImages, m_genFps->DpsRate());
                           reported=true;
                        }
                     }
                     else {
                        reported=false;
                     }
                  }
                  else {
                     ::fprintf(stderr, "CV4Lwcfilter::Entry() - callback target not longer valid\n");
                  }
               }
               else {
                  ::fprintf(stderr, "CV4Lwcfilter::Entry() - VIDIOC_DQBUF failed - errno: %d %s\n", errno, strerror(errno));
               }

               // if there is neeed to change the FPS do it now before getting the next buffer
               if (m_fpsChangeNeeded) {
                  _V_ ::fprintf(stderr, "CV4Lwcfilter::Entry() - Fps change requested\n");
                  m_fpsChangeNeeded=false;
                  LocalChangeFps(); capImages=0; droppedImages=0; startMs = NowMs(); // restart debug helper for FPS tracking
                  // this will also requeue all buffers
               }
               else {
                  // queue next sample
                  errno=0;
                  _D_ ::fprintf(stderr, "CV4Lwcfilter::Entry() - VIDIOC_QBUF\n");
                  cRet = (-1 != xioctl(m_device,VIDIOC_QBUF,&buf));
                  if ( !cRet ) {
                     ::fprintf(stderr, "CV4Lwcfilter::Entry() - VIDIOC_QBUF failed - errno: %d %s", errno, strerror(errno));
                     ::fprintf(stderr, " - buffer# %u\n", buf.index);
                  }
               }

               if (m_streamOffNeeded) LocalStreamOFF();
            }//streaming

         }// data received
      }
   }// task loop
   _V_ ::fprintf(stderr, "CV4Lwcfilter::Entry() - exit\n");
}


// thread must abort itself
void CV4Lwcfilter::AbortThread()
{
   _V_ ::fprintf(stdout, "CV4Lwcfilter::AbortThread()\n");

	// will make the thread to terminate
	m_abort = true; // thread will end now (in a while)
}


//!@brief Make destination an copy of source / but with transform
bool CV4Lwcfilter::CopyConvert(wxUint8 *pSource, size_t slength)
{

   _D_ ::fprintf(stdout, "CV4Lwcfilter::CopyConvert() - entry\n");

   // Convert the sample data
   // from Input Format I420(IYUV) planar 12bit
   // to Output Format RGB24 currently

   bool retVal=false;

   // this is done only if capturing is required (pounds CPU due to image conversion)
   if (m_captureFlag) {
      _D_ ::fprintf(stdout, "CV4Lwcfilter::CopyConvert() - capture flag is set\n");
      // make sure we use actual data

      if (m_isTypeRGB) {
         _D_ ::fprintf(stdout, "CV4Lwcfilter::CopyConvert() - type RGB\n");
         // source assumed to be BGR bitmap data
         switch (m_conversion) {
            case EVC_RGB24_RGB24:
               ::memcpy(m_bmpData, pSource, m_size); // BGR is copy only
               retVal = true;
            break;

            default:
               m_size = 0; // does not return a thing
               retVal = false; // there is no Bayer when yource is RGB
               ::fprintf(stderr, "CV4Lwcfilter::CopyConvert() - unexpected pixelformat");
         }//switch conversion
      }
      else {
         _D_ ::fprintf(stdout, "CV4Lwcfilter::CopyConvert() - type YUV\n");
         switch (m_conversion) {
            case EVC_CFA_RGB24:
               _D_ ::fprintf(stdout, "CV4Lwcfilter::CopyConvert() - Bayer CFA to RGB24\n");
               // convert Y from Bayer CFA to RGB24
               switch (m_pixelOrder) {
                  case EVO_GB:
                     CvtBayerGB2BGR (CV_BayerGB2BGR, m_width, m_height, pSource, m_bmpData);
                  break;
                  case EVO_BG:
                     CvtBayerGB2BGR (CV_BayerBG2BGR, m_width, m_height, pSource, m_bmpData);
                  break;
                  case EVO_GR:
                     CvtBayerGB2BGR (CV_BayerGR2BGR, m_width, m_height, pSource, m_bmpData);
                  break;
                  case EVO_RG:
                     CvtBayerGB2BGR (CV_BayerRG2BGR, m_width, m_height, pSource, m_bmpData);
                  break;
               }//switch
               retVal = true;
            break;

            case EVC_BY8_RGB24:
               _D_ ::fprintf(stdout, "CV4Lwcfilter::CopyConvert() - Bayer BY8 to RGB24\n");
               // convert Y from Bayer BY8 to RGB24
               switch (m_pixelOrder) {
                  case EVO_GB:
                     CvtBayerGB2BGR (CV_BayerGB2BGR, m_width, m_height, pSource, m_bmpData);
                  break;
                  case EVO_BG:
                     CvtBayerGB2BGR (CV_BayerBG2BGR, m_width, m_height, pSource, m_bmpData);
                  break;
                  case EVO_GR:
                     CvtBayerGB2BGR (CV_BayerGR2BGR, m_width, m_height, pSource, m_bmpData);
                  break;
                  case EVO_RG:
                     CvtBayerGB2BGR (CV_BayerRG2BGR, m_width, m_height, pSource, m_bmpData);
                  break;
               }//switch
               retVal = true;
            break;

            case EVC_I420_RGB24: {
               _D_ ::fprintf(stdout, "CV4Lwcfilter::CopyConvert() - I420 (IYUV, YU12) to RGB24\n");
               // convert YUV planar to BGR24
               // Y[height][width] - U[height/2][width/2] - V[height/2][width/2]
               // source is assumed planar YUV full size Y then 1/4 U then 1/4 V
               wxUint8* uPtr = &pSource[m_width*m_height];
               wxUint8* vPtr = &uPtr[(m_width>>1) * (m_height>>1)];
               CvtIYUV2BGR (m_width, m_height, pSource, uPtr, vPtr, m_bmpData);
               retVal = true;
            }
            break;

            case EVC_YUY2_RGB24:
               _D_ ::fprintf(stdout, "CV4Lwcfilter::CopyConvert() - YUY2 (YUYV) to RGB24\n");
               // convert YUY2 planar to BGR24
               // Y[height][width] - U[height/2][width/2] - V[height/2][width/2]
               CvtYUYV2BGR (m_width, m_height, pSource, m_bmpData);
               retVal = true;
            break;

            case EVC_YVYU_RGB24:
               _D_ ::fprintf(stdout, "CV4Lwcfilter::CopyConvert() - YVYU to RGB24\n");
               // convert YVYU planar to BGR24
               // Y[height][width] - U[height/2][width/2] - V[height/2][width/2]
               CvtYVYU2BGR (m_width, m_height, pSource, m_bmpData);
               retVal = true;
            break;

            case EVC_UYVY_RGB24:
               _D_ ::fprintf(stdout, "CV4Lwcfilter::CopyConvert() - UYVY to RGB24\n");
               // convert YVYU planar to BGR24
               // Y[height][width] - U[height/2][width/2] - V[height/2][width/2]
               CvtUYVY2BGR (m_width, m_height, pSource, m_bmpData);
               retVal = true;
            break;

            case EVC_Y800_RGB24:
               _D_ ::fprintf(stdout, "CV4Lwcfilter::CopyConvert() - Y800 to RGB24\n");
               // convert Y800 planar to BGR24
               // Y[height][width] - U[height/2][width/2] - V[height/2][width/2]
               CvtY8002BGR (m_width, m_height, pSource, m_bmpData);
               retVal = true;
            break;

            default:
               m_size = 0; // does not return a thing
               retVal = false; // there is no Bayer when yource is RGB
               ::fprintf(stderr, "CV4Lwcfilter::CopyConvert() - unexpected pixelformat");
         }//switch conversion
      }//if type YUV

      // calls back into v4l2stream to allow further processing of the sample
      if( retVal && m_callback ) {
         _D_ ::fprintf(stdout, "CV4Lwcfilter::CopyConvert() - calling SampleCBX\n");
         retVal = m_callback->SampleCBX(pSource, slength, m_bmpData, m_size, m_conversion); // return the bitmaps
         if ( !retVal ) ::fprintf(stderr, "CV4Lwcfilter::CopyConvert() - SampleCBX(data) failed \n");
      }
   }//if captureflag

   else {
      _D_ ::fprintf(stdout, "CV4Lwcfilter::CopyConvert() - capture flag is NOT set\n");
      // not capturing
      retVal = true;
      if( m_callback ) {
         retVal = m_callback->SampleCBX(NULL, 0, NULL, 0, EVC_UNKNOWN); // return an empty tick
         if ( !retVal ) ::fprintf(stderr, "CV4Lwcfilter::CopyConvert() - SampleCBX(NULL) failed \n");
      }
   }

   return retVal;

}

EVideoConversion CV4Lwcfilter::GetFromPixFormat(wxUint32 fourCC)
{

   switch (fourCC) {
      case wxAC_FOURCC_I420:
      case wxAC_FOURCC_IYUV:
      case wxAC_FOURCC_YU12:
        return EVC_I420_RGB24; break;

      case wxAC_FOURCC_YUY2:
      case wxAC_FOURCC_YUYV:
        return EVC_YUY2_RGB24; break;

      case wxAC_FOURCC_YVYU:
        return EVC_YVYU_RGB24; break;

      case wxAC_FOURCC_UYVY:
        return EVC_UYVY_RGB24; break;

      case wxAC_FOURCC_GREY:
      case wxAC_FOURCC_Y800:
        return EVC_Y800_RGB24; break;

      case wxAC_FOURCC_BY8:
        return EVC_BY8_RGB24; break;

      case wxAC_FOURCC_Y16:
      case wxAC_FOURCC_YP16:
        return EVC_Y16; break;

      default:
        ::fprintf(stderr, "CV4Lwcfilter::GetFromPixFormat() - unknown fourCC 0x%x \n", fourCC);
        return EVC_RGB24_RGB24; break;
   }
}

///----------------------------------------------------------------------------
/// Implement the wcFilter Interface
///----------------------------------------------------------------------------

//!@brief set the callback to deliver the images
bool CV4Lwcfilter::SetCallback( CSampleCB *  Callback )
{
    m_callback = Callback;

    return true;
}

//!@brief Return the current pixel order selected
bool CV4Lwcfilter::get_V4LwcfPixOrder(EVideoPixelOrder &pixOrder)
{
    pixOrder = m_pixelOrder;
    return true;

}

//!@brief Set the required pixel order
bool CV4Lwcfilter::put_V4LwcfPixOrder(EVideoPixelOrder pixOrder)
{
    m_pixelOrder = pixOrder;
    return true;

}

//!@brief Return the current conversion format
bool CV4Lwcfilter::get_V4LwcfConversion(EVideoConversion &conversion)
{
    conversion = m_conversion;
    return true;
}


//!@brief Set the required conversion format
bool CV4Lwcfilter::put_V4LwcfConversion(EVideoConversion conversion)
{
    m_conversion = conversion;
    return true;

}


//!@brief get/put flag for enable/disable converting and streaming
//!@brief disable streaming allows to save some resources while not used
bool CV4Lwcfilter::get_V4LwcfCapture(bool &enabled)
{
   enabled = m_captureFlag;
   return true;
}

bool CV4Lwcfilter::put_V4LwcfCapture(bool enabled)
{
   m_captureFlag = enabled;
   return true;
}


//!@brief gut the current FPS/DPS values  //20100702:BM - added
bool CV4Lwcfilter::GetRates(float& fps, float& dps)
{
   fps = m_genFps->FpsRate();
   dps = m_genFps->DpsRate();
   return true;
}


//!@brief put the new FPS values
bool CV4Lwcfilter::put_V4LwcfFps(wxUint32 num, wxUint32 denom)
{
   m_newNum = num;  m_newDenom = denom;
   m_fpsChangeNeeded=true;
   return true;
}


//!@brief Handle the stream in the task to be in sync  //20100702:BM - added
bool CV4Lwcfilter::StreamON()
{
   if ( ! m_streaming) LocalStreamON(); // done without thread sync when starting stream
   return true;
}

bool CV4Lwcfilter::StreamOFF()
{
   if ( m_streaming) m_streamOffNeeded=true;
   return true;
}

///--------------------------------------------------------------------------
/// Various format conversions from YUV to BGR
///--------------------------------------------------------------------------

#define  CV_DESCALE(x,n)     (((x) + (1 << ((n)-1))) >> (n))
#define fix(x,n)      (int)((x)*(1 << (n)) + 0.5)
#define descale       CV_DESCALE
#define shift  10
#define CV_CAST_8U(t)    (wxUint8)( !((t) & ~255) ? (t) : (t) > 0 ? 255 : 0)

//* original
#define  yuvRCr   fix(1.403,shift)
#define  yuvGCr   (-fix(0.344,shift))
#define  yuvGCb   (-fix(0.714,shift))
#define  yuvBCb   fix(1.773,shift)
//*/
// Philips Doc
/*
#define  yuvRCr   fix(1.371,shift)
#define  yuvGCr   (-fix(0.336,shift))
#define  yuvGCb   (-fix(0.698,shift))
#define  yuvBCb   fix(1.732,shift)
*/
// Win type BGR plane
#define  CVT_YCrCb2BGR( srcY, srcCb, srcCr, dst )     \
{ \
    int Y = (srcY)[0] << shift, Cr = (srcCr)[0] - 128, \
        Cb = (srcCb)[0] - 128; \
    int b, g, r; \
    \
    b = descale( Y + yuvBCb*Cb, shift ); \
    g = descale( Y + yuvGCr*Cr + yuvGCb*Cb, shift ); \
    r = descale( Y + yuvRCr*Cr, shift ); \
    \
    (dst)[0] = CV_CAST_8U(b); \
    (dst)[1] = CV_CAST_8U(g); \
    (dst)[2] = CV_CAST_8U(r); \
}

// Linux type RGB plane
#define  CVT_YCrCb2RGB( srcY, srcCb, srcCr, dst ) \
{ \
    int Y = (srcY)[0] << shift, Cr = (srcCr)[0] - 128, \
        Cb = (srcCb)[0] - 128; \
    int b, g, r; \
    \
    b = descale( Y + yuvBCb*Cb, shift ); \
    g = descale( Y + yuvGCr*Cr + yuvGCb*Cb, shift ); \
    r = descale( Y + yuvRCr*Cr, shift ); \
    \
    (dst)[0] = CV_CAST_8U(r); \
    (dst)[1] = CV_CAST_8U(g); \
    (dst)[2] = CV_CAST_8U(b); \
}

//___________END CV lib

// this ist YUV to BGR (Win type) Or RGB (Linux tape)
// depends on macro used (CVT_YCrCb2BGR or CVT_YCrCb2RGB)
void CV4Lwcfilter::CvtIYUV2BGR (long width, long height,
							   wxUint8 *ySrc, wxUint8 *uSrc,
							   wxUint8 *vSrc, wxUint8 *bgrDst)
{
register int x,y;
wxUint8 *pDst, *pySrc, *puSrc, *pvSrc;

   _D_ ::fprintf(stderr, "CV4Lwcfilter::CvtIYUV2BGR() - wxh: %ldx%ld\n", width, height);

   if ( ySrc==NULL ) {
      ::fprintf(stderr, "CV4Lwcfilter::CvtIYUV2BGR() - ySrc Ptr = NULL \n"); return;
   }
   if ( uSrc==NULL ) {
      ::fprintf(stderr, "CV4Lwcfilter::CvtIYUV2BGR() - uSrc Ptr = NULL \n"); return;
   }
   if ( vSrc==NULL ) {
      ::fprintf(stderr, "CV4Lwcfilter::CvtIYUV2BGR() - vSrc Ptr = NULL \n"); return;
   }
   if ( bgrDst==NULL ) {
      ::fprintf(stderr, "CV4Lwcfilter::CvtIYUV2BGR() - bgr Ptr = NULL \n"); return;
   }

	pDst = bgrDst;
	for (y=0; y<height; y++) // upright map Linux / wx style
//	for (y=height-1; y>=0; y--) // invert map Win style
	{
		pySrc = &ySrc[y*width];
		//U and V are per 4 pixel
		puSrc = &uSrc[(y>>1)*(width>>1)];
		pvSrc = &vSrc[(y>>1)*(width>>1)];
		for (x=0; x<width; x+=2)
		{
			CVT_YCrCb2BGR(pySrc, puSrc, pvSrc, pDst);
			pDst+=3; pySrc++;
			CVT_YCrCb2BGR(pySrc, puSrc, pvSrc, pDst);
			pDst+=3; pySrc++;
			puSrc++; pvSrc++;
		}
	}
} // CvtIYUV2BGR

/* Converts a YUYV frame of width "width and height "height" at "src" straight
 * into a BGR24 frame at "dst" (must be allocated y caller).
*/
void CV4Lwcfilter::CvtYUYV2BGR (long width, long height, wxUint8 *ySrc, wxUint8 *bgrDst)
{
wxUint8 *pDst = bgrDst;
int a=0, i=0, size = width*height;

   _D_ ::fprintf(stderr, "CV4Lwcfilter::CvtYUYV2BGR() - wxh: %ldx%ld\n", width, height);

   if ( ySrc==NULL ) {
      ::fprintf(stderr, "CV4Lwcfilter::CvtYUYV2BGR() - ySrc Ptr = NULL \n"); return;
   }
   if ( bgrDst==NULL ) {
      ::fprintf(stderr, "CV4Lwcfilter::CvtYUYV2BGR() - bgr Ptr = NULL \n"); return;
   }

   while(i++<size){
      if (!a) {
         CVT_YCrCb2BGR(&ySrc[0], &ySrc[1], &ySrc[3], pDst);
         pDst+=3;
      }
      else {
         CVT_YCrCb2BGR(&ySrc[2], &ySrc[1], &ySrc[3], pDst);
         pDst+=3;
      }

      if (a++) {
         a = 0;
         ySrc += 4;
      }
   }
}

/* Converts a YVYU frame of width "width and height "height" at "src" straight
 * into a BGR24 frame at "dst" (must be allocated y caller).
 */
void CV4Lwcfilter::CvtYVYU2BGR (long width, long height, wxUint8 *ySrc, wxUint8 *bgrDst)
{
wxUint8 *pDst = bgrDst;
int a=0, i=0, size = width*height;

   _D_ ::fprintf(stderr, "CV4Lwcfilter::CvtYVYU2BGR() - wxh: %ldx%ld\n", width, height);

   if ( ySrc==NULL ) {
      ::fprintf(stderr, "CV4Lwcfilter::CvtYVYU2BGR() - ySrc Ptr = NULL \n"); return;
   }
   if ( bgrDst==NULL ) {
      ::fprintf(stderr, "CV4Lwcfilter::CvtYVYU2BGR() - bgr Ptr = NULL \n"); return;
   }

   while(i++<size){
      if (!a) {
         CVT_YCrCb2BGR(&ySrc[0], &ySrc[3], &ySrc[1], pDst);
         pDst+=3;
      }
      else {
         CVT_YCrCb2BGR(&ySrc[2], &ySrc[3], &ySrc[1], pDst);
         pDst+=3;
      }

      if (a++) {
         a = 0;
         ySrc += 4;
      }
   }
}

/* Converts a UYVY frame of width "width and height "height" at "src" straight
 * into a BGR24 frame at "dst" (must be allocated y caller).
 */
void CV4Lwcfilter::CvtUYVY2BGR (long width, long height, wxUint8 *ySrc, wxUint8 *bgrDst)
{
wxUint8 *pDst = bgrDst;
int a=0, i=0, size = width*height;

   _D_ ::fprintf(stderr, "CV4Lwcfilter::CvtUYVY2BGR() - wxh: %ldx%ld\n", width, height);

   if ( ySrc==NULL ) {
      ::fprintf(stderr, "CV4Lwcfilter::CvtUYVY2BGR() - ySrc Ptr = NULL \n"); return;
   }
   if ( bgrDst==NULL ) {
      ::fprintf(stderr, "CV4Lwcfilter::CvtUYVY2BGR() - bgr Ptr = NULL \n"); return;
   }

   while(i++<size){
      if (!a) {
         CVT_YCrCb2BGR(&ySrc[1], &ySrc[0], &ySrc[2], pDst);
         pDst+=3;
      }
      else {
         CVT_YCrCb2BGR(&ySrc[3], &ySrc[0], &ySrc[2], pDst);
         pDst+=3;
      }

      if (a++) {
         a = 0;
         ySrc += 4;
      }
   }
}

/* Converts a Y800 frame of width "width and height "height" at "src" straight
 * into a BGR24 frame at "dst" (must be allocated y caller).
 */
void CV4Lwcfilter::CvtY8002BGR (long width, long height, wxUint8 *ySrc, wxUint8 *bgrDst)
{
wxUint8 *pDst = bgrDst;
int i=0, size = width*height;

   _D_ ::fprintf(stderr, "CV4Lwcfilter::CvtY8002BGR() - wxh: %ldx%ld\n", width, height);

   if ( ySrc==NULL ) {
      ::fprintf(stderr, "CV4Lwcfilter::CvtY8002BGR() - ySrc Ptr = NULL \n"); return;
   }
   if ( bgrDst==NULL ) {
      ::fprintf(stderr, "CV4Lwcfilter::CvtY8002BGR() - bgr Ptr = NULL \n"); return;
   }

   while(i++<size){
      *pDst++=*ySrc;*pDst++=*ySrc;*pDst++=*ySrc++;
   }
}

/********************************* COPYRIGHT NOTICE *******************************\
  Original code for Bayer->BGR/RGB conversion is provided by Dirk Schaefer
  from MD-Mathematische Dienste GmbH. Below is the copyright notice:

    IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
    By downloading, copying, installing or using the software you agree
    to this license. If you do not agree to this license, do not download,
    install, copy or use the software.

    Contributors License Agreement:

      Copyright (c) 2002,
      MD-Mathematische Dienste GmbH
      Im Defdahl 5-10
      44141 Dortmund
      Germany
      www.md-it.de

    Redistribution and use in source and binary forms,
    with or without modification, are permitted provided
    that the following conditions are met:

    Redistributions of source code must retain
    the above copyright notice, this list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    The name of Contributor may not be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
    THE POSSIBILITY OF SUCH DAMAGE.
\**********************************************************************************/

// this ist RAW CFA to BGR (Win type) Or RGB (Linux tape)
// depends on bayerType used in call (xxBGR or xxRGB)
void CV4Lwcfilter::CvtBayerGB2BGR (long bayerType, long width, long height,
                                    wxUint8 *ySrc, wxUint8 *bgrDst)
{
	// local setup consts
	int code = bayerType;
	int bayerStep = width; // step i.e. stride is with
	int dstStep = width*3;

    int blue = code == CV_BayerBG2BGR || code == CV_BayerGB2BGR ? -1 : 1;
    int start_with_green = code == CV_BayerGB2BGR || code == CV_BayerGR2BGR;

	register wxUint8 *bayer = ySrc;
	register wxUint8 *dst   = bgrDst;
//BEGIN
/*
	// must make upsidedown image
	dst += (height-1)*dstStep; //move down
	// and decrease dstDteps therefore
    dst += -dstStep + 3 + 1;
*/
	// must make upsright image
	// and increase dstDteps therefore
    dst += +dstStep + 3 + 1;

    // cut boundary
    height -= 2;
    width -= 2;


//    for( ; height--; bayer += bayerStep, dst -= dstStep ) // upsidedown
    for( ; height--; bayer += bayerStep, dst += dstStep )   // upright
    {
        int t0, t1;
        const wxUint8 *bayerEnd = bayer + width;

        if( start_with_green )
        {
            t0 = (bayer[0] + bayer[bayerStep*2] + 1) >> 1;
            t1 = (bayer[bayerStep] + bayer[bayerStep+2] + 1) >> 1;
            dst[-blue] = (wxUint8)t0;
            dst[0] = bayer[bayerStep+1];
            dst[blue] = (wxUint8)t1;
            bayer++;
            dst += 3;
        }

        if( blue > 0 )
        {
            for( ; bayer <= bayerEnd - 2; bayer += 2, dst += 6 )
            {
                t0 = (bayer[0] + bayer[2] + bayer[bayerStep*2] +
                      bayer[bayerStep*2+2] + 2) >> 2;
                t1 = (bayer[1] + bayer[bayerStep] +
                      bayer[bayerStep+2] + bayer[bayerStep*2+1]+2) >> 2;
                dst[-1] = (wxUint8)t0;
                dst[0] = (wxUint8)t1;
                dst[1] = bayer[bayerStep+1];

                t0 = (bayer[2] + bayer[bayerStep*2+2] + 1) >> 1;
                t1 = (bayer[bayerStep+1] + bayer[bayerStep+3] + 1) >> 1;
                dst[2] = (wxUint8)t0;
                dst[3] = bayer[bayerStep+2];
                dst[4] = (wxUint8)t1;
            }
        }
        else
        {
            for( ; bayer <= bayerEnd - 2; bayer += 2, dst += 6 )
            {
                t0 = (bayer[0] + bayer[2] + bayer[bayerStep*2] +
                      bayer[bayerStep*2+2] + 2) >> 2;
                t1 = (bayer[1] + bayer[bayerStep] +
                      bayer[bayerStep+2] + bayer[bayerStep*2+1]+2) >> 2;
                dst[1] = (wxUint8)t0;
                dst[0] = (wxUint8)t1;
                dst[-1] = bayer[bayerStep+1];

                t0 = (bayer[2] + bayer[bayerStep*2+2] + 1) >> 1;
                t1 = (bayer[bayerStep+1] + bayer[bayerStep+3] + 1) >> 1;
                dst[4] = (wxUint8)t0;
                dst[3] = bayer[bayerStep+2];
                dst[2] = (wxUint8)t1;
            }
        }

        if( bayer < bayerEnd )
        {
            t0 = (bayer[0] + bayer[2] + bayer[bayerStep*2] +
                  bayer[bayerStep*2+2] + 2) >> 2;
            t1 = (bayer[1] + bayer[bayerStep] +
                  bayer[bayerStep+2] + bayer[bayerStep*2+1]+2) >> 2;
            dst[-blue] = (wxUint8)t0;
            dst[0] = (wxUint8)t1;
            dst[blue] = bayer[bayerStep+1];
            bayer++;
            dst += 3;
        }
		// rewind pointers to Begin of scanline
        bayer -= width;
        dst -= width*3;

        blue = -blue;
        start_with_green = !start_with_green;
    }

} // CvtBayerGB2BGR


#endif //__UNIX__

