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
 * @file           UxUsb_LibUsb.h
 *****************************************************************************
 * class UxUsb_LibUsb
 *
 * Astronomy USB Appliance driver for Linux using libUsb
 *  Implements Usb Access for Linux using libusb
 *
 * This is part of the Driver Library of wxAstroCapture
 *
 * Copyright (C) 2009 Martin Burri  (bm98@burri-web.org)
 *
 * Pls note that the program has to link libusb
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
 * @date           01-Feb-2009
 *
 *****************************************************************************
 *<hr>
 * @b Updates
 * - dd-mmm-yyyy V. Name: Description
 *
 *****************************************************************************/

#ifndef _UXUSB_LIBUSB_H_
#define _UXUSB_LIBUSB_H_

#include "usb.h"

class UxUsb_LibUsb
{
public:
   UxUsb_LibUsb();
   virtual ~UxUsb_LibUsb();

public:
   typedef enum XferType {
      UNDEF, INTERRUPT, BULK, ISO
      };

   /// Defines the library errors
   typedef enum UsbErrors {
     USB_OK                        = 0,      // All is ok
     USB_TIMEOUT                   = -1,     // An USB timeout (can be OK on reads)

     USB_GEN_ERR                   = -10,     // Generic library error
     USB_INV_HANDLE                = -11,     // Invalid Handle
     USB_IO_ERR                    = -13,     // Read/Write Error
     USB_SW_ERR                    = -19,     // Software Error (not initialized etc)

     // USB Error Codes -1000 - -1999
     USB_USB_ERR                   = -1000,  // Generic USB error
     USB_USB_NOT_IMPL              = -1001,  // Function not implemented
     USB_USB_DEV_NA                = -1002,  // USB Device Not Available
     USB_USB_OPEN_FAILED           = -1003,  //  Failed to open the USB device.
     USB_USB_CONTROL_RW_ERR        = -1010,  //  Failed to RW via EP0
     USB_USB_BULK_RW_ERR           = -1011,  //  Failed to RW with Bulk cmd
     USB_USB_INT_RW_ERR            = -1012,  //  Failed to RW with Interrupt cmd
     USB_USB_ENDPOINT_NA           = -1020,  //  Endpoint not available
     USB_USB_WRONG_ENDPOINT        = -1021,  //  Endpoint not available
     USB_USB_BAD_LENGTH            = -1022,  //  RW length does not match given parameter

     USB_IHEX_ERR                  = -1050,  //  Intel HEX File error
     USB_IHEX_FNF                  = -1051,  //  Intel HEX File not found
   };

   /// @brief An OS independent millisecond sleep command
   void Sleep_ms(unsigned short  mseconds); // portable sleep

   /// USB device management

   int DeviceProperties(unsigned short vid, unsigned short pid,
                           char* manufacturer, char* product, char* serial, unsigned short bufLen);

   /// @brief  Opens a specific USB device
   /// @param  vid: Usb VID
   /// @param  pid: Usb PID
   /// @return a status as GenUsbErrors
   int DeviceOpen(unsigned short vid, unsigned short pid);

   /// @brief Close and release the USB device
   /// @return A status as GenUsbErrors
   int DeviceClose();

   /// @brief Collects and prints the USB information to stderr (device has to be open)
   /// @return A status as GenUsbErrors
   int DeviceInfo(void);

   /// @brief Returns if the device is OK and usable
   /// @return Returns true if the device is OK and opened
   bool DeviceAvailable(void);


   /// USB Control Interface
   /// @brief Read through EP0 (USB Control Command)
   /// @return A status as GenUsbErrors
   int ControlRead(unsigned char reqCode,  unsigned short value, unsigned short index, unsigned char *b, unsigned short& len);
   /// @brief Write through EP0 (USB Control Command)
   /// @return A status as GenUsbErrors
   int ControlWrite(unsigned char reqCode, unsigned short value, unsigned short index, unsigned char *b, unsigned short len);

