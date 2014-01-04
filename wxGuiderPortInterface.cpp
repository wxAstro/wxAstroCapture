#include "wxGuiderPortInterface.h"
#include "wxGuidingConfig.h"

#include "wxIO/wxGuiderPort.h"

#include "wxFactory.h"

#include <time.h>
#include <math.h>



wxGuiderPortInterface::wxGuiderPortInterface()
: m_gPort(0)
, m_lastTimestamp(0)
{
   //ctor
}

wxGuiderPortInterface::~wxGuiderPortInterface()
{
   guideEnd();
}


bool wxGuiderPortInterface::guideStart()
{
   guideEnd();
   m_gPort = wxF()->guidePort();
   if (! m_gPort->ReopenPort())
   {
      wxMessageBox(_T("wxGuider - cannot reopen port - check settings and progs that may use it!"));
   }

   m_lastTimestamp = clock(); // create a valid startingpoint
   if(m_gPort) {
      set_guiding_active(true);
      return true;
   }
   return false;
}

bool wxGuiderPortInterface::guideError(unsigned long timestamp  // (in)  Time stamp   [ms]
                                      ,double ra                // (in)  Error in RA  [arcseconds]
                                      ,double dec               // (in)  Error in DEC [arcseconds]
                                      )
{
   unsigned long dT = (timestamp-m_lastTimestamp); // duration since last pulse

   if(m_gPort) {
      //CA:20090412 - reports now in ms so we just check
      if ( dT<100) {
         ::fprintf(stderr, "wxGuiderPortInterface::guideError - SW-ERROR Short dT %d (should be >=100)\n", int(dT));
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

      bool directionRA  = (ra>0.0)  ? false: true; //RA- : RA+
      bool directionDEC = (dec>0.0) ? false: true; //Dec- : Dec+

      // check for out of limit durations
      if ( durationRA  < g_pulse_limit_lo ) durationRA  = 0; // out of limits
      if ( durationRA  > g_pulse_limit_hi ) durationRA  = g_pulse_limit_hi;

      if ( durationDEC < g_pulse_limit_lo ) durationDEC = 0; // out of limits
      if ( durationDEC > g_pulse_limit_hi ) durationDEC = g_pulse_limit_hi;

      if ( (durationRA>0) || (durationDEC>0) ) {

         // update recorded timestamp only when issuing a guide pulse
         m_lastTimestamp = timestamp;

         // only if something to do...
         // guide command goes via wxGuiderPort !!
         m_gPort->PulseGuide(directionRA, durationRA, directionDEC, durationDEC); // GUIDE!
      }
      return true;
   }
   return false;
}

// end a guiding sequence
bool wxGuiderPortInterface::guideEnd()
{
   if(m_gPort) {
      m_gPort->GXmodeOff();
      m_gPort->ReleasePort();
      m_gPort = 0;
   }
   set_guiding_active(false);
   m_lastTimestamp = 0;
   return true;
}

wxGuider::guider_interface wxGuiderPortInterface::interface_type()
{
   return PORT_INTERFACE;
}
