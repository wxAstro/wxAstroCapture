#include "wxIoPortCtrl.h"

//(*InternalHeaders(wxIoPortCtrl)
#include <wx/bitmap.h>
#include <wx/font.h>
#include <wx/fontenum.h>
#include <wx/fontmap.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/settings.h>
//*)


//BEGIN_EVENT_TABLE(wxIoPortCtrl,wxControl)
	//(*EventTable(wxIoPortCtrl)
	//*)
//END_EVENT_TABLE()



wxIoPortCtrl::wxIoPortCtrl(wxWindow* parent,
                           wxWindowID id,
                           const wxPoint& pos,
                           const wxSize& size,
                           long style,
                           const wxString& name
                           )
: wxPanel(parent,id,pos,size,style,name)
,m_alignpins(8), m_boxSizer(new wxBoxSizer(wxHORIZONTAL))
{
	SetAlignPins();
}

size_t wxIoPortCtrl::AlignPins()
{
   return m_alignpins;
}

void wxIoPortCtrl::SetAlignPins(size_t apins)
{
   m_alignpins = apins;
}

wxIoPortCtrl::~wxIoPortCtrl()
{
   ClearPins();
}

void wxIoPortCtrl::ClearPins()
{
   // the boxes are owned here, and must be destroyed before clearing the vector
   // Calling Clear(true) also deletes the child windows
   m_boxSizer->Clear(true);

   m_boxes.clear();
}

void wxIoPortCtrl::CreatePins(int npins, unsigned char flags)
{
   ClearPins();

   wxString sflags =_T("");
   for(int ipin=0; ipin<npins; ipin++) {
      int flag = flags % 2;
      flags /= 2;
      sflags = ((flag)? wxT('1') : wxT('0')) + sflags;
   }
   CreatePins(sflags);
}

void wxIoPortCtrl::CreatePins(const wxString& flags)
{
   ClearPins();

   wxSize siz = wxSize(14,-1);

   // due to problems with alignment, we fill the sizer with check-box
   // sized spacers for the slots to the left of the check boxes
   int number = flags.length();
   int nspace = m_alignpins-number;
   if(nspace > 0){
      for(int ispace=0; ispace<nspace; ispace++) {
         m_boxSizer->AddSpacer(siz.GetWidth());
      }
   }

   // reserve space in vector.
   // Not strictly required, but more optimal
   m_boxes.reserve(number);

   // traverse string and create controls
   for(int ibox=0;ibox<number;ibox++) {

      // create checkbox with proper initial value
      wxCheckBox* CheckBox = new wxCheckBox(this,wxNewId(),wxEmptyString,wxDefaultPosition, siz,0,wxDefaultValidator,_T("ID_CHECKBOX1"));
      if(flags.at(ibox) == wxT('0')) CheckBox->SetValue(false);
      else                           CheckBox->SetValue(true);

      // add box to sizer and our vector
      m_boxSizer->Add(CheckBox,1,wxALIGN_CENTER|wxALIGN_CENTER_VERTICAL,5);
      m_boxes.push_back(CheckBox);
   }
   m_boxSizer->AddSpacer(2);

   // recompute the control layout
	this->SetSizer(m_boxSizer);
	m_boxSizer->Fit(this);
   m_boxSizer->SetSizeHints(this);
   m_boxSizer->Layout();
	Layout();
}

size_t wxIoPortCtrl::NumPins() const
{
   return m_boxes.size();
}

wxString wxIoPortCtrl::GetPinsString() const
{
   wxString flags;
   wxCheckBoxVector::const_iterator ibox = m_boxes.begin();
   while(ibox != m_boxes.end()) {
      wxCheckBox* cb = *ibox++;
      flags.Append((cb->GetValue())? wxT('1') : wxT('0'));
   }
   return flags;
}

unsigned char wxIoPortCtrl::GetPinsByte() const
{
   unsigned char value=0;
   unsigned char factor=1;
   wxCheckBoxVector::const_reverse_iterator ibox = m_boxes.rbegin();
   while(ibox != m_boxes.rend()) {
      wxCheckBox* cb = *ibox++;
      unsigned char pin_value = (cb->GetValue())? 1 : 0;
      value  += pin_value*factor;
      factor *= 2;
   }
   return value;
}

bool  wxIoPortCtrl::GetPinValue(size_t iflag, bool& value) const
{
   if(iflag < m_boxes.size()) {
      value = m_boxes[iflag]->GetValue();
      return true;
   }
   return false;
}




