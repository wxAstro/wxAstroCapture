#include "SecondaryProps.h"

//(*InternalHeaders(SecondaryProps)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "wx/bookctrl.h"
#include "wxWebcam/wxWebcam.h"
#include "wxFactory.h"

//(*IdInit(SecondaryProps)
const long SecondaryProps::ID_STATICTEXT1 = wxNewId();
const long SecondaryProps::ID_SLIDER1 = wxNewId();
const long SecondaryProps::ID_STATICTEXT2 = wxNewId();
const long SecondaryProps::ID_SLIDER2 = wxNewId();
const long SecondaryProps::ID_STATICTEXT3 = wxNewId();
const long SecondaryProps::ID_SLIDER3 = wxNewId();
const long SecondaryProps::ID_STATICTEXT4 = wxNewId();
const long SecondaryProps::ID_SLIDER4 = wxNewId();
const long SecondaryProps::ID_RADIOBOX1 = wxNewId();
const long SecondaryProps::ID_CHECKBOX1 = wxNewId();
const long SecondaryProps::ID_CHECKBOX2 = wxNewId();
//*)

const long SecondaryProps::ID_TIMER1= wxNewId();

BEGIN_EVENT_TABLE(SecondaryProps,wxPanel)
	//(*EventTable(SecondaryProps)
	//*)
   EVT_TIMER(SecondaryProps::ID_TIMER1,SecondaryProps::OnTimer)
END_EVENT_TABLE()

