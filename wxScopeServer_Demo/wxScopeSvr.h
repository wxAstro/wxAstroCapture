/**
 * @file wxScopeSvr.h
 *****************************************************************************
 * wxScopeSvr
 *
 * Definition of a demo Scope Server routine
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

#ifndef _WXSCOPESVR_H__
#define _WXSCOPESVR_H__

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/socket.h"


class wxScopeServerDialog;

class wxScopeSvr : public wxSocketServer
{
public:
   wxScopeSvr(wxScopeServerDialog* dialog, const wxSockAddress& address, wxSocketFlags flags = wxSOCKET_NONE);
   virtual ~wxScopeSvr();

   // call UseParallelGuiding with parameter true if the
   // telescope in question supports parallel guiding, i.e. it
   // is able to handle RA and DEC motors running at the same time.
   // If you are not sure, use the default value
   void UseParallelGuiding(bool parallel = false);

public:

   // InitConnection() shall be called when
   // a client connect is accepted, in order to initialise the timings.
   // The purpose is to be able to detect and ignore delayed guiding messages
   void InitConnection() { m_threadStart = 0; }

   //! returns false on error otherwise true
   bool HandleINPUT(wxSocketBase *sock);

private:
   // Parallel or serial guiding
   // Note that durations are deliberatly signed

   void GuideParallel(int ra_milli, int dec_milli);
   void GuideSerial(int ra_milli, int dec_milli);

private:
   wxScopeServerDialog* m_pDialog;  // ?? good idea ??

   wxLongLong     m_threadStart;  // Estimated absolute time of start
                                  // of clients guiding thread in [ms]
                                  // ref wxGetLocalTimeMillis()

   bool  m_parallel;
};


#endif // _WXSCOPESVR_H__

