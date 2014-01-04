#ifndef SETTINGSCONTROLPORT_H
#define SETTINGSCONTROLPORT_H

#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

//(*Headers(SettingsControlPort)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/button.h>
//*)

#include "wxcustom/wxIoPortCtrl.h"
#include "wxcustom/wxPropertyPage.h"
#include "wxIO/wxPort.h"

class SettingsControlPort : public wxPropertyPage
{
	public:

		SettingsControlPort(wxWindow* parent,wxWindowID id = -1);
		virtual ~SettingsControlPort();

   	void Init();
   	bool OnApply();
   	void OnCancel();

		void CreatePortControls(bool enable, bool defaults);

		wxString PortNameControl();
		wxString WebcamPortName();

		bool IsLExposing();

		//(*Identifiers(SettingsControlPort)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT2;
		static const long ID_STATICTEXT7;
		static const long ID_BUTTON1;
		static const long ID_STATICTEXT6;
		static const long ID_CUSTOM3;
		static const long ID_STATICTEXT3;
		static const long ID_CUSTOM4;
		static const long ID_STATICTEXT4;
		static const long ID_CUSTOM5;
		static const long ID_CHECKBOX3;
		static const long ID_STATICTEXT5;
		static const long ID_CUSTOM6;
		static const long ID_CHECKBOX1;
		static const long ID_STATICTEXT9;
		static const long ID_CUSTOM7;
		static const long ID_CHECKBOX2;
		static const long ID_PANEL1;
		//*)

	protected:

		//(*Handlers(SettingsControlPort)
		void OnPortChoiceSelect(wxCommandEvent& event);
		void OnDefaultsButtonClick(wxCommandEvent& event);
		//*)

		//(*Declarations(SettingsControlPort)
		wxIoPortCtrl* LX_init;
		wxIoPortCtrl* LX_frame;
		wxIoPortCtrl* LX_shutter;
		wxIoPortCtrl* LX_mask;
		wxCheckBox* UseInvertedLogic;
		wxPanel* Panel1;
		wxButton* DefaultsButton;
		wxCheckBox* UseShutterCheckBox;
		wxStaticText* StaticText7;
		wxIoPortCtrl* LX_amp;
		wxCheckBox* UseAmpCheckBox;
		wxFlexGridSizer* FlexGridSizer1;
		wxChoice* PortChoice;
		//*)

	private:

		DECLARE_EVENT_TABLE()

};

#endif
