
#include "wxGuiderBridgeInterface.h"

#include "wx/socket.h"

#include "wxIO/wxGuiderPort.h"
#include "wxIO/GP_BRIDGE.h"

#include "wxGuidingConfig.h"

wxGuiderBridgeInterface::wxGuiderBridgeInterface()
: m_socket(NULL)
, m_gPort(NULL)
, m_lastTimestamp(0)
{
   //ctor
}

wxGuiderBridgeInterface::~wxGuiderBridgeInterface()
{
   //dtor
   guideEnd();
}



bool wxGuiderBridgeInterface::guideStart()
{
   guideEnd();

   // Create the socket
   wxIPV4address addr;

   addr.LocalHost();   // any on this machine

   wxString hostname;
   short port = 5618;
   wxGuidingConfig::getBridgeParameters(hostname,port);

   addr.Hostname(hostname); // could be on a remote machine :-)
   addr.Service(port);

   m_socket = new wxSocketClient();

   m_socket->Connect(addr, false); // does not block here
   m_socket->WaitOnConnect(10,0);   // max 10 seconds to wait until completion
   if (m_socket->IsConnected()) {
      m_lastTimestamp = clock(); // create a valid startingpoint
      m_gPort = new GP_BRIDGE(m_socket);
      set_guiding_active(true);
   }
   else {
      m_socket->Close();
   }

   return (m_gPort!=NULL);
}

bool wxGuiderBridgeInterface::guideError(unsigned long timestamp  // (in)  Time stamp   [ms]
                                        ,double ra                // (in)  Error in RA  [arcseconds]
                                        ,double dec               // (in)  Error in DEC [arcseconds]
                                        )
{
   unsigned long dT = (timestamp-m_lastTimestamp); // duration since last pulse

   if(m_gPort) {
      //CA:20090412 - reports now in milliseconds so we just check
      if ( dT<100) {
         ::fprintf(stderr, "wxGuiderPortInterface::guideError - SW-ERROR Short dT %d (should be >=10)\n", int(dT));
         return false; // reporting interval is too short - bug??
      }


      const unsigned long g_pulse_limit_lo  =  100; // don't issue pulses less than this (millisec)
      const unsigned long g_pulse_limit_hi  = guideInterval();

      // get the up-to date tuning factors (these may have changed since previous pulse!!)

      // RA
      double RA_Dz = raDz();
      double RA_K  = raK();
      double RA_Q  = raQ();

      // DEC
      double DEC_Dz = decDz();
      double DEC_K  = decK();
      double DEC_Q  = decQ();

      // check for dead zones
      if(fabs(ra)  < RA_Dz)ra = 0.0;
      if(fabs(dec) < DEC_Dz)dec = 0.0;

      // formula based durations
      unsigned long durationRA    = (unsigned long)(RA_K  * fabs(ra)  + RA_Q);
      unsigned long durationDEC   = (unsigned long)(DEC_K * fabs(dec) + DEC_Q);

      unsigned short directionRA  = (ra>0.0)  ? 0: 1; //RA- : RA+
      unsigned short directionDEC = (dec>0.0) ? 0: 1; //Dec- : Dec+

      // check for out of limit durations
      if ( durationRA  < g_pulse_limit_lo ) durationRA  = 0; // out of limits
      if ( durationRA  > g_pulse_limit_hi ) durationRA  = g_pulse_limit_hi;

      if ( durationDEC < g_pulse_limit_lo ) durationDEC = 0; // out of limits
      if ( durationDEC > g_pulse_limit_hi ) durationDEC = g_pulse_limit_hi;

      if ( (durationRA>0) || (durationDEC>0) ) {

         // update recorded timestamp only when issuing a guide pulse
         m_lastTimestamp = timestamp;

         // only if something to do...
         // guide command goes directly to the driver not via wxGuiderPort !!
         m_gPort->PulseGuide(directionRA, durationRA, directionDEC, durationDEC, 0); // GUIDE!
      }

      return true;
   }
   return false;
}

// end a guiding sequence
bool wxGuiderBridgeInterface::guideEnd()
{
   if (m_gPort) delete m_gPort; m_gPort=NULL;
   if (m_socket) {
      m_socket->Close();
      delete m_socket;
   }
   set_guiding_active(false);
   m_socket=NULL;
   m_lastTimestamp = 0;
   return true;
}

wxGuider::guider_interface wxGuiderBridgeInterface::interface_type()
{
   return BRIDGE_INTERFACE;
}
