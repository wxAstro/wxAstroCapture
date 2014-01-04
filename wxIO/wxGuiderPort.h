
#ifndef _WXGUIDERPORT_H__
#define _WXGUIDERPORT_H__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/config.h>

#include "wxPortDescr.h"
#include "wxGuiderPortProtocol.h"
#include "wxPortManager.h"

class wxGuiderPort
{
public:
   wxGuiderPort();
	virtual ~wxGuiderPort();

   //! returns true if assigned and open
   bool IsInitialised() const;

   //! those two enums are repeated here to avoid inclusion of wxPort into calling app !

   //!! NOTE: make sure this enum matches the one of wxPort !!!
   enum EPGBaudRate {
      EPGB_1200  = 0,
      EPGB_2400,
      EPGB_4800,
      EPGB_9600,
      EPGB_19200,
      EPGB_38400,
      EPGB_LAST,   // must remain here !!
   };
   //!! NOTE: make sure this enum matches the one of wxPort !!!
   enum EPGHandshake {
      EPGH_None = 0,
      EPGH_XonXoff,
      EPGH_RtsCts,
      EPGH_LAST,   // must remain here !!
   };

   // note for parallel ports use the bit value
   void SetGXmask(unsigned char pVal);       // others will not be affected
   void SetGXinit(unsigned char pVal);       // init GX - draw all high
   void SetGXRA_pos(unsigned char pVal);       // RA+
   void SetGXRA_neg(unsigned char pVal);       // RA-
   void SetGXDEC_pos(unsigned char pVal);      // Dec+
   void SetGXDEC_neg(unsigned char pVal);      // Dec-
   void SetInvertedLogic(bool pVal);         // set (parallel) logic inverted
   void SetSerialHandshake(EPGHandshake pVal);
   void SetSerialBaudrate(EPGBaudRate pVal);

   // return current settings
   const wxString& PortName();
   const wxString& ProtocolName();

   unsigned char GXmask() const;       //
   unsigned char GXinit() const;       // init GX
   unsigned char GXRA_pos() const;       // RA+
   unsigned char GXRA_neg() const;       // RA-
   unsigned char GXDEC_pos() const;      // Dec+
   unsigned char GXDEC_neg() const;      // Dec-
   bool InvertedLogic() const;         // the (parallel) logic inverted
   EPGHandshake SerialHandshake() const;
   EPGBaudRate SerialBaudrate() const;

   // Default values sometimes needed by GUI
   static unsigned char GXmask_default();       //
   static unsigned char GXinit_default();       // init GX
   static unsigned char GXRA_pos_default();       // RA+
   static unsigned char GXRA_neg_default();       // RA-
   static unsigned char GXDEC_pos_default();      // Dec+
   static unsigned char GXDEC_neg_default();      // Dec-

   //! Commits a config change to the portmanager
   bool CommitConfiguration();               // Open the port with mask setting

   //! just return the GX bits to 'standard'
   void SetGXDefault();

   bool IsPortParallel() const;
   bool IsPortSerial() const;
   bool IsPortGPUSB() const;  //BM:20081226 added
   bool IsPortDummy() const;

   void ReleasePort();
   bool ReopenPort();

   bool UseGXPort(const wxPortDescr& portDescr, wxGuiderPortProtocol::EPGProtocol protocol);


   // save and restore long exposure info to configuration
   bool SaveToConfigGuidePort(wxConfig* config);
   bool RestoreFromConfigGuidePort(wxConfig* config);

public:
/*
   enum EPGDirection {
      // bit coded fields to allow multiple direction guiding
      EPGD_None  = 0, // ..
      EPGD_RA_pos  = 1, // RA+
      EPGD_RA_neg  = 2, // RA-
      EPGD_DEC_pos = 4, // Dec+
      EPGD_DEC_neg = 8, // Dec-
   };
*/
   //! combined pulse guide command
   //! define pos (e.g. RA+) or neg (e.g. DEC-) direction for both
   //! define pulse length in milliseconds (no guiding to a direction if that duration is =0)
   void PulseGuide(bool positiveDirectionRA, unsigned long pulseDurationRA_ms,
                   bool positiveDirectionDEC, unsigned long pulseDurationDEC_ms);
   //! RA direction guiding
   void PulseGuideRA(bool positiveDirection, unsigned long pulseDuration_ms);
   //! DEC direction guiding
   void PulseGuideDEC(bool positiveDirection, unsigned long pulseDuration_ms);

   //! Switches Guiding off
   void GXmodeOff();

private:
   //BM:20081228  private now -- use SetGXDefault()
   void SetDefaultSerial();
   void SetDefaultParallel();
   void SetDefaultGPUSB();  //BM:20081226 added

private:
   wxPortHandle          m_ioPortHandle; // this handle is owned here
   wxGuiderPortProtocol* m_pGuider;

   wxUint8  m_GX_mask;
   wxUint8  m_GX_init;
   wxUint8  m_GXRA_pos;
   wxUint8  m_GXRA_neg;
   wxUint8  m_GXDEC_pos;
   wxUint8  m_GXDEC_neg;
   bool     m_invertedLogic; // true is inverted logic

   EPGHandshake   m_handshake;
   EPGBaudRate    m_baudrate;

};



#endif // _WXGUIDERPORT_H__

