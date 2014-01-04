#include "SettingsGuidePort.h"

//(*InternalHeaders(SettingsGuidePort)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "wxFactory.h"
#include "SettingsGuidePortSerial.h"
#include "SettingsGuidePortParallel.h"
#include "wxIO/wxGuiderPort.h"

#include "wxWebcam/wxWebcam.h"

//(*IdInit(SettingsGuidePort)
const long SettingsGuidePort::ID_STATICTEXT1 = wxNewId();
const long SettingsGuidePort::ID_CHOICE1 = wxNewId();
const long SettingsGuidePort::ID_STATICTEXT8 = wxNewId();
const long SettingsGuidePort::ID_CHOICE2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SettingsGuidePort,wxPanel)
	//(*EventTable(SettingsGuidePort)
	//*)
END_EVENT_TABLE()

SettingsGuidePort::SettingsGuidePort(wxWindow* parent,wxWindowID id)
{
	//(*Initialize(SettingsGuidePort)
	wxStaticText* StaticText8;
	wxStaticText* StaticText1;
	wxBoxSizer* BoxSizer2;
	wxBoxSizer* BoxSizer1;
	wxBoxSizer* BoxSizer3;
	
	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Port"), wxDefaultPosition, wxSize(70,-1), 0, _T("ID_STATICTEXT1"));
	BoxSizer2->Add(StaticText1, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	PortChoice = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxSize(130,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	BoxSizer2->Add(PortChoice, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer2, 1, wxTOP|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Protocol"), wxDefaultPosition, wxSize(70,-1), 0, _T("ID_STATICTEXT8"));
	BoxSizer3->Add(StaticText8, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ProtocolChoice = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxSize(130,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	BoxSizer3->Add(ProtocolChoice, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer3, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizerPortPanel = new wxBoxSizer(wxHORIZONTAL);
	BoxSizer1->Add(BoxSizerPortPanel, 4, wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
	
	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SettingsGuidePort::OnPortChoiceSelect);
	Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SettingsGuidePort::OnProtocolChoiceSelect);
	//*)

	// Init();
}

SettingsGuidePort::~SettingsGuidePort()
{
	//(*Destroy(SettingsGuidePort)
	//*)
}

void SettingsGuidePort::Init()
{
   SetInit(false);
   wxWebcam* webcam = wxF()->webcam();
   if(!webcam)return;
   bool enable = !webcam->isGuiding();
   Enable(enable);

   if(enable) {

      wxPortDescrList& pList = wxF()->portManager()->PortDescriptors();
      wxPortDescrList::port_iterator iport = pList.port_begin();
      // fill the dropdown list

      PortChoice->Clear();
      while(iport != pList.port_end()){
         const wxPortDescr& port_entry = pList.PortDescr(iport++);
         if (port_entry.IsGuidePort()) PortChoice->Append(port_entry.PortName()); //BM:20081228 only GX ports added
      }
      m_serial   = new SettingsGuidePortSerial(this);
      m_parallel = new SettingsGuidePortParallel(this);

      m_parallel->Hide();
      m_serial->Hide();
      PortChoice->SetSelection(0);


      // skip if not port interfac active
      if(wxGuider* guider = wxF()->guider()) {
         if(guider->interface_type() != wxGuider::PORT_INTERFACE)return;
      }

      // Set the port dropdown selection here
      wxString portName = wxF()->guidePort()->PortName();
      if (!PortChoice->SetStringSelection(portName))
         PortChoice->SetSelection(0); // defaults to 1st in list if not found

      // get the name of the current protocol
      wxString protocolName = wxF()->guidePort()->ProtocolName();

      // set up GUI controls to match the given port and protocol
      bool show_serial   = wxF()->guidePort()->IsPortSerial();
      bool show_parallel = wxF()->guidePort()->IsPortParallel();
      bool show_gpusb = wxF()->guidePort()->IsPortGPUSB(); //BM:20081228
      ShowPortControls(protocolName,show_serial,show_parallel,show_gpusb);
   }
   SetInit(enable);
}

wxString SettingsGuidePort::PortNameGuider()
{
   return PortChoice->GetString(PortChoice->GetSelection());
}
wxString SettingsGuidePort::ProtocolNameGuider()
{
   int sel = ProtocolChoice->GetSelection(); // choice can be empty !!
   if (sel<0)
      return wxT("");
   else
      return ProtocolChoice->GetString(ProtocolChoice->GetSelection());
}

// setup the port display based on the given parameters
void SettingsGuidePort::ShowPortControls(const wxString& protocolName, bool show_serial,bool show_parallel,bool show_gpusb)
{
   ShowProtocols(protocolName,show_serial,show_parallel,show_gpusb); // propagate to protocols show first

   BoxSizerPortPanel->Clear();
   if (show_serial) {
      m_serial->Show(true);
      m_parallel->Show(false);
      BoxSizerPortPanel->Add(m_serial,3,wxALL|wxEXPAND|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL,0);
   }
   else if (show_parallel) {
      m_serial->Show(false);
      m_parallel->Show(true);
      BoxSizerPortPanel->Add(m_parallel,1,wxALL|wxALIGN_RIGHT|wxALIGN_TOP,0);
   }
   else {
      m_parallel->Hide();
      m_serial->Hide();
      BoxSizerPortPanel->Clear();
   }

   BoxSizerPortPanel->Layout();
   Layout();
}

void SettingsGuidePort::ShowProtocols(const wxString& protocolName, bool show_serial, bool show_parallel, bool show_gpusb)
{
   ProtocolChoice->Clear();

   // filter out and show the protocols that are relevant for the port type
   wxGuiderPortProtocol::EPGProtocol protocol = wxGuiderPortProtocol::EPGP_None;
   while(protocol != wxGuiderPortProtocol::EPGP_LAST) {

      bool add_protocol = false;
      if(show_parallel  && wxGuiderPortProtocol::SupportsParallel(protocol))add_protocol = true;
      else if(show_serial && wxGuiderPortProtocol::SupportsSerial(protocol))add_protocol = true;
      else if(show_gpusb && wxGuiderPortProtocol::SupportsGPUSB(protocol))add_protocol = true; //BM:20081228

      if(add_protocol)ProtocolChoice->Append(wxGuiderPortProtocol::ProtocolName(protocol));
      protocol = wxGuiderPortProtocol::EPGProtocol(protocol+1);
   }

   // we have to set the dropdown selection here - from given name ??valid
   if (!ProtocolChoice->SetStringSelection(protocolName))
      ProtocolChoice->SetSelection(0); // defaults to 1st in list if not found
}

void SettingsGuidePort::OnPortChoiceSelect(wxCommandEvent& WXUNUSED(event))
{
   // At this stage do NOT apply any changes to the port or protocol objects
   // No port or protocol changes shall happen until OK (or Apply) has been pressed

   // Just refresh the GUI to reflect the current port selection
   const wxPortDescr& port_entry = wxF()->portManager()->PortDescriptor(PortNameGuider());

   // check to see if the chosen port type is compatible with the presently selected protocol
   if (! wxGuiderPortProtocol::CanUseProtocol(port_entry, wxGuiderPortProtocol::ProtocolEnum(ProtocolNameGuider()) )) {
      // The chosen port is not compatible with the previously selected protocol
      // Update the GUI using a default, compatible protocol
      ShowPortControls(wxT(""), port_entry.IsPortSerial(), port_entry.IsPortParallel(), port_entry.IsPortGPUSB());
   }
   else {
      // The chosen port is compatible with the existing protocol
      // Just update the GUI for the given protocol
      ShowPortControls(ProtocolNameGuider(), port_entry.IsPortSerial(), port_entry.IsPortParallel(), port_entry.IsPortGPUSB());
   }
}

void SettingsGuidePort::OnProtocolChoiceSelect(wxCommandEvent& WXUNUSED(event))
{
   // At this stage do NOT apply any changes to the port or protocol objects
   // No port or protocol changes shall happen until OK (or Apply) has been pressed

   // Just refresh the GUI to reflect the current protocol selection
   const wxPortDescr& port_entry = wxF()->portManager()->PortDescriptor(PortNameGuider());

   // The available protocols will always be compatible with the current port at this stage
   ShowPortControls(ProtocolNameGuider(), port_entry.IsPortSerial(), port_entry.IsPortParallel(), port_entry.IsPortGPUSB());
}



bool SettingsGuidePort::ApplyPortProtocol()
{
   // this is a helper function, to be called ONLY from OnApply()

   // Are we using the port interface at all?
   if(wxGuider* guider = wxF()->guider()) {
      if(guider->interface_type() != wxGuider::PORT_INTERFACE) return false;
   }

   // Here we apply the GUI settings to the system
   // port is given - and the protocol will always apply

   // port chosen - use this one
   const wxPortDescr& port_entry = wxF()->portManager()->PortDescriptor(PortNameGuider());
   if (! wxF()->guidePort()->UseGXPort(port_entry, wxGuiderPortProtocol::ProtocolEnum(ProtocolNameGuider())) ) {

      // using the None protocol when somehow the given protocol was not accepted
      // this should really never happen, because the other events ensure compatible settings
      wxF()->guidePort()->UseGXPort(port_entry, wxGuiderPortProtocol::EPGP_None);
      wxString message = wxT("Failed to setup guider port ");
      message += PortNameGuider();
      message += wxT(" using ");
      message += ProtocolNameGuider();
      wxMessageBox(message);
      return false;
   }
   return true;
}

void SettingsGuidePort::OnCancel()
{
   // force reinit on next occation
   SetInit(false);
}

bool SettingsGuidePort::OnApply()
{
   if(!IsInit())return false;
   // force reinit on next occation
   SetInit(false);

   if(wxF()->guider()->guiding_active())return false;

   // Are we using the port interface at all?
   if(wxGuider* guider = wxF()->guider()) {
      if(guider->interface_type() != wxGuider::PORT_INTERFACE)return false;
   }

   // Set port AND protocol first
   ApplyPortProtocol();

   // Now the port and protocol objects have been set up in the system
   // We can now proceed and apply the details to the newly activated port

   // Get the port and set the values
   wxGuiderPort* gport = wxF()->guidePort();

   if(gport->IsPortSerial()) {
      gport->SetGXDefault();
      gport->SetSerialHandshake(m_serial->handshake());
      gport->SetSerialBaudrate(m_serial->baudrate());

      // Give a warning for incompatible settings
      wxGuiderPortProtocol::EPGProtocol protocol = wxGuiderPortProtocol::ProtocolEnum(ProtocolNameGuider());
      if(protocol == wxGuiderPortProtocol::EPGP_MTS_3SDI) {
         if(m_serial->baudrate() != wxGuiderPort::EPGB_9600) {
            wxMessageBox(_T("Warning: MTS-3 requires baudrate 9600"));
            return false;
         }
      }

   }
   else if(gport->IsPortParallel()) {
      gport->SetGXDefault();

      gport->SetGXmask(m_parallel->GXmask());          // others will not be affected
      gport->SetGXinit(m_parallel->GXinit());          // init GX - draw all high
      gport->SetGXRA_pos(m_parallel->GXRA_pos());       // RA+
      gport->SetGXRA_neg(m_parallel->GXRA_neg());       // RA-
      gport->SetGXDEC_pos(m_parallel->GXDEC_pos());      // Dec+
      gport->SetGXDEC_neg(m_parallel->GXDEC_neg());      // Dec-
      //gport->SetInvertedLogic(true);                  // set (parallel) logic inverted
   }
   else if(gport->IsPortGPUSB()) { //BM:20081228
      gport->SetGXDefault();
   }
   else if(gport->IsPortDummy()) {
      // The user deliberately selected !None!
      // nothing to do
   }
   else {
      return false;
   }


   // finished setting up the bits..
   // NOTE CommitConfiguration() does NOT release the Port !!!
   if ( ! gport->CommitConfiguration() ) {
      wxMessageBox(_T("Cannot configure guider control port ") + gport->PortName() \
                                 + _T(", please check port settings."));
   }
   // apply from camera port to config
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
