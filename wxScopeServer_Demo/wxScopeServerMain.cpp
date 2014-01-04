/**
 * @file wxScopeServerMain.cpp
 *****************************************************************************
 * wxScopeServerMain
 *
 * Implementation of a wxWidgets GUI application dialog
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

#ifdef WX_PRECOMP
#include "wx_pch.h"
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "wxScopeServerMain.h"

#include "wxScopeSvr.h"
#include "wxPushButton.h"

//helper functions
enum wxbuildinfoformat {
    short_f, long_f };

wxString wxbuildinfo(wxbuildinfoformat format)
{
    wxString wxbuild(wxVERSION_STRING);

    if (format == long_f )
    {
#if defined(__WXMSW__)
        wxbuild << _T("-Windows");
#elif defined(__UNIX__)
        wxbuild << _T("-Linux");
#endif

#if wxUSE_UNICODE
        wxbuild << _T("-Unicode build");
#else
        wxbuild << _T("-ANSI build");
#endif // wxUSE_UNICODE
    }

    return wxbuild;
}

//(*InternalHeaders(wxScopeServerDialog)
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "pics/ra_plus_up.xpm"
#include "pics/ra_plus_down.xpm"
#include "pics/ra_minus_up.xpm"
#include "pics/ra_minus_down.xpm"

#include "pics/de_plus_up.xpm"
#include "pics/de_plus_down.xpm"
#include "pics/de_minus_up.xpm"
#include "pics/de_minus_down.xpm"

#include "pics/buttons_16x16.xpm"
#include "pics/buttons_64x64.xpm"

//(*IdInit(wxScopeServerDialog)
const long wxScopeServerDialog::ID_STATICTEXT1 = wxNewId();
const long wxScopeServerDialog::ID_BUTTON1 = wxNewId();
const long wxScopeServerDialog::ID_STATICLINE1 = wxNewId();
const long wxScopeServerDialog::ID_BUTTON2 = wxNewId();
const long wxScopeServerDialog::ID_TEXTCTRL1 = wxNewId();
const long wxScopeServerDialog::ID_BUTTON5 = wxNewId();
const long wxScopeServerDialog::ID_BUTTON4 = wxNewId();
const long wxScopeServerDialog::ID_BUTTON3 = wxNewId();
const long wxScopeServerDialog::ID_BUTTON6 = wxNewId();
const long wxScopeServerDialog::ID_CHECKBOX1 = wxNewId();
//*)

// IDs for the controls and the menu commands
enum
{
  // menu items
  SERVER_QUIT = wxID_EXIT,
  SERVER_ABOUT = wxID_ABOUT,

  // id for sockets
  SERVER_ID = 100,
  SOCKET_ID
};

BEGIN_EVENT_TABLE(wxScopeServerDialog,wxDialog)
	//(*EventTable(wxScopeServerDialog)
	//*)
  EVT_MENU(SERVER_QUIT,  wxScopeServerDialog::OnQuit)
  EVT_MENU(SERVER_ABOUT, wxScopeServerDialog::OnAbout)
  EVT_SOCKET(SERVER_ID,  wxScopeServerDialog::OnServerEvent)
  EVT_SOCKET(SOCKET_ID,  wxScopeServerDialog::OnSocketEvent)
END_EVENT_TABLE()

wxScopeServerDialog::wxScopeServerDialog(wxWindow* parent,wxWindowID id)
{
	//(*Initialize(wxScopeServerDialog)
	Create(parent, id, _("wxScopeServer"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxSize(243,474));
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Welcome to \nwxScopeServer"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	wxFont StaticText1Font(18,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,_T("Tahoma"),wxFONTENCODING_DEFAULT);
	StaticText1->SetFont(StaticText1Font);
	BoxSizer2->Add(StaticText1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 10);
	BoxSizer3 = new wxBoxSizer(wxVERTICAL);
	Button1 = new wxButton(this, ID_BUTTON1, _("About"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	BoxSizer3->Add(Button1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
	StaticLine1 = new wxStaticLine(this, ID_STATICLINE1, wxDefaultPosition, wxSize(10,-1), wxLI_HORIZONTAL, _T("ID_STATICLINE1"));
	BoxSizer3->Add(StaticLine1, 0, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
	Button2 = new wxButton(this, ID_BUTTON2, _("Quit"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	BoxSizer3->Add(Button2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
	BoxSizer2->Add(BoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
	BoxSizer1->Add(BoxSizer2, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
	Tx_ServerLog = new wxTextCtrl(this, ID_TEXTCTRL1, _("wxScopeServer Demo:\n"), wxDefaultPosition, wxSize(280,110), wxTE_MULTILINE|wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	BoxSizer4->Add(Tx_ServerLog, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	BoxSizer1->Add(BoxSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
	STD_GridSizer = new wxGridSizer(3, 3, 0, 0);
	STD_GridSizer->Add(-1,-1,0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	m_de_plus = new wxPushButton(this, ID_BUTTON5, _("DE+"), wxDefaultPosition, wxSize(32,32), 0, wxDefaultValidator, _T("ID_BUTTON5"));
	STD_GridSizer->Add(m_de_plus, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	STD_GridSizer->Add(-1,-1,0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_ra_minus = new wxPushButton(this, ID_BUTTON4, _("RA-"), wxDefaultPosition, wxSize(32,32), 0, wxDefaultValidator, _T("ID_BUTTON4"));
	STD_GridSizer->Add(m_ra_minus, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	STD_GridSizer->Add(-1,-1,0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	m_ra_plus = new wxPushButton(this, ID_BUTTON3, _("RA+"), wxDefaultPosition, wxSize(32,32), 0, wxDefaultValidator, _T("ID_BUTTON3"));
	STD_GridSizer->Add(m_ra_plus, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	STD_GridSizer->Add(-1,-1,0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	m_de_minus = new wxPushButton(this, ID_BUTTON6, _("DE-"), wxDefaultPosition, wxSize(32,32), 0, wxDefaultValidator, _T("ID_BUTTON6"));
	STD_GridSizer->Add(m_de_minus, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	STD_GridSizer->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	BoxSizer5->Add(STD_GridSizer, 0, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	BoxSizer5->Add(50,-1,0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ParallelCheckBox = new wxCheckBox(this, ID_CHECKBOX1, _("Parallel guiding"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	ParallelCheckBox->SetValue(false);
	BoxSizer5->Add(ParallelCheckBox, 1, wxALL|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
	BoxSizer1->Add(BoxSizer5, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->SetSizeHints(this);
	
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&wxScopeServerDialog::OnAbout);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&wxScopeServerDialog::OnQuit);
	Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&wxScopeServerDialog::OnDEplusClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&wxScopeServerDialog::OnRAminusClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&wxScopeServerDialog::OnRAplusClick);
	Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&wxScopeServerDialog::OnDEminusClick);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&wxScopeServerDialog::OnParallelCheckBoxClick);
	//*)


/// Our Init


   m_ra_minus->SetIcons(new wxIcon(ra_minus_up_xpm),new wxIcon(ra_minus_down_xpm),NULL,NULL);
   m_ra_plus->SetIcons(new wxIcon(ra_plus_up_xpm),new wxIcon(ra_plus_down_xpm),NULL,NULL);

   m_de_minus->SetIcons(new wxIcon(de_minus_up_xpm),new wxIcon(de_minus_down_xpm),NULL,NULL);
   m_de_plus->SetIcons(new wxIcon(de_plus_up_xpm),new wxIcon(de_plus_down_xpm),NULL,NULL);

   wxIconBundle icons;
   icons.AddIcon(wxIcon(&buttons_16x16_xpm[0]));
   icons.AddIcon(wxIcon(&buttons_64x64_xpm[0]));
   SetIcons(icons);


   // Create the address - defaults to localhost:0 initially
   wxIPV4address addr;
   addr.Service(5618);

   // Create the socket
   m_server = new wxScopeSvr(this, addr);
   // We use Ok() here to see if the server is really listening
   if (! m_server->Ok())
   {
      Tx_ServerLog->AppendText(_("Could not listen at the specified port !\n\n"));
      return;
   }
   else
   {
      Tx_ServerLog->AppendText(_("Server listening: port 5618.\n\n"));
   }

     // Setup the event handler and subscribe to connection events
   m_server->SetEventHandler(*this, SERVER_ID);
   m_server->SetNotify(wxSOCKET_CONNECTION_FLAG);
   m_server->Notify(true);

   m_busy = false;
   m_numClients = 0;
}

wxScopeServerDialog::~wxScopeServerDialog()
{
	//(*Destroy(wxScopeServerDialog)
	//*)
   delete m_server;
}

void wxScopeServerDialog::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close(true);
}

void wxScopeServerDialog::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxString msg = wxbuildinfo(long_f);
    wxMessageBox(msg, _("Welcome to..."));
}

/// Server Code

void wxScopeServerDialog::OnServerEvent(wxSocketEvent& event)
{
  wxString s = _("OnServerEvent: ");
  wxSocketBase *sock;

  switch(event.GetSocketEvent())
  {
    case wxSOCKET_CONNECTION : s.Append(_("wxSOCKET_CONNECTION\n")); break;
    default                  : s.Append(_("Unexpected event !\n")); break;
  }

  Tx_ServerLog->AppendText(s);

  // Accept new connection if there is one in the pending
  // connections queue, else exit. We use Accept(false) for
  // non-blocking accept (although if we got here, there
  // should ALWAYS be a pending connection).

  // only accept one client at any time

  if(m_numClients == 0) {
     m_server->InitConnection();
     sock = m_server->Accept(false);

     if (sock)
     {
       Tx_ServerLog->AppendText(_("New client connection accepted\n\n"));
     }
     else
     {
       Tx_ServerLog->AppendText(_("Error: couldn't accept a new connection\n\n"));
       return;
     }

     sock->SetEventHandler(*this, SOCKET_ID);
     sock->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
     sock->Notify(true);
     sock->SetFlags(wxSOCKET_NOWAIT);

     m_numClients++;
  }
  else {
     Tx_ServerLog->AppendText(_("New client connection refused, server busy\n"));
  }
}

void wxScopeServerDialog::OnSocketEvent(wxSocketEvent& event)
{
  wxString s = _("OnSocketEvent: ");
  wxSocketBase *sock = event.GetSocket();
/*
  // First, print a message
  switch(event.GetSocketEvent())
  {
    case wxSOCKET_INPUT : s.Append(_("wxSOCKET_INPUT\n")); break;
    case wxSOCKET_LOST  : s.Append(_("wxSOCKET_LOST\n")); break;
    default             : s.Append(_("Unexpected event !\n")); break;
  }
  Tx_ServerLog->AppendText(s);
*/

  // Now we process the event
  switch(event.GetSocketEvent())
  {
    case wxSOCKET_INPUT:
    {
      // We disable input events, so that the test doesn't trigger
      // wxSocketEvent again.
      sock->SetNotify(wxSOCKET_LOST_FLAG);

      if (!m_server->HandleINPUT(sock)) // delegate input handling to class
         Tx_ServerLog->AppendText(_("HandleINPUT failed\n"));

      // Enable input events again.
      sock->SetNotify(wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG);
      break;
    }
    case wxSOCKET_LOST:
    {
      m_numClients--;

      // Destroy() should be used instead of delete wherever possible,
      // due to the fact that wxSocket uses 'delayed events' (see the
      // documentation for wxPostEvent) and we don't want an event to
      // arrive to the event handler (the frame, here) after the socket
      // has been deleted. Also, we might be doing some other thing with
      // the socket at the same time; for example, we might be in the
      // middle of a test or something. Destroy() takes care of all
      // this for us.

      Tx_ServerLog->AppendText(_("Closing socket.\n\n"));
      sock->Notify(false);
      sock->Destroy();
      break;
    }
    default: ;
  }

}

