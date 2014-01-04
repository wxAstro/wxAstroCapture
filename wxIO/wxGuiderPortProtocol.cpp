

#include "wxGuiderPortProtocol.h"


/// wxGuiderPortProtocol

/// static part

// setup protocol property array
wxGuiderPortProtocol::ProtoPropT wxGuiderPortProtocol::protocolProps[EPGP_LAST] =
{
   // name                     ser    par    GPUSB
   {wxT("None"),               false, false, false},
   {wxT("ST4 like parallel"),  false, true,  true},//BM:20081226 added for USB GPSUB module
//   {wxT("PowerFlex MTS-3"),   true,  false,  false},
   {wxT("Celestron NexStar"),  true,  false, false},
   {wxT("LX200 style slew"),   true,  false, false},
   {wxT("LX200 style Qx end"), true,  false, false},//BM:20081226 added for USB GPSUB module
   {wxT("LX200 style pulse"),  true,  false, false},
   {wxT("Bridge pulse out"),   false, false, false},
};

wxGuiderPortProtocol::~wxGuiderPortProtocol()
{
   // we have disable the m_pulseQueue
  // NOT USED m_pulseQueue.clear();
}

const wxString& wxGuiderPortProtocol::ProtocolName(EPGProtocol protocol)
{
   return (protocol<EPGP_LAST)? protocolProps[protocol].name : protocolProps[EPGP_None].name;
}

wxGuiderPortProtocol::EPGProtocol wxGuiderPortProtocol::ProtocolEnum(const wxString& protocolName)
{
   for (unsigned i= EPGP_None; i<EPGP_LAST; i++) {
      if ( 0==protocolName.CmpNoCase(protocolProps[i].name) ) return (EPGProtocol)i;
   }
   return EPGP_None;
}

bool wxGuiderPortProtocol::SupportsSerial(EPGProtocol protocol)
{
   return (protocol<EPGP_LAST)? protocolProps[protocol].canSerial : protocolProps[EPGP_None].canSerial;
}

bool wxGuiderPortProtocol::SupportsParallel(EPGProtocol protocol)
{
   return (protocol<EPGP_LAST)? protocolProps[protocol].canParallel : protocolProps[EPGP_None].canParallel;
}

//BM:20081227 added
bool wxGuiderPortProtocol::SupportsGPUSB(EPGProtocol protocol)
{
   return (protocol<EPGP_LAST)? protocolProps[protocol].canGPUSB : protocolProps[EPGP_None].canParallel;
}


bool wxGuiderPortProtocol::CanUseProtocol(const wxPortDescr& portDescr, wxGuiderPortProtocol::EPGProtocol protocol)
{
   if(portDescr.IsPortSerial()) {
      return wxGuiderPortProtocol::SupportsSerial(protocol);
   }
   else if(portDescr.IsPortParallel()) {
      return wxGuiderPortProtocol::SupportsParallel(protocol);
   }
   else if(portDescr.IsPortGPUSB()) {
      return wxGuiderPortProtocol::SupportsGPUSB(protocol);
   }
   return false;
}

/// END OF static part


/// class common part

//! for the driver - extracts and deletes the next command from the queue using the mutex
bool wxGuiderPortProtocol::ExtractNextCommand(PulseQueueT& qElement)
{
   // before using the list we must acquire the mutex
   wxMutexLocker lock(m_mutexQueue);

   // extract the one and only pulse if valid
   if((m_pulse.pulseDurationRA_ms  != 0)
   || (m_pulse.pulseDurationDEC_ms != 0)) {

      // take a copy
      qElement = m_pulse;

      // invalidate the one and only pulse
      m_pulse.dirWrdRA = 0;
      m_pulse.dirWrdDEC = 0;
      m_pulse.stopWrd = 0;
      m_pulse.pulseDurationRA_ms = 0;
      m_pulse.pulseDurationDEC_ms = 0;

      m_mutexQueue.Unlock();
      return true;
   }

/*
   if (m_pulseQueue.size()>0) {

      qElement = m_pulseQueue.front(); // copy element allows deletion of queue element
      m_pulseQueue.pop_front();  // release element
      m_mutexQueue.Unlock();
      return true;
   }
*/
   else {
      return false;
   }
   // lock is managed by wxMutexLocker
}


//! Pulse Guide command with RA/DE individual duration
void wxGuiderPortProtocol::PulseGuide(unsigned short dirWrdRA,  unsigned long pulseDurationRA_ms,
                                      unsigned short dirWrdDEC, unsigned long pulseDurationDEC_ms,
                                      unsigned short stopWrd)
{
   if ( EPGP_None!=Protocol() ) {
      // before using the list we must acquire the mutex
      wxMutexLocker lock(m_mutexQueue);

      // just add it to the list
      PulseQueueT queueEntry;
      queueEntry.dirWrdRA = dirWrdRA;
      queueEntry.dirWrdDEC = dirWrdDEC;
      queueEntry.stopWrd = stopWrd;
      queueEntry.pulseDurationRA_ms = pulseDurationRA_ms;
      queueEntry.pulseDurationDEC_ms = pulseDurationDEC_ms;

      // not really
      //  m_pulseQueue.push_back(queueEntry);

      // instead update the one and only pulse
      m_pulse = queueEntry;

      m_semaCommand.Post(); // unlock the thread
   }
   // lock is managed by wxMutexLocker
}


