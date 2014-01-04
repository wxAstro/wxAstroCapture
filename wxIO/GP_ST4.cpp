

#include "GP_ST4.h"

#include "../wxFactory.h"

/// GP_ST4_Thread

class GP_ST4_Thread : public wxThread
{
public:
   GP_ST4_Thread(GP_ST4* caller, wxPortHandle* pPortHandle);
   virtual ~GP_ST4_Thread(){};

  //! thread interface
   ExitCode Entry();

   //! Prematurely stop thread execution
   void AbortThread(){m_aborted=true;};

private:
   GP_ST4_Thread();
	GP_ST4*         m_pInterface;    // caller ptr
   wxPortHandle*   m_pPortHandle;   // IO board ptr

   // some status vars
	int            m_exitCode;
	bool           m_aborted;

};

GP_ST4_Thread::GP_ST4_Thread()
: wxThread(wxTHREAD_JOINABLE)
, m_pInterface(NULL)
, m_pPortHandle(NULL)
, m_exitCode(0)
, m_aborted(true)
{
}

GP_ST4_Thread::GP_ST4_Thread(GP_ST4* caller, wxPortHandle* pPortHandle)
: wxThread(wxTHREAD_JOINABLE)
, m_pInterface(caller)
, m_pPortHandle(pPortHandle)
, m_exitCode(0)
, m_aborted(false)
{
   // must have a parallel type port
   if ( ! (m_pPortHandle->IsPortParallel() || m_pPortHandle->IsPortGPUSB() ) ) m_aborted=true;
}

wxThread::ExitCode GP_ST4_Thread::Entry()
{
   // Thread actions
static long WAIT_INTERVALL = 500;  // milliseconds

   // INIT
	m_exitCode = 0;            // task result
	bool done = m_aborted;     // task status
   GP_ST4::PulseQueueT queueElement;

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
            // get access to the command queue
            if ( m_pInterface->ExtractNextCommand(queueElement) ) {
               /// now do the pulse
               if (queueElement.pulseDurationRA_ms>0) {
                  // the direction arrives as proper bits set by wxGuiderPort already
                  wxF()->portManager()->PPut(*m_pPortHandle, queueElement.dirWrdRA);

                  Sleep(queueElement.pulseDurationRA_ms);

                  // stop the pulse
                  wxF()->portManager()->PPut(*m_pPortHandle, queueElement.stopWrd);
               }//RA guiding
               //one direction after the other
               if (queueElement.pulseDurationDEC_ms>0) {
                  // the direction arrives as proper bits set by wxGuiderPort already
                  wxF()->portManager()->PPut(*m_pPortHandle, queueElement.dirWrdDEC);

                  Sleep(queueElement.pulseDurationDEC_ms);

                  // stop the pulse
                  wxF()->portManager()->PPut(*m_pPortHandle, queueElement.stopWrd);
               }//DEC guiding
            }// got queue element
         }// got a command trigger
      }// not aborted
   }// thread loop

   //Exit(&m_exitCode);   // exit thread
   return &m_exitCode;  // thread completed successfully
}


/// GP_ST4

GP_ST4::GP_ST4()
: m_pulseThread(NULL)
{
}

GP_ST4::GP_ST4(wxPortHandle& pHandle)
: m_pulseThread(NULL)
{
   m_pulseThread = new GP_ST4_Thread(this, &pHandle);
   m_pulseThread->Create();
   m_pulseThread->Run();   // let's run it
}

GP_ST4::~GP_ST4()
{
   m_pulseThread->AbortThread();
   m_pulseThread->Wait(); // will respond if finished
   delete m_pulseThread;  m_pulseThread = NULL;

   // parent class wxGuiderPortProtocol cleans up its own stuff here
   // m_pulseQueue.clear();  // don't do this here
}