SecondaryProps::SecondaryProps(wxBookCtrlBase* parent,wxWindowID id)
: m_timer(this,ID_TIMER1)
,m_parent(parent)
,m_edit_busy(false)
{
	//(*Initialize(SecondaryProps)
	wxStaticText* StaticText2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer2;
	wxStaticText* StaticText1;
	wxStaticText* StaticText3;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;
	wxStaticText* StaticText4;
	
	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->Add(5,5,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(5,5,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("WB-R"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	m_wb_red_slider = new wxSlider(this, ID_SLIDER1, 50, 0, 100, wxDefaultPosition, wxSize(120,40), wxSL_LABELS, wxDefaultValidator, _T("ID_SLIDER1"));
	FlexGridSizer2->Add(m_wb_red_slider, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("WB-B"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	m_wb_blue_slider = new wxSlider(this, ID_SLIDER2, 50, 0, 100, wxDefaultPosition, wxSize(-1,40), wxSL_LABELS, wxDefaultValidator, _T("ID_SLIDER2"));
	FlexGridSizer2->Add(m_wb_blue_slider, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer2->Add(-1,5,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(-1,5,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Contr."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	m_contrast_slider = new wxSlider(this, ID_SLIDER3, 0, 0, 100, wxDefaultPosition, wxSize(120,40), wxSL_LABELS, wxDefaultValidator, _T("ID_SLIDER3"));
	FlexGridSizer2->Add(m_contrast_slider, 1, wxALL|wxEXPAND|wxSHAPED|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Satur."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer2->Add(StaticText4, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	m_saturation_slider = new wxSlider(this, ID_SLIDER4, 50, 0, 100, wxDefaultPosition, wxSize(120,40), wxSL_LABELS, wxDefaultValidator, _T("ID_SLIDER4"));
	FlexGridSizer2->Add(m_saturation_slider, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 2);
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	wxString __wxRadioBoxChoices_1[5] = 
	{
		_("Indoor"),
		_("Outdoor"),
		_("FL"),
		_("Freeze"),
		_("Auto")
	};
	m_radiobox_wb = new wxRadioBox(this, ID_RADIOBOX1, _("White bal."), wxDefaultPosition, wxDefaultSize, 5, __wxRadioBoxChoices_1, 1, wxRA_SPECIFY_COLS, wxDefaultValidator, _T("ID_RADIOBOX1"));
	FlexGridSizer3->Add(m_radiobox_wb, 1, wxBOTTOM|wxLEFT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxVERTICAL, this, _("Modes"));
	m_checkbox_colour = new wxCheckBox(this, ID_CHECKBOX1, _("Colour"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	m_checkbox_colour->SetValue(false);
	StaticBoxSizer1->Add(m_checkbox_colour, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_checkbox_flip_horiz = new wxCheckBox(this, ID_CHECKBOX2, _("Flip Horiz."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	m_checkbox_flip_horiz->SetValue(false);
	StaticBoxSizer1->Add(m_checkbox_flip_horiz, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(StaticBoxSizer1, 1, wxTOP|wxLEFT|wxEXPAND|wxALIGN_RIGHT|wxALIGN_BOTTOM, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	
	Connect(ID_SLIDER1,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&SecondaryProps::OnSliderScrollThumbTrack);
	Connect(ID_SLIDER1,wxEVT_SCROLL_THUMBRELEASE,(wxObjectEventFunction)&SecondaryProps::OnSliderScrollThumbRelease);
	Connect(ID_SLIDER1,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&SecondaryProps::OnWbRedScrollChanged);
	Connect(ID_SLIDER2,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&SecondaryProps::OnSliderScrollThumbTrack);
	Connect(ID_SLIDER2,wxEVT_SCROLL_THUMBRELEASE,(wxObjectEventFunction)&SecondaryProps::OnSliderScrollThumbRelease);
	Connect(ID_SLIDER2,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&SecondaryProps::OnWbBlueScrollChanged);
	Connect(ID_SLIDER3,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&SecondaryProps::OnSliderScrollThumbTrack);
	Connect(ID_SLIDER3,wxEVT_SCROLL_THUMBRELEASE,(wxObjectEventFunction)&SecondaryProps::OnSliderScrollThumbRelease);
	Connect(ID_SLIDER3,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&SecondaryProps::OnContrastScrollChanged);
	Connect(ID_SLIDER4,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&SecondaryProps::OnSliderScrollThumbTrack);
	Connect(ID_SLIDER4,wxEVT_SCROLL_THUMBRELEASE,(wxObjectEventFunction)&SecondaryProps::OnSliderScrollThumbRelease);
	Connect(ID_SLIDER4,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&SecondaryProps::OnSaturationScrollChanged);
	Connect(ID_RADIOBOX1,wxEVT_COMMAND_RADIOBOX_SELECTED,(wxObjectEventFunction)&SecondaryProps::OnRadioboxWbSelect);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&SecondaryProps::OnCheckboxColourClick);
	Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&SecondaryProps::OnCheckboxFlipHorizClick);
	//*)

   Init();
   // re-initialise every XXXX msecond
   m_timer.Start(1000);

}

SecondaryProps::~SecondaryProps()
{
	//(*Destroy(SecondaryProps)
	//*)

	m_timer.Stop();
}

void SecondaryProps::OnTimer(wxTimerEvent& WXUNUSED(event))
{
   // update controls, unless we are busy editing
   if(!m_edit_busy)Init();
}

void SecondaryProps::Init()
{
   // if this page is not the current one, do nothing
   if(m_parent->GetCurrentPage() != this)return;


   if(wxWebcam* camera = wxFactory::singleton()->webcam()) {

      // if the camera is busy capturing, do nothing
      if(camera->IsCapturing())return;

      wxWebcam::DeviceProperties* properties = camera->properties();

      m_wb_red_slider->Enable(false);
      m_wb_blue_slider->Enable(false);

      if(properties->HasProperty_WhiteBalanceMode()) {
         m_radiobox_wb->Enable(true);

         long wb_mode = properties->Property_WhiteBalanceMode();
         m_radiobox_wb->SetSelection(wb_mode);

         if(EWB_Freeze == wb_mode) {
            if(properties->HasProperty_WhiteBalRed()) {
               m_wb_red_slider->Enable(true);
               m_wb_red_slider->SetValue(properties->Property_WhiteBalRed(true));
            }
            if(properties->HasProperty_WhiteBalBlue()) {
               m_wb_blue_slider->Enable(true);
               m_wb_blue_slider->SetValue(properties->Property_WhiteBalBlue(true));
            }
         }
      }
      else m_radiobox_wb->Enable(false);

      if(properties->HasProperty_Contrast()) {
         m_contrast_slider->Enable(true);
         m_contrast_slider->SetValue(properties->Property_Contrast(true));
      }
      else m_contrast_slider->Enable(false);

      if(properties->HasProperty_Saturation()) {
         m_saturation_slider->Enable(true);
         m_saturation_slider->SetValue(properties->Property_Saturation(true));//
      }
      else m_saturation_slider->Enable(false);

      if(properties->HasProperty_ColorEnable()) {
         m_checkbox_colour->Enable(true);
         m_checkbox_colour->SetValue(properties->Property_ColorEnable());
      }
      else m_checkbox_colour->Enable(false);

      if(properties->HasProperty_FlipHorizontal()) {
         m_checkbox_flip_horiz->Enable(true);
         m_checkbox_flip_horiz->SetValue(properties->Property_FlipHorizontal());
      }
      else m_checkbox_flip_horiz->Enable(false);
   }
}

void SecondaryProps::OnRadioboxWbSelect(wxCommandEvent& event)
{
   if(wxWebcam::DeviceProperties* props = wxFactory::singleton()->webcam()->properties()) {
      int propValue = event.GetInt();
      props->SetProperty_WhiteBalanceMode(propValue);
   }
}

void SecondaryProps::OnCheckboxColourClick(wxCommandEvent& event)
{
   if(wxWebcam::DeviceProperties* props = wxFactory::singleton()->webcam()->properties()) {
      bool propValue = event.IsChecked();
      props->SetProperty_ColorEnable(propValue);
   }
}

void SecondaryProps::OnCheckboxFlipHorizClick(wxCommandEvent& event)
{
   if(wxWebcam::DeviceProperties* props = wxFactory::singleton()->webcam()->properties()) {
      bool propValue = event.IsChecked();
      props->SetProperty_FlipHorizontal(propValue);
   }
}


void SecondaryProps::OnWbRedScrollChanged(wxScrollEvent& event)
{
   if(wxWebcam::DeviceProperties* props = wxFactory::singleton()->webcam()->properties()) {
      int propValue = event.GetPosition();
      props->SetProperty_WhiteBalRed(propValue,true);
   }
}

void SecondaryProps::OnWbBlueScrollChanged(wxScrollEvent& event)
{
   if(wxWebcam::DeviceProperties* props = wxFactory::singleton()->webcam()->properties()) {
      int propValue = event.GetPosition();
      props->SetProperty_WhiteBalBlue(propValue,true);
   }
}

void SecondaryProps::OnSaturationScrollChanged(wxScrollEvent& event)
{
   if(wxWebcam::DeviceProperties* props = wxFactory::singleton()->webcam()->properties()) {
      int propValue = event.GetPosition();
      props->SetProperty_Saturation(propValue,true);
   }
}

void SecondaryProps::OnContrastScrollChanged(wxScrollEvent& event)
{
   if(wxWebcam::DeviceProperties* props = wxFactory::singleton()->webcam()->properties()) {
      int propValue = event.GetPosition();
      props->SetProperty_Contrast(propValue,true);
   }
}

void SecondaryProps::OnSliderScrollThumbTrack(wxScrollEvent& WXUNUSED(event))
{
   m_edit_busy = true;
}

void SecondaryProps::OnSliderScrollThumbRelease(wxScrollEvent& WXUNUSED(event))
{
   m_edit_busy = false;
}
