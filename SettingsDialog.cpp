#include "SettingsDialog.h"

#include "SettingsVideoCapture.h"
#include "SettingsControlPort.h"
// #include "SettingsGuidePort.h"  // CA 20080823 "Guide Port" tab removed from "OptionsSettings"
#include "SettingsNamePolicy.h"

#include "wx/bookctrl.h"

BEGIN_EVENT_TABLE(SettingsDialog,wxPropertyPageDialog)
   EVT_BOOKCTRL_PAGE_CHANGED(wxID_ANY, wxPropertyPageDialog::OnPageChanged)
END_EVENT_TABLE()

SettingsDialog::SettingsDialog(wxWindow* parent)
{

    Create(parent, wxID_ANY, _("Settings"), wxDefaultPosition, wxDefaultSize,
        wxDEFAULT_DIALOG_STYLE|wxPROPSHEET_SHRINKTOFIT |wxRESIZE_BORDER
    );

    SettingsVideoCapture* vc = new SettingsVideoCapture(GetBookCtrl());
    SettingsControlPort*  cp = new SettingsControlPort(GetBookCtrl());
  //  SettingsGuidePort*    gp = new SettingsGuidePort(GetBookCtrl());   // CA 20080823 "Guide Port" tab removed from "OptionsSettings"
    SettingsNamePolicy*   cnp = new SettingsNamePolicyContainer(GetBookCtrl());
    SettingsNamePolicy*   fnp = new SettingsNamePolicyFile(GetBookCtrl());

    m_pages.push_back(vc);
    m_pages.push_back(cnp);
    m_pages.push_back(fnp);
    m_pages.push_back(cp);
//     m_pages.push_back(gp);  // CA 20080823 "Guide Port" tab removed from "OptionsSettings"

    GetBookCtrl()->AddPage(vc, wxT("Video Capture"));
    GetBookCtrl()->AddPage(cnp, wxT("Container Name"));
    GetBookCtrl()->AddPage(fnp, wxT("File Name"));
    GetBookCtrl()->AddPage(cp, wxT("LE Control Port"));
 //   GetBookCtrl()->AddPage(gp, wxT("Guide Port"));  // CA 20080823 "Guide Port" tab removed from "OptionsSettings"
    CreateButtons(wxOK|wxCANCEL);

    LayoutDialog();
}

SettingsDialog::~SettingsDialog()
{
   //dtor
}

