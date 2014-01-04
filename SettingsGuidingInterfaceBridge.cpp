#include "SettingsGuidingInterfaceBridge.h"

//(*InternalHeaders(SettingsGuidingInterfaceBridge)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/msgdlg.h>

#include "wxGuidingConfig.h"

//(*IdInit(SettingsGuidingInterfaceBridge)
const long SettingsGuidingInterfaceBridge::ID_STATICTEXT1 = wxNewId();
const long SettingsGuidingInterfaceBridge::ID_TEXTCTRL1 = wxNewId();
const long SettingsGuidingInterfaceBridge::ID_STATICTEXT2 = wxNewId();
const long SettingsGuidingInterfaceBridge::ID_TEXTCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SettingsGuidingInterfaceBridge,wxPanel)
	//(*EventTable(SettingsGuidingInterfaceBridge)
	//*)
END_EVENT_TABLE()

SettingsGuidingInterfaceBridge::SettingsGuidingInterfaceBridge(wxWindow* parent,wxWindowID id)
{
	//(*Initialize(SettingsGuidingInterfaceBridge)
	wxStaticText* StaticText2;
	wxStaticText* StaticText1;
	wxBoxSizer* BoxSizer2;
	wxBoxSizer* BoxSizer1;
	wxBoxSizer* BoxSizer3;
	
	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Host"), wxDefaultPosition, wxSize(50,-1), 0, _T("ID_STATICTEXT1"));
	BoxSizer2->Add(StaticText1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrlHost = new wxTextCtrl(this, ID_TEXTCTRL1, _("localhost"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	BoxSizer2->Add(TextCtrlHost, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Port"), wxDefaultPosition, wxSize(50,-1), 0, _T("ID_STATICTEXT2"));
	BoxSizer3->Add(StaticText2, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrlPort = new wxTextCtrl(this, ID_TEXTCTRL2, _("5618"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	BoxSizer3->Add(TextCtrlPort, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer3, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
	//*)

	Init();
}

SettingsGuidingInterfaceBridge::~SettingsGuidingInterfaceBridge()
{
	//(*Destroy(SettingsGuidingInterfaceBridge)
	//*)
}

void SettingsGuidingInterfaceBridge::Init()
{
   // get values from config
   wxString host;
   short port;
   wxGuidingConfig::getBridgeParameters(host,port);
   TextCtrlHost->SetValue(host);

   wxString txt;
   txt.Printf(_T("%d"),port);
   TextCtrlPort->SetValue(txt);
}


void SettingsGuidingInterfaceBridge::SaveToConfig()
{
   wxString port = TextCtrlPort->GetValue();

   unsigned long iport=0;
   if(port.ToULong(&iport)) {
     short sport = iport;
     wxGuidingConfig::saveBridgeParameters(TextCtrlHost->GetValue(),sport);
   }
   else {
      wxMessageBox(port,_T("Port must be a 4 digit number"));
   }
}
