// wxWebcamV4L2.cpp: implementation of the wxWebcamV4L2 class.
//
//////////////////////////////////////////////////////////////////////

#include "wxWebcamV4L2.h"

#if defined(__UNIX__)

#include "uxcapture.h"

#include "captypes.h"
#include "capturebase.h"

#include "../GenericDebHelp.h"

IMPLEMENT_DYNAMIC_CLASS(wxWebcamV4L2, wxWebcam)

BEGIN_EVENT_TABLE(wxWebcamV4L2,wxWebcam)
   EVT_PAINT(wxWebcamV4L2::OnDraw)
   EVT_MOUSEWHEEL(wxWebcamV4L2::OnMouseWheel)
   EVT_RIGHT_DCLICK(wxWebcamV4L2::OnMouseDClick)
   EVT_RIGHT_DOWN(wxWebcamV4L2::OnMouseRightDown)
   EVT_RIGHT_UP(wxWebcamV4L2::OnMouseRightUp)
   EVT_LEFT_DOWN(wxWebcamV4L2::OnMouseLeftDown)
   EVT_LEFT_UP(wxWebcamV4L2::OnMouseLeftUp)
   EVT_MOTION(wxWebcamV4L2::OnMouseMove)
   EVT_ENTER_WINDOW(wxWebcamV4L2::OnMouseEnterWindow)
   EVT_LEAVE_WINDOW(wxWebcamV4L2::OnMouseLeaveWindow)
END_EVENT_TABLE()



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

wxWebcamV4L2::wxWebcamV4L2(wxWindow* parent, wxWindowID id)
: wxWebcam(parent,id)
{

   // here we are OS specific
   m_ds = new CUXcapture();
   m_pProperties = new DeviceProperties(this);
}

wxWebcamV4L2::~wxWebcamV4L2()
{
   // disconnect the camera
   StopCapture();
   DisconnectCamera();
}


wxString wxWebcamV4L2::VideoTechnology() const
{
   return wxT("V4L2");
}

wxImage* wxWebcamV4L2::liveImage() const
{
   return frameImage();
}

//* void wxWebcamV4L2::OnTimer(wxTimerEvent& event)
void wxWebcamV4L2::TimerEvent()
{
   float fps, dps;
   if ( m_ds->GetRates(fps, dps) ) {
      m_fps = fps; m_dps = dps;
   }

   // generic timer handling
   wxWebcam::TimerEvent();
}

bool wxWebcamV4L2::HasDriverDialogs() const
{
   return false;
}

void wxWebcamV4L2::ShowDisplayDialog()
{
}

void wxWebcamV4L2::ShowFormatDialog()
{
   wxString msg = _("This feature does not exist on this platform. Please use \"Video Properties\" instead.");
   wxMessageBox(msg, _("Driver Format Dialog"));
}

void wxWebcamV4L2::ShowSourceDialog()
{
   wxString msg = _("This feature does not exist on this platform. Please use \"Video Properties\" instead.");
   wxMessageBox(msg, _("Driver Source Dialog"));
}

#endif

