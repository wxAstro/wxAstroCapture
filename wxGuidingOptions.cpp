#include "wxGuidingOptions.h"
#include "wxGuidingInterfaceFile.h"
#include "wxGuidingInterfaceBridge.h"
#include "wxGuidingInterfaceSerialPort.h"
#include "wxGuidingInterfaceParallelPort.h"
#include "wxGuidingInterfaceUsbPort.h"

#include "wxFactory.h"
#include "wxMessageArea.h"

#include "wxcustom/wxPropertyPage.h"

//(*InternalHeaders(wxGuidingOptions)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <map>
using namespace std;

//(*IdInit(wxGuidingOptions)
const long wxGuidingOptions::ID_PANEL1 = wxNewId();
const long wxGuidingOptions::ID_CHOICEBOOK1 = wxNewId();
const long wxGuidingOptions::ID_PANEL2 = wxNewId();
const long wxGuidingOptions::ID_NOTEBOOK1 = wxNewId();
const long wxGuidingOptions::ID_BUTTON1 = wxNewId();
const long wxGuidingOptions::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(wxGuidingOptions,wxDialog)
	//(*EventTable(wxGuidingOptions)
	//*)
END_EVENT_TABLE()

wxGuidingOptions* wxGuidingOptions::m_self = 0;
wxGuidingOptions* wxGuidingOptions::singleton()
{
   return m_self;
}

