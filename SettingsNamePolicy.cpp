#include "SettingsNamePolicy.h"

//(*InternalHeaders(SettingsNamePolicy)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/listbox.h>
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

#include <wx/msgdlg.h>
#include "wxFactory.h"
#include "wxWebcam/wxArtcam.h"

//(*IdInit(SettingsNamePolicy)
const long SettingsNamePolicy::ID_STATICTEXT1 = wxNewId();
const long SettingsNamePolicy::ID_STATICTEXT2 = wxNewId();
const long SettingsNamePolicy::ID_LISTBOX1 = wxNewId();
const long SettingsNamePolicy::ID_BUTTON1 = wxNewId();
const long SettingsNamePolicy::ID_BUTTON2 = wxNewId();
const long SettingsNamePolicy::ID_STATICTEXT3 = wxNewId();
const long SettingsNamePolicy::ID_LISTBOX2 = wxNewId();
const long SettingsNamePolicy::ID_BUTTON3 = wxNewId();
const long SettingsNamePolicy::ID_BUTTON4 = wxNewId();
const long SettingsNamePolicy::ID_CHECKBOX1 = wxNewId();
const long SettingsNamePolicy::ID_STATICTEXT4 = wxNewId();
const long SettingsNamePolicy::ID_TEXTCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SettingsNamePolicy,wxPanel)
	//(*EventTable(SettingsNamePolicy)
	//*)
END_EVENT_TABLE()

