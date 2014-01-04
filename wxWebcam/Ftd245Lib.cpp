/**

  Astronomy USB Appliance - Control Library

Copyright (C) 2009 Martin Burri  (bm@burri-web.org)

This program is free software licensed under LGPL;
you can redistribute it and/or modify it under the terms of
the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License,
or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/**
 * @file           Ftd245Lib.cpp
 *****************************************************************************
 * class Ftd245Lib
 *
 * Astronomy USB Appliance driver for Linux using libUsb
 * Upper Level driver for FTDI245 equipped boards
 *  specifically for the Artemis/ATIK USB1.1 cameras
 *
 * This is part of the Driver Library of wxAstroCapture
 *
 * Copyright (C) 2009 Martin Burri  (bm98@burri-web.org)
 *
 * Pls note that the program has to link libusb
 *
 * The libusb driver will disconnect any drivers already connected in order to access
 * the device - it will however not reconnect them so you have to do a
 * plug cycle to make them available to other drivers again.
 *
 *<hr>
 *
 * @b Project      wxAstroCapture<br>
 *
 * @author         M. Burri
 * @date           14-Feb-2009
 *
 *****************************************************************************
 *<hr>
 * @b Updates
 * - dd-mmm-yyyy V. Name: Description
 *
 *****************************************************************************/


#include "Ftd245Lib.h"

/// There is a derived version used for the INDI framework
///  which has some files in another place than wxAstroCapture
#ifdef INDI
   #include "GenericDebHelp.h"
#else
   #include "../GenericDebHelp.h"
#endif

#include "ArtVidPid.h"
#include "ARTUSB_LibUsb.h"

#include <unistd.h>   // write(), read(), getpid(), close()
#include <string.h>   // bzero()
#include <stdio.h>    // This is assumed, but best to include it explicitly.
#include <errno.h>    // errno


// local shortcuts
typedef unsigned char  _byte;
typedef unsigned short _word;
typedef unsigned long  _dword;

#define FTDI_CHANNEL      0  // the used chip has only one channel
#define FTDI_CHANNEL_A    1  // multi channel - channel SIO_A
#define FTDI_CHANNEL_B    2  // multi channel - channel SIO_B
#define FTDI_CHANNEL_C    3  // multi channel - channel PARALLEL



// FROM FTDI_SIO
/* Commands (VendorRequest) */

/* FTDI_RESET */
#define FTDI_RESET               0 /* Reset the port */
#define FTDI_RESET_SIO           0
#define FTDI_RESET_PURGE_RX      1
#define FTDI_RESET_PURGE_TX      2
/*
 * The Reset SIO command has this effect:
 *
 *    Sets flow control set to 'none'
 *    Event char = $0D
 *    Event trigger = disabled
 *    Purge RX buffer
 *    Purge TX buffer
 *    Clear DTR
 *    Clear RTS
 *    baud and data format not reset
 *
 * The Purge RX and TX buffer commands affect nothing except the buffers
*/

#define FTDI_MODEM_CTRL          1 /* Set the modem control register */

/* FTDI_SIO_SET_FLOW_CTRL */
#define FTDI_SET_FLOW_CTRL       2 /* Set flow control register */
#define FTDI_DISABLE_FLOW_CTRL   0x0

#define FTDI_SET_BAUD_RATE       3 /* Set baud rate */
#define FTDI_SET_DATA            4 /* Set the data characteristics of the port */

/* FTDI_SIO_GET_MODEM_STATUS */
/* Retrieve the current value of the modem status register */
#define FTDI_GET_MODEM_STATUS    5 /* Retrieve current value of modern status register */
#define FTDI_CTS_MASK            0x10
#define FTDI_DSR_MASK            0x20
#define FTDI_RI_MASK             0x40
#define FTDI_RLSD_MASK           0x80
/*
 *   wValue:          zero
 *   wIndex:          Port
 *   wLength:         1
 *   Data:            Status
 *
 * One byte of data is returned
 * B0..3 0
 * B4    CTS                                 0 = inactive / 1 = active
 * B5    DSR                                 0 = inactive / 1 = active
 * B6    Ring Indicator (RI)                 0 = inactive / 1 = active
 * B7    Receive Line Signal Detect (RLSD)   0 = inactive / 1 = active
 */

