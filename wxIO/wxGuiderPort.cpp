

#include "wxGuiderPort.h"
#include "../wxConfigPath.h"

#include "../wxFactory.h"

#include "GP_ST4.h"
#include "GP_MTS_3SDI.h"
#include "GP_NexStar.h"
#include "GP_LX200.h"
#include "GP_LX200PG.h"


/// wxGuiderPort

// Defaults for Standard Guiding parallel port
// using lower 4 bits -
// bit 0 = LE Frame A (1)
// bit 1 = LE Frame B (2)
// bit 2 = Amp toggle (4)
// bit 3 = Shutter if not hardwired (8)
static const wxUint8 _GXP_mask    = 0xf0;      // others will not be affected
static const wxUint8 _GXP_init    = 0x00;      // init GX - draw all lo
static const wxUint8 _GXPRA_pos   = 0x10;
static const wxUint8 _GXPRA_neg   = 0x20;
static const wxUint8 _GXPDEC_pos  = 0x40;
static const wxUint8 _GXPDEC_neg  = 0x80;
static const bool _GXP_inverted         = false;    // parallel logic is usually straight - Guide = High Signal
static const unsigned _GXP_handshake    = 0;        // don't used but must have a default
static const unsigned _GXP_baudrate     = 0;        // don't used but must have a default

// Serial defaults
static const wxUint8 _GXS_mask    = 0;        // others will not be affected
static const wxUint8 _GXS_init    = 0;
static const wxUint8 _GXSRA_pos   = 0;
static const wxUint8 _GXSRA_neg   = 0;
static const wxUint8 _GXSDEC_pos  = 0;
static const wxUint8 _GXSDEC_neg  = 0;
static const bool _GXS_inverted         = false;    // don't used but must have a default
static const unsigned _GXS_handshake    = 1;        // Xon Xoff
static const unsigned _GXS_baudrate     = 3;        // 9600

// GPUSB defaults  //BM:20081226 added
static const wxUint8 _GXGP_mask    = 0xf;      // others will not be affected
static const wxUint8 _GXGP_init    = 0x0;      // init GX - draw all lo translates to deasserted
static const wxUint8 _GXGPRA_neg   = 0x1;
static const wxUint8 _GXGPRA_pos   = 0x2;
static const wxUint8 _GXGPDEC_neg  = 0x4;
static const wxUint8 _GXGPDEC_pos  = 0x8;
static const bool _GXGP_inverted         = false;    // parallel logic is usually straight - Guide = High Signal
static const unsigned _GXGP_handshake    = 0;        // don't used but must have a default
static const unsigned _GXGP_baudrate     = 0;        // don't used but must have a default


static const wxString _EmptyString = wxString(wxT(""));


wxGuiderPort::wxGuiderPort()
: m_pGuider(new wxGuiderPortProtocol()) // alloc None
, m_GX_mask(_GXP_mask)
, m_GX_init(_GXP_init)
, m_GXRA_pos(_GXPRA_pos)
, m_GXRA_neg(_GXPRA_neg)
, m_GXDEC_pos(_GXPDEC_pos)
, m_GXDEC_neg(_GXPDEC_neg)
, m_invertedLogic(_GXP_inverted)
, m_handshake((EPGHandshake)_GXP_handshake)
, m_baudrate((EPGBaudRate)_GXP_baudrate)
{
}

wxGuiderPort::~wxGuiderPort()
{
   if (m_pGuider) delete m_pGuider;
   if ( m_ioPortHandle.PortValid() )
      wxF()->portManager()->PClose(m_ioPortHandle);
}

bool wxGuiderPort::IsInitialised() const
{
    return m_ioPortHandle.PortValid();
}

