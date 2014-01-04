// ParPort.cpp: implementation of the CParPort class.
//
//---------------------------------------------------------------------------
// Copyright:	2002 - Martin Burri - CH 5618 Bettwil - Switzerland
// date:	27-Dec-2002
// author:	Martin Burri
// rev:		1.0
// history:	none
//---------------------------------------------------------------------------
// IObits Implementation
//---------------------------------------------------------------------------


//
//////////////////////////////////////////////////////////////////////

#include "PortUX.h"
#include "PortEnumUX.h"

#include "GPUsb.h"
#include "LXUsb.h"

#include <sys/types.h>
#include <sys/stat.h>
#include<sys/ioctl.h>
#include <sys/file.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <linux/ppdev.h>
#include <linux/parport.h>


////////////////////////////////////////////////////////////////
//
// http://people.redhat.com/twaugh/parport/html/x623.html
// http://ldp.rtin.bz/linuxfocus/common/src/article205/ppdev.html
// use  sysctl -e dev.parport
//  to find what is attached to the parport device
//

// Helper functions
static int xioctl(int device, int request, void* arg)
{
   // repeat the call to ioctl as long as the previous call was interrupted prematurely
   int r = 0;
   int tryx=10;
   do {
     r = ioctl(device,request,arg); tryx--;
   } while ( -1 == r && EINTR == errno && tryx );

   return r;
}

////////////////////////////////////////////////////////////////
// PUBLIC METHODS


/////////////////////////////////////////////////////////////////////////////
//
// Constructor
// get the OS type
//
PortUX::PortUX()
: m_ParallelPort_Handle(-1)
, m_SerialPort_Handle(-1)
, m_pGpUsb(NULL)
, m_pLxUsb(NULL)
{

}


/////////////////////////////////////////////////////////////////////////////
//
// Constructor
// Closes Porttalk access if required
//
PortUX::~PortUX()
{
   PClose();
}




/////////////////////////////////////////////////////////////////////////////
//
// Explicit start of port access
//
PortUX::PortAccessStatus PortUX::POpen(const wxPortDescr& portDevice)
{
	m_lastError = IPAC_Success;

   SetPortParallel(portDevice.IsPortParallel());
   SetPortSerial(portDevice.IsPortSerial());
   SetPortGPUSB(portDevice.IsPortGPUSB());//BM:20081226 added
   SetPortLXUSB(portDevice.IsPortLXUSB());//BM:20081226 added
   SetPortLXLED(portDevice.IsPortLXLED());//BM:20081226 added
   SetPortDummy(portDevice.IsPortDummy());
   SetPortName(portDevice.PortName());
   SetPortPath(portDevice.PortPath());

   if (IsPortParallel()) {
      SetNumbits(8); SetBitmask(0xff);
      SetPortOut(true); SetPortIn(false);
   }
   else if (IsPortSerial()) {
      SetNumbits(2); SetBitmask(0x3);
      SetPortOut(true); SetPortIn(true);
   }
   else if (IsPortGPUSB()) {
      // this port goes through an USB HID device from Shoestring via Win DLL
      // we support the 4 lines b0..b3 --> RA-, RA+, DEC-, DEC+ (LED is handled by the driver)
      SetNumbits(4); SetBitmask(0x0f);
      SetPortOut(true); SetPortIn(true);
   }
   else if (IsPortLXUSB()) {
      // this port goes through an USB HID device from Shoestring via Win DLL
      // we support the 4 lines b0..b3 --> Frame1, Frame2, Shutter, Amp (LED is handled by the driver)
      SetNumbits(4); SetBitmask(0x0f);
      SetPortOut(true); SetPortIn(true);
   }

   return POpen_Second(); // call real openport
}

