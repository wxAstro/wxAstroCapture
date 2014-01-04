#ifndef SETTINGSGUIDEPORT_H
#define SETTINGSGUIDEPORT_H

//(*Headers(SettingsGuidePort)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/panel.h>
#include <wx/choice.h>
//*)

#include "wxcustom/wxIoPortCtrl.h"
#include "wxcustom/wxPropertyPage.h"
#include "wxIO/wxPort.h"

class SettingsGuidePortSerial;
class SettingsGuidePortParallel;
class wxPortDescr;

class SettingsGuidePort : public wxPropertyPage
{
	public:

		SettingsGuidePort(wxWindow* parent,wxWindowID id = -1);
		virtual ~SettingsGuidePort();

      void Init();
		bool OnApply();
    	void OnCancel();

		void ShowPortControls(const wxString& protocolName, bool show_serial,bool show_parallel,bool show_gpusb);//BM:20081228
		void ShowProtocols(const wxString& protocolName,bool show_serial,bool show_parallel,bool show_gpusb);//BM:20081228

      wxString PortNameGuider();
      wxString ProtocolNameGuider();

      bool ApplyPortProtocol();

		//(*Identifiers(SettingsGuidePort)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT8;
		static const long ID_CHOICE2;
		//*)

	protected:

		//(*Handlers(SettingsGuidePort)
		void OnPortChoiceSelect(wxCommandEvent& event);
		void OnProtocolChoiceSelect(wxCommandEvent& event);
		//*)

		//(*Declarations(SettingsGuidePort)
		wxBoxSizer* BoxSizerPortPanel;
		wxChoice* ProtocolChoice;
		wxChoice* PortChoice;
		//*)

		SettingsGuidePortSerial*   m_serial;
		SettingsGuidePortParallel* m_parallel;

	private:

		DECLARE_EVENT_TABLE()
};

#endif
