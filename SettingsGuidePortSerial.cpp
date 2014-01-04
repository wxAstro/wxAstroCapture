#include "SettingsGuidePortSerial.h"

//(*InternalHeaders(SettingsGuidePortSerial)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "wxFactory.h"


//(*IdInit(SettingsGuidePortSerial)
const long SettingsGuidePortSerial::ID_STATICTEXT1 = wxNewId();
const long SettingsGuidePortSerial::ID_CHOICE1 = wxNewId();
const long SettingsGuidePortSerial::ID_STATICTEXT2 = wxNewId();
const long SettingsGuidePortSerial::ID_CHOICE2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SettingsGuidePortSerial,wxPanel)
	//(*EventTable(SettingsGuidePortSerial)
	//*)
END_EVENT_TABLE()

SettingsGuidePortSerial::SettingsGuidePortSerial(wxWindow* parent,wxWindowID id)
{
	//(*Initialize(SettingsGuidePortSerial)
	wxStaticText* StaticText2;
	wxStaticText* StaticText1;
	wxBoxSizer* BoxSizer2;
	wxBoxSizer* BoxSizer3;
	
	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	BoxSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Baudrate"), wxDefaultPosition, wxSize(70,-1), 0, _T("ID_STATICTEXT1"));
	BoxSizer2->Add(StaticText1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BaudRateChoice = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxSize(100,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	BaudRateChoice->Append(_("9600"));
	BoxSizer2->Add(BaudRateChoice, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer2, 1, wxTOP|wxBOTTOM|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Handshake"), wxDefaultPosition, wxSize(70,-1), 0, _T("ID_STATICTEXT2"));
	BoxSizer3->Add(StaticText2, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	HandShakeChoice = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxSize(100,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	HandShakeChoice->Append(_("None"));
	HandShakeChoice->Append(_("XON/XOFF"));
	HandShakeChoice->Append(_("RTS/CTS"));
	BoxSizer3->Add(HandShakeChoice, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer3, 1, wxTOP|wxBOTTOM|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
	//*)

	Init();
}

SettingsGuidePortSerial::~SettingsGuidePortSerial()
{
	//(*Destroy(SettingsGuidePortSerial)
	//*)
}

void SettingsGuidePortSerial::Init()
{

   // set up baudrate
   int baudrate = 1200;
   BaudRateChoice->Clear();
   wxGuiderPort::EPGBaudRate ebaudrate = wxGuiderPort::EPGB_1200;
   while(ebaudrate != wxGuiderPort::EPGB_LAST) {
      ebaudrate = wxGuiderPort::EPGBaudRate(ebaudrate+1);

      wxString txt;
      txt.Printf(wxT("%d"),baudrate);
      baudrate *= 2;
      BaudRateChoice->Append(txt);
   }
   BaudRateChoice->Select(0);
   HandShakeChoice->Select(0);

   // skip if not port interface active
   if(wxGuider* guider = wxF()->guider()) {
      if(guider->interface_type() != wxGuider::PORT_INTERFACE)return;
   }

   wxGuiderPort* gport = wxF()->guidePort();
   BaudRateChoice->Select(gport->SerialBaudrate());
   HandShakeChoice->Select(gport->SerialHandshake());
}


wxGuiderPort::EPGBaudRate  SettingsGuidePortSerial::baudrate()
{
   return wxGuiderPort::EPGBaudRate(BaudRateChoice->GetSelection());
}

wxGuiderPort::EPGHandshake SettingsGuidePortSerial::handshake()
{
   return wxGuiderPort::EPGHandshake(HandShakeChoice->GetSelection());
}