PortUX::PortAccessStatus PortUX::POpen_Second()
{
   if (IsPortParallel()) {
      m_ParallelPort_Handle = ::open(PortPath().fn_str(), O_RDWR);
      if (m_ParallelPort_Handle < 0) {
        switch (errno) {
            case EACCES:        m_lastError=IPAC_AccessDenied;  break;
            case ENOENT:        m_lastError=IPAC_NoInterface;   break;
            case EROFS:         m_lastError=IPAC_AccessDenied;  break;
            case EWOULDBLOCK:   m_lastError=IPAC_AccessDenied;  break;
            default: m_lastError=IPAC_OtherError;
        }//switch
        m_ParallelPort_Handle = -1;
      }
      else {
        int rlck = ::flock(m_ParallelPort_Handle, LOCK_EX | LOCK_NB);
        if (rlck) {
           // cannot lock - drop handle
            m_lastError = IPCA_PortInUse;
            ::close(m_ParallelPort_Handle);
            m_ParallelPort_Handle = -1;
        }
        // PPDEV procedure
        if ( xioctl(m_ParallelPort_Handle, PPCLAIM, NULL) <0 ) {
           // cannot claim - drop handle
            m_lastError = IPCA_PortInUse;
            ::flock(m_ParallelPort_Handle, LOCK_UN | LOCK_NB);
            ::close(m_ParallelPort_Handle);
            m_ParallelPort_Handle = -1;
        }
        int mode = IEEE1284_MODE_COMPAT;
        if ( xioctl (m_ParallelPort_Handle, PPSETMODE, &mode) <0 ){
           // cannot set proper mode - drop handle
            m_lastError = IPAC_NotSupported;
            xioctl(m_ParallelPort_Handle, PPRELEASE, NULL);
            ::flock(m_ParallelPort_Handle, LOCK_UN | LOCK_NB);
            ::close(m_ParallelPort_Handle);
            m_ParallelPort_Handle = -1;
        }
      }
   }
   else if (IsPortSerial()) {
      m_SerialPort_Handle = ::open(PortPath().fn_str(),  O_RDWR | O_NOCTTY );
      if(m_SerialPort_Handle<0) {
      // result conversion
         switch (errno) {
            case EACCES:        m_lastError=IPAC_AccessDenied;  break;
            case ENOENT:        m_lastError=IPAC_NoInterface;   break;
            case EROFS:         m_lastError=IPAC_AccessDenied;  break;
            case EWOULDBLOCK:   m_lastError=IPAC_AccessDenied;  break;
            default: m_lastError=IPAC_OtherError;
         }//switch
         m_SerialPort_Handle = -1;
      }
      else {
         ConfigPortNow();
         PSetBaudrate(BaudRate());
         PSetHandshake(Handshake());
      }
   }
   else if (IsPortGPUSB()) {
      // GPUSB Port - start access is OK but not more
      CloseGPUSB();
      m_pGpUsb = GPUsb::Create();
      bool retVal = m_pGpUsb->Open();
      m_lastError = (retVal) ? IPAC_Success : IPAC_NoInterface;
      if (m_lastError==IPAC_Success) {
         m_pGpUsb->SetValue(0);// all lines high i.e. there is no guiding then
      }
      else {
         CloseGPUSB(); // just delete this port
      }
  }
   else if (IsPortLXUSB()) {
      // LXUSB Port - start access is OK but not more
      CloseLXUSB();
      m_pLxUsb = LXUsb::Create();
      bool retVal = m_pLxUsb->Open();
      m_lastError = (retVal) ? IPAC_Success : IPAC_NoInterface;
      if (m_lastError==IPAC_Success) {
         m_pLxUsb->SetValue(0); // all lines high i.e. cam is in regular mode then
      }
      else {
         CloseLXUSB(); // just delete this port
      }
   }
   else if (IsPortLXLED()) {
      // TucLED Port - start access is OK but not more
      // note this port does not exist in Windows
   }
   else if (IsPortDummy()) {
      // Dummy Port - start access is OK but not more
   }
   else {
      // other Port ?? - start access is OK but not more
   }

	return m_lastError;
}



