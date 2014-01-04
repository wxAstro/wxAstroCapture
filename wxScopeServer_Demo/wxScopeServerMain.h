/**
 * @file wxScopeServerMain.h
 *****************************************************************************
 * wxScopeServerMain
 *
 * Definition of a wxWidgets GUI application dialog
 *
 *<hr>
 *
 * @b Project	wxScopeServer Demo Project<br>
 *
 * @author     C.A. Arnholm / M. Burri
 * @date       05-Dec-2007
 * Copyright	2007 - C.A. Arnholm / M. Burri<br>
 * rev:			1.0<br>
 *
 * This is copyrighted material - NOT freeware NOR GPL NOR anything else !!!!!
 *****************************************************************************
 *<hr>
 * @b Updates
 *
 *****************************************************************************
 *<hr>
 * License:
 * You are allowed to use the files included in the wxScopeServer Project for
 * any NON commercial (NON shareware etc.) purposes only.
 * If you want to use it for any other purpose you have to obtain a proper license
 * - just call <ca _at_ arnholm _dot_ org>
 * - or call <bm98 _at_ burri-web _dot_ org>
 *
 * You may distribute the files included in the wxScopeServer Demo project included
 * in projects that you have made yourself providing the above paragraph is respected
 * and the copyright and limited warranty is left included in the source files.
 * Please make sure using an appropriate installer to maintain your customers
 * PC in a healthy condition - thanks
 *<br>
 *
 * No warranties whatsoever
 *<br>
 * LIMITED WARRANTY AND DISCLAIMER OF WARRANTY
 * THE Copyright Holder EXPRESSLY DISCLAIMS ANY WARRANTY FOR THE SOFTWARE. THIS SOFTWARE
 * AND THE ACCOMPANYING FILES ARE GIVEN FOR FREE "AS IS" AND WITHOUT WARRANTIES AS TO
 * PERFORMANCE OF MERCHANTABILITY OR ANY OTHER WARRANTIES WHETHER EXPRESSED OR
 * IMPLIED, OR NONINFRINGEMENT.
 *<br>
 * THIS SOFTWARE IS NOT FAULT TOLERANT AND SHOULD NOT BE USED IN ANY ENVIRONMENT
 * WHICH REQUIRES THIS.
 *<br>
 * The Licensed Software is provided AS IS without warranty of any kind. The Copyright Holder
 * makes no further representations or warranties expressly or implied such as by way
 * of example but not of limitation regarding merchant ability or fitness for
 * any particular purpose, or that Licensed Software is error free, or that the use
 * of the Licensed Software or any copies thereof will not infringe any patent,
 * copyright or trademark of third parties.
 *<br>
 * NO LIABILITY FOR DAMAGES. In no event shall the Copyright Holder or agents or its
 * suppliers be liable to you for any consequential, incidental or indirect damages
 * whatsoever (including, without limitation, damages for loss of business profits,
 * business interruption, loss of business information, or any other pecuniary loss)
 * arising out of the use of or inability to use this SOFTWARE EVEN IF the
 * Copyright Holder HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 *<br>
 * The entire risk arising out of use or performance of the SOFTWARE remains with you.
 *<br>
 * Because of the various hardware and software environments into which this software
 * may be put, NO WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE IS OFFERED.
 *<br>
 * The liability of the Copyright Holder for actual damages from any cause whatsoever,
 * and regardless of the form of action (whether in contract, tort (including negligence),
 * product liability or otherwise) WILL BE LIMITED TO ZERO, NOTHING, NIENTE, NADA, NICHTS.
 *<br>
 ******************************************************************************/

#ifndef WXSCOPESERVERMAIN_H
#define WXSCOPESERVERMAIN_H

#include "wxScopeServerApp.h"

#include "wx/socket.h"

//(*Headers(wxScopeServerDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/statline.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class wxScopeSvr;
class wxPushButton;

class wxScopeServerDialog: public wxDialog
{
	public:

		wxScopeServerDialog(wxWindow* parent,wxWindowID id = -1);
		virtual ~wxScopeServerDialog();

      void WriteLog(const wxString& text);

      void RAplus(bool down);
      void RAminus(bool down);
      void RAoff();

      void DEplus(bool down);
      void DEminus(bool down);
      void DEoff();

	private:

		//(*Handlers(wxScopeServerDialog)
		void OnQuit(wxCommandEvent& event);
		void OnAbout(wxCommandEvent& event);
		void OnRAminusClick(wxCommandEvent& event);
		void OnParallelCheckBoxClick(wxCommandEvent& event);
		void OnRAplusClick(wxCommandEvent& event);
		void OnDEminusClick(wxCommandEvent& event);
		void OnDEplusClick(wxCommandEvent& event);
		//*)

   void OnServerEvent(wxSocketEvent& event);
   void OnSocketEvent(wxSocketEvent& event);

		//(*Identifiers(wxScopeServerDialog)
		static const long ID_STATICTEXT1;
		static const long ID_BUTTON1;
		static const long ID_STATICLINE1;
		static const long ID_BUTTON2;
		static const long ID_TEXTCTRL1;
		static const long ID_BUTTON5;
		static const long ID_BUTTON4;
		static const long ID_BUTTON3;
		static const long ID_BUTTON6;
		static const long ID_CHECKBOX1;
		//*)

		//(*Declarations(wxScopeServerDialog)
		wxBoxSizer* BoxSizer4;
		wxPushButton* m_ra_minus;
		wxBoxSizer* BoxSizer5;
		wxButton* Button1;
		wxStaticText* StaticText1;
		wxBoxSizer* BoxSizer2;
		wxPushButton* m_de_plus;
		wxButton* Button2;
		wxGridSizer* STD_GridSizer;
		wxPushButton* m_de_minus;
		wxStaticLine* StaticLine1;
		wxCheckBox* ParallelCheckBox;
		wxBoxSizer* BoxSizer1;
		wxBoxSizer* BoxSizer3;
		wxTextCtrl* Tx_ServerLog;
		wxPushButton* m_ra_plus;
		//*)

      wxScopeSvr*     m_server;
      bool            m_busy;
      int             m_numClients;

		DECLARE_EVENT_TABLE()
};

#endif // WXSCOPESERVERMAIN_H
