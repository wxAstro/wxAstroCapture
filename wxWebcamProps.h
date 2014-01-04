#ifndef WXWEBCAMPROPS_H
#define WXWEBCAMPROPS_H

#include <wx/propdlg.h>
class wxNotebookEvent;

class wxWebcamProps : public wxPropertySheetDialog
{
   public:
      wxWebcamProps(wxWindow* parent);
      virtual ~wxWebcamProps();

      static wxWebcamProps* singleton();

   protected:
      void OnClose(wxCloseEvent& event);

   private:
      static wxWebcamProps* m_self;

   private:
      DECLARE_EVENT_TABLE()
};



#endif // WXWEBCAMPROPS_H