// some straight forward things (could be inline to optimize)
void wxGuiderPort::SetGXmask(wxUint8 pVal)         {m_GX_mask = pVal;}
void wxGuiderPort::SetGXinit(wxUint8 pVal)         {m_GX_init = pVal;}
void wxGuiderPort::SetGXRA_pos(wxUint8 pVal)         {m_GXRA_pos = pVal;}
void wxGuiderPort::SetGXRA_neg(wxUint8 pVal)         {m_GXRA_neg = pVal;}
void wxGuiderPort::SetGXDEC_pos(wxUint8 pVal)        {m_GXDEC_pos = pVal;}
void wxGuiderPort::SetGXDEC_neg(wxUint8 pVal)        {m_GXDEC_neg = pVal;}
void wxGuiderPort::SetInvertedLogic(bool pVal)           {m_invertedLogic = pVal;}
void wxGuiderPort::SetSerialHandshake(EPGHandshake pVal) {m_handshake = pVal;};
void wxGuiderPort::SetSerialBaudrate(EPGBaudRate pVal)   {
	m_baudrate = pVal;
};


wxUint8 wxGuiderPort::GXmask() const     {return m_GX_mask;}
wxUint8 wxGuiderPort::GXinit() const     {return m_GX_init;}
wxUint8 wxGuiderPort::GXRA_pos() const     {return m_GXRA_pos;}
wxUint8 wxGuiderPort::GXRA_neg() const     {return m_GXRA_neg;}
wxUint8 wxGuiderPort::GXDEC_pos() const    {return m_GXDEC_pos;}
wxUint8 wxGuiderPort::GXDEC_neg() const    {return m_GXDEC_neg;}
bool wxGuiderPort::InvertedLogic() const       {return m_invertedLogic;}
wxGuiderPort::EPGHandshake wxGuiderPort::SerialHandshake() const {return m_handshake;};
wxGuiderPort::EPGBaudRate wxGuiderPort::SerialBaudrate() const   {return m_baudrate;};

// Default values sometimes needed by GUI
wxUint8 wxGuiderPort::GXmask_default()    { return _GXP_mask; }
wxUint8 wxGuiderPort::GXinit_default()    { return _GXP_init; }
wxUint8 wxGuiderPort::GXRA_pos_default()  { return _GXPRA_pos;}
wxUint8 wxGuiderPort::GXRA_neg_default()  { return _GXPRA_neg;}
wxUint8 wxGuiderPort::GXDEC_pos_default() { return _GXPDEC_pos;}
wxUint8 wxGuiderPort::GXDEC_neg_default() { return _GXPDEC_neg;}

const wxString& wxGuiderPort::PortName()     {return m_ioPortHandle.PortName();}
const wxString& wxGuiderPort::ProtocolName() {return wxGuiderPortProtocol::ProtocolName(m_pGuider->Protocol());}

bool wxGuiderPort::IsPortParallel() const      {return m_ioPortHandle.IsPortParallel();}
bool wxGuiderPort::IsPortSerial() const        {return m_ioPortHandle.IsPortSerial();}
bool wxGuiderPort::IsPortGPUSB() const        {return m_ioPortHandle.IsPortGPUSB();} //BM:20081226 added
bool wxGuiderPort::IsPortDummy() const         {return m_ioPortHandle.IsPortDummy();}

bool wxGuiderPort::CommitConfiguration()
{
   if ( ! IsInitialised()) return false; // cannot use the port

   wxPortManager* pm = wxF()->portManager();
   bool retVal = pm->PSetMask(m_ioPortHandle, m_GX_mask); // set the bits used for GX
   if (IsPortSerial()) {
      retVal &= pm->PSetHandshake(m_ioPortHandle, (wxPort::EHandshake)m_handshake);
      retVal &= pm->PSetBaudrate(m_ioPortHandle, (wxPort::EBaudRate)m_baudrate);
   }

   // Any release of the guide port is managed bu the application
   // directly, therefore the statement below is commented out

//   pm->PRelease(m_ioPortHandle); ///@@@ Don't hold it after config
   return retVal;
}

void wxGuiderPort::SetGXDefault()
{
   SetDefaultParallel(); // default to have something set
   if ( m_ioPortHandle.IsPortSerial() ) SetDefaultSerial();
   if ( m_ioPortHandle.IsPortGPUSB() ) SetDefaultGPUSB();  //BM:20081226 added
}


