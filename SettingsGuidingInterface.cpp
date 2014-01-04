#include "SettingsGuidingInterface.h"

//(*InternalHeaders(SettingsGuidingInterface)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/dirdlg.h>
#include "wxGuidingConfig.h"
#include "wxFactory.h"
#include "wxGuider.h"

#include "SettingsGuidingInterfacePort.h"
#include "SettingsGuidingInterfaceFile.h"
#include "SettingsGuidingInterfaceBridge.h"

//(*IdInit(SettingsGuidingInterface)
const long SettingsGuidingInterface::ID_CHOICE1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SettingsGuidingInterface,wxPanel)
	//(*EventTable(SettingsGuidingInterface)
	//*)
END_EVENT_TABLE()

SettingsGuidingInterface::SettingsGuidingInterface(wxWindow* parent,wxWindowID id)
: m_portParams(0)
, m_fileParams(0)
, m_bridgeParams(0)
{
	//(*Initialize(SettingsGuidingInterface)
	wxStaticBoxSizer* StaticBoxSizer2;
	wxStaticBoxSizer* StaticBoxSizer1;
	
	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Interface type"));
	InterfaceChoice = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxSize(200,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	InterfaceChoice->Append(_("File Interface"));
	InterfaceChoice->Append(_("Port Interface"));
	InterfaceChoice->Append(_("Bridge Interface"));
	StaticBoxSizer1->Add(InterfaceChoice, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(StaticBoxSizer1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer2 = new wxStaticBoxSizer(wxVERTICAL, this, _("Interface parameters"));
	BoxSizerParameters = new wxBoxSizer(wxVERTICAL);
	StaticBoxSizer2->Add(BoxSizerParameters, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	BoxSizer1->Add(StaticBoxSizer2, 3, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
	
	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SettingsGuidingInterface::OnInterfaceChoiceSelect);
	//*)

	Init();
}

SettingsGuidingInterface::~SettingsGuidingInterface()
{
	//(*Destroy(SettingsGuidingInterface)
	//*)
}

void SettingsGuidingInterface::Init()
{

   m_portParams   = new SettingsGuidingInterfacePort(this);
   m_fileParams   = new SettingsGuidingInterfaceFile(this);
   m_bridgeParams = new SettingsGuidingInterfaceBridge(this);

   wxGuider::guider_interface intfc = wxF()->guider()->interface_type();
   InterfaceChoice->SetSelection(intfc);
   ShowParams(intfc);

   SetInit(true);
}

bool SettingsGuidingInterface::OnApply()
{
   wxGuider::guider_interface intfce = wxGuider::guider_interface(InterfaceChoice->GetSelection());
   wxF()->setGuiderInterface(intfce);

   switch(intfce) {
      case wxGuider::FILE_INTERFACE: { m_fileParams->SaveToConfig(); break; }
      case wxGuider::PORT_INTERFACE: { m_portParams->SaveToConfig(); break; }
      case wxGuider::BRIDGE_INTERFACE: { m_bridgeParams->SaveToConfig(); break; }
      default: {}
   }
   return true;
}


void SettingsGuidingInterface::OnInterfaceChoiceSelect(wxCommandEvent& WXUNUSED(event))
{
   int guide_interface = InterfaceChoice->GetSelection();

   // 0 = File Interface
   // 1 = Port Interface
   // 2 = Bridge Interface
   ShowParams(guide_interface);
}

void SettingsGuidingInterface::ShowParams(int type)
{
    BoxSizerParameters->Clear();
    wxWindow* params = 0;
    switch(type) {
       case 0:
          {
            m_fileParams->Show(true);
            m_portParams->Show(false);
            m_bridgeParams->Show(false);
            params = m_fileParams;
			break;
          }
       case 1:
          {
            m_fileParams->Show(false);
            m_portParams->Show(true);
            m_bridgeParams->Show(false);
            params = m_portParams;
			break;
          }
       case 2:
          {
            m_fileParams->Show(false);
            m_portParams->Show(false);
            m_bridgeParams->Show(true);
            params = m_bridgeParams;
			break;
          }
       default:
         {
            m_fileParams->Show(false);
            m_portParams->Show(false);
            m_bridgeParams->Show(false);
            params = 0;
          }
    };

    if(params)BoxSizerParameters->Add(params,3,wxALL|wxEXPAND|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL,0);
    BoxSizerParameters->Layout();
    BoxSizer1->Layout();
    Layout();
}
