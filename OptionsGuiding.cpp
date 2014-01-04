#include "OptionsGuiding.h"
#include "SettingsGuiding.h"
#include "SettingsGuidingInterface.h"
#include "SettingsGuidePort.h"
#include <wx/bookctrl.h>

BEGIN_EVENT_TABLE(OptionsGuiding,wxPropertyPageDialog)
   EVT_COMMAND(wxID_OK,wxEVT_COMMAND_BUTTON_CLICKED,OptionsGuiding::OnOk)
   EVT_COMMAND(wxID_CANCEL,wxEVT_COMMAND_BUTTON_CLICKED,OptionsGuiding::OnCancel)
   EVT_CLOSE(OptionsGuiding::OnClose)
 	EVT_BOOKCTRL_PAGE_CHANGED(wxID_ANY, wxPropertyPageDialog::OnPageChanged)
END_EVENT_TABLE()


OptionsGuiding* OptionsGuiding::m_self = 0;
OptionsGuiding* OptionsGuiding::singleton()
{
   return m_self;
}

OptionsGuiding::OptionsGuiding(wxWindow* parent)
{

   Create(parent, wxID_ANY, _("Guiding Options"), wxDefaultPosition, wxDefaultSize,
     wxDEFAULT_DIALOG_STYLE|wxPROPSHEET_SHRINKTOFIT |wxRESIZE_BORDER
   );

   SettingsGuiding* sg = new SettingsGuiding(GetBookCtrl());
   m_pages.push_back(sg );
   GetBookCtrl()->AddPage(sg, wxT("Calibration"));

   SettingsGuidingInterface* sgi = new SettingsGuidingInterface(GetBookCtrl());
   SettingsGuidePort*         gp = new SettingsGuidePort(GetBookCtrl());
   m_pages.push_back(sgi );
   m_pages.push_back(gp );
   GetBookCtrl()->AddPage(sgi, wxT("Interface"));
   GetBookCtrl()->AddPage(gp , wxT("Guide Port"));
   // The really simple way to create a "Cancel" button (with icon on Linux)
   CreateButtons(wxOK|wxCANCEL);

   LayoutDialog();
   m_self = this;
}

OptionsGuiding::~OptionsGuiding()
{
   m_self = 0;
}

