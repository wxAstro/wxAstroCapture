

#include "GP_MTS_3SDI.h"

#include "../wxFactory.h"

#include "MTS3_PF.h"

/// GP_MTS_3SDI_Thread

class GP_MTS_3SDI_Thread : public wxThread
{
public:
   GP_MTS_3SDI_Thread(GP_MTS_3SDI* caller, wxPortHandle* pPortHandle);
   virtual ~GP_MTS_3SDI_Thread();

  //! thread interface
   ExitCode Entry();

   //! Prematurely stop thread execution
   void AbortThread(){m_aborted=true;};

private:
   GP_MTS_3SDI_Thread();
	GP_MTS_3SDI*    m_pInterface;    // caller ptr
   wxPortHandle*   m_pPortHandle;   // IO board ptr
   MTS3_PF*        m_mts;           // MTS3 PowerFlex io protocol

   // some status vars
	int            m_exitCode;
	bool           m_aborted;

};

GP_MTS_3SDI_Thread::~GP_MTS_3SDI_Thread()
{
   delete m_mts;
}

GP_MTS_3SDI_Thread::GP_MTS_3SDI_Thread()
: wxThread(wxTHREAD_JOINABLE)
, m_pInterface(NULL)
, m_pPortHandle(NULL)
, m_mts(NULL)
, m_exitCode(0)
, m_aborted(true)
{
}

GP_MTS_3SDI_Thread::GP_MTS_3SDI_Thread(GP_MTS_3SDI* caller, wxPortHandle* pPortHandle)
: wxThread(wxTHREAD_JOINABLE)
, m_pInterface(caller)
, m_pPortHandle(pPortHandle)
, m_mts(NULL)
, m_exitCode(0)
, m_aborted(false)
{
   if (!m_pPortHandle->IsPortSerial()) m_aborted=true; // must have a serial port

   // NOTICE: The port baudrate MUST be set to 9600!

   m_mts = new MTS3_PF(wxF()->portManager(),m_pPortHandle);
   if(!m_mts->is_on())m_aborted=true;
}

wxThread::ExitCode GP_MTS_3SDI_Thread::Entry()
{
   // Thread actions
static long WAIT_INTERVALL = 500;  // milliseconds

   // INIT
	m_exitCode = 0;            // task result
	bool done = m_aborted;     // task status
   GP_MTS_3SDI::PulseQueueT queueElement;

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

               // convert the guide info into signed ms durations, indicating directions
               long ra_sign = (queueElement.dirWrdRA  == 0)? -1 : +1;
               long de_sign = (queueElement.dirWrdDEC == 0)? -1 : +1;
               long ra_ms   = ra_sign*queueElement.pulseDurationRA_ms;
               long de_ms   = de_sign*queueElement.pulseDurationDEC_ms;

               // actually do the guiding (motors in parallel)
               m_mts->guide(ra_ms,de_ms);

            }// got queue element
         }// got a command trigger
      }// not aborted
   }// thread loop

   //Exit(&m_exitCode);   // exit thread
   return &m_exitCode;  // thread completed successfully
}


/// GP_MTS_3SDI

GP_MTS_3SDI::GP_MTS_3SDI()
: m_pulseThread(NULL)
{
}

GP_MTS_3SDI::GP_MTS_3SDI(wxPortHandle& pHandle)
: m_pulseThread(NULL)
{
   m_pulseThread = new GP_MTS_3SDI_Thread(this, &pHandle);
   m_pulseThread->Create();
   m_pulseThread->Run();   // let's run it
}

GP_MTS_3SDI::~GP_MTS_3SDI()
{
   m_pulseThread->AbortThread();
   m_pulseThread->Wait(); // will respond if finished
   delete m_pulseThread; m_pulseThread=NULL;


   // parent class wxGuiderPortProtocol cleans up its own stuff here
   // m_pulseQueue.clear();  // don't do this here
}