wxGuidingOptions::wxGuidingOptions(wxWindow* parent,wxWindowID id,const wxPoint& WXUNUSED(pos),const wxSize& WXUNUSED(size))
{
	//(*Initialize(wxGuidingOptions)
	wxButton* ApplyButton;
	wxBoxSizer* BoxSizer2;
	wxButton* CloseButton;
	wxBoxSizer* BoxSizer1;
	wxBoxSizer* BoxSizer3;
	
	Create(parent, id, _("Guiding Options"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	Notebook1 = new wxNotebook(this, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK1"));
	m_calibration = new SettingsGuiding(Notebook1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	m_guiding_interface = new wxPanel(Notebook1, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	BoxSizer3 = new wxBoxSizer(wxVERTICAL);
	m_interfaceChoiceBook = new wxChoicebook(m_guiding_interface, ID_CHOICEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_CHOICEBOOK1"));
	BoxSizer3->Add(m_interfaceChoiceBook, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	m_guiding_interface->SetSizer(BoxSizer3);
	BoxSizer3->Fit(m_guiding_interface);
	BoxSizer3->SetSizeHints(m_guiding_interface);
	Notebook1->AddPage(m_calibration, _("Calibration"), false);
	Notebook1->AddPage(m_guiding_interface, _("Guiding Interface"), false);
	BoxSizer1->Add(Notebook1, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5);
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	ApplyButton = new wxButton(this, ID_BUTTON1, _("Apply"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	BoxSizer2->Add(ApplyButton, 1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5);
	CloseButton = new wxButton(this, ID_BUTTON2, _("Close"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	BoxSizer2->Add(CloseButton, 1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5);
	BoxSizer1->Add(BoxSizer2, 0, wxLEFT|wxRIGHT|wxALIGN_RIGHT|wxALIGN_BOTTOM, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
	
	Connect(ID_CHOICEBOOK1,wxEVT_COMMAND_CHOICEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&wxGuidingOptions::OnIinterfaceChoiceBookPageChanged);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&wxGuidingOptions::OnApplyButtonClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&wxGuidingOptions::OnCloseButtonClick);
	//*)

   m_self = this;
   Init();
}

wxGuidingOptions::~wxGuidingOptions()
{
	//(*Destroy(wxGuidingOptions)
	//*)
}

void wxGuidingOptions::Init()
{
   InitInterfaces();
}

void wxGuidingOptions::InitInterfaces()
{
   /// Add the required pages to m_interfaceChoiceBook
   /// First delete what we have so far
   m_interfaceChoiceBook->DeleteAllPages();

   /// File interface is always available
   wxGuidingInterfaceFile* file_page = new wxGuidingInterfaceFile(m_interfaceChoiceBook);
   m_interfaceChoiceBook->AddPage(file_page,file_page->GetName());

   /// Bridge interface is always available
   wxGuidingInterfaceBridge* bridge_page = new wxGuidingInterfaceBridge(m_interfaceChoiceBook);
   m_interfaceChoiceBook->AddPage(bridge_page,bridge_page->GetName());

   /// traverse the serial and parallel ports

   /// maintain a map between interface names and choicebook page numbers
   map<wxString,int> interface_pages;
   int ipageno = 0;
   interface_pages[wxT("FILE_INTERFACE")]   = ipageno++;
   interface_pages[wxT("BRIDGE_INTERFACE")] = ipageno++;

   wxPortDescrList& pList = wxF()->portManager()->PortDescriptors();
   wxPortDescrList::port_iterator iport = pList.port_begin();
   while(iport != pList.port_end()){
      const wxPortDescr& port_entry = pList.PortDescr(iport++);
      if (port_entry.IsGuidePort())  {
         if(port_entry.IsPortSerial()) {

            /// create and add the serial port page
            wxGuidingInterfaceSerialPort* serial_port_page = new wxGuidingInterfaceSerialPort(m_interfaceChoiceBook);
            serial_port_page->SetName(port_entry.PortName());
            m_interfaceChoiceBook->AddPage(serial_port_page,serial_port_page->GetName());
            interface_pages[serial_port_page->GetName()] = ipageno++;
         }
         else if(port_entry.IsPortParallel()) {

            /// create and add the parallel port page
            wxGuidingInterfaceParallelPort* parallel_port_page = new wxGuidingInterfaceParallelPort(m_interfaceChoiceBook);
            parallel_port_page->SetName(port_entry.PortName());
            m_interfaceChoiceBook->AddPage(parallel_port_page,parallel_port_page->GetName());
            interface_pages[parallel_port_page->GetName()] = ipageno++;
         }
         else if(port_entry.IsPortGPUSB()) {

            /// create and add the GPUSB port page
            wxGuidingInterfaceUsbPort* usb_port_page = new wxGuidingInterfaceUsbPort(m_interfaceChoiceBook);
            usb_port_page->SetName(port_entry.PortName());
            m_interfaceChoiceBook->AddPage(usb_port_page,usb_port_page->GetName());
            interface_pages[usb_port_page->GetName()] = ipageno++;
         }
      }
   }

   /// select the default page based on the last saved configuration
   wxString default_page = wxT("FILE_INTERFACE");
   wxGuider::guider_interface intfc = wxF()->guider()->interface_type();
   switch(intfc) {
      case wxGuider::FILE_INTERFACE:   { default_page = wxT("FILE_INTERFACE")   ; break; }
      case wxGuider::BRIDGE_INTERFACE: { default_page = wxT("BRIDGE_INTERFACE") ; break; }
      case wxGuider::PORT_INTERFACE:   {
         if(wxGuiderPort*  port = wxF()->guidePort()) {
            default_page = port->PortName();
         }
         break;
      }
      default: { };
   };

   /// force the page selection
   m_interfaceChoiceBook->ChangeSelection(interface_pages[default_page]);
   m_interfaceChoiceBook->Refresh();

}

void wxGuidingOptions::OnIinterfaceChoiceBookPageChanged(wxChoicebookEvent& WXUNUSED(event))
{
   if(wxNoteBookPageEx* page = dynamic_cast<wxNoteBookPageEx*>(m_interfaceChoiceBook->GetPage(m_interfaceChoiceBook->GetSelection()))) {

      /// reinitialise the page
      page->Init();
   }
   else {
      wxMessageSink(wxT("Programming error: OnIinterfaceChoiceBookPageChanged failed to get ChoiceBook page"));
   }
}


void wxGuidingOptions::OnApplyButtonClick(wxCommandEvent& WXUNUSED(event))
{
   /// Just a cludge until we have eliminated the old guiding dialog
   /// Don't show message here if dynamic_cast returns 0, since some pages are not wxPropertyPage
   if(wxPropertyPage* page =  dynamic_cast<wxPropertyPage*>(Notebook1->GetPage(Notebook1->GetSelection()))) {
      /// Apply the page
      page->OnApply();
   }


   if(wxNoteBookPageEx* interface_page = dynamic_cast<wxNoteBookPageEx*>(m_interfaceChoiceBook->GetPage(m_interfaceChoiceBook->GetSelection()))) {

      /// Apply the page
      interface_page->OnApply();
   }
   else {
      wxMessageSink(wxT("Programming error: OnApplyButtonClick failed to get ChoiceBook page"));
   }
}

void wxGuidingOptions::OnCloseButtonClick(wxCommandEvent& WXUNUSED(event))
{
   /// close the dialog
   /// The dialog is a singleton, so we simply hide it
   Show(false);
}
