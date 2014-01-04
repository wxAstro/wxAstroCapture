#include "wxMainToolBar.h"
#include "wxthings/spinctld.h"
#include <wx/choice.h>
#include <wx/stattext.h>
#include <wx/artprov.h>
#include "wxFactory.h"
#include "wxWebcam/wxWebcam.h"
#include "wxWebcam/wxArtcam.h"

#define TOOLBAR_ICONS 1
#include "wxWebcam_icons.h"

const long wxMainToolBar::ID_OBJECT= wxNewId();
const long wxMainToolBar::ID_TYPE_CHOICE= wxNewId();
const long wxMainToolBar::ID_ZOOM_SPIN= wxNewId();
const long wxMainToolBar::ID_UNZOOM= wxNewId();
const long wxMainToolBar::ID_BIN_CHOICE = wxNewId();
const long wxMainToolBar::ID_FULLFRAME = wxNewId();
const long wxMainToolBar::ID_SET_TEMP = wxNewId();
const long wxMainToolBar::ID_WARMUP = wxNewId();
const long wxMainToolBar::ID_RESTART = wxNewId();

BEGIN_EVENT_TABLE(wxMainToolBar,wxToolBar)
   EVT_CHOICE(ID_BIN_CHOICE,wxMainToolBar::OnBinChoiceSelect)
   EVT_CHOICE(ID_TYPE_CHOICE,wxMainToolBar::OnTypeChoiceSelect)
   EVT_SPINCTRL(ID_ZOOM_SPIN,wxMainToolBar::OnZoomSpinUpdated)
   EVT_TOOL(ID_UNZOOM,wxMainToolBar::OnUnZoom)
   EVT_TOOL(ID_FULLFRAME,wxMainToolBar::OnFullFrame)
   EVT_TOOL(ID_OBJECT,wxMainToolBar::OnObject)
   EVT_TOOL(ID_SET_TEMP,wxMainToolBar::OnSetTemp)
   EVT_TOOL(ID_WARMUP,wxMainToolBar::OnWarmUp)
   EVT_TOOL(ID_RESTART,wxMainToolBar::OnRestartExposure)
END_EVENT_TABLE()

wxMainToolBar* wxMainToolBar::m_self = 0;

#if wxUSE_UNICODE
  static const wxString Celsius = _T("\x2103");
#else
  static const wxString Celsius = _T("°C");
#endif

wxMainToolBar::wxMainToolBar(wxWindow* parent,
                             wxWindowID id,
                             const wxPoint& pos,
                             const wxSize& size,
                             long style,
                             const wxString& name)
: wxToolBar(parent,id,pos,size,style,name)
, m_first_dpos(0)
, m_binChoice(0)
, m_zoomSpin(0)
, m_typeChoice(0)
, m_object_text(0)
, m_temp_text(0)
, m_cstat(ECS_Off)
{
 	m_self = this;

   InitToolBar();
}



void wxMainToolBar::InitToolBar()
{
   // set up the toolbar controls
   int ipos = 0;
   if(m_first_dpos == 0) {
      ipos += InsertObject(ipos);
      ipos += InsertType(ipos);
      ipos += InsertBin(ipos); // we need to be able to set binning always
      m_first_dpos = ipos;
   }

   if(m_first_dpos > 0) {
      int ntools = GetToolsCount();
      size_t max_pos = ntools-1;

      // Delete the camera dependent tools
      for(size_t ipos=max_pos; ipos>m_first_dpos; ipos--) {
         DeleteToolByPos(ipos);
      }

      // clear references to the deleted tools
      m_zoomSpin = 0;
      m_temp_text = 0;
   }

   wxCamera* camera = wxF()->cam();
   if(camera && camera->IsArtcam()) {
      ipos = m_first_dpos;
      ipos += InsertZoom(ipos);
      ipos += InsertSubframe(ipos);
      ipos += InsertRestartExposure(ipos);
      ipos += InsertCooling(ipos);
   }
   //20080525:BM try to support more webcam
   else if(camera && camera->IsWebcam()) {
      ipos = m_first_dpos;
      ipos += InsertZoom(ipos);
      ipos += InsertSubframe(ipos);
   }

	Realize();

	CamEnable();
}

