

#ifndef _GP_BRIDGE_H__
#define _GP_BRIDGE_H__


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wxGuiderPortProtocol.h"

class wxSocketClient;
class GP_BRIDGE_Thread;
class GP_BRIDGE : public wxGuiderPortProtocol
{
public:
   GP_BRIDGE(wxSocketClient* pHandle);
   virtual ~GP_BRIDGE();

   EPGProtocol Protocol() const {return EPGP_BRIDGE;};

private:
   GP_BRIDGE();
   GP_BRIDGE_Thread* m_pulseThread;
};

#endif // _GP_LX200_H__