void wxGuiderPort::SetDefaultSerial()
{
   m_GX_mask = _GXS_mask;
   m_GX_init = _GXS_init;
   m_GXRA_pos = _GXSRA_pos;
   m_GXRA_neg = _GXSRA_neg;
   m_GXDEC_pos = _GXSDEC_pos;
   m_GXDEC_neg = _GXSDEC_neg;
   m_invertedLogic = _GXS_inverted;
   m_handshake = (EPGHandshake)_GXS_handshake;
   m_baudrate = (EPGBaudRate)_GXS_baudrate;
}

void wxGuiderPort::SetDefaultParallel()
{
   m_GX_mask = _GXP_mask;
   m_GX_init = _GXP_init;
   m_GXRA_pos = _GXPRA_pos;
   m_GXRA_neg = _GXPRA_neg;
   m_GXDEC_pos = _GXPDEC_pos;
   m_GXDEC_neg = _GXPDEC_neg;
   m_invertedLogic = _GXP_inverted;
   m_handshake = (EPGHandshake)_GXP_handshake;
   m_baudrate = (EPGBaudRate)_GXP_baudrate;
}

//BM:20081226 added
void wxGuiderPort::SetDefaultGPUSB()
{
   m_GX_mask = _GXGP_mask;
   m_GX_init = _GXGP_init;
   m_GXRA_pos = _GXGPRA_pos;
   m_GXRA_neg = _GXGPRA_neg;
   m_GXDEC_pos = _GXGPDEC_pos;
   m_GXDEC_neg = _GXGPDEC_neg;
   m_invertedLogic = _GXGP_inverted;
   m_handshake = (EPGHandshake)_GXGP_handshake;
   m_baudrate = (EPGBaudRate)_GXGP_baudrate;
}

void wxGuiderPort::ReleasePort()
{
   if ( ! m_ioPortHandle.PortValid()) return;

   wxF()->portManager()->PRelease(m_ioPortHandle);
}

bool wxGuiderPort::ReopenPort()
{
   if ( ! m_ioPortHandle.PortValid()) return false;

   return wxF()->portManager()->PReopen(m_ioPortHandle);
}


// create the IO port handle to be used with Guiding
bool wxGuiderPort::UseGXPort(const wxPortDescr& portDescr, wxGuiderPortProtocol::EPGProtocol protocol)
{
   // cleanup from previous - if any
   if (m_pGuider) delete m_pGuider; m_pGuider=NULL;

   // should be already droped by now but ???
   wxPortManager* pm = wxF()->portManager();
   if ( m_ioPortHandle.PortValid() ) pm->PClose(m_ioPortHandle);

   // open the new port with the default mask
   m_ioPortHandle = pm->POpen(portDescr, m_GX_mask);

   // some sanity check
   if ( ! m_ioPortHandle.PortValid()) {
      pm->PClose(m_ioPortHandle);
      m_pGuider = new wxGuiderPortProtocol(); // must have one, using the dummy
      return false; // ERROR EXIT
   }

   // choose the scope protocol driver
   switch (protocol) {
      case wxGuiderPortProtocol::EPGP_None:
         m_pGuider = new wxGuiderPortProtocol();
         break;
      case wxGuiderPortProtocol::EPGP_ST4:
         m_pGuider = new GP_ST4(m_ioPortHandle);
         break;
      case wxGuiderPortProtocol::EPGP_MTS_3SDI:
         m_pGuider = new GP_MTS_3SDI(m_ioPortHandle);
         break;
      case wxGuiderPortProtocol::EPGP_NexStar:
         m_pGuider = new GP_NexStar(m_ioPortHandle);
         break;
      case wxGuiderPortProtocol::EPGP_LX200:
         m_pGuider = new GP_LX200(m_ioPortHandle, false);
         break;
      case wxGuiderPortProtocol::EPGP_LX200Qx: // BM:20081226 added
         m_pGuider = new GP_LX200(m_ioPortHandle, true);
         break;
      case wxGuiderPortProtocol::EPGP_LX200PG:
         m_pGuider = new GP_LX200PG(m_ioPortHandle);
         break;
      case wxGuiderPortProtocol::EPGP_BRIDGE:
         m_pGuider = new wxGuiderPortProtocol(); // not yet usable here
         break;
      case wxGuiderPortProtocol::EPGP_LAST:
      default:
         ;
   }//switch

   return (m_pGuider!=NULL);
}

