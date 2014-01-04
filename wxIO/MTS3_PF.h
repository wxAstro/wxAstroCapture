#ifndef MTS3_PF_H
#define MTS3_PF_H

/*
    MTS3_PF implements the PowerFlex protocol used in V2.xx firmware of the MTS3_xxx controllers.

    Documentation of the PowerFlex protocol corresponding to V2.xx firmware:
    Complete documentation is found in Chapter 10 of http://www.boxdoerfer.de/manualm3.pdf

    Notice that this implementation is incomplete wrt. the above protocol, it
    handles only the minimum required for autoguiding to work.

    Notice also that V3.xx firmware of the same MTS3_xxx controllers is
    NOT compatible with the Powerflex protocol. The V3.xx firmware
    implements the LX200 protocol instead, so for that situation, use
    LX200PG (Pulse Guide) protocol instead of the MTS3 PowerFlex protocol.

    Author: Carsten Arnholm, 2007

*/

class wxPortManager;
class wxPortHandle;

class MTS3_PF {
public:
   MTS3_PF(wxPortManager* pmgr,wxPortHandle* port );
   virtual ~MTS3_PF();

   bool lefton();   // push LEFT key
   bool leftoff();  // release LEFT key

   bool righton();  // push RIGHT key
   bool rightoff(); // release RIGHT key

   bool upon();     // push UP key
   bool upoff();    // release UP key

   bool downon();   // push DOWN key
   bool downoff();  // release DOWN key

   bool is_on();    // true when switched ON

/*
   Perform parallel guiding, i.e. both motors running at the same time

   ra_ms : a signed value indicating direction and duration [ms] of RA guiding
             positive = LEFT
             negative = RIGHT

   de_ms : a signed value indicating direction and duration [ms] of DE guiding
             positive = DOWN
             negative = UP
*/
   bool guide(long ra_ms, long de_ms);

private:
   bool RAtrack();  // start tracking (SLOW) in RA

   // readout of key switches
   void readswitch(bool& on, bool& fast, bool& up, bool& down, bool& left, bool& right);

   bool is_fast();  // true when FAST enebled

   bool onon();     // switch ON
   bool onoff();    // switch OFF

   bool faston();   // switch FAST
   bool fastoff();  // switch SLOW

   bool ready();    // true if last command is finished
   bool sleep_ready(); // sleep until ready

   void RAguideoff(bool right);
   void DEguideoff(bool up);

private:
   // fundamental serial IO
   bool send232(unsigned char value);
   bool receive232(unsigned char& value);
   unsigned char receive232();

   // high level protocol IO for long and float
   void sendlong(long value);
   void sendfloat(float value);
   long receivelong();

private:
   wxPortManager* m_pmgr;     // all IO via this object
   wxPortHandle*  m_port;     // the port chosen
   bool           m_conn;     // true when connection appears successful
};



#endif // MTS3_PF_H
