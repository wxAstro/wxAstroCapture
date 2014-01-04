#include "SettingsGuidingInterfaceFile.h"

//(*InternalHeaders(SettingsGuidingInterfaceFile)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/dirdlg.h>
#include "wxGuidingConfig.h"

//(*IdInit(SettingsGuidingInterfaceFile)
const long SettingsGuidingInterfaceFile::ID_STATICTEXT1 = wxNewId();
const long SettingsGuidingInterfaceFile::ID_TEXTCTRL1 = wxNewId();
const long SettingsGuidingInterfaceFile::ID_BUTTON1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SettingsGuidingInterfaceFile,wxPanel)
	//(*EventTable(SettingsGuidingInterfaceFile)
	//*)
END_EVENT_TABLE()

SettingsGuidingInterfaceFile::SettingsGuidingInterfaceFile(wxWindow* parent,wxWindowID id)
{
	//(*Initialize(SettingsGuidingInterfaceFile)
	wxStaticText* StaticText1;
	wxButton* DirButton;
	wxFlexGridSizer* FlexGridSizer1;
	
	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Target folder for X.DAT and Y.DAT"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	BoxSizer1->Add(StaticText1, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1 = new wxFlexGridSizer(0, 3, 0, 0);
	m_target_dir = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(200,-1), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(m_target_dir, 0, wxTOP|wxBOTTOM|wxLEFT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	DirButton = new wxButton(this, ID_BUTTON1, _("..."), wxDefaultPosition, wxSize(30,-1), 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer1->Add(DirButton, 0, wxTOP|wxBOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(FlexGridSizer1, 0, wxTOP|wxBOTTOM|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
	
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SettingsGuidingInterfaceFile::OnDirButtonClick);
	//*)

	Init();
}

SettingsGuidingInterfaceFile::~SettingsGuidingInterfaceFile()
{
	//(*Destroy(SettingsGuidingInterfaceFile)
	//*)
}

void SettingsGuidingInterfaceFile::Init()
{
   m_target_dir->SetValue(wxGuidingConfig::getFileInterfacePath());
}


void SettingsGuidingInterfaceFile::OnDirButtonClick(wxCommandEvent& WXUNUSED(event))
{
   wxDirDialog dialog(this,_T("Select target folder for X.DAT && Y.DAT"),m_target_dir->GetValue(),wxDD_NEW_DIR_BUTTON);
   if(dialog.ShowModal() == wxID_OK) {
      m_target_dir->SetValue(dialog.GetPath());
      m_target_dir->MarkDirty();
   }
}

void SettingsGuidingInterfaceFile::SaveToConfig()
{
   wxGuidingConfig::saveFileInterfacePath(m_target_dir->GetValue());
}
