

#ifndef _GP_LX200PG_H__
#define _GP_LX200PG_H__


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wxGuiderPort.h"


class GP_LX200PG_Thread;
class GP_LX200PG : public wxGuiderPortProtocol
{
public:
   GP_LX200PG(wxPortHandle& pHandle);
   virtual ~GP_LX200PG();

   EPGProtocol Protocol() const {return EPGP_LX200PG;};

private:
   GP_LX200PG();
   GP_LX200PG_Thread* m_pulseThread;
};

#endif // _GP_LX200PG_H__

