#ifndef SETTINGSVIDEOCAPTURE_H
#define SETTINGSVIDEOCAPTURE_H

#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

//(*Headers(SettingsVideoCapture)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/statline.h>
#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/button.h>
//*)
#include "wxcustom/wxPropertyPage.h"

class SettingsVideoCapture : public wxPropertyPage
{
	public:

		SettingsVideoCapture(wxWindow* parent,wxWindowID id = -1);
		virtual ~SettingsVideoCapture();

		void Init();
		bool OnApply();

		//(*Identifiers(SettingsVideoCapture)
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_BUTTON1;
		static const long ID_STATICLINE1;
		static const long ID_STATICLINE2;
		static const long ID_STATICLINE3;
		static const long ID_STATICTEXT2;
		static const long ID_STATICTEXT5;
		static const long ID_STATICTEXT7;
		static const long ID_CHOICE2;
		static const long ID_CHOICE3;
		static const long ID_CHOICE5;
		static const long ID_STATICTEXT3;
		static const long ID_STATICTEXT6;
		static const long ID_STATICTEXT8;
		static const long ID_CHOICE1;
		static const long ID_CHOICE4;
		static const long ID_CHOICE6;
		static const long ID_PANEL1;
		//*)

	protected:

		//(*Handlers(SettingsVideoCapture)
		void OnDirButtonClick(wxCommandEvent& event);
		//*)

		//(*Declarations(SettingsVideoCapture)
		wxChoice* m_perf_level;
		wxTextCtrl* m_folderCtrl;
		wxStaticText* StaticText8;
		wxChoice* m_captureChoice;
		wxChoice* m_frame_timestamp;
		wxStaticText* StaticText7;
		wxTextCtrl* m_fileNamePrefixCtrl;
		wxChoice* m_containerChoice;
		wxChoice* m_frame_font;
		wxChoice* m_fps_font;
		//*)

	private:

		DECLARE_EVENT_TABLE()
};

#endif
