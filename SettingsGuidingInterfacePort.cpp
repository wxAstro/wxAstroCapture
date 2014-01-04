#include "SettingsGuidingInterfacePort.h"

//(*InternalHeaders(SettingsGuidingInterfacePort)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(SettingsGuidingInterfacePort)
const long SettingsGuidingInterfacePort::ID_STATICTEXT1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SettingsGuidingInterfacePort,wxPanel)
	//(*EventTable(SettingsGuidingInterfacePort)
	//*)
END_EVENT_TABLE()

SettingsGuidingInterfacePort::SettingsGuidingInterfacePort(wxWindow* parent,wxWindowID id)
{
	//(*Initialize(SettingsGuidingInterfacePort)
	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Please edit the port settings \nusing the \"Guide Port\" tab"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	BoxSizer1->Add(StaticText1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
	//*)
}

SettingsGuidingInterfacePort::~SettingsGuidingInterfacePort()
{
	//(*Destroy(SettingsGuidingInterfacePort)
	//*)
}

void SettingsGuidingInterfacePort::SaveToConfig()
{
   // saving of port parameters is really done somewhere else....
}
