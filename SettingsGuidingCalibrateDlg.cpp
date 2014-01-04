#include "SettingsGuidingCalibrateDlg.h"

//(*InternalHeaders(SettingsGuidingCalibrateDlg)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "wxFactory.h"

#define CROSS_HAIR_ICON 1
#include "wxWebcam_icons.h"
#include "wxGuidingConfig.h"

#include "SettingsGuiding.h"
#include "wxWebcam/wxWebcam.h"

//(*IdInit(SettingsGuidingCalibrateDlg)
const long SettingsGuidingCalibrateDlg::ID_STATICTEXT1 = wxNewId();
const long SettingsGuidingCalibrateDlg::ID_STATICLINE1 = wxNewId();
const long SettingsGuidingCalibrateDlg::ID_STATICTEXT2 = wxNewId();
const long SettingsGuidingCalibrateDlg::ID_BUTTON1 = wxNewId();
const long SettingsGuidingCalibrateDlg::ID_STATICLINE2 = wxNewId();
const long SettingsGuidingCalibrateDlg::ID_STATICTEXT4 = wxNewId();
const long SettingsGuidingCalibrateDlg::ID_STATICLINE3 = wxNewId();
const long SettingsGuidingCalibrateDlg::ID_STATICTEXT5 = wxNewId();
const long SettingsGuidingCalibrateDlg::ID_STATICLINE4 = wxNewId();
const long SettingsGuidingCalibrateDlg::ID_STATICTEXT3 = wxNewId();
const long SettingsGuidingCalibrateDlg::ID_TEXTCTRL3 = wxNewId();
const long SettingsGuidingCalibrateDlg::ID_STATICTEXT6 = wxNewId();
const long SettingsGuidingCalibrateDlg::ID_TEXTCTRL1 = wxNewId();
const long SettingsGuidingCalibrateDlg::ID_STATICTEXT7 = wxNewId();
const long SettingsGuidingCalibrateDlg::ID_TEXTCTRL2 = wxNewId();
const long SettingsGuidingCalibrateDlg::ID_STATICLINE5 = wxNewId();
const long SettingsGuidingCalibrateDlg::ID_OK_BUTTON = wxNewId();
const long SettingsGuidingCalibrateDlg::ID_CANCEL_BUTTON = wxNewId();
//*)

static const long ID_CALIBRATE_TIMER = wxNewId();
static const double pi = 4.0*atan(1.0);

BEGIN_EVENT_TABLE(SettingsGuidingCalibrateDlg,wxDialog)
	//(*EventTable(SettingsGuidingCalibrateDlg)
	//*)
	EVT_TIMER(ID_CALIBRATE_TIMER,SettingsGuidingCalibrateDlg::OnGuideTimer)
END_EVENT_TABLE()

