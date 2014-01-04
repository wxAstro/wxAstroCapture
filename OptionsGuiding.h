#ifndef OPTIONSGUIDING_H
#define OPTIONSGUIDING_H

#include <wx/propdlg.h>
#include "wxcustom/wxPropertyPageDialog.h"

class OptionsGuiding : public wxPropertyPageDialog
{
   public:
      OptionsGuiding(wxWindow* parent);
      virtual ~OptionsGuiding();

      static OptionsGuiding* singleton();

   protected:
      void OnOk(wxCommandEvent& WXUNUSED(event))     { OnApply();
                                                       Destroy();
                                                     }
      void OnCancel(wxCommandEvent& WXUNUSED(event)) { wxPropertyPageDialog::OnCancel();
                                                       Destroy();
                                                     }
      void OnClose(wxCloseEvent& WXUNUSED(event))    { wxPropertyPageDialog::OnCancel();
                                                       Destroy();
                                                     }

   private:
      static OptionsGuiding* m_self;

   private:
      DECLARE_EVENT_TABLE()
};



#endif // OPTIONSGUIDING_H
