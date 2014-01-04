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
 * @file           Ftd245IO.cpp
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

#include "Ftd245IO.h"


#include "Ftd245IOUX.h"
#include "Ftd245Lib.h"


//! some consts for the FT245 USB buffer
//! the buffersize is optimized to catch the URB statusinfo
///  together with the content in 64 byte chucks
#define FT_MAX_BUF_SIZE 0x1000
#define FT_DEF_BUF_SIZE 0x40
#define FT_OPT_ASK_SIZE(x) ( (x) - (( (x) /64)*2) )

#define SCRATCH_SIZE 0x4000  // size of a scratch buffer

/// @brief returns a new driver instance (allows OS independent code)
/// NOTE: caller is responsible to delete the received object

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Ftd245IO::Ftd245IO()
: m_deviceIndex(-1)
, m_board_present(false)
, m_bufferSize(0)
, m_scratchBuffer(0)
{

   m_scratchBuffer = new unsigned char[SCRATCH_SIZE]; // for read to NIL
	m_bufferSize = FT_DEF_BUF_SIZE;
}

Ftd245IO::~Ftd245IO()
{
   if (m_scratchBuffer) delete m_scratchBuffer; m_scratchBuffer=NULL;
   m_bufferSize = 0;
   m_deviceIndex=-1;
   m_board_present=false;
}

//////////////////////////////////////////////////////////////////////
// External Interface
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//! @brief  Close the connection with a device
void Ftd245IO::ShutConnection()
{
	m_board_present = false;
}

//////////////////////////////////////////////////////////////////////
//! @brief  Returns the status of the connected board
//! @return bool: true if the board is ready
bool Ftd245IO::BoardAvailable()
{
   return m_board_present && DeviceAvailable();
}


//////////////////////////////////////////////////////////////////////
// retrurns true if read was OK
// false otherwise (on IO error, or nothing read)
bool Ftd245IO::ReadBYTE(unsigned char &pByte)
{
	unsigned char buf[5];
	size_t read = 0;
	Ftd245IOState ftStatus = (Ftd245IOState)Read((void*)buf, 1, read);
	if (ftStatus == FTS_OK) {
		if (read == 1)
		{
			pByte = buf[0];
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////
// returns true if read was OK
// false otherwise (on IO error)
bool Ftd245IO::ReadBYTEs(unsigned char* pByte, size_t bufSize, size_t& bytesRead)
{
   size_t read = 0;

   size_t queue;
   Ftd245IOState ftStatus = (Ftd245IOState)GetQueueStatus(queue);

	bytesRead = 0;
	ftStatus = (Ftd245IOState)Read((void*)pByte, bufSize, read);
	if (ftStatus == FTS_OK) {
		bytesRead = read;
      return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////
// read and trash data from device
// retrurns true if read was OK
// false otherwise (on IO error)
bool Ftd245IO::ReadBYTEsNIL(size_t& bytesRead)
{
	size_t read = 0;

	bytesRead = 0;
   ClearInputChannel(); // try to get empty as fast as possible
   Ftd245IOState ftStatus = (Ftd245IOState)Read((void*)m_scratchBuffer, SCRATCH_SIZE, read);
	if (ftStatus == FTS_OK) {
		bytesRead = read;
		return true;
	}
   return false;
}


//////////////////////////////////////////////////////////////////////
bool Ftd245IO::WriteBYTE(unsigned char byte)
{
	unsigned char buf[5];
	size_t written;
	buf[0] = byte;
	Ftd245IOState ftStatus = (Ftd245IOState)Write((void*)buf, 1, written);
	if (ftStatus == FTS_OK) {
		if ( written == 1 ) {
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////
bool Ftd245IO::WriteBYTEs(unsigned char* pByte, size_t bufSize, size_t& bytesWritten)
{
	size_t written = 0;

	bytesWritten = 0;
	Ftd245IOState ftStatus = (Ftd245IOState)Write((void*)pByte, bufSize, written);
	if (ftStatus == FTS_OK) {
		bytesWritten = written;
		if (bytesWritten == bufSize) {
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////
void Ftd245IO::SetDefaultBuffer()
{
	Ftd245IOState retVal;

   if (m_bufferSize == FT_DEF_BUF_SIZE) return;

	retVal = SetUSBParameters(FT_DEF_BUF_SIZE);
	if (retVal==FTS_OK)
		m_bufferSize = FT_DEF_BUF_SIZE;
}


//////////////////////////////////////////////////////////////////////
void Ftd245IO::SetMaxBuffer()
{
	Ftd245IOState retVal;

   if (m_bufferSize == FT_MAX_BUF_SIZE) return;

	retVal = SetUSBParameters(FT_MAX_BUF_SIZE);
	if (retVal==FTS_OK)
		m_bufferSize = FT_MAX_BUF_SIZE;
}

//////////////////////////////////////////////////////////////////////
size_t Ftd245IO::GetAskSize() const
{
   return FT_OPT_ASK_SIZE(FT_MAX_BUF_SIZE);
}



/// ////////////////////////////////////////////////////////////////////////////
/// Cam Enumeration through libusb
/// ////////////////////////////////////////////////////////////////////////////
#include "ARTUSB_LibUsb.h"

//! @brief  Return true if Nth USB device exists and is a camera.
//static
bool Ftd245IO::DeviceIsCCDCamera(int deviceIndex)
{
   return ARTUSB_LibUsb::DeviceAttached(static_cast<ARTUSB_LibUsb::ARTUSB_LibUsb_Device>(deviceIndex));
}


//! @brief  Retrieve props from usb descriptor.
//static
bool Ftd245IO::GetDeviceProperties(int deviceIndex, char pName[], char pSerial[])
{
   ARTUSB_LibUsb usb;

   unsigned short vid,pid;

   if ( ! ARTUSB_LibUsb::GetDeviceVidPid(static_cast<ARTUSB_LibUsb::ARTUSB_LibUsb_Device>(deviceIndex), vid, pid) ) {
      pName[0] = 0; pSerial[0] = 0;
      return false;
   }

   char manufacturer[c_devStringLength];
   int err = usb.DeviceProperties(vid, pid, manufacturer, pName, pSerial, c_devStringLength);

   if (err != ARTUSB_LibUsb::USB_OK) {
      pName[0] = 0; pSerial[0] = 0;
      return false;
   }
   return true;
}

//! @brief  Get USB Identifier of Nth USB device. Return false if no such device.
//! @brief  buffer provided with pName must be at least 40 chars long.
bool Ftd245IO::DeviceName(int deviceIndex, char pName[])
//static
{
   char serial[c_devStringLength];
   return GetDeviceProperties(deviceIndex, pName, serial);
}

//! @brief  Get USB Serial number of Nth USB device. Return false if no such device.
//! @brief  buffer provided with pName must be at least 40 chars long.
bool Ftd245IO::DeviceSerial(int deviceIndex, char pSerial[])
//static
{
   char name[c_devStringLength];
   return GetDeviceProperties(deviceIndex, name, pSerial);
}