bool PortUX::IsPortOpen()
{
   if (IsPortParallel()) {
      return (m_ParallelPort_Handle>=0);
   }
   else if (IsPortSerial()) {
      return (m_SerialPort_Handle>=0);
   }
   else if (IsPortDummy()) {
      return (true); // dummy is always open
   }
   else if (IsPortGPUSB()) {
     return (m_pGpUsb!=NULL);
   }
   else if (IsPortLXUSB()) {
     return (m_pLxUsb!=NULL);
   }
   else if (IsPortLXLED()) {
      return false;
   }
   else if (IsPortDummy()) {
      return true; // dummy is always open
   }
   else {
      return false;
   }
}

void PortUX::PClose()
{
	ClosePPDev();
   CloseSerialPort();
	CloseGPUSB();
	CloseLXUSB();
}


/////////////////////////////////////////////////////////////////////////////
//
// Returns the outgoing Byte at port portAddress
PortUX::PortAccessStatus PortUX::get_port(wxUint8 &inByte)
{
   inByte=0; // default

   if (Released()) return IPAC_PortReleased;

	if ( IsPortParallel() ) {

      m_lastError = IPAC_Success;
      if (m_ParallelPort_Handle <0) {
         m_lastError = IPAC_NoInterface;
         return m_lastError;
      }

       int iocMode;
       wxUint8 cIn;

       iocMode = 0; // null = Forward Direction registers
       if ( xioctl(m_ParallelPort_Handle, PPDATADIR, &iocMode) <0 ) {
         m_lastError = IPAC_GetFailed;
       }
       else {
           if ( xioctl(m_ParallelPort_Handle, PPRDATA, &cIn) <0) {
               m_lastError = IPAC_GetFailed;
           }
           else {
               inByte = cIn;
           }
      }
   }

   else if (IsPortGPUSB()) {
      m_lastError = IPAC_Success;
      if (m_pGpUsb == NULL) {
         m_lastError = IPAC_NoInterface;
         return m_lastError; // ERROR EXIT
      }
      unsigned short val;
      m_pGpUsb->GetValue(val);
      inByte = wxUint8(val & 0xff);
   }

   else if (IsPortLXUSB()) {
      m_lastError = IPAC_Success;
      if (m_pLxUsb == NULL) {
         m_lastError = IPAC_NoInterface;
         return m_lastError; // ERROR EXIT
      }
      unsigned short val;
      m_pLxUsb->GetValue(val);
      inByte = wxUint8(val & 0xff);
   }

   else {
      // other ports
      m_lastError = IPAC_NotSupported;
   }

	return m_lastError;

}



/////////////////////////////////////////////////////////////////////////////
//
// Sets the Byte at port portAddress
//
PortUX::PortAccessStatus PortUX::put_port(wxUint8  outByte)
{
   if (Released()) return IPAC_PortReleased;

	if ( ! IsPortParallel() ) {
      m_lastError = IPAC_NotSupported;
	}

	if ( IsPortParallel() ) {

      m_lastError = IPAC_Success;
      if (m_ParallelPort_Handle <0) {
         m_lastError = IPAC_NoInterface;
         return m_lastError;
      }

       int iocMode;

       iocMode = 0; //  null = Forward Direction
       if ( xioctl(m_ParallelPort_Handle, PPDATADIR, &iocMode) <0 ) {
         m_lastError = IPAC_PutFailed;
       }
       else {
           if ( xioctl(m_ParallelPort_Handle, PPWDATA, &outByte) <0) {
               m_lastError = IPAC_PutFailed;
           }
      }
   }
   else if (IsPortGPUSB()) {
      m_lastError = IPAC_Success;
      if (m_pGpUsb == NULL) {
         m_lastError = IPAC_NoInterface;
         return m_lastError; // ERROR EXIT
      }
      m_pGpUsb->SetValue((unsigned short)(outByte));
   }

   else if (IsPortLXUSB()) {
      m_lastError = IPAC_Success;
      if (m_pLxUsb == NULL) {
         m_lastError = IPAC_NoInterface;
         return m_lastError; // ERROR EXIT
      }
      m_pLxUsb->SetValue((unsigned short)(outByte));
   }

   else {
      // other ports
      m_lastError = IPAC_NotSupported;
   }

	return m_lastError;
}


