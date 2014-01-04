
#ifndef _WXPORT_H__
#define _WXPORT_H__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wxPortDescr.h"

#include <vector>

class wxPort
{
public:
	wxPort();
	virtual ~wxPort() {};

public:
   //! Just return a vector of port entries on this system
   typedef std::vector<wxPortDescr> PortEntryVector;
   static void EnumeratePorts(PortEntryVector& port_entries);

public:
   //! create this port
   static wxPort* Construct(const wxPortDescr& port_entry);


public:
   enum PortAccessStatus
	{
		IPAC_Success            = 0,
		IPAC_AccessDenied,
		IPAC_FileCopyFailed,
		IPAC_ServiceInstFailed,
		IPAC_ServiceNameInvalid,
		IPAC_OtherError,
		IPAC_NoInterface,
		IPAC_GetFailed,
		IPAC_PutFailed,
		IPAC_Timeout,
		IPAC_CommSetupFailed,
		IPAC_CommLineStatFailed,
		IPAC_NotSupported,
		IPAC_PortReleased,
		IPCA_PortInUse,
	};

   //!! NOTE: make sure this enum matches the one of wxGuiderPort !!!
   enum EBaudRate {
      EBR_1200  = 0,
      EBR_2400,
      EBR_4800,
      EBR_9600,
      EBR_19200,
      EBR_38400,
      EBR_LAST,   // must remain here !!

   };

   //!! NOTE: make sure this enum matches the one of wxGuiderPort !!!
   enum EHandshake
   {
      ELD_None = 0,
      ELD_SW,        // HW handshake e.g. RTS/CTS
      ELD_HW,        // SW handshake e.g. XON/XOFF
      ELD_LAST,   // must remain here !!
   };



   /// These are implemented in wxPort as they are OS independent


   //! provides access to the last encountered error
	inline const PortAccessStatus LastError() const {return m_lastError;};

   //! provides some information if the port is used
   inline bool IsPortOut() const {return m_portOutput;}
   inline bool IsPortIn() const {return m_portInput;}
   inline bool IsPortInOut() const {return (m_portInput && m_portOutput);}

   inline bool IsPortParallel() const {return m_portParallel;}
   inline bool IsPortSerial() const {return m_portSerial;}
   inline bool IsPortGPUSB() const {return m_portUSB_GPUSB;}//BM:20081226 added
   inline bool IsPortLXUSB() const {return m_portUSB_LXUSB;}//BM:20081226 added
   inline bool IsPortLXLED() const {return m_portUSB_LXLED;}//BM:20081226 added
   inline bool IsPortDummy() const {return m_portDummy;}
   const wxString& PortName() const {return m_portName;}


   /// in general the port is provided as a number of bits
   /// starting with b0 ... bn (max size is 16 bits for a port)

   //! returns the bits available for that port as a bitmask
   //! parallel ports are simply mapped
   //! serial ports such as RS232 are provided as dual types:
   //!  either having 2bits control lines as parallel port
   //!   --> b0 out RTS, b1 out DTR / b0 in CTS, b1 in DSR
   //!  or providing the real 8 bit serial in/out with may be a line protocol implemented

   //! returns the number of bits available (e.g. a serial port in ctrl line mode provides only 2 bits)
   inline wxUint16 NumBitsAvailable(){return m_numbits;};
   //! returns the bit mask of the bits available (e.g. a serial port in ctrl line mode 0x0003)
   inline wxUint16 BitMaskAvailable(){return m_bitmask;};

   //! Reopens a released port - must have been Opened before
   PortAccessStatus PReopen();
   //! Releases the port but not the handle (use PReopen() to use it again)
   PortAccessStatus PRelease();

   //! Get a value from the port - absolute value (all bits)
   PortAccessStatus PGet(wxUint16& pValue);
   //! Get a value from the port - masked value (masked bits only)
   PortAccessStatus PGet(wxUint16& pValue, wxUint16 mask);

   //! Write a value to the port - absolute value ignores other users!!
   PortAccessStatus PPut(wxUint16 pValue);
   //! Write a value to the port - masking retains the values set by other users
   PortAccessStatus PPut(wxUint16 pValue, wxUint16 mask);



   /// These are implemented OS dependent

   //! Open a port with descriptor and line discipline (if supported)
   virtual PortAccessStatus POpen(const wxPortDescr& port) =0;

   //! returns true if this port is opened
	virtual bool IsPortOpen() =0;

   //! Close a port
   virtual void PClose() =0;

   //! Set the ports bauderate (effective for serial RX/TX only)
   virtual PortAccessStatus PSetBaudrate(EBaudRate baudrate) =0;
   //! Set the ports handshake (effective for serial RX/TX only)
   virtual PortAccessStatus PSetHandshake(EHandshake handshake) =0;
   //! Set the rw timeout (if supported)
   virtual PortAccessStatus PSetTimeout(long toInMS, long toOutMS) =0;

   //! Receive a byte (effective for serial RX only)
   virtual PortAccessStatus PRx(wxUint8& pValue) =0;
   //! Send a byte (effective for serial TX only)
   virtual PortAccessStatus PTx(wxUint8 pValue) =0;


protected:
   // used by derived class
	PortAccessStatus    m_lastError;

   void SetPortOut(bool newVal);
   void SetPortIn(bool newVal);

   void SetPortParallel(bool newVal);
   void SetPortSerial(bool newVal);
   void SetPortGPUSB(bool newVal);  //BM:20081226 added
   void SetPortLXUSB(bool newVal);  //BM:20081226 added
   void SetPortLXLED(bool newVal);  //BM:20081226 added
   void SetPortDummy(bool newVal);
   void SetPortName(const wxString& portName);
   void SetPortPath(const wxString& portPath);
   const wxString& PortPath() const {return m_portPath;}

   bool Released();
   void SetReleased(bool newVal);

   void SetNumbits(wxUint16 nbits);
   void SetBitmask(wxUint16 bmask);

   void SetBaudRate(EBaudRate baudrate);
   EBaudRate BaudRate();
   void SetHandshake(EHandshake handshake);
   EHandshake Handshake();

   wxUint16 MergedOutValue(wxUint16 pValue, wxUint16 pMask);
   wxUint16 MaskedInValue(wxUint16 pValue, wxUint16 pMask);


private:
   // OS dependent local classes
   virtual PortAccessStatus POpen_Second() =0;  // internal open port

   // parallel port handling
	virtual PortAccessStatus get_port(wxUint8 &inByte) =0;
	virtual PortAccessStatus put_port(wxUint8  outByte) =0;

   // serial port handling
	virtual void RTS(wxUint8 outByte) =0;
	virtual wxUint8 CTS() =0;
	virtual void DTR(wxUint8 outByte) =0;
	virtual wxUint8 DSR() =0;


   // local vars used and served by wxPort
   wxUint16 m_numbits;  // number of bits available
   wxUint16 m_bitmask;  // bitmask of this port

   EBaudRate      m_baudrate;
   EHandshake     m_handshake;

   wxUint16       m_outvalue; // contains the latest merged outvalue (for masked ops)

   // port flags
   bool     m_portOutput;
   bool     m_portInput;

   bool     m_portParallel;
   bool     m_portSerial;
   bool     m_portUSB_GPUSB;//BM:20081226 added
   bool     m_portUSB_LXUSB;//BM:20081226 added
   bool     m_portUSB_LXLED;//BM:20081226 added
   bool     m_portDummy;

   bool     m_released;

   wxString m_portName;  // required for saving
   wxString m_portPath;  // required for saving
};




#endif // _WXPORT_H__
