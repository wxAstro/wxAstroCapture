

#include "wxLongExpoPort.h"

#include <assert.h>

#include "../wxFactory.h"
#include "wxPortManager.h"

// Defaults for Standard LE parallel port
// using lower 4 bits -
// bit 0 = LE Frame A (1)
// bit 1 = LE Frame B (2)
// bit 2 = Amp toggle (4)
// bit 3 = Shutter if not hardwired (8)
static const wxUint8 _LXP_mask    = 0xf;      // others will not be affected
static const wxUint8 _LXP_init    = 0xf;      // init LE - draw all high
static const wxUint8 _LXP_readout = 3;        // the LE Frame bits
static const wxUint8 _LXP_ampOn   = 4;        // the Amp Bit
static const wxUint8 _LXP_shutOn  = 8;        // the Shutter Bit
static const bool _LXP_inverted         = false;    // parallel logic is usually NOT inverted
static const bool _LXP_ampUsed          = false;    // amp not used
static const bool _LXP_shutterUsed      = false;    // shutter not used

// Defaults for Standard LE serial port
// using RTS and DTR
// RTS = LE Frame A (1)
// DTR = Amp toggle (2)
static const wxUint8 _LXS_mask    = 0x3;      // others will not be affected
static const wxUint8 _LXS_init    = 0x3;      // init LE - draw all high
static const wxUint8 _LXS_readout = 1;        // the LE Frame bits
static const wxUint8 _LXS_ampOn   = 2;        // the Amp Bit
static const wxUint8 _LXS_shutOn  = 0;        // the Shutter Bit (not with serial)
static const bool _LXS_inverted         = true;     // serial logic is usually inverted
static const bool _LXS_ampUsed          = false;    // amp not used
static const bool _LXS_shutterUsed      = false;    // shutter not used

// Defaults for Standard LE LXGPUSB Port //BM:20081228 added
// using lower 4 bits -
// bit 0 = LE Frame A (1)
// bit 1 = LE Frame B (2)
// bit 2 = Shutter if not hardwired (4)
// bit 3 = Amp toggle (8)
static const wxUint8 _LXLP_mask    = 0xf;      // others will not be affected
static const wxUint8 _LXLP_init    = 0xf;      // init LE - draw all high translates to deasserted
static const wxUint8 _LXLP_readout = 3;        // the LE Frame bits
static const wxUint8 _LXLP_shutOn  = 4;        // the Shutter Bit
static const wxUint8 _LXLP_ampOn   = 8;        // the Amp Bit
static const bool _LXLP_inverted         = false;    // parallel logic is usually NOT inverted
static const bool _LXLP_ampUsed          = false;    // amp not used
static const bool _LXLP_shutterUsed      = false;    // shutter not used

// Defaults for Standard LE LXLED (TucLED) Port //BM:20081228 added
// using LED only
// LED on = LE Frame A (1)
static const wxUint8 _LXL_mask    = 0x1;      // others will not be affected
static const wxUint8 _LXL_init    = 0x1;      // init LE - draw all high
static const wxUint8 _LXL_readout = 1;        // the LE Frame bits (LED On = readout enabled)
static const wxUint8 _LXL_ampOn   = 0;        // the Amp Bit (not with LED)
static const wxUint8 _LXL_shutOn  = 0;        // the Shutter Bit (not with LED)
static const bool _LXL_inverted         = false;     // serial logic is usually inverted
static const bool _LXL_ampUsed          = false;    // amp not used
static const bool _LXL_shutterUsed      = false;    // shutter not used


static const wxString _EmptyString = wxString(wxT(""));



wxLongExpoPort::wxLongExpoPort()
: m_lxMode(false)
, m_ampMode(false)
, m_shutterMode(false)
, m_expoCycle(false)
, m_invertedLogic(false)
, m_LX_mask(_LXP_mask)
, m_LX_init(_LXP_init)
, m_LX_readout(_LXP_readout)
, m_LX_ampOn(_LXP_ampOn)
, m_LX_shutOn(_LXP_shutOn)
{
}

wxLongExpoPort::~wxLongExpoPort()
{
   if ( m_ioPortHandle.PortValid() )
      wxF()->portManager()->PClose(m_ioPortHandle);
}

