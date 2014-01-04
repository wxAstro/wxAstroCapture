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
 * @file           ARTUSB_LibUsb.h
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

#ifndef _ARTUSB_LIBUSB_H_
#define _ARTUSB_LIBUSB_H_

#include <usb.h>   // get libUsb headers

#ifdef INDI
   #include "UxUsb_LibUsb.h"
#else
   #include "../wxIO/UxUsb_LibUsb.h"
#endif


// implements a wrapper for the Linux libUSB user mode layer
// provides the Shoestring LXUSB and GPUSB VID/PID
// and routes USB access to the proper endpoints


class ARTUSB_LibUsb : public UxUsb_LibUsb
{
public:
   /// @brief  Defines the Art/ATIK camera devices
   enum ARTUSB_LibUsb_Device {
      ARTUSB_FIRST=0,
      ARTUSB_ARTEMIS = ARTUSB_FIRST,
      ARTUSB_ATIK16,
      ARTUSB_ATIK16HR,
      ARTUSB_ATIK16C,
      ARTUSB_ATIK16HRC,
      ARTUSB_ATIK16IC,
      ARTUSB_ATIK16ICC,
      ARTUSB_ATIK16ICS,
      ARTUSB_ATIK16ICSC,
      ARTUSB_MINIART,
      ARTUSB_LAST,
   };

public:
   /// @brief the USB VID and PID of the Artemis/Atik Cameras
   /// @param  const ARTUSB_LibUsb_Device device: deviceIndex to query
   /// @param  unsigned short& vid: USB VID
   /// @param  unsigned short& pid: USB PID
   /// @return true if the asked device is found
   static bool GetDeviceVidPid(const ARTUSB_LibUsb_Device device, unsigned short& vid, unsigned short& pid);

   /// @brief the /dev/atik_something path of the Artemis/Atik Cameras
   /// @param  const ARTUSB_LibUsb_Device device: deviceIndex to query
   /// @param  char path[]: provided string to fill the path (make sure it is at least 20 chars long)
   /// @return true if the asked device is found
   static bool GetDevicePath(const ARTUSB_LibUsb_Device device, char path[]);

   /// @brief Checks if a device is plugged in
   /// @param  const ARTUSB_LibUsb_Device device: deviceIndex to query
   /// @return true if the asked device is found
   static bool DeviceAttached(const ARTUSB_LibUsb_Device device);


public:
   ARTUSB_LibUsb();
   virtual ~ARTUSB_LibUsb();

public:
   /// @brief  Open the Artemis/ATIK device
   /// @return a status as ARTUSB_LibUsb_Errors
   int DeviceOpen(const ARTUSB_LibUsb_Device device);

   /// @brief Return a number of bytes from  USB
   /// @return A status as ARTUSB_LibUsb_Errors
   int ReadFTDI(unsigned long& len, unsigned char *b, unsigned long offset=0);

   /// @brief Send a number of bytes through USB from b
   /// @return A status as ARTUSB_LibUsb_Errors
   int WriteFTDI(unsigned long len, unsigned char *b);


   /// @brief Send a read command through USB from b
   /// @return A status as ARTUSB_LibUsb_Errors
   int ReadFTDIcmd(unsigned char cmd,  unsigned short subCmd, unsigned short channel, unsigned char *b, unsigned short& len);

   /// @brief Send a command through USB from b
   /// @return A status as ARTUSB_LibUsb_Errors
   int WriteFTDIcmd(unsigned char cmd, unsigned short subCmd, unsigned short channel);


private:
   // maintain the known Artemis/ATIK cameras here

   /// @brief the USB PID and Name record
   struct ArtDevIdRec {
      const unsigned short devPid;
      const char*          devName;
   };
   /// @brief the USB PID and device names of the Artemis/Atik Cameras (55-atik.rules)
   static const ArtDevIdRec ARTCAM_DEVID[ARTUSB_LAST];
   /// @brief the USB VID of the Artemis/Atik Cameras (55-atik.rules)
   static const unsigned short ARTUSB_VID;

};


#endif // _ARTUSB_LIBUSB_H_
