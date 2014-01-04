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
 * @file           ARTUSB_LibUsb.cpp
 *****************************************************************************
 * class ARTUSB_LibUsb
 *
 * Astronomy USB Appliance driver through libUsb.
 * The basic driver for Artemis/ATIK cameras with USB 1.1 interface
 * supported cameras are: see AtikVidPid.h
 *
 * This is part of the Driver Library of wxAstroCapture
 *
 * Copyright (C) 2009 Martin Burri  (bm98@burri-web.org)
 *
 * Pls note that the program has to link libusb
 *
 * Also note that the file "55-atik.rules" has to be copied
 * into the /etc/udev/rules.d  folder to allow access for non root users
 * The rules file will assign group permission to the plugdev group
 * and in addition create a link in /dev as follows:
 *  /dev/atik_cammodel
 * e.g. atik_16hr
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
 * @date           14-Mar-2009
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

#include "ARTUSB_LibUsb.h"  // our header

#include "ArtVidPid.h"


// timeout default - host side timeout
#define DEFAULT_TIMEOUT   500

// the FTDI Endpoints
#define FTDI_EP_OUT     0x02
#define FTDI_EP_IN      0x01  // will get 0x81 later (USB IntoHost Direction Flag)

///////////////////////////////////////////////////////////////////////////////
/// Static Class Code


/// @brief the USB VID of the Artemis/ATIK Cameras
const unsigned short ARTUSB_LibUsb::ARTUSB_VID = FTDI_VENDOR_VID_x;

/// @brief the device PID and name of the Artemis/ATIK Cameras (55-atik.rules)
/// Note: sequence must be same as enum ARTUSB_LibUsb_Device
const ARTUSB_LibUsb::ArtDevIdRec ARTUSB_LibUsb::ARTCAM_DEVID[] = {
   { FTDI_ARTEMIS_PID_x,         "atik_art" },
   { FTDI_ATIK_ATK16_PID_x,      "atik_16" },
   { FTDI_ATIK_ATK16HR_PID_x,    "atik_16hr" },
   { FTDI_ATIK_ATK16C_PID_x,     "atik_16_c" },
   { FTDI_ATIK_ATK16HRC_PID_x,   "atik_16hr_c" },
   { FTDI_ATIK_ATK16IC_PID_x,    "atik_16ic" },
   { FTDI_ATIK_ATK16ICC_PID_x,   "atik_16ic_c" },
   { FTDI_ATIK_ATK16ICS_PID_x,   "atik_16ics" },
   { FTDI_ATIK_ATK16ICSC_PID_x,  "atik_16ics_c" },
   { FTDI_MINIART_PID_x,         "atik_mini" },
};

///////////////////////////////////////////////////////////////////////////////
/// @brief the USB VID and PID of the Artemis/Atik Cameras
/// @param  const ARTUSB_LibUsb_Device device: deviceIndex to query
/// @param  unsigned short& vid: USB VID
/// @param  unsigned short& pid: USB PID
/// @return true if the asked device is found
bool ARTUSB_LibUsb::GetDeviceVidPid(const ARTUSB_LibUsb_Device device, unsigned short& vid, unsigned short& pid)
{
   if ( (device>=ARTUSB_FIRST) && (device<ARTUSB_LAST) ) {
      vid = ARTUSB_VID;
      pid = ARTCAM_DEVID[device].devPid;
      return true;
   }
   return false;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief the /dev/atik_somethingS path of the Artemis/Atik Cameras
/// @param  const ARTUSB_LibUsb_Device device: deviceIndex to query
/// @param  char path[]: provided string to fill the path (make sure it is at least 20 chars long)
/// @return true if the asked device is found
bool ARTUSB_LibUsb::GetDevicePath(const ARTUSB_LibUsb_Device device, char path[])
{
   if ( DeviceAttached(device) ) {
      ::snprintf(path, 20, "/dev/%sS", ARTCAM_DEVID[device].devName);
      return true;
   }
   return false;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Checks if a device is plugged in
/// @param  const ARTUSB_LibUsb_Device device: deviceIndex to query
/// @return true if the asked device is found
bool ARTUSB_LibUsb::DeviceAttached(const ARTUSB_LibUsb_Device device)
{
   bool retVal = false;
   DIR *dp;
   struct dirent *dirp;

   if ( (dp  = ::opendir("/dev") ) == NULL) return false; // cannot open dir

   while ( (dirp = ::readdir(dp) ) != NULL) {
      if ( (device>=ARTUSB_FIRST) && (device<ARTUSB_LAST) ) {
         if ( ::strncasecmp(dirp->d_name, ARTCAM_DEVID[device].devName, ::strlen(ARTCAM_DEVID[device].devName)) == 0) { retVal = true; break;};
      }
   }
   ::closedir(dp);  // finally close the directory

   return retVal;
}

///////////////////////////////////////////////////////////////////////////////
/// Dynamic Class Code


ARTUSB_LibUsb::ARTUSB_LibUsb()
{
}


ARTUSB_LibUsb::~ARTUSB_LibUsb()
{
}

///////////////////////////////////////////////////////////////////////////////
///
/// @brief Open the ARTUSB device
/// @return a status as ARTUSB_LibUsb_Errors
int ARTUSB_LibUsb::DeviceOpen(const ARTUSB_LibUsb_Device device)
{
   if ( (device>=ARTUSB_FIRST) && (device<ARTUSB_LAST) ) {
      return UxUsb_LibUsb::DeviceOpen(ARTUSB_VID, ARTCAM_DEVID[device].devPid);
   }
   else return USB_INV_HANDLE;
}


/// @brief Return a number of bytes from  USB
/// @return A status as ARTUSB_LibUsb_Errors
int ARTUSB_LibUsb::ReadFTDI(unsigned long& len, unsigned char *b, unsigned long offset)
{
   return UxUsb_LibUsb::Read(FTDI_EP_IN, len, b, offset);
}

/// @brief Send a number of bytes through USB from b
/// @return A status as ARTUSB_LibUsb_Errors
int ARTUSB_LibUsb::WriteFTDI(unsigned long len, unsigned char *b)
{
   return UxUsb_LibUsb::Write(FTDI_EP_OUT, len, b);
}



/// @brief Send a read command through USB from b
/// @return A status as ARTUSB_LibUsb_Errors
int ARTUSB_LibUsb::ReadFTDIcmd(unsigned char cmd,  unsigned short subCmd, unsigned short channel, unsigned char *b, unsigned short& len)
{
   return UxUsb_LibUsb::ControlRead(cmd, subCmd, channel, b, len);
}


/// @brief Send a command through USB from b
/// @return A status as ARTUSB_LibUsb_Errors
int ARTUSB_LibUsb::WriteFTDIcmd(unsigned char cmd, unsigned short subCmd, unsigned short channel)
{
   return UxUsb_LibUsb::ControlWrite(cmd, subCmd, channel, NULL,0);
}