#define FTDI_SET_EVENT_CHAR     _byte(6) /* Set the event character */
#define FTDI_SET_ERROR_CHAR     _byte(7) /* Set the error character */

/*
 * FTDI_SIO_SET_LATENCY_TIMER
 *
 * Set the timeout interval. The FTDI collects data from the slave
 * device, transmitting it to the host when either A) 62 bytes are
 * received, or B) the timeout interval has elapsed and the buffer
 * contains at least 1 byte.  Setting this value to a small number
 * can dramatically improve performance for applications which send
 * small packets, since the default value is 16ms.
 */
#define FTDI_SET_LATENCY_TIMER  _byte(9) /* Set the latency timer */

/*
 * FTDI_SIO_GET_LATENCY_TIMER
 *
 * Set the timeout interval. The FTDI collects data from the slave
 * device, transmitting it to the host when either A) 62 bytes are
 * received, or B) the timeout interval has elapsed and the buffer
 * contains at least 1 byte.  Setting this value to a small number
 * can dramatically improve performance for applications which send
 * small packets, since the default value is 16ms.
 */
#define FTDI_GET_LATENCY_TIMER  _byte(10) /* Get the latency timer */

// END FROM FTDI_SIO


Ftd245Lib::Ftd245Lib()
: Ftd245IO()
, m_pDriver(new ARTUSB_LibUsb) // get the libusb driver
, m_deviceConnected(false)
, m_latencyTimerValue(16)
, m_rxTimeout(100)
, m_txTimeout(100)
{
}

Ftd245Lib::~Ftd245Lib()
{
   ShutConnection();
   if (m_pDriver) delete m_pDriver; m_pDriver = NULL; // drop the libusb driver
}