void wxScopeServerDialog::WriteLog(const wxString& text)
{
   Tx_ServerLog->AppendText(text);
}

void wxScopeServerDialog::RAplus(bool down)
{
   m_ra_plus->SetButtonState(down);
}

void wxScopeServerDialog::RAminus(bool down)
{
   m_ra_minus->SetButtonState(down);
}

void wxScopeServerDialog::RAoff()
{
   m_ra_plus->SetButtonState(false);
   m_ra_minus->SetButtonState(false);
}


void wxScopeServerDialog::DEplus(bool down)
{
   m_de_plus->SetButtonState(down);
}

void wxScopeServerDialog::DEminus(bool down)
{
   m_de_minus->SetButtonState(down);
}

void wxScopeServerDialog::DEoff()
{
   m_de_plus->SetButtonState(false);
   m_de_minus->SetButtonState(false);
}

void wxScopeServerDialog::OnParallelCheckBoxClick(wxCommandEvent& event)
{
   bool parallel = event.IsChecked();
   m_server->UseParallelGuiding(parallel);
}

void wxScopeServerDialog::OnRAminusClick(wxCommandEvent& WXUNUSED(event))
{
   if(m_ra_minus->IsDown()) {
      // TODO: Start RA motor in negative direction
      if(m_numClients==0)WriteLog(_T("RA- down\n"));
   }
   else {
      // TODO: Stop RA motor
     if(m_numClients==0)WriteLog(_T("RA- up\n"));
   }
}

void wxScopeServerDialog::OnRAplusClick(wxCommandEvent& WXUNUSED(event))
{
   if(m_ra_plus->IsDown()) {
      // TODO: Start RA motor in positive direction
      if(m_numClients==0)WriteLog(_T("RA+ down\n"));
   }
   else {
      // TODO: Stop RA motor
     if(m_numClients==0)WriteLog(_T("RA+ up\n"));
   }
}

void wxScopeServerDialog::OnDEminusClick(wxCommandEvent& WXUNUSED(event))
{
   if(m_de_minus->IsDown()) {
      // TODO: Start DEC motor in negative direction
     if(m_numClients==0)WriteLog(_T("DE- down\n"));
   }
   else {
      // TODO: Stop DEC motor
      if(m_numClients==0)WriteLog(_T("DE- up\n"));
   }
}

void wxScopeServerDialog::OnDEplusClick(wxCommandEvent& WXUNUSED(event))
{
   if(m_de_plus->IsDown()) {
      // TODO: Start DEC motor in positive direction
      if(m_numClients==0)WriteLog(_T("DE+ down\n"));
   }
   else {
      // TODO: Stop DEC motor
      if(m_numClients==0)WriteLog(_T("DE+ up\n"));
   }
}
