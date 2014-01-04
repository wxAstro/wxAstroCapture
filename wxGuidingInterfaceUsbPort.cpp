#include "wxGuidingInterfaceUsbPort.h"

//(*InternalHeaders(wxGuidingInterfaceUsbPort)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "wxIO/wxGuiderPort.h"
#include "wxFactory.h"
#include "wxWebcam/wxWebcam.h"


//(*IdInit(wxGuidingInterfaceUsbPort)
const long wxGuidingInterfaceUsbPort::ID_STATICTEXT8 = wxNewId();
const long wxGuidingInterfaceUsbPort::ID_CHOICE3 = wxNewId();
//*)

BEGIN_EVENT_TABLE(wxGuidingInterfaceUsbPort,wxPanel)
	//(*EventTable(wxGuidingInterfaceUsbPort)
	//*)
END_EVENT_TABLE()

wxGuidingInterfaceUsbPort::wxGuidingInterfaceUsbPort(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(wxGuidingInterfaceUsbPort)
	wxBoxSizer* BoxSizer4;
	wxStaticText* StaticText8;
	
	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	BoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Protocol"), wxDefaultPosition, wxSize(70,-1), 0, _T("ID_STATICTEXT8"));
	BoxSizer4->Add(StaticText8, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ProtocolChoice = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxSize(160,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
	BoxSizer4->Add(ProtocolChoice, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer4, 0, wxTOP|wxBOTTOM|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
	//*)

	Init();
}

wxGuidingInterfaceUsbPort::~wxGuidingInterfaceUsbPort()
{
	//(*Destroy(wxGuidingInterfaceUsbPort)
	//*)
}


void wxGuidingInterfaceUsbPort::Init()
{
   InitProtocolChoice();
   InitPortParameters();
}



void wxGuidingInterfaceUsbPort::InitProtocolChoice()
{
   ProtocolChoice->Clear();

   // filter out and show the protocols that are relevant for the port type
   wxGuiderPortProtocol::EPGProtocol protocol = wxGuiderPortProtocol::EPGP_None;
   while(protocol != wxGuiderPortProtocol::EPGP_LAST) {

      if(wxGuiderPortProtocol::SupportsGPUSB(protocol)) {
         ProtocolChoice->Append(wxGuiderPortProtocol::ProtocolName(protocol));
      }
      protocol = wxGuiderPortProtocol::EPGProtocol(protocol+1);
   }

   // just to make sure we have a default
   ProtocolChoice->SetSelection(0);

   if(wxGuider* guider = wxF()->guider()) {
      // if we already have a GPUSB  guide port, use its values as defaults
      wxGuiderPort* gport_f = wxF()->guidePort();
      if(gport_f && gport_f->IsPortGPUSB()) {
         ProtocolChoice->SetStringSelection(gport_f->ProtocolName());
      }
   }
}


void wxGuidingInterfaceUsbPort::InitPortParameters()
{
   // nothing here right now
}



void wxGuidingInterfaceUsbPort::OnApply()
{
   ApplyPort();
}


bool wxGuidingInterfaceUsbPort::ApplyPort()
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

   if(gport->IsPortGPUSB()) {
      gport->SetGXDefault();
   }
   else {
      wxMessageBox(wxT("Programming error: Guide port is not GPUSB type!!!"));
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

