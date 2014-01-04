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
 * @file           Ftd245Lib.h
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


#ifndef _FTD245LIB_H__
#define _FTD245LIB_H__

#include "Ftd245IO.h"

//////////////////////////////////////////////////////////////////////
//! The USB Interface class
class ARTUSB_LibUsb;
class Ftd245Lib : public Ftd245IO
{
public:
	Ftd245Lib();
	virtual ~Ftd245Lib();

public:

	//////////////////////////////////////////////////////////////////////
	//! External Interface
	//////////////////////////////////////////////////////////////////////
   //! @brief  does some initialization and test of connection with the USB board
   //! @param  int deviceIndex: the index of the device to open
   bool InitBoard(int deviceIndex);

   //! @brief  Close the connection with a device
	void ShutConnection();

   /// @brief An OS independent millisecond sleep command
   void Sleep_ms(unsigned short milliseconds);

protected:
   //! local methods to support the generic interface

   //! @brief  Returns the status of the connected device
   //! @return bool: true if the device is ready
	bool DeviceAvailable();

   //! @brief  Read from the device
   //! @param  void* lpvBuffer: the read buffer
   //! @param  size_t dwBuffSize: the size of the read buffer
   //! @param  size_t& lpdwBytesRead: variable to return the bytes read
   //! @return State information (Ftd245IOState)
	Ftd245IOState Read(void* lpvBuffer, size_t dwBuffSize, size_t& lpdwBytesRead);

   //! @brief  Empty the read channel
	void ClearInputChannel();

   //! @brief  Write to the device
   //! @param  void* lpvBuffer: the write buffer
   //! @param  size_t dwBuffSize: the size of the write buffer
   //! @param  size_t& lpdwBytes: variable to return the bytes written
   //! @return State information (Ftd245IOState)
	Ftd245IOState Write(void* lpvBuffer, size_t dwBuffSize, size_t& lpdwBytes);

   //! @brief  Get the size of the read queue
   //! @param  size_t& lpdwAmountInRxQueue: variable to return the bytes in read queue
   //! @return State information (Ftd245IOState)
	Ftd245IOState GetQueueStatus(size_t& lpdwAmountInRxQueue);

   //! @brief  Set the board timeouts
   //! @param  unsigned long dwReadTimeout: read timeout in milliseconds
   //! @param  unsigned long dwWriteTimeout: write timeout in milliseconds
   //! @return State information (Ftd245IOState)
	Ftd245IOState SetTimeouts(unsigned long dwReadTimeout, unsigned long dwWriteTimeout);

   //! @brief  Get the board timeouts
   //! @param  unsigned long& dwReadTimeout: returns read timeout in milliseconds
   //! @param  unsigned long& dwWriteTimeout: returns write timeout in milliseconds
	void GetTimeouts(unsigned long& dwReadTimeout, unsigned long& dwWriteTimeout);

   //! @brief  Set the transfer parameter
   //! @param  size_t dwInTransferSize: read transfer size of the usb interface
   //! @return State information (Ftd245IOState)
   Ftd245IOState SetUSBParameters (size_t dwInTransferSize);

private:
   //! connect a device
   int ConnectDevice(int deviceIndex);

   //! FTDI commands via USB - ControlMessage
   bool FTD245_Reset();
   bool FTD245_Purge();
   bool FTD245_PurgeRX();
   bool FTD245_PurgeTX();
   bool FTD245_SetDatasize8();
   bool FTD245_SetFlowCtrlDisabled();
   bool FTD245_GetLatencyTimer(unsigned char &timerVal);
   bool FTD245_SetLatencyTimer(unsigned char timerVal);
   /*
    First (lower in result) byte:
    - B0..B3 - should be 0
    - B4       Clear to send (CTS)                 - 0 = inactive - 1 = active
    - B5       Data set ready (DTS)                - 0 = inactive - 1 = active
    - B6       Ring indicator (RI)                 - 0 = inactive - 1 = active
    - B7       Receive line signal detect (RLSD)   - 0 = inactive - 1 = active

    Second (upper in result) byte:
    - B0       Data ready (DR)
    - B1       Overrun error (OE)
    - B2       Parity error (PE)
    - B3       Framing error (FE)
    - B4       Break interrupt (BI)
    - B5       Transmitter holding register (THRE)
    - B6       Transmitter empty (TEMT)
    - B7       Error in RCVR FIFO
    */
   bool FTD245_GetModemStatus(unsigned short &mStatus);

private:
   ARTUSB_LibUsb* m_pDriver;    // Pointer to the USB interface
   bool           m_deviceConnected;
   unsigned char  m_latencyTimerValue;      // the FTDI status pace if no data is available
   unsigned long  m_rxTimeout, m_txTimeout; // the stored timeout values

};



#endif  // _FTD245LIB_H__
