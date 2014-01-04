#ifndef WXGUIDINGINTERFACEUSBPORT_H
#define WXGUIDINGINTERFACEUSBPORT_H

//(*Headers(wxGuidingInterfaceUsbPort)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/panel.h>
#include <wx/choice.h>
//*)

#include "wxIO/wxGuiderPort.h"
#include "wxNoteBookPageEx.h"

class wxGuidingInterfaceUsbPort:  public wxNoteBookPageEx
{
	public:

		wxGuidingInterfaceUsbPort(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~wxGuidingInterfaceUsbPort();


		void Init();

		void InitProtocolChoice();
		void InitPortParameters();

      /// On apply is called whenever the current page values should be applied
      virtual void OnApply();

		//(*Declarations(wxGuidingInterfaceUsbPort)
		wxChoice* ProtocolChoice;
		wxBoxSizer* BoxSizer1;
		//*)

	protected:

		//(*Identifiers(wxGuidingInterfaceUsbPort)
		static const long ID_STATICTEXT8;
		static const long ID_CHOICE3;
		//*)

	private:

      bool ApplyPort();

		//(*Handlers(wxGuidingInterfaceUsbPort)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
