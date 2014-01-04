#include "PrimaryProps.h"

//(*InternalHeaders(PrimaryProps)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "wx/bookctrl.h"
#include "wxWebcam/wxWebcam.h"
#include "wxFactory.h"

//(*IdInit(PrimaryProps)
const long PrimaryProps::ID_STATICTEXT1 = wxNewId();
const long PrimaryProps::ID_SLIDER1 = wxNewId();
const long PrimaryProps::ID_STATICTEXT2 = wxNewId();
const long PrimaryProps::ID_SLIDER2 = wxNewId();
const long PrimaryProps::ID_STATICTEXT3 = wxNewId();
const long PrimaryProps::ID_SLIDER3 = wxNewId();
const long PrimaryProps::ID_STATICTEXT6 = wxNewId();
const long PrimaryProps::ID_CHOICE1 = wxNewId();
const long PrimaryProps::ID_CHECKBOX2 = wxNewId();
const long PrimaryProps::ID_CHECKBOX1 = wxNewId();
const long PrimaryProps::ID_RADIOBOX2 = wxNewId();
//*)

const long PrimaryProps::ID_TIMER1= wxNewId();

BEGIN_EVENT_TABLE(PrimaryProps,wxPanel)
	//(*EventTable(PrimaryProps)
	//*)
   EVT_TIMER(PrimaryProps::ID_TIMER1,PrimaryProps::OnTimer)
END_EVENT_TABLE()