//! Switches Guiding off
// leave GX mode i.e. set all from init
void wxGuiderPort::GXmodeOff()
{
   if ( ! IsInitialised()) return; // cannot use the port

   wxPortManager* pm = wxF()->portManager();
   if (IsPortParallel()) {
      unsigned short wrd = m_GX_init;
      wrd = (m_invertedLogic) ? ~wrd: wrd;
      pm->PPut(m_ioPortHandle, wrd);
   }
   else if (IsPortGPUSB()) {  //BM:20081226 added
      unsigned short wrd = m_GX_init;
      wrd = (m_invertedLogic) ? ~wrd: wrd;
      pm->PPut(m_ioPortHandle, wrd);
   }
}

//! combined pulse guide command
//! define pos (e.g. RA+) or neg (e.g. DEC-) direction for both
//! define pulse length in milliseconds (no guiding to a direction if that duration is =0)
void wxGuiderPort::PulseGuide(bool positiveDirectionRA, unsigned long pulseDurationRA_ms,
                              bool positiveDirectionDEC, unsigned long pulseDurationDEC_ms)
{
   if (m_pGuider) {
      unsigned short dirWrdRA = 0, dirWrdDEC = 0, stopWrd = 0;
      if ( IsPortParallel() || IsPortGPUSB() ) { //BM:20090307 fix to make it work..
         // we have to translate the direction into the proper bits
         if ( pulseDurationRA_ms>0 ) {
            dirWrdRA  = (positiveDirectionRA) ? m_GXRA_pos : m_GXRA_neg;
         }
         if ( pulseDurationDEC_ms>0 ) {
            dirWrdDEC = (positiveDirectionDEC) ? m_GXDEC_pos : m_GXDEC_neg;
         }
         stopWrd = m_GX_init;

         // invert logic if needed and mask bits according to config
         dirWrdRA  = ((m_invertedLogic) ? ~dirWrdRA  : dirWrdRA)  & m_GX_mask;
         dirWrdDEC = ((m_invertedLogic) ? ~dirWrdDEC : dirWrdDEC) & m_GX_mask;
         stopWrd   = ((m_invertedLogic) ? ~stopWrd   : stopWrd)   & m_GX_mask;
      }
      else {
         // others go with ==0 / !=0  type and stop is not used
         if ( pulseDurationRA_ms>0 ) {
            dirWrdRA  = (positiveDirectionRA) ? 1 : 0;
         }
         if ( pulseDurationDEC_ms>0 ) {
            dirWrdDEC = (positiveDirectionDEC) ? 1 : 0;
         }
      }

      // guide pulse only if any duration >0 (no pulse needed if both are 0)
      if ( (pulseDurationRA_ms>0) || (pulseDurationDEC_ms>0) ) {
         m_pGuider->PulseGuide(dirWrdRA,  pulseDurationRA_ms,
                               dirWrdDEC, pulseDurationDEC_ms, stopWrd);
      }
   }
}

//! RA direction guiding
void wxGuiderPort::PulseGuideRA(bool positiveDirection, unsigned long pulseDuration_ms)
{
   PulseGuide(positiveDirection, pulseDuration_ms, true, 0); // no DEC guiding
}

//! DEC direction guiding
void wxGuiderPort::PulseGuideDEC(bool positiveDirection, unsigned long pulseDuration_ms)
{
   PulseGuide(true, 0, positiveDirection, pulseDuration_ms); // no RA guiding
}



bool wxGuiderPort::SaveToConfigGuidePort(wxConfig* config)
{
   wxConfigPath path(config,wxT("GuidingPort"));
   wxString portname = this->PortName();

   //save port name, bits and flags
   config->Write(_T("GXGuiderPort"),portname);
   config->Write(_T("GXGuiderProtocol"),(long)m_pGuider->Protocol());

   config->Write(_T("GXmask"),this->GXmask());
   config->Write(_T("GXinit"),this->GXinit());
   config->Write(_T("GXRA_pos"),this->GXRA_pos());
   config->Write(_T("GXDEC_pos"),this->GXDEC_pos());
   config->Write(_T("GXRA_neg"),this->GXRA_neg());
   config->Write(_T("GXDEC_neg"),this->GXDEC_neg());
   config->Write(_T("GXinvertedLogic"), this->InvertedLogic());
   config->Write(_T("GXhandshake"), this->SerialHandshake());
   config->Write(_T("GXbaudrate"), this->SerialBaudrate());

   return true;
}