   /// USB Interface
   /// @brief Read from EPn len bytes into buffer b
   /// @return A status as GenUsbErrors
   int Read(unsigned short ep, unsigned long& len, unsigned char *b, unsigned long offset=0);
   /// @brief Write to EPn len bytes from buffer b
   /// @return A status as GenUsbErrors
   int Write(unsigned short ep, unsigned long len, unsigned char *b);

   /// @brief Read from EPn only one byte into buffer b
   /// @return A status as GenUsbErrors
   int ReadByte(unsigned short ep, unsigned char& b);
   /// @brief Write to EPn only one byte from buffer b
   /// @return A status as GenUsbErrors
   int WriteByte(unsigned short ep, unsigned char b);

   /// @brief Get the actual timeout values for USB operations
	void GetTimeouts(unsigned long &dwReadTimeout, unsigned long &dwWriteTimeout);
   /// @brief Set the actual timeout values for USB operations (10 < x <10'000  ms)
	void SetTimeouts(unsigned long dwReadTimeout, unsigned long dwWriteTimeout);

private:
   static void Sleep_ms_UX(unsigned short  mseconds );
   /// @brief Bulk Read from EPn len bytes into buffer b, packet offset applied
   int BulkRead(unsigned short ep, unsigned long& len, unsigned char* b, unsigned long offset=0);
   /// @brief Bulk Write to EPn len bytes from buffer b, packet offset applied
   int BulkWrite(unsigned short ep, unsigned long len, unsigned char* b);
   /// @brief Bulk Read from EPn len bytes into buffer b, packet offset applied
   int IntRead(unsigned short ep, unsigned long& len, unsigned char* b);
   /// @brief Bulk Write to EPn len bytes from buffer b, packet offset applied
   int IntWrite(unsigned short ep, unsigned long len, unsigned char* b);

   int AssignEndpoints(void);

   usb_dev_handle*    m_DevHandle;
   struct usb_device* m_USBDevice;
   struct usb_bus*    m_busses;
   bool               m_devOK;

   // helper class for internal buffering
   class IOBuffer
   {
   public:
      static const size_t C_BUFFERSIZE_SMALL;
      static const size_t C_BUFFERSIZE_LARGE;

      IOBuffer(size_t size);
      ~IOBuffer();

      void Init();

      size_t Size();
      size_t Level();
      size_t Available();

      // retrieve bytes
      size_t GetBytes(unsigned char *dst, size_t amount);
      // stuff bytes
      size_t PutBytes(const unsigned char *src, size_t amount);

   private:
      unsigned char     *m_buffer;     // holds the buffer
      unsigned char     *m_pBuffer;    // ptr within buffer
      size_t             m_size;
      size_t             m_level;      // bytes in buffer
      size_t             m_remaining;  // bytes left to buffer
   };


   // internal endpoint descriptor
   struct EndPointDescT {
      EndPointDescT() :pScratchArea(NULL), pInBuffer(NULL), pOutBuffer(NULL) {Init();};
      void Init() {epNumber=-1; epSize=0; epType=UNDEF;
                   if (pScratchArea) delete pScratchArea; pScratchArea=NULL; scratchSize=0;
                   if (pInBuffer) delete pInBuffer; pInBuffer = NULL;
                   if (pOutBuffer) delete pOutBuffer; pOutBuffer = NULL;
                   directionIN=false; directionOUT=false;
                  };

      int            epNumber;   // valid if it corresponds with the
      bool           directionIN;
      bool           directionOUT;
      XferType       epType;
      size_t         epSize;
      size_t         scratchSize; // each EP has it's own io scratch buffer
      unsigned char* pScratchArea; // to avoid alloc/dealloc mem

      IOBuffer*      pInBuffer;   // the IO buffer
      IOBuffer*      pOutBuffer;   // the IO buffer
   };

   EndPointDescT m_Endpoint[USB_MAXENDPOINTS];

   unsigned long m_rxTimeout;
   unsigned long m_txTimeout;

};

#endif // _UXUSB_LIBUSB_H_
