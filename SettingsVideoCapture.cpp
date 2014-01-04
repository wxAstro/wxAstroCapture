#include "SettingsVideoCapture.h"

//(*InternalHeaders(SettingsVideoCapture)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "wxFactory.h"

//(*IdInit(SettingsVideoCapture)
const long SettingsVideoCapture::ID_STATICTEXT4 = wxNewId();
const long SettingsVideoCapture::ID_TEXTCTRL2 = wxNewId();
const long SettingsVideoCapture::ID_STATICTEXT1 = wxNewId();
const long SettingsVideoCapture::ID_TEXTCTRL1 = wxNewId();
const long SettingsVideoCapture::ID_BUTTON1 = wxNewId();
const long SettingsVideoCapture::ID_STATICLINE1 = wxNewId();
const long SettingsVideoCapture::ID_STATICLINE2 = wxNewId();
const long SettingsVideoCapture::ID_STATICLINE3 = wxNewId();
const long SettingsVideoCapture::ID_STATICTEXT2 = wxNewId();
const long SettingsVideoCapture::ID_STATICTEXT5 = wxNewId();
const long SettingsVideoCapture::ID_STATICTEXT7 = wxNewId();
const long SettingsVideoCapture::ID_CHOICE2 = wxNewId();
const long SettingsVideoCapture::ID_CHOICE3 = wxNewId();
const long SettingsVideoCapture::ID_CHOICE5 = wxNewId();
const long SettingsVideoCapture::ID_STATICTEXT3 = wxNewId();
const long SettingsVideoCapture::ID_STATICTEXT6 = wxNewId();
const long SettingsVideoCapture::ID_STATICTEXT8 = wxNewId();
const long SettingsVideoCapture::ID_CHOICE1 = wxNewId();
const long SettingsVideoCapture::ID_CHOICE4 = wxNewId();
const long SettingsVideoCapture::ID_CHOICE6 = wxNewId();
const long SettingsVideoCapture::ID_PANEL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SettingsVideoCapture,wxPanel)
	//(*EventTable(SettingsVideoCapture)
	//*)
END_EVENT_TABLE()

