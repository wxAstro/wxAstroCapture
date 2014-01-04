

#include "GP_NexStar.h"

#include "../wxFactory.h"


/// GP_NexStar_Thread

class GP_NexStar_Thread : public wxThread
{
public:
   GP_NexStar_Thread(GP_NexStar* caller, wxPortHandle* pPortHandle);
   virtual ~GP_NexStar_Thread(){};

  //! thread interface
   ExitCode Entry();

   //! Prematurely stop thread execution
   void AbortThread(){m_aborted=true;};

private:
   GP_NexStar_Thread();
	GP_NexStar*     m_pInterface;    // caller ptr
   wxPortHandle*   m_pPortHandle;   // IO board ptr

   // some status vars
	int            m_exitCode;
	bool           m_aborted;

};

GP_NexStar_Thread::GP_NexStar_Thread()
: wxThread(wxTHREAD_JOINABLE)
, m_pInterface(NULL)
, m_pPortHandle(NULL)
, m_exitCode(0)
, m_aborted(true)
{
}

GP_NexStar_Thread::GP_NexStar_Thread(GP_NexStar* caller, wxPortHandle* pPortHandle)
: wxThread(wxTHREAD_JOINABLE)
, m_pInterface(caller)
, m_pPortHandle(pPortHandle)
, m_exitCode(0)
, m_aborted(false)
{
   if (!m_pPortHandle->IsPortSerial()) m_aborted=true; // must have a serial port
}

wxThread::ExitCode GP_NexStar_Thread::Entry()
{
   // Thread actions
static long WAIT_INTERVALL = 500;  // milliseconds

   // INIT
	m_exitCode = 0;            // task result
	bool done = m_aborted;     // task status
   GP_NexStar::PulseQueueT queueElement;

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
               // setup the pulse
               unsigned char x, buf[50]; unsigned cCnt = 0;
               if (queueElement.pulseDurationRA_ms>0) {
                  if (queueElement.dirWrdRA==0) {// RA-
                     buf[cCnt]='P';cCnt++; buf[cCnt]=2;cCnt++; buf[cCnt]=16;cCnt++; buf[cCnt]=37;cCnt++;
                     buf[cCnt]=1;cCnt++; buf[cCnt]=0;cCnt++; buf[cCnt]=0;cCnt++; buf[cCnt]=0;cCnt++;
                  }
                  else {// RA+
                     buf[cCnt]='P';cCnt++; buf[cCnt]=2;cCnt++; buf[cCnt]=16;cCnt++; buf[cCnt]=36;cCnt++;
                     buf[cCnt]=1;cCnt++; buf[cCnt]=0;cCnt++; buf[cCnt]=0;cCnt++; buf[cCnt]=0;cCnt++;
                  }
                  for (unsigned c=0; c<cCnt; c++) {
                     wxF()->portManager()->PTx(*m_pPortHandle, buf[c]);
                  }//for
                  wxF()->portManager()->PRx(*m_pPortHandle, x); // replies with #

                  Sleep(queueElement.pulseDurationRA_ms);

                  // stop the pulse
                  buf[4]=0;buf[12]=0;
                  for (unsigned c=0; c<cCnt; c++) {
                     wxF()->portManager()->PTx(*m_pPortHandle, buf[c]);
                  }//for
                  wxF()->portManager()->PRx(*m_pPortHandle, x); // replies with #
               }//RA guiding
               //one direction after the other
               cCnt=0;
               if (queueElement.pulseDurationDEC_ms>0) {
                  if (queueElement.dirWrdDEC==0) {// DEC-
                     buf[cCnt]='P';cCnt++; buf[cCnt]=2;cCnt++; buf[cCnt]=17;cCnt++; buf[cCnt]=37;cCnt++;
                     buf[cCnt]=1;cCnt++; buf[cCnt]=0;cCnt++; buf[cCnt]=0;cCnt++; buf[cCnt]=0;cCnt++;
                  }
                  else {// DEC+
                     buf[cCnt]='P';cCnt++; buf[cCnt]=2;cCnt++; buf[cCnt]=17;cCnt++; buf[cCnt]=36;cCnt++;
                     buf[cCnt]=1;cCnt++; buf[cCnt]=0;cCnt++; buf[cCnt]=0;cCnt++; buf[cCnt]=0;cCnt++;
                  }
                  for (unsigned c=0; c<cCnt; c++) {
                     wxF()->portManager()->PTx(*m_pPortHandle, buf[c]);
                  }//for
                  wxF()->portManager()->PRx(*m_pPortHandle, x); // replies with #

                  Sleep(queueElement.pulseDurationDEC_ms);

                  // stop the pulse
                  buf[4]=0;buf[12]=0;
                  for (unsigned c=0; c<cCnt; c++) {
                     wxF()->portManager()->PTx(*m_pPortHandle, buf[c]);
                  }//for
                  wxF()->portManager()->PRx(*m_pPortHandle, x); // replies with #
               }//DEC guiding
            }// got queue element
         }// got a command trigger
      }// not aborted
   }// thread loop

   //Exit(&m_exitCode);   // exit thread
   return &m_exitCode;  // thread completed successfully
}


/// GP_NexStar

GP_NexStar::GP_NexStar()
: m_pulseThread(NULL)
{
}

GP_NexStar::GP_NexStar(wxPortHandle& pHandle)
: m_pulseThread(NULL)
{
   m_pulseThread = new GP_NexStar_Thread(this, &pHandle);
   m_pulseThread->Create();
   m_pulseThread->Run();   // let's run it
}

GP_NexStar::~GP_NexStar()
{
   m_pulseThread->AbortThread();
   m_pulseThread->Wait(); // will respond if finished
   delete m_pulseThread; m_pulseThread = NULL;


   // parent class wxGuiderPortProtocol cleans up its own stuff here
   // m_pulseQueue.clear();  // don't do this here
}


