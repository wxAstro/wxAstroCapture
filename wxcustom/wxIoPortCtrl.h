#ifndef WXIOPORTCTRL_H
#define WXIOPORTCTRL_H

#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include <vector>

//(*Headers(wxIoPortCtrl)
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/panel.h>
//*)
class wxIoPortCtrl : public wxPanel {
public:
   wxIoPortCtrl(wxWindow* parent,
                wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxTAB_TRAVERSAL | wxNO_BORDER,
                const wxString& name = wxPanelNameStr);
   virtual ~wxIoPortCtrl();

   // Create pin controls based on "00001111" strings
   void CreatePins(const wxString& flags);
   void CreatePins(int npins, unsigned char flags);

   // remove all current checkboxes
   void ClearPins();

   // return number of pins
   size_t NumPins() const;

   // return current settings as a "00001111" string
   wxString GetPinsString() const;

   // return current setting as unsigned byte value
   unsigned char GetPinsByte() const;

   // look up an individual pin value, returns false if outside range
   bool GetPinValue(size_t iflag, bool& value) const;

   // number of pins to align after, default = 8
   size_t AlignPins();
   void SetAlignPins(size_t apins=8);

protected:
   size_t           m_alignpins;
   wxBoxSizer*      m_boxSizer;
   typedef std::vector<wxCheckBox*> wxCheckBoxVector;
   wxCheckBoxVector m_boxes;

//   DECLARE_EVENT_TABLE()
};

#endif