SettingsGuidingCalibrateDlg::SettingsGuidingCalibrateDlg(SettingsGuiding* parent,wxWindowID id)
: m_guide_timer(this,ID_CALIBRATE_TIMER)
, m_normal(false)
, m_parent(parent)
{
	//(*Initialize(SettingsGuidingCalibrateDlg)
	wxBoxSizer* BoxSizer6;
	wxButton* PickStarButton;
	wxBoxSizer* BoxSizer5;
	wxButton* ok_button;
	wxBoxSizer* BoxSizer7;
	wxStaticText* StaticText2;
	wxButton* cancel_button;
	wxStaticText* StaticText6;
	wxStaticText* StaticText1;
	wxBoxSizer* BoxSizer2;
	wxStaticText* m_cameraAngle;
	wxStaticLine* StaticLine4;
	wxStaticLine* StaticLine2;
	wxStaticText* StaticText5;
	wxStaticLine* StaticLine3;
	wxStaticLine* StaticLine1;
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;
	wxBoxSizer* BoxSizer3;
	wxStaticText* StaticText4;
	wxStaticLine* StaticLine5;
	
	Create(parent, id, _("Auto Calibrate"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("1) Place a star near the eastern\nedge of the preview."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	BoxSizer2->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer2, 0, wxTOP|wxBOTTOM|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticLine1 = new wxStaticLine(this, ID_STATICLINE1, wxDefaultPosition, wxSize(10,-1), wxLI_HORIZONTAL, _T("ID_STATICLINE1"));
	BoxSizer1->Add(StaticLine1, 1, wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer5 = new wxBoxSizer(wxVERTICAL);
	BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("2) Press the button\nand click the star."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	BoxSizer3->Add(StaticText2, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	PickStarButton = new wxButton(this, ID_BUTTON1, _("Pick Star"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	BoxSizer3->Add(PickStarButton, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer5->Add(BoxSizer3, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer5, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticLine2 = new wxStaticLine(this, ID_STATICLINE2, wxDefaultPosition, wxSize(10,-1), wxLI_HORIZONTAL, _T("ID_STATICLINE2"));
	BoxSizer1->Add(StaticLine2, 1, wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("3) Switch off the RA motor"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	BoxSizer6->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer6, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticLine3 = new wxStaticLine(this, ID_STATICLINE3, wxDefaultPosition, wxSize(10,-1), wxLI_HORIZONTAL, _T("ID_STATICLINE3"));
	BoxSizer1->Add(StaticLine3, 1, wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("4) As the star reaches the other \nedge, or OK is clicked, the rectangle \ndisappears.\n\nRestart RA motor and press \nOK to accept computed values"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	BoxSizer7->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer7, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticLine4 = new wxStaticLine(this, ID_STATICLINE4, wxDefaultPosition, wxSize(10,-1), wxLI_HORIZONTAL, _T("ID_STATICLINE4"));
	BoxSizer1->Add(StaticLine4, 0, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Elapsed time [ms]"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_elapsed_time = new wxTextCtrl(this, ID_TEXTCTRL3, _("0"), wxDefaultPosition, wxSize(60,-1), wxTE_READONLY|wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer1->Add(m_elapsed_time, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Image scale [\"/pixel]"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer1->Add(StaticText6, 1, wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_imageScale = new wxTextCtrl(this, ID_TEXTCTRL1, _("1.0"), wxDefaultPosition, wxSize(60,-1), wxTE_READONLY|wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(m_imageScale, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_cameraAngle = new wxStaticText(this, ID_STATICTEXT7, _("Camera angle [d]"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer1->Add(m_cameraAngle, 1, wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_camera_angle = new wxTextCtrl(this, ID_TEXTCTRL2, _("0.0"), wxDefaultPosition, wxSize(60,-1), wxTE_READONLY|wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer1->Add(m_camera_angle, 0, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(FlexGridSizer1, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticLine5 = new wxStaticLine(this, ID_STATICLINE5, wxDefaultPosition, wxSize(10,-1), wxLI_HORIZONTAL, _T("ID_STATICLINE5"));
	BoxSizer1->Add(StaticLine5, 0, wxTOP|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
	ok_button = new wxButton(this, ID_OK_BUTTON, _("OK"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_OK_BUTTON"));
	BoxSizer8->Add(ok_button, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	cancel_button = new wxButton(this, ID_CANCEL_BUTTON, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CANCEL_BUTTON"));
	BoxSizer8->Add(cancel_button, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer8, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
	
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SettingsGuidingCalibrateDlg::OnPickStarButtonClick);
	Connect(ID_OK_BUTTON,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SettingsGuidingCalibrateDlg::OnOKbuttonClick);
	Connect(ID_CANCEL_BUTTON,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SettingsGuidingCalibrateDlg::OnCancelButtonClick);
	Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&SettingsGuidingCalibrateDlg::OnClose);
	//*)

   int fx(0),fy(0);
   parent->GetScreenPosition(&fx,&fy);
   Move(wxPoint(fx,fy));

   Layout();
}

SettingsGuidingCalibrateDlg::~SettingsGuidingCalibrateDlg()
{
	//(*Destroy(SettingsGuidingCalibrateDlg)
	//*)
}


void SettingsGuidingCalibrateDlg::OnOKbuttonClick(wxCommandEvent& WXUNUSED(event))
{
   DisconnectMouseEvent();

   if(ComputeUpdateCalibration()) {
      m_stop_watch_started = false;
      m_normal = true;
   }

   if(m_normal) {
      m_parent->SetAngleValue(m_camera_angle->GetValue());
      m_parent->SetArcsecValue(m_imageScale->GetValue());
   }
   else {
      wxMessageBox(_T("Calibration was not successful, please allow the guide star to drift off the edge."));
   }

   Destroy();
}


void SettingsGuidingCalibrateDlg::OnCancelButtonClick(wxCommandEvent& WXUNUSED(event))
{
   DisconnectMouseEvent();
   Destroy();
}

void SettingsGuidingCalibrateDlg::OnClose(wxCloseEvent& WXUNUSED(event))
{
   DisconnectMouseEvent();
   Destroy();
}

void SettingsGuidingCalibrateDlg::DisconnectMouseEvent()
{
   m_guide_timer.Stop();
   if(wxWebcam* camera = wxF()->webcam()) {
      // disable the video mouse click event
      camera->Disconnect(camera->GetId(),wxEVT_LEFT_DOWN);
      camera->SetCursor(*wxSTANDARD_CURSOR);
      camera->cancelGuideBox();
   }
}

void SettingsGuidingCalibrateDlg::OnPickStarButtonClick(wxCommandEvent& WXUNUSED(event))
{
   m_guide_timer.Stop();
   m_elapsed_time->SetValue(wxT("0"));
   m_camera_angle->SetValue(wxT("0.0"));
   m_imageScale->SetValue(wxT("1.0"));

   if(wxCamera* camera = wxF()->cam()) {
      camera->Connect(camera->GetId(),wxEVT_LEFT_DOWN,wxMouseEventHandler(SettingsGuidingCalibrateDlg::OnvideoPanelLeftDown),0,this);

#ifdef _WXMSW_
      // make a 32x32 cross hair cursor with centereed hot spot
      wxImage cimage(cross_hair_32x32_xpm);
      cimage.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X,cimage.GetWidth()/2);
      cimage.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y,cimage.GetHeight()/2);
      camera->wxWindow::SetCursor(wxCursor(cimage));
#else
      // using stock cursor on Linux, as the MSW code makes a black crosshair and vice versa!!
      camera->wxWindow::SetCursor(wxCursor(wxCURSOR_BULLSEYE));
#endif
   }
}

void SettingsGuidingCalibrateDlg::OnvideoPanelLeftDown(wxMouseEvent& event)
{
   // mouse event is must be re-enabled by user if needed
   DisconnectMouseEvent();

   wxWebcam* camera =  wxF()->webcam();
   if (!camera) return;

   wxImage* image = camera->liveImage();
   if(!image->IsOk())return;

   int guideBoxSize,bacgroundCutoff,guideInterval;
   double imageScale, cameraAngle;
   wxGuidingConfig::getGuidingConfig(guideBoxSize,bacgroundCutoff,guideInterval,imageScale,cameraAngle);

   // obviously, we must override the parameters that we are supposed to calculate
   cameraAngle = 0.0;
   imageScale  = 1.0;
   m_normal    = false; // will be set to true if the calibration appears successful

   // we also override the guide interval to get sufficient resolution in the calibration
   guideInterval = 200;  // [ms]

   m_guide_subframe.setCameraAngle(cameraAngle);
   m_guide_subframe.setImageScale(imageScale);
   m_guide_subframe.setCutoffLimit(bacgroundCutoff);

   // get position and guide box size
   wxPoint DC_point = event.GetPosition();
   wxSize subsiz(guideBoxSize,guideBoxSize);

   // convert to image coordinates and enable the guide box display
   wxRealPoint imagePoint;
   camera->imagePointfromDC(DC_point,imagePoint);
   camera->setGuideBox(imagePoint,imagePoint,subsiz);

   // init the subframe
   wxSize fullsize(image->GetWidth(),image->GetHeight());
   m_guide_subframe.setFrameInfo(fullsize,subsiz,imagePoint);
   bool inverted = camera->liveImageInverted();

   // lock onto the guidestar
   bool locked = false;
   if(image && m_guide_subframe.copySubFrame(*image,inverted)) {

      // lock on the selected guide star
      wxStar offset_star;
      if(m_guide_subframe.guide(offset_star)) {

         // Initialise the offset position to be the same as the lock position
         // by declaring that the offset is zero
         m_offset_star = wxStar(0.0,0.0,0.0);

         // just a simple timer loop to drive the guiding
         m_guide_timer.Start(guideInterval);
         locked = true;

         // the stopwatch will be started when movement is detected
         m_stop_watch_started = false;

      }
   }

}

bool SettingsGuidingCalibrateDlg::ComputeUpdateCalibration()
{
   if(!m_stop_watch_started)return false;

   // calculate the angle in degrees;
   double dx = m_offset_star.x();
   double dy = m_offset_star.y();
   double angdeg  = atan2(dy,dx) * 180. / pi;

   // modify angle to be in 1st or 4th quadrant
   while(angdeg >  90.0)angdeg -= 180;
   while(angdeg < -90.0)angdeg += 180;

   wxString angle_string;
   angle_string.Printf(wxT("%6.2f"),angdeg);
   m_camera_angle->SetValue(angle_string);

   // calculate the image scale
   double seconds = m_stop_watch.Time()/1000.0;

   // for a full 360 degree rotation, a sidereal day is required, i.e. 86164 s
   // Compute fraction of sidereal day for star to cross FOV
   double fraction  = seconds/86164.;

   // convert the fraction into arc seconds
   double arcsec = 360*60*60*fraction;

   // to get image scale = "/pixel, divide by number of pixels traversed
   double pixels = sqrt(dx*dx + dy*dy);
   if(pixels > 0) {

      // we now have the computed image scale
      double image_scale = arcsec/pixels;

      wxString image_scale_string;
      image_scale_string.Printf(wxT("%6.2f"),image_scale);
      m_imageScale->SetValue(image_scale_string);
   }

   return m_stop_watch_started;
}


void SettingsGuidingCalibrateDlg::OnGuideTimer(wxTimerEvent& WXUNUSED(event))
{
   if (!wxF()->cam()->IsWebcam()) return;

   wxWebcam* camera =  wxF()->webcam();
   wxImage* image = camera->liveImage();
   bool inverted = camera->liveImageInverted();

   // extract guiding subframe from fullframe
   if(image && m_guide_subframe.copySubFrame(*image,inverted)) {

      // compute the star offset
      if(m_guide_subframe.guide(m_offset_star)) {

         if(!m_stop_watch_started) {
            double dx = m_offset_star.x();
            double dy = m_offset_star.y();
            double offset = sqrt(dx*dx + dy*dy);

            // start the stopwatch when the star has moved at least two pixels
            if(offset > 2.0) {
               m_stop_watch.Start();
               m_stop_watch_started = true;
            }
         }

         // show the elapsed time [ms] continuously
         if(m_stop_watch_started)m_elapsed_time->SetValue(wxString::Format(wxT("%d"),m_stop_watch.Time()));

         // update camera guide box overlay
         wxRealPoint curcen = m_guide_subframe.centre();
         wxRealPoint lockpos = m_guide_subframe.lockpos();
         camera->setGuideBox(lockpos,curcen,m_guide_subframe.size());

         // update the calibration parameters
         ComputeUpdateCalibration();
      }
      else {
         // guide star has been lost, so presumably it is off the screen
         // we terminate immediately
         m_guide_timer.Stop();
         m_elapsed_time->SetValue(wxString::Format(wxT("%d"),m_stop_watch.Time()));

         if(ComputeUpdateCalibration()) {
            m_stop_watch_started = false;
            m_normal = true;
         }
         camera->cancelGuideBox();
      }
   }
}
