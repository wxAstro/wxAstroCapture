

#ifndef _GP_ST4_H__
#define _GP_ST4_H__


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wxGuiderPort.h"


class GP_ST4_Thread;
class GP_ST4 : public wxGuiderPortProtocol
{
public:
   GP_ST4(wxPortHandle& pHandle);
   virtual ~GP_ST4();

   EPGProtocol Protocol() const {return EPGP_ST4;};

private:
   GP_ST4();
   GP_ST4_Thread* m_pulseThread;
};

#endif // _GP_ST4_H__

