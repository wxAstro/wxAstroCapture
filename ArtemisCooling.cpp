#include "ArtemisCooling.h"

//(*InternalHeaders(ArtemisCooling)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "wxthings/spinctld.h"
#include "wxWebcam/wxCamera.h"
#include "wxFactory.h"

//(*IdInit(ArtemisCooling)
const long ArtemisCooling::ID_STATICTEXT1 = wxNewId();
const long ArtemisCooling::ID_STATICTEXT2 = wxNewId();
const long ArtemisCooling::ID_STATICTEXT4 = wxNewId();
const long ArtemisCooling::ID_STATICTEXT5 = wxNewId();
const long ArtemisCooling::ID_STATICTEXT3 = wxNewId();
const long ArtemisCooling::ID_SPINCTRL1 = wxNewId();
const long ArtemisCooling::ID_BUTTON1 = wxNewId();
const long ArtemisCooling::ID_BUTTON2 = wxNewId();
//*)

#if wxUSE_UNICODE
  static const wxString Celsius = _T("\x2103");
#else
  static const wxString Celsius = _T("°C");
#endif

BEGIN_EVENT_TABLE(ArtemisCooling,wxPanel)
	//(*EventTable(ArtemisCooling)
	//*)
	EVT_IDLE(ArtemisCooling::OnIdle)
END_EVENT_TABLE()

ArtemisCooling::ArtemisCooling(wxWindow* parent,wxWindowID WXUNUSED(id),const wxPoint& WXUNUSED(pos),const wxSize& WXUNUSED(size))
{
	//(*Initialize(ArtemisCooling)
	wxBoxSizer* BoxSizer6;
	wxBoxSizer* BoxSizer5;
	wxStaticText* StaticText1;
	wxBoxSizer* BoxSizer2;
	wxStaticText* StaticText3;
	wxButton* StartWarmup;
	wxBoxSizer* BoxSizer1;
	wxBoxSizer* BoxSizer3;
	wxStaticText* StaticText4;
	wxButton* SetTarget;
	
	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Current temp"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	BoxSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_cur_temp = new wxStaticText(this, ID_STATICTEXT2, _("10.00 C"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	BoxSizer2->Add(m_cur_temp, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	BoxSizer1->Add(BoxSizer2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Cooling status"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	BoxSizer6->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_cooling_status = new wxStaticText(this, ID_STATICTEXT5, _("Cooling down"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	BoxSizer6->Add(m_cooling_status, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	BoxSizer1->Add(BoxSizer6, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Target temp"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	BoxSizer3->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_target_spin = new wxSpinCtrlDbl(this, ID_SPINCTRL1, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 100, 0, _T("ID_SPINCTRL1"));
	m_target_spin->SetValue(_T("0"));
	BoxSizer3->Add(m_target_spin, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	BoxSizer1->Add(BoxSizer3, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
	StartWarmup = new wxButton(this, ID_BUTTON1, _("Start Warmup"), wxDefaultPosition, wxSize(80,-1), 0, wxDefaultValidator, _T("ID_BUTTON1"));
	BoxSizer5->Add(StartWarmup, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SetTarget = new wxButton(this, ID_BUTTON2, _("Set Target"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	BoxSizer5->Add(SetTarget, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer5, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
	
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ArtemisCooling::OnStartWarmupClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ArtemisCooling::OnSetTargetClick);
	//*)

	Init();
}

ArtemisCooling::~ArtemisCooling()
{
	//(*Destroy(ArtemisCooling)
	//*)
}

void ArtemisCooling::Init()
{
   m_target_spin->SetRange(-100.0,100.0);
   m_target_spin->SetIncrement(0.5);
   if(wxCamera* cam = wxF()->cam()) {
      m_target_spin->SetValue(cam->CoolingSetpoint());
   }
}

void ArtemisCooling::OnIdle(wxIdleEvent& WXUNUSED(event))
{
   if(wxCamera* cam = wxF()->cam()) {
      if(cam->HasCooling()) {

         bool redo_layout = false;

         wxString cur_temp = wxString::Format(_T("%5.1lf")+Celsius,cam->Temperature() );
         if(m_cur_temp->GetLabel() != cur_temp) {
            m_cur_temp->SetLabel(cur_temp);
            redo_layout = true;
         }

         ECoolingStatus cstat = cam->CoolingStatus();
         wxString ctxt;
         switch(cstat) {
            case ECS_Off:     { ctxt = wxT("Off");        break; }
            case ECS_Cooling: { ctxt = wxT("Cooling");    break; }
            case ECS_Holding: { ctxt = wxT("Holding");    break; } /// new status, new text
            case ECS_Warmup:  { ctxt = wxT("Warming");    break; }
            default:          { ctxt = wxT("N/A");               }
         }

         if(m_cooling_status->GetLabel() != ctxt) {
            m_cooling_status->SetLabel(ctxt);
            redo_layout = true;
         }

         if(redo_layout)Layout();
      }
   }
}

void ArtemisCooling::OnStartWarmupClick(wxCommandEvent& WXUNUSED(event))
{
   if(wxCamera* cam = wxF()->cam()) {
      if(cam->HasCooling()) {
         cam->SetWarmup();
      }
   }
}

void ArtemisCooling::OnSetTargetClick(wxCommandEvent& WXUNUSED(event))
{
   if(wxCamera* cam = wxF()->cam()) {
      if(cam->HasCooling()) {
         double temp = m_target_spin->GetValue();
         cam->SetTemperature(temp);
      }
   }
}