void wxMainToolBar::CamEnable()
{
   wxCamera* cam = wxF()->cam();

   if(cam && cam->IsArtcam()) {
      EnableBin(true);
      EnableZoom(true);
      EnableSubframe(true);
   }
   else if(cam && cam->IsWebcam()) {
      wxGuider* guider = wxF()->guider();
      //20080525:BM try to support more webcam (make all true instead of false)
      EnableBin(false);
      EnableZoom(!guider->guiding_active());  //20080825:CA Zoomed guiding not supported
      EnableSubframe(!guider->guiding_active());//20080826:CA Subframed guiding not supported
   }
   else if (cam && cam->IsConnected()) {
      EnableBin(false);
      EnableZoom(false);
      EnableSubframe(false);
   }
   else {
      EnableBin(true);
      EnableZoom(true);
      EnableSubframe(true);
  }
}

void wxMainToolBar::UpdateToolBar()
{
   UpdateTemp();
   // UpdateObject();
   UpdateZoom();
   // UpdateType();

}

wxMainToolBar::~wxMainToolBar()
{
   //dtor
}


int wxMainToolBar::InsertObject(int pos)
{
   wxString object;
   wxF()->getFitsHeaderValue(wxT("OBJECT"),object);
   if(object == wxT("")) {
      object = wxT("Object");
      wxF()->saveFitsHeaderValue(wxT("OBJECT"),object);
   }

   int ipos = pos;
	m_object_text = new wxStaticText(this, wxNewId(), object, wxDefaultPosition, wxDefaultSize, 0);

   // separator, leading text
   InsertSeparator(ipos++);
   InsertControl(ipos++,m_object_text);
	InsertTool(ipos++,ID_OBJECT, _("Object"),wxBitmap(&star_16x16_blue_xpm[0]) , wxNullBitmap, wxITEM_NORMAL, _("Object"), wxEmptyString);
	return (ipos-pos);
}


