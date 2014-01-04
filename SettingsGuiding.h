#ifndef SETTINGSGUIDING_H
#define SETTINGSGUIDING_H



//(*Headers(SettingsGuiding)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>
#include <wx/button.h>
//*)
#include "wxthings/spinctld.h"
#include "wxcustom/wxPropertyPage.h"

class SettingsGuiding: public wxPropertyPage
{
	public:
		SettingsGuiding(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = wxT("panel"));
		virtual ~SettingsGuiding();

		void Init();
		bool OnApply();

		bool SetAngleValue(const wxString& cameraAngle);
		bool SetArcsecValue(const wxString& arcsec);

		//(*Identifiers(SettingsGuiding)
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT5;
		static const long ID_SPINCTRL3;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL4;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT8;
		static const long ID_SPINCTRL6;
		static const long ID_BUTTON1;
		//*)

	protected:

		//(*Handlers(SettingsGuiding)
		void OnAutoCalibrateButtonClick(wxCommandEvent& event);
		//*)

		//(*Declarations(SettingsGuiding)
		wxSpinCtrlDbl* m_rotation_SpinCtrl;
		wxSpinCtrl* m_interval;
		wxStaticText* StaticText1;
		wxSpinCtrl* m_box_size;
		wxSpinCtrlDbl* m_arcsec_SpinCtrl;
		wxButton* AutoCalibrateButton;
		wxSpinCtrl* m_bckg_cutoff;
		//*)

	private:

		DECLARE_EVENT_TABLE()
};

#endif