SettingsVideoCapture::SettingsVideoCapture(wxWindow* parent,wxWindowID WXUNUSED(id))
{
	//(*Initialize(SettingsVideoCapture)
	wxStaticText* StaticText2;
	wxStaticText* StaticText6;
	wxPanel* Panel1;
	wxStaticText* StaticText1;
	wxStaticText* StaticText3;
	wxStaticLine* StaticLine2;
	wxStaticText* StaticText5;
	wxStaticLine* StaticLine3;
	wxStaticLine* StaticLine1;
	wxButton* DirButton;
	wxFlexGridSizer* FlexGridSizer1;
	wxStaticText* StaticText4;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	Panel1 = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 3, 0, 0);
	StaticText4 = new wxStaticText(Panel1, ID_STATICTEXT4, _("Capture file name prefix"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_fileNamePrefixCtrl = new wxTextCtrl(Panel1, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer1->Add(m_fileNamePrefixCtrl, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(Panel1, ID_STATICTEXT1, _("Capture folder"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_folderCtrl = new wxTextCtrl(Panel1, ID_TEXTCTRL1, _("Text"), wxDefaultPosition, wxSize(280,-1), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	m_folderCtrl->SetMaxLength(100);
	FlexGridSizer1->Add(m_folderCtrl, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	DirButton = new wxButton(Panel1, ID_BUTTON1, _("..."), wxDefaultPosition, wxSize(30,-1), 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer1->Add(DirButton, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticLine1 = new wxStaticLine(Panel1, ID_STATICLINE1, wxDefaultPosition, wxSize(10,-1), wxLI_HORIZONTAL, _T("ID_STATICLINE1"));
	FlexGridSizer1->Add(StaticLine1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticLine2 = new wxStaticLine(Panel1, ID_STATICLINE2, wxDefaultPosition, wxSize(10,-1), wxLI_HORIZONTAL, _T("ID_STATICLINE2"));
	FlexGridSizer1->Add(StaticLine2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticLine3 = new wxStaticLine(Panel1, ID_STATICLINE3, wxDefaultPosition, wxSize(10,-1), wxLI_HORIZONTAL, _T("ID_STATICLINE3"));
	FlexGridSizer1->Add(StaticLine3, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(Panel1, ID_STATICTEXT2, _("Sample format"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	StaticText5 = new wxStaticText(Panel1, ID_STATICTEXT5, _("Frame# font"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText7 = new wxStaticText(Panel1, ID_STATICTEXT7, _("Fps font"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer1->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_captureChoice = new wxChoice(Panel1, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	m_captureChoice->Append(_("YUV420"));
	m_captureChoice->Append(_("RGB24"));
	FlexGridSizer1->Add(m_captureChoice, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5);
	m_frame_font = new wxChoice(Panel1, ID_CHOICE3, wxDefaultPosition, wxSize(90,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
	m_frame_font->SetSelection( m_frame_font->Append(_("Off")) );
	m_frame_font->Append(_("Small"));
	m_frame_font->Append(_("Medium"));
	m_frame_font->Append(_("Large"));
	m_frame_font->Append(_("Huge"));
	FlexGridSizer1->Add(m_frame_font, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_fps_font = new wxChoice(Panel1, ID_CHOICE5, wxDefaultPosition, wxSize(90,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE5"));
	m_fps_font->SetSelection( m_fps_font->Append(_("Off")) );
	m_fps_font->Append(_("Small"));
	m_fps_font->Append(_("Medium"));
	m_fps_font->Append(_("Large"));
	m_fps_font->Append(_("Huge"));
	FlexGridSizer1->Add(m_fps_font, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(Panel1, ID_STATICTEXT3, _("Container file format"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
	StaticText6 = new wxStaticText(Panel1, ID_STATICTEXT6, _("In-frame timestamp"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer1->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText8 = new wxStaticText(Panel1, ID_STATICTEXT8, _("Performance level"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer1->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_containerChoice = new wxChoice(Panel1, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	m_containerChoice->Append(_("AVI"));
	m_containerChoice->Append(_("ZIP - FITS"));
	m_containerChoice->Append(_("FOLDER - FITS"));
	FlexGridSizer1->Add(m_containerChoice, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5);
	m_frame_timestamp = new wxChoice(Panel1, ID_CHOICE4, wxDefaultPosition, wxSize(90,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE4"));
	m_frame_timestamp->SetSelection( m_frame_timestamp->Append(_("Off")) );
	m_frame_timestamp->Append(_("Small"));
	m_frame_timestamp->Append(_("Medium"));
	FlexGridSizer1->Add(m_frame_timestamp, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_perf_level = new wxChoice(Panel1, ID_CHOICE6, wxDefaultPosition, wxSize(90,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE6"));
	m_perf_level->SetSelection( m_perf_level->Append(_("Low")) );
	m_perf_level->Append(_("Mid"));
	m_perf_level->Append(_("High"));
	FlexGridSizer1->Add(m_perf_level, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel1->SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(Panel1);
	FlexGridSizer1->SetSizeHints(Panel1);

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SettingsVideoCapture::OnDirButtonClick);
	//*)

	Init();
}

SettingsVideoCapture::~SettingsVideoCapture()
{
	//(*Destroy(SettingsVideoCapture)
	//*)
}

void SettingsVideoCapture::Init()
{
   wxString folder = wxF()->captureFolder();
   m_folderCtrl->SetValue(folder);
   m_folderCtrl->MarkDirty();

   wxString prefix = wxF()->capturePrefix();
   m_fileNamePrefixCtrl->SetValue(prefix);

   wxConfig* config = wxF()->config();
   int sformat,sdefVal(0);
   config->Read(_T("CaptureFormat"),&sformat,sdefVal);
   m_captureChoice->SetSelection(sformat);

   int cformat,cdefVal(0);
   config->Read(_T("ContainerFormat"),&cformat,cdefVal);
   m_containerChoice->SetSelection(cformat);

   int ifont_size,ifont_size_def(2);
   config->Read(_T("FrameCounterFontSize"),&ifont_size,ifont_size_def);
   m_frame_font->SetSelection(ifont_size);

   int iframe_timestamp,iframe_timestamp_def(0);
   config->Read(_T("FrameTimeStampSize"),&iframe_timestamp,iframe_timestamp_def);
   m_frame_timestamp->SetSelection(iframe_timestamp);

   ifont_size=2; ifont_size_def=2;
   config->Read(_T("FpsFontSize"),&ifont_size,ifont_size_def);
   m_fps_font->SetSelection(ifont_size);

   int iperf_level,iperf_level_def(0);
   config->Read(_T("PerformanceLevel"),&iperf_level,iperf_level_def);
   m_perf_level->SetSelection(iperf_level);

   SetInit(true);
}

void SettingsVideoCapture::OnDirButtonClick(wxCommandEvent& WXUNUSED(event))
{
   wxDirDialog dialog(this,_T("Select folder for video capture"),m_folderCtrl->GetValue(),wxDD_NEW_DIR_BUTTON);
   if(dialog.ShowModal() == wxID_OK) {
      m_folderCtrl->SetValue(dialog.GetPath());
      m_folderCtrl->MarkDirty();
   }
}

bool SettingsVideoCapture::OnApply()
{
   wxF()->setCaptureFolder(m_folderCtrl->GetValue(),m_fileNamePrefixCtrl->GetValue());

   wxFactory::FrameCounterSize font_size = wxFactory::CNT_OFF;
   switch(m_frame_font->GetSelection()) {
      case 0:  {font_size = wxFactory::CNT_OFF;     break; }
      case 1:  {font_size = wxFactory::CNT_SMALL;   break; }
      case 2:  {font_size = wxFactory::CNT_MEDIUM;  break; }
      case 3:  {font_size = wxFactory::CNT_LARGE;   break; }
      case 4:  {font_size = wxFactory::CNT_HUGE;    break; }
      default: {font_size = wxFactory::CNT_MEDIUM;}
   };
   wxF()->SetFrameCounterFontSize(font_size);

   wxFactory::FrameTimeStampSize fts_size = wxF()->FrameTimeStampFontSize();
   switch(m_frame_timestamp->GetSelection()) {
      case 0:  {fts_size = wxFactory::FTS_OFF;     break; }
      case 1:  {fts_size = wxFactory::FTS_SMALL;   break; }
      case 2:  {fts_size = wxFactory::FTS_MEDIUM;  break; }
      default: {fts_size = wxFactory::FTS_MEDIUM;}
   };
   wxF()->SetFrameTimeStampFontSize(fts_size);

   //20100703:BM - added new settings
   wxFactory::FrameCounterSize ffont_size = wxFactory::CNT_OFF;
   switch(m_fps_font->GetSelection()) {
      case 0:  {ffont_size = wxFactory::CNT_OFF;     break; }
      case 1:  {ffont_size = wxFactory::CNT_SMALL;   break; }
      case 2:  {ffont_size = wxFactory::CNT_MEDIUM;  break; }
      case 3:  {ffont_size = wxFactory::CNT_LARGE;   break; }
      case 4:  {ffont_size = wxFactory::CNT_HUGE;    break; }
      default: {ffont_size = wxFactory::CNT_MEDIUM;}
   };
   wxF()->SetFpsFontSize(ffont_size);

   //20100703:BM - added new settings
   wxFactory::PerfPollIntervall perf_level = wxFactory::PPL_LOW;
   switch(m_perf_level->GetSelection()) {
      case 0:  {perf_level = wxFactory::PPL_LOW;     break; }
      case 1:  {perf_level = wxFactory::PPL_MID;   break; }
      case 2:  {perf_level = wxFactory::PPL_HIGH;  break; }
      default: {perf_level = wxFactory::PPL_LOW;}
   };
   wxF()->SetPerformanceLevel(perf_level);

   wxConfig* config = wxF()->config();
   config->Write(_T("CaptureFormat"),m_captureChoice->GetSelection());
   config->Write(_T("ContainerFormat"),m_containerChoice->GetSelection());
   config->Write(_T("FrameCounterFontSize"),m_frame_font->GetSelection());
   config->Write(_T("FrameTimeStampSize"),m_frame_timestamp->GetSelection());
   //20100703:BM - added new settings
   config->Write(_T("FpsFontSize"),m_fps_font->GetSelection());
   config->Write(_T("PerformanceLevel"),m_perf_level->GetSelection());
   return true;
}