int wxMainToolBar::InsertType(int pos)
{
	wxStaticText* tool_text = new wxStaticText(this, wxNewId(), _("Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
   m_typeChoice = new wxChoice(this, ID_TYPE_CHOICE, wxDefaultPosition, wxSize(70,-1), 0, 0, 0, wxDefaultValidator, _T("Frame Type"));
   m_typeChoice->Append(_("LIGHT"));
   m_typeChoice->Append(_("DARK"));
   m_typeChoice->Append(_("BIAS"));
   m_typeChoice->Append(_("FLAT"));

   wxString imgtyp;
   wxF()->getFitsHeaderValue(wxT("IMAGETYP"),imgtyp);
   if(wxNOT_FOUND == m_typeChoice->FindString(imgtyp)) {
      // illegal value, set to LIGHT
      imgtyp = _("LIGHT");
      wxF()->saveFitsHeaderValue(wxT("IMAGETYP"),imgtyp);
   }

   m_typeChoice->SetStringSelection(imgtyp);

   int ipos = pos;

   // Separator, leading text + binning choice box
   InsertSeparator(ipos++);
   InsertControl(ipos++,tool_text);
   InsertControl(ipos++,m_typeChoice);
   SetToolShortHelp(ID_TYPE_CHOICE,_("Frame type"));
   Realize();

   return (ipos-pos);
}


int wxMainToolBar::InsertZoom(int pos)
{
	wxStaticText* tool_text = new wxStaticText(this, wxNewId(), _("Zoom"), wxDefaultPosition, wxDefaultSize, 0);
  	m_zoomSpin = new wxSpinCtrlDbl(this, ID_ZOOM_SPIN, _T("1.0"), wxDefaultPosition,wxSize(60,-1), 0, 0, 100, 0, _("Zoom"));

   m_zoomSpin->SetRange(0.01,5.0);
   m_zoomSpin->SetValue(1.0);
   m_zoomSpin->SetIncrement(0.1);
   m_zoomSpin->SetFormat(_T("%.3lf"));

   int ipos = pos;

   // Separator, leading text + spinctrl
   InsertSeparator(ipos++);
   InsertControl(ipos++,tool_text);
   InsertControl(ipos++,m_zoomSpin);
   InsertTool(ipos++,ID_UNZOOM, _("Undo Zoom"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_UNDO")),wxART_TOOLBAR,wxSize(15,16)), wxNullBitmap, wxITEM_NORMAL, _("Undo Zoom"), wxEmptyString);
   SetToolShortHelp(ID_ZOOM_SPIN,_("Zoom factor"));
   Realize();

   return (ipos-pos);
}

int wxMainToolBar::InsertBin(int pos)
{
	wxStaticText* tool_text = new wxStaticText(this, wxNewId(), _("Bin"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
   m_binChoice = new wxChoice(this, ID_BIN_CHOICE, wxDefaultPosition, wxSize(60,-1), 0, 0, 0, wxDefaultValidator, _T("Binning"));
   m_binChoice->Append(_("1:1"));
   m_binChoice->Append(_("2:2"));
   m_binChoice->Append(_("3:3"));
   m_binChoice->Append(_("4:4"));
   m_binChoice->Append(_("5:5"));
   m_binChoice->Append(_("6:6"));
   m_binChoice->SetSelection(wxArtcam::DefaultBinning()-1);

   int ipos = pos;

   // Separator, leading text + binning choice box
   InsertSeparator(ipos++);
   InsertControl(ipos++,tool_text);
   InsertControl(ipos++,m_binChoice);
   SetToolShortHelp(ID_BIN_CHOICE,_("Binning level"));
   Realize();

   return (ipos-pos);
}

int wxMainToolBar::InsertSubframe(int pos)
{
   int ipos = pos;
	wxStaticText* tool_text = new wxStaticText(this, wxNewId(), _("Fullframe"), wxDefaultPosition, wxDefaultSize, 0);

   // separator, leading text
   InsertSeparator(ipos++);
   InsertControl(ipos++,tool_text);
	InsertTool(ipos++,ID_FULLFRAME, _("Reset Fullframe"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_UNDO")),wxART_TOOLBAR,wxSize(15,16)), wxNullBitmap, wxITEM_NORMAL, _("Reset Fullframe"), wxEmptyString);
	return (ipos-pos);
}

int wxMainToolBar::InsertCooling(int pos)
{
   if(wxArtcam* camera = wxF()->artcam()) {

      if(!camera->HasCooling())return 0;

      int ipos = pos;
      m_temp_text = new wxStaticText(this, wxNewId(), _("Temp"), wxDefaultPosition, wxDefaultSize, 0);

      // separator, leading text
      InsertSeparator(ipos++);
      InsertControl(ipos++,m_temp_text);
      InsertTool(ipos++,ID_SET_TEMP, _("Set Temperature"), wxBitmap(&temp_red_15x16_xpm[0]) , wxNullBitmap, wxITEM_NORMAL, _("Set Temperature"), wxEmptyString);
/*
      if(camera->HasWarmup()) {
         InsertTool(ipos++,ID_WARMUP, _("Warm up"),  wxBitmap(&heat_15x16_xpm[0]) , wxNullBitmap, wxITEM_NORMAL, _("Warm up"), wxEmptyString);
      }
*/
      return (ipos-pos);
   }
   return 0;
}

int wxMainToolBar::InsertRestartExposure(int pos)
{
   int ipos = pos;
	wxStaticText* tool_text = new wxStaticText(this, wxNewId(), _("Restart"), wxDefaultPosition, wxDefaultSize, 0);

   // separator, leading text
   InsertSeparator(ipos++);
   InsertControl(ipos++,tool_text);
//	InsertTool(ipos++,ID_RESTART, _("Cancel"), wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_CROSS_MARK")),wxART_TOOLBAR), wxNullBitmap, wxITEM_NORMAL, _("Cancel exposure"), wxEmptyString);
	InsertTool(ipos++,ID_RESTART, _("Restart"), wxBitmap(&restart_15x16_xpm[0]), wxNullBitmap, wxITEM_NORMAL, _("Restart exposure"), wxEmptyString);
	return (ipos-pos);
}




void wxMainToolBar::EnableBin(bool enable)
{
   if(wxControl* control = FindControl(ID_BIN_CHOICE)) control->Enable(enable);
}

void wxMainToolBar::EnableZoom(bool enable)
{
   if(wxControl* control = FindControl(ID_ZOOM_SPIN )) control->Enable(enable);
   if(wxToolBarToolBase* tool = FindById(ID_UNZOOM)) tool->Enable(enable);
}

void wxMainToolBar::EnableSubframe(bool enable)
{
   if(wxToolBarToolBase* tool = FindById(ID_FULLFRAME)) tool->Enable(enable);
}


void wxMainToolBar::OnTypeChoiceSelect(wxCommandEvent& WXUNUSED(event))
{
   wxPropGrid::Entries entries;
   wxF()->getFitsHeaderEntries(entries);
   wxPropGrid::Entries::iterator i = entries.begin();
   while(i != entries.end()) {
      if(wxT("IMAGETYP") == i->keyword) {
         i->value = m_typeChoice->GetString(m_typeChoice->GetSelection());
         wxF()->saveFitsHeaderEntries(entries);
         return;
      }
      i++;
   }
}

void wxMainToolBar::UpdateType()
{
   wxString imgtyp;
   if(wxF()->getFitsHeaderValue(wxT("IMAGETYP"),imgtyp)) {
      m_typeChoice->SetStringSelection(imgtyp);
   }
}


void wxMainToolBar::OnBinChoiceSelect(wxCommandEvent& WXUNUSED(event))
{
   if(m_binChoice) {
      unsigned short bin = m_binChoice->GetSelection()+1;
      if(wxArtcam* camera = wxF()->artcam()) {
         camera->SetBinning(bin);
      }
      else {
         wxArtcam::SetDefaultBinning(bin);
      }
   }
}

void wxMainToolBar::OnZoomSpinUpdated(wxSpinEvent& WXUNUSED(event))
{
   if(m_zoomSpin) {
      wxCamera* camera = wxF()->cam();
      if(camera && camera->IsArtcam()) {
         double  factor = m_zoomSpin->GetValue();
         if(wxArtcam* camera = wxF()->artcam()) {
            camera->ZoomRedraw(factor);
         }
      }
      else if(camera && camera->IsWebcam()) {
         double  factor = m_zoomSpin->GetValue();
         if(wxWebcam* camera = wxF()->webcam()) {
            camera->ZoomRedraw(factor);
         }
      }
   }
}

void wxMainToolBar::OnUnZoom(wxCommandEvent& WXUNUSED(event))
{
   if(m_zoomSpin) {
      if(wxCamera* camera = wxF()->cam()) {
         camera->unZoom();
         m_zoomSpin->SetValue(camera->zoomFactor());
      }
   }
}

void wxMainToolBar::OnFullFrame(wxCommandEvent& WXUNUSED(event))
{
   if(wxArtcam* camera = wxF()->artcam()) {
      camera->SetFullFrame();
   }
   else if(wxWebcam* camera = wxF()->webcam()) {
      camera->SetFullFrame();
   }
}

void  wxMainToolBar::OnObject(wxCommandEvent& WXUNUSED(event))
{
   wxString message = wxT("Object Name/Id");
   wxString caption = wxT("FITS header OBJECT Id");
   wxString object = wxT("Object");
   wxF()->getFitsHeaderValue(wxT("OBJECT"),object);
   wxTextEntryDialog dlg(this, message,caption,object);
   if(dlg.ShowModal() == wxID_OK) {
      object = dlg.GetValue();

      // remove leading and/or trailing blanks
      object.Trim(true);
      object.Trim(false);
      wxF()->saveFitsHeaderValue(wxT("OBJECT"),object);

      UpdateObject();
   }
}

void wxMainToolBar::OnSetTemp(wxCommandEvent& WXUNUSED(event))
{
   if(wxArtcam* camera = wxF()->artcam()) {
      wxString message = wxT("Set camera target temperature " + Celsius);
      wxString caption = wxT("Camera target temperature in" + Celsius);
      wxTextEntryDialog dlg(this, message,caption,wxString::Format(_T("%5.1lf"),camera->CoolingSetpoint() ));
      if(dlg.ShowModal() == wxID_OK) {
         wxString text_temp = dlg.GetValue();

         // remove leading and/or trailing blanks
         text_temp.Trim(true);
         text_temp.Trim(false);

         double temp(0.0);
         if(text_temp.ToDouble(&temp)) {
            camera->SetTemperature(temp);
         }
         else wxMessageBox(text_temp + wxT(" is not a number."));

         UpdateTemp();
      }
   }
}

void wxMainToolBar::OnWarmUp(wxCommandEvent&  WXUNUSED(event))
{
   if(wxCamera* camera = wxF()->cam()) {
      if(camera->HasCooling()) {
         if(wxYES == wxMessageBox(wxT("Start warm-up procedure?"),wxT("Camera Warm-up"),wxYES_NO)) {
            camera->SetWarmup();
         }
      }
   }
}

void wxMainToolBar::UpdateZoom()
{
   if(m_zoomSpin) {
      if(wxCamera* camera = wxF()->cam()) {
         double camzoom = camera->zoomFactor();
         double ctrlzoom = m_zoomSpin->GetValue();
         double diff = fabs(camzoom-ctrlzoom);

         // update the zoom value if significantly changed
         if(diff > 0.001) {
            m_zoomSpin->SetValue(camzoom);
         }
      }
   }
}

void wxMainToolBar::UpdateObject()
{
   wxString object = wxT("Object");
   wxF()->getFitsHeaderValue(wxT("OBJECT"),object);
   if(object == wxT(""))object = wxT("Object");

   // update the object text if it has changed
   if(m_object_text->GetLabel() != object) {
      m_object_text->SetLabel(object);
      Realize();
   }
}

void wxMainToolBar::UpdateTemp()
{
   if(m_temp_text) {
      if(wxCamera* camera = wxF()->cam()) {
         if(camera->HasCooling()) {
            bool do_realize = false;

            // update the temperature text if changed
            wxString temp_txt =  wxString::Format(_T("%5.1lf")+Celsius,camera->Temperature() );
            wxString cur_txt = m_temp_text->GetLabel();
            if(temp_txt != cur_txt) {
               m_temp_text->SetLabel(temp_txt);
               do_realize = true;
            }

            // update the temperature button icon if status has changed
            const char** bitmap = 0;
            ECoolingStatus cstat = camera->CoolingStatus();
            if(m_cstat != cstat) {
               switch(cstat) {
                  case ECS_Off:     { bitmap = &temp_red_15x16_xpm[0]; break; }
                  case ECS_Cooling: { bitmap = &temp_yellow_15x16_xpm[0];  break; } // Red means target not reached
                  case ECS_Holding: { bitmap = &temp_blue_15x16_xpm[0];  break; }   // Blue
                  case ECS_Warmup:  { bitmap = &heat_15x16_xpm[0];      break; }
                  default:          { bitmap = &temp_red_15x16_xpm[0];        }     // Red (should not happen..)
               };
               SetToolNormalBitmap(ID_SET_TEMP,wxBitmap(bitmap));
               m_cstat = cstat;
               do_realize = true;
            }

            if(do_realize)Realize();
         }
      }
   }
}

void wxMainToolBar::OnRestartExposure(wxCommandEvent& WXUNUSED(event))
{
   wxCamera* camera = wxF()->cam();
   if(camera && camera->IsArtcam()) {
      camera->RestartExposure();
   }
}
