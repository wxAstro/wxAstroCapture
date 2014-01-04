#ifndef WXGUIDINGINTERFACESERIALPORT_H
#define WXGUIDINGINTERFACESERIALPORT_H

//(*Headers(wxGuidingInterfaceSerialPort)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/panel.h>
#include <wx/choice.h>
//*)

#include "wxIO/wxGuiderPort.h"
#include "wxNoteBookPageEx.h"

class wxGuidingInterfaceSerialPort :  public wxNoteBookPageEx
{
	public:

		wxGuidingInterfaceSerialPort(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~wxGuidingInterfaceSerialPort();

		void Init();

		void InitProtocolChoice();
		void InitPortParameters();

      /// On apply is called whenever the current page values should be applied
      virtual void OnApply();

		//(*Declarations(wxGuidingInterfaceSerialPort)
		wxChoice* BaudRateChoice;
		wxChoice* HandShakeChoice;
		wxChoice* ProtocolChoice;
		wxBoxSizer* BoxSizer1;
		//*)

	protected:

		//(*Identifiers(wxGuidingInterfaceSerialPort)
		static const long ID_STATICTEXT8;
		static const long ID_CHOICE3;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE2;
		//*)

	private:

      bool ApplyPort();
      wxGuiderPort::EPGBaudRate  baudrate();
      wxGuiderPort::EPGHandshake handshake();

		//(*Handlers(wxGuidingInterfaceSerialPort)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
