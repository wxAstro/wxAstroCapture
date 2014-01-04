// wxWebcamUnix.h: interface for the wxWebcamUnix class.
//
//////////////////////////////////////////////////////////////////////

#ifndef WXWEBCAMV4L2_H
#define WXWEBCAMV4L2_H

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if defined(__UNIX__)

#include "captypes.h"
#include "wxWebcam.h"


class wxImage;

class wxWebcamV4L2 : public wxWebcam   {

public:
   wxWebcamV4L2(): wxWebcam(){};
   wxWebcamV4L2(wxWindow* parent, wxWindowID id= -1);
   virtual ~wxWebcamV4L2();


   // OS dependent items
   wxString VideoTechnology() const;

   wxImage* liveImage() const;

   // regular timer event at the nominal pace
   void TimerEvent();

private:
   bool HasDriverDialogs() const;
   void ShowDisplayDialog();
   void ShowFormatDialog();
   void ShowSourceDialog();


private:
   DECLARE_EVENT_TABLE()
   DECLARE_DYNAMIC_CLASS(wxWebcamV4L2)
};


#endif  // __UNIX__

#endif  //WXWEBCAMV4L2_H
