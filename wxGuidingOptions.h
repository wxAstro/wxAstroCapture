#ifndef WXGUIDINGOPTIONS_H
#define WXGUIDINGOPTIONS_H

//(*Headers(wxGuidingOptions)
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/choicebk.h>
//*)

#include "SettingsGuiding.h"

class wxGuidingOptions: public wxDialog
{
	public:

      static wxGuidingOptions* singleton();

		wxGuidingOptions(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~wxGuidingOptions();

		// First time initialisation
		void Init();

		// InitInterfaces should be called each time the available interfaces (ports) must be re-assessed
		// InitInterfaces is called from Init()
		void InitInterfaces();

		//(*Declarations(wxGuidingOptions)
		wxNotebook* Notebook1;
		wxChoicebook* m_interfaceChoiceBook;
		SettingsGuiding* m_calibration;
		wxPanel* m_guiding_interface;
		//*)

	protected:

		//(*Identifiers(wxGuidingOptions)
		static const long ID_PANEL1;
		static const long ID_CHOICEBOOK1;
		static const long ID_PANEL2;
		static const long ID_NOTEBOOK1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

   private:
      static wxGuidingOptions* m_self;

		//(*Handlers(wxGuidingOptions)
		void OnApplyButtonClick(wxCommandEvent& event);
		void OnCloseButtonClick(wxCommandEvent& event);
		void OnIinterfaceChoiceBookPageChanged(wxChoicebookEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