PrimaryProps::PrimaryProps(wxBookCtrlBase* parent,wxWindowID id)
: m_timer(this,ID_TIMER1)
,m_parent(parent)
,m_edit_busy(false)
{
	//(*Initialize(PrimaryProps)
	wxStaticText* StaticText2;
	wxStaticText* StaticText6;
	wxFlexGridSizer* FlexGridSizer2;
	wxStaticText* StaticText1;
	wxStaticText* StaticText3;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->Add(5,5,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(5,5,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Brigh"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_brightness_slider = new wxSlider(this, ID_SLIDER1, 50, 0, 100, wxDefaultPosition, wxSize(140,40), wxSL_LABELS, wxDefaultValidator, _T("ID_SLIDER1"));
	m_brightness_slider->Disable();
	FlexGridSizer1->Add(m_brightness_slider, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Gam"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_gamma_slider = new wxSlider(this, ID_SLIDER2, 0, 0, 100, wxDefaultPosition, wxSize(-1,40), wxSL_LABELS, wxDefaultValidator, _T("ID_SLIDER2"));
	m_gamma_slider->Disable();
	FlexGridSizer1->Add(m_gamma_slider, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(10,30,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Gain"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_gain_slider = new wxSlider(this, ID_SLIDER3, 0, 0, 100, wxDefaultPosition, wxSize(-1,40), wxSL_LABELS, wxDefaultValidator, _T("ID_SLIDER3"));
	m_gain_slider->Disable();
	FlexGridSizer1->Add(m_gain_slider, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Shut"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer1->Add(StaticText6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_shutter_choice = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	m_shutter_choice->Append(_("1/25"));
	m_shutter_choice->Append(_("1/33"));
	m_shutter_choice->Append(_("1/50"));
	m_shutter_choice->Append(_("1/100"));
	m_shutter_choice->Append(_("1/250"));
	m_shutter_choice->Append(_("1/500"));
	m_shutter_choice->Append(_("1/1000"));
	m_shutter_choice->Append(_("1/1500"));
	m_shutter_choice->Append(_("1/2500"));
	m_shutter_choice->Append(_("1/5000"));
	m_shutter_choice->Append(_("1/10000"));
	m_shutter_choice->Disable();
	FlexGridSizer1->Add(m_shutter_choice, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(FlexGridSizer1, 4, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	m_checkbox_fullauto = new wxCheckBox(this, ID_CHECKBOX2, _("Full auto"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	m_checkbox_fullauto->SetValue(false);
	FlexGridSizer2->Add(m_checkbox_fullauto, 1, wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_checkbox_autoexp = new wxCheckBox(this, ID_CHECKBOX1, _("Auto exp."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	m_checkbox_autoexp->SetValue(false);
	FlexGridSizer2->Add(m_checkbox_autoexp, 1, wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	wxString __wxRadioBoxChoices_1[7] =
	{
		_("5"),
		_("10"),
		_("15"),
		_("20"),
		_("25"),
		_("30"),
		_("60")
	};
	m_radiobox_fps = new wxRadioBox(this, ID_RADIOBOX2, _("FPS"), wxDefaultPosition, wxDefaultSize, 7, __wxRadioBoxChoices_1, 1, wxRA_SPECIFY_COLS, wxDefaultValidator, _T("ID_RADIOBOX2"));
	FlexGridSizer2->Add(m_radiobox_fps, 1, wxALL|wxEXPAND|wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 4);
	BoxSizer1->Add(FlexGridSizer2, 2, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);

	Connect(ID_SLIDER1,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&PrimaryProps::OnSliderScrollThumbTrack);
	Connect(ID_SLIDER1,wxEVT_SCROLL_THUMBRELEASE,(wxObjectEventFunction)&PrimaryProps::OnSliderScrollThumbRelease);
	Connect(ID_SLIDER1,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&PrimaryProps::OnBrightnessSliderScrollChanged);
	Connect(ID_SLIDER2,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&PrimaryProps::OnSliderScrollThumbTrack);
	Connect(ID_SLIDER2,wxEVT_SCROLL_THUMBRELEASE,(wxObjectEventFunction)&PrimaryProps::OnSliderScrollThumbRelease);
	Connect(ID_SLIDER2,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&PrimaryProps::OnGammaSliderScrollChanged);
	Connect(ID_SLIDER3,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&PrimaryProps::OnSliderScrollThumbTrack);
	Connect(ID_SLIDER3,wxEVT_SCROLL_THUMBRELEASE,(wxObjectEventFunction)&PrimaryProps::OnSliderScrollThumbRelease);
	Connect(ID_SLIDER3,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&PrimaryProps::OnGainSliderScrollChanged);
	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&PrimaryProps::OnShutterChoiceSelect);
	Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&PrimaryProps::OnCheckboxFullautoClick);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&PrimaryProps::OnCheckboxAutoExpClick);
	Connect(ID_RADIOBOX2,wxEVT_COMMAND_RADIOBOX_SELECTED,(wxObjectEventFunction)&PrimaryProps::OnRadioboxFpsSelect);
	//*)

   Init();

   // re-initialise every xxxx msecond
   m_timer.Start(1000);
}

void PrimaryProps::Init()
{
   // if we are busy editing, do nothing
   if(m_edit_busy)return;

   // if this page is not the current one, do nothing
   if(m_parent->GetCurrentPage() != this)return;


   if(wxWebcam* camera = wxFactory::singleton()->webcam()) {

      // if the camera is busy capturing, do nothing
      if(camera->IsCapturing())return;

      // get access to the camera properties object
      wxWebcam::DeviceProperties* properties = camera->properties();

      bool has_auto_exposure = properties->HasProperty_AutoExposure()>0;
      bool auto_on = false;
      if(has_auto_exposure) {
         m_checkbox_autoexp->Enable(true);
         auto_on = properties->Property_AutoExposure();
      }

      if( properties->HasProperty_Brightness()) {
         m_brightness_slider->Enable(true);
         m_brightness_slider->SetValue(properties->Property_Brightness(true));
      }
      else m_brightness_slider->Enable(false);

      if(properties->HasProperty_Gamma()) {
         m_gamma_slider->Enable(true);
         m_gamma_slider->SetValue(properties->Property_Gamma(true));
      }
      else m_gamma_slider->Enable(false);

      bool has_gain=false;
      if(properties->HasProperty_Gain()) {
         bool enable = (auto_on)? false : true;
         m_gain_slider->Enable(enable);
         m_gain_slider->SetValue(properties->Property_Gain(true));
         has_gain = true;
      }
      else m_gain_slider->Enable(false);

      bool has_shutter=false;
      if(properties->HasProperty_Exposure()) {
         bool enable = (auto_on)? false : true;
         m_shutter_choice->Enable(enable);
         m_shutter_choice->SetSelection(properties->Property_Exposure());
         has_shutter = true;
      }
      else m_shutter_choice->Enable(false);

      if(properties->HasProperty_FPS()) {
         m_radiobox_fps->Enable(true);

         long min,max,step;
         properties->PropertyMinMax_FPS(min,max,step);

         // disable the FPS buttons that the camera doesn't support
         int nbuttons = m_radiobox_fps->GetRowCount();
         for(int ibutt=0; ibutt<nbuttons; ibutt++) {
            m_radiobox_fps->Enable(ibutt,(ibutt<max)? true: false);
         }

         m_radiobox_fps->SetSelection(properties->Property_FPS());
      }
      else m_radiobox_fps->Enable(false);

      if(properties->HasProperty_AutoExposure()) {
         m_checkbox_autoexp->Enable(true);
         bool auto_on = properties->Property_AutoExposure();
         m_checkbox_autoexp->SetValue(auto_on);
      }
      else {
         m_checkbox_autoexp->Enable(false);
      }

      if(properties->HasProperty_AutoAll()) {
         m_checkbox_fullauto->Enable(true);
         bool is_auto = properties->Property_AutoAll();
         m_checkbox_fullauto->SetValue(is_auto);
         if(is_auto)m_checkbox_autoexp->Enable(false);
      }
      else m_checkbox_fullauto->Enable(false);

   }
}

PrimaryProps::~PrimaryProps()
{
	//(*Destroy(PrimaryProps)
	//*)

	m_timer.Stop();
}

void PrimaryProps::OnRadioboxFpsSelect(wxCommandEvent& event)
{
   if(wxWebcam::DeviceProperties* props = wxFactory::singleton()->webcam()->properties()) {
      int propValue = event.GetInt();
      props->SetProperty_FPS(propValue);
   }
}

void PrimaryProps::OnCheckboxFullautoClick(wxCommandEvent& event)
{
   if(wxWebcam::DeviceProperties* props = wxFactory::singleton()->webcam()->properties()) {
      bool propValue = event.IsChecked();
      props->SetProperty_AutoAll(propValue);
      if(propValue){
         m_checkbox_autoexp->SetValue(true);
         m_checkbox_autoexp->Enable(false);
      }
      else {
         m_checkbox_autoexp->Enable(true);
      }
   }
}

void PrimaryProps::OnCheckboxAutoExpClick(wxCommandEvent& event)
{
   if(wxWebcam::DeviceProperties* props = wxFactory::singleton()->webcam()->properties()) {
      bool propValue = event.IsChecked();
      props->SetProperty_AutoExposure(propValue);
   }
}

void PrimaryProps::OnTimer(wxTimerEvent& WXUNUSED(event))
{
   // update controls, unless we are busy editing
   if(!m_edit_busy)Init();
}

void PrimaryProps::OnGainSliderScrollChanged(wxScrollEvent& event)
{
   if(wxWebcam::DeviceProperties* props = wxFactory::singleton()->webcam()->properties()) {
      int propValue = event.GetPosition();
      props->SetProperty_Gain(propValue,true);
   }
}


void PrimaryProps::OnBrightnessSliderScrollChanged(wxScrollEvent& event)
{
   if(wxWebcam::DeviceProperties* props = wxFactory::singleton()->webcam()->properties()) {
      int propValue = event.GetPosition();
      props->SetProperty_Brightness(propValue,true);
   }
}

void PrimaryProps::OnShutterChoiceSelect(wxCommandEvent& event)
{
   if(wxWebcam::DeviceProperties* props = wxFactory::singleton()->webcam()->properties()) {
      int propValue = event.GetSelection();
      props->SetProperty_Exposure(propValue);
   }
}

void PrimaryProps::OnGammaSliderScrollChanged(wxScrollEvent& event)
{
   if(wxWebcam::DeviceProperties* props = wxFactory::singleton()->webcam()->properties()) {
      int propValue = event.GetPosition();
      props->SetProperty_Gamma(propValue,true);
   }
}


void PrimaryProps::OnSliderScrollThumbTrack(wxScrollEvent& WXUNUSED(event))
{
   m_edit_busy = true;
}

void PrimaryProps::OnSliderScrollThumbRelease(wxScrollEvent& WXUNUSED(event))
{
   m_edit_busy = false;
}
