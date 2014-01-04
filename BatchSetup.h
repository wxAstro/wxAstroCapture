#ifndef BATCHSETUP_H
#define BATCHSETUP_H

class BatchExposure;

//(*Headers(BatchSetup)
#include <wx/panel.h>
class wxStaticText;
class wxBoxSizer;
class wxStaticBoxSizer;
//*)

#include "BatchJobManager.h"

class BatchSetup: public wxPanel {
public:

   BatchSetup(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
   virtual ~BatchSetup();

   void Init();

   typedef BatchJobManager::BatchList BatchList;

   // produce list of exposure specifications
   // get the specification as a list of ExpNumPair values
   void GetValue(BatchList& spec);

   //(*Declarations(BatchSetup)
   BatchExposure* m_exp7;
   BatchExposure* m_exp4;
   BatchExposure* m_exp1;
   BatchExposure* m_exp3;
   BatchExposure* m_exp2;
   BatchExposure* m_exp5;
   BatchExposure* m_exp6;
   //*)

protected:

   //(*Identifiers(BatchSetup)
   static const long ID_STATICTEXT1;
   static const long ID_STATICTEXT2;
   static const long ID_STATICTEXT3;
   static const long ID_CUSTOM1;
   static const long ID_CUSTOM2;
   static const long ID_CUSTOM7;
   static const long ID_CUSTOM3;
   static const long ID_CUSTOM4;
   static const long ID_CUSTOM5;
   static const long ID_CUSTOM6;
   //*)

private:
   typedef std::list<BatchExposure*> BatchExposureList;
   void GetExposures(BatchExposureList& exposures);

   //(*Handlers(BatchSetup)
   void OnResetButtonClick(wxCommandEvent& event);
   //*)

   DECLARE_EVENT_TABLE()
};

#endif
