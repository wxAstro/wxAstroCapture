#include "BatchExposure.h"

//(*InternalHeaders(BatchExposure)
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(BatchExposure)
const long BatchExposure::ID_CHECKBOX1 = wxNewId();
const long BatchExposure::ID_SPINCTRL1 = wxNewId();
const long BatchExposure::ID_SPINCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(BatchExposure,wxPanel)
	//(*EventTable(BatchExposure)
	//*)
END_EVENT_TABLE()

BatchExposure::BatchExposure(wxWindow* parent,wxWindowID id,const wxPoint& WXUNUSED(pos),const wxSize& WXUNUSED(size))
{
	//(*Initialize(BatchExposure)
	wxBoxSizer* BoxSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_active = new wxCheckBox(this, ID_CHECKBOX1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	m_active->SetValue(false);
	BoxSizer1->Add(m_active, 0, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	m_exptime = new wxSpinCtrlDbl(this, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxSize(55,-1), 0, 0, 100, 1, _T("ID_SPINCTRL1"));
	m_exptime->SetValue(_T("1"));
	BoxSizer1->Add(m_exptime, 0, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	m_numexp = new wxSpinCtrl(this, ID_SPINCTRL2, _T("1"), wxDefaultPosition, wxSize(55,-1), 0, 0, 1000, 1, _T("ID_SPINCTRL2"));
	m_numexp->SetValue(_T("1"));
	BoxSizer1->Add(m_numexp, 0, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
	//*)

	Init();
}

BatchExposure::~BatchExposure()
{
	//(*Destroy(BatchExposure)
	//*)
}

void BatchExposure::Init()
{
   m_exptime->SetRange(0.0,1000.0);
   m_exptime->SetIncrement(0.5);
   m_exptime->SetValue(1.0);

}

bool BatchExposure::GetValue(int& numexp, double& exptime)
{
   numexp  = m_numexp->GetValue();
   exptime = m_exptime->GetValue();
   return m_active->IsChecked();
}

void BatchExposure::SetValue(int numexp, double exptime, bool checked)
{
   m_active->SetValue(checked);
   m_numexp->SetValue(numexp);
   m_exptime->SetValue(exptime);
}
