#include "wxPropertyPageDialog.h"
#include "wxPropertyPage.h"

wxPropertyPageDialog::wxPropertyPageDialog()
{
   //ctor
}

wxPropertyPageDialog::~wxPropertyPageDialog()
{
   //dtor
}

int wxPropertyPageDialog::ShowModal()
{
   int retval = wxDialog::ShowModal();
   if(retval == wxID_OK) {
      // traverse pages and apply any changes
      OnApply();
   }
   else {
      // the changes are to be discarded
      OnCancel();
   }
   return retval;
}

void wxPropertyPageDialog::OnApply()
{
   PageList::iterator ipage = m_pages.begin();
   while(ipage != m_pages.end()) {
      wxPropertyPage* page = *ipage++;
      page->OnApply();
   }
}

void wxPropertyPageDialog::OnCancel()
{
   PageList::iterator ipage = m_pages.begin();
   while(ipage != m_pages.end()) {
      wxPropertyPage* page = *ipage++;
      page->OnCancel();
   }
}

void wxPropertyPageDialog::OnPageChanged(wxBookCtrlEvent & event)
{

   int page = event.GetSelection();
   if(page != -1) {
      if(wxWindow* window = GetBookCtrl()->GetPage(page)) {
         if(wxPropertyPage* ppage = dynamic_cast<wxPropertyPage*>(window)) {
            if(!ppage->IsInit()) {
               ppage->Init();
            }
         }
      }
   }
}
