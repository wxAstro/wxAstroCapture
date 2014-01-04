#include "SettingsGuidePortParallel.h"

//(*InternalHeaders(SettingsGuidePortParallel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "wxFactory.h"
#include "wxIO/wxGuiderPort.h"

//(*IdInit(SettingsGuidePortParallel)
const long SettingsGuidePortParallel::ID_STATICTEXT1 = wxNewId();
const long SettingsGuidePortParallel::ID_STATICTEXT7 = wxNewId();
const long SettingsGuidePortParallel::ID_STATICTEXT6 = wxNewId();
const long SettingsGuidePortParallel::ID_CUSTOM3 = wxNewId();
const long SettingsGuidePortParallel::ID_STATICTEXT3 = wxNewId();
const long SettingsGuidePortParallel::ID_CUSTOM4 = wxNewId();
const long SettingsGuidePortParallel::ID_STATICTEXT4 = wxNewId();
const long SettingsGuidePortParallel::ID_CUSTOM5 = wxNewId();
const long SettingsGuidePortParallel::ID_STATICTEXT2 = wxNewId();
const long SettingsGuidePortParallel::ID_CUSTOM6 = wxNewId();
const long SettingsGuidePortParallel::ID_STATICTEXT9 = wxNewId();
const long SettingsGuidePortParallel::ID_CUSTOM7 = wxNewId();
const long SettingsGuidePortParallel::ID_STATICTEXT5 = wxNewId();
const long SettingsGuidePortParallel::ID_CUSTOM1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SettingsGuidePortParallel,wxPanel)
	//(*EventTable(SettingsGuidePortParallel)
	//*)
END_EVENT_TABLE()

