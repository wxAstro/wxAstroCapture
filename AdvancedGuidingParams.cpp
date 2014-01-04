#include "AdvancedGuidingParams.h"

//(*InternalHeaders(AdvancedGuidingParams)
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "wxFactory.h"
#include "wxGuider.h"

//(*IdInit(AdvancedGuidingParams)
const long AdvancedGuidingParams::ID_STATICTEXT5 = wxNewId();
const long AdvancedGuidingParams::ID_STATICTEXT4 = wxNewId();
const long AdvancedGuidingParams::ID_STATICTEXT1 = wxNewId();
const long AdvancedGuidingParams::ID_SPINCTRL1 = wxNewId();
const long AdvancedGuidingParams::ID_SPINCTRL2 = wxNewId();
const long AdvancedGuidingParams::ID_STATICTEXT2 = wxNewId();
const long AdvancedGuidingParams::ID_SPINCTRL3 = wxNewId();
const long AdvancedGuidingParams::ID_SPINCTRL4 = wxNewId();
const long AdvancedGuidingParams::ID_STATICTEXT3 = wxNewId();
const long AdvancedGuidingParams::ID_SPINCTRL5 = wxNewId();
const long AdvancedGuidingParams::ID_SPINCTRL6 = wxNewId();
const long AdvancedGuidingParams::ID_BUTTON1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(AdvancedGuidingParams,wxDialog)
	//(*EventTable(AdvancedGuidingParams)
	//*)
END_EVENT_TABLE()

AdvancedGuidingParams* AdvancedGuidingParams::m_self=0;

