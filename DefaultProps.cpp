#include "DefaultProps.h"
#include <wx/bookctrl.h>

//(*InternalHeaders(DefaultProps)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "wxWebcam/wxWebcam.h"
#include "wxFactory.h"


//(*IdInit(DefaultProps)
const long DefaultProps::ID_BUTTON2 = wxNewId();
const long DefaultProps::ID_BUTTON3 = wxNewId();
const long DefaultProps::ID_BUTTON4 = wxNewId();
const long DefaultProps::ID_STATICTEXT3 = wxNewId();
//*)

const long DefaultProps::ID_TIMER1= wxNewId();

BEGIN_EVENT_TABLE(DefaultProps,wxPanel)
	//(*EventTable(DefaultProps)
	//*)
	EVT_TIMER(DefaultProps::ID_TIMER1,DefaultProps::OnTimer)
END_EVENT_TABLE()

DefaultProps::DefaultProps(wxBookCtrlBase* parent,wxWindowID WXUNUSED(id))
: m_parent(parent)
, m_timer(this,ID_TIMER1)
{
	//(*Initialize(DefaultProps)
	wxBoxSizer* BoxSizer4;
	wxBoxSizer* BoxSizer2;
	wxBoxSizer* BoxSizer3;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	BoxSizer3 = new wxBoxSizer(wxVERTICAL);
	StaticBoxSizer2 = new wxStaticBoxSizer(wxVERTICAL, this, _("User Settings"));
	m_userSave_button = new wxButton(this, ID_BUTTON2, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	m_userSave_button->Disable();
	StaticBoxSizer2->Add(m_userSave_button, 1, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_userRestore_button = new wxButton(this, ID_BUTTON3, _("Restore"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	m_userRestore_button->Disable();
	StaticBoxSizer2->Add(m_userRestore_button, 1, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer3->Add(StaticBoxSizer2, 1, wxTOP|wxBOTTOM|wxLEFT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer3 = new wxStaticBoxSizer(wxVERTICAL, this, _("Factory Settings"));
	m_factoryRestore_button = new wxButton(this, ID_BUTTON4, _("Restore"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	m_factoryRestore_button->Disable();
	StaticBoxSizer3->Add(m_factoryRestore_button, 0, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer3->Add(StaticBoxSizer3, 1, wxLEFT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer2->Add(BoxSizer3, 2, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_LEFT|wxALIGN_TOP, 5);
	BoxSizer4 = new wxBoxSizer(wxVERTICAL);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT3, _("These buttons control save \nand restore of camera presets\nwithin the camera memory.\n\nThe restore buttons apply\nthe user or factory settings\nas active camera setting. \n\nThe save button saves the\nactive camera setting in \nthe camera user area."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	BoxSizer4->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5);
	BoxSizer2->Add(BoxSizer4, 3, wxTOP|wxBOTTOM|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(BoxSizer2);
	BoxSizer2->Fit(this);
	BoxSizer2->SetSizeHints(this);

	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DefaultProps::OnUserSave);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DefaultProps::OnUserRestore);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DefaultProps::OnFactoryRestore);
	//*)

	Init();
	m_timer.Start(1000);
}

DefaultProps::~DefaultProps()
{
	//(*Destroy(DefaultProps)
	//*)
	m_timer.Stop();
}

void DefaultProps::OnTimer(wxTimerEvent& WXUNUSED(event))
{
   Init();
}

void DefaultProps::Init()
{
   // if this page is not the current one, do nothing
   if(m_parent->GetCurrentPage() != this)return;

   if(wxWebcam* camera = wxFactory::singleton()->webcam()) {

      // if the camera is busy capturing, do nothing
      if(camera->IsCapturing())return;

      bool enable = false;
      if(camera->IsConnected())enable = true;

      m_userSave_button->Enable(enable);
      m_userRestore_button->Enable(enable);
      m_factoryRestore_button->Enable(enable);

   }
}

bool DefaultProps::Enable(bool enable)
{
   Init();
   return enable;
}


void DefaultProps::OnUserSave(wxCommandEvent& WXUNUSED(event))
{
   if(wxWebcam::DeviceProperties* props = wxFactory::singleton()->webcam()->properties()) {
      props->Settings_SaveUser();
   }
}

void DefaultProps::OnUserRestore(wxCommandEvent& WXUNUSED(event))
{
   if(wxWebcam::DeviceProperties* props = wxFactory::singleton()->webcam()->properties()) {
      props->Settings_LoadUser();
   }
}

void DefaultProps::OnFactoryRestore(wxCommandEvent& WXUNUSED(event))
{
   if(wxWebcam::DeviceProperties* props = wxFactory::singleton()->webcam()->properties()) {
      props->Settings_LoadFactory();
   }
}
