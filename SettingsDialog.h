#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <wx/propdlg.h>
#include <list>
#include "wxcustom/wxPropertyPageDialog.h"

#include "wx/bookctrl.h"
class wxPropertyPage;

class SettingsDialog : public wxPropertyPageDialog
{
   public:
      SettingsDialog(wxWindow* parent);
      virtual ~SettingsDialog();
   protected:
   private:
      DECLARE_EVENT_TABLE()
};



#endif // SETTINGSDIALOG_H
