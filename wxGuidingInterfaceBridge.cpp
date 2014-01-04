#include "wxGuidingInterfaceBridge.h"

//(*InternalHeaders(wxGuidingInterfaceBridge)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "wxGuidingConfig.h"
#include "wxFactory.h"

#include <wx/msgdlg.h>

//(*IdInit(wxGuidingInterfaceBridge)
const long wxGuidingInterfaceBridge::ID_STATICTEXT1 = wxNewId();
const long wxGuidingInterfaceBridge::ID_TEXTCTRL1 = wxNewId();
const long wxGuidingInterfaceBridge::ID_STATICTEXT2 = wxNewId();
const long wxGuidingInterfaceBridge::ID_TEXTCTRL2 = wxNewId();
const long wxGuidingInterfaceBridge::ID_PANEL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(wxGuidingInterfaceBridge,wxPanel)
	//(*EventTable(wxGuidingInterfaceBridge)
	//*)
END_EVENT_TABLE()

wxGuidingInterfaceBridge::wxGuidingInterfaceBridge(wxWindow* parent,wxWindowID id,const wxPoint& WXUNUSED(pos),const wxSize& WXUNUSED(size))
{
	//(*Initialize(wxGuidingInterfaceBridge)
	wxStaticText* StaticText2;
	wxStaticText* StaticText1;
	wxBoxSizer* BoxSizer2;
	wxBoxSizer* BoxSizer1;
	wxBoxSizer* BoxSizer3;
	
	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	Panel1 = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	StaticText1 = new wxStaticText(Panel1, ID_STATICTEXT1, _("Host"), wxDefaultPosition, wxSize(50,-1), 0, _T("ID_STATICTEXT1"));
	BoxSizer2->Add(StaticText1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrlHost = new wxTextCtrl(Panel1, ID_TEXTCTRL1, _("localhost"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	BoxSizer2->Add(TextCtrlHost, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
	StaticText2 = new wxStaticText(Panel1, ID_STATICTEXT2, _("Port"), wxDefaultPosition, wxSize(50,-1), 0, _T("ID_STATICTEXT2"));
	BoxSizer3->Add(StaticText2, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrlPort = new wxTextCtrl(Panel1, ID_TEXTCTRL2, _("5618"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	BoxSizer3->Add(TextCtrlPort, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer3, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel1->SetSizer(BoxSizer1);
	BoxSizer1->Fit(Panel1);
	BoxSizer1->SetSizeHints(Panel1);
	//*)

	Init();
}

wxGuidingInterfaceBridge::~wxGuidingInterfaceBridge()
{
	//(*Destroy(wxGuidingInterfaceBridge)
	//*)
}

void wxGuidingInterfaceBridge::Init()
{
   SetName(wxT("Bridge Interface"));

   InitBridgeParameters();
}


void wxGuidingInterfaceBridge::InitBridgeParameters()
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


void wxGuidingInterfaceBridge::OnApply()
{


   wxString port = TextCtrlPort->GetValue();

   unsigned long iport=0;
   if(port.ToULong(&iport)) {

      // enforce bridge interface
      wxF()->setGuiderInterface(wxGuider::BRIDGE_INTERFACE);
      short sport = iport;
      wxGuidingConfig::saveBridgeParameters(TextCtrlHost->GetValue(),sport);
   }
   else {
      wxMessageBox(port,_T("Port must be a 4 digit number"));
   }
}
