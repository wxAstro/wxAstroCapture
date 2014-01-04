#ifndef BATCHEXPOSURE_H
#define BATCHEXPOSURE_H

#include "wxthings/spinctld.h"

//(*Headers(BatchExposure)
#include <wx/panel.h>
class wxSpinEvent;
class wxCheckBox;
class wxSpinCtrl;
class wxBoxSizer;
//*)

class BatchExposure: public wxPanel {
public:

   BatchExposure(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
   virtual ~BatchExposure();

   void Init();

   void SetValue(int numexp, double exptime, bool checked);
   bool GetValue(int& numexp, double& exptime);

   //(*Declarations(BatchExposure)
   wxSpinCtrlDbl* m_exptime;
   wxSpinCtrl* m_numexp;
   wxCheckBox* m_active;
   //*)

protected:

   //(*Identifiers(BatchExposure)
   static const long ID_CHECKBOX1;
   static const long ID_SPINCTRL1;
   static const long ID_SPINCTRL2;
   //*)

private:

   //(*Handlers(BatchExposure)
   //*)

   DECLARE_EVENT_TABLE()
};

#endif