///////////////////////////////////////////////////////////////////////////////
//! @brief  Close the connection with a device
void Ftd245Lib::ShutConnection()
{
   _V_ ::fprintf(stderr, "Ftd245Lib::ShutConnection() - device closed.\n");
	if (m_deviceConnected) {
	   // disconnect first
		m_pDriver->DeviceResetAndClose();
		m_deviceConnected = false;
	}
   Ftd245IO::ShutConnection(); //       m_board_present = false;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief portable sleep command; take milliseconds as argument
void Ftd245Lib::Sleep_ms(unsigned short  mseconds )
{
   ARTUSB_LibUsb::Sleep_ms(mseconds); // use the one from ARTUSB_LibUsb (it is the lowest level)
}


///////////////////////////////////////////////////////////////////////////////
//! connect a device
int Ftd245Lib::ConnectDevice(int deviceIndex)
{
   _V_ ::fprintf(stderr, "Ftd245Lib::ConnectDevice(%d)\n", deviceIndex);
	int err = ARTUSB_LibUsb::USB_OK;

	if (!m_pDriver) return ARTUSB_LibUsb::USB_SW_ERR; // ERROR EXIT: no usb driver

	if (m_deviceConnected) {
	   // disconnect first
		m_pDriver->DeviceClose();
		m_deviceConnected = false;
	}
   // now try to open the given device
	err = m_pDriver->DeviceOpen(static_cast<ARTUSB_LibUsb::ARTUSB_LibUsb_Device>(deviceIndex));
	m_deviceConnected = (err==ARTUSB_LibUsb::USB_OK) ? true : false;

	return err;
}

///////////////////////////////////////////////////////////////////////////////
//! @brief  Check if the USB board is available (USB device is connected)
bool Ftd245Lib::DeviceAvailable(void)
{
   if (m_pDriver)
      return m_pDriver->DeviceAvailable();
   else
      return false;
}

//////////////////////////////////////////////////////////////////////
//! @brief  Initialize and test of connection with the USB board
//! @param  int deviceIndex: relates to interface slots 0...9
//! @return bool: returns true if an FTDI board was found with the given deviceIndex
bool Ftd245Lib::InitBoard(int deviceIndex)
{
   _V_ ::fprintf(stderr, "Ftd245Lib::InitBoard(%d)\n", deviceIndex);

   if (!m_pDriver) return false;

   bool retVal = true;
   int err=ARTUSB_LibUsb::USB_USB_DEV_NA;

   err=ConnectDevice(deviceIndex);
   if (err==ARTUSB_LibUsb::USB_OK) ::fprintf(stderr, "Ftd245Lib::InitBoard() - found \n");

   retVal = (err==ARTUSB_LibUsb::USB_OK);

   // do some tests to make sure IO things are working
   if (retVal) {
      retVal = FTD245_Reset();    // simple chip reset
      if (!retVal) ::fprintf(stderr, "Ftd245Lib::InitBoard() - FTD245_Reset() failed\n");
   }

   if (retVal) {
      retVal = FTD245_Purge();    // empty IO channels
      if (!retVal) ::fprintf(stderr, "Ftd245Lib::InitBoard() - FTD245_Purge() failed\n");
   }

   if (retVal) {
      retVal = FTD245_SetDatasize8();
      if (!retVal) ::fprintf(stderr, "Ftd245Lib::InitBoard() - FTD245_SetDatasize8() failed\n");
   }

   if (retVal) {
      retVal = FTD245_SetFlowCtrlDisabled();
      if (!retVal) ::fprintf(stderr, "Ftd245Lib::InitBoard() - FTD245_SetFlowCtrlDisabled() failed\n");
   }

   if (retVal) {
      retVal = FTD245_GetLatencyTimer(m_latencyTimerValue);
      if (!retVal) ::fprintf(stderr, "Ftd245Lib::InitBoard() - FTD245_GetLatencyTimer() failed\n");
   }

   if (retVal) {
      m_pDriver->GetTimeouts(m_rxTimeout, m_txTimeout); // load driver defaults
      _V_ ::fprintf(stderr, "Ftd245Lib::InitBoard() timouts: %lu, %lu\n", m_rxTimeout, m_txTimeout);

   }

   if ( retVal ) {
      m_deviceIndex = deviceIndex;
      m_board_present = true;
   }
   else {
      ShutConnection();
   }

   return retVal;
}


//////////////////////////////////////////////////////////////////////
//! @brief  Read from the device
//! @param  void* lpvBuffer: the read buffer
//! @param  size_t dwBuffSize: the size of the read buffer
//! @param  size_t& lpdwBytesRead: variable to return the bytes read
//! @return State information (Ftd245IOState)
Ftd245IO::Ftd245IOState Ftd245Lib::Read(void* lpvBuffer, size_t dwBuffSize, size_t& lpdwBytesRead)
{
   lpdwBytesRead = 0;
   _dword len = dwBuffSize;
   int err=ARTUSB_LibUsb::USB_OK;

	if (!m_deviceConnected) return FTS_INVALID_HANDLE; // ERROR EXIT - no device connected

   //! the FTDI controller returns at least 2 status bytes with every read with an interval
   ///  of the 'latency timer value' which is 16ms per default
   ///  the usb read procedure therefore will treat those reads accordingly

   // the generic USB read procedure has no timeouts while the 16ms status is available
   // so we have to implement the 'netto' timeout handling here
   int nullReads = 1 + ( (m_latencyTimerValue>0) ? m_rxTimeout/m_latencyTimerValue : m_rxTimeout/16 );

   while (nullReads>0) {
      err = m_pDriver->ReadFTDI(len, (unsigned char *)lpvBuffer, 2); // READ OFFSET 2 bytes for FTDI

      if ( (err==ARTUSB_LibUsb::USB_OK) && (len>0) ){
         break; // from while
      }
      else if ( (err==ARTUSB_LibUsb::USB_OK) && (len==0) ){
         nullReads--; // do it until we have a timeout or a read
         len = (nullReads>0) ? dwBuffSize : 0; // reset read length for retry
//::fprintf(stderr, "Ftd245Lib::Read() - nullread :%lu\n", nullReads);
         continue; // with while
      }
      else if (err==ARTUSB_LibUsb::USB_TIMEOUT) {
         // a regular read timeout - should not happen with the FTDI ..
         _V_ ::fprintf(stderr, "Ftd245Lib::Read() -  Read() returned timeout!\n");
         len=0; err=ARTUSB_LibUsb::USB_OK; // timeout translates into OK and len = 0
         break; // from while
      }
      else {
         // IO error
         ::fprintf(stderr, "Ftd245Lib::Read() -  Read() returned error %lu - device closed!\n", len);
         ShutConnection(); // not longer
         len=0;
         break; // from while
      }
   }//while
   lpdwBytesRead = len;

//::fprintf(stderr, "Ftd245Lib::Read() - %lu of %lu asked - bytes!\n", len, dwBuffSize);
   return (err==ARTUSB_LibUsb::USB_OK)?FTS_OK:FTS_IO_ERROR;
   // will return length=0 on timeouts
}


//////////////////////////////////////////////////////////////////////
//! @brief  Empty the read channel
void Ftd245Lib::ClearInputChannel()
{
   FTD245_PurgeRX();
}


//////////////////////////////////////////////////////////////////////
//! @brief  Write to the device
//! @param  void* lpvBuffer: the write buffer
//! @param  size_t dwBuffSize: the size of the write buffer
//! @param  size_t& lpdwBytes: variable to return the bytes written
//! @return State information (Ftd245IOState)
Ftd245IO::Ftd245IOState Ftd245Lib::Write(void* lpvBuffer, size_t dwBuffSize, size_t& lpdwBytes)
{
   lpdwBytes = 0;
   _dword len = dwBuffSize;

	if (!m_deviceConnected) return FTS_INVALID_HANDLE;

   int err = m_pDriver->WriteFTDI(len, (unsigned char *)lpvBuffer);
   if (err!=ARTUSB_LibUsb::USB_OK) {
      // IO error
      ::fprintf(stderr, "Ftd245Lib::Write() -  Write() returned error %lu - device closed!\n", len);
      ShutConnection(); // not longer
   }
   lpdwBytes = len;

   return (ARTUSB_LibUsb::USB_OK==err)?FTS_OK:FTS_IO_ERROR;
}


//////////////////////////////////////////////////////////////////////
//! @brief  Set the board timeouts
//! @param  unsigned long dwReadTimeout: read timeout in milliseconds
//! @param  unsigned long dwWriteTimeout: write timeout in milliseconds
//! @return State information (Ftd245IOState)
Ftd245IO::Ftd245IOState Ftd245Lib::SetTimeouts(unsigned long dwReadTimeout, unsigned long dwWriteTimeout)
{
	if (!m_deviceConnected) return FTS_INVALID_HANDLE;

   m_pDriver->SetTimeouts(dwReadTimeout, dwWriteTimeout);
   m_pDriver->GetTimeouts(m_rxTimeout, m_txTimeout); // load to get the driver numbers
   _V_ ::fprintf(stderr, "Ftd245Lib::SetTimeouts(%lu, %lu)\n", m_rxTimeout, m_txTimeout);
   return FTS_OK;
}

//////////////////////////////////////////////////////////////////////
//! @brief  Get the board timeouts
//! @param  unsigned long& dwReadTimeout: returns read timeout in milliseconds
//! @param  unsigned long& dwWriteTimeout: returns write timeout in milliseconds
void Ftd245Lib::GetTimeouts(unsigned long& dwReadTimeout, unsigned long& dwWriteTimeout)
{
   dwReadTimeout=m_rxTimeout;
   dwWriteTimeout=m_txTimeout;
}


//////////////////////////////////////////////////////////////////////
//! @brief  Get the size of the read queue
//! @param  size_t& lpdwAmountInRxQueue: variable to return the bytes in read queue
//! @return State information (Ftd245IOState)
Ftd245IO::Ftd245IOState Ftd245Lib::GetQueueStatus(size_t& lpdwAmountInRxQueue)
{
   lpdwAmountInRxQueue = 0;// cannot in this driver
   return FTS_OK;
}

//////////////////////////////////////////////////////////////////////
//! @brief  Set the transfer parameter
//! @param  size_t dwInTransferSize: read transfer size of the usb interface
//! @return State information (Ftd245IOState)
Ftd245IO::Ftd245IOState Ftd245Lib::SetUSBParameters (size_t dwInTransferSize)
{
   return FTS_OK; // cannot in Linux
}




/// //////////////////////////////////////////////////////////////////////////
//! FTDI commands via USB - ControlMessages

bool Ftd245Lib::FTD245_Reset()
{
   _V_ ::fprintf(stderr, "Ftd245Lib::FTD245_Reset()\n");

	if (!m_deviceConnected) return false;

	int err = ARTUSB_LibUsb::USB_OK;
   // send a bmVendor request Reset => code=0, value=0, index=channel
	err = m_pDriver->WriteFTDIcmd(FTDI_RESET, FTDI_RESET_SIO, FTDI_CHANNEL);
	if (err==ARTUSB_LibUsb::USB_OK) {
      return true;
	}
	else {
      ::fprintf(stderr, "Ftd245Lib::FTD245_Reset() returns false\n");
	}
	return (ARTUSB_LibUsb::USB_OK==err);
}


//! Clears the read buffer on the chip
bool Ftd245Lib::FTD245_PurgeRX()
{
   _V_ ::fprintf(stderr, "Ftd245Lib::FTD245_PurgeRX()\n");

	if (!m_deviceConnected) return false;

	int err = ARTUSB_LibUsb::USB_OK;
   // send a bmVendor request PurgeRX => code=0, value=1, index=channel
	err = m_pDriver->WriteFTDIcmd(FTDI_RESET, FTDI_RESET_PURGE_RX, FTDI_CHANNEL);
	if (err==ARTUSB_LibUsb::USB_OK) {
      return true;
	}
	else {
      ::fprintf(stderr, "Ftd245Lib::FTD245_PurgeRX() returns false\n");
	}
	return (ARTUSB_LibUsb::USB_OK==err);
}


//! Clears the write buffer on the chip
bool Ftd245Lib::FTD245_PurgeTX()
{
   _V_ ::fprintf(stderr, "Ftd245Lib::FTD245_PurgeTX()\n");

	if (!m_deviceConnected) return false;

	int err = ARTUSB_LibUsb::USB_OK;
   // send a bmVendor request PurgeTX => code=0, value=2, index=channel
	err = m_pDriver->WriteFTDIcmd(FTDI_RESET, FTDI_RESET_PURGE_TX, FTDI_CHANNEL);
	if (err==ARTUSB_LibUsb::USB_OK) {
      return true;
	}
	else {
      ::fprintf(stderr, "Ftd245Lib::FTD245_PurgeTX() returns false\n");
	}
	return (ARTUSB_LibUsb::USB_OK==err);
}


//! Clears the write buffer on the chip
bool Ftd245Lib::FTD245_Purge()
{
   _V_ ::fprintf(stderr, "Ftd245Lib::FTD245_Purge()\n");

	if (!m_deviceConnected) return false;

   bool retVal;
   retVal = FTD245_PurgeRX();
   if (retVal) retVal = FTD245_PurgeTX();
   return retVal;
}


//! Set Datasize 8
bool Ftd245Lib::FTD245_SetDatasize8()
{
   _V_ ::fprintf(stderr, "Ftd245Lib::FTD245_SetDatasize8()\n");

	if (!m_deviceConnected) return false;

	int err = ARTUSB_LibUsb::USB_OK;
	err = m_pDriver->WriteFTDIcmd(FTDI_SET_DATA, 8, FTDI_CHANNEL);
	if (err==ARTUSB_LibUsb::USB_OK) {
      return true;
	}
	else {
      ::fprintf(stderr, "Ftd245Lib::FTD245_SetLatencyTimer() returns false\n");
	}
	return (ARTUSB_LibUsb::USB_OK==err);
}


//! Set FlowCtrl Disabled
bool Ftd245Lib::FTD245_SetFlowCtrlDisabled()
{
   _V_ ::fprintf(stderr, "Ftd245Lib::FTD245_SetFlowCtrlDisabled()\n");

	if (!m_deviceConnected) return false;

	int err = ARTUSB_LibUsb::USB_OK;
	err = m_pDriver->WriteFTDIcmd(FTDI_SET_FLOW_CTRL, FTDI_DISABLE_FLOW_CTRL, FTDI_CHANNEL);
	if (err==ARTUSB_LibUsb::USB_OK) {
      return true;
	}
	else {
      ::fprintf(stderr, "Ftd245Lib::FTD245_SetLatencyTimer() returns false\n");
	}
	return (ARTUSB_LibUsb::USB_OK==err);
}


//! Set latency timer
bool Ftd245Lib::FTD245_SetLatencyTimer(unsigned char timerVal)
{
   _V_ ::fprintf(stderr, "Ftd245Lib::FTD245_SetLatencyTimer(%hd)\n", timerVal);

	if (!m_deviceConnected) return false;
   if (timerVal<1) return false; // must be 1 .. 255

	int err = ARTUSB_LibUsb::USB_OK;
   _word w_val = timerVal;
   // send a bmVendor request SetLTimer => code=0x09, value=timer, index=channel
	err = m_pDriver->WriteFTDIcmd(FTDI_SET_LATENCY_TIMER, w_val, FTDI_CHANNEL);
	if (err==ARTUSB_LibUsb::USB_OK) {
      return true;
	}
	else {
      ::fprintf(stderr, "Ftd245Lib::FTD245_SetLatencyTimer() returns false\n");
	}
	return (ARTUSB_LibUsb::USB_OK==err);
}


//! Get latency timer
bool Ftd245Lib::FTD245_GetLatencyTimer(unsigned char &timerVal)
{
   _V_ ::fprintf(stderr, "Ftd245Lib::FTD245_GetLatencyTimer()\n");

	if (!m_deviceConnected) return false;

	int err = ARTUSB_LibUsb::USB_OK;
   _byte b_val;
   _word w_val=1;
   // send a bmVendor request GetLTimer => code=0x0A, value=0, index=channel- returns LTimer Val byte
	err = m_pDriver->ReadFTDIcmd(FTDI_GET_LATENCY_TIMER, 0, FTDI_CHANNEL, &b_val, w_val);
	if ( (err==ARTUSB_LibUsb::USB_OK) && (w_val==1) ) {
	   timerVal = b_val;
      return true;
	}
	else {
      ::fprintf(stderr, "Ftd245Lib::FTD245_GetLatencyTimer() returns false\n");
	}
	return (ARTUSB_LibUsb::USB_OK==err);
}


bool Ftd245Lib::FTD245_GetModemStatus(unsigned short &mStatus)
{
   _V_ ::fprintf(stderr, "Ftd245Lib::FTD245_GetModemStatus()\n");

	if (!m_deviceConnected) return false;

	int err = ARTUSB_LibUsb::USB_OK;
   _byte bp_val[2];
   _word w_val=2;
   // send a bmVendor request GetLTimer => code=0x05, value=0, index=channel- returns 2 status bytes byte
	err = m_pDriver->ReadFTDIcmd(FTDI_GET_MODEM_STATUS, 0, FTDI_CHANNEL, bp_val, w_val);
	if ( (err==ARTUSB_LibUsb::USB_OK) && (w_val==2) ) {
	   mStatus = (bp_val[1] << 8) | bp_val[0];
      return true;
	}
	else {
      ::fprintf(stderr, "Ftd245Lib::FTD245_GetModemStatus() returns false\n");
	}
	return (ARTUSB_LibUsb::USB_OK==err);
}


