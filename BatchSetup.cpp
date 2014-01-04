#include "BatchSetup.h"
#include <math.h>

//(*InternalHeaders(BatchSetup)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "BatchExposure.h"
#include "wxFactory.h"

//(*IdInit(BatchSetup)
const long BatchSetup::ID_STATICTEXT1 = wxNewId();
const long BatchSetup::ID_STATICTEXT2 = wxNewId();
const long BatchSetup::ID_STATICTEXT3 = wxNewId();
const long BatchSetup::ID_CUSTOM1 = wxNewId();
const long BatchSetup::ID_CUSTOM2 = wxNewId();
const long BatchSetup::ID_CUSTOM7 = wxNewId();
const long BatchSetup::ID_CUSTOM3 = wxNewId();
const long BatchSetup::ID_CUSTOM4 = wxNewId();
const long BatchSetup::ID_CUSTOM5 = wxNewId();
const long BatchSetup::ID_CUSTOM6 = wxNewId();
//*)

BEGIN_EVENT_TABLE(BatchSetup,wxPanel)
	//(*EventTable(BatchSetup)
	//*)
END_EVENT_TABLE()

BatchSetup::BatchSetup(wxWindow* parent,wxWindowID WXUNUSED(id),const wxPoint& WXUNUSED(pos),const wxSize& WXUNUSED(size))
{
	//(*Initialize(BatchSetup)
	wxStaticText* StaticText2;
	wxStaticText* StaticText1;
	wxBoxSizer* BoxSizer2;
	wxStaticText* StaticText3;
	wxBoxSizer* BoxSizer1;
	wxStaticBoxSizer* StaticBoxSizer1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Batch Setup"));
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Run"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	BoxSizer2->Add(StaticText1, 0, wxBOTTOM|wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	BoxSizer2->Add(10,-1,0, wxTOP|wxBOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Time [s]"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	BoxSizer2->Add(StaticText2, 0, wxBOTTOM|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 3);
	BoxSizer2->Add(10,-1,0, wxTOP|wxBOTTOM|wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Num"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	BoxSizer2->Add(StaticText3, 0, wxBOTTOM|wxRIGHT|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 3);
	BoxSizer1->Add(BoxSizer2, 1, wxBOTTOM|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	m_exp1 = new BatchExposure(this,ID_CUSTOM1,wxDefaultPosition,wxDefaultSize);
	BoxSizer1->Add(m_exp1, 0, wxBOTTOM|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_exp2 = new BatchExposure(this,ID_CUSTOM2,wxDefaultPosition,wxDefaultSize);
	BoxSizer1->Add(m_exp2, 0, wxBOTTOM|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_exp3 = new BatchExposure(this,ID_CUSTOM7,wxDefaultPosition,wxDefaultSize);
	BoxSizer1->Add(m_exp3, 0, wxBOTTOM|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_exp4 = new BatchExposure(this,ID_CUSTOM3,wxDefaultPosition,wxDefaultSize);
	BoxSizer1->Add(m_exp4, 0, wxBOTTOM|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_exp5 = new BatchExposure(this,ID_CUSTOM4,wxDefaultPosition,wxDefaultSize);
	BoxSizer1->Add(m_exp5, 0, wxBOTTOM|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_exp6 = new BatchExposure(this,ID_CUSTOM5,wxDefaultPosition,wxDefaultSize);
	BoxSizer1->Add(m_exp6, 0, wxBOTTOM|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_exp7 = new BatchExposure(this,ID_CUSTOM6,wxDefaultPosition,wxDefaultSize);
	BoxSizer1->Add(m_exp7, 0, wxBOTTOM|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer1->Add(BoxSizer1, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(StaticBoxSizer1);
	StaticBoxSizer1->Fit(this);
	StaticBoxSizer1->SetSizeHints(this);
	//*)

	Init();
}

BatchSetup::~BatchSetup()
{
	//(*Destroy(BatchSetup)
	//*)
}

void BatchSetup::GetExposures(BatchExposureList& exposures)
{
   // the m_exposures list does not own the contents,
   // it just enables iterating over values
   exposures.clear();
   exposures.push_back(m_exp1);
   exposures.push_back(m_exp2);
   exposures.push_back(m_exp3);
   exposures.push_back(m_exp4);
   exposures.push_back(m_exp5);
   exposures.push_back(m_exp6);
   exposures.push_back(m_exp7);
}

void BatchSetup::Init()
{
   BatchExposureList exposures;
   GetExposures(exposures);

   BatchJobManager* bjm = wxF()->batchJobManager();
   if(bjm->RestoreFromConfig()) {

      // batch exposure was successfully restored from config
      // Set in GUI
      BatchJobManager::iterator ibj = bjm->begin();
      BatchExposureList::iterator iexp = exposures.begin();
      while(iexp != exposures.end() && ibj != bjm->end()) {
         BatchExposure* exp = *iexp++;
         const BatchJobManager::ExpNumSpec& expnum = *ibj++;
         exp->SetValue(expnum.numexp,expnum.exptime,expnum.active);
      }

   }
   else {
      // set some default values
      int numexp      = pow(2.0,double(exposures.size()));
      double exptime  = 1.0;
      BatchExposureList::iterator iexp = exposures.begin();
      while(iexp != exposures.end()) {
         BatchExposure* exp = *iexp++;
         exp->SetValue(numexp,exptime,false);
         numexp  /= 2;
         exptime *= 2;
      }
   }
}

void BatchSetup::GetValue(BatchList& spec)
{
   BatchExposureList exposures;
   GetExposures(exposures);

   // produce list of exposure specifications
   BatchExposureList::iterator iexp = exposures.begin();
   while(iexp != exposures.end()) {
      BatchExposure* exp = *iexp++;
      int numexp = 0;
      double exptime = 0.0;
      bool active = exp->GetValue(numexp,exptime);
      spec.push_back(BatchJobManager::ExpNumSpec(exptime,numexp,active));
   }
}

void BatchSetup::OnResetButtonClick(wxCommandEvent& WXUNUSED(event))
{
   Init();
}
