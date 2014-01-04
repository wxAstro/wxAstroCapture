#ifndef WXSTATUSBAREX_H
#define WXSTATUSBAREX_H

#if defined(_WXMSW_ )

#define wxStatusBarEx wxStatusBar

#else

#include <wx/statusbr.h>

class wxStatusBarEx : public wxStatusBar
{
public:
   wxStatusBarEx(wxWindow* parent, wxWindowID id = wxID_ANY, long style = wxST_SIZEGRIP, const wxString& name = _T("statusBar"));
   virtual ~wxStatusBarEx();

   void OnPaint(wxPaintEvent& WXUNUSED(event) );
private:
   DECLARE_EVENT_TABLE()
};

#endif

#endif // WXSTATUSBAREX_H
