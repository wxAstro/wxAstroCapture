#include "wxGuidingInterfaceParallelPort.h"

//(*InternalHeaders(wxGuidingInterfaceParallelPort)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "wxcustom/wxIoPortCtrl.h"
#include "wxIO/wxGuiderPort.h"
#include "wxFactory.h"
#include "wxWebcam/wxWebcam.h"

//(*IdInit(wxGuidingInterfaceParallelPort)
const long wxGuidingInterfaceParallelPort::ID_STATICTEXT8 = wxNewId();
const long wxGuidingInterfaceParallelPort::ID_CHOICE3 = wxNewId();
const long wxGuidingInterfaceParallelPort::ID_STATICTEXT10 = wxNewId();
const long wxGuidingInterfaceParallelPort::ID_STATICTEXT17 = wxNewId();
const long wxGuidingInterfaceParallelPort::ID_STATICTEXT11 = wxNewId();
const long wxGuidingInterfaceParallelPort::ID_CUSTOM2 = wxNewId();
const long wxGuidingInterfaceParallelPort::ID_STATICTEXT12 = wxNewId();
const long wxGuidingInterfaceParallelPort::ID_CUSTOM8 = wxNewId();
const long wxGuidingInterfaceParallelPort::ID_STATICTEXT13 = wxNewId();
const long wxGuidingInterfaceParallelPort::ID_CUSTOM9 = wxNewId();
const long wxGuidingInterfaceParallelPort::ID_STATICTEXT14 = wxNewId();
const long wxGuidingInterfaceParallelPort::ID_CUSTOM10 = wxNewId();
const long wxGuidingInterfaceParallelPort::ID_STATICTEXT15 = wxNewId();
const long wxGuidingInterfaceParallelPort::ID_CUSTOM11 = wxNewId();
const long wxGuidingInterfaceParallelPort::ID_STATICTEXT16 = wxNewId();
const long wxGuidingInterfaceParallelPort::ID_CUSTOM12 = wxNewId();
//*)

BEGIN_EVENT_TABLE(wxGuidingInterfaceParallelPort,wxPanel)
	//(*EventTable(wxGuidingInterfaceParallelPort)
	//*)
END_EVENT_TABLE()

