#include "wxGuidingInterfaceSerialPort.h"

//(*InternalHeaders(wxGuidingInterfaceSerialPort)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "wxIO/wxGuiderPort.h"
#include "wxFactory.h"
#include "wxWebcam/wxWebcam.h"

//(*IdInit(wxGuidingInterfaceSerialPort)
const long wxGuidingInterfaceSerialPort::ID_STATICTEXT8 = wxNewId();
const long wxGuidingInterfaceSerialPort::ID_CHOICE3 = wxNewId();
const long wxGuidingInterfaceSerialPort::ID_STATICTEXT1 = wxNewId();
const long wxGuidingInterfaceSerialPort::ID_CHOICE1 = wxNewId();
const long wxGuidingInterfaceSerialPort::ID_STATICTEXT2 = wxNewId();
const long wxGuidingInterfaceSerialPort::ID_CHOICE2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(wxGuidingInterfaceSerialPort,wxPanel)
	//(*EventTable(wxGuidingInterfaceSerialPort)
	//*)
END_EVENT_TABLE()

wxGuidingInterfaceSerialPort::wxGuidingInterfaceSerialPort(wxWindow* parent,wxWindowID id,const wxPoint&  WXUNUSED(pos),const wxSize& WXUNUSED(size))
{
	//(*Initialize(wxGuidingInterfaceSerialPort)
	wxBoxSizer* BoxSizer4;
	wxStaticText* StaticText2;
	wxStaticText* StaticText8;
	wxStaticText* StaticText1;
	wxBoxSizer* BoxSizer2;
	wxBoxSizer* BoxSizer3;
	
	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	BoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Protocol"), wxDefaultPosition, wxSize(70,-1), 0, _T("ID_STATICTEXT8"));
	BoxSizer4->Add(StaticText8, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ProtocolChoice = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxSize(160,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
	BoxSizer4->Add(ProtocolChoice, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer4, 0, wxTOP|wxBOTTOM|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Baudrate"), wxDefaultPosition, wxSize(70,-1), 0, _T("ID_STATICTEXT1"));
	BoxSizer2->Add(StaticText1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BaudRateChoice = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxSize(160,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	BaudRateChoice->Append(_("9600"));
	BoxSizer2->Add(BaudRateChoice, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer2, 0, wxTOP|wxBOTTOM|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Handshake"), wxDefaultPosition, wxSize(70,-1), 0, _T("ID_STATICTEXT2"));
	BoxSizer3->Add(StaticText2, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	HandShakeChoice = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxSize(160,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	HandShakeChoice->Append(_("None"));
	HandShakeChoice->Append(_("XON/XOFF"));
	HandShakeChoice->Append(_("RTS/CTS"));
	BoxSizer3->Add(HandShakeChoice, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer3, 0, wxTOP|wxBOTTOM|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
	//*)

	Init();
}

wxGuidingInterfaceSerialPort::~wxGuidingInterfaceSerialPort()
{
	//(*Destroy(wxGuidingInterfaceSerialPort)
	//*)
}


void wxGuidingInterfaceSerialPort::Init()
{
   InitProtocolChoice();
   InitPortParameters();
}


void wxGuidingInterfaceSerialPort::InitProtocolChoice()
{
   ProtocolChoice->Clear();

   // filter out and show the protocols that are relevant for the port type
   wxGuiderPortProtocol::EPGProtocol protocol = wxGuiderPortProtocol::EPGP_None;
   while(protocol != wxGuiderPortProtocol::EPGP_LAST) {

      if(wxGuiderPortProtocol::SupportsSerial(protocol)) {
         ProtocolChoice->Append(wxGuiderPortProtocol::ProtocolName(protocol));
      }
      protocol = wxGuiderPortProtocol::EPGProtocol(protocol+1);
   }

   // just to make sure we have a default
   ProtocolChoice->SetSelection(0);

   if(wxGuider* guider = wxF()->guider()) {
      // if we already have a serial guide port, use its values as defaults
      wxGuiderPort* gport_f = wxF()->guidePort();
      if(gport_f && gport_f->IsPortSerial()) {
         ProtocolChoice->SetStringSelection(gport_f->ProtocolName());
      }
   }
}


void wxGuidingInterfaceSerialPort::InitPortParameters()
{
   // set up baudrate
   // Handshake choices are edited into the control
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

   if(wxGuiderPort* gport = wxF()->guidePort()) {
      BaudRateChoice->Select(gport->SerialBaudrate());
      HandShakeChoice->Select(gport->SerialHandshake());
   }
}


void wxGuidingInterfaceSerialPort::OnApply()
{
   ApplyPort();
}


bool wxGuidingInterfaceSerialPort::ApplyPort()
{
   // this is a helper function, to be called ONLY from OnApply()
   // Here we apply the GUI settings to the system

   // Force use of the port interface
   wxF()->setGuiderInterface(wxGuider::PORT_INTERFACE);

   // the port name is given in the page name
   wxString guide_port_name = GetName();

   // get the chosen protocol name
   int isel = ProtocolChoice->GetSelection(); // choice can be empty !!
   wxString protocol_name =  wxT("");
   if(isel>=0)protocol_name = ProtocolChoice->GetString(isel);

   // port chosen - use this one
   const wxPortDescr& port_entry = wxF()->portManager()->PortDescriptor(guide_port_name);
   if (! wxF()->guidePort()->UseGXPort(port_entry, wxGuiderPortProtocol::ProtocolEnum(protocol_name)) ) {

      // using the None protocol when somehow the given protocol was not accepted
      // this should really never happen, because the other events ensure compatible settings
      wxF()->guidePort()->UseGXPort(port_entry, wxGuiderPortProtocol::EPGP_None);
      wxString message = wxT("Failed to setup guider port ");
      message += guide_port_name;
      message += wxT(" using ");
      message += protocol_name;
      wxMessageBox(message);
      return false;
   }

   // The port is now active
   // Get the port and set the serial port values
   wxGuiderPort* gport = wxF()->guidePort();

   if(gport->IsPortSerial()) {
      gport->SetGXDefault();
      gport->SetSerialHandshake(handshake());
      gport->SetSerialBaudrate(baudrate());
   }
   else {
      wxMessageBox(wxT("Programming error: Guide port is not serial type!!!"));
   }

   // finished setting up the bits..
   // NOTE CommitConfiguration() does NOT release the Port !!!
   if ( ! gport->CommitConfiguration() ) {
      wxMessageBox(_T("Cannot configure guider control port ") + gport->PortName() \
                                 + _T(", please check port settings."));
   }

   // apply from guide port to config
   gport->SaveToConfigGuidePort(wxF()->config());

   // unless there is an ongoing guiding session, we release the guide port here
   // so it is available for other programs (GOTO etc):
   bool release = true;
   wxWebcam* webcam = wxF()->webcam();
   if(webcam) {
      release = !webcam->isGuiding();
   }
   if(release)gport->ReleasePort();

   return true;
}

wxGuiderPort::EPGBaudRate  wxGuidingInterfaceSerialPort::baudrate()
{
   return wxGuiderPort::EPGBaudRate(BaudRateChoice->GetSelection());
}

wxGuiderPort::EPGHandshake wxGuidingInterfaceSerialPort::handshake()
{
   return wxGuiderPort::EPGHandshake(HandShakeChoice->GetSelection());
}

