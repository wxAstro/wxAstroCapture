#ifndef SETTINGSGUIDINGINTERFACEPORT_H
#define SETTINGSGUIDINGINTERFACEPORT_H

//(*Headers(SettingsGuidingInterfacePort)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/panel.h>
//*)

class SettingsGuidingInterfacePort: public wxPanel
{
	public:

		SettingsGuidingInterfacePort(wxWindow* parent,wxWindowID id = -1);
		virtual ~SettingsGuidingInterfacePort();

		void SaveToConfig();

		//(*Identifiers(SettingsGuidingInterfacePort)
		static const long ID_STATICTEXT1;
		//*)

	protected:

		//(*Handlers(SettingsGuidingInterfacePort)
		//*)

		//(*Declarations(SettingsGuidingInterfacePort)
		wxStaticText* StaticText1;
		wxBoxSizer* BoxSizer1;
		//*)

	private:

		DECLARE_EVENT_TABLE()
};

#endif