wxGuidingInterfaceParallelPort::wxGuidingInterfaceParallelPort(wxWindow* parent,wxWindowID id,const wxPoint&  WXUNUSED(pos),const wxSize& WXUNUSED(size))
{
	//(*Initialize(wxGuidingInterfaceParallelPort)
	wxStaticText* StaticText10;
	wxBoxSizer* BoxSizer4;
	wxBoxSizer* BoxSizer6;
	wxBoxSizer* BoxSizer5;
	wxBoxSizer* BoxSizer7;
	wxBoxSizer* BoxSizer8;
	wxStaticText* StaticText13;
	wxStaticText* StaticText14;
	wxStaticText* StaticText8;
	wxStaticText* StaticText11;
	wxBoxSizer* BoxSizer2;
	wxStaticText* StaticText15;
	wxStaticText* StaticText12;
	wxBoxSizer* BoxSizer9;
	wxBoxSizer* BoxSizer3;
	wxStaticText* StaticText16;
	
	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	BoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Protocol"), wxDefaultPosition, wxSize(70,-1), 0, _T("ID_STATICTEXT8"));
	BoxSizer4->Add(StaticText8, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ProtocolChoice = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxSize(160,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
	BoxSizer4->Add(ProtocolChoice, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer4, 0, wxTOP|wxBOTTOM|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	StaticText10 = new wxStaticText(this, ID_STATICTEXT10, _("Port pins"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
	BoxSizer2->Add(StaticText10, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	StaticTextPinLabels = new wxStaticText(this, ID_STATICTEXT17, _("D7 <-------> D0"), wxDefaultPosition, wxSize(105,-1), wxALIGN_CENTRE, _T("ID_STATICTEXT17"));
	BoxSizer2->Add(StaticTextPinLabels, 0, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	BoxSizer1->Add(BoxSizer2, 0, wxTOP|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
	StaticText11 = new wxStaticText(this, ID_STATICTEXT11, _("GX Mask"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
	BoxSizer3->Add(StaticText11, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_BOTTOM, 4);
	GX_mask = new wxIoPortCtrl(this,ID_CUSTOM2,wxDefaultPosition,wxDefaultSize,wxEXPAND,_T("ID_CUSTOM2"));
	BoxSizer3->Add(GX_mask, 0, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_BOTTOM, 4);
	BoxSizer1->Add(BoxSizer3, 0, wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	BoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
	StaticText12 = new wxStaticText(this, ID_STATICTEXT12, _("GX Init"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT12"));
	BoxSizer5->Add(StaticText12, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_BOTTOM, 4);
	GX_init = new wxIoPortCtrl(this,ID_CUSTOM8,wxDefaultPosition,wxDefaultSize,wxEXPAND,_T("ID_CUSTOM8"));
	BoxSizer5->Add(GX_init, 0, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_BOTTOM, 4);
	BoxSizer1->Add(BoxSizer5, 0, wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	BoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
	StaticText13 = new wxStaticText(this, ID_STATICTEXT13, _("GX RA pos"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT13"));
	BoxSizer6->Add(StaticText13, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_BOTTOM, 4);
	GX_RA_pos = new wxIoPortCtrl(this,ID_CUSTOM9,wxDefaultPosition,wxDefaultSize,wxEXPAND,_T("ID_CUSTOM9"));
	BoxSizer6->Add(GX_RA_pos, 0, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_BOTTOM, 4);
	BoxSizer1->Add(BoxSizer6, 0, wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	BoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
	StaticText14 = new wxStaticText(this, ID_STATICTEXT14, _("GX RA neg"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT14"));
	BoxSizer7->Add(StaticText14, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_BOTTOM, 4);
	GX_RA_neg = new wxIoPortCtrl(this,ID_CUSTOM10,wxDefaultPosition,wxDefaultSize,wxEXPAND,_T("ID_CUSTOM10"));
	BoxSizer7->Add(GX_RA_neg, 0, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_BOTTOM, 4);
	BoxSizer1->Add(BoxSizer7, 0, wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	BoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
	StaticText15 = new wxStaticText(this, ID_STATICTEXT15, _("GX DEC pos"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT15"));
	BoxSizer9->Add(StaticText15, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_BOTTOM, 4);
	GX_DEC_pos = new wxIoPortCtrl(this,ID_CUSTOM11,wxDefaultPosition,wxDefaultSize,wxEXPAND,_T("ID_CUSTOM11"));
	BoxSizer9->Add(GX_DEC_pos, 0, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_BOTTOM, 4);
	BoxSizer1->Add(BoxSizer9, 0, wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	BoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
	StaticText16 = new wxStaticText(this, ID_STATICTEXT16, _("GX DEC neg"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT16"));
	BoxSizer8->Add(StaticText16, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_BOTTOM, 4);
	GX_DEC_neg = new wxIoPortCtrl(this,ID_CUSTOM12,wxDefaultPosition,wxDefaultSize,wxEXPAND,_T("ID_CUSTOM12"));
	BoxSizer8->Add(GX_DEC_neg, 0, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_BOTTOM, 4);
	BoxSizer1->Add(BoxSizer8, 0, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
	//*)

	Init();
}

wxGuidingInterfaceParallelPort::~wxGuidingInterfaceParallelPort()
{
	//(*Destroy(wxGuidingInterfaceParallelPort)
	//*)
}

void wxGuidingInterfaceParallelPort::Init()
{
   InitProtocolChoice();
   InitPortPinsControl();
}

void wxGuidingInterfaceParallelPort::InitProtocolChoice()
{
   ProtocolChoice->Clear();

   // filter out and show the protocols that are relevant for the port type
   wxGuiderPortProtocol::EPGProtocol protocol = wxGuiderPortProtocol::EPGP_None;
   while(protocol != wxGuiderPortProtocol::EPGP_LAST) {

      if(wxGuiderPortProtocol::SupportsParallel(protocol)) {
         ProtocolChoice->Append(wxGuiderPortProtocol::ProtocolName(protocol));
      }
      protocol = wxGuiderPortProtocol::EPGProtocol(protocol+1);
   }

   // just to make sure we have a default
   ProtocolChoice->SetSelection(0);

   if(wxGuider* guider = wxF()->guider()) {
      // if we already have a parallel guide port, use its values as defaults
      wxGuiderPort* gport_f = wxF()->guidePort();
      if(gport_f && gport_f->IsPortParallel()) {
         ProtocolChoice->SetStringSelection(gport_f->ProtocolName());
      }
   }
}

void wxGuidingInterfaceParallelPort::InitPortPinsControl()
{
   // just make a guide port object on the stack
   // to ensure we don't change the one in the factory untill user presses apply

   wxGuiderPort gport;
   // set some defaults
   gport.SetGXmask(wxGuiderPort::GXmask_default());       // others will not be affected
   gport.SetGXinit(wxGuiderPort::GXinit_default());       // init GX - draw all high
   gport.SetGXRA_pos(wxGuiderPort::GXRA_pos_default());       // RA+
   gport.SetGXRA_neg(wxGuiderPort::GXRA_neg_default());       // RA-
   gport.SetGXDEC_pos(wxGuiderPort::GXDEC_pos_default());      // Dec+
   gport.SetGXDEC_neg(wxGuiderPort::GXDEC_neg_default());      // Dec-

      // skip if not port interfac active
   if(wxGuider* guider = wxF()->guider()) {

      // if we already have a parallel guide port, use its values as defaults
      wxGuiderPort* gport_f = wxF()->guidePort();
      if(gport_f && gport_f->IsPortParallel()) {
         gport.SetGXmask(gport_f->GXmask());       // others will not be affected
         gport.SetGXinit(gport_f->GXinit());       // init GX - draw all high
         gport.SetGXRA_pos(gport_f->GXRA_pos());       // RA+
         gport.SetGXRA_neg(gport_f->GXRA_neg());       // RA-
         gport.SetGXDEC_pos(gport_f->GXDEC_pos());      // Dec+
         gport.SetGXDEC_neg(gport_f->GXDEC_neg());      // Dec-
         gport.SetInvertedLogic(gport_f->InvertedLogic());         // set (parallel) logic inverted
      }
   }

   // fill the controls
#ifdef _WXMSW_
   StaticTextPinLabels->SetLabel(_("D7<----------->D0"));
#else
   StaticTextPinLabels->SetLabel(_("D7<--------->D0"));
#endif
   GX_mask->CreatePins(8,gport.GXmask());
   GX_init->CreatePins(8,gport.GXinit());
   GX_RA_pos->CreatePins(8,gport.GXRA_pos());
   GX_RA_neg->CreatePins(8,gport.GXRA_neg());
   GX_DEC_pos->CreatePins(8,gport.GXDEC_pos());
   GX_DEC_neg->CreatePins(8,gport.GXDEC_neg());

	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
	BoxSizer1->Layout();
   Layout();
}

void wxGuidingInterfaceParallelPort::OnApply()
{
   ApplyPort();
}


bool wxGuidingInterfaceParallelPort::ApplyPort()
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
   // Get the port and set the parallel port values
   wxGuiderPort* gport = wxF()->guidePort();

   if(gport->IsPortParallel()) {

      gport->SetGXmask(GXmask());          // others will not be affected
      gport->SetGXinit(GXinit());          // init GX - draw all high
      gport->SetGXRA_pos(GXRA_pos());       // RA+
      gport->SetGXRA_neg(GXRA_neg());       // RA-
      gport->SetGXDEC_pos(GXDEC_pos());      // Dec+
      gport->SetGXDEC_neg(GXDEC_neg());      // Dec-

   }
   else {
      wxMessageBox(wxT("Programming error: Guide port is notparallel type!!!"));
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


unsigned char wxGuidingInterfaceParallelPort::GXmask() const   { return GX_mask->GetPinsByte(); }
unsigned char wxGuidingInterfaceParallelPort::GXinit() const   { return GX_init->GetPinsByte(); }       // init GX
unsigned char wxGuidingInterfaceParallelPort::GXRA_pos() const  { return GX_RA_pos->GetPinsByte(); }       // RA+
unsigned char wxGuidingInterfaceParallelPort::GXRA_neg() const  { return GX_RA_neg->GetPinsByte(); }       // RA-
unsigned char wxGuidingInterfaceParallelPort::GXDEC_pos() const { return GX_DEC_pos->GetPinsByte(); }      // Dec+
unsigned char wxGuidingInterfaceParallelPort::GXDEC_neg() const { return GX_DEC_neg->GetPinsByte(); }      // Dec-
