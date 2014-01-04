

#ifndef _GP_NexStar_H__
#define _GP_NexStar_H__


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wxGuiderPort.h"


class GP_NexStar_Thread;
class GP_NexStar : public wxGuiderPortProtocol
{
public:
   GP_NexStar(wxPortHandle& pHandle);
   virtual ~GP_NexStar();

   EPGProtocol Protocol() const {return EPGP_NexStar;};

private:
   GP_NexStar();
   GP_NexStar_Thread* m_pulseThread;
};

#endif // _GP_NexStar_H__

