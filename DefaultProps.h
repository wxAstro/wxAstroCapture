#ifndef DEFAULTPROPS_H
#define DEFAULTPROPS_H

#include <wx/wxprec.h>
#include <wx/timer.h>
#ifdef __BORLANDC__
    #pragma hdrstop
#endif

//(*Headers(DefaultProps)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/panel.h>
#include <wx/button.h>
//*)

class wxBookCtrlBase;

class DefaultProps: public wxPanel
{
	public:

		DefaultProps(wxBookCtrlBase* parent,wxWindowID id = -1);
		virtual ~DefaultProps();

		void Init();
		bool Enable(bool enable = true);


		//(*Identifiers(DefaultProps)
		static const long ID_BUTTON2;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		static const long ID_STATICTEXT3;
		//*)

	protected:

      static const long ID_TIMER1;
      void OnTimer(wxTimerEvent& event);


		//(*Handlers(DefaultProps)
		void OnUserSave(wxCommandEvent& event);
		void OnUserRestore(wxCommandEvent& event);
		void OnFactoryRestore(wxCommandEvent& event);
		//*)

		//(*Declarations(DefaultProps)
		wxStaticBoxSizer* StaticBoxSizer2;
		wxButton* m_factoryRestore_button;
		wxStaticText* StaticText2;
		wxButton* m_userRestore_button;
		wxStaticBoxSizer* StaticBoxSizer3;
		wxButton* m_userSave_button;
		//*)

		wxBookCtrlBase* m_parent;
 		wxTimer         m_timer;

	private:

		DECLARE_EVENT_TABLE()
};

#endif
