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
 * @file           SAUSB_LibUsb.cpp
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


#include <sys/types.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>

#include "SAUSB_LibUsb.h"  // our header


// timeout default - host side timeout
#define DEFAULT_TIMEOUT   500

// Enpoints to read and write for (HID devices it is EP1 and EP2)
#define READ_EP   1
#define WRITE_EP  2


///////////////////////////////////////////////////////////////////////////////
/// Static Class Code

/// @brief the USB VID of the Shoestring Astronomy Appliances
const unsigned short SAUSB_LibUsb::SAUSB_VID = 0x134a;

/// @brief the device PID and name of the Shoestring Astronomy Appliance (55-shoestring.rules)
/// Note: sequence must be same as enum SAUSB_LibUsb_Device
const SAUSB_LibUsb::SADevIdRec SAUSB_LibUsb::SAUSB_DEVID[] = {
   { 0x9020,  "hid_gpusb_" }, // /dev/hid_gpusb_b:d  (b=bus number, d= dev number)
   { 0x9022,  "hid_lxusb_" }, // /dev/hid_lxusb_b:d  (b=bus number, d= dev number)
};

///////////////////////////////////////////////////////////////////////////////
/// @brief the USB VID and PID of the Shoestring Astronomy Appliance
/// @param  unsigned short& vid: USB VID
/// @param  unsigned short& pid: USB PID
/// @return true if the asked device is found
bool SAUSB_LibUsb::GetDeviceVidPid(const SAUSB_LibUsb_Device device, unsigned short& vid, unsigned short& pid)
{
   if ( (device>=SAUSB_FIRST) && (device<SAUSB_LAST) ) {
      vid = SAUSB_VID;
      pid = SAUSB_DEVID[device].devPid;
      return true;
   }
   return false;
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Checks if a device is plugged in
/// @return true if the asked device is found
bool SAUSB_LibUsb::DeviceAttached(const SAUSB_LibUsb_Device device)
{
   bool retVal = false;
   DIR *dp;
   struct dirent *dirp;

   if ( (dp  = ::opendir("/dev") ) == NULL) return false; // cannot open dir

   while ( (dirp = ::readdir(dp) ) != NULL) {
      if ( (device>=SAUSB_FIRST) && (device<SAUSB_LAST) ) {
         if ( ::strncasecmp(dirp->d_name, SAUSB_DEVID[device].devName, ::strlen(SAUSB_DEVID[device].devName)) == 0) { retVal = true; break;};
      }
   }
   ::closedir(dp);  // finally close the directory

   return retVal;
}


///////////////////////////////////////////////////////////////////////////////
/// Dynamic Class Code


SAUSB_LibUsb::SAUSB_LibUsb()
{
}


SAUSB_LibUsb::~SAUSB_LibUsb()
{
}

///////////////////////////////////////////////////////////////////////////////
///
/// @brief Open the GPUSB device
/// @return a status as SAUSB_LibUsb_Errors
int SAUSB_LibUsb::DeviceOpen(const SAUSB_LibUsb_Device device)
{
   if ( (device>=SAUSB_FIRST) && (device<SAUSB_LAST) ) {
      return UxUsb_LibUsb::DeviceOpen(SAUSB_VID, SAUSB_DEVID[device].devPid);
   }
   else return USB_INV_HANDLE;
}



/// @brief Return a byte through Interrupt Read from EP1 into uchar
/// @return A status as SAUSB_LibUsb_Errors
int SAUSB_LibUsb::ReadByte(unsigned char &b)
{
   return UxUsb_LibUsb::ReadByte(READ_EP, b);
}


/// @brief Send a byte through Interrupt Write to EP1 from uchar
/// @return A status as SAUSB_LibUsb_Errors
int SAUSB_LibUsb::WriteByte(unsigned char b)
{
   return UxUsb_LibUsb::WriteByte(WRITE_EP, b);
}


