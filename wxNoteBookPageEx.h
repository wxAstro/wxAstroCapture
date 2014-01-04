#ifndef WXNOTEBOOKPAGEEX_H
#define WXNOTEBOOKPAGEEX_H

// This class is just an abstract interface used for pages in a wxNoteBook or wxChoiceBook
#include <wx/panel.h>

class wxNoteBookPageEx : public wxPanel {
public:
   wxNoteBookPageEx();
   virtual ~wxNoteBookPageEx();

   /// Init is called when page should be re-initialised
   virtual void Init() = 0;

   /// On apply is called whenever the current page values should be applied
   virtual void OnApply() = 0;
};

#endif // WXNOTEBOOKPAGEEX_H
