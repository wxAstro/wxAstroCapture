#include "SettingsGuiding.h"
#include "wxGuidingConfig.h"
#include "SettingsGuidingCalibrateDlg.h"

//(*InternalHeaders(SettingsGuiding)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/msgdlg.h>

//(*IdInit(SettingsGuiding)
const long SettingsGuiding::ID_STATICTEXT1 = wxNewId();
const long SettingsGuiding::ID_SPINCTRL1 = wxNewId();
const long SettingsGuiding::ID_STATICTEXT5 = wxNewId();
const long SettingsGuiding::ID_SPINCTRL3 = wxNewId();
const long SettingsGuiding::ID_STATICTEXT2 = wxNewId();
const long SettingsGuiding::ID_SPINCTRL4 = wxNewId();
const long SettingsGuiding::ID_STATICTEXT4 = wxNewId();
const long SettingsGuiding::ID_SPINCTRL2 = wxNewId();
const long SettingsGuiding::ID_STATICTEXT8 = wxNewId();
const long SettingsGuiding::ID_SPINCTRL6 = wxNewId();
const long SettingsGuiding::ID_BUTTON1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SettingsGuiding,wxPanel)
	//(*EventTable(SettingsGuiding)
	//*)
END_EVENT_TABLE()

SettingsGuiding::SettingsGuiding(wxWindow* parent, wxWindowID WXUNUSED(id), const wxPoint& WXUNUSED(pos), const wxSize& WXUNUSED(size), long WXUNUSED(style), const wxString& WXUNUSED(name))
{
	//(*Initialize(SettingsGuiding)
	wxBoxSizer* BoxSizer4;
	wxBoxSizer* BoxSizer6;
	wxBoxSizer* BoxSizer5;
	wxBoxSizer* BoxSizer7;
	wxBoxSizer* BoxSizer8;
	wxStaticText* StaticText8;
	wxStaticBoxSizer* StaticBoxSizerCalibrate;
	wxBoxSizer* BoxSizer2;
	wxStaticText* StaticText3;
	wxStaticText* StaticText7;
	wxBoxSizer* BoxSizer1;
	wxBoxSizer* BoxSizer9;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxBoxSizer* BoxSizer3;
	wxStaticText* StaticText4;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Guide star image tracking"));
	BoxSizer2 = new wxBoxSizer(wxVERTICAL);
	BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT1, _("Subframe size [pixels]"), wxDefaultPosition, wxSize(130,-1), 0, _T("ID_STATICTEXT1"));
	BoxSizer3->Add(StaticText3, 1, wxBOTTOM|wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_box_size = new wxSpinCtrl(this, ID_SPINCTRL1, _T("0"), wxDefaultPosition, wxSize(60,-1), 0, 0, 255, 0, _T("ID_SPINCTRL1"));
	m_box_size->SetValue(_T("0"));
	BoxSizer3->Add(m_box_size, 0, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer2->Add(BoxSizer3, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	BoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT5, _("Lower cutoff level"), wxDefaultPosition, wxSize(130,-1), 0, _T("ID_STATICTEXT5"));
	BoxSizer4->Add(StaticText4, 1, wxBOTTOM|wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_bckg_cutoff = new wxSpinCtrl(this, ID_SPINCTRL3, _T("0"), wxDefaultPosition, wxSize(60,-1), 0, 0, 100, 0, _T("ID_SPINCTRL3"));
	m_bckg_cutoff->SetValue(_T("0"));
	BoxSizer4->Add(m_bckg_cutoff, 0, wxBOTTOM|wxRIGHT|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer2->Add(BoxSizer4, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	BoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT2, _("Sampling interval [ms]"), wxDefaultPosition, wxSize(150,-1), 0, _T("ID_STATICTEXT2"));
	BoxSizer5->Add(StaticText1, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_interval = new wxSpinCtrl(this, ID_SPINCTRL4, _T("0"), wxDefaultPosition, wxSize(60,-1), 0, 0, 100, 0, _T("ID_SPINCTRL4"));
	m_interval->SetValue(_T("0"));
	BoxSizer5->Add(m_interval, 0, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer2->Add(BoxSizer5, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	StaticBoxSizer1->Add(BoxSizer2, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	BoxSizer1->Add(StaticBoxSizer1, 0, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(-1,35,0, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizerCalibrate = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Guide telescope && camera"));
	BoxSizer6 = new wxBoxSizer(wxVERTICAL);
	BoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT4, _("Image scale [\"/pixel]"), wxDefaultPosition, wxSize(130,-1), 0, _T("ID_STATICTEXT4"));
	BoxSizer7->Add(StaticText7, 1, wxBOTTOM|wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_arcsec_SpinCtrl = new wxSpinCtrlDbl(this, ID_SPINCTRL2, _T("0"), wxDefaultPosition, wxSize(60,-1), 0, 0, 100, 0, _T("ID_SPINCTRL2"));
	m_arcsec_SpinCtrl->SetValue(_T("0"));
	BoxSizer7->Add(m_arcsec_SpinCtrl, 0, wxBOTTOM|wxRIGHT|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer6->Add(BoxSizer7, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	BoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Camera angle [d]"), wxDefaultPosition, wxSize(150,-1), 0, _T("ID_STATICTEXT8"));
	BoxSizer8->Add(StaticText8, 1, wxBOTTOM|wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_rotation_SpinCtrl = new wxSpinCtrlDbl(this, ID_SPINCTRL6, _T("0"), wxDefaultPosition, wxSize(60,-1), 0, -180, 180, 0, _T("ID_SPINCTRL6"));
	m_rotation_SpinCtrl->SetValue(_T("0"));
	BoxSizer8->Add(m_rotation_SpinCtrl, 0, wxBOTTOM|wxRIGHT|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer6->Add(BoxSizer8, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	BoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
	AutoCalibrateButton = new wxButton(this, ID_BUTTON1, _("Auto calibrate..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	BoxSizer9->Add(AutoCalibrateButton, 0, wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer6->Add(BoxSizer9, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	StaticBoxSizerCalibrate->Add(BoxSizer6, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	BoxSizer1->Add(StaticBoxSizerCalibrate, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SettingsGuiding::OnAutoCalibrateButtonClick);
	//*)

	Init();
}

SettingsGuiding::~SettingsGuiding()
{
	//(*Destroy(SettingsGuiding)
	//*)
}

void SettingsGuiding::Init()
{
   // Guiding
   m_box_size->SetRange(0,200);
   m_bckg_cutoff->SetRange(0,255);
   m_interval->SetRange(100,60000);

   m_arcsec_SpinCtrl->SetRange(0.0,3600.0);
   m_arcsec_SpinCtrl->SetIncrement(0.1);

   m_rotation_SpinCtrl->SetRange(-180.0,180.0);
   m_rotation_SpinCtrl->SetIncrement(0.25);

   int guideBoxSize,bacgroundCutoff,guideInterval;
   double imageScale,cameraAngle;

   if(wxGuidingConfig::getGuidingConfig(guideBoxSize,bacgroundCutoff,guideInterval,imageScale,cameraAngle))
   {
      m_box_size->SetValue(guideBoxSize);
      m_bckg_cutoff->SetValue(bacgroundCutoff);
      m_interval->SetValue(guideInterval);
      m_arcsec_SpinCtrl->SetValue(imageScale);
      m_rotation_SpinCtrl->SetValue(cameraAngle);
   }

   SetInit(true);
}

bool SettingsGuiding::OnApply()
{
   return wxGuidingConfig::saveGuidingConfig(m_box_size->GetValue(),
                                             m_bckg_cutoff->GetValue(),
                                             m_interval->GetValue(),
                                             m_arcsec_SpinCtrl->GetValue(),
                                             m_rotation_SpinCtrl->GetValue());
}

void SettingsGuiding::OnAutoCalibrateButtonClick(wxCommandEvent& WXUNUSED(event))
{
   // No need to manage the pointer here, the dialog will self destruct on close
   SettingsGuidingCalibrateDlg* calibrateDlg = new SettingsGuidingCalibrateDlg(this);
   calibrateDlg->Show();


//    wxMessageBox(_T("Sorry, autocalibration has not yet been implemented. \nPlease use manual options." ));
}


bool SettingsGuiding::SetAngleValue(const wxString& cameraAngle)
{
   m_rotation_SpinCtrl->SetValue(cameraAngle);
   return true;
}

bool SettingsGuiding::SetArcsecValue(const wxString& arcsec)
{
   m_arcsec_SpinCtrl->SetValue(arcsec);
   return true;
}
