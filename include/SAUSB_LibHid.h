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
 * @file           SAUSB_LibHid.h
 *****************************************************************************
 * class SAUSB_LibUsb
 *
 * Shoestring Astronomy Appliance driver through hid services for the
 * GPUSB and LXUSB products running under Windows
 *
 * This is part of the Guiding Driver Library of wxAstroCapture
 *
 * Copyright (C) 2009 Martin Burri  (bm98@burri-web.org)
 *
 * Pls note that the program has to link hid.lib & setupapi.lib
 *
 *<hr>
 *
 * @b Project      wxAstroCapture<br>
 *
 * @author         M. Burri
 * @date           15-Jan-2009
 *
 *****************************************************************************
 *<hr>
 * @b Updates
 * - dd-mmm-yyyy V. Name: Description
 *
 *****************************************************************************/

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the SAUSB_LibHid_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// SAUSB_LibHid_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef SAUSB_LIBHID_EXPORTS
#define SAUSB_LIBHID_API __declspec(dllexport)
#else
#define SAUSB_LIBHID_API __declspec(dllimport)
#endif

#include <windows.h>

// This class is exported from the SAUSB_LibHid.dll
class SAUSB_LIBHID_API SAUSB_LibHid
{
public:

   /// @brief the USB VID of the Shoestring Astronomy Appliances
   static const unsigned short SAUSB_VID;
   /// @brief the USB PID of the Shoestring Astronomy GPUSB Appliance
   static const unsigned short GPUSB_PID;
   /// @brief the USB PID of the Shoestring Astronomy LXUSB Appliance
   static const unsigned short LXUSB_PID;

   /// @brief  Defines the library errors
   enum SAUSB_LibHid_Errors {
      USB_OK                        = 0,      // >=0 : All is ok

     USB_GEN_ERR                   = -1,     // Generic library error
     USB_INV_HANDLE                = -2,     // Invalid Handle
     USB_IO_ERR                    = -3,     // Read/Write Error

     // USB Error Codes -1000 - -1999
     USB_USB_ERR                   = -1000,  // Generic USB error
     USB_USB_NOT_IMPL              = -1001,  // Function not implemented
     USB_USB_DEV_NA                = -1002,  // USB Device Not Available
     USB_USB_OPEN_FAILED           = -1003,  //  Failed to open the USB device.
     USB_USB_CONTROL_RW_ERR        = -1004,  //  Failed to RW via Control EP0
     USB_USB_BULK_RW_ERR           = -1005,  //  Failed to RW with Bulk cmd
     USB_USB_INT_RW_ERR            = -1006,  //  Failed to RW with Interrupt cmd
     USB_USB_ENDPOINT_NA           = -1007,  //  Endpoint not available
     USB_USB_BAD_LENGTH            = -1008,  //  RW length does not match given parameter
   };

   /// @brief  Defines the devices
   enum SAUSB_LibHid_Device {
      SAUSB_GPUSB = 0,
      SAUSB_LXUSB,
   };


   /// @brief Checks if a device is plugged in
   /// @return true if the asked device is found
   static bool DeviceAttached(const SAUSB_LibHid_Device device);

   /// @brief Sleep for an amount of milliseconds
   static void Sleep_ms(unsigned short  mseconds ); // OS independent sleep

private:
   /// @brief the device name of the Shoestring Astronomy GPUSB Appliance (55-shoestring.rules)
   static const char* GPUSB_DEVNAME;
   /// @brief the device name of the Shoestring Astronomy LXUSB Appliance (55-shoestring.rules)
   static const char* LXUSB_DEVNAME;



public:
   SAUSB_LibHid(void);
   virtual ~SAUSB_LibHid();

   /// @brief  Open the GPUSB device
   /// @return a status as SAUSB_LibHid_Errors
   int DeviceOpen(const SAUSB_LibHid_Device device);

   /// @brief Close and release the USB device
   /// @return A status as SAUSB_LibHid_Errors
   int DeviceClose();

   /// @brief Collects and prints the USB information to stderr (device has to be open)
   /// @return A status as SAUSB_LibHid_Errors
   int DeviceInfo(void);

   /// @brief Returns if the device is OK and usable
   /// @return Returns true if the device is OK and opened
   bool BoardAvailable(void);

   /// @brief Return a byte through Interrupt Read from EP1 into uchar
   /// @return A status as SAUSB_LibHid_Errors
   int ReadByte(unsigned char &b);

   /// @brief Send a byte through Interrupt Write to EP1 from uchar
   /// @return A status as SAUSB_LibHid_Errors
   int WriteByte(unsigned char b);

   /// @brief Get the actual timeout values for USB operations
   void GetTimeouts(unsigned long &dwReadTimeout, unsigned long &dwWriteTimeout);
   /// @brief Set the actual timeout values for USB operations (10 < x <10'000  ms)
   void SetTimeouts(unsigned long dwReadTimeout, unsigned long dwWriteTimeout);


private:
   /// @brief Open an USB device with VID and PID
   int DeviceOpen(unsigned short vid, unsigned short pid);

   HANDLE            m_DevHandle;  // the device handle
   HANDLE            m_ReportEvent;

   bool              m_devOK;      // state var

   unsigned long m_rxTimeout;
   unsigned long m_txTimeout;
};