AdvancedGuidingParams::AdvancedGuidingParams(wxWindow* parent,wxWindowID id)
{
	//(*Initialize(AdvancedGuidingParams)
	wxButton* ResetDefaultsButton;
	wxStaticText* StaticText2;
	wxStaticText* StaticText1;
	wxStaticText* StaticText3;
	wxStaticText* StaticText5;
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;
	wxStaticText* StaticText4;
	
	Create(parent, id, _("Advanced"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	FlexGridSizer1 = new wxFlexGridSizer(3, 3, 0, 0);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("RA"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	wxFont StaticText5Font(12,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	StaticText5->SetFont(StaticText5Font);
	FlexGridSizer1->Add(StaticText5, 1, wxTOP|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("DEC"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	wxFont StaticText4Font(12,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	StaticText4->SetFont(StaticText4Font);
	FlexGridSizer1->Add(StaticText4, 1, wxTOP|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Dead\nZone"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxTOP|wxBOTTOM|wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_RA_dz_SpinCtrl = new wxSpinCtrlDbl(this, ID_SPINCTRL1, _T("0.5"), wxDefaultPosition, wxSize(50,-1), 0, 0, 100, 0, _T("ID_SPINCTRL1"));
	m_RA_dz_SpinCtrl->SetValue(_T("0.5"));
	FlexGridSizer1->Add(m_RA_dz_SpinCtrl, 1, wxTOP|wxBOTTOM|wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_DEC_dz_SpinCtrl = new wxSpinCtrlDbl(this, ID_SPINCTRL2, _T("0.7"), wxDefaultPosition, wxSize(50,-1), 0, 0, 100, 0, _T("ID_SPINCTRL2"));
	m_DEC_dz_SpinCtrl->SetValue(_T("0.7"));
	FlexGridSizer1->Add(m_DEC_dz_SpinCtrl, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("K"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxTOP|wxBOTTOM|wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_RA_k_SpinCtrl = new wxSpinCtrlDbl(this, ID_SPINCTRL3, _T("20.0"), wxDefaultPosition, wxSize(50,-1), 0, 0, 100, 20, _T("ID_SPINCTRL3"));
	m_RA_k_SpinCtrl->SetValue(_T("20.0"));
	FlexGridSizer1->Add(m_RA_k_SpinCtrl, 1, wxTOP|wxBOTTOM|wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_DEC_k_SpinCtrl = new wxSpinCtrlDbl(this, ID_SPINCTRL4, _T("30.0"), wxDefaultPosition, wxSize(50,-1), 0, 0, 100, 30, _T("ID_SPINCTRL4"));
	m_DEC_k_SpinCtrl->SetValue(_T("30.0"));
	FlexGridSizer1->Add(m_DEC_k_SpinCtrl, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Q"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxTOP|wxBOTTOM|wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_RA_q_SpinCtrl = new wxSpinCtrlDbl(this, ID_SPINCTRL5, _T("40.0"), wxDefaultPosition, wxSize(50,-1), 0, 0, 100, 40, _T("ID_SPINCTRL5"));
	m_RA_q_SpinCtrl->SetValue(_T("40.0"));
	FlexGridSizer1->Add(m_RA_q_SpinCtrl, 1, wxTOP|wxBOTTOM|wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_DEC_q_SpinCtrl = new wxSpinCtrlDbl(this, ID_SPINCTRL6, _T("40.0"), wxDefaultPosition, wxSize(50,-1), 0, 0, 100, 40, _T("ID_SPINCTRL6"));
	m_DEC_q_SpinCtrl->SetValue(_T("40.0"));
	FlexGridSizer1->Add(m_DEC_q_SpinCtrl, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(FlexGridSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	ResetDefaultsButton = new wxButton(this, ID_BUTTON1, _("Reset Defaults"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	BoxSizer1->Add(ResetDefaultsButton, 0, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
	
	Connect(ID_SPINCTRL1,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&AdvancedGuidingParams::OnRA_dz_SpinCtrlChange);
	Connect(ID_SPINCTRL2,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&AdvancedGuidingParams::OnDEC_dz_SpinCtrlChange);
	Connect(ID_SPINCTRL3,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&AdvancedGuidingParams::OnRA_k_SpinCtrlChange);
	Connect(ID_SPINCTRL4,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&AdvancedGuidingParams::OnDEC_k_SpinCtrlChange);
	Connect(ID_SPINCTRL5,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&AdvancedGuidingParams::OnRA_q_SpinCtrlChange);
	Connect(ID_SPINCTRL6,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&AdvancedGuidingParams::OnDEC_q_SpinCtrlChange);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&AdvancedGuidingParams::OnResetDefaultsButtonClick);
	Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&AdvancedGuidingParams::OnClose);
	//*)

   m_self = this;
   Init();
}

AdvancedGuidingParams* AdvancedGuidingParams::singleton(wxWindow* parent)
{
   if(!m_self){
      m_self = new AdvancedGuidingParams(parent);
   }
   m_self->InitPosition();
   return m_self;
}

AdvancedGuidingParams::~AdvancedGuidingParams()
{
	//(*Destroy(AdvancedGuidingParams)
	//*)
}

void AdvancedGuidingParams::InitPosition()
{
   int fx(0),fy(0);
   GetParent()->GetScreenPosition(&fx,&fy);
   Move(fx-15,fy-20);
}

void AdvancedGuidingParams::Init()
{
   m_RA_dz_SpinCtrl->SetRange(0,100.0);    m_RA_dz_SpinCtrl->SetIncrement(0.1);
   m_RA_k_SpinCtrl->SetRange(0,200.0);     m_RA_k_SpinCtrl->SetIncrement(1.0);
   m_RA_q_SpinCtrl->SetRange(0,1000.0);    m_RA_q_SpinCtrl->SetIncrement(1.0);

   m_DEC_dz_SpinCtrl->SetRange(0,100.0);   m_DEC_dz_SpinCtrl->SetIncrement(0.1);
   m_DEC_k_SpinCtrl->SetRange(0,200.0);    m_DEC_k_SpinCtrl->SetIncrement(1.0);
   m_DEC_q_SpinCtrl->SetRange(0,1000.0);   m_DEC_q_SpinCtrl->SetIncrement(1.0);

   getValuesFromGuider();
}

void AdvancedGuidingParams::getValuesFromGuider()
{
   if(wxGuider* guider = wxF()->guider()) {

      // set defaults for RA
      m_RA_dz_SpinCtrl->SetValue(guider->raDz());
      m_RA_k_SpinCtrl->SetValue(guider->raK());
      m_RA_q_SpinCtrl->SetValue(guider->raQ());

      // set defaults for DEC
      m_DEC_dz_SpinCtrl->SetValue(guider->decDz());
      m_DEC_k_SpinCtrl->SetValue(guider->decK());
      m_DEC_q_SpinCtrl->SetValue(guider->decQ());
   }
}

void AdvancedGuidingParams::OnClose(wxCloseEvent& WXUNUSED(event))
{
   Destroy();
   m_self = 0;
}

void AdvancedGuidingParams::OnResetDefaultsButtonClick(wxCommandEvent& WXUNUSED(event))
{
   if(wxGuider* guider = wxF()->guider()) {
      guider->setTuningDefaults();
      getValuesFromGuider();
   }
}

void AdvancedGuidingParams::OnRA_dz_SpinCtrlChange(wxSpinEvent& WXUNUSED(event))
{
   if(wxGuider* guider = wxF()->guider()) {
      guider->setRaDz(m_RA_dz_SpinCtrl->GetValue());
   }
}

void AdvancedGuidingParams::OnRA_k_SpinCtrlChange(wxSpinEvent& WXUNUSED(event))
{
   if(wxGuider* guider = wxF()->guider()) {
      guider->setRaK(m_RA_k_SpinCtrl->GetValue());
   }
}

void AdvancedGuidingParams::OnRA_q_SpinCtrlChange(wxSpinEvent& WXUNUSED(event))
{
   if(wxGuider* guider = wxF()->guider()) {
      guider->setRaQ(m_RA_q_SpinCtrl->GetValue());
   }
}

void AdvancedGuidingParams::OnDEC_dz_SpinCtrlChange(wxSpinEvent& WXUNUSED(event))
{
   if(wxGuider* guider = wxF()->guider()) {
      guider->setDecDz(m_DEC_dz_SpinCtrl->GetValue());
   }
}

void AdvancedGuidingParams::OnDEC_k_SpinCtrlChange(wxSpinEvent& WXUNUSED(event))
{
   if(wxGuider* guider = wxF()->guider()) {
      guider->setDecK(m_DEC_k_SpinCtrl->GetValue());
   }
}

void AdvancedGuidingParams::OnDEC_q_SpinCtrlChange(wxSpinEvent& WXUNUSED(event))
{
   if(wxGuider* guider = wxF()->guider()) {
      guider->setDecQ(m_DEC_q_SpinCtrl->GetValue());
   }
}
