#ifndef PRIMARYPROPS_H
#define PRIMARYPROPS_H

#include <wx/wxprec.h>
#include <wx/timer.h>
#ifdef __BORLANDC__
    #pragma hdrstop
#endif

//(*Headers(PrimaryProps)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/radiobox.h>
#include <wx/checkbox.h>
#include <wx/slider.h>
#include <wx/panel.h>
#include <wx/choice.h>
//*)

class wxBookCtrlBase;

class PrimaryProps: public wxPanel
{
	public:

		PrimaryProps(wxBookCtrlBase* parent,wxWindowID id = -1);
		virtual ~PrimaryProps();

		void Init();

		//(*Identifiers(PrimaryProps)
		static const long ID_STATICTEXT1;
		static const long ID_SLIDER1;
		static const long ID_STATICTEXT2;
		static const long ID_SLIDER2;
		static const long ID_STATICTEXT3;
		static const long ID_SLIDER3;
		static const long ID_STATICTEXT6;
		static const long ID_CHOICE1;
		static const long ID_CHECKBOX2;
		static const long ID_CHECKBOX1;
		static const long ID_RADIOBOX2;
		//*)

      static const long ID_TIMER1;
      void OnTimer(wxTimerEvent& event);

    //  void OnIdle(wxIdleEvent& event);
	protected:

		//(*Handlers(PrimaryProps)
		void OnShutterChoiceSelect(wxCommandEvent& event);
		void OnRadioboxFpsSelect(wxCommandEvent& event);
		void OnCheckboxFullautoClick(wxCommandEvent& event);
		void OnCheckboxAutoExpClick(wxCommandEvent& event);
		void OnGainSliderScrollChanged(wxScrollEvent& event);
		void OnBrightnessSliderScrollChanged(wxScrollEvent& event);
		void OnGammaSliderScrollChanged(wxScrollEvent& event);
		void OnSliderScrollThumbTrack(wxScrollEvent& event);
		void OnSliderScrollThumbRelease(wxScrollEvent& event);
		//*)


		//(*Declarations(PrimaryProps)
		wxChoice* m_shutter_choice;
		wxSlider* m_gain_slider;
		wxRadioBox* m_radiobox_fps;
		wxSlider* m_brightness_slider;
		wxCheckBox* m_checkbox_fullauto;
		wxBoxSizer* BoxSizer1;
		wxSlider* m_gamma_slider;
		wxCheckBox* m_checkbox_autoexp;
		//*)

		wxTimer         m_timer;
		wxBookCtrlBase* m_parent;
		bool            m_edit_busy; // true when editing a control

	private:

		DECLARE_EVENT_TABLE()
};

#endif
