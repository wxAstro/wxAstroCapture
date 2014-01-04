#ifndef WXMESSAGEAREA_H
#define WXMESSAGEAREA_H

//(*Headers(wxMessageArea)
#include <wx/dialog.h>
class wxStaticBitmap;
class wxStdDialogButtonSizer;
class wxListCtrl;
class wxBoxSizer;
//*)

class wxMessageArea: public wxDialog {
public:

   wxMessageArea(wxWindow* parent);
   virtual ~wxMessageArea();

   void ShowMessage(const wxArrayString& messages, const wxString& caption = wxT("Message"), const wxPoint& pos = wxDefaultPosition);

   static void wxMessageSink(const wxArrayString& messages, const wxString& caption =  wxT("Message"), const wxPoint& pos = wxDefaultPosition);
   static void wxMessageSink(const wxString& message, const wxString& caption =  wxT("Message"), const wxPoint& pos = wxDefaultPosition);
   static void Clear();

   //(*Declarations(wxMessageArea)
   wxListCtrl* m_listctrl;
   wxStaticBitmap* StaticBitmap1;
   //*)

protected:

   //(*Identifiers(wxMessageArea)
   static const long ID_STATICBITMAP1;
   static const long ID_LISTCTRL1;
   //*)

private:

   //(*Handlers(wxMessageArea)
   //*)

   DECLARE_EVENT_TABLE()

private:

  static wxMessageArea* m_self;
};

inline void wxMessageSink(const wxArrayString& messages, const wxString& caption =  wxT("Message"), const wxPoint& pos = wxDefaultPosition)
{ wxMessageArea::wxMessageSink(messages,caption,pos); }

inline void wxMessageSink(const wxString& message, const wxString& caption =  wxT("Message"), const wxPoint& pos = wxDefaultPosition)
{ wxMessageArea::wxMessageSink(message,caption,pos); }


#endif
