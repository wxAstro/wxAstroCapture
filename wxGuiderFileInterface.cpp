#include "wxGuiderFileInterface.h"
#include "wxGuidingConfig.h"

#include <time.h>
#include <math.h>



wxGuiderFileInterface::wxGuiderFileInterface()
: m_xdat(0)
, m_ydat(0)
{
   //ctor
}

wxGuiderFileInterface::~wxGuiderFileInterface()
{
   guideEnd();
}


bool wxGuiderFileInterface::guideStart()
{
   guideEnd();

   wxString path = wxGuidingConfig::getFileInterfacePath();
   wxChar last = path.Last();

   // check the last character
   if((last != '\\') && (last != '/'))path += '/';

   wxString xpath = path + wxT("X.DAT");
   wxString ypath = path + wxT("Y.DAT");
   m_xdat = new ofstream(xpath.char_str());
   m_ydat = new ofstream(ypath.char_str());

   if(m_xdat && m_ydat){
      set_guiding_active(true);
      return true;
   }
   return false;
}

bool wxGuiderFileInterface::guideError(unsigned long timestamp  // (in)  Time stamp   [ms]
                                      ,double ra                // (in)  Error in RA  [arcseconds]
                                      ,double dec               // (in)  Error in DEC [arcseconds]
                                      )
{
   if(m_xdat && m_ydat) {
      //CA:20090412 - reports now ms
      double seconds=(double)timestamp/1000.0;

      wxString xf,yf;
      xf.Printf(wxT(" %8.2f  %8.2f"),seconds, ra);
      yf.Printf(wxT(" %8.2f  %8.2f"),seconds,dec);

      // rewind x.dat and write new line
      m_xdat->seekp (0, ios::beg);
      (*m_xdat) << xf.char_str() << endl;

      // rewind y.dat and write new line
      m_ydat->seekp (0, ios::beg);
      (*m_ydat) << yf.char_str() << endl;

      return true;
   }
   return false;
}

// end a guiding sequence
bool wxGuiderFileInterface::guideEnd()
{
   delete m_xdat; m_xdat = 0;
   delete m_ydat; m_ydat = 0;
   set_guiding_active(false);
   return true;
}

wxGuider::guider_interface wxGuiderFileInterface::interface_type()
{
   return FILE_INTERFACE;
}
