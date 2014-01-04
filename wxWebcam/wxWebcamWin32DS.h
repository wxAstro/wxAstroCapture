// wxWebcamWin32DS.h: interface for the wxWebcamWin32DS class.
//
//////////////////////////////////////////////////////////////////////

#ifndef wxWebcamWin32DS_H
#define wxWebcamWin32DS_H

#include "wxWebcam.h"

#if defined(_WXMSW_ )

#include "captypes.h"


class CDScapture;
class CAviLib;
class CAviBuffer;

class wxWebcamWin32DS : public wxWebcam  {
   friend class DeviceProperties;
public:
    wxWebcamWin32DS() : wxWebcam() {}
    wxWebcamWin32DS(wxWindow* parent, wxWindowID id= -1);
	virtual ~wxWebcamWin32DS();

   // OS dependent items
   wxString VideoTechnology() const;

   // regular timer event at the nominal pace
   void TimerEvent();

   // called to resize the window
   void OnSize(wxSizeEvent& event);

   // overrides for drawing reticle on Windows/DirectShow
   void OnDrawOverlay(bool in_paint_event);

private:
   bool HasDriverDialogs() const;
   void ShowDisplayDialog();
   void ShowFormatDialog();
   void ShowSourceDialog();

private:
   DECLARE_EVENT_TABLE()
   DECLARE_DYNAMIC_CLASS(wxWebcamWin32DS)
};

#endif  //_WXMSW_

#endif  // wxWebcamWin32DS_H