bool wxGuiderPort::RestoreFromConfigGuidePort(wxConfig* config)
{
   wxConfigPath path(config,wxT("GuidingPort"));

   wxString portname;
   // either we find a portname in config or have to try the first in the list
   if(config->Read(_T("GXGuiderPort"),&portname, wxT("~"))) {
      wxPortDescr pd = wxF()->portManager()->PortDescriptorFirst(); // get first (should be "!None!")
      if ( 0 == portname.CmpNoCase(wxT("~")) ) {
         // ~ means no config value found
         wxMessageBox(_T("Auto-selected guider port ") + pd.PortName() + _T(", please check port settings."));
      }
      else {
         // there was a port already used, so use its descriptor
         pd = wxF()->portManager()->PortDescriptor(portname);
      }
      long protocol = (long)wxGuiderPortProtocol::EPGP_None;
      if(config->Read(_T("GXGuiderProtocol"),&protocol)) {
         if ( wxGuiderPortProtocol::EPGP_None == protocol ) {
            // ~ means no config value found
/*

            wxMessageBox(_T("Auto-selected guider protocol ")
               + wxGuiderPortProtocol::ProtocolName((wxGuiderPortProtocol::EPGProtocol)protocol)
               + _T(", please check port settings."));
*/
         }
      }

      // Let GX use this port
      if ( UseGXPort(pd, (wxGuiderPortProtocol::EPGProtocol)protocol) ) {
         // OK - setup the bits and flags from config (or use defaults if not configured yet)
         long value=this->GXmask();
         if(config->Read(_T("GXmask"),&value)) this->SetGXmask(wxUint8(value));

         value=this->GXinit();
         if(config->Read(_T("GXinit"),&value)) this->SetGXinit(wxUint8(value));

         value=this->GXRA_pos();
         if(config->Read(_T("GXRA_pos"),&value)) this->SetGXRA_pos(wxUint8(value));

         value=this->GXDEC_pos();
         if(config->Read(_T("GXDEC_pos"),&value)) this->SetGXDEC_pos(wxUint8(value));

         value=this->GXRA_neg();
         if(config->Read(_T("GXRA_neg"),&value)) this->SetGXRA_neg(wxUint8(value));

         value=this->GXDEC_neg();
         if(config->Read(_T("GXDEC_neg"),&value)) this->SetGXDEC_neg(wxUint8(value));

         value=this->InvertedLogic();
//         if(config->Read(_T("GXinvertedLogic"),&value)) this->SetInvertedLogic((value!=0)?true:false);
   /// NOTE just to make sure that it is really not inverted by any means (there is no switch in GUI)
   ///  this is because I cannot delete the already existing entry anymore - may be in future versions
   ///  we can omit this
         if(config->Read(_T("GXinvertedLogic"),&value)) this->SetInvertedLogic((value!=0)?false:false);

         value=this->SerialHandshake();
         if(config->Read(_T("GXhandshake"),&value)) this->SetSerialHandshake((EPGHandshake)value);

         value=this->SerialBaudrate();
         if(config->Read(_T("GXbaudrate"),&value)) this->SetSerialBaudrate((EPGBaudRate)value);

         // finished setting up the bits..
         if ( ! this->CommitConfiguration() ) {
            wxMessageBox(_T("Cannot configure guider control port ") + pd.PortName() \
                                       + _T(", please check port settings."));
         }
         this->GXmodeOff();  // init to make port lines defined

      }
      else {
         wxMessageBox(_T("Cannot use port ") + pd.PortName() + _T(" for guiding, please check port settings."));
         return false;
      }
   }
   else {

     // back up one level
     wxConfigPath path(config,wxT(".."));
      // create the default setting
     SaveToConfigGuidePort(config);
   }

   return true;
}
