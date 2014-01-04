// ParPort.cpp: implementation of the PortW32 class.
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

// Copyright of 3rd party code
/******************************************************************************/
/*                                                                            */
/*                          IoExample for PortTalk V2.1                       */
/*                        Version 2.1, 12th January 2002                      */
/*                          http://www.beyondlogic.org                        */
/*                                                                            */
/* Copyright © 2002 Craig Peacock. Craig.Peacock@beyondlogic.org              */
/* Any publication or distribution of this code in source form is prohibited  */
/* without prior written permission of the copyright holder. This source code */
/* is provided "as is", without any guarantee made as to its suitability or   */
/* fitness for any particular use. Permission is herby granted to modify or   */
/* enhance this sample code to produce a derivative program which may only be */
/* distributed in compiled object form only.                                  */
/******************************************************************************/

//
//////////////////////////////////////////////////////////////////////

#include "PortW32.h"

#include <assert.h>
#include <winioctl.h>
#include "porttalk_IOCTL.h"

#include "PortEnumW32.h"

#include "GPUsb.h"
#include "LXUsb.h"

#include <wx/stdpaths.h>
#include <wx/filename.h>
/////////////////////////////////////////////////////////////////////////////
// CPortIO

// a class used to control IO ports of a INTEL like processor
// does differentiate between protected OSs (NT,2000,XP) and
// those that have unprotected access (Win95,98,Me)
// Protected access is done through the 'porttalk.sys' service
// see copyright above

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// PUBLIC METHODS



/////////////////////////////////////////////////////////////////////////////
//
// Constructor
// get the OS type
//
PortW32::PortW32()
: m_PortTalk_Handle(INVALID_HANDLE_VALUE)
, m_ProtectedOS(FALSE)
, m_portAddress(0)
, m_SerialPort_Handle(INVALID_HANDLE_VALUE)
, m_pGpUsb(NULL)
, m_pLxUsb(NULL)
{
OSVERSIONINFOEX osvi;
BOOL bOsVersionInfoEx;

//BEGIN

	// get the OS type

	// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
	// If that fails, try using the OSVERSIONINFO structure.

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

   bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi);
	if( ! bOsVersionInfoEx ) {
	  // If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.

      osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
      if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) )
         return; // hmmm, strange - possibly not windows at all !!!!
	}

	switch (osvi.dwPlatformId)
	{
		case VER_PLATFORM_WIN32_NT:
			// NT, 2000, XP, future??
			m_ProtectedOS = TRUE;
			break;

		case VER_PLATFORM_WIN32_WINDOWS:
			// 95, 98xy, Me
			m_ProtectedOS = FALSE;
			break;

		case VER_PLATFORM_WIN32s:
			// Win3.1 with Win32 extensions (should not happen...)
			m_ProtectedOS = FALSE;
			break;
   }//END switch

}//END constructor


/////////////////////////////////////////////////////////////////////////////
//
// Constructor
// Closes Porttalk access if required
//
PortW32::~PortW32()
{
	PClose();
}