bool wxLongExpoPort::IsInitialised() const
{
    return m_ioPortHandle.PortValid();
}

// some straight forward things (could be inline to optimize)
void wxLongExpoPort::SetLXmask(wxUint8 pVal)     {m_LX_mask = pVal;}
void wxLongExpoPort::SetLXinit(wxUint8 pVal)     {m_LX_init = pVal;}
void wxLongExpoPort::SetLXreadout(wxUint8 pVal)  {m_LX_readout = pVal;}
void wxLongExpoPort::SetLXampOn(wxUint8 pVal)    {m_LX_ampOn = pVal;}
void wxLongExpoPort::SetLXshutOn(wxUint8 pVal)   {m_LX_shutOn = pVal;}
void wxLongExpoPort::SetInvertedLogic(bool pVal) {m_invertedLogic = pVal;}

wxUint8 wxLongExpoPort::LXmask() const      {return m_LX_mask;}
wxUint8 wxLongExpoPort::LXinit() const      {return m_LX_init;}
wxUint8 wxLongExpoPort::LXreadout() const   {return m_LX_readout;}
wxUint8 wxLongExpoPort::LXampOn() const     {return m_LX_ampOn;}
wxUint8 wxLongExpoPort::LXshutOn() const    {return m_LX_shutOn;}
bool wxLongExpoPort::InvertedLogic() const  {return m_invertedLogic;}


const wxString& wxLongExpoPort::PortName()  {return m_ioPortHandle.PortName();}
bool wxLongExpoPort::IsPortParallel() const {return m_ioPortHandle.IsPortParallel();}
bool wxLongExpoPort::IsPortSerial() const   {return m_ioPortHandle.IsPortSerial();}
bool wxLongExpoPort::IsPortLXUSB() const    {return m_ioPortHandle.IsPortLXUSB();}
bool wxLongExpoPort::IsPortLXLED() const    {return m_ioPortHandle.IsPortLXLED();}
bool wxLongExpoPort::IsPortDummy() const    {return m_ioPortHandle.IsPortDummy();}


// enter LX mode i.e. unset all but readout and amp
void wxLongExpoPort::LXmodeOn()
{
   if ( ! IsInitialised()) return; // cannot use the port

   unsigned short wrd = m_LX_readout | m_LX_ampOn;
   wrd = (m_invertedLogic) ? ~wrd: wrd;
   wxF()->portManager()->PPut(m_ioPortHandle, wrd);

   m_lxMode = true;
}

// leave LX mode i.e. set all from init
void wxLongExpoPort::LXmodeOff()
{
   if ( ! IsInitialised()) return; // cannot use the port

   unsigned short wrd = m_LX_init;
   wrd = (m_invertedLogic) ? ~wrd: wrd;
   wxF()->portManager()->PPut(m_ioPortHandle, wrd);

   m_lxMode = false;
}

// start of a LE cycle
// set shutter cycle if enabled
bool wxLongExpoPort::PreStartExposure()
{
   if ( ! IsInitialised()) return false; // cannot use the port

   if (m_expoCycle) return false;
   if (!m_lxMode) return false;

   m_expoCycle=true;

   if (m_shutterMode && m_LX_shutOn) {
        // this is only supported for parallel ports
      unsigned short wrd = m_LX_ampOn | m_LX_readout | m_LX_shutOn;
      wrd = (m_invertedLogic) ? ~wrd: wrd;
      wxF()->portManager()->PPut(m_ioPortHandle, wrd);
      // wait 300ms now via Timer
   }
   return true;
}

// set frame readout off (amp too if set)
// NOTE:  best to sync this with image capture cycle
bool wxLongExpoPort::StartExposure()
{
   if ( ! IsInitialised()) return false; // cannot use the port

   if (!m_expoCycle) return false;
   if (!m_lxMode) return false;

   unsigned short wrd = (m_ampMode)? 0 : m_LX_ampOn;
   wrd = (m_invertedLogic) ? ~wrd: wrd;
   wxF()->portManager()->PPut(m_ioPortHandle, wrd);

   return true;
}

