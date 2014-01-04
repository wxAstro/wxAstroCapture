#ifndef SECONDARYPROPS_H
#define SECONDARYPROPS_H

#include <wx/wxprec.h>
#include <wx/timer.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

//(*Headers(SecondaryProps)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/radiobox.h>
#include <wx/checkbox.h>
#include <wx/slider.h>
#include <wx/panel.h>
//*)

class wxBookCtrlBase;

class SecondaryProps: public wxPanel
{
	public:

		SecondaryProps(wxBookCtrlBase* parent,wxWindowID id = -1);
		virtual ~SecondaryProps();

   	void Init();

		//(*Identifiers(SecondaryProps)
		static const long ID_STATICTEXT1;
		static const long ID_SLIDER1;
		static const long ID_STATICTEXT2;
		static const long ID_SLIDER2;
		static const long ID_STATICTEXT3;
		static const long ID_SLIDER3;
		static const long ID_STATICTEXT4;
		static const long ID_SLIDER4;
		static const long ID_RADIOBOX1;
		static const long ID_CHECKBOX1;
		static const long ID_CHECKBOX2;
		//*)

      static const long ID_TIMER1;
      void OnTimer(wxTimerEvent& event);

	protected:

		//(*Handlers(SecondaryProps)
		void OnRadioboxWbSelect(wxCommandEvent& event);
		void OnCheckboxColourClick(wxCommandEvent& event);
		void OnCheckboxFlipHorizClick(wxCommandEvent& event);
		void OnWbRedScrollChanged(wxScrollEvent& event);
		void OnWbBlueScrollChanged(wxScrollEvent& event);
		void OnSaturationScrollChanged(wxScrollEvent& event);
		void OnContrastScrollChanged(wxScrollEvent& event);
		void OnSliderScrollThumbTrack(wxScrollEvent& event);
		void OnSliderScrollThumbRelease(wxScrollEvent& event);
		//*)

		//(*Declarations(SecondaryProps)
		wxSlider* m_wb_red_slider;
		wxSlider* m_contrast_slider;
		wxRadioBox* m_radiobox_wb;
		wxCheckBox* m_checkbox_colour;
		wxSlider* m_saturation_slider;
		wxSlider* m_wb_blue_slider;
		wxCheckBox* m_checkbox_flip_horiz;
		//*)

		wxTimer         m_timer;
		wxBookCtrlBase* m_parent;
		bool            m_edit_busy; // true when editing a control

	private:

		DECLARE_EVENT_TABLE()
};

#endif
