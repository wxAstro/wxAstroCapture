#ifndef WXGUIDER_H
#define WXGUIDER_H

/*
   wxGuider is the abstract interface for all guiders.
   The application knows only about wxGuider objects for issuing guiding commands
*/

#include <cstring>

class wxGuider {
public:
   wxGuider();
   virtual ~wxGuider();

   // begin a guiding sequence
   virtual bool guideStart() = 0;

   // receive current guiding error, issue corrective guiding command to telescope
   virtual bool guideError(unsigned long timestamp  // (in)  Time stamp   [ms]
                          ,double ra                // (in)  Error in RA  [arcseconds]
                          ,double dec               // (in)  Error in DEC [arcseconds]
                           ) = 0;

   // end a guiding sequence
   virtual bool guideEnd() = 0;



   // set current guide interval
   virtual bool setGuideInterval(size_t interval_ms);

   // return current guide interval
   virtual size_t guideInterval();

   // convenient enum to identify the interface type used
   enum guider_interface { FILE_INTERFACE = 0,
                          PORT_INTERFACE,
                          BRIDGE_INTERFACE };

   virtual guider_interface interface_type() = 0;

   /*
       Guide pulse length is computed as follows, independent for RA and DEC.
       When the absolute error is less than the defined dead zone value, no guiding occurs.

       Guide pulse length [ms] = k [ms/arcsec] * error [arcsec] + q [ms]
   */

   void setTuningDefaults();

   // set and retrieve tuning factors for RA
   void setRaDz(double ra_dz);
   void setRaK(double ra_k);
   void setRaQ(double ra_q);

   double raDz();
   double raK();
   double raQ();

   // set and retrieve tuning factors for DEC
   void setDecDz(double dec_dz);
   void setDecK(double dec_k);
   void setDecQ(double dec_q);

   double decDz();
   double decK();
   double decQ();

   // This function returns true only when a guiding session is ongoing.
   bool guiding_active();

   void set_guiding_active(bool active);

private:
   size_t m_interval_ms; // guiding interval in ms


   double m_ra_dz; // [arcsec] dead zone
   double m_ra_k;  // [ms/arcsec] amplification factor k
   double m_ra_q;  // [ms] minimum pulse length

   double m_dec_dz; // [arcsec] dead zone
   double m_dec_k;  // [ms/arcsec] amplification factor k
   double m_dec_q;  // [ms] minimum pulse length

   bool   m_guiding_active; // True when guiding session is ongoing
};



#endif // WXGUIDER_H
