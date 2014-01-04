#ifndef SETTINGSGUIDEPORTSERIAL_H
#define SETTINGSGUIDEPORTSERIAL_H

//(*Headers(SettingsGuidePortSerial)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/panel.h>
#include <wx/choice.h>
//*)

#include "wxIO/wxGuiderPort.h"

class SettingsGuidePortSerial: public wxPanel
{
	public:

		SettingsGuidePortSerial(wxWindow* parent,wxWindowID id = -1);
		virtual ~SettingsGuidePortSerial();

		void Init();

		wxGuiderPort::EPGBaudRate  baudrate();
		wxGuiderPort::EPGHandshake handshake();

		//(*Identifiers(SettingsGuidePortSerial)
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE2;
		//*)

	protected:

		//(*Handlers(SettingsGuidePortSerial)
		//*)

		//(*Declarations(SettingsGuidePortSerial)
		wxChoice* BaudRateChoice;
		wxChoice* HandShakeChoice;
		wxBoxSizer* BoxSizer1;
		//*)

	private:

		DECLARE_EVENT_TABLE()
};

#endif
