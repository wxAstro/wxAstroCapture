#ifndef WXGUIDERPORTINTERFACE_H
#define WXGUIDERPORTINTERFACE_H

#include "wxGuider.h"


class wxGuiderPort;
class wxGuiderPortInterface : public wxGuider {
public:
   wxGuiderPortInterface();
   virtual ~wxGuiderPortInterface();

   bool guideStart();

   // receive current guiding error, issue corrective guiding command to telescope
   bool guideError(unsigned long timestamp  // (in)  Time stamp   [ms]
                  ,double ra                // (in)  Error in RA  [arcseconds]
                  ,double dec               // (in)  Error in DEC [arcseconds]
                   );
   // end a guiding sequence
   bool guideEnd();

   guider_interface interface_type();

protected:
private:
   wxGuiderPort* m_gPort;
   unsigned long m_lastTimestamp;
};



#endif // WXGUIDERPORTINTERFACE_H
