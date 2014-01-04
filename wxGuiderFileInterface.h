#ifndef WXGUIDERFILEINTERFACE_H
#define WXGUIDERFILEINTERFACE_H

#include "wxGuider.h"
#include <wx/string.h>

#include <fstream>
using namespace std;

class wxGuiderFileInterface : public wxGuider {
public:
   wxGuiderFileInterface();
   virtual ~wxGuiderFileInterface();

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
   ofstream* m_xdat;
   ofstream* m_ydat;
};



#endif // WXGUIDERFILEINTERFACE_H
