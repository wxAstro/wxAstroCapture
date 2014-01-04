#include "wxMessageArea.h"

//(*InternalHeaders(wxMessageArea)
#include <wx/listctrl.h>
#include <wx/artprov.h>
#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/intl.h>
#include <wx/statbmp.h>
#include <wx/button.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

#include <wx/app.h>
#include <wx/listctrl.h>

//(*IdInit(wxMessageArea)
const long wxMessageArea::ID_STATICBITMAP1 = wxNewId();
const long wxMessageArea::ID_LISTCTRL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(wxMessageArea,wxDialog)
	//(*EventTable(wxMessageArea)
	//*)
END_EVENT_TABLE()

wxMessageArea* wxMessageArea::m_self = 0;

wxMessageArea::wxMessageArea(wxWindow* parent)
{
	//(*Initialize(wxMessageArea)
	wxBoxSizer* BoxSizer2;
	wxBoxSizer* BoxSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;
	
	Create(parent, wxID_ANY, _("Message Box"), wxDefaultPosition, wxDefaultSize, wxSTAY_ON_TOP|wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("wxID_ANY"));
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	StaticBitmap1 = new wxStaticBitmap(this, ID_STATICBITMAP1, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_INFORMATION")),wxART_OTHER), wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER, _T("ID_STATICBITMAP1"));
	BoxSizer1->Add(StaticBitmap1, 0, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5);
	BoxSizer2 = new wxBoxSizer(wxVERTICAL);
	m_listctrl = new wxListCtrl(this, ID_LISTCTRL1, wxDefaultPosition, wxSize(300,100), wxLC_REPORT|wxLC_NO_HEADER|wxNO_BORDER, wxDefaultValidator, _T("ID_LISTCTRL1"));
	BoxSizer2->Add(m_listctrl, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	BoxSizer2->Add(StdDialogButtonSizer1, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer2, 1, wxTOP|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
	//*)

   Clear();

   m_listctrl->InsertColumn(0,wxT(""));


   m_self = this;
}

wxMessageArea::~wxMessageArea()
{
	//(*Destroy(wxMessageArea)
	//*)

	if(m_self == this)m_self = 0;
}

void wxMessageArea::Clear()
{
   if(m_self)m_self->Destroy();
}

void  wxMessageArea::ShowMessage(const wxArrayString& messages, const wxString& caption , const wxPoint& pos)
{
   SetTitle(caption);
   m_listctrl->DeleteAllItems();
   int nitems = messages.GetCount();
   for(int index=0;index<nitems;index++) {
      m_listctrl->InsertItem(index,messages[index]);
   }
   m_listctrl->SetColumnWidth(0,wxLIST_AUTOSIZE );
   Layout();
   Move(pos);
   Show();
}

void wxMessageArea::wxMessageSink(const wxArrayString& messages, const wxString& caption, const wxPoint& pos)
{
   if(m_self) {
      m_self->ShowMessage(messages,caption,pos);
   }
   else {
      wxWindow* parent = wxTheApp->GetTopWindow();
      m_self = new wxMessageArea(parent);
      m_self->ShowMessage(messages,caption,pos);
   }
}


void wxMessageArea::wxMessageSink(const wxString& message, const wxString& caption, const wxPoint& pos)
{
   wxArrayString messages;
   messages.Add(message);
   wxMessageSink(messages,caption,pos);
}
