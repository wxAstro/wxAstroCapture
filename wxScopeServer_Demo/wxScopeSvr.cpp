/**
 * @file wxScopeSvr.cpp
 *****************************************************************************
 * wxScopeSvr
 *
 * Implementation of a demo Scope Server routine
 *
 *<hr>
 *
 * @b Project	wxScopeServer Demo Project<br>
 *
 * @author     C.A. Arnholm / M. Burri
 * @date       05-Dec-2007
 * Copyright	2007 - C.A. Arnholm / M. Burri<br>
 * rev:			1.0<br>
 *
 * This is copyrighted material - NOT freeware NOR GPL NOR anything else !!!!!
 *****************************************************************************
 *<hr>
 * @b Updates
 *
 *****************************************************************************
 *<hr>
 * License:
 * You are allowed to use the files included in the wxScopeServer Project for
 * any NON commercial (NON shareware etc.) purposes only.
 * If you want to use it for any other purpose you have to obtain a proper license
 * - just call <ca _at_ arnholm _dot_ org>
 * - or call <bm98 _at_ burri-web _dot_ org>
 *
 * You may distribute the files included in the wxScopeServer Demo project included
 * in projects that you have made yourself providing the above paragraph is respected
 * and the copyright and limited warranty is left included in the source files.
 * Please make sure using an appropriate installer to maintain your customers
 * PC in a healthy condition - thanks
 *<br>
 *
 * No warranties whatsoever
 *<br>
 * LIMITED WARRANTY AND DISCLAIMER OF WARRANTY
 * THE Copyright Holder EXPRESSLY DISCLAIMS ANY WARRANTY FOR THE SOFTWARE. THIS SOFTWARE
 * AND THE ACCOMPANYING FILES ARE GIVEN FOR FREE "AS IS" AND WITHOUT WARRANTIES AS TO
 * PERFORMANCE OF MERCHANTABILITY OR ANY OTHER WARRANTIES WHETHER EXPRESSED OR
 * IMPLIED, OR NONINFRINGEMENT.
 *<br>
 * THIS SOFTWARE IS NOT FAULT TOLERANT AND SHOULD NOT BE USED IN ANY ENVIRONMENT
 * WHICH REQUIRES THIS.
 *<br>
 * The Licensed Software is provided AS IS without warranty of any kind. The Copyright Holder
 * makes no further representations or warranties expressly or implied such as by way
 * of example but not of limitation regarding merchant ability or fitness for
 * any particular purpose, or that Licensed Software is error free, or that the use
 * of the Licensed Software or any copies thereof will not infringe any patent,
 * copyright or trademark of third parties.
 *<br>
 * NO LIABILITY FOR DAMAGES. In no event shall the Copyright Holder or agents or its
 * suppliers be liable to you for any consequential, incidental or indirect damages
 * whatsoever (including, without limitation, damages for loss of business profits,
 * business interruption, loss of business information, or any other pecuniary loss)
 * arising out of the use of or inability to use this SOFTWARE EVEN IF the
 * Copyright Holder HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 *<br>
 * The entire risk arising out of use or performance of the SOFTWARE remains with you.
 *<br>
 * Because of the various hardware and software environments into which this software
 * may be put, NO WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE IS OFFERED.
 *<br>
 * The liability of the Copyright Holder for actual damages from any cause whatsoever,
 * and regardless of the form of action (whether in contract, tort (including negligence),
 * product liability or otherwise) WILL BE LIMITED TO ZERO, NOTHING, NIENTE, NADA, NICHTS.
 *<br>
 ******************************************************************************/


#include "wxScopeSvr.h"

#include "wxScopeServerMain.h"


/// wxScopeSvr Main part

wxScopeSvr::wxScopeSvr(wxScopeServerDialog* dialog, const wxSockAddress& address, wxSocketFlags flags)
: wxSocketServer(address, flags)
, m_pDialog(dialog)
, m_threadStart(0)
, m_parallel(false)
{

}

wxScopeSvr::~wxScopeSvr()
{
   // cleanup the rest
}

void wxScopeSvr::UseParallelGuiding(bool parallel)
{
   m_parallel = parallel;
}

