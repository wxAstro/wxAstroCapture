#ifndef WXGUIDERBRIDGEINTERFACE_H
#define WXGUIDERBRIDGEINTERFACE_H

#include "wxGuider.h"

class wxGuiderPortProtocol;
class wxSocketClient;

class wxGuiderBridgeInterface : public wxGuider {
public:
   wxGuiderBridgeInterface();
   virtual ~wxGuiderBridgeInterface();

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
   wxSocketClient*       m_socket;
   wxGuiderPortProtocol* m_gPort;
   unsigned long m_lastTimestamp;
};



#endif // WXGUIDERBRIDGEINTERFACE_H