SettingsGuidePortParallel::SettingsGuidePortParallel(wxWindow* parent,wxWindowID id)
{
	//(*Initialize(SettingsGuidePortParallel)
	wxStaticText* StaticText9;
	wxStaticText* StaticText2;
	wxStaticText* StaticText6;
	wxStaticText* StaticText1;
	wxStaticText* StaticText3;
	wxStaticText* StaticText5;
	wxStaticText* StaticText4;
	
	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 3, 0, 0);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT1, _("Port pins"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("D7 D0"), wxDefaultPosition, wxSize(105,-1), wxALIGN_RIGHT, _T("ID_STATICTEXT7"));
	FlexGridSizer1->Add(StaticText7, 2, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(20,10,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("GX Mask"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, _T("ID_STATICTEXT6"));
	FlexGridSizer1->Add(StaticText6, 1, wxBOTTOM|wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 4);
	GX_mask = new wxIoPortCtrl(this,ID_CUSTOM3,wxDefaultPosition,wxDefaultSize,wxEXPAND,_T("ID_CUSTOM3"));
	FlexGridSizer1->Add(GX_mask, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("GX Init"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxBOTTOM|wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 4);
	GX_init = new wxIoPortCtrl(this,ID_CUSTOM4,wxDefaultPosition,wxDefaultSize,wxEXPAND,_T("ID_CUSTOM4"));
	FlexGridSizer1->Add(GX_init, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("GX RA pos"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxBOTTOM|wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 4);
	GX_RA_pos = new wxIoPortCtrl(this,ID_CUSTOM5,wxDefaultPosition,wxDefaultSize,wxEXPAND,_T("ID_CUSTOM5"));
	FlexGridSizer1->Add(GX_RA_pos, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT2, _("GX RA neg"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText1, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 4);
	GX_RA_neg = new wxIoPortCtrl(this,ID_CUSTOM6,wxDefaultPosition,wxDefaultSize,wxEXPAND,_T("ID_CUSTOM6"));
	FlexGridSizer1->Add(GX_RA_neg, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText9 = new wxStaticText(this, ID_STATICTEXT9, _("GX DEC pos"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
	FlexGridSizer1->Add(StaticText9, 1, wxBOTTOM|wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 4);
	GX_DEC_pos = new wxIoPortCtrl(this,ID_CUSTOM7,wxDefaultPosition,wxDefaultSize,wxEXPAND,_T("ID_CUSTOM7"));
	FlexGridSizer1->Add(GX_DEC_pos, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("GX DEC neg"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer1->Add(StaticText5, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 4);
	GX_DEC_neg = new wxIoPortCtrl(this,ID_CUSTOM1,wxDefaultPosition,wxDefaultSize,wxEXPAND,_T("ID_CUSTOM1"));
	FlexGridSizer1->Add(GX_DEC_neg, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)

	Init();
}

SettingsGuidePortParallel::~SettingsGuidePortParallel()
{
	//(*Destroy(SettingsGuidePortParallel)
	//*)
}

void SettingsGuidePortParallel::Init()
{

   // just make a guide port object on the stack
   // to ensure we don't change the one in the factory untill user presses apply

   wxGuiderPort gport;
   // set some defaults
   gport.SetGXmask(wxGuiderPort::GXmask_default());       // others will not be affected
   gport.SetGXinit(wxGuiderPort::GXinit_default());       // init GX - draw all high
   gport.SetGXRA_pos(wxGuiderPort::GXRA_pos_default());       // RA+
   gport.SetGXRA_neg(wxGuiderPort::GXRA_neg_default());       // RA-
   gport.SetGXDEC_pos(wxGuiderPort::GXDEC_pos_default());      // Dec+
   gport.SetGXDEC_neg(wxGuiderPort::GXDEC_neg_default());      // Dec-

      // skip if not port interfac active
   if(wxGuider* guider = wxF()->guider()) {
      if(guider->interface_type() == wxGuider::PORT_INTERFACE) {
         // if we already have a parallel guide port, use its values as defaults
         wxGuiderPort* gport_f = wxF()->guidePort();
         if(gport_f && gport_f->IsPortParallel()) {
            gport.SetGXmask(gport_f->GXmask());       // others will not be affected
            gport.SetGXinit(gport_f->GXinit());       // init GX - draw all high
            gport.SetGXRA_pos(gport_f->GXRA_pos());       // RA+
            gport.SetGXRA_neg(gport_f->GXRA_neg());       // RA-
            gport.SetGXDEC_pos(gport_f->GXDEC_pos());      // Dec+
            gport.SetGXDEC_neg(gport_f->GXDEC_neg());      // Dec-
            gport.SetInvertedLogic(gport_f->InvertedLogic());         // set (parallel) logic inverted
         }
      }
   }

   // fill the controls
   StaticText7->SetLabel(_("D7<----------->D0"));
   GX_mask->CreatePins(8,gport.GXmask());
   GX_init->CreatePins(8,gport.GXinit());
   GX_RA_pos->CreatePins(8,gport.GXRA_pos());
   GX_RA_neg->CreatePins(8,gport.GXRA_neg());
   GX_DEC_pos->CreatePins(8,gport.GXDEC_pos());
   GX_DEC_neg->CreatePins(8,gport.GXDEC_neg());

   FlexGridSizer1->Layout();
   Layout();
}

unsigned char SettingsGuidePortParallel::GXmask() const  { return GX_mask->GetPinsByte(); }
unsigned char SettingsGuidePortParallel::GXinit() const  { return GX_init->GetPinsByte(); }       // init GX
unsigned char SettingsGuidePortParallel::GXRA_pos() const  { return GX_RA_pos->GetPinsByte(); }       // RA+
unsigned char SettingsGuidePortParallel::GXRA_neg() const  { return GX_RA_neg->GetPinsByte(); }       // RA-
unsigned char SettingsGuidePortParallel::GXDEC_pos() const { return GX_DEC_pos->GetPinsByte(); }      // Dec+
unsigned char SettingsGuidePortParallel::GXDEC_neg() const { return GX_DEC_neg->GetPinsByte(); }      // Dec-

