#include "LoadSaveProps.h"

//(*InternalHeaders(LoadSaveProps)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "wxWebcam/wxWebcam.h"
#include "wxFactory.h"
#include <wx/config.h>

//(*IdInit(LoadSaveProps)
const long LoadSaveProps::ID_STATICTEXT2 = wxNewId();
const long LoadSaveProps::ID_LISTBOX1 = wxNewId();
const long LoadSaveProps::ID_STATICTEXT3 = wxNewId();
const long LoadSaveProps::ID_TEXTCTRL1 = wxNewId();
const long LoadSaveProps::ID_BUTTON5 = wxNewId();
const long LoadSaveProps::ID_BUTTON2 = wxNewId();
const long LoadSaveProps::ID_BUTTON4 = wxNewId();
const long LoadSaveProps::ID_BUTTON1 = wxNewId();
const long LoadSaveProps::ID_BUTTON3 = wxNewId();
const long LoadSaveProps::ID_STATICTEXT1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(LoadSaveProps,wxPanel)
	//(*EventTable(LoadSaveProps)
	//*)
	EVT_IDLE(LoadSaveProps::OnIdle)
END_EVENT_TABLE()

LoadSaveProps::LoadSaveProps(wxWindow* parent,wxWindowID WXUNUSED(id))
{
	//(*Initialize(LoadSaveProps)
	wxBoxSizer* BoxSizer5;
	wxButton* DeleteSlot;
	wxBoxSizer* BoxSizer2;
	wxStaticText* StaticText3;
	wxBoxSizer* BoxSizer1;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxBoxSizer* BoxSizer3;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	BoxSizer5 = new wxBoxSizer(wxVERTICAL);
	m_slotLabel = new wxStaticText(this, ID_STATICTEXT2, _("Slots"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	BoxSizer5->Add(m_slotLabel, 0, wxTOP|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_listbox = new wxListBox(this, ID_LISTBOX1, wxDefaultPosition, wxSize(-1,100), 0, 0, wxLB_SINGLE|wxLB_SORT, wxDefaultValidator, _T("ID_LISTBOX1"));
	BoxSizer5->Add(m_listbox, 0, wxTOP|wxBOTTOM|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Manage slots"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	BoxSizer5->Add(StaticText3, 0, wxTOP|wxRIGHT|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
	m_slotname = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	BoxSizer3->Add(m_slotname, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	m_AddSlotBtn = new wxButton(this, ID_BUTTON5, _("Add"), wxDefaultPosition, wxSize(50,-1), 0, wxDefaultValidator, _T("ID_BUTTON5"));
	m_AddSlotBtn->Disable();
	BoxSizer3->Add(m_AddSlotBtn, 0, wxTOP|wxBOTTOM|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer5->Add(BoxSizer3, 0, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	DeleteSlot = new wxButton(this, ID_BUTTON2, _("Delete"), wxDefaultPosition, wxSize(30,-1), 0, wxDefaultValidator, _T("ID_BUTTON2"));
	BoxSizer5->Add(DeleteSlot, 0, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	BoxSizer1->Add(BoxSizer5, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer2 = new wxBoxSizer(wxVERTICAL);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxVERTICAL, this, _("Selected slot"));
	m_SaveFromCameraBtn = new wxButton(this, ID_BUTTON4, _("Save to slot"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	m_SaveFromCameraBtn->Disable();
	StaticBoxSizer1->Add(m_SaveFromCameraBtn, 0, wxTOP|wxBOTTOM|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_LoadToCameraBtn = new wxButton(this, ID_BUTTON1, _("Restore to camera"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	m_LoadToCameraBtn->Disable();
	StaticBoxSizer1->Add(m_LoadToCameraBtn, 0, wxTOP|wxBOTTOM|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_SetAsDefaultButton = new wxButton(this, ID_BUTTON3, _("Set as default"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	m_SetAsDefaultButton->Disable();
	StaticBoxSizer1->Add(m_SetAsDefaultButton, 0, wxTOP|wxBOTTOM|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer1->Add(0,0,1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer2->Add(StaticBoxSizer1, 2, wxTOP|wxBOTTOM|wxEXPAND|wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("The slots here are \nsaved to the hard \ndisk."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	BoxSizer2->Add(StaticText1, 0, wxTOP|wxBOTTOM|wxLEFT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer2, 0, wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 10);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);

	Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LoadSaveProps::OnAddSlotClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LoadSaveProps::OnDeleteSlotClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LoadSaveProps::OnSaveFromCameraClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LoadSaveProps::OnLoadToCameraClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LoadSaveProps::OnSetAsDefaultClick);
	//*)

	Init();
}

LoadSaveProps::~LoadSaveProps()
{
	//(*Destroy(LoadSaveProps)
	//*)
}

void LoadSaveProps::Init()
{
   FillSlots();
}

void LoadSaveProps::FillSlots()
{
   wxConfig* config = wxFactory::singleton()->config();
   wxString camPath = cameraPath(_T(""));
   m_listbox->Clear();

   // if the camera hasn't saved anything before, just return
   if(!config->Exists(camPath)) return;

   // set the config path to the current camera
   wxString OldPath = config->GetPath();
   config->SetPath(camPath);

   wxString defaultSlot;
   config->Read(_T("DefaultSlot"),&defaultSlot,_T(""));

   // enumeration variables
   wxString str;
   long dummy;

   // first enum all entries
   bool bCont = config->GetFirstGroup(str, dummy);
   while ( bCont ) {

      m_listbox->Append(str);
      if(str == defaultSlot) {
         m_slotLabel->SetLabel(_T("Slots [ ") + defaultSlot + _T(" ]"));
      }

      bCont = config->GetNextGroup(str, dummy);
   }

   // restore the config path to what it was before
   config->SetPath(OldPath);
}

void LoadSaveProps::OnAddSlotClick(wxCommandEvent&  WXUNUSED(event))
{
   wxString slotname = m_slotname->GetValue();
   if(slotname.Length() > 0) {
      int index = m_listbox->FindString(slotname);
      if(wxNOT_FOUND == index) {
         if(addSlot(slotname)) {
            m_listbox->Append(slotname);
            m_slotname->SetValue(_T(""));
         }
      }
      else {
         wxMessageBox( _T("Slot ") + slotname +  _T(" already exists"), _T("Cannot add new slot"));
      }
   }
   else {
      wxMessageBox( _T("Please type a slot name"), _T("Empty slot name specified"));
   }
}

wxString LoadSaveProps::cameraPath(const wxString& slotname)
{
   wxWebcam* webcam = wxFactory::singleton()->webcam();
   return webcam->ConfigPath(slotname);
}


bool LoadSaveProps::addSlot(const wxString& slotname)
{
   if(wxConfig* config = wxFactory::singleton()->config()) {
      wxWebcam* webcam = wxFactory::singleton()->webcam();

      // set config path to the slot for this camera
      wxString OldPath = config->GetPath();
      config->SetPath(cameraPath(slotname));

      bool retval = webcam->SaveToConfig(config);

      // restore the config path to what it was before
      config->SetPath(OldPath);

      return retval;
   }
   return false;
}

void LoadSaveProps::OnDeleteSlotClick(wxCommandEvent&  WXUNUSED(event))
{
   int index = m_listbox->GetSelection();
   if(wxNOT_FOUND == index) {
      wxMessageBox( _T("No slot selected"), _T("Cannot delete slot"));
   }
   else {
      if(wxConfig* config = wxFactory::singleton()->config()) {

         wxString OldPath = config->GetPath();
         wxString slotname = m_listbox->GetString(index);

         if(config->DeleteGroup(cameraPath(slotname))) {
            m_listbox->Delete(index);
         }
         else {
            wxMessageBox( _T("Delete of slot failed: "+ cameraPath(slotname)), _T("Cannot delete slot"));
         }

         // restore the config path to what it was before
         config->SetPath(OldPath);

      }
   }
}


void LoadSaveProps::OnLoadToCameraClick(wxCommandEvent&  WXUNUSED(event))
{
   int index = m_listbox->GetSelection();
   if(wxNOT_FOUND == index) {
      wxMessageBox( _T("No slot selected"), _T("Cannot restore from slot"));
   }
   else {
      wxString slotname = m_listbox->GetString(index);
      if(wxConfig* config = wxFactory::singleton()->config()) {

         if(!config->Exists(cameraPath(slotname))) {
            wxMessageBox( _T("Slot does not exist: ") + cameraPath(slotname),  _T("Restore failed"));
            return;
         }

         wxWebcam* webcam = wxFactory::singleton()->webcam();
         // set config path to the slot for this camera
         wxString OldPath = config->GetPath();
         config->SetPath(cameraPath(slotname));

         if(!webcam->RestoreFromConfig(config)) {
            wxMessageBox( _T("Restore failed"), _T("Cannot read slot"));
         }

         // restore the config path to what it was before
         config->SetPath(OldPath);
      }
   }
}

void LoadSaveProps::OnSaveFromCameraClick(wxCommandEvent&  WXUNUSED(event))
{
   int index = m_listbox->GetSelection();
   if(wxNOT_FOUND == index) {
      wxMessageBox( _T("No slot selected"), _T("Cannot save to slot"));
   }
   else {
      wxString slotname = m_listbox->GetString(index);
      if(wxConfig* config = wxFactory::singleton()->config()) {

         // set config path to the slot for this camera
         wxString OldPath = config->GetPath();
         config->SetPath(cameraPath(slotname));

         wxWebcam* webcam = wxFactory::singleton()->webcam();
         if(!webcam->SaveToConfig(config)) {
            wxMessageBox( _T("Save failed"), _T("Cannot save to slot"));
         }

         // restore the config path to what it was before
         config->SetPath(OldPath);
      }
   }
}


void LoadSaveProps::OnSetAsDefaultClick(wxCommandEvent& WXUNUSED(event))
{
  int index = m_listbox->GetSelection();
   if(wxNOT_FOUND == index) {
      wxMessageBox( _T("No slot selected"), _T("Cannot set default slot"));
   }
   else {
      if(wxConfig* config = wxFactory::singleton()->config()) {

         wxString OldPath = config->GetPath();
         wxString slotname = m_listbox->GetString(index);
         config->SetPath(cameraPath(_T("")));

         config->Write(_T("DefaultSlot"),slotname);

         // restore the config path to what it was before
         config->SetPath(OldPath);

         m_slotLabel->SetLabel(_T("Slots [ ") + slotname + _T(" ]"));
      }
   }
}


void LoadSaveProps::OnIdle(wxIdleEvent& WXUNUSED(event))
{
   bool enable = false;

   wxWebcam* webcam = wxFactory::singleton()->webcam();
   if(webcam->IsConnected())enable = true;

   m_AddSlotBtn->Enable(enable);
   m_LoadToCameraBtn->Enable(enable);
   m_SaveFromCameraBtn->Enable(enable);
   m_SetAsDefaultButton->Enable(enable);


}