SettingsNamePolicy::SettingsNamePolicy(wxWindow* parent,wxWindowID WXUNUSED(id))
: m_show_increment_checkbox(false)
{
	//(*Initialize(SettingsNamePolicy)
	wxBoxSizer* BoxSizer4;
	wxBoxSizer* BoxSizer6;
	wxBoxSizer* BoxSizer5;
	wxBoxSizer* BoxSizer7;
	wxBoxSizer* BoxSizer8;
	wxStaticText* StaticText2;
	wxStaticText* StaticText1;
	wxBoxSizer* BoxSizer2;
	wxStaticText* StaticText3;
	wxBoxSizer* BoxSizer1;
	wxButton* AddButton;
	wxBoxSizer* BoxSizer9;
	wxButton* RemoveButton;
	wxBoxSizer* BoxSizer3;
	wxStaticText* StaticText4;
	
	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Name policy definition"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	BoxSizer2->Add(StaticText1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer2, 0, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
	BoxSizer4 = new wxBoxSizer(wxVERTICAL);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Available"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	BoxSizer4->Add(StaticText2, 0, wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_list_available = new wxListBox(this, ID_LISTBOX1, wxDefaultPosition, wxSize(140,100), 0, 0, 0, wxDefaultValidator, _T("ID_LISTBOX1"));
	BoxSizer4->Add(m_list_available, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer3->Add(BoxSizer4, 0, wxTOP|wxBOTTOM|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer5 = new wxBoxSizer(wxVERTICAL);
	BoxSizer5->Add(-1,-1,0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	AddButton = new wxButton(this, ID_BUTTON1, _("Add =>"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	BoxSizer5->Add(AddButton, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	RemoveButton = new wxButton(this, ID_BUTTON2, _("<= Remove"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	BoxSizer5->Add(RemoveButton, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer3->Add(BoxSizer5, 0, wxTOP|wxBOTTOM|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer6 = new wxBoxSizer(wxVERTICAL);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Used"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	BoxSizer6->Add(StaticText3, 0, wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_list_used = new wxListBox(this, ID_LISTBOX2, wxDefaultPosition, wxSize(140,100), 0, 0, 0, wxDefaultValidator, _T("ID_LISTBOX2"));
	BoxSizer6->Add(m_list_used, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
	UpButton = new wxButton(this, ID_BUTTON3, _("Up"), wxDefaultPosition, wxSize(35,-1), 0, wxDefaultValidator, _T("ID_BUTTON3"));
	BoxSizer8->Add(UpButton, 0, wxTOP|wxBOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	DownButton = new wxButton(this, ID_BUTTON4, _("Down"), wxDefaultPosition, wxSize(45,-1), 0, wxDefaultValidator, _T("ID_BUTTON4"));
	BoxSizer8->Add(DownButton, 0, wxTOP|wxBOTTOM|wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer6->Add(BoxSizer8, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer3->Add(BoxSizer6, 0, wxTOP|wxBOTTOM|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer9 = new wxBoxSizer(wxVERTICAL);
	BoxSizer9->Add(150,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_auto_reset_checkbox = new wxCheckBox(this, ID_CHECKBOX1, _("Auto-reset increment"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	m_auto_reset_checkbox->SetValue(false);
	BoxSizer9->Add(m_auto_reset_checkbox, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer3->Add(BoxSizer9, 1, wxTOP|wxBOTTOM|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer3, 0, wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer7 = new wxBoxSizer(wxVERTICAL);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Name preview:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	BoxSizer7->Add(StaticText4, 0, wxTOP|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_name_preview = new wxTextCtrl(this, ID_TEXTCTRL2, _("Text"), wxDefaultPosition, wxSize(350,-1), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	BoxSizer7->Add(m_name_preview, 0, wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer7, 0, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
	
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SettingsNamePolicy::OnAddButtonClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SettingsNamePolicy::OnRemoveButtonClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SettingsNamePolicy::OnUpButtonClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SettingsNamePolicy::OnDownButtonClick);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&SettingsNamePolicy::OnAutoResetCheckboxClick);
	//*)
}

SettingsNamePolicy::~SettingsNamePolicy()
{
	//(*Destroy(SettingsNamePolicy)
	//*)
}

void SettingsNamePolicy::Init()
{
   m_list_available->Append(policy()->GetAvailableTemplate());
   m_list_used->Append(policy()->GetNameTemplate());

   m_auto_reset_checkbox->Show(m_show_increment_checkbox);

   RefreshNamePreview();
   SetInit(true);

}


void SettingsNamePolicy::OnAddButtonClick(wxCommandEvent& WXUNUSED(event))
{
   size_t index = m_list_available->GetSelection();
   if(wxNOT_FOUND != int(index)) {
      wxString label = m_list_available->GetString(index);

      // set current selection to next item (if possible) before deleting from list
      if(m_list_available->GetCount() > index+1) m_list_available->SetSelection(index+1);
      m_list_available->Delete(index);

      // insert before selected item in used list, if any
      int index_used = m_list_used->GetSelection();
      if(wxNOT_FOUND != index_used) {
         m_list_used->Insert(label,index_used);
      }
      else {
         m_list_used->Append(label);
      }
      RefreshNamePreview();
   }
   else {
      wxMessageBox(_T("Nothing selected in list of available items"));
   }
}

void SettingsNamePolicy::OnRemoveButtonClick(wxCommandEvent& WXUNUSED(event))
{
   size_t index = m_list_used->GetSelection();
   if(wxNOT_FOUND != int(index)) {
      wxString label = m_list_used->GetString(index);
      if(!policy()->IsMandatoryItem(label)) {

         // set current selection to next item (if possible) before deleting from list
         if(m_list_used->GetCount() > index+1) m_list_used->SetSelection(index+1);
         m_list_used->Delete(index);

         m_list_available->Append(label);
         RefreshNamePreview();
      }
      else {
         wxMessageBox(_T("The item '") + label + _T("' is mandatory here and cannot be removed."));
      }
   }
   else {
      wxMessageBox(_T("Nothing selected in list of used items"));
   }
}

void SettingsNamePolicy::OnUpButtonClick(wxCommandEvent& WXUNUSED(event))
{
   size_t index = m_list_used->GetSelection();
   if(wxNOT_FOUND != int(index)) {
      wxString label = m_list_used->GetString(index);
      m_list_used->Delete(index);
      if(index >0)--index;
      m_list_used->Insert(label,index);
      m_list_used->SetSelection(index);
      RefreshNamePreview();
   }
   else {
      wxMessageBox(_T("Nothing selected in list of used items"));
   }
}

void SettingsNamePolicy::OnDownButtonClick(wxCommandEvent& WXUNUSED(event))
{
   size_t index = m_list_used->GetSelection();
   if(wxNOT_FOUND != int(index)) {
      wxString label = m_list_used->GetString(index);
      m_list_used->Delete(index);
      if(index < m_list_used->GetCount())++index;
      m_list_used->Insert(label,index);
      m_list_used->SetSelection(index);
      RefreshNamePreview();
   }
   else {
      wxMessageBox(_T("Nothing selected in list of used items"));
   }
}

void SettingsNamePolicy::RefreshNamePreview()
{
   // mirror selections to policy
   policy()->SetNameTemplate(m_list_used->GetStrings());
   policy()->SetAvailableTemplate(m_list_available->GetStrings());

   // recompute the name
   policy()->SetPreviewMode(true);
   m_name_preview->SetValue(policy()->GetName());
   policy()->SetPreviewMode(false);
}

bool SettingsNamePolicy::OnApply()
{
   // we have already changed the policy object
   // just confirm by saving it to wxConfig
   return policy()->SaveToConfig(wxF()->config());
}

void SettingsNamePolicy::OnCancel()
{
   // abandon the settings by restoring from config
   policy()->RestoreFromConfig(wxF()->config());
}


/// Specialisation for Container name policy


wxNamePolicy* SettingsNamePolicyContainer::policy()
{
   return wxF()->getNamePolicyContainer();
}


/// Specialisation for File name policy


wxNamePolicy* SettingsNamePolicyFile::policy()
{
   return wxF()->getNamePolicyFile();
}

void SettingsNamePolicyFile::Init()
{
   m_show_increment_checkbox= true;
   SettingsNamePolicy::Init();

   m_auto_reset_checkbox->SetValue(wxF()->getNamePolicyFile()->GetAutoResetIncrement());
}


void SettingsNamePolicy::OnAutoResetCheckboxClick(wxCommandEvent& event)
{
   policy()->SetAutoResetIncrement(event.IsChecked());
}
