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
 * @file           Ftd245IOUX.h
 *****************************************************************************
 * class Ftd245IOUX
 *
 * Astronomy USB Appliance driver for Linux using ftdi_sio
 * Upper Level driver for FTDI245 equipped boards
 *  specifically for the Artemis/ATIK USB1.1 cameras
 *
 * This is part of the Driver Library of wxAstroCapture
 *
 * Copyright (C) 2009 Martin Burri  (bm98@burri-web.org)
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


#ifndef _FTD245IOUX_H__
#define _FTD245IOUX_H__

#include "Ftd245IO.h"

//////////////////////////////////////////////////////////////////////
//! The USB Interface class
class Ftd245IOUX : public Ftd245IO
{
public:
	Ftd245IOUX();
	virtual ~Ftd245IOUX();

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
   int xioctl(int device, int request, void* arg);
   int               m_device;
};



#endif  // _FTD245IOUX_H__
