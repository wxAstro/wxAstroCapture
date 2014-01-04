#ifndef SETTINGSGUIDINGINTERFACE_H
#define SETTINGSGUIDINGINTERFACE_H

//(*Headers(SettingsGuidingInterface)
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/choice.h>
//*)

#include "wxcustom/wxPropertyPage.h"

class	SettingsGuidingInterfacePort;
class	SettingsGuidingInterfaceFile;
class SettingsGuidingInterfaceBridge;


class SettingsGuidingInterface: public wxPropertyPage
{
	public:

		SettingsGuidingInterface(wxWindow* parent,wxWindowID id = -1);
		virtual ~SettingsGuidingInterface();

		void Init();
		bool OnApply();

		void ShowParams(int type);

		//(*Identifiers(SettingsGuidingInterface)
		static const long ID_CHOICE1;
		//*)

	protected:

		//(*Handlers(SettingsGuidingInterface)
		void OnInterfaceChoiceSelect(wxCommandEvent& event);
		//*)

		//(*Declarations(SettingsGuidingInterface)
		wxBoxSizer* BoxSizerParameters;
		wxBoxSizer* BoxSizer1;
		wxChoice* InterfaceChoice;
		//*)

	private:

   	SettingsGuidingInterfacePort* m_portParams;
   	SettingsGuidingInterfaceFile* m_fileParams;
   	SettingsGuidingInterfaceBridge* m_bridgeParams;

		DECLARE_EVENT_TABLE()
};

#endif