//! for the caller: read from socket and queue command
//! returns false on error otherwise true
bool wxScopeSvr::HandleINPUT(wxSocketBase *sock)
{
   // 0123456789a123456789b1
   // M12345678Rs1234Ds1234 --> msgLen=21 chars
   // M<-Time->R+1234D+1234

   // The message contains 21 characters
   // M12345678 = Time in [ms] since start of client's thread
   // Rs1234    = Time in [ms] and direction of RA guiding pulse
   // Ds1234    = Time in [ms] and direction of DE guiding pulse

   const size_t c_len=25;
   const size_t MsgLength = 21;

   char c[c_len]; char dummy[c_len];
   int cCnt=MsgLength;

   memset(c,0,c_len);
   memset(dummy,0,c_len);

   // read MsgLength values
   cCnt=0;
   bool done = false;
   do {
      sock->Read(dummy, MsgLength);
      int err = sock->Error();
      if (!  ( (wxSOCKET_NOERROR==err)
            || (wxSOCKET_INVOP==err  ) )) { // Read if nothing is here raises this one ??? should be WOULDBLOCK...

         return false; // ERROR EXIT
      }
      unsigned dc = sock->LastCount();
      // no full msg available
      if (dc<MsgLength) {
         if (dc) sock->Unread(dummy, dc); // put it back
         done = true;
      }
      else {
         // copy as long as we get messages
         cCnt=dc;
         memcpy(c,dummy,cCnt);
      }
   } while (!done);
//   if (cCnt<MsgLength) return false; // return false for debugging only

/*
   cCnt=MsgLength; cIdx = 0;
   while (cCnt>0) {
      sock->Read(&c[cIdx], cCnt);
      if (wxSOCKET_NOERROR!=sock->Error()) {
         return false; // ERROR EXIT
      }
      unsigned lc = sock->LastCount();
      cCnt-=lc; cIdx+=lc;
     }
*/
   c[MsgLength] = '\n';
 //  m_pDialog->WriteLog(wxString::FromAscii(c));

   // sanity check
   if ( c[0]!='M' || c[9]!='R' || c[15]!='D' ) return false; // ERROR EXIT

   unsigned long  MsgTime = 0;
   c[9]  = 0; MsgTime        = atol(&c[1]);   // Time in [ms] since start of client's thread
   c[15] = 0; int ra_milli   = atoi(&c[10]);  // read with sign
   c[21] = 0; int dec_milli  = atol(&c[16]);  // read with sign

   wxLongLong delay = 0;  // estimated delay in [ms] of this event
   if(m_threadStart == 0) {
      // This is the first guiding instance since InitConnection(),
      // estimate the time since the guiding started.
      // This should work even if the clocks on the machines are not in sync
      m_threadStart = wxGetLocalTimeMillis() - MsgTime;
   }
   else {
      // Check how old this event is, it may have been
      // waiting in the tcp/ip queue. Skip if too old.

      // actual time in [ms] since thread start on this server
      wxLongLong ActualMsgTime = wxGetLocalTimeMillis() - m_threadStart;

      // Compare with similar value passed from client.
      // How much in [ms] is this message delayed in the queue?
      delay = ActualMsgTime - MsgTime;
   }

   // Skip the guiding pulse if the event is more than 2 seconds delayed.
   // Just return and expect the next one to be less old.
   if(delay < 2000) {

      // Do the guiding
      if(m_parallel)   GuideParallel(ra_milli,dec_milli);
      else             GuideSerial(ra_milli,dec_milli);

   }

   return true;
}


void wxScopeSvr::GuideParallel(int ra_milli, int dec_milli)
{
   m_pDialog->WriteLog(wxString::Format(_T("Par [ms]:  RA %05d  DE %05d \n"),ra_milli,dec_milli));

   // The essence of Parallel guiding is to allow both RA and DE motors
   // to be engaged at the same time.

   // NOTE: Some controllers do not allow such parallel guiding
   // and then you must use serial guiding instead

   // ignore if zero only
   if(ra_milli==0 && dec_milli==0)return;

   // start RA guiding
   if     (ra_milli > 0) m_pDialog->RAplus(true);
   else if(ra_milli < 0) m_pDialog->RAminus(true);
   else m_pDialog->RAoff();

   // Start DEC guiding
   if     (dec_milli > 0) m_pDialog->DEplus(true);
   else if(dec_milli < 0) m_pDialog->DEminus(true);
   else m_pDialog->DEoff();

   // durations withou sign
   int ra_dur  = abs(ra_milli);
   int dec_dur = abs(dec_milli);

   // total duration equals longest duration
   // phase1 equals shortest duration
   // phase2 equals difference
   int total(0),phase1(0);
   bool ra_shortest=false;
   if(ra_dur > dec_dur){
      total  = ra_dur;
      phase1 = dec_dur;
      ra_shortest = false;
   }
   else {
      total  = dec_dur;
      phase1 = ra_dur;
      ra_shortest = true;
   }
   int phase2 = total - phase1;

   // let phase1 finish
   ::wxMilliSleep(phase1);
   if(ra_shortest)m_pDialog->RAoff();
   else m_pDialog->DEoff();

   // let phase2 finish
   ::wxMilliSleep(phase2);
   if(ra_shortest)m_pDialog->DEoff();
   else m_pDialog->RAoff();
}

void wxScopeSvr::GuideSerial(int ra_milli, int de_milli)
{
   m_pDialog->WriteLog(wxString::Format(_T("Ser [ms]:  RA %05d   DE %05d\n"),ra_milli,de_milli));

   // In serial guiding we simply process RA first and DE after

   if(abs(ra_milli) > 0) {
      if     (ra_milli > 0) m_pDialog->RAplus(true);
      else if(ra_milli < 0) m_pDialog->RAminus(true);

      ::wxMilliSleep(abs(ra_milli));
      m_pDialog->RAoff();
   }


   if(abs(de_milli) > 0) {
      if     (de_milli > 0) m_pDialog->DEplus(true);
      else if(de_milli < 0) m_pDialog->DEminus(true);

      ::wxMilliSleep(abs(de_milli));
      m_pDialog->DEoff();
   }

}


