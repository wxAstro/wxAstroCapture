#include "wxWebcamProps.h"
#include "wx/bookctrl.h"
#include "PrimaryProps.h"
#include "SecondaryProps.h"
#include "DefaultProps.h"
#include "LoadSaveProps.h"
#include "ArtemisProps.h"
#include "ArtemisCooling.h"
#include "wxFactory.h"
#include "wxWebcam/wxWebcam.h"
#include "wxWebcam/wxArtcam.h"

//#include <wx/sizer.h>

BEGIN_EVENT_TABLE(wxWebcamProps,wxPropertySheetDialog)
   EVT_CLOSE(wxWebcamProps::OnClose)
END_EVENT_TABLE()


wxWebcamProps* wxWebcamProps::m_self = 0;

wxWebcamProps::wxWebcamProps(wxWindow* parent)
{
   //ctor
   wxString title = _("Video Properties");
   bool is_artemis = false;
   bool has_cooling = false;
   if(wxCamera* camera = wxFactory::singleton()->cam()) {
      title = camera->CameraName();
      is_artemis = camera->IsArtcam();
      if(camera->HasCooling())has_cooling=true;
   }

   Create(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize,
     wxDEFAULT_DIALOG_STYLE|wxPROPSHEET_SHRINKTOFIT |wxRESIZE_BORDER
   );

   if(is_artemis) {
      GetBookCtrl()->AddPage(new ArtemisProps(GetBookCtrl()), wxT("Display"));
      if(has_cooling) {
         GetBookCtrl()->AddPage(new ArtemisCooling(GetBookCtrl()), wxT("Cooling"));
      }
   }
   else {
      GetBookCtrl()->AddPage(new PrimaryProps(GetBookCtrl()), wxT("Exposure"));
      GetBookCtrl()->AddPage(new SecondaryProps(GetBookCtrl()), wxT("Colour ++"));
      GetBookCtrl()->AddPage(new DefaultProps(GetBookCtrl()), wxT("Presets"));
      GetBookCtrl()->AddPage(new LoadSaveProps(GetBookCtrl()), wxT("Save/Restore"));
   }

   // The really simple way to create a "Cancel" button (with icon on Linux)
   //CreateButtons(wxCANCEL);

   // The more complex alternative,
   // create a button that says "Close" instead of "Cancel"
   // No icon with this button
   const long ID_CLOSE_BUTTON = wxNewId();
   wxStdDialogButtonSizer* sizer = new wxStdDialogButtonSizer();
   wxButton* close = new wxButton(this, ID_CLOSE_BUTTON,wxT("Close"));
   sizer->SetCancelButton(close);
   sizer->Realize();
   GetInnerSizer()->Add( sizer, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT|wxRIGHT, 2);
   GetInnerSizer()->AddSpacer(2); // space below button
   Connect(ID_CLOSE_BUTTON,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&wxWebcamProps::OnClose);

   LayoutDialog();
   m_self = this;


   // set the window positio to what it used to be last time
   long pos_x=0,pos_y=0;
   if(wxFactory::singleton()->config()->Read(_T("PropsPos_x"),&pos_x)
   && wxFactory::singleton()->config()->Read(_T("PropsPos_y"),&pos_y)) {

      long screen_x = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
      long screen_y = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);
      if((pos_x>0 && pos_x<screen_x) && (pos_y>0 && pos_y<screen_y)) {
         Move(wxPoint(pos_x,pos_y));
      }
   }
}

wxWebcamProps::~wxWebcamProps()
{
    //dtor

    m_self = 0;
}

void wxWebcamProps::OnClose(wxCloseEvent& WXUNUSED(event))
{
   int fx(0),fy(0);
   GetScreenPosition(&fx,&fy);
   if(fx > -1 && fy > -1) {
      long pos_x(fx),pos_y(fy);
      wxFactory::singleton()->config()->Write(_T("PropsPos_x"),pos_x);
      wxFactory::singleton()->config()->Write(_T("PropsPos_y"),pos_y);
   }

   // self-destruct the object when dialog is closed
   Destroy();
}

wxWebcamProps* wxWebcamProps::singleton()
{
   return m_self;
}
