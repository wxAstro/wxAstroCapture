#include "MTS3_PF.h"

#include "wxPortManager.h"
#include <wx/thread.h>

static unsigned char onon_code =0xAE;  // MTS-3 onon  , MTS-4S onoff
static unsigned char onoff_code=0xAF;  // MTS-3 onoff , MTS-4S onon


MTS3_PF::MTS3_PF(wxPortManager* pmgr,wxPortHandle* port)
: m_pmgr(pmgr)
, m_port(port)
, m_conn(false)
{
   // test connection immediately, it should already be on
   m_conn = is_on();

   // We decide to fail if the connection is not established already.
   // The port is typically shared with another application for GOTO,
   // and we leave it to that application to turn the controller ON/OFF.
   // i.e. we consider wxAstoCapture to be a secondary user of the serial port

   // if(! m_conn) onon();

   if(m_conn) {
      // make sure we are tracking normally
      RAtrack();

//   This is for debugging purposes only
/*
      faston();
      float RAslow      = 2.673971f;
      float fast_freq = 32*RAslow;
      // RA
      sendfloat(fast_freq);
      send232(0xBE);

      // DE
      sendfloat(fast_freq);
      send232(0xBF);
*/
   }
   else {
      wxMessageBox(_T("MTS-3 not connected: Either not switched on, or serial port busy?"));
   }
}

MTS3_PF::~MTS3_PF()
{
}

bool MTS3_PF::send232(unsigned char value)
{
   // actually send the value to the port
   return m_pmgr->PTx(*m_port,value);
}

bool MTS3_PF::receive232(unsigned char& value)
{
   // receive value from port
   return m_pmgr->PRx(*m_port,value);
}

unsigned char MTS3_PF::receive232()
{
   unsigned char value=0;
   m_conn = receive232(value);
   return value;
}

void MTS3_PF::sendlong(long value)
{
   if(!m_conn)return;

   // convert the long to a MTS3 compatible representation

   send232(((value >> 28)& 15) + 0x80);
   send232(((value >> 24)& 15) + 0x90);
   send232(((value >> 20)& 15) + 0x80);
   send232(((value >> 16)& 15) + 0x90);
   send232(((value >> 12)& 15) + 0x80);
   send232(((value >> 8)& 15) + 0x90);
   send232(((value >> 4)& 15) + 0x80);
   send232(((value )& 15) + 0x90);
}

void MTS3_PF::sendfloat(float value)
{
   union mix {
      float f;
      long l;
   };
   union mix x;
   x.f=value;
   sendlong(x.l);
}

long MTS3_PF::receivelong()
{
   if(!m_conn)return 0;

   // convert from MTS3 representation

   long x=0;
   x=receive232();
   x=(x<<8) | receive232();
   x=(x<<8) | receive232();
   x=(x<<8) | receive232();
   return x;
}

bool MTS3_PF::ready()    // true if last command is finished
{
   send232(0xA2);
   return (receive232()==0);
}

bool MTS3_PF::sleep_ready()  // sleep until ready
{
   if(!m_conn)return false;

   const int max_count=50;
   int counter=0;
   while(!ready() && counter<max_count)
   {
      wxThread::Sleep(10);
      counter++;
   }
   return (counter<max_count);
}


bool MTS3_PF::onon()     // switch ON
{
   send232(onon_code);
   // check to see if onon was successful.
   // It could be unsuccesful due to incompatibility with MTS-4S

   // check to see if ON button is set
   send232(0xB3);
   unsigned char switches = receive232();
   bool on   = (switches & 0x20) != 0;  // true when MTS-3
   if(!on) {
      // this could be a MTS-4S

      // swap on and off codes
      unsigned char tmp = onon_code;
      onon_code = onoff_code;
      onoff_code = tmp;
      send232(onon_code);

      // check to see if ON button is set
      send232(0xB3);
      switches = receive232();
      on   = (switches & 0x20) != 0;  // true when MTS-3
      if(!on) {
         // this didn't work either, swap back
         tmp = onon_code;
         onon_code = onoff_code;
         onoff_code = tmp;
      }
   }

   m_conn = on;

   return m_conn;
}

