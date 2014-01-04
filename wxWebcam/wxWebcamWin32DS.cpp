// wxWebcamWin32DS.cpp: implementation of the wxWebcamWin32DS class.
//
//////////////////////////////////////////////////////////////////////


#include "wxWebcamWin32DS.h"
#if defined(_WXMSW_ )

//*#define ID_TIMER           10005

// interface for CDScapture, our interface to DirectShow
#include "dscapture.h"

#include "captypes.h"
//#include "avi/avilib.h"

#include "../GenericDebHelp.h"

IMPLEMENT_DYNAMIC_CLASS(wxWebcamWin32DS, wxWebcam)

BEGIN_EVENT_TABLE(wxWebcamWin32DS,wxWebcam)
   EVT_SIZE(wxWebcamWin32DS::OnSize)
   EVT_PAINT(wxWebcamWin32DS::OnDraw)
   EVT_MOUSEWHEEL(wxWebcamWin32DS::OnMouseWheel)
   EVT_RIGHT_DCLICK(wxWebcamWin32DS::OnMouseDClick)
   EVT_RIGHT_DOWN(wxWebcamWin32DS::OnMouseRightDown)
   EVT_RIGHT_UP(wxWebcamWin32DS::OnMouseRightUp)
   EVT_LEFT_DOWN(wxWebcamWin32DS::OnMouseLeftDown)
   EVT_LEFT_UP(wxWebcamWin32DS::OnMouseLeftUp)
   EVT_MOTION(wxWebcamWin32DS::OnMouseMove)
   EVT_ENTER_WINDOW(wxWebcamWin32DS::OnMouseEnterWindow)
   EVT_LEAVE_WINDOW(wxWebcamWin32DS::OnMouseLeaveWindow)
END_EVENT_TABLE()

// undef USE_COLLECT to use plain stream for AVI capture
// #define USE_COLLECT
// if USE_COLLECT is defined the follwing parameter are used

// defined how many images are sampled before returning one image
#define COLLECT_NUM     3
// defines the algorith to be used when returning the sampled image
#define COLLECT_MATH    EMK_Mean
// other options are
//#define COLLECT_MATH    EMK_Stretch
//#define COLLECT_MATH    EMK_Log
//#define COLLECT_MATH    EMK_HistStretch
//#define COLLECT_MATH    EMK_DropStretch


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

wxWebcamWin32DS::wxWebcamWin32DS(wxWindow* parent, wxWindowID id)
: wxWebcam(parent,id)
{
    // these two belong wxWebcam and are initialized here in the WIN instance
    // will be dropped only by wxWebcam
    m_ds = new CDScapture();
    m_pProperties = new DeviceProperties(this);
}

wxWebcamWin32DS::~wxWebcamWin32DS()
{
   // disconnect the camera
   StopCapture();
   DisconnectCamera();
}

bool wxWebcamWin32DS::HasDriverDialogs() const
{
   return true;
}


void wxWebcamWin32DS::ShowDisplayDialog()
{
}

void wxWebcamWin32DS::ShowFormatDialog()
{
   // manufacturers Format dialog
   m_ds->ShowFormatDlg((long)m_parent->GetHWND());
}

void wxWebcamWin32DS::ShowSourceDialog()
{
   // manufacturers Source dialog
   m_ds->ShowSourceDlg((long)m_parent->GetHWND());
}

wxString wxWebcamWin32DS::VideoTechnology() const
{
   return wxT("WDM");
}


void wxWebcamWin32DS::TimerEvent()
{
   float fps, dps;
   if ( m_ds->GetRates(fps, dps) ) {
      m_fps = fps; m_dps = dps;
   }

   // preview is shown only in NON LE mode
   // TODO make this not called every tic
//   m_ds->VideoOutputVisible(m_preview);
   m_ds->VideoOutputVisible(false);

   // generic timer handling
   wxWebcam::TimerEvent();
}

void wxWebcamWin32DS::OnSize(wxSizeEvent& WXUNUSED(event))
{
    m_ds->ScaleVideoOutput();
}


// This is the Win specific implementation
// for drawing overlay decorations
//  it is specific because in the streamed image must be drawn differently
void wxWebcamWin32DS::OnDrawOverlay(bool in_paint_event)
{
//   if(!m_isLExposing) {
   if(false) {

      // in normal streaming (non LE) mode, we use an alternate method of drawing overlay symbols

      // this draws into the streaming image via the WDM filter in the dscaplib.dll
      CDScapture* win_ds = (CDScapture*)ds();
      if(win_ds) {
         win_ds->Clear();

         unsigned short width,height;
         win_ds->FrameDimension(width,height);

         if(m_overlayGuideBoxDraw) {
            const unsigned long white = 0x00ffffff;
            win_ds->SetPen(white, 1);

            // the rectangle coordinates must be slightly tweaked
            long x = m_overlayGuideBox.GetX();
            long y = m_overlayGuideBox.GetY();
            long w = m_overlayGuideBox.GetWidth();
            long h = m_overlayGuideBox.GetHeight();
            win_ds->DrawRectangle(x,y,w,h);

            long y0 = m_overlayGuideLine[0].y;
            long y1 = m_overlayGuideLine[1].y;
            win_ds->DrawLine(m_overlayGuideLine[0].x,y0,
                             m_overlayGuideLine[1].x,y1);
         }
         if(isReticleEnabled()) {
            // compute vital dimensions for drawing reticle
            short h2=height*0.5, w2=width*0.5;
            short diam = (h2 < w2)? h2 : w2;
            short rrad1 = diam/3;
            short rrad2 = rrad1*2;
            short cx = width/2;
            short cy = height/2;

            // actually draw the reticle
            const unsigned long red = 0x00ff0000;
            win_ds->SetPen(red, 1);
            win_ds->DrawCircle(cx, cy, rrad1);
            win_ds->DrawCircle(cx, cy, rrad2);
            win_ds->DrawLine(cx-w2, cy, cx+w2, cy);
            win_ds->DrawLine(cx, cy-h2, cx, cy+h2);
         }

      }
   }
   else {

      // use standard implementation in LE mode
      wxWebcam::OnDrawOverlay(in_paint_event);
   }
}

#endif  // _WXMSW_

