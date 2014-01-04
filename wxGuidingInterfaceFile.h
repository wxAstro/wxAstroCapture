#ifndef WXGUIDINGINTERFACEFILE_H
#define WXGUIDINGINTERFACEFILE_H

//(*Headers(wxGuidingInterfaceFile)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/panel.h>
#include <wx/button.h>
//*)

#include "wxNoteBookPageEx.h"

class wxGuidingInterfaceFile :  public wxNoteBookPageEx
{
	public:

		wxGuidingInterfaceFile(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~wxGuidingInterfaceFile();

		void Init();

      /// On apply is called whenever the current page values should be applied
      virtual void OnApply();

		//(*Declarations(wxGuidingInterfaceFile)
		wxTextCtrl* m_target_dir;
		//*)

	protected:

		//(*Identifiers(wxGuidingInterfaceFile)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_BUTTON1;
		//*)

	private:

		//(*Handlers(wxGuidingInterfaceFile)
		void OnDirButtonClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
