

#include "GP_LX200PG.h"

#include "../wxFactory.h"

/// GP_LX200PG_Thread

class GP_LX200PG_Thread : public wxThread
{
public:
   GP_LX200PG_Thread(GP_LX200PG* caller, wxPortHandle* pPortHandle);
   virtual ~GP_LX200PG_Thread(){};

  //! thread interface
   ExitCode Entry();

   //! Prematurely stop thread execution
   void AbortThread(){m_aborted=true;};

private:
   GP_LX200PG_Thread();
	GP_LX200PG*     m_pInterface;    // caller ptr
   wxPortHandle*   m_pPortHandle;   // IO board ptr

   // some status vars
	int            m_exitCode;
	bool           m_aborted;

};

GP_LX200PG_Thread::GP_LX200PG_Thread()
: wxThread(wxTHREAD_JOINABLE)
, m_pInterface(NULL)
, m_pPortHandle(NULL)
, m_exitCode(0)
, m_aborted(true)
{
}

GP_LX200PG_Thread::GP_LX200PG_Thread(GP_LX200PG* caller, wxPortHandle* pPortHandle)
: wxThread(wxTHREAD_JOINABLE)
, m_pInterface(caller)
, m_pPortHandle(pPortHandle)
, m_exitCode(0)
, m_aborted(false)
{
   if (!m_pPortHandle->IsPortSerial()) m_aborted=true; // must have a serial port
}

wxThread::ExitCode GP_LX200PG_Thread::Entry()
{
   // Thread actions
static long WAIT_INTERVALL = 500;  // milliseconds

   // INIT
	m_exitCode = 0;            // task result
	bool done = m_aborted;     // task status
   GP_LX200PG::PulseQueueT queueElement;

   // Sanity check
   if (!m_pPortHandle) {
      m_exitCode = 2;
      done = true;
   }

   {
      // set scope to guidespeed - sending some # terminators should put the scope into pace
      char buf[50]; unsigned cCnt = 0;
      ::strcpy(&buf[cCnt], "###:RG##");
      cCnt+=8;
      for (unsigned c=0; c<cCnt; c++) {
         wxF()->portManager()->PTx(*m_pPortHandle, buf[c]);
      }//for
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
         // PG for LX200 GPS mount Commands include;
         // Mgexxxx, Mgwxxxx, Mgnxxxx, Mgsxxxx , where "xxxx" are correction pulse width in milliseconds.
         // do really something
         if (wxSEMA_NO_ERROR==m_pInterface->m_semaCommand.WaitTimeout(WAIT_INTERVALL)) {
            // get access to the command queue
            if ( m_pInterface->ExtractNextCommand(queueElement) ) {

               /// now do the pulse
               // setup the pulse
               char buf[50]; unsigned cCnt = 0;
               if (queueElement.pulseDurationRA_ms>0) {
                  if (queueElement.dirWrdRA==0) ::sprintf(&buf[cCnt], ":Mgw%4.4lu#", queueElement.pulseDurationRA_ms); // RA-
                  else                          ::sprintf(&buf[cCnt], ":Mge%4.4lu#", queueElement.pulseDurationRA_ms); // RA+
                  cCnt+=9;

                  for (unsigned c=0; c<cCnt; c++) {
                     wxF()->portManager()->PTx(*m_pPortHandle, buf[c]);
                  }//for
               }//RA guiding

               //one direction after the other -but without wait
               cCnt = 0;
               if (queueElement.pulseDurationDEC_ms>0) {
                  if (queueElement.dirWrdDEC==0) ::sprintf(&buf[cCnt], ":Mgs%4.4lu#", queueElement.pulseDurationDEC_ms); // DEC-
                  else                           ::sprintf(&buf[cCnt], ":Mgn%4.4lu#", queueElement.pulseDurationDEC_ms); // DEC+
                  cCnt+=9;

                  for (unsigned c=0; c<cCnt; c++) {
                     wxF()->portManager()->PTx(*m_pPortHandle, buf[c]);
                  }//for
               }//DEC guiding
            }// got queue element
         }// got a command trigger
      }// not aborted
   }// thread loop

   //Exit(&m_exitCode);   // exit thread
   return &m_exitCode;  // thread completed successfully
}


/// GP_LX200PG

GP_LX200PG::GP_LX200PG()
{
   m_pulseThread = NULL;
}

GP_LX200PG::GP_LX200PG(wxPortHandle& pHandle)
{
   m_pulseThread = new GP_LX200PG_Thread(this, &pHandle);
   m_pulseThread->Create();
   m_pulseThread->Run();   // let's run it
}

GP_LX200PG::~GP_LX200PG()
{
   m_pulseThread->AbortThread();
   m_pulseThread->Wait(); // will respond if finished
   delete m_pulseThread;

   // parent class wxGuiderPortProtocol cleans up its own stuff here
   // m_pulseQueue.clear();  // don't do this here
}