/////////////////////////////////////////////////////////////////////////////
//
// Close the manager handle
//
void PortUX::ClosePPDev(void)
{
   if (m_ParallelPort_Handle >= 0) {
      // PPDEV prodecure
      xioctl(m_ParallelPort_Handle, PPRELEASE, NULL);
      ::flock(m_ParallelPort_Handle, LOCK_UN | LOCK_NB);
      ::close(m_ParallelPort_Handle);
      m_ParallelPort_Handle = -1;
   }
}



/////////////////SERIAL////////////

void PortUX::RTS(wxUint8 outByte)
{
   if (Released()) return;

   if ( ! IsPortSerial() ) {
      m_lastError = IPAC_NotSupported;
   }

   m_lastError = IPAC_Success;
   if (m_SerialPort_Handle <0) {
      m_lastError = IPAC_NoInterface;
      return;
   }


   int ioctx = TIOCM_RTS;  // which one
   int iocmd = (outByte==0) ? TIOCMBIC : TIOCMBIS; // Clear or Set bit
   if ( xioctl( m_SerialPort_Handle, iocmd, &ioctx )<0 ) {
      m_lastError = IPAC_CommLineStatFailed;
   }

}


wxUint8 PortUX::CTS()
{
   if (Released()) return 0;

   if ( ! IsPortSerial() ) {
      m_lastError = IPAC_NotSupported;
   }

   m_lastError = IPAC_Success;
   if (m_SerialPort_Handle <0) {
      m_lastError = IPAC_NoInterface;
      return 0;
   }

   int ioctx;
   if ( xioctl( m_SerialPort_Handle, TIOCMGET, &ioctx )<0 ) {
      m_lastError = IPAC_CommLineStatFailed;
      return 0;
   }
   else {
      return ((ioctx & TIOCM_CTS)==TIOCM_CTS) ? 1 : 0;
   }

}


void PortUX::DTR(wxUint8 outByte)
{
   if (Released()) return;

   if ( ! IsPortSerial() ) {
      m_lastError = IPAC_NotSupported;
   }

   m_lastError = IPAC_Success;
   if (m_SerialPort_Handle <0) {
      m_lastError = IPAC_NoInterface;
      return;
   }

   int ioctx = TIOCM_DTR;  // which one
   int iocmd = (outByte==0) ? TIOCMBIC : TIOCMBIS; // Clear or Set bit
   if ( xioctl( m_SerialPort_Handle, iocmd, &ioctx )<0 ) {
      m_lastError = IPAC_CommLineStatFailed;
   }

}


wxUint8 PortUX::DSR()
{
   if (Released()) return 0;

   if ( ! IsPortSerial() ) {
      m_lastError = IPAC_NotSupported;
   }

   m_lastError = IPAC_Success;
   if (m_SerialPort_Handle <0) {
      m_lastError = IPAC_NoInterface;
      return 0;
   }

   int ioctx;
   if ( xioctl( m_SerialPort_Handle, TIOCMGET, &ioctx )<0 ) {
      m_lastError = IPAC_CommLineStatFailed;
      return 0;
   }
   else {
      return ((ioctx & TIOCM_DSR)==TIOCM_DSR) ? 1 : 0;
   }
}


wxUint8 PortUX::RING()
{
   if (Released()) return 0;

   if ( ! IsPortSerial() ) {
      m_lastError = IPAC_NotSupported;
   }

   m_lastError = IPAC_Success;
   if (m_SerialPort_Handle <0) {
      m_lastError = IPAC_NoInterface;
      return 0;
   }

   int ioctx;
   if ( xioctl( m_SerialPort_Handle, TIOCMGET, &ioctx )<0 ) {
      m_lastError = IPAC_CommLineStatFailed;
      return 0;
   }
   else {
      return ((ioctx & TIOCM_RNG)==TIOCM_RNG) ? 1 : 0;
   }
}


