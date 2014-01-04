#ifndef SETTINGSGUIDINGCALIBRATEDLG_H
#define SETTINGSGUIDINGCALIBRATEDLG_H

//(*Headers(SettingsGuidingCalibrateDlg)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/statline.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

#include "wxSubFrame.h"

#include <wx/timer.h>
class SettingsGuiding;

class SettingsGuidingCalibrateDlg: public wxDialog
{
	public:

		SettingsGuidingCalibrateDlg(SettingsGuiding* parent, wxWindowID id = -1);
		virtual ~SettingsGuidingCalibrateDlg();

		//(*Identifiers(SettingsGuidingCalibrateDlg)
		static const long ID_STATICTEXT1;
		static const long ID_STATICLINE1;
		static const long ID_STATICTEXT2;
		static const long ID_BUTTON1;
		static const long ID_STATICLINE2;
		static const long ID_STATICTEXT4;
		static const long ID_STATICLINE3;
		static const long ID_STATICTEXT5;
		static const long ID_STATICLINE4;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL3;
		static const long ID_STATICTEXT6;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT7;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICLINE5;
		static const long ID_OK_BUTTON;
		static const long ID_CANCEL_BUTTON;
		//*)

	protected:

	   // Manually defined handlers & other functions
   	void OnvideoPanelLeftDown(wxMouseEvent& event);
   	void DisconnectMouseEvent();
   	void OnGuideTimer(wxTimerEvent& event);

   	bool ComputeUpdateCalibration();

		//(*Handlers(SettingsGuidingCalibrateDlg)
		void OnClose(wxCloseEvent& event);
		void OnPickStarButtonClick(wxCommandEvent& event);
		void OnOKbuttonClick(wxCommandEvent& event);
		void OnCancelButtonClick(wxCommandEvent& event);
		//*)

		//(*Declarations(SettingsGuidingCalibrateDlg)
		wxTextCtrl* m_elapsed_time;
		wxBoxSizer* BoxSizer8;
		wxTextCtrl* m_camera_angle;
		wxStaticText* StaticText3;
		wxTextCtrl* m_imageScale;
		//*)

	private:

		DECLARE_EVENT_TABLE()

   private:
      wxSubFrame  m_guide_subframe;      // the object that handles guiding calculations
      wxTimer     m_guide_timer;         // The guide timer
      wxStar      m_offset_star;         // position of star relative to locked position
      wxStopWatch m_stop_watch;          // used for measuring the time it takes for star to drift
      bool        m_stop_watch_started;  // true when stopwatch has been started
      bool        m_normal;              // true when calibration appears successful
      SettingsGuiding* m_parent;
};

#endif
