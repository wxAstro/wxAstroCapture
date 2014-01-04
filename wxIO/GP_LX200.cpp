

#include "GP_LX200.h"

#include "../wxFactory.h"


/// GP_LX200_Thread

class GP_LX200_Thread : public wxThread
{
public:
   // sendQx is a modifier to change the protocol sent by the driver
   //  it will cause to send explicitely :Qx# where x is the direction [ensw]
   //  if false it will send only the general stop command :Q#
   GP_LX200_Thread(GP_LX200* caller, wxPortHandle* pPortHandle, bool sendQx);
   virtual ~GP_LX200_Thread(){};

  //! thread interface
   ExitCode Entry();

   //! Prematurely stop thread execution
   void AbortThread(){m_aborted=true;};

private:
   GP_LX200_Thread();
	GP_LX200*       m_pInterface;    // caller ptr
   wxPortHandle*   m_pPortHandle;   // IO board ptr

   // some status vars
	int            m_exitCode;
	bool           m_aborted;
	bool           m_sendQX;

};

GP_LX200_Thread::GP_LX200_Thread()
: wxThread(wxTHREAD_JOINABLE)
, m_pInterface(NULL)
, m_pPortHandle(NULL)
, m_exitCode(0)
, m_aborted(true)
, m_sendQX(false)
{
}

GP_LX200_Thread::GP_LX200_Thread(GP_LX200* caller, wxPortHandle* pPortHandle, bool sendQx)
: wxThread(wxTHREAD_JOINABLE)
, m_pInterface(caller)
, m_pPortHandle(pPortHandle)
, m_exitCode(0)
, m_aborted(false)
, m_sendQX(sendQx)
{
   if (!m_pPortHandle->IsPortSerial()) m_aborted=true; // must have a serial port
}

wxThread::ExitCode GP_LX200_Thread::Entry()
{
   // Thread actions
static long WAIT_INTERVALL = 500;  // milliseconds

   // INIT
	m_exitCode = 0;            // task result
	bool done = m_aborted;     // task status
   GP_LX200::PulseQueueT queueElement;

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
         // do really something
         if (wxSEMA_NO_ERROR==m_pInterface->m_semaCommand.WaitTimeout(WAIT_INTERVALL)) {
            // get access to the command queue
            if ( m_pInterface->ExtractNextCommand(queueElement) ) {

               /// now do the pulse
               // setup the pulse
               char buf[50]; unsigned cCnt = 0;
               if (queueElement.pulseDurationRA_ms>0) {
                  if (queueElement.dirWrdRA==0) ::strcpy(&buf[cCnt], ":Mw#");  // RA-
                  else                          ::strcpy(&buf[cCnt], ":Me#");  // RA+
                  cCnt+=4;

                  for (unsigned c=0; c<cCnt; c++) {
                     wxF()->portManager()->PTx(*m_pPortHandle, buf[c]);
                  }//for

                  Sleep(queueElement.pulseDurationRA_ms);

                  // stop the pulse
                  cCnt=0;
                  if (m_sendQX) {
                     // new end explicitely
                     if (queueElement.dirWrdRA==0) ::strcpy(&buf[cCnt], ":Qw#");  // RA-
                     else                          ::strcpy(&buf[cCnt], ":Qe#");  // RA+
                     cCnt+=4;
                  }
                  else {
                     ::strcpy(&buf[cCnt], ":Q#"); cCnt+=3;
                  }
                  for (unsigned c=0; c<cCnt; c++) {
                     wxF()->portManager()->PTx(*m_pPortHandle, buf[c]);
                  }//for
               }//RA guiding
               //one direction after the other
               cCnt=0;
               if (queueElement.pulseDurationDEC_ms>0) {
                  if (queueElement.dirWrdDEC==0) ::strcpy(&buf[cCnt], ":Ms#");  // DEC-
                  else                           ::strcpy(&buf[cCnt], ":Mn#");  // DEC+
                  cCnt+=4;

                  for (unsigned c=0; c<cCnt; c++) {
                     wxF()->portManager()->PTx(*m_pPortHandle, buf[c]);
                  }//for

                  Sleep(queueElement.pulseDurationDEC_ms);

                  // stop the pulse
                  cCnt=0;
                  if (m_sendQX) {
                     // new end explicitely
                     if (queueElement.dirWrdDEC==0) ::strcpy(&buf[cCnt], ":Qs#");  // DEC-
                     else                           ::strcpy(&buf[cCnt], ":Qn#");  // DEC+
                     cCnt+=4;
                  }
                  else {
                     ::strcpy(&buf[cCnt], ":Q#"); cCnt+=3;
                  }
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


/// GP_LX200

GP_LX200::GP_LX200()
: m_pulseThread(NULL)
, m_sendQX(false)
{
}

// sendQx is a modifier to change the protocol sent by the driver
//  it will cause to send explicitely :Qx# where x is the direction [ensw]
//  if false it will send only the general stop command :Q#
GP_LX200::GP_LX200(wxPortHandle& pHandle, bool sendQx)
: m_pulseThread(NULL)
, m_sendQX(sendQx)
{
   m_pulseThread = new GP_LX200_Thread(this, &pHandle, sendQx);
   m_pulseThread->Create();
   m_pulseThread->Run();   // let's run it
}

GP_LX200::~GP_LX200()
{
   m_pulseThread->AbortThread();
   m_pulseThread->Wait(); // will respond if finished
   delete m_pulseThread; m_pulseThread=NULL;

   // parent class wxGuiderPortProtocol cleans up its own stuff here
   // m_pulseQueue.clear();  // don't do this here
}

GP_LX200::EPGProtocol GP_LX200::Protocol() const
{
   if (m_sendQX) return EPGP_LX200Qx;
   else          return EPGP_LX200;
}


