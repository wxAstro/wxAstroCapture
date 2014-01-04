
#ifndef _PORTW32_H__
#define _PORTW32_H__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wxPort.h"

class GPUsb;
class LXUsb;

class PortW32 : public wxPort
{
public:
	PortW32();
	virtual ~PortW32();

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

	// parallel port handling
	PortAccessStatus OpenPortTalk(void);
	void ClosePortTalk(void);
	HRESULT StartPortTalkDriver(void);
	HRESULT InstallPortTalkDriver(void);

	// member variables
	HANDLE  m_PortTalk_Handle;  // Handle for PortTalk Driver
	BOOL    m_ProtectedOS;      // true if we have a protected OS
	DWORD   m_portAddress;      // only this address is supported


   // serial port handling
	PortAccessStatus OpenSerialPort(void);
	void CloseSerialPort(void);
	HRESULT StartSerialPortDriver(void);
	HRESULT InstallSerialPortDriver(void);

	// member variables
	HANDLE    m_SerialPort_Handle;  // Handle for COMM Driver
	wxString  m_comName;

   // GPUSB handling
   GPUsb*   m_pGpUsb;
	void CloseGPUSB(void);

   // LXUSB handling
   LXUsb*   m_pLxUsb;
	void CloseLXUSB(void);


};

#endif // _PORTW32_H__

