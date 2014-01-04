#include "ArtemisProps.h"

//(*InternalHeaders(ArtemisProps)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "wxWebcam/wxArtcam.h"
#include "wxFactory.h"

//(*IdInit(ArtemisProps)
const long ArtemisProps::ID_STATICTEXT1 = wxNewId();
const long ArtemisProps::ID_SPINCTRL_BLACK = wxNewId();
const long ArtemisProps::ID_SLIDER_BLACK = wxNewId();
const long ArtemisProps::ID_STATICTEXT2 = wxNewId();
const long ArtemisProps::ID_SPINCTRL_WHITE = wxNewId();
const long ArtemisProps::ID_SLIDER_WHITE = wxNewId();
const long ArtemisProps::ID_STATICTEXT3 = wxNewId();
const long ArtemisProps::ID_SPINCTRL_GAMMA = wxNewId();
const long ArtemisProps::ID_SLIDER_GAMMA = wxNewId();
const long ArtemisProps::ID_STATICTEXT5 = wxNewId();
const long ArtemisProps::ID_SPINCTRL_GLOG = wxNewId();
const long ArtemisProps::ID_SLIDER_GLOG = wxNewId();
const long ArtemisProps::ID_CHECKBOX1 = wxNewId();
const long ArtemisProps::ID_BUTTON_RESET_GAMMA = wxNewId();
//*)

BEGIN_EVENT_TABLE(ArtemisProps,wxPanel)
	//(*EventTable(ArtemisProps)
	//*)
END_EVENT_TABLE()

