

#ifndef _GP_MTS_3SDI_H__
#define _GP_MTS_3SDI_H__


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wxGuiderPort.h"


class GP_MTS_3SDI_Thread;
class GP_MTS_3SDI : public wxGuiderPortProtocol
{
public:
   GP_MTS_3SDI(wxPortHandle& pHandle);
   virtual ~GP_MTS_3SDI();

   EPGProtocol Protocol() const {return EPGP_MTS_3SDI;};

private:
   GP_MTS_3SDI();
   GP_MTS_3SDI_Thread* m_pulseThread;
};

#endif // _GP_ST4_H__

