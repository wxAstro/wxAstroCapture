#include "SettingsControlPort.h"

//(*InternalHeaders(SettingsControlPort)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "wxWebcamDBMain.h"
#include "wxFactory.h"
#include "wxIO/wxLongExpoPort.h"
#include "wxIO/wxPortManager.h"



//(*IdInit(SettingsControlPort)
const long SettingsControlPort::ID_STATICTEXT1 = wxNewId();
const long SettingsControlPort::ID_CHOICE1 = wxNewId();
const long SettingsControlPort::ID_STATICTEXT2 = wxNewId();
const long SettingsControlPort::ID_STATICTEXT7 = wxNewId();
const long SettingsControlPort::ID_BUTTON1 = wxNewId();
const long SettingsControlPort::ID_STATICTEXT6 = wxNewId();
const long SettingsControlPort::ID_CUSTOM3 = wxNewId();
const long SettingsControlPort::ID_STATICTEXT3 = wxNewId();
const long SettingsControlPort::ID_CUSTOM4 = wxNewId();
const long SettingsControlPort::ID_STATICTEXT4 = wxNewId();
const long SettingsControlPort::ID_CUSTOM5 = wxNewId();
const long SettingsControlPort::ID_CHECKBOX3 = wxNewId();
const long SettingsControlPort::ID_STATICTEXT5 = wxNewId();
const long SettingsControlPort::ID_CUSTOM6 = wxNewId();
const long SettingsControlPort::ID_CHECKBOX1 = wxNewId();
const long SettingsControlPort::ID_STATICTEXT9 = wxNewId();
const long SettingsControlPort::ID_CUSTOM7 = wxNewId();
const long SettingsControlPort::ID_CHECKBOX2 = wxNewId();
const long SettingsControlPort::ID_PANEL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SettingsControlPort,wxPanel)
	//(*EventTable(SettingsControlPort)
	//*)
END_EVENT_TABLE()

