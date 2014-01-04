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
 * @file           Ftd245IO.h
 *****************************************************************************
 * class Ftd245IO
 *
 * Astronomy USB Appliance driver for Linux using libUsb or serial IO
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

#ifndef _FTD245IO_H__
#define _FTD245IO_H__

#include <stddef.h>

//////////////////////////////////////////////////////////////////////
//! The USB Interface class
class ARTUSB_LibUsb;
class Ftd245IO
{
public:
	//////////////////////////////////////////////////////////////////////
	//! Enumeration Interface
	//////////////////////////////////////////////////////////////////////
   //! @brief  Return true if Nth USB device exists and is a camera.
   static bool DeviceIsCCDCamera(int deviceIndex);

   //! @brief  Get USB Identifier of Nth USB device. Return false if no such device.
   //! @brief  buffer provided with pName must be at least 40 chars long.
   static bool DeviceName(int deviceIndex, char pName[]);

   //! @brief  Get USB Serial number of Nth USB device. Return false if no such device.
   //! @brief  buffer provided with pSerial must be at least 40 chars long.
   static bool DeviceSerial(int deviceIndex, char pSerial[]);

private:
   static bool GetDeviceProperties(int deviceIndex, char pName[], char pSerial[]);
   static const int c_devStringLength = 40;


public:
	enum Ftd245IOState {
		FTS_UNKOWN = -1,
		// from FTD2XX
		FTS_OK = 0,
      FTS_INVALID_HANDLE,
      FTS_DEVICE_NOT_FOUND,
      FTS_DEVICE_NOT_OPENED,
      FTS_IO_ERROR,
      FTS_INSUFFICIENT_RESOURCES,
      FTS_INVALID_PARAMETER,

		// ours
		FTS_CantLoad_ft8u245_dll = 20,
		FTS_CantFind_FT_Write,
		FTS_CantFind_FT_Read,
		FTS_CantFind_FT_Open,
		FTS_CantFind_FT_OpenEx,
		FTS_CantFind_FT_ListDevices,
		FTS_CantFind_FT_Close,
		FTS_CantFind_FT_ResetDevice,
		FTS_CantFind_FT_Purge,
		FTS_CantFind_FT_SetTimeouts,
		FTS_CantFind_FT_GetQueueStatus,
		FTS_CantFind_FT_SetUSBParameters,
		FTS_USB1_notResponding,
		FTS_DLP_USB2_notResponding,
		FTS_CannotListDevices,
		FTS_NoDevicesFound,
		FTS_CannotOpenDev0,
	};

	Ftd245IO();
	virtual ~Ftd245IO();

	//////////////////////////////////////////////////////////////////////
	//! External Interface (generic and OS dependent ones)
	//////////////////////////////////////////////////////////////////////

   //! @brief  Initialize and test of connection with the USB board
   //! @param  int deviceIndex: relates to interface slots 0...9
   //! @return bool: returns true if an FTDI board was found with the given deviceIndex
   virtual bool InitBoard(int deviceIndex) =0;

   //! @brief  Returns the status of the connected board
   //! @return bool: true if the board is ready
	bool BoardAvailable();

   int DeviceIndex() const {return m_deviceIndex;};

   //! @brief  Closes the connection with the FTDI board
	virtual void ShutConnection();

   //! @brief  Read one byte from the FTDI chip
   //! @param  unsigned char&: accepts the byte read
   //! @return bool: returns true if successful
	bool ReadBYTE(unsigned char& pByte);

   //! @brief  Read a number of bytes from the FTDI chip
   //! @param  unsigned char* pByte: pointer to a byte buffer
   //! @param  size_t bufSize: the size of the byte buffer
   //! @param  size_t& bytesRead: accepts the bytes read
   //! @return bool: returns true if successful or timout and bytesRead=0, else false
	bool ReadBYTEs(unsigned char* pByte, size_t bufSize, size_t& bytesRead);

   //! @brief  Reads bytes from the device as long as there are some available
   //! @param  size_t& bytesRead: accepts the bytes read
   //! @return bool: returns true if successful
	bool ReadBYTEsNIL(size_t &bytesRead);

   //! @brief  Writes one byte to the FTDI board
   //! @param  unsigned char byte: the byte to be written
   //! @return bool: returns true if successful
	bool WriteBYTE(unsigned char byte);

   //! @brief  Writes one byte to the FTDI board
   //! @param  unsigned char* pByte: pointer to a byte buffer
   //! @param  size_t bufSize: the size of the byte buffer
   //! @param  size_t& bytesWritten: accepts the bytes written
   //! @return bool: returns true if successful
	bool WriteBYTEs(unsigned char* pByte, size_t bufSize, size_t& bytesWritten);

   //! @brief  Set an optimized buffer length for short messages
	void SetDefaultBuffer();
   //! @brief  Set an optimized buffer length for long messages
	void SetMaxBuffer();
   //! @brief  Returns the optimized data read amount for the current buffer
	size_t GetAskSize() const;

   //! @brief  Set timeouts for the Read and Write ops
   //! @param  unsigned long dwReadTimeout: the read timeout in ms
   //! @param  unsigned long dwWriteTimeout: the write timeout in ms
   //! @return Ftd245IOState: returns FTS_OK if successful
	virtual Ftd245IOState SetTimeouts(unsigned long dwReadTimeout, unsigned long dwWriteTimeout) =0;

   //! @brief  Get the board timeouts
   //! @param  unsigned long& dwReadTimeout: returns read timeout in milliseconds
   //! @param  unsigned long& dwWriteTimeout: returns write timeout in milliseconds
	virtual void GetTimeouts(unsigned long& dwReadTimeout, unsigned long& dwWriteTimeout) =0;

   //! @brief  Implements a Sleep command
   //! @param  unsigned short milliseconds: the time to sleep in ms
   virtual void Sleep_ms(unsigned short milliseconds) =0;



protected:
	// must be implemented OS dependent
   //! @brief  Returns the status of the connected device
   //! @return bool: true if the device is ready
	virtual bool DeviceAvailable() = 0;

   //! @brief  Read from the device
   //! @param  void* lpvBuffer: the read buffer
   //! @param  size_t dwBuffSize: the size of the read buffer
   //! @param  size_t& lpdwBytesRead: variable to return the bytes read
   //! @return State information (Ftd245IOState)
	virtual Ftd245IOState Read(void* lpvBuffer, size_t dwBuffSize, size_t& lpdwBytesRead) =0;

   //! @brief  Empty the read channel
	virtual void ClearInputChannel() =0;

   //! @brief  Write to the device
   //! @param  void* lpvBuffer: the write buffer
   //! @param  size_t dwBuffSize: the size of the write buffer
   //! @param  size_t& lpdwBytes: variable to return the bytes written
   //! @return State information (Ftd245IOState)
	virtual Ftd245IOState Write(void* lpvBuffer, size_t dwBuffSize, size_t& lpdwBytes) =0;

   //! @brief  Get the size of the read queue
   //! @param  size_t& lpdwAmountInRxQueue: variable to return the bytes in read queue
   //! @return State information (Ftd245IOState)
	virtual Ftd245IOState GetQueueStatus(size_t& lpdwAmountInRxQueue) =0;

   //! @brief  Set the transfer parameter
   //! @param  size_t dwInTransferSize: read transfer size of the usb interface
   //! @return State information (Ftd245IOState)
	virtual Ftd245IOState SetUSBParameters (size_t dwInTransferSize) =0;

protected:
   int               m_deviceIndex;
	bool              m_board_present;
	size_t            m_bufferSize;
	unsigned char*    m_scratchBuffer;

private:

};



#endif  // _FTD245IO_H__
