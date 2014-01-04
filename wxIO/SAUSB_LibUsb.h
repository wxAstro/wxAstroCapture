/**

  Shoestring Appliance - Control Library

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
 * @file           SAUSB_LibUsb.h
 *****************************************************************************
 * class SAUSB_LibUsb
 *
 * Shoestring Astronomy Appliance driver through libUsb for the
 * GPUSB and LXUSB products running under Linux
 *
 * This is part of the Guiding Driver Library of wxAstroCapture
 *
 * Copyright (C) 2009 Martin Burri  (bm98@burri-web.org)
 *
 * Pls note that the program has to link libusb
 *
 * Also note that the file "55-shoestring.rules" has to be copied
 * into the /etc/udev/rules.d  folder to allow access for non root users
 * The rules file will assign group permission to the plugdev group
 * and in addition create a link in /dev as follows:
 * GPUSB device   /dev/hid_gpusb_b:d   (b= USB bus number, d=device no on bus)
 * LXUSB device   /dev/hid_lxusb_b:d   (b= USB bus number, d=device no on bus)
 * so it is easy to detect the presence of one of the devices
 *
 * The driver will disconnect any drivers already connected in order to access
 * the device - it will however not reconnect them so you have to do a
 * plug cycle to make them available to other drivers again.
 *
 *<hr>
 *
 * @b Project      wxAstroCapture<br>
 *
 * @author         M. Burri
 * @date           10-Jan-2009
 *
 *****************************************************************************
 *<hr>
 * @b Updates
 * - dd-mmm-yyyy V. Name: Description
 *
 *****************************************************************************/

#ifndef _SAUSB_LIBUSB_H__
#define _SAUSB_LIBUSB_H__

#include <usb.h>   // get libUsb headers

#include "UxUsb_LibUsb.h"

// implements a wrapper for the Linux libUSB user mode layer
// provides the Shoestring LXUSB and GPUSB VID/PID
// and routes USB access to the proper endpoints


class SAUSB_LibUsb : public UxUsb_LibUsb
{
public:
   /// @brief  Defines the devices
   enum SAUSB_LibUsb_Device {
      SAUSB_FIRST = 0,
      SAUSB_GPUSB = SAUSB_FIRST,
      SAUSB_LXUSB,
      SAUSB_LAST,
   };

public:
   /// @brief the USB VID and PID of the Shoestring Astronomy Appliance (55-shoestring.rules)
   /// @param  unsigned short& vid: USB VID
   /// @param  unsigned short& pid: USB PID
   /// @return true if the asked device is found
   static bool GetDeviceVidPid(const SAUSB_LibUsb_Device device, unsigned short& vid, unsigned short& pid);

   /// @brief Checks if a device is plugged in
   /// @return true if the asked device is found
   static bool DeviceAttached(const SAUSB_LibUsb_Device device);


public:
   SAUSB_LibUsb();
   virtual ~SAUSB_LibUsb();

public:

   /// @brief  Open the GPUSB device
   /// @return a status as SAUSB_LibUsb_Errors
   int DeviceOpen(const SAUSB_LibUsb_Device device);

   /// @brief Return a byte through Interrupt Read from EP1 into uchar
   /// @return A status as SAUSB_LibUsb_Errors
   int ReadByte(unsigned char &b);

   /// @brief Send a byte through Interrupt Write to EP1 from uchar
   /// @return A status as SAUSB_LibUsb_Errors
   int WriteByte(unsigned char b);


private:
   // maintain the known Shoestring Astronomy Appliances here

   /// @brief the USB PID and Name record
   struct SADevIdRec {
      const unsigned short devPid;
      const char*          devName;
   };
   /// @brief the USB PID and device names of the Shoestring Astronomy Appliance (55-shoestring.rules)
   static const SADevIdRec SAUSB_DEVID[SAUSB_LAST];
   /// @brief the USB VID of the Shoestring Astronomy Appliance (55-shoestring.rules)
   static const unsigned short SAUSB_VID;

};


#endif // _SAUSB_LIBUSB_H__