wxUint8 PortUX::RLSD()
{
   if (Released()) return 0;

   if ( ! IsPortSerial() ) {
      m_lastError = IPAC_NotSupported;
   }

   m_lastError = IPAC_Success;
   if (m_SerialPort_Handle <0) {
      m_lastError = IPAC_NoInterface;
      return 0;
   }


   int ioctx;
   if ( xioctl( m_SerialPort_Handle, TIOCMGET, &ioctx )<0 ) {
      m_lastError = IPAC_CommLineStatFailed;
      return 0;
   }
   else {
      return ((ioctx & TIOCM_CAR)==TIOCM_CAR) ? 1 : 0;
   }
}


//! Set the rw timeout (if supported)
PortUX::PortAccessStatus PortUX::PSetTimeout(long toInMS, long toOutMS)
{
   if (Released()) return IPAC_PortReleased;

	m_lastError = IPAC_Success;
	if ( IsPortSerial() ) {
      if (m_SerialPort_Handle <0) {
         m_lastError = IPAC_NoInterface; return m_lastError; // ERROR EXIT
      }

      // a quick check wether the COM is available at all
      termios mode;
      ::memset(&mode, 0, sizeof(mode));
      if (::tcgetattr(m_SerialPort_Handle, &mode) != 0) {
         m_lastError = IPAC_CommSetupFailed; return m_lastError; // ERROR EXIT
      }

      cc_t vt = (cc_t)(toInMS/100); vt = (vt<1)?1:vt;
      mode.c_cc[VMIN] = 0;
      mode.c_cc[VTIME] = vt; // *.1 sec
      // If MIN = 0 and TIME > 0, TIME serves as a timeout value.
      // The read will be satisfied if a single character is read, or TIME is exceeded (t = TIME *0.1 s).
      // If TIME is exceeded, no character will be returned.

      // now set the mode
      ::tcflush(m_SerialPort_Handle, TCIOFLUSH);
      if (::tcsetattr(m_SerialPort_Handle, TCSANOW, &mode) == 0) {
         ; //OK
      }
      else {
         m_lastError = IPAC_CommSetupFailed;
      }
	}
	else if ( IsPortParallel() ) {
		// not avail - just exit with success
	}
   return m_lastError;
}

//! Set the ports bauderate (effective for serial RX/TX only)
PortUX::PortAccessStatus PortUX::PSetBaudrate(EBaudRate baudrate)
{
   if (Released()) return IPAC_PortReleased;

	m_lastError = IPAC_Success;
	if ( ! IsPortSerial() ) {
      m_lastError = IPAC_NotSupported; return m_lastError; // ERROR EXIT
	}
	if (m_SerialPort_Handle <0) {
		m_lastError = IPAC_NoInterface; return m_lastError; // ERROR EXIT
	}

   wxPort::SetBaudRate(baudrate); //BM must persist here...

   // a quick check wether the COM is available at all
   termios mode;
   ::memset(&mode, 0, sizeof(mode));
   if (::tcgetattr(m_SerialPort_Handle, &mode) != 0) {
      m_lastError = IPAC_CommSetupFailed; return m_lastError; // ERROR EXIT
   }

   // just translate into UX Baudrate
   switch (baudrate) {
      case EBR_1200:
         ::cfsetispeed(&mode, B1200);
         ::cfsetospeed(&mode, B1200);
         break;
      case EBR_2400:
         ::cfsetispeed(&mode, B2400);
         ::cfsetospeed(&mode, B2400);
         break;
      case EBR_4800:
         ::cfsetispeed(&mode, B4800);
         ::cfsetospeed(&mode, B4800);
         break;
      case EBR_9600:
         ::cfsetispeed(&mode, B9600);
         ::cfsetospeed(&mode, B9600);
         break;
      case EBR_19200:
         ::cfsetispeed(&mode, B19200);
         ::cfsetospeed(&mode, B19200);
         break;
      case EBR_38400:
         ::cfsetispeed(&mode, B38400);
         ::cfsetospeed(&mode, B38400);
         break;

      case EBR_LAST:
      default:
      m_lastError = IPAC_CommSetupFailed; return m_lastError; // ERROR EXIT
   }// switch

   // now set the mode
   ::tcflush(m_SerialPort_Handle, TCIOFLUSH);
   if (::tcsetattr(m_SerialPort_Handle, TCSANOW, &mode) == 0) {
      ; //OK
   }
   else {
      m_lastError = IPAC_CommSetupFailed;
   }
   return m_lastError;
}

