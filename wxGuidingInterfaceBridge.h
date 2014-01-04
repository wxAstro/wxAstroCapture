#ifndef WXGUIDINGINTERFACEBRIDGE_H
#define WXGUIDINGINTERFACEBRIDGE_H

//(*Headers(wxGuidingInterfaceBridge)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/panel.h>
//*)

#include "wxNoteBookPageEx.h"

class wxGuidingInterfaceBridge :  public wxNoteBookPageEx
{
	public:

		wxGuidingInterfaceBridge(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~wxGuidingInterfaceBridge();

   	void Init();
      void InitBridgeParameters();

      /// On apply is called whenever the current page values should be applied
      virtual void OnApply();

		//(*Declarations(wxGuidingInterfaceBridge)
		wxPanel* Panel1;
		wxTextCtrl* TextCtrlHost;
		wxTextCtrl* TextCtrlPort;
		//*)

	protected:

		//(*Identifiers(wxGuidingInterfaceBridge)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL2;
		static const long ID_PANEL1;
		//*)

	private:

		//(*Handlers(wxGuidingInterfaceBridge)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