void MTS3_PF::readswitch(bool& on, bool& fast, bool& up, bool& down, bool& left, bool& right)
{
   /*
   A3: The return value of readswitch:
   Bit #5: ON (readswitch() & 0x20)
   Bit #4: FAST (readswitch() & 0x10)
   Bit #3: UP (readswitch() & 0x08)
   Bit #2: DOWN (readswitch() & 0x04)
   Bit #1: LEFT (readswitch() & 0x02)
   Bit #0: RIGHT (readswitch() & 0x01)
   */

   send232(0xB3);
   unsigned char switches = receive232();

   on    = (switches & 0x20) != 0;
   fast  = (switches & 0x10) != 0;
   up    = (switches & 0x08) != 0;
   down  = (switches & 0x04) != 0;
   left  = (switches & 0x02) != 0;
   right = (switches & 0x01) != 0;
}


bool MTS3_PF::is_on()     // true when switched ON
{
   // do not check for connection here
   bool on, fast, up, down,left, right;
   readswitch(on, fast, up, down,left, right);
   return on;
}


bool MTS3_PF::is_fast()  // true when in FAST mode
{
   if(!sleep_ready())return false;

   bool on, fast, up, down,left, right;
   readswitch(on, fast, up, down,left, right);
   return fast;
}


bool MTS3_PF::onoff()    // switch OFF
{
   if(!sleep_ready())return false;
   return send232(onoff_code);
}

bool MTS3_PF::faston()   // switch FAST
{
   if(!sleep_ready())return false;
   return send232(0xB0);
}

bool MTS3_PF::fastoff()  // switch SLOW
{
   if(!sleep_ready())return false;
   return send232(0xB1);
}

bool MTS3_PF::lefton()   // push LEFT key
{
   if(!sleep_ready())return false;
   return send232(0xA8);
}

bool MTS3_PF::leftoff()  // release LEFT key
{
   if(!sleep_ready())return false;
   return send232(0xA9);
}

bool MTS3_PF::righton()  // push RIGHT key
{
   if(!sleep_ready())return false;
   return send232(0xAA);
}

bool MTS3_PF::rightoff() // release RIGHT key
{
   if(!sleep_ready())return false;
   return send232(0xAB);
}

bool MTS3_PF::upon()     // push UP key
{
   if(!sleep_ready())return false;
   return send232(0xA4);
}

bool MTS3_PF::upoff()    // release UP key
{
   if(!sleep_ready())return false;
   return send232(0xA5);
}

bool MTS3_PF::downon()   // push DOWN key
{
   if(!sleep_ready())return false;
   return send232(0xA6);
}

bool MTS3_PF::downoff()  // release DOWN key
{
   if(!sleep_ready())return false;
   return send232(0xA7);
}

bool MTS3_PF::RAtrack()  // start tracking (SLOW) in RA
{
   if(!sleep_ready())return false;
   return send232(0xC5);
}

bool MTS3_PF::guide(long ra_ms, long de_ms)
{
/*
   ra_ms : a signed value indicating direction and duration [ms] of RA guiding
             positive = LEFT
             negative = RIGHT

   de_ms : a signed value indicating direction and duration [ms] of DE guiding
             positive = DOWN
             negative = UP
*/

   // ignore if zero only
   if(ra_ms==0 && de_ms==0)return true;

   bool right(false),up(false);

   // start RA guiding, unless the pulse is zero
   if (ra_ms >     0)  { lefton();  right = false;  }
   else if(ra_ms < 0)  { righton(); right = true;   }

   // Start DEC guiding, unless the pulse is zero
   if      (de_ms > 0) { downon();  up    = false;  }
   else if (de_ms < 0) { upon();    up    = true;   }

   // durations without sign
   int ra_dur  = abs(ra_ms);
   int dec_dur = abs(de_ms);

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

   if(phase1 > 0) {
      // let phase1 finish
      wxThread::Sleep(phase1);
      if(ra_shortest)RAguideoff(right);
      else DEguideoff(up);
   }

   // let phase2 finish
   wxThread::Sleep(phase2);

   // make sure all guiding is off
   DEguideoff(up);
   RAguideoff(right);

   return true;
}


void MTS3_PF::RAguideoff(bool right)
{
   if(right)rightoff();
   else     leftoff();
}

void MTS3_PF::DEguideoff(bool up)
{
   if(up)upoff();
   else  downoff();
}
