#ifndef SETTINGSGUIDINGINTERFACEBRIDGE_H
#define SETTINGSGUIDINGINTERFACEBRIDGE_H

//(*Headers(SettingsGuidingInterfaceBridge)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/panel.h>
//*)

class SettingsGuidingInterfaceBridge: public wxPanel
{
	public:

		SettingsGuidingInterfaceBridge(wxWindow* parent,wxWindowID id = -1);
		virtual ~SettingsGuidingInterfaceBridge();

      void Init();
		void SaveToConfig();

		//(*Identifiers(SettingsGuidingInterfaceBridge)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL2;
		//*)

	protected:

		//(*Handlers(SettingsGuidingInterfaceBridge)
		//*)

		//(*Declarations(SettingsGuidingInterfaceBridge)
		wxTextCtrl* TextCtrlHost;
		wxTextCtrl* TextCtrlPort;
		//*)

	private:

		DECLARE_EVENT_TABLE()
};

#endif