SettingsControlPort::SettingsControlPort(wxWindow* parent,wxWindowID id)
{
	//(*Initialize(SettingsControlPort)
	wxStaticText* StaticText9;
	wxStaticText* StaticText2;
	wxStaticText* StaticText6;
	wxStaticText* StaticText1;
	wxBoxSizer* BoxSizer2;
	wxStaticText* StaticText3;
	wxStaticText* StaticText5;
	wxBoxSizer* BoxSizer1;
	wxStaticText* StaticText4;
	
	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	Panel1 = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	StaticText1 = new wxStaticText(Panel1, ID_STATICTEXT1, _("Port"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	BoxSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	PortChoice = new wxChoice(Panel1, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	BoxSizer2->Add(PortChoice, 4, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer2, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1 = new wxFlexGridSizer(0, 4, 0, 0);
	StaticText2 = new wxStaticText(Panel1, ID_STATICTEXT2, _("Port pins"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText7 = new wxStaticText(Panel1, ID_STATICTEXT7, _("D7 D0"), wxDefaultPosition, wxSize(105,-1), wxALIGN_RIGHT, _T("ID_STATICTEXT7"));
	FlexGridSizer1->Add(StaticText7, 2, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(20,10,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	DefaultsButton = new wxButton(Panel1, ID_BUTTON1, _("Defaults"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer1->Add(DefaultsButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText6 = new wxStaticText(Panel1, ID_STATICTEXT6, _("LX Mask"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, _T("ID_STATICTEXT6"));
	FlexGridSizer1->Add(StaticText6, 1, wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	LX_mask = new wxIoPortCtrl(Panel1,ID_CUSTOM3,wxDefaultPosition,wxDefaultSize,wxEXPAND,_T("ID_CUSTOM3"));
	FlexGridSizer1->Add(LX_mask, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
	FlexGridSizer1->Add(-1,10,1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(-1,10,1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(Panel1, ID_STATICTEXT3, _("LX Init"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	LX_init = new wxIoPortCtrl(Panel1,ID_CUSTOM4,wxDefaultPosition,wxDefaultSize,wxEXPAND,_T("ID_CUSTOM4"));
	FlexGridSizer1->Add(LX_init, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(Panel1, ID_STATICTEXT4, _("LX Frame"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	LX_frame = new wxIoPortCtrl(Panel1,ID_CUSTOM5,wxDefaultPosition,wxDefaultSize,wxEXPAND,_T("ID_CUSTOM5"));
	FlexGridSizer1->Add(LX_frame, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	UseInvertedLogic = new wxCheckBox(Panel1, ID_CHECKBOX3, _("Inverted logic"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
	UseInvertedLogic->SetValue(false);
	FlexGridSizer1->Add(UseInvertedLogic, 1, wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText5 = new wxStaticText(Panel1, ID_STATICTEXT5, _("LX Amp"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer1->Add(StaticText5, 1, wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	LX_amp = new wxIoPortCtrl(Panel1,ID_CUSTOM6,wxDefaultPosition,wxDefaultSize,wxEXPAND,_T("ID_CUSTOM6"));
	FlexGridSizer1->Add(LX_amp, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	UseAmpCheckBox = new wxCheckBox(Panel1, ID_CHECKBOX1, _("Use Amp"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	UseAmpCheckBox->SetValue(false);
	FlexGridSizer1->Add(UseAmpCheckBox, 1, wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText9 = new wxStaticText(Panel1, ID_STATICTEXT9, _("LX Shutter"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
	FlexGridSizer1->Add(StaticText9, 1, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	LX_shutter = new wxIoPortCtrl(Panel1,ID_CUSTOM7,wxDefaultPosition,wxDefaultSize,wxEXPAND,_T("ID_CUSTOM7"));
	FlexGridSizer1->Add(LX_shutter, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	UseShutterCheckBox = new wxCheckBox(Panel1, ID_CHECKBOX2, _("Use Shutter"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	UseShutterCheckBox->SetValue(false);
	FlexGridSizer1->Add(UseShutterCheckBox, 1, wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(FlexGridSizer1, 4, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel1->SetSizer(BoxSizer1);
	BoxSizer1->Fit(Panel1);
	BoxSizer1->SetSizeHints(Panel1);
	
	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SettingsControlPort::OnPortChoiceSelect);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SettingsControlPort::OnDefaultsButtonClick);
	//*)
  	Layout();

// Do not do Init() too early
//	Init();
}


SettingsControlPort::~SettingsControlPort()
{
	//(*Destroy(SettingsControlPort)
	//*)
}

wxString SettingsControlPort::PortNameControl()
{
   return PortChoice->GetString(PortChoice->GetSelection());
}

/*
Init        - load from config and show and commit
Apply       - commit and save to config, release port and leave
Switch Port - load default for new port
*/
void SettingsControlPort::Init()
{

   bool enabled = !IsLExposing();
   Enable(enabled);
   SetInit(enabled);
   if(enabled) {
      wxPortDescrList& pList = wxF()->portManager()->PortDescriptors();
      wxPortDescrList::port_iterator iport = pList.port_begin();
      // fill the dropdown list
      while(iport != pList.port_end()){
         const wxPortDescr& port_entry = pList.PortDescr(iport++);
         if (port_entry.IsLxPort()) PortChoice->Append(port_entry.PortName()); //BM:20081228 only LX ports added
      }


      wxF()->lePort()->RestoreFromConfigLongExpo(wxF()->config()); // does commit itself

      // we have to set the dropdown selection here before calling CreatePortControls()
      bool port_found = false;
      wxString pn = wxF()->lePort()->PortName();
      if(pn.Length() > 0) {

         // we got a name from config.
         // But is it really  one of the ports on the system?
         // If SetStringSelection returns false, it isn't
         if(PortChoice->SetStringSelection(pn)) {
            port_found = true;
         }
      }

      if(!port_found) {
         // either no name in config or the name is not recognised
         // We default to zero = !None!
         PortChoice->SetSelection(0);
      }

   }

   CreatePortControls(enabled, false); //BM:20081229 - dont load defaults here
}

void SettingsControlPort::CreatePortControls(bool enable, bool defaults)
{
   if(enable) {

      // having a le obj...
      const wxPortDescr& port_entry = wxF()->portManager()->PortDescriptor(PortNameControl());
      if (! wxF()->lePort()->UseLEPort(port_entry) ) {
         wxMessageBox(wxT("Failed to setup long exposure object."));
         return;
      }
      if (defaults) {
         wxF()->lePort()->SetLXDefault(); //BM:20081229 - load defaults if needed
         wxF()->lePort()->CommitConfiguration(); // must be called after setting changes (also SetLXDefault())
      }

      if(port_entry.IsPortParallel()) {
         StaticText7->SetLabel(_("D7<----------->D0"));
         LX_mask->Enable(true);              LX_mask->CreatePins(8,wxF()->lePort()->LXmask());
         LX_init->Enable(true);              LX_init->CreatePins(8,wxF()->lePort()->LXinit());
         // Frame
         LX_frame->Enable(true);             LX_frame->CreatePins(8,wxF()->lePort()->LXreadout());
         // AMP
         LX_amp->Enable(true);                LX_amp->CreatePins(8,wxF()->lePort()->LXampOn());
         UseAmpCheckBox->Enable(true);       UseAmpCheckBox->SetValue(wxF()->lePort()->IsAMPmodeOn());
         // Cycle shutter
         LX_shutter->Enable(true);           LX_shutter->CreatePins(8,wxF()->lePort()->LXshutOn());
         UseShutterCheckBox->Enable(true);   UseShutterCheckBox->SetValue(wxF()->lePort()->IsCycleShutter());
         // Inverted Logic truc
         UseInvertedLogic->SetValue(false); UseInvertedLogic->Enable(false);
      }

      else if(port_entry.IsPortSerial()) {
         StaticText7->SetLabel( _("DTR RTS"));
         LX_mask->Enable(true);              LX_mask->CreatePins(2,wxF()->lePort()->LXmask());
         LX_init->Enable(true);              LX_init->CreatePins(2,wxF()->lePort()->LXinit());
         // Frame
         LX_frame->Enable(true);             LX_frame->CreatePins(2,wxF()->lePort()->LXreadout());
         // AMP
         LX_amp->Enable(true);               LX_amp->CreatePins(2,wxF()->lePort()->LXampOn());
         UseAmpCheckBox->Enable(true);       UseAmpCheckBox->SetValue(wxF()->lePort()->IsAMPmodeOn());
         // Cycle shutter
         LX_shutter->CreatePins(0,0);        LX_shutter->Enable(false);
         UseShutterCheckBox->Enable(false);  UseShutterCheckBox->SetValue(false);
         // Inverted Logic truc
         UseInvertedLogic->Enable(true);     UseInvertedLogic->SetValue(wxF()->lePort()->InvertedLogic());

     }
     else if(wxF()->lePort()->IsPortLXUSB()) { //BM:20081228 added
        // show pins but don't allow editing
         StaticText7->SetLabel(_("D3<---->D0"));
         LX_mask->Enable(true);
         LX_mask->CreatePins(4,wxF()->lePort()->LXmask());           LX_mask->Enable(false);
         LX_init->Enable(true);
         LX_init->CreatePins(4,wxF()->lePort()->LXinit());           LX_init->Enable(false);
         // Frame
         LX_frame->Enable(true); // due to whatever reason one has to enable the control before creating pins and then disable it
         LX_frame->CreatePins(4,wxF()->lePort()->LXreadout());       LX_frame->Enable(false);
         // AMP
         LX_amp->Enable(true);
         LX_amp->CreatePins(4,wxF()->lePort()->LXampOn());           LX_amp->Enable(false);
         UseAmpCheckBox->Enable(true);       UseAmpCheckBox->SetValue(wxF()->lePort()->IsAMPmodeOn());
         // Cycle shutter
         LX_shutter->Enable(true);
         LX_shutter->CreatePins(4,wxF()->lePort()->LXshutOn());            LX_shutter->Enable(false);
         UseShutterCheckBox->Enable(true);   UseShutterCheckBox->SetValue(wxF()->lePort()->IsCycleShutter());
         // Inverted Logic truc
         UseInvertedLogic->Enable(false);    UseInvertedLogic->SetValue(false);
     }
     else {
         // clear dialog from entry elements
         StaticText7->SetLabel( _(""));
         LX_mask->CreatePins(0,0);            LX_mask->Enable(false);
         LX_init->CreatePins(0,0);            LX_mask->Enable(false);
         LX_frame->CreatePins(0,0);           LX_frame->Enable(false);
         LX_amp->CreatePins(0,0);             LX_amp->Enable(false);
         UseAmpCheckBox->SetValue(false);     UseAmpCheckBox->Enable(false);
         LX_shutter->CreatePins(0,0);         LX_shutter->Enable(false);
         UseShutterCheckBox->SetValue(false); UseShutterCheckBox->Enable(false);
         UseInvertedLogic->SetValue(false);   UseInvertedLogic->Enable(false);
      }
   }

   FlexGridSizer1->Layout();
  	Layout();
}

bool SettingsControlPort::OnApply()
{
   // if page has not been touched, don't do anything
   if(!IsInit())return false;
   if(IsLExposing())return false;

   //BM:20081228 added "for parallel and serial only"
   if ( wxF()->lePort()->IsPortParallel() || wxF()->lePort()->IsPortSerial() ) {
      // apply from control to camera port
      wxF()->lePort()->SetInvertedLogic(UseInvertedLogic->GetValue());

      // AMP
      if(UseAmpCheckBox->GetValue()) wxF()->lePort()->AMPmodeOn();
      else                           wxF()->lePort()->AMPmodeOff();

      // Cycle shutter
      if(UseShutterCheckBox->GetValue()) wxF()->lePort()->CycleShutterOn();
      else                               wxF()->lePort()->CycleShutterOff();

      // actual pin values
      wxF()->lePort()->SetLXmask(LX_mask->GetPinsByte());
      wxF()->lePort()->SetLXinit(LX_init->GetPinsByte());
      wxF()->lePort()->SetLXreadout(LX_frame->GetPinsByte());
      wxF()->lePort()->SetLXampOn(LX_amp->GetPinsByte());
      wxF()->lePort()->SetLXshutOn(LX_shutter->GetPinsByte());
      wxF()->lePort()->LXmodeOff();  // init to make port lines defined
   }
   else if(wxF()->lePort()->IsPortLXUSB()) { //BM:20081228 added
      wxF()->lePort()->SetLXDefault(); // set defaults
      // then may be changed ones
      // AMP
      if(UseAmpCheckBox->GetValue()) wxF()->lePort()->AMPmodeOn();
      else                           wxF()->lePort()->AMPmodeOff();

      // Cycle shutter
      if(UseShutterCheckBox->GetValue()) wxF()->lePort()->CycleShutterOn();
      else                               wxF()->lePort()->CycleShutterOff();
   }
   else if(wxF()->lePort()->IsPortLXLED()) { //BM:20081228 added
      wxF()->lePort()->SetLXDefault(); // set defaults in any case
   }
   else {
         ; // TODO other or not selected
   }

   // finished setting up the bits..
   if ( ! wxF()->lePort()->CommitConfiguration() ) {
      wxMessageBox(_T("Cannot configure long exposure control port ") + wxF()->lePort()->PortName() \
                                 + _T(", please check port settings."));
   }
   // apply from camera port to config
   wxF()->lePort()->SaveToConfigLongExpo(wxF()->config());

   // release LE port if possible
   wxF()->lePortRelease();

   return true;
}

void SettingsControlPort::OnCancel()
{
   // releasea LE port if possible
   wxF()->lePortRelease();
}

void SettingsControlPort::OnPortChoiceSelect(wxCommandEvent& WXUNUSED(event))
{
   // we have to set the dropdown selection here before calling CreatePortControls()
   // and it is set by the user

   // this leaves the previous port in a Default state
   wxF()->lePort()->SetLXDefault();
   wxF()->lePort()->CommitConfiguration(); // must be called after setting changes (also SetLXDefault())

   // here the new port is created and put in use
   CreatePortControls(true, true);  // load defaults for the new port
}

void SettingsControlPort::OnDefaultsButtonClick(wxCommandEvent& WXUNUSED(event))
{
   CreatePortControls(true, true); // load defaults for the port
}

bool SettingsControlPort::IsLExposing()
{
   if (wxWebcam* webcam = wxF()->webcam()) {
      return webcam->IsLExposing();
   }
   return false;
}
