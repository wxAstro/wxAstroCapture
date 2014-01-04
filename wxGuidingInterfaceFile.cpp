#include "wxGuidingInterfaceFile.h"

//(*InternalHeaders(wxGuidingInterfaceFile)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/dirdlg.h>
#include "wxGuidingConfig.h"
#include "wxFactory.h"

//(*IdInit(wxGuidingInterfaceFile)
const long wxGuidingInterfaceFile::ID_STATICTEXT1 = wxNewId();
const long wxGuidingInterfaceFile::ID_TEXTCTRL1 = wxNewId();
const long wxGuidingInterfaceFile::ID_BUTTON1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(wxGuidingInterfaceFile,wxPanel)
	//(*EventTable(wxGuidingInterfaceFile)
	//*)
END_EVENT_TABLE()

wxGuidingInterfaceFile::wxGuidingInterfaceFile(wxWindow* parent,wxWindowID id,const wxPoint& WXUNUSED(pos),const wxSize& WXUNUSED(size))
{
	//(*Initialize(wxGuidingInterfaceFile)
	wxStaticText* StaticText1;
	wxBoxSizer* BoxSizer2;
	wxBoxSizer* BoxSizer1;
	wxButton* DirButton;
	
	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Target folder for X.DAT and Y.DAT"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	BoxSizer1->Add(StaticText1, 0, wxALL|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	m_target_dir = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(200,-1), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	BoxSizer2->Add(m_target_dir, 1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5);
	DirButton = new wxButton(this, ID_BUTTON1, _("..."), wxDefaultPosition, wxSize(30,-1), 0, wxDefaultValidator, _T("ID_BUTTON1"));
	BoxSizer2->Add(DirButton, 0, wxALL|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	BoxSizer1->Add(BoxSizer2, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
	
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&wxGuidingInterfaceFile::OnDirButtonClick);
	//*)

	Init();
}

void wxGuidingInterfaceFile::Init()
{
   SetName(wxT("File Interface"));
   m_target_dir->SetValue(wxGuidingConfig::getFileInterfacePath());
}

wxGuidingInterfaceFile::~wxGuidingInterfaceFile()
{
	//(*Destroy(wxGuidingInterfaceFile)
	//*)
}

void wxGuidingInterfaceFile::OnDirButtonClick(wxCommandEvent& WXUNUSED(event))
{
   wxDirDialog dialog(this,_T("Select target folder for X.DAT && Y.DAT"),m_target_dir->GetValue(),wxDD_NEW_DIR_BUTTON);
   if(dialog.ShowModal() == wxID_OK) {
      m_target_dir->SetValue(dialog.GetPath());
      m_target_dir->MarkDirty();
   }
}

void wxGuidingInterfaceFile::OnApply()
{
   // enforce file interface
   wxF()->setGuiderInterface(wxGuider::FILE_INTERFACE);

   wxGuidingConfig::saveFileInterfacePath(m_target_dir->GetValue());
}
