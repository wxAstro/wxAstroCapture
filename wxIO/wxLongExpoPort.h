
#ifndef _WXLONGEXPOPORT_H__
#define _WXLONGEXPOPORT_H__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/config.h>

//#include "wxPort.h"
#include "wxPortDescr.h"
#include "wxPortManager.h"

class wxLongExpoPort
{
public:
   wxLongExpoPort();
	virtual ~wxLongExpoPort();

   //! returns true if assigned and open
   bool IsInitialised() const;

   // note for parallel ports use the bit value
   // for serial ports use RTS = 1; DTR = 2
   void SetLXmask(wxUint8 pVal);       // others will not be affected
   void SetLXinit(wxUint8 pVal);       // init LE - draw all high
   void SetLXreadout(wxUint8 pVal);    // the LE Frame bits
   void SetLXampOn(wxUint8 pVal);      // the Amp Bit
   void SetLXshutOn(wxUint8 pVal);     // the Shutter Bit
   void SetInvertedLogic(bool pVal);         // set (serial) logic inverted

   // return current settings
   const wxString& PortName();
   wxUint8 LXmask() const;       //
   wxUint8 LXinit() const;       // init LE
   wxUint8 LXreadout() const;    // the LE Frame bits
   wxUint8 LXampOn() const;      // the Amp Bit
   wxUint8 LXshutOn() const;     // the Shutter Bit
   bool InvertedLogic() const;         // the (serial) logic inverted

   //! Commits a config change to the portmanager
   bool CommitConfiguration();               // Open the port with mask setting

   //! just return the LX bits to 'standard'
   void SetLXDefault();

   // if On - enable the shutter for at least one frame (300ms) before
   //   the exposure starts - this will clear the CCD wells completely
   //   using the cam internal electronic shutter (avoids ghost images)
   void CycleShutterOn() {m_shutterMode=true;};
   void CycleShutterOff() {m_shutterMode=false;};
   bool IsCycleShutter() const {return m_shutterMode;};
   void SetCycleShutter(bool pVal) {m_shutterMode=pVal;};

   // if On - turn amp on/off during exposure
   void AMPmodeOn() {m_ampMode=true;};
   void AMPmodeOff() {m_ampMode=false;};
   bool IsAMPmodeOn() const {return m_ampMode;};
   void SetAMPmodeOn(bool pVal) {m_ampMode=pVal;};

   // enter LX mode i.e. unset all but readout and amp
   void LXmodeOn();
   // leave LX mode i.e. set all from init
   void LXmodeOff();
   bool IsLXmodeOn() const {return m_lxMode;};


   // set frame readout off (amp too if set)
   // fails if not in LXmode
   // fails if already in exposure mode
   // NOTE:  best to sync this with image capture cycle
   bool PreStartExposure();
   bool StartExposure();
   // called when the amp should be switched on (if set)
   bool TriggerExposure();
   // called to enable framereadout
   // NOTE:  best to sync this with image capture cycle
   bool ReadExposure();
   // called to finish the capture cycle (a new one is allowed now)
   void EndExposure();

   bool IsPortParallel() const;
   bool IsPortSerial() const;
   bool IsPortLXUSB() const; //BM:20081228 added
   bool IsPortLXLED() const; //BM:20081228 added
   bool IsPortDummy() const;


   bool UseLEPort(const wxPortDescr& portDescr);
   // save and restore long exposure info to configuration
   bool SaveToConfigLongExpo(wxConfig* config);
   bool RestoreFromConfigLongExpo(wxConfig* config);

private:
   //BM:20081228  private now -- use SetLXDefault()
   void SetDefaultSerial();
   void SetDefaultParallel();
   void SetDefaultLXUSB();
   void SetDefaultLXLED();

private:
   wxPortHandle  m_ioPortHandle; // this handle is owned here

   bool    m_lxMode;
   bool    m_ampMode;
   bool    m_shutterMode;
   bool    m_expoCycle;
   bool    m_invertedLogic; // true is inverted logic

   wxUint8 m_LX_mask;
   wxUint8 m_LX_init;
   wxUint8 m_LX_readout;
   wxUint8 m_LX_ampOn;
   wxUint8 m_LX_shutOn;

};

#endif // _WXLONGEXPOPORT_H__

