
#include "wx/socket.h"

#include "GP_BRIDGE.h"

#include "../wxFactory.h"
#include "wxGuiderPort.h"


/// GP_BRIDGE_Thread

class GP_BRIDGE_Thread : public wxThread
{
public:
   GP_BRIDGE_Thread(GP_BRIDGE* caller, wxSocketClient* pPortHandle);
   virtual ~GP_BRIDGE_Thread(){};

  //! thread interface
   ExitCode Entry();

   //! Prematurely stop thread execution
   void AbortThread(){m_aborted=true;};

private:
   GP_BRIDGE_Thread();
	GP_BRIDGE*        m_pInterface;    // caller ptr
   wxSocketClient*   m_pPortHandle;   // IO board ptr
   wxLongLong        m_threadStart;
   // some status vars
	int            m_exitCode;
	bool           m_aborted;
};

GP_BRIDGE_Thread::GP_BRIDGE_Thread()
: wxThread(wxTHREAD_JOINABLE)
, m_pInterface(NULL)
, m_pPortHandle(NULL)
, m_threadStart(::wxGetLocalTimeMillis())
, m_exitCode(0)
, m_aborted(true)
{
}

GP_BRIDGE_Thread::GP_BRIDGE_Thread(GP_BRIDGE* caller, wxSocketClient* pPortHandle)
: wxThread(wxTHREAD_JOINABLE)
, m_pInterface(caller)
, m_pPortHandle(pPortHandle)
, m_threadStart(::wxGetLocalTimeMillis())
, m_exitCode(0)
, m_aborted(false)
{
   if (!m_pPortHandle->IsOk()) m_aborted=true; // must have a tcp port
}

wxThread::ExitCode GP_BRIDGE_Thread::Entry()
{
   // Thread actions
static long WAIT_INTERVALL = 500;  // milliseconds

   // INIT
	m_exitCode = 0;            // task result
	bool done = m_aborted;     // task status
   GP_BRIDGE::PulseQueueT queueElement;

   // Sanity check
   if (!m_pPortHandle) {
      m_exitCode = 2;
      done = true;
   }

   /* ----------------------------------------------------------- */
   while(!done) {
      // wxThread obligation
      if ( this->TestDestroy() ) {
         // must terminate prematurely
         m_aborted = true;
      }
      if (m_aborted) {
         // task was aborted
         m_exitCode = 1;
         done = true;  // thread will die now
      }
      else {
         // do really something
         if (wxSEMA_NO_ERROR==m_pInterface->m_semaCommand.WaitTimeout(WAIT_INTERVALL)) {
            if (! m_pPortHandle->IsConnected()) {
               // make sure the TCP port is still connected here
               m_aborted=true; // will terminate the thread as the connection is broken
            }
            else {
               // OK proceed assuming the port remains open for a while
               if ( m_pInterface->ExtractNextCommand(queueElement) ) {
                  // got a command ..
                  /// now do the pulse <M12345678Rs1234Ds1234> total 21 chars
                  char x[25]; unsigned cCnt = 0;
                  unsigned long tim = (::wxGetLocalTimeMillis()-m_threadStart).ToLong();
                  ::sprintf(&x[cCnt], "M%8.8lu", (unsigned long)tim); cCnt+=9; // message id
                  x[cCnt++] = 'R';  // RA
                  x[cCnt++] = (queueElement.dirWrdRA==0) ? '-' : '+';  // RA- : RA+
                  ::sprintf(&x[cCnt], "%4.4lu", (unsigned long)queueElement.pulseDurationRA_ms); cCnt+=4;
                  x[cCnt++] = 'D';  // DEC
                  x[cCnt++] = (queueElement.dirWrdDEC==0) ? '-' : '+';  // DEC- : DEC+
                  ::sprintf(&x[cCnt], "%4.4lu", (unsigned long)queueElement.pulseDurationDEC_ms); cCnt+=4;

                  if (m_pPortHandle->WaitForWrite(5))
                     m_pPortHandle->Write(x, cCnt);
                  // else cannot write ???
               }// connected
            }// got queue element
         }// got a command trigger
      }// not aborted
   }// thread loop

   //Exit(&m_exitCode);   // exit thread
   return &m_exitCode;  // thread completed successfully
}


/// GP_BRIDGE

GP_BRIDGE::GP_BRIDGE()
{
   m_pulseThread = NULL;
}

GP_BRIDGE::GP_BRIDGE(wxSocketClient* pHandle)
{
   m_pulseThread = new GP_BRIDGE_Thread(this, pHandle);
   m_pulseThread->Create();
   m_pulseThread->Run();   // let's run it
}

GP_BRIDGE::~GP_BRIDGE()
{
   m_pulseThread->AbortThread();
   m_pulseThread->Wait(); // will respond if finished
   delete m_pulseThread;

  // parent class wxGuiderPortProtocol cleans up its own stuff here
}