// called when the amp should be switched on (if set)
bool wxLongExpoPort::TriggerExposure()
{
   if ( ! IsInitialised()) return false; // cannot use the port

   if (!m_expoCycle)
      return false;

   if (!m_ampMode)
      return true;  // nothing to do if not in AmpMode

   unsigned short wrd = m_LX_ampOn;
   wrd = (m_invertedLogic) ? ~wrd: wrd;
   wxF()->portManager()->PPut(m_ioPortHandle, wrd);

   return true;
}

// called to enable framereadout
// NOTE:  best to sync this with image capture cycle
bool wxLongExpoPort::ReadExposure()
{
   if ( ! IsInitialised()) return false; // cannot use the port

   if (!m_expoCycle)
      return false;

   unsigned short wrd = m_LX_ampOn | m_LX_readout;
   wrd = (m_invertedLogic) ? ~wrd: wrd;
   wxF()->portManager()->PPut(m_ioPortHandle, wrd);

   return true;
}

// called to finish the capture cycle (a new one is allowed now)
void wxLongExpoPort::EndExposure()
{
   if ( ! IsInitialised()) return; // cannot use the port

    m_expoCycle = false;
    if (!m_lxMode) return;

    LXmodeOn(); // usually already there
}

bool wxLongExpoPort::CommitConfiguration()
{
   if ( ! IsInitialised()) return false; // cannot use the port

   return wxF()->portManager()->PSetMask(m_ioPortHandle, m_LX_mask); // set the bits used for LE
}

void wxLongExpoPort::SetLXDefault()
{
   SetDefaultParallel(); // default to have something set
   if ( m_ioPortHandle.IsPortSerial() ) SetDefaultSerial();
   else if ( m_ioPortHandle.IsPortLXUSB() ) SetDefaultLXUSB(); //BM:20081228 added
   else if ( m_ioPortHandle.IsPortLXLED() ) SetDefaultLXLED(); //BM:20081228 added

   // reset state vars
   m_lxMode = false;
   m_expoCycle = false;
}

void wxLongExpoPort::SetDefaultParallel()
{
   m_LX_mask = _LXP_mask;
   m_LX_init = _LXP_init;
   m_LX_readout = _LXP_readout;
   m_LX_ampOn = _LXP_ampOn;
   m_LX_shutOn = _LXP_shutOn;
   m_invertedLogic = _LXP_inverted;
   m_ampMode = _LXP_ampUsed;
   m_shutterMode = _LXP_shutterUsed;
}

void wxLongExpoPort::SetDefaultSerial()
{
   m_LX_mask = _LXS_mask;
   m_LX_init = _LXS_init;
   m_LX_readout = _LXS_readout;
   m_LX_ampOn = _LXS_ampOn;
   m_LX_shutOn = _LXS_shutOn;
   m_invertedLogic = _LXS_inverted;
   m_ampMode = _LXS_ampUsed;
   m_shutterMode = _LXS_shutterUsed;
}

void wxLongExpoPort::SetDefaultLXUSB() //BM:20081228 added
{
   m_LX_mask = _LXLP_mask;
   m_LX_init = _LXLP_init;
   m_LX_readout = _LXLP_readout;
   m_LX_ampOn = _LXLP_ampOn;
   m_LX_shutOn = _LXLP_shutOn;
   m_invertedLogic = _LXLP_inverted;
   m_ampMode = _LXLP_ampUsed;
   m_shutterMode = _LXLP_shutterUsed;
}

void wxLongExpoPort::SetDefaultLXLED() //BM:20081228 added
{
   m_LX_mask = _LXL_mask;
   m_LX_init = _LXL_init;
   m_LX_readout = _LXL_readout;
   m_LX_ampOn = _LXL_ampOn;
   m_LX_shutOn = _LXL_shutOn;
   m_invertedLogic = _LXL_inverted;
   m_ampMode = _LXL_ampUsed;
   m_shutterMode = _LXL_shutterUsed;
}