//! Set the ports handshake (effective for serial RX/TX only)
PortUX::PortAccessStatus PortUX::PSetHandshake(EHandshake handshake)
{
   if (Released()) return IPAC_PortReleased;

	m_lastError = IPAC_Success;
	if ( ! IsPortSerial() ) {
      m_lastError = IPAC_NotSupported; return m_lastError; // ERROR EXIT
	}
	if (m_SerialPort_Handle <0) {
		m_lastError = IPAC_NoInterface; return m_lastError; // ERROR EXIT
	}


   wxPort::SetHandshake(handshake); //BM must persist here...

   // a quick check wether the COM is available at all
   struct termios mode;
   ::memset(&mode, 0, sizeof(mode));
   if (::tcgetattr(m_SerialPort_Handle, &mode) != 0) {
      m_lastError = IPAC_CommSetupFailed; return m_lastError; // ERROR EXIT
   }

   // just translate into UX handshake
   switch (handshake) {
      case ELD_None:
         mode.c_iflag &= ~IXON;   // Enable XON/XOFF flow control on output.
         mode.c_iflag &= ~IXOFF;  // Enable XON/XOFF flow control on input
         mode.c_cflag &= ~CRTSCTS; // Not RTS CTS
       break;

      case ELD_SW:
         mode.c_iflag |= IXON;   // Enable XON/XOFF flow control on output.
         mode.c_iflag |= IXOFF;  // Enable XON/XOFF flow control on input
         mode.c_cflag &= ~CRTSCTS; // Not RTS CTS
       break;

      case ELD_HW:
         mode.c_iflag &= ~IXON;   // Enable XON/XOFF flow control on output.
         mode.c_iflag &= ~IXOFF;  // Enable XON/XOFF flow control on input
         mode.c_cflag |= CRTSCTS; // Not RTS CTS
       break;

      case ELD_LAST:
      default:
         m_lastError = IPAC_CommSetupFailed; return m_lastError; // ERROR EXIT
   }// switch

   // now set the mode
   ::tcflush(m_SerialPort_Handle, TCIOFLUSH);
   if (::tcsetattr(m_SerialPort_Handle, TCSANOW, &mode) == 0) {
      ; //OK
   }
   else {
      m_lastError = IPAC_CommSetupFailed;
   }
   return m_lastError;
}

//! Receive a byte (effective for serial RX only)
PortUX::PortAccessStatus PortUX::PRx(wxUint8& pValue)
{
   if (Released()) return IPAC_PortReleased;

	m_lastError = IPAC_Success;
	if ( ! IsPortSerial() ) {
      m_lastError = IPAC_NotSupported; return m_lastError; // ERROR EXIT
	}
	if (m_SerialPort_Handle <0) {
		m_lastError = IPAC_NoInterface; return m_lastError; // ERROR EXIT
	}


	// read one character
	wxUint8 szBuf;
	ssize_t dwIncommingReadSize;

    // we have non blocking IO - check for data available
    dwIncommingReadSize = ::read(m_SerialPort_Handle, &szBuf, 1); // read bytewise
    // readsize can be:
    //    >0 (a caracter is read)
    //    =0 (end of file - note this is true only once the it becomes <0 !!)
    //    <0 (an error condition)
	if(dwIncommingReadSize > 0) {
		wxASSERT(dwIncommingReadSize == 1);	// hmmm expected only one char ???!
		pValue = szBuf;
	}
	else {
		// this is timeout 'cause of any reason
        m_lastError = IPAC_GetFailed;
	}

  return m_lastError;

}


