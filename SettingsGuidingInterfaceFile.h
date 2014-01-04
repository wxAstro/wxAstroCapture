#ifndef SETTINGSGUIDINGINTERFACEFILE_H
#define SETTINGSGUIDINGINTERFACEFILE_H

//(*Headers(SettingsGuidingInterfaceFile)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/panel.h>
#include <wx/button.h>
//*)

class SettingsGuidingInterfaceFile: public wxPanel
{
	public:

		SettingsGuidingInterfaceFile(wxWindow* parent,wxWindowID id = -1);
		virtual ~SettingsGuidingInterfaceFile();

		void Init();

		void SaveToConfig();

		//(*Identifiers(SettingsGuidingInterfaceFile)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_BUTTON1;
		//*)

	protected:

		//(*Handlers(SettingsGuidingInterfaceFile)
		void OnDirButtonClick(wxCommandEvent& event);
		//*)

		//(*Declarations(SettingsGuidingInterfaceFile)
		wxTextCtrl* m_target_dir;
		wxBoxSizer* BoxSizer1;
		//*)

	private:

		DECLARE_EVENT_TABLE()
};

#endif
