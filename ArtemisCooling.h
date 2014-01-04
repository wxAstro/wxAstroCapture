#ifndef ARTEMISCOOLING_H
#define ARTEMISCOOLING_H

//(*Headers(ArtemisCooling)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>
#include <wx/button.h>
//*)

class wxSpinCtrlDbl;

class ArtemisCooling: public wxPanel
{
	public:

		ArtemisCooling(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ArtemisCooling();

		void Init();

		//(*Declarations(ArtemisCooling)
		wxSpinCtrlDbl* m_target_spin;
		wxStaticText* m_cur_temp;
		wxStaticText* m_cooling_status;
		//*)

		void OnIdle(wxIdleEvent& event);

	protected:

		//(*Identifiers(ArtemisCooling)
		static const long ID_STATICTEXT1;
		static const long ID_STATICTEXT2;
		static const long ID_STATICTEXT4;
		static const long ID_STATICTEXT5;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(ArtemisCooling)
		void OnStartWarmupClick(wxCommandEvent& event);
		void OnSetTargetClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