//! Send a byte (effective for serial TX only)
PortUX::PortAccessStatus PortUX::PTx(wxUint8 pValue)
{
   if (Released()) return IPAC_PortReleased;

	m_lastError = IPAC_Success;
	if ( ! IsPortSerial() ) {
      m_lastError = IPAC_NotSupported; return m_lastError; // ERROR EXIT
	}
	if (m_SerialPort_Handle <0) {
		m_lastError = IPAC_NoInterface; return m_lastError; // ERROR EXIT
	}


	wxUint8 szBuf = pValue;
	ssize_t dwOutgoingWriteSize;

    // we have non blocking IO - check if we can write
    dwOutgoingWriteSize = ::write(m_SerialPort_Handle, &szBuf, 1); // write bytewise
    // writesize can be:
    //    >0 (a caracter is written)
    //    =0 (end of file - note this is true only once the it becomes <0 !!)
    //    <0 (an error condition)
	if(dwOutgoingWriteSize > 0) {
		wxASSERT(dwOutgoingWriteSize == 1);	// hmmm expected only one char ???!
	}
	else  {
		// this indicated EOF - which is a closed FD of a tty device
    // must close this connection
		m_lastError = IPAC_PutFailed;
	}

    return m_lastError;
}

//! Setup the serial configuration - this is heavily OS dependent
bool PortUX::ConfigPortNow()
{
	m_lastError = IPAC_Success;
	if ( ! IsPortSerial() ) {
      m_lastError = IPAC_NotSupported; return false; // ERROR EXIT
	}
	if (m_SerialPort_Handle <0) {
		m_lastError = IPAC_NoInterface; return false; // ERROR EXIT
	}


   // a quick check wether the COM is available at all
   termios mode;
   ::memset(&mode, 0, sizeof(mode));
   if (::tcgetattr(m_SerialPort_Handle, &mode) != 0) {
      m_lastError = IPAC_CommSetupFailed;
      return false;
   }

   // settings for "stty -F /dev/tts/2 cs7 -cstopb parenb -parodd" or the like
   // clear all
   ::memset(&mode, 0, sizeof(mode));

   // set baud default 9600
   ::cfsetispeed(&mode, B9600);
   ::cfsetospeed(&mode, B9600);

   // fixed members
   mode.c_lflag = 0; // receive mode is non tty style (no local handling)

   mode.c_oflag = 0; // send mode is non tty style (no further out handling)

   mode.c_iflag |= IGNBRK; // ignore break on input
   mode.c_iflag |= IGNPAR; // ignore parity

   mode.c_cflag |= CLOCAL; // ignore modem status lines
   mode.c_cflag |= CREAD;  // enable receiver
   mode.c_cflag |= CS8;    // 8bit
   // default is no XON/XOFF
   // default is no RTS CTS

   mode.c_cc[VMIN] = 0;
   mode.c_cc[VTIME] = 5; // *.1 sec
   // If MIN = 0 and TIME > 0, TIME serves as a timeout value.
   // The read will be satisfied if a single character is read, or TIME is exceeded (t = TIME *0.1 s).
   // If TIME is exceeded, no character will be returned.

   // now set the mode
   ::tcflush(m_SerialPort_Handle, TCIOFLUSH);
   if (::tcsetattr(m_SerialPort_Handle, TCSANOW, &mode) == 0) {
      return true;
   }
   else {
      m_lastError = IPAC_CommSetupFailed;
      return false; // bad luck
   }
}


/////////////////////////////////////////////////////////////////////////////
//
// Close the manager handle
//
void PortUX::CloseSerialPort(void)
{
   if (m_SerialPort_Handle>=0) {
      ::close(m_SerialPort_Handle);
      m_SerialPort_Handle = -1;
   }
}


/////////////////////////////////////////////////////////////////////////////
//
// Close the manager handle
//
void PortUX::CloseGPUSB(void)
{
	if (m_pGpUsb) {
	   m_pGpUsb->Close();
	   delete m_pGpUsb; m_pGpUsb=NULL;
	}
}


/////////////////////////////////////////////////////////////////////////////
//
// Close the manager handle
//
void PortUX::CloseLXUSB(void)
{
	if (m_pLxUsb) {
	   m_pLxUsb->Close();
	   delete m_pLxUsb; m_pLxUsb=NULL;
	}
}
