
#ifndef _WXGUIDERPORTPROTOCOL_H__
#define _WXGUIDERPORTPROTOCOL_H__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wxPortDescr.h"

// #include <list>

//! a class that implements a generic scope protocol = driver
//! specific drivers should derive from this class and implement
//! a worker thread that uses the semaphore as command trigger and
//! extract commands from the queue using the mutex for queue access
class wxGuiderPortProtocol
{
friend class GP_ST4_Thread;      // grant access to locals
friend class GP_MTS_3SDI_Thread; // grant access to locals
friend class GP_NexStar_Thread;  // grant access to locals
friend class GP_LX200_Thread;    // grant access to locals
friend class GP_LX200PG_Thread;  // grant access to locals
friend class GP_BRIDGE_Thread;   // grant access to locals

public:
   virtual ~wxGuiderPortProtocol();

   //! The known telescope protocols = low level drivers
   enum EPGProtocol {
      EPGP_None      = 0,
      EPGP_ST4,      // ST4 like parallel port interface
      EPGP_NexStar,  // Celestron, Syntia RS232 Protocol
      EPGP_LX200,    // Meade LX Protocol M_ commands
      EPGP_LX200Qx,  // Meade LX Protocol M_ commands and Qx as stop (instead of only :Q)
      EPGP_LX200PG,  // Meade LX Protocol M_NNNN pulse guide commands
      EPGP_BRIDGE,   // Simple NWSE-X Protocol used for the Bridge
      EPGP_LAST,     // must remain here !!
      EPGP_MTS_3SDI, // MTS-3SDI Powerflex (unused)
   };


   //! Get the protocol name from a sepcific protocol
   static const wxString& ProtocolName(EPGProtocol protocol);     // might be useful for the GUI
   //! Get the protocol enum for a name (match is NoCase) returns EPGP_None if not found
   static EPGProtocol ProtocolEnum(const wxString& protocolName); // might be useful for the GUI

   //! reply if the specific protocol can handle the port type
   static bool SupportsSerial(EPGProtocol protocol);
   static bool SupportsParallel(EPGProtocol protocol);
   static bool SupportsGPUSB(EPGProtocol protocol); //BM:20081227 added

   // check to see if the port descriptor is compatible with the given protocol
   // this is a completely non-intrusive check
   static bool CanUseProtocol(const wxPortDescr& portDescr, EPGProtocol protocol);

   // this class on it's own implements a dummy guider protocol with no functionality
   virtual EPGProtocol Protocol() const {return EPGP_None;};

   //! Pulse Guide command with RA/DE individual duration
   void PulseGuide(unsigned short dirWrdRA,  unsigned long pulseDurationRA_ms,
                   unsigned short dirWrdDEC, unsigned long pulseDurationDEC_ms,
                   unsigned short stopWrd);

protected:
   // the dirWrd contains the direction specific part
   //   for serial ports it is =!0 for positive and =0 for negative direction
   //   for parallel port it contains the already prepared bitpattern (only wxGuiderPort knows it)
   // stopWrd is used for parallel port only and contains the stop command to be submitted
   // pulseDuration is specific for RA and DEC
   // the driver implementation is free to combine directional pulses or let them run
   // sequentially (RA MUST then come first - implementation rule!!)
   struct PulseQueueT {
      unsigned long  pulseDurationRA_ms;
      unsigned long  pulseDurationDEC_ms;
      unsigned short dirWrdRA;
      unsigned short dirWrdDEC;
      unsigned short stopWrd;
   };
 //  typedef std::list<PulseQueueT> PQLIST;
 //  PQLIST  m_pulseQueue;

 // instead of the PQLIST, we simply maintain the latest pulse
   PulseQueueT m_pulse;

   //! for the driver - extracts and deletes the next command from the queue using the mutex
   //! returns false if no element is available otherwise true
   bool ExtractNextCommand(PulseQueueT& qElement);

   wxSemaphore    m_semaCommand;

private:
   wxMutex        m_mutexQueue;

   struct ProtoPropT {
      wxString name;
      bool     canSerial;
      bool     canParallel;
      bool     canGPUSB;   //BM:20081226 added for USB GPSUB module
   };
   static ProtoPropT protocolProps[EPGP_LAST];

};


#endif // _WXGUIDERPORTPROTOCOL_H__

