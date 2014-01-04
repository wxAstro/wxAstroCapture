// ParPort.h : Declaration of the CPortIO
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
// ParPort.h: interface for the CParPort class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _PORTUX_H__
#define _PORTUX_H__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wxPort.h"

class GPUsb;
class LXUsb;
class PortUX : public wxPort
{
public:
	PortUX();
	virtual ~PortUX();

public:
   //! Open a port with descriptor and line discipline (if supported)
   PortAccessStatus POpen(const wxPortDescr& port);
   //! returns true if this port is opened
	bool IsPortOpen();

   //! Close a port
   void PClose();

   //! Set the ports bauderate (effective for serial RX/TX only)
   PortAccessStatus PSetBaudrate(EBaudRate baudrate);
   //! Set the ports handshake (effective for serial RX/TX only)
   PortAccessStatus PSetHandshake(EHandshake handshake);
   //! Set the rw timeout (if supported)
   PortAccessStatus PSetTimeout(long toInMS, long toOutMS);

   //! Receive a byte (effective for serial RX only)
   PortAccessStatus PRx(wxUint8& pValue);
   //! Send a byte (effective for serial TX only)
   PortAccessStatus PTx(wxUint8 pValue);

private:

   PortAccessStatus POpen_Second();  // internal open port

   // parallel port handling
	PortAccessStatus get_port(wxUint8 &inByte);
	PortAccessStatus put_port(wxUint8  outByte);

   // serial port handling
	void RTS(wxUint8 outByte);
	wxUint8 CTS();
	void DTR(wxUint8 outByte);
	wxUint8 DSR();

	wxUint8 RING();
	wxUint8 RLSD();


private:
   void ClosePPDev(void);
   int  m_ParallelPort_Handle;  // Handle for ppdev Driver

   void CloseSerialPort(void);
   bool ConfigPortNow();
   // member variables
   int  m_SerialPort_Handle;  // Handle for COMM Driver

   // GPUSB handling
   GPUsb*   m_pGpUsb;
	void CloseGPUSB(void);

   // LXUSB handling
   LXUsb*   m_pLxUsb;
	void CloseLXUSB(void);

};

#endif // _PORTUX_H__

