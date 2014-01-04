#ifndef WXPROPERTYPAGEDIALOG_H
#define WXPROPERTYPAGEDIALOG_H

#include <wx/propdlg.h>
#include <list>
#include "wx/bookctrl.h"
class wxPropertyPage;

class wxPropertyPageDialog : public wxPropertySheetDialog
{
   public:
      wxPropertyPageDialog();
      virtual ~wxPropertyPageDialog();

      virtual int ShowModal();
      virtual void OnApply();
      virtual void OnCancel();

      void OnPageChanged(wxBookCtrlEvent & event);

   protected:
      typedef std::list<wxPropertyPage*> PageList;
      PageList m_pages;

   private:
};



#endif // WXPROPERTYPAGEDIALOG_H
