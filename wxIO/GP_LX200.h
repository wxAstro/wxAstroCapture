

#ifndef _GP_LX200_H__
#define _GP_LX200_H__


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wxGuiderPort.h"

class GP_LX200_Thread;
class GP_LX200 : public wxGuiderPortProtocol
{
public:
   // sendQx is a modifier to change the protocol sent by the driver
   //  it will cause to send explicitely :Qx# where x is the direction [ensw]
   //  if false it will send only the general stop command :Q#
   GP_LX200(wxPortHandle& pHandle, bool sendQx=false);
   virtual ~GP_LX200();

   EPGProtocol Protocol() const;

private:
   GP_LX200();
   GP_LX200_Thread* m_pulseThread;
  	bool             m_sendQX;
};

#endif // _GP_LX200_H__

