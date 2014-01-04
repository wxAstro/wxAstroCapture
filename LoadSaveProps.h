#ifndef LOADSAVEPROPS_H
#define LOADSAVEPROPS_H

#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

//(*Headers(LoadSaveProps)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/listbox.h>
#include <wx/panel.h>
#include <wx/button.h>
//*)

class LoadSaveProps: public wxPanel
{
	public:

		LoadSaveProps(wxWindow* parent,wxWindowID id = -1);
		virtual ~LoadSaveProps();
		void Init();

		//(*Identifiers(LoadSaveProps)
		static const long ID_STATICTEXT2;
		static const long ID_LISTBOX1;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL1;
		static const long ID_BUTTON5;
		static const long ID_BUTTON2;
		static const long ID_BUTTON4;
		static const long ID_BUTTON1;
		static const long ID_BUTTON3;
		static const long ID_STATICTEXT1;
		//*)

	protected:

		//(*Handlers(LoadSaveProps)
		void OnAddSlotClick(wxCommandEvent& event);
		void OnDeleteSlotClick(wxCommandEvent& event);
		void OnLoadToCameraClick(wxCommandEvent& event);
		void OnSaveFromCameraClick(wxCommandEvent& event);
		void OnSetAsDefaultClick(wxCommandEvent& event);
		//*)

		void OnIdle(wxIdleEvent& event);

		bool addSlot(const wxString& slotname);
		wxString cameraPath(const wxString& slotname);
		void FillSlots();

		//(*Declarations(LoadSaveProps)
		wxButton* m_AddSlotBtn;
		wxButton* m_LoadToCameraBtn;
		wxStaticText* m_slotLabel;
		wxTextCtrl* m_slotname;
		wxButton* m_SaveFromCameraBtn;
		wxStaticText* StaticText1;
		wxListBox* m_listbox;
		wxButton* m_SetAsDefaultButton;
		//*)

	private:

		DECLARE_EVENT_TABLE()
};

#endif
