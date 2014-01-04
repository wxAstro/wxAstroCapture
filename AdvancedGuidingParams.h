#ifndef ADVANCEDGUIDINGPARAMS_H
#define ADVANCEDGUIDINGPARAMS_H

//(*Headers(AdvancedGuidingParams)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

#include "wxthings/spinctld.h"

class AdvancedGuidingParams: public wxDialog
{
	public:

		AdvancedGuidingParams(wxWindow* parent,wxWindowID id = -1);
		virtual ~AdvancedGuidingParams();
		void InitPosition();
		void Init();

		static AdvancedGuidingParams* singleton(wxWindow* parent);
		void getValuesFromGuider();

		//(*Identifiers(AdvancedGuidingParams)
		static const long ID_STATICTEXT5;
		static const long ID_STATICTEXT4;
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL3;
		static const long ID_SPINCTRL4;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL5;
		static const long ID_SPINCTRL6;
		static const long ID_BUTTON1;
		//*)

	protected:

		//(*Handlers(AdvancedGuidingParams)
		void OnClose(wxCloseEvent& event);
		void OnResetDefaultsButtonClick(wxCommandEvent& event);
		void OnRA_dz_SpinCtrlChange(wxSpinEvent& event);
		void OnRA_k_SpinCtrlChange(wxSpinEvent& event);
		void OnRA_q_SpinCtrlChange(wxSpinEvent& event);
		void OnDEC_dz_SpinCtrlChange(wxSpinEvent& event);
		void OnDEC_k_SpinCtrlChange(wxSpinEvent& event);
		void OnDEC_q_SpinCtrlChange(wxSpinEvent& event);
		//*)

		//(*Declarations(AdvancedGuidingParams)
		wxSpinCtrlDbl* m_DEC_dz_SpinCtrl;
		wxSpinCtrlDbl* m_RA_q_SpinCtrl;
		wxSpinCtrlDbl* m_RA_k_SpinCtrl;
		wxSpinCtrlDbl* m_DEC_q_SpinCtrl;
		wxSpinCtrlDbl* m_RA_dz_SpinCtrl;
		wxSpinCtrlDbl* m_DEC_k_SpinCtrl;
		//*)

	private:

		DECLARE_EVENT_TABLE()

   private:
      static AdvancedGuidingParams* m_self;
};

#endif
