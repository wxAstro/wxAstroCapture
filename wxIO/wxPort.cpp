// wxPort.cpp: implementation of the wxPort class.
//
//////////////////////////////////////////////////////////////////////


#include "wxPort.h"

#if defined(_WXMSW_ )
 #include "PortW32.h"
 #include "PortEnumW32.h"
#else
 #include "PortUX.h"
 #include "PortEnumUX.h"
#endif


wxPort* wxPort::Construct(const wxPortDescr& port_entry)
{
   wxPort* port = 0;
   wxPort::PortAccessStatus status;
   #if defined(_WXMSW_ )
      port = new PortW32();
//      status = dynamic_cast<PortW32*>(port)->POpen(port_entry);
   #else
      port = new PortUX();
//      status = dynamic_cast<PortUX*>(port)->POpen(port_entry);
   #endif

   status = port->POpen(port_entry);
   if(status == wxPort::IPAC_Success){
      //port->PPut(0xff); // all lines high ?? NO depends on usage
   }
   else {
      wxMessageBox(_T("Error accessing port: " + port_entry.PortName()));
   }
   return port;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

wxPort::wxPort()
: m_lastError(IPAC_NoInterface)
, m_numbits(0)
, m_bitmask(0)
, m_baudrate(EBR_9600)
, m_handshake(ELD_None)
, m_outvalue(0)
, m_portOutput(false)
, m_portInput(false)
, m_portParallel(false)
, m_portSerial(false)
, m_portUSB_GPUSB(false)//BM:20081226 added
, m_portUSB_LXUSB(false)//BM:20081226 added
, m_portUSB_LXLED(false)//BM:20081226 added
, m_portDummy(true)
, m_released(false)
, m_portName(wxT(""))
, m_portPath(wxT(""))
{

}

void wxPort::SetPortOut(bool newVal) {m_portOutput=newVal;}
void wxPort::SetPortIn(bool newVal) {m_portInput=newVal;}

void wxPort::SetPortParallel(bool newVal) {m_portParallel=newVal;}
void wxPort::SetPortSerial(bool newVal) {m_portSerial=newVal;}
void wxPort::SetPortGPUSB(bool newVal) {m_portUSB_GPUSB=newVal;}//BM:20081226 added
void wxPort::SetPortLXUSB(bool newVal) {m_portUSB_LXUSB=newVal;}//BM:20081226 added
void wxPort::SetPortLXLED(bool newVal) {m_portUSB_LXLED=newVal;}//BM:20081226 added
void wxPort::SetPortDummy(bool newVal) {m_portDummy=newVal;}

bool wxPort::Released() {return m_released;};
void wxPort::SetReleased(bool newVal) {m_released=newVal;}

void wxPort::SetNumbits(wxUint16 nbits) {m_numbits=nbits;};
void wxPort::SetBitmask(wxUint16 bmask) {m_bitmask=bmask;};

void wxPort::SetBaudRate(EBaudRate baudrate) {m_baudrate=baudrate;};
wxPort::EBaudRate wxPort::BaudRate() {return m_baudrate;};
void wxPort::SetHandshake(EHandshake handshake) {m_handshake=handshake;};
wxPort::EHandshake wxPort::Handshake() {return m_handshake;};

wxUint16 wxPort::MergedOutValue(wxUint16 pValue, wxUint16 pMask)
{
   m_outvalue = ((m_outvalue & ~pMask) | (pValue & pMask)) & m_bitmask;
   return m_outvalue;
}
wxUint16 wxPort::MaskedInValue(wxUint16 pValue, wxUint16 pMask)
{
   return (pValue & m_bitmask) & pMask;
}


void wxPort::SetPortName(const wxString& portName)
{
   m_portName = portName;
}

void wxPort::SetPortPath(const wxString& portPath)
{
   m_portPath = portPath;
}

void wxPort::EnumeratePorts(PortEntryVector& port_entries)
{
   // we have to use ifdefs here
   #if defined(_WXMSW_ )
      PortEnumW32& enumerator = PortEnumW32::Instance();
   #else
      PortEnumUX& enumerator = PortEnumUX::Instance();
   #endif

   port_entries.clear();
   if(enumerator.EnumeratePorts()) {
      size_t nport = enumerator.NumItems();
      port_entries.reserve(nport);
      for(size_t i=0;i<nport; i++) {
         // return copies;
         port_entries.push_back(*enumerator.GetPort(i));
      }
   }

}

//! Reopens a released port - must have been Opened before
//! supported is only the serial port (parallel needs more management - TODO)
wxPort::PortAccessStatus wxPort::PReopen()
{
   m_lastError = IPAC_Success;

   if ( IsPortOpen() ) return IPAC_Success; // allready, nothing to do

   if ( IsPortSerial() ) {
      SetReleased(false);
      m_lastError = POpen_Second();
      if (IPAC_Success!=m_lastError){
         SetReleased(true); // failed to reopen
      }
   }
   return m_lastError;
}


//! Releases the port but not the handle (use PReopen() to use it again)
//! supported is only the serial port (parallel needs more management - TODO)
wxPort::PortAccessStatus wxPort::PRelease()
{
   if (! IsPortOpen() ) return IPAC_Success;  // allready, nothing to do

   if ( IsPortSerial() ) {
      PClose();
      SetReleased(true);
   }
   return IPAC_Success;
}


//! Get a value from the port
wxPort::PortAccessStatus wxPort::PGet(wxUint16& pValue)
{
   return  PGet(pValue, 0xff); // just call with full bits
}


//! Get a value from the port with mask
wxPort::PortAccessStatus wxPort::PGet(wxUint16& pValue, wxUint16 mask)
{
   if (Released()) return IPAC_PortReleased;

   if (!IsPortIn()) return IPAC_NotSupported;

   if (IsPortParallel()) {
      wxUint8 inByte;
      PortAccessStatus retVal = get_port(inByte);
      pValue = MaskedInValue(inByte, mask);
      return retVal;
   }
   else if (IsPortSerial()) {
      wxUint8 inByte=DSR(); inByte = inByte<<1; inByte|=CTS();
      pValue = MaskedInValue(inByte, mask);
      return IPAC_Success;
   }
   else if (IsPortGPUSB()) {//BM:20081226 added
      wxUint8 inByte;
      PortAccessStatus retVal = get_port(inByte);
      pValue = MaskedInValue(inByte, mask);
      return retVal;
   }
   else if (IsPortLXUSB()) {//BM:20081226 added
      wxUint8 inByte;
      PortAccessStatus retVal = get_port(inByte);
      pValue = MaskedInValue(inByte, mask);
      return retVal;
   }
   else if (IsPortLXLED()) {//BM:20081226 added
      wxUint8 inByte;
      PortAccessStatus retVal = get_port(inByte);
      pValue = MaskedInValue(inByte, mask);
      return retVal;
   }
   else if (IsPortDummy()) {
      return IPAC_NoInterface;
   }
   else {
      return IPAC_NoInterface;
   }
}


//! Write a value to the port - absolute value ignores other users!!
wxPort::PortAccessStatus wxPort::PPut(wxUint16 pValue)
{
   return PPut(pValue, 0xff); // just call with full bits
}

//! Write a value to the port - masking retains the values set by other users
wxPort::PortAccessStatus wxPort::PPut(wxUint16 pValue, wxUint16 mask)
{
   if (Released()) return IPAC_PortReleased;

   if (!IsPortOut()) return IPAC_NotSupported;

   if (IsPortParallel()) {
      wxUint8 outByte = MergedOutValue(pValue, mask);
      PortAccessStatus retVal = put_port(outByte);
      return retVal;
   }
   else if (IsPortSerial()) {
      wxUint8 outByte = MergedOutValue(pValue, mask);
      DTR( outByte & 2 );
      RTS( outByte & 1 );
      return IPAC_Success;
   }
   else if (IsPortGPUSB()) {//BM:20081226 added
      wxUint8 outByte = MergedOutValue(pValue, mask);
      PortAccessStatus retVal = put_port(outByte);
      return retVal;
   }
   else if (IsPortLXUSB()) {//BM:20081226 added
      wxUint8 outByte = MergedOutValue(pValue, mask);
      PortAccessStatus retVal = put_port(outByte);
      return retVal;
   }
   else if (IsPortLXLED()) {//BM:20081226 added
      wxUint8 outByte = MergedOutValue(pValue, mask);
      PortAccessStatus retVal = put_port(outByte);
      return retVal;
   }
   else if (IsPortDummy()) {
      return IPAC_NoInterface;
   }
   else {
      return IPAC_NoInterface;
   }
}