////////////////////////////////////////////////////////////////////////////
//
// Explicit start of port access
//
PortW32::PortAccessStatus PortW32::POpen(const wxPortDescr& portDevice)
{
	m_lastError = IPAC_Success;

   if (portDevice.PortIndex()<0) {
      m_lastError = IPAC_NoInterface;
      return m_lastError;
   }
   // just copy from descriptor
   SetPortParallel(portDevice.IsPortParallel());
   SetPortSerial(portDevice.IsPortSerial());
   SetPortGPUSB(portDevice.IsPortGPUSB());//BM:20081226 added
   SetPortLXUSB(portDevice.IsPortLXUSB());//BM:20081226 added
   SetPortLXLED(portDevice.IsPortLXLED());//BM:20081226 added
   SetPortDummy(portDevice.IsPortDummy());
   SetPortName(portDevice.PortName());
   SetPortPath(portDevice.PortPath());

   // depending on the port type we setup things here
   if (IsPortParallel()) {
      // regular parallel port - things have to be setup in the dialog
      // we support all 8 bits of the port
      m_portAddress = portDevice.PortAddress();
      SetNumbits(8); SetBitmask(0xff);
      SetPortOut(true); SetPortIn(false);
   }
   else if (IsPortSerial()) {
      // regular or virtual serial port - things have to be setup in the dialog
      // we support only RTS and DTR as port outputs, CTS and DTR as inputs (and RX and TX for data)
      m_comName.Printf(wxT("%s"), PortPath());
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

PortW32::PortAccessStatus PortW32::POpen_Second()
{
   if (IsPortParallel()) {
      if (m_ProtectedOS)
         m_lastError = this->OpenPortTalk();
      else
         m_PortTalk_Handle = (HANDLE)0;  // dummy for raw access init done

      if (m_lastError!=IPAC_Success)
         m_portAddress = 0;
   }

   else if (IsPortSerial()) {
      m_lastError = this->OpenSerialPort();
      if (m_lastError==IPAC_Success) {
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


bool PortW32::IsPortOpen()
{
   if (IsPortParallel()) {
      return (m_PortTalk_Handle!=INVALID_HANDLE_VALUE);
   }
   else if (IsPortSerial()) {
      return (m_SerialPort_Handle!=INVALID_HANDLE_VALUE);
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

void PortW32::PClose()
{
	ClosePortTalk();
	CloseSerialPort();
	CloseGPUSB();
	CloseLXUSB();
}


/////////////////////////////////////////////////////////////////////////////
//
// Returns the Byte at port portAddress
//
PortW32::PortAccessStatus PortW32::get_port(wxUint8& inByte)
{
   if (Released()) return IPAC_PortReleased;

	if ( IsPortParallel() ) {
      m_lastError = IPAC_Success;
      if (m_PortTalk_Handle == INVALID_HANDLE_VALUE) {
         m_lastError = IPAC_NoInterface;
         return m_lastError; // ERROR EXIT
      }
      if (m_portAddress == 0) {
         m_lastError = IPAC_NoInterface;
         return m_lastError; // ERROR EXIT
      }

      if (m_ProtectedOS) {
         BOOL        success;
         USHORT      myPort = (USHORT)m_portAddress;
         BYTE        myByte = 0;
         DWORD       bytesReturned;

         success = DeviceIoControl(m_PortTalk_Handle,
                           (DWORD)IOCTL_READ_PORT_UCHAR,
                           &myPort, sizeof(myPort),                  /*in data*/
                           &myByte, sizeof(myByte), &bytesReturned,  /*out data*/
                           NULL);

         if (!success)
            //printf("Error occured during inportb while talking to PortTalk driver %d\n",GetLastError());
            m_lastError = IPAC_GetFailed;
         else
            inByte = myByte;
      }

      else {
         // raw port access
         USHORT myPort = (USHORT)m_portAddress;
         BYTE   myByte = 0;
         __asm
         {
            mov dx, myPort
            in  al, dx
            mov myByte, al
         }
         inByte = myByte;
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

}//END Get



/////////////////////////////////////////////////////////////////////////////
//
// Sets the Byte at port portAddress
//
// returns:
// S_OK
// E_NOINTERFACE	if access was not started or failed
// E_FAIL			if something went wrong
//
PortW32::PortAccessStatus PortW32::put_port(wxUint8 outByte)
{
   if (Released()) return IPAC_PortReleased;

	if ( IsPortParallel() ) {
      m_lastError = IPAC_Success;
      if (m_PortTalk_Handle == INVALID_HANDLE_VALUE) {
         m_lastError = IPAC_NoInterface;
         return m_lastError; // ERROR EXIT
      }
      if (m_portAddress == 0) {
         m_lastError = IPAC_NoInterface;
         return m_lastError; // ERROR EXIT
      }

      if (m_ProtectedOS) {
         BOOL        success;
         BYTE        buffer[3];
         PUSHORT     pmyPort = (PUSHORT)&buffer[0];
         PBYTE       pmyByte = (PBYTE)&buffer[2];
         DWORD       bytesReturned;

         *pmyPort = (USHORT)m_portAddress;
         *pmyByte = outByte;

         success = DeviceIoControl(m_PortTalk_Handle,
                           (DWORD)IOCTL_WRITE_PORT_UCHAR,
                           &buffer, sizeof(buffer),
                           NULL, 0, &bytesReturned,
                           NULL);

         if (!success)
            //printf("Error occured during inportb while talking to PortTalk driver %d\n",GetLastError());
            m_lastError = IPAC_PutFailed;
      }

      else {
         // raw port access
         USHORT myPort = (USHORT)m_portAddress;
         BYTE   myByte = outByte;
         __asm
         {
            mov al, myByte
            mov dx, myPort
            out dx, al
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
}//END put_port



////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// PRIVATE METHODS


/////////////////////////////////////////////////////////////////////////////
//
// Open the PortTalk Manager
//
// returns:
//   problems from service startup (see StartPortTalkDriver)
//
PortW32::PortAccessStatus PortW32::OpenPortTalk(void)
{
HRESULT hr = S_OK;

//BEGIN
	if (m_ProtectedOS) {
		/* Open PortTalk Driver. If we cannot open it, try installing and starting it */
		m_PortTalk_Handle = CreateFile("\\\\.\\PortTalk",
									 GENERIC_READ,
									 0,
									 NULL,
									 OPEN_EXISTING,
									 FILE_ATTRIBUTE_NORMAL,
									 NULL);

		if(m_PortTalk_Handle == INVALID_HANDLE_VALUE) {
			/* Start or Install PortTalk Driver */
			hr = this->StartPortTalkDriver();
			/* Then try to open once more, before failing */
			m_PortTalk_Handle = CreateFile("\\\\.\\PortTalk",
										 GENERIC_READ,
										 0,
										 NULL,
										 OPEN_EXISTING,
										 FILE_ATTRIBUTE_NORMAL,
										 NULL);

			if(m_PortTalk_Handle == INVALID_HANDLE_VALUE) {
				//printf("PortTalk: Couldn't access PortTalk Driver, Please ensure driver is loaded.\n\n");
				if ( SUCCEEDED(hr) ) {
				   //even if installation and startup was successfull
					hr = E_NOTIMPL; // ???
				}
			}
		}
		// result conversion
		switch (hr) {
			case S_OK:           return IPAC_Success;	break;
			case E_ACCESSDENIED: return IPAC_AccessDenied; break;
			case E_NOINTERFACE:  return IPAC_ServiceNameInvalid; break;
			case E_HANDLE:       return IPAC_FileCopyFailed; break;
			case E_ABORT:        return IPAC_ServiceInstFailed; break;
			case E_NOTIMPL:      return IPAC_OtherError; break;
			default: return IPAC_OtherError;
		}
	}
	else {
		assert(false);  // SW error non protected shall not land here
		return IPAC_OtherError;
	}
}//END OpenPortTalk




/////////////////////////////////////////////////////////////////////////////
//
// Close the manager handle
//
void PortW32::ClosePortTalk(void)
{
//BEGIN
	if (m_PortTalk_Handle != INVALID_HANDLE_VALUE) {
		if (m_ProtectedOS)
			CloseHandle(m_PortTalk_Handle);

		m_PortTalk_Handle = INVALID_HANDLE_VALUE;
	}
}//END ClosePortTalk



/////////////////////////////////////////////////////////////////////////////
//
// Starts the PortTalk Service
//
// returns:
//  S_OK			if all went fine
//  E_ACCESSDENIED	if the service manager cold not be accessed
//  E_NOINTERFACE	if the service name was not valid ????
//  E_HANDLE		if the file could not be copied and therefore the service could not start
//  E_ABORT			if the service installation failed
//
HRESULT PortW32::StartPortTalkDriver(void)
{
SC_HANDLE  SchSCManager;
SC_HANDLE  schService;
BOOL       ret;
DWORD      err;
HRESULT hr= S_OK;

//BEGIN
    /* Open Handle to Service Control Manager */
    SchSCManager = OpenSCManager (NULL,                        /* machine (NULL == local) */
                                  NULL,                        /* database (NULL == default) */
                                  SC_MANAGER_ALL_ACCESS);      /* access required */

    if (SchSCManager == NULL)
      if (GetLastError() == ERROR_ACCESS_DENIED) {
         /* We do not have enough rights to open the SCM, therefore we must */
         /* be a poor user with only user rights. */
         //printf("PortTalk: You do not have rights to access the Service Control Manager and\n");
         //printf("PortTalk: the PortTalk driver is not installed or started. Please ask \n");
         //printf("PortTalk: your administrator to install the driver on your behalf.\n");
         return E_ACCESSDENIED;
       }//END if access denied

	// lucky we have access to the service manager
    do {
         /* Open a Handle to the PortTalk Service Database */
         schService = OpenService(SchSCManager,         /* handle to service control manager database */
                                  "PortTalk",           /* pointer to name of service to start */
                                  SERVICE_ALL_ACCESS);  /* type of access to service */

		if (schService == NULL)
            switch (GetLastError()){
                case ERROR_ACCESS_DENIED:
                        //printf("PortTalk: You do not have rights to the PortTalk service database\n");
						if (SchSCManager)
							CloseServiceHandle(SchSCManager);
                        return(E_ACCESSDENIED);

                case ERROR_INVALID_NAME:
                        //printf("PortTalk: The specified service name is invalid.\n");
						if (SchSCManager)
							CloseServiceHandle(SchSCManager);
                        return(E_NOINTERFACE);

                case ERROR_SERVICE_DOES_NOT_EXIST:
                        //printf("PortTalk: The PortTalk driver does not exist. Installing driver.\n");
                        //printf("PortTalk: This can take up to 30 seconds on some machines . .\n");
                        hr = this->InstallPortTalkDriver();
						if ( FAILED(hr) )
						{
							if (SchSCManager)
								CloseServiceHandle(SchSCManager);
							return hr;  // ABORT from installation
						}

                        break; // next round
			}//END switch

	// loop until service access exist (or bail out before)
    } while (schService == NULL);

	// here we have either S_OK or S_FALSE (if the file could not be copied)

    /* Start the PortTalk Driver. Errors will occur here if PortTalk.SYS file doesn't exist */

   ret = StartService (schService,    /* service identifier */
                        0,             /* number of arguments */
                        NULL);         /* pointer to arguments */

   if (ret) {
	//	printf("PortTalk: The PortTalk driver has been successfully started.\n");
		hr = S_OK;
	}
   else {
      err = GetLastError();
      if (err == ERROR_SERVICE_ALREADY_RUNNING) {
			//printf("PortTalk: The PortTalk driver is already running.\n");
			hr = S_OK;
		}
		else if ( (err == ERROR_FILE_NOT_FOUND) || (err==ERROR_PATH_NOT_FOUND) ) { //BM:20081228 add missing path
		   // NOTE: seems Win cannot run services where the driver file is located on a networked drive !!!

		   // deinstall and try again
		   DeleteService(schService);
		}
      else {
          //printf("PortTalk: Unknown error while starting PortTalk driver service.\n");
          //printf("PortTalk: Does PortTalk.SYS exist in your \\System32\\Drivers Directory?\n");
			if (hr==S_FALSE)
				hr = E_HANDLE;  // was due to file copy problem
			else
				hr = E_FAIL;      // other unknown reason
        }
    }

    /* Close handle to Service Control Manager */
    if (schService != NULL)
		CloseServiceHandle (schService);

	if (SchSCManager)
		CloseServiceHandle(SchSCManager);

    return(hr);
}//END StartPortTalkDriver


/////////////////////////////////////////////////////////////////////////////
//
//  Copies the file and installs the service
//
// Returns:
//  S_OK	if all went fine
//  S_FALSE	if the file could not be copied but the service was registered
//	E_ABORT	if the service could not be registerd
//
HRESULT PortW32::InstallPortTalkDriver(void)
{
SC_HANDLE  SchSCManager = NULL;
SC_HANDLE  schService = NULL;
DWORD      err;
HRESULT hr = S_OK;

//BEGIN

    /* Get Current Directory. Assumes PortTalk.SYS driver is in this directory.    */
    /* Doesn't detect if file exists, nor if file is on removable media - if this  */
    /* is the case then when windows next boots, the driver will fail to load and  */
    /* a error entry is made in the event viewer to reflect this */

    wxStandardPaths wxSP;
    wxFileName appPath = wxSP.GetExecutablePath();
    wxFileName sysName(appPath.GetPath(),wxT("PortTalk.sys"));
    wxString sysNameString = sysName.GetFullPath();
    if (!sysName.IsFileExecutable())
       return E_ABORT;

    /* Open Handle to Service Control Manager */
    SchSCManager = OpenSCManager (NULL,                   /* machine (NULL == local) */
                                  NULL,                   /* database (NULL == default) */
                                  SC_MANAGER_ALL_ACCESS); /* access required */

    /* Create Service/Driver - This adds the appropriate registry keys in */
    /* HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services - It doesn't  */
    /* care if the driver exists, or if the path is correct.              */
    schService = CreateService (SchSCManager,                      /* SCManager database */
                                "PortTalk",                        /* name of service */
                                "PortTalk",                        /* name to display */
                                SERVICE_ALL_ACCESS,                /* desired access */
                                SERVICE_KERNEL_DRIVER,             /* service type */
                                SERVICE_DEMAND_START,              /* start type */
                                SERVICE_ERROR_NORMAL,              /* error control type */
                                sysNameString.fn_str(),            /* service's binary */
                                NULL,                              /* no load ordering group */
                                NULL,                              /* no tag identifier */
                                NULL,                              /* no dependencies */
                                NULL,                              /* LocalSystem account */
                                NULL                               /* no password */
                                );

   if (schService == NULL) {
      // could not create service
      err = GetLastError();
      if (err == ERROR_SERVICE_EXISTS)  {
         //printf("PortTalk: Driver already exists. No action taken.\n");
      }
      else {
         //printf("PortTalk: Unknown error while creating Service.\n");
         hr = E_ABORT;
      }
   }
   else {
      //printf("PortTalk: Driver successfully installed.\n");
      /* Close Handle to Service Control Manager */
      CloseServiceHandle (schService);
   }

   if (SchSCManager)
      CloseServiceHandle(SchSCManager);

	return hr;
}//END InstallPortTalkDriver


/// //////SERIAL//////////////////////

void PortW32::RTS(wxUint8 outByte)
{
   if (Released()) return;

	m_lastError = IPAC_Success;
	if ( ! IsPortSerial() ) {
      m_lastError = IPAC_NotSupported; return; // ERROR EXIT
	}
	if (m_SerialPort_Handle == INVALID_HANDLE_VALUE) {
		m_lastError = IPAC_NoInterface; return; // ERROR EXIT
	}

	if ( !EscapeCommFunction(m_SerialPort_Handle, (outByte==0) ? CLRRTS : SETRTS) )
		m_lastError = IPAC_CommLineStatFailed;

}


wxUint8 PortW32::CTS()
{
DWORD mdmStat;

   if (Released()) return 0;

	m_lastError = IPAC_Success;
	if ( ! IsPortSerial() ) {
      m_lastError = IPAC_NotSupported; return 0; // ERROR EXIT
	}
	if (m_SerialPort_Handle == INVALID_HANDLE_VALUE) {
		m_lastError = IPAC_NoInterface; return 0; // ERROR EXIT
	}

	if(!GetCommModemStatus(m_SerialPort_Handle, &mdmStat) )
		m_lastError = IPAC_CommLineStatFailed;
	else
		return ((mdmStat & MS_CTS_ON)==MS_CTS_ON) ? 1 : 0;

	return 0;
}

void PortW32::DTR(wxUint8 outByte)
{
   if (Released()) return;

	m_lastError = IPAC_Success;
	if ( ! IsPortSerial() ) {
      m_lastError = IPAC_NotSupported; return; // ERROR EXIT
	}
	if (m_SerialPort_Handle == INVALID_HANDLE_VALUE) {
		m_lastError = IPAC_NoInterface; return; // ERROR EXIT
	}

	if ( !EscapeCommFunction(m_SerialPort_Handle, (outByte==0) ? CLRDTR : SETDTR) )
		m_lastError = IPAC_CommLineStatFailed;
}

wxUint8 PortW32::DSR()
{
DWORD mdmStat;

   if (Released()) return 0;

	m_lastError = IPAC_Success;
	if ( ! IsPortSerial() ) {
      m_lastError = IPAC_NotSupported; return 0; // ERROR EXIT
	}
	if (m_SerialPort_Handle == INVALID_HANDLE_VALUE) {
		m_lastError = IPAC_NoInterface; return 0; // ERROR EXIT
	}

	if(!GetCommModemStatus(m_SerialPort_Handle, &mdmStat) )
		m_lastError = IPAC_CommLineStatFailed;
	else
		return ((mdmStat & MS_DSR_ON)==MS_DSR_ON) ? 1 : 0;

	return 0;
}

//! Set the rw timeout  (if supported)
PortW32::PortAccessStatus PortW32::PSetTimeout(long toInMS, long toOutMS)
{
   if (Released()) return IPAC_PortReleased;

	m_lastError = IPAC_Success;

	if ( IsPortSerial() ) {
      if (m_SerialPort_Handle == INVALID_HANDLE_VALUE) {
         m_lastError = IPAC_NoInterface; return m_lastError; // ERROR EXIT
      }
	   COMMTIMEOUTS ctb;
      BOOL fSuccess;
	   fSuccess = ::GetCommTimeouts(m_SerialPort_Handle, &ctb);
      if (!fSuccess) {
        // Handle the error.
         m_lastError = IPAC_CommSetupFailed; return m_lastError; // ERROR EXIT
      }
      // read bytes immediately and return with timeout if non are available
      ctb.ReadIntervalTimeout = MAXDWORD;
      ctb.ReadTotalTimeoutMultiplier = MAXDWORD;
      ctb.ReadTotalTimeoutConstant = toInMS;
      // write bytes immediately and return with timeout if not possible
      ctb.WriteTotalTimeoutMultiplier = 1;
      ctb.WriteTotalTimeoutConstant = toOutMS;

	   fSuccess = ::SetCommTimeouts(m_SerialPort_Handle, &ctb);
      if (!fSuccess) {
        // Handle the error.
         m_lastError = IPAC_CommSetupFailed; return m_lastError; // ERROR EXIT
      }
      return m_lastError;
	}
	else if ( IsPortParallel() ) {
		// not avail - just exit with success
	}

	return m_lastError;
}

//! Set the ports bauderate (effective for serial RX/TX only)
PortW32::PortAccessStatus PortW32::PSetBaudrate(EBaudRate baudrate)
{
   if (Released()) return IPAC_PortReleased;

	m_lastError = IPAC_Success;
	if ( ! IsPortSerial() ) {
      m_lastError = IPAC_NotSupported; return m_lastError; // ERROR EXIT
	}
	if (m_SerialPort_Handle == INVALID_HANDLE_VALUE) {
		m_lastError = IPAC_NoInterface; return m_lastError; // ERROR EXIT
	}

   wxPort::SetBaudRate(baudrate); //BM must persist here...

   DCB dcb;
   BOOL fSuccess;

	fSuccess = ::GetCommState(m_SerialPort_Handle, &dcb);
	if (!fSuccess) {
	  // Handle the error.
      m_lastError = IPAC_CommSetupFailed; return m_lastError; // ERROR EXIT
	}

   // just translate into Win Baudrate
   switch (baudrate) {
      case EBR_1200: dcb.BaudRate = CBR_1200; break;
      case EBR_2400: dcb.BaudRate = CBR_2400; break;
      case EBR_4800: dcb.BaudRate = CBR_4800; break;
      case EBR_9600: dcb.BaudRate = CBR_9600; break;
      case EBR_19200: dcb.BaudRate = CBR_19200; break;
      case EBR_38400: dcb.BaudRate = CBR_38400; break;

      case EBR_LAST:
      default:
      m_lastError = IPAC_CommSetupFailed; return m_lastError; // ERROR EXIT
   }// switch

	// Fill in the rest of DCB:
	dcb.ByteSize	= 8;           // BYTE data size, xmit, and rcv
	dcb.Parity		= NOPARITY;    // BYTE parity bit
	dcb.StopBits	= ONESTOPBIT;  // BYTE stop bits

	fSuccess = ::SetCommState(m_SerialPort_Handle, &dcb);

	if (!fSuccess) {
	  // Handle the error.
      m_lastError = IPAC_CommSetupFailed; return m_lastError; // ERROR EXIT
	}
	return m_lastError;
}

//! Set the ports handshake (effective for serial RX/TX only)
PortW32::PortAccessStatus PortW32::PSetHandshake(EHandshake handshake)
{
   if (Released()) return IPAC_PortReleased;

	m_lastError = IPAC_Success;
	if ( ! IsPortSerial() ) {
      m_lastError = IPAC_NotSupported; return m_lastError; // ERROR EXIT
	}
	if (m_SerialPort_Handle == INVALID_HANDLE_VALUE) {
		m_lastError = IPAC_NoInterface; return m_lastError; // ERROR EXIT
	}

   wxPort::SetHandshake(handshake); //BM must persist here...

   DCB dcb;
   BOOL fSuccess;

	fSuccess = ::GetCommState(m_SerialPort_Handle, &dcb);
	if (!fSuccess) {
	  // Handle the error.
      m_lastError = IPAC_CommSetupFailed; return m_lastError; // ERROR EXIT
	}

   // just translate into Win handshake
   switch (handshake) {
      case ELD_None:
         dcb.fOutxCtsFlow = FALSE;
         dcb.fOutxDsrFlow = FALSE;
         dcb.fDtrControl = DTR_CONTROL_ENABLE;
         dcb.fDsrSensitivity = FALSE;
         dcb.fOutX = FALSE;
         dcb.fInX = FALSE;
         dcb.fRtsControl = RTS_CONTROL_ENABLE;
       break;

      case ELD_SW:
         dcb.fOutxCtsFlow = FALSE;
         dcb.fOutxDsrFlow = FALSE;
         dcb.fDtrControl = DTR_CONTROL_ENABLE;
         dcb.fDsrSensitivity = FALSE;
         dcb.fOutX = TRUE;
         dcb.fInX = TRUE;
         dcb.fRtsControl = RTS_CONTROL_ENABLE;
       break;

      case ELD_HW:
         dcb.fOutxCtsFlow = TRUE;
         dcb.fOutxDsrFlow = FALSE;
         dcb.fDtrControl = DTR_CONTROL_ENABLE;
         dcb.fDsrSensitivity = FALSE;
         dcb.fOutX = FALSE;
         dcb.fInX = FALSE;
         dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
       break;

      case ELD_LAST:
      default:
         m_lastError = IPAC_CommSetupFailed; return m_lastError; // ERROR EXIT
   }// switch

	fSuccess = ::SetCommState(m_SerialPort_Handle, &dcb);

	if (!fSuccess) {
	  // Handle the error.
      m_lastError = IPAC_CommSetupFailed; return m_lastError; // ERROR EXIT
	}
	return m_lastError;
}

//! Receive a byte (effective for serial RX only)
PortW32::PortAccessStatus PortW32::PRx(wxUint8& pValue)
{
   BYTE szBuf;
   DWORD dwIncommingReadSize;

   if (Released()) return IPAC_PortReleased;

	m_lastError = IPAC_Success;
	if ( ! IsPortSerial() ) {
      m_lastError = IPAC_NotSupported; return m_lastError; // ERROR EXIT
	}
	if (m_SerialPort_Handle == INVALID_HANDLE_VALUE) {
		m_lastError = IPAC_NoInterface; return m_lastError; // ERROR EXIT
	}

	// read one character
	if(::ReadFile(m_SerialPort_Handle, &szBuf, 1, &dwIncommingReadSize, NULL) != 0) {
		// succeeded
		if(dwIncommingReadSize > 0) {
			wxASSERT(dwIncommingReadSize == 1);	// hmmm expected only one char ???!
			pValue = szBuf;
		}
		else {
			m_lastError = IPAC_Timeout;
		}
	}
	else {
		//unsigned long error = ::GetLastError(); //ERROR_COUNTER_TIMEOUT serial IO timeout ???
		m_lastError = IPAC_GetFailed;
	}
	return m_lastError;
}



//! Send a byte (effective for serial TX only)
PortW32::PortAccessStatus PortW32::PTx(wxUint8 pValue)
{
   BYTE szBuf = pValue;
   DWORD dwNumberOfBytesSent = 0;

   if (Released()) return IPAC_PortReleased;

	m_lastError = IPAC_Success;
	if ( ! IsPortSerial() ) {
      m_lastError = IPAC_NotSupported; return m_lastError; // ERROR EXIT
	}
	if (m_SerialPort_Handle == INVALID_HANDLE_VALUE) {
		m_lastError = IPAC_NoInterface; return m_lastError; // ERROR EXIT
	}

	if(::WriteFile(m_SerialPort_Handle, &szBuf, 1, &dwNumberOfBytesSent, NULL) != 0) {
		// succeeded
		if(dwNumberOfBytesSent > 0) {
			wxASSERT(dwNumberOfBytesSent == 1);	// hmmm expected only one char ???!
		}
		else {
			m_lastError = IPAC_Timeout;
		}
	}
	else {
		//unsigned long error = ::GetLastError(); //ERROR_COUNTER_TIMEOUT serial IO timeout
		m_lastError = IPAC_PutFailed;
	}
	return m_lastError;
}



////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// PRIVATE METHODS


/////////////////////////////////////////////////////////////////////////////
//
// Open the SerialPort Manager
//
//
PortW32::PortAccessStatus PortW32::OpenSerialPort(void)
{
    /* Open PortTalk Driver. If we cannot open it, try installing and starting it */
    m_SerialPort_Handle = CreateFile(m_comName,
                                 GENERIC_READ | GENERIC_WRITE,
                                 0,			/* Shared Mode */
                                 NULL,		/* Security */
                                 OPEN_EXISTING, /* Creation Disposition */
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL);		/* Non Overlapped */

    if(m_SerialPort_Handle == INVALID_HANDLE_VALUE) {
		unsigned long error = ::GetLastError();
		// result conversion
		switch (error)
		{
			case ERROR_SUCCESS:
            ::SetupComm(m_SerialPort_Handle, 64,64); // buffers
            PSetTimeout(500, 500);
            PSetHandshake(ELD_None);
            PSetBaudrate(EBR_9600);
            return IPAC_Success;
            break;
			case ERROR_FILE_NOT_FOUND:	return IPAC_NoInterface; break;
			case ERROR_PATH_NOT_FOUND:	return IPAC_NoInterface; break;
			case ERROR_ACCESS_DENIED:	return IPAC_AccessDenied; break;
			case ERROR_INVALID_HANDLE:	return IPAC_NoInterface; break;
			case ERROR_SERIAL_NO_DEVICE:return IPAC_NoInterface; break;
			default: return IPAC_OtherError;
		}
	}
	return IPAC_Success;
}//OpenSerialPort


/////////////////////////////////////////////////////////////////////////////
//
// Close the manager handle
//
void PortW32::CloseSerialPort(void)
{
//BEGIN
	if (m_SerialPort_Handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_SerialPort_Handle);
		m_SerialPort_Handle = INVALID_HANDLE_VALUE;
	}
}//CloseSerialPort


/////////////////////////////////////////////////////////////////////////////
//
// Close the manager handle
//
void PortW32::CloseGPUSB(void)
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
void PortW32::CloseLXUSB(void)
{
	if (m_pLxUsb) {
	   m_pLxUsb->Close();
	   delete m_pLxUsb; m_pLxUsb=NULL;
	}
}