// create the IO port handle to be used with LE
bool wxLongExpoPort::UseLEPort(const wxPortDescr& portDescr)
{
   wxPortManager* pm = wxF()->portManager();
   // cleanup from previous - if any
   // should be already droped by now but ???
   if ( m_ioPortHandle.PortValid() ) pm ->PClose(m_ioPortHandle);

   // open the new port with the default mask
   m_ioPortHandle = pm ->POpen(portDescr, m_LX_mask);

   // some sanity check
   if ( ! m_ioPortHandle.PortValid()) {
      pm ->PClose(m_ioPortHandle);
      return false; // ERROR EXIT
   }
   return true;
}

bool wxLongExpoPort::SaveToConfigLongExpo(wxConfig* config)
{
   wxString OldPath = config->GetPath();
   wxString portname = this->PortName();

   config->SetPath(wxT("LongExposure"));
   //save port name, bits and flags
   config->Write(_T("LXCamPort"),portname);
   config->Write(_T("LXmask"),this->LXmask());
   config->Write(_T("LXinit"),this->LXinit());
   config->Write(_T("LXreadout"),this->LXreadout());
   config->Write(_T("LXampOn"),this->LXampOn());
   config->Write(_T("LXshutOn"),this->LXshutOn());
   config->Write(_T("LXinvertedLogic"), this->InvertedLogic());
   config->Write(_T("LXuseAmp"), this->IsAMPmodeOn());
   config->Write(_T("LXuseShutter"), this->IsCycleShutter());

   config->SetPath(OldPath);
   return true;
}

bool wxLongExpoPort::RestoreFromConfigLongExpo(wxConfig* config)
{
   wxString OldPath = config->GetPath();
   config->SetPath(wxT("LongExposure"));

   wxString portname;
   wxPortManager* pm = wxF()->portManager();
   // either we find a portname in config or have to try the first in the list
   if(config->Read(_T("LXCamPort"),&portname, wxT("~"))) {
      wxPortDescr pd = pm->PortDescriptorFirst(); // get first (should be "!None!")
      if ( 0 == portname.CmpNoCase(wxT("~")) || 0==portname.CmpNoCase(wxT(""))  ) {
         // ~ or empty string means no config value found
         // an empty string can appear in the config, so we have to test for it
         wxMessageBox(_T("Auto-selected long exposure control port ") + pd.PortName() + _T(", please check port settings."));
      }
      else {
         // there was a port already used, so use its descriptor
         pd = pm->PortDescriptor(portname);
      }

      // Let LE use this port
      if ( UseLEPort(pd) ) {
         // OK - setup the bits and flags from config (or use defaults if not configured yet)
         long value=this->LXmask();
         if(config->Read(_T("LXmask"),&value)) this->SetLXmask(wxUint8(value));

         value=this->LXinit();
         if(config->Read(_T("LXinit"),&value)) this->SetLXinit(wxUint8(value));

         value=this->LXreadout();
         if(config->Read(_T("LXreadout"),&value)) this->SetLXreadout(wxUint8(value));

         value=this->LXampOn();
         if(config->Read(_T("LXampOn"),&value)) this->SetLXampOn(wxUint8(value));

         value=this->LXshutOn();
         if(config->Read(_T("LXshutOn"),&value)) this->SetLXshutOn(wxUint8(value));

         value=this->InvertedLogic();
         if(config->Read(_T("LXinvertedLogic"),&value)) this->SetInvertedLogic((value!=0)?true:false);

         value=this->InvertedLogic();
         if(config->Read(_T("LXuseAmp"),&value)) this->SetAMPmodeOn((value!=0)?true:false);

         value=this->InvertedLogic();
         if(config->Read(_T("LXuseShutter"),&value)) this->SetCycleShutter((value!=0)?true:false);

         // finished setting up the bits..
         if ( ! this->CommitConfiguration() ) {
            wxMessageBox(_T("Cannot configure long exposure control port ") + pd.PortName() \
                                       + _T(", please check port settings."));
         }
         this->LXmodeOff();  // init to make port lines defined

      }
      else {
         wxMessageBox(_T("Cannot use port ") + pd.PortName() + _T(" for long exposure, please check port settings."));
         return false;
      }
   }
   else {
      // create the default setting
      config->SetPath(OldPath);
      SaveToConfigLongExpo(config);
   }

   config->SetPath(OldPath);

   return true;
}