ArtemisProps::ArtemisProps(wxWindow* parent,wxWindowID id)
{
	//(*Initialize(ArtemisProps)
	wxBoxSizer* BoxSizer4;
	wxBoxSizer* BoxSizer6;
	wxBoxSizer* BoxSizer5;
	wxStaticText* StaticText2;
	wxStaticText* StaticText1;
	wxBoxSizer* BoxSizer2;
	wxStaticText* StaticText3;
	wxStaticText* StaticText5;
	wxBoxSizer* BoxSizer1;
	wxBoxSizer* BoxSizer3;
	
	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Black"), wxDefaultPosition, wxSize(35,-1), 0, _T("ID_STATICTEXT1"));
	BoxSizer2->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	SpinCtrl_black = new wxSpinCtrl(this, ID_SPINCTRL_BLACK, _T("0"), wxDefaultPosition, wxSize(70,-1), 0, 0, 65535, 0, _T("ID_SPINCTRL_BLACK"));
	SpinCtrl_black->SetValue(_T("0"));
	BoxSizer2->Add(SpinCtrl_black, 0, wxTOP|wxBOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_black = new wxSlider(this, ID_SLIDER_BLACK, 0, 0, 32767, wxDefaultPosition, wxSize(110,20), 0, wxDefaultValidator, _T("ID_SLIDER_BLACK"));
	BoxSizer2->Add(Slider_black, 2, wxTOP|wxBOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("White"), wxDefaultPosition, wxSize(35,-1), 0, _T("ID_STATICTEXT2"));
	BoxSizer3->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	SpinCtrl_white = new wxSpinCtrl(this, ID_SPINCTRL_WHITE, _T("65535"), wxDefaultPosition, wxSize(70,-1), 0, 0, 65535, 65535, _T("ID_SPINCTRL_WHITE"));
	SpinCtrl_white->SetValue(_T("65535"));
	BoxSizer3->Add(SpinCtrl_white, 0, wxTOP|wxBOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_white = new wxSlider(this, ID_SLIDER_WHITE, 32767, 0, 32767, wxDefaultPosition, wxSize(110,20), 0, wxDefaultValidator, _T("ID_SLIDER_WHITE"));
	BoxSizer3->Add(Slider_white, 2, wxTOP|wxBOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer3, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Gamma"), wxDefaultPosition, wxSize(35,-1), 0, _T("ID_STATICTEXT3"));
	BoxSizer4->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	SpinCtrl_gamma = new wxSpinCtrlDbl(this, ID_SPINCTRL_GAMMA, _T("0.0"), wxDefaultPosition, wxSize(70,-1), 0, -10, 10, 0, _T("ID_SPINCTRL_GAMMA"));
	SpinCtrl_gamma->SetValue(_T("0.0"));
	BoxSizer4->Add(SpinCtrl_gamma, 0, wxTOP|wxBOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_gamma = new wxSlider(this, ID_SLIDER_GAMMA, 100, 0, 200, wxDefaultPosition, wxSize(110,20), 0, wxDefaultValidator, _T("ID_SLIDER_GAMMA"));
	BoxSizer4->Add(Slider_gamma, 2, wxTOP|wxBOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer4, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("GLog"), wxDefaultPosition, wxSize(35,-1), 0, _T("ID_STATICTEXT5"));
	BoxSizer6->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	SpinCtrl_glog = new wxSpinCtrlDbl(this, ID_SPINCTRL_GLOG, _T("1"), wxDefaultPosition, wxSize(70,-1), 0, 0, 10, 1, _T("ID_SPINCTRL_GLOG"));
	SpinCtrl_glog->SetValue(_T("1"));
	BoxSizer6->Add(SpinCtrl_glog, 0, wxTOP|wxBOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_glog = new wxSlider(this, ID_SLIDER_GLOG, 10, 0, 100, wxDefaultPosition, wxSize(110,20), 0, wxDefaultValidator, _T("ID_SLIDER_GLOG"));
	BoxSizer6->Add(Slider_glog, 2, wxTOP|wxBOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer6, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
	CheckBox_AutoStretch = new wxCheckBox(this, ID_CHECKBOX1, _("Auto histo. stretch"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_AutoStretch->SetValue(false);
	BoxSizer5->Add(CheckBox_AutoStretch, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ResetGamma_Button = new wxButton(this, ID_BUTTON_RESET_GAMMA, _("Reset Gamma"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_RESET_GAMMA"));
	BoxSizer5->Add(ResetGamma_Button, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer5, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
	
	Connect(ID_SPINCTRL_BLACK,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&ArtemisProps::OnSpinCtrl_blackChange);
	Connect(ID_SLIDER_BLACK,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&ArtemisProps::OnSlider_blackCmdScrollChanged);
	Connect(ID_SPINCTRL_WHITE,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&ArtemisProps::OnSpinCtrl_whiteChange1);
	Connect(ID_SLIDER_WHITE,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&ArtemisProps::OnSlider_whiteCmdScrollChanged);
	Connect(ID_SPINCTRL_GAMMA,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&ArtemisProps::OnSpinCtrl_gammaChange);
	Connect(ID_SLIDER_GAMMA,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&ArtemisProps::OnSlider_gammaCmdScrollChanged);
	Connect(ID_SPINCTRL_GLOG,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&ArtemisProps::OnSpinCtrl_glogChange);
	Connect(ID_SLIDER_GLOG,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&ArtemisProps::OnSlider_glogCmdScrollChanged);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ArtemisProps::OnCheckBox_AutoStretchClick);
	Connect(ID_BUTTON_RESET_GAMMA,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ArtemisProps::OnResetGamma_ButtonClick);
	//*)


   Init();
}

void ArtemisProps::Init()
{
   int ndigit_gamma = 6;
   SpinCtrl_gamma->SetValue(0.0);
   SpinCtrl_gamma->SetIncrement(0.1);
   SpinCtrl_gamma->SetRange(-10.0,10.0);
   SpinCtrl_gamma->SetDigits(ndigit_gamma);
   Slider_gamma->SetThumbLength(1);

   SpinCtrl_glog->SetValue(1.0);
   SpinCtrl_glog->SetIncrement(0.1);
   SpinCtrl_glog->SetRange(0.0,10);
   SpinCtrl_glog->SetDigits(ndigit_gamma);
   Slider_glog->SetThumbLength(1);

   // init controls
   double black_point,white_point, glog;
   bool  autoPt = CheckBox_AutoStretch->GetValue();

   if(wxArtcam* camera = wxF()->artcam()) {

      // get autoPt from camera
      camera->ImageProperties(glog, autoPt, black_point, white_point);
      if(autoPt) {
         // this will compute black_point & white_point, but will not return then
         camera->SetImageProperties(glog, autoPt, black_point, white_point);
         // Get black_point, white_point
         camera->ImageProperties(glog, autoPt, black_point, white_point);
      }

      SetBlackPoint(long(black_point));
      SetWhitePoint(long(white_point));
      SpinCtrl_glog->SetValue(glog);
      SetGamma(glog);
      CheckBox_AutoStretch->SetValue(autoPt);

      if(autoPt) {
         // disable sliders if autostretch is active
         SpinCtrl_black->Enable(false);
         Slider_black->Enable(false);
         SpinCtrl_white->Enable(false);
         Slider_white->Enable(false);
      }
   }

}

ArtemisProps::~ArtemisProps()
{
	//(*Destroy(ArtemisProps)
	//*)
}

void ArtemisProps::SetBlackPoint(long black)
{
   SpinCtrl_black->SetValue(black);
   Slider_black->SetValue(black/2);
}

void ArtemisProps::SetWhitePoint(long white)
{
   SpinCtrl_white->SetValue(white);
   Slider_white->SetValue(white/2);
}

void ArtemisProps::SetGlog(double gamma)
{
   double glog = exp(gamma/4.3);
   SpinCtrl_glog->SetValue(glog);
   Slider_glog->SetValue(int(glog*10.0));
}

void ArtemisProps::SetGamma(double glog)
{
   if(glog < 0.1)glog = 0.1;
   double gamma = 4.3*log(glog);
   SpinCtrl_gamma->SetValue(gamma);
   Slider_gamma->SetValue(int((gamma+10)*10.0));
}

void ArtemisProps::OnSpinCtrl_blackChange(wxSpinEvent& WXUNUSED(event))
{
   Slider_black->SetValue(SpinCtrl_black->GetValue()/2);
   UpdateCameraProperties(false);
}

void ArtemisProps::OnSlider_blackCmdScrollChanged(wxScrollEvent& WXUNUSED(event))
{
   SpinCtrl_black->SetValue(Slider_black->GetValue()*2);
   UpdateCameraProperties(false);
}

void ArtemisProps::OnSpinCtrl_whiteChange1(wxSpinEvent& WXUNUSED(event))
{
   Slider_white->SetValue(SpinCtrl_white->GetValue()/2);
   UpdateCameraProperties(false);
}

void ArtemisProps::OnSlider_whiteCmdScrollChanged(wxScrollEvent& WXUNUSED(event))
{
   SpinCtrl_white->SetValue(Slider_white->GetValue()*2);
   UpdateCameraProperties(false);
}

void ArtemisProps::OnSpinCtrl_gammaChange(wxSpinEvent& WXUNUSED(event))
{
   Slider_gamma->SetValue(int((SpinCtrl_gamma->GetValue()+10)*10.0));
   SetGlog(SpinCtrl_gamma->GetValue());
   UpdateCameraProperties(true);
}

void ArtemisProps::OnSlider_gammaCmdScrollChanged(wxScrollEvent& event)
{
   double gamma = event.GetPosition();
   SpinCtrl_gamma->SetValue(gamma/10.-10);
   SetGlog(SpinCtrl_gamma->GetValue());
   UpdateCameraProperties(true);
}


void ArtemisProps::OnSpinCtrl_glogChange(wxSpinEvent& WXUNUSED(event))
{
   Slider_glog->SetValue(int(SpinCtrl_glog->GetValue()*10.0));
   SetGamma(SpinCtrl_glog->GetValue());
   UpdateCameraProperties(true);
}


void ArtemisProps::OnSlider_glogCmdScrollChanged(wxScrollEvent& event)
{
   double glog = event.GetPosition();
   SpinCtrl_glog->SetValue(glog*0.1);
   SetGamma(SpinCtrl_glog->GetValue());
   UpdateCameraProperties(true);
}



void ArtemisProps::OnCheckBox_AutoStretchClick(wxCommandEvent& WXUNUSED(event))
{
//   UpdateCameraProperties();

   double black_point(0.0), white_point(65535);
   double glog  = SpinCtrl_glog->GetValue();

   // reset camera black and white points to native values to allow
   // computing new auto values via histogram
   wxArtcam* camera = wxF()->artcam();

   if(bool autoPt = CheckBox_AutoStretch->GetValue()) {
      if(camera) {
         unsigned short pivot,vlow,vhig,max;
         camera->Histogram().PivotDev(pivot,vlow,vhig,max);

         // actual 16 bit black/white points
         double scale = white_point/double(max);
         black_point = vlow*scale;
         white_point = vhig*scale;

         ///BM: auto should work now
         camera->SetImageProperties(glog, true, black_point, white_point);

         // Get black_point, white_point values computed and update GUI
         camera->ImageProperties(glog, autoPt, black_point, white_point);
         SetBlackPoint(long(black_point));
         SetWhitePoint(long(white_point));

         SpinCtrl_black->Enable(false);
         Slider_black->Enable(false);
         SpinCtrl_white->Enable(false);
         Slider_white->Enable(false);
      }
   }
   else {

      if(camera) {

         //change only autostretch, keep black & white points
         bool autoPoint;
         camera->ImageProperties(glog,autoPoint,black_point,white_point);
         camera->SetImageProperties(glog,false,black_point,white_point);
         SetBlackPoint(long(black_point));
         SetWhitePoint(long(white_point));
      }

      SpinCtrl_black->Enable(true);
      Slider_black->Enable(true);
      SpinCtrl_white->Enable(true);
      Slider_white->Enable(true);

   }
}


void ArtemisProps::OnResetGamma_ButtonClick(wxCommandEvent& WXUNUSED(event))
{
   SetGlog(0.0);
   SetGamma(1.0);
   UpdateCameraProperties(true);
}

void  ArtemisProps::UpdateCameraProperties(bool holdAuto)
{
   double black_point = SpinCtrl_black->GetValue();
   double white_point = SpinCtrl_white->GetValue();
   double glog       = SpinCtrl_glog->GetValue();
   bool   autoPt      = CheckBox_AutoStretch->GetValue();

   if(wxArtcam* camera = wxF()->artcam()) {
      ///BM: e.g. gamma will retiain the AutoBox setting
      if (holdAuto) {
         camera->SetImageProperties(glog, autoPt, black_point, white_point);
      }
      else {
         // do not set auto parameters here
         CheckBox_AutoStretch->SetValue(false); ///BM: disable autobox if needed
         camera->SetImageProperties(glog, false, black_point, white_point);
      }
   }
}




