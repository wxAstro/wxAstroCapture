#ifndef ARTEMISPROPS_H
#define ARTEMISPROPS_H

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

//(*Headers(ArtemisProps)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/slider.h>
#include <wx/panel.h>
#include <wx/button.h>
//*)
#include "wxthings/spinctld.h"

class ArtemisProps: public wxPanel
{
	public:

		ArtemisProps(wxWindow* parent,wxWindowID id = -1);
		virtual ~ArtemisProps();

		void Init();

		//(*Identifiers(ArtemisProps)
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL_BLACK;
		static const long ID_SLIDER_BLACK;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL_WHITE;
		static const long ID_SLIDER_WHITE;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL_GAMMA;
		static const long ID_SLIDER_GAMMA;
		static const long ID_STATICTEXT5;
		static const long ID_SPINCTRL_GLOG;
		static const long ID_SLIDER_GLOG;
		static const long ID_CHECKBOX1;
		static const long ID_BUTTON_RESET_GAMMA;
		//*)

	protected:

		//(*Handlers(ArtemisProps)
		void OnSpinCtrl_blackChange(wxSpinEvent& event);
		void OnSpinCtrl_whiteChange(wxSpinEvent& event);
		void OnSpinCtrl_whiteChange1(wxSpinEvent& event);
		void OnSpinCtrl_gammaChange(wxSpinEvent& event);
		void OnCheckBox_AutoStretchClick(wxCommandEvent& event);
		void OnSlider_blackCmdScrollChanged(wxScrollEvent& event);
		void OnSlider_whiteCmdScrollChanged(wxScrollEvent& event);
		void OnSlider_gammaCmdScrollChanged(wxScrollEvent& event);
		void OnSlider_glogCmdScrollChanged(wxScrollEvent& event);
		void OnSpinCtrl_glogChange(wxSpinEvent& event);
		void OnResetGamma_ButtonClick(wxCommandEvent& event);
		//*)

		//(*Declarations(ArtemisProps)
		wxSlider* Slider_gamma;
		wxButton* ResetGamma_Button;
		wxSpinCtrlDbl* SpinCtrl_glog;
		wxCheckBox* CheckBox_AutoStretch;
		wxSpinCtrlDbl* SpinCtrl_gamma;
		wxSpinCtrl* SpinCtrl_white;
		wxSlider* Slider_glog;
		wxSpinCtrl* SpinCtrl_black;
		wxSlider* Slider_black;
		wxSlider* Slider_white;
		//*)

		void UpdateCameraProperties(bool holdAuto);

      void SetBlackPoint(long black);
      void SetWhitePoint(long white);

		void SetGlog(double gamma);
		void SetGamma(double glog);

	private:

		DECLARE_EVENT_TABLE()
};

#endif
