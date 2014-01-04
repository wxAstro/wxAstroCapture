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
 * @file           UxUsb_LibUsb.cpp
 *****************************************************************************
 * class UxUsb_LibUsb
 *
 * Astronomy USB Appliance driver for Linux using libUsb
 *
 * This is part of the Driver Library of wxAstroCapture
 *  Implements GenericUsb for Linux
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

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "UxUsb_LibUsb.h"

#include "IOBuffer.h"

#ifdef INDI
   #include "GenericDebHelp.h"
#else
   #include "../GenericDebHelp.h"
#endif

// local shortcuts
typedef unsigned char  _byte;
typedef unsigned short _word;
typedef unsigned long  _dword;

// timeout default values - host side timeout
#define HOST_RX_TIMEOUT   1500
#define HOST_TX_TIMEOUT   1500

#define MAX_NULLREAD        20  // number of null read trys
#define MAX_NULLWRITE       10  // number of null write trys

#define USBLIB_TIMEOUT    -110  // the return value

//! defines the buffer size of this class
const size_t UxUsb_LibUsb::C_BUFFERSIZE_SMALL = 0x1000;  // 4kB
const size_t UxUsb_LibUsb::C_BUFFERSIZE_LARGE = 0x8000; // 32kB

///////////////////////////////////////////////////////////////////////////////
/// @brief portable sleep command; take milliseconds as argument
//static
void UxUsb_LibUsb::Sleep_ms_UX(unsigned short  mseconds )
{
   // Windows Sleep uses miliseconds
   // linux usleep uses microsecond
   // the argument is   coming in millisecond.
   long useconds = long(mseconds) * 1000;
   // usleep can only handle <1000000 usec
   if (useconds>=1000000) {
      long seconds = useconds / 1000000;
      sleep(seconds);
      useconds %=1000000;
   }
   if (useconds>20) usleep(useconds);
}

///////////////////////////////////////////////////////////////////////////////
//static
int UxUsb_LibUsb::DeviceProperties(unsigned short vid, unsigned short pid,
                           char* manufacturer, char* product, char* serial, unsigned short bufLen)
{
   usb_bus*          busses;
   usb_dev_handle*   devHandle;

   int err = USB_USB_DEV_NA;
   int retV;

   // rescan to find changes
   usb_find_busses();
   usb_find_devices();
   busses = usb_get_busses();

   usb_bus* bus;// bus  iterator
   for (bus = busses; bus; bus = bus->next) {
      struct usb_device *dev; // device  iterator
      for (dev = bus->devices; dev; dev = dev->next) {
         // check if we search for that device
         if ( (dev->descriptor.idVendor==vid) && (dev->descriptor.idProduct== pid) ) {
            devHandle = usb_open(dev);
            if (devHandle) {
               _V_ ::fprintf(stderr, "UxUsb_LibUsb::DeviceProperties(0x%04x : 0x%04x) - Found Device!!\n", vid, pid);
               retV = usb_get_string_simple(devHandle, dev->descriptor.iManufacturer, manufacturer, bufLen);
               if (retV==-1) {
                  ::fprintf(stderr, "  ERROR in usb_get_string_simple(manufacturer)\n");
               }
               else {
                  _V_ ::fprintf(stderr, "  Manufacturer:  %s\n", manufacturer);
               }
               retV = usb_get_string_simple(devHandle, dev->descriptor.iProduct, product, bufLen);
               if (retV==-1) {
                  ::fprintf(stderr, "  ERROR in usb_get_string_simple(product)\n");
               }
               else {
                  _V_ ::fprintf(stderr, "  Product:       %s\n", product);
               }
               retV = usb_get_string_simple(devHandle, dev->descriptor.iSerialNumber, serial, bufLen);
               if (retV==-1) {
                  ::fprintf(stderr, "  ERROR in usb_get_string_simple(serialnumber)\n");
               }
               else {
                  _V_ ::fprintf(stderr, "  Serial:        %s\n", serial);
               }
               err = (retV!=-1) ? USB_OK : USB_GEN_ERR;
               usb_close(devHandle);
            }
          }
       }
    }
    return err;
}


//!/////////////////////////////////////////////////////////////////////////

UxUsb_LibUsb::UxUsb_LibUsb()
: m_DevHandle(NULL)
, m_USBDevice(NULL)
, m_busses(NULL)
, m_devOK(false)
, m_rxTimeout(HOST_RX_TIMEOUT)
, m_txTimeout(HOST_TX_TIMEOUT)
{
   usb_init();   // Init Linux USB subsystem
   //usb_set_debug(99);
}


UxUsb_LibUsb::~UxUsb_LibUsb()
{
   // we had one before
   DeviceClose();
   m_DevHandle = NULL;
   m_USBDevice = NULL;
   m_busses = NULL;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief An OS independent millisecond sleep command
void UxUsb_LibUsb::Sleep_ms(unsigned short  mseconds)
{
   Sleep_ms_UX(mseconds);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Get the actual timeout values for USB operations
void UxUsb_LibUsb::GetTimeouts(unsigned long &dwReadTimeout, unsigned long &dwWriteTimeout)
{
   dwReadTimeout  = m_rxTimeout;
   dwWriteTimeout = m_txTimeout;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Set the actual timeout values for USB operations (10 < x <20'000  ms)
void UxUsb_LibUsb::SetTimeouts(unsigned long dwReadTimeout, unsigned long dwWriteTimeout)
{
   if ( (dwReadTimeout>10) && (dwReadTimeout<20000) ) m_rxTimeout = dwReadTimeout;
   if ( (dwWriteTimeout>10) && (dwWriteTimeout<20000) ) m_txTimeout = dwWriteTimeout;
}



///////////////////////////////////////////////////////////////////////////////
/// @brief  Opens a specific USB device
/// @param  vid: Usb VID
/// @param  pid: Usb PID
/// @return a status as GenUsbErrors
int UxUsb_LibUsb::DeviceOpen(unsigned short vid, unsigned short pid)
{
   usb_bus *bus;
   int err = USB_USB_DEV_NA;
   int rVal = 0;

   // we had one before
   DeviceClose();

   // rescan to find changes
   usb_find_busses();
   usb_find_devices();
   m_busses = usb_get_busses();

   for (bus = m_busses; bus; bus = bus->next) {
      struct usb_device *dev;
      for (dev = bus->devices; dev; dev = dev->next) {
         // check if we search for that device
         if ( (dev->descriptor.idVendor==vid) && (dev->descriptor.idProduct== pid) ) {
            // Open the device, claim the interface and do your processing
            m_USBDevice = dev;
            m_DevHandle = usb_open(m_USBDevice);
            if (m_DevHandle) {
               int detach_errno = 0;
               err = USB_OK;  // OK so far
               _V_ ::fprintf(stderr, "UxUsb_LibUsb::DeviceOpen - Found Device!!\n");
               // have to claim an interface before we can do anything
               rVal = usb_claim_interface(m_DevHandle, 0);
               if (rVal != 0) {
                   // Try to detach ftdi_sio kernel module - Returns ENODATA if driver is not loaded
                  if ( (usb_detach_kernel_driver_np(m_DevHandle, 0) < 0) && (errno != ENODATA) )
                     detach_errno = errno;

                  rVal = usb_claim_interface(m_DevHandle, 0);
                  if (rVal != 0) {
                     ::fprintf(stderr, "UxUsb_LibUsb::DeviceOpen - usb_claim_interface error: %d %s\n", errno, usb_strerror());
                     err = USB_USB_OPEN_FAILED;
                     usb_close (m_DevHandle);
                     if (detach_errno == EPERM) {
                        ::fprintf(stderr, "UxUsb_LibUsb::DeviceOpen - inappropriate permissions on device!\n");
                     }
                     else {
                        ::fprintf(stderr,  "UxUsb_LibUsb::DeviceOpen - unable to claim usb device. Make sure modules using the device are unloaded!\n");
                     }
                  }
               }//if
               m_devOK =( (USB_OK==err) && (USB_OK==AssignEndpoints()));
               break;
            }
          }
       }
    }
    return err;
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Reset and close the USB device
/// @return A status as GenUsbErrors
int UxUsb_LibUsb::DeviceResetAndClose()
{
   int err = USB_OK;

	if (m_devOK) {
	   err= usb_release_interface(m_DevHandle, 0);
	   err= usb_reset(m_DevHandle);    // discon/reconnect device
	   err= usb_close(m_DevHandle);
	   Sleep_ms(500); // allow some time to reset
		m_devOK = false;
	}
   for (size_t i=0;i<USB_MAXENDPOINTS; i++) m_Endpoint[i].Init(); // init internal EPs
   return err;
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Close and release the USB device
/// @return A status as GenUsbErrors
int UxUsb_LibUsb::DeviceClose()
{
   int err = USB_OK;

	if (m_devOK) {
	   usb_release_interface(m_DevHandle, 0);
	   //usb_reset(m_DevHandle);    // discon/reconnect device
	   usb_close(m_DevHandle);
	   Sleep_ms(500); // allow some time to reset
		m_devOK = false;
	}
   for (size_t i=0;i<USB_MAXENDPOINTS; i++) m_Endpoint[i].Init(); // init internal EPs
   return err;
}


///////////////////////////////////////////////////////////////////////////////
/// @brief scans and assignes endpoints from default config, IF 0, alt 0
int UxUsb_LibUsb::AssignEndpoints()
{
   struct usb_config_descriptor* cfg = &(m_USBDevice->config[0]);
   struct usb_interface_descriptor* iface = &(cfg->interface->altsetting[0]);

   int eptCount = iface->bNumEndpoints;
   _V_ ::fprintf(stderr, "UxUsb_LibUsb::AssignEndpoints - Number of Endpoints present %d\n", eptCount);

   for (int i=0; i<eptCount; i++) {
      struct usb_endpoint_descriptor* epd = &(iface->endpoint[i]);
      unsigned char addr = epd->bEndpointAddress & USB_ENDPOINT_ADDRESS_MASK;
      bool bIn =   ((epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_ENDPOINT_IN);
      bool bBulk = (epd->bmAttributes == USB_ENDPOINT_TYPE_BULK);
      bool bInt =  (epd->bmAttributes == USB_ENDPOINT_TYPE_INTERRUPT);
      //bool bIso =  (epd->bmAttributes == USB_ENDPOINT_TYPE_ISOCHRONOUS);

      // prepare the internal EP descriptors (control EPs will not come here)
      if (bBulk) {
         m_Endpoint[addr].epType = BULK;
         m_Endpoint[addr].epSize  = epd->wMaxPacketSize;  // the EP size
         _V_ ::fprintf(stderr, "Assign Endpoint 0x%02x Bulk, Size %u", addr, m_Endpoint[addr].epSize);
         if(bIn) { // IN
            m_Endpoint[addr].epNumber    = addr | USB_ENDPOINT_IN;
            m_Endpoint[addr].directionIN = true;  _V_ ::fprintf(stderr, " In \n");
            if (m_Endpoint[addr].epSize<=64) {
               m_Endpoint[addr].pInBuffer = new IOBuffer(C_BUFFERSIZE_SMALL);
               m_Endpoint[addr].scratchSize=64*64; // 4kB optimized for USB1.1
            }
            else {
               m_Endpoint[addr].pInBuffer = new IOBuffer(C_BUFFERSIZE_LARGE);
               m_Endpoint[addr].scratchSize=m_Endpoint[addr].pInBuffer->Size() / 2;
            }
            assert( m_Endpoint[addr].pInBuffer->Size()>=m_Endpoint[addr].scratchSize ); // avoid erranous sw settings
         }
         else { // OUT
            m_Endpoint[addr].epNumber     = addr | USB_ENDPOINT_OUT;
            m_Endpoint[addr].directionOUT = true; _V_ ::fprintf(stderr, " Out \n");
            if (m_Endpoint[addr].epSize<=64) {
               m_Endpoint[addr].pOutBuffer = new IOBuffer(C_BUFFERSIZE_SMALL);
               m_Endpoint[addr].scratchSize=64*64;  // smaller EPs
            }
            else {
               m_Endpoint[addr].pOutBuffer = new IOBuffer(C_BUFFERSIZE_LARGE);
               m_Endpoint[addr].scratchSize=m_Endpoint[addr].pOutBuffer->Size() / 2;
            }
            assert( m_Endpoint[addr].pOutBuffer->Size()>=m_Endpoint[addr].scratchSize ); // avoid erranous sw settings
         }
         m_Endpoint[addr].pScratchArea = new unsigned char [m_Endpoint[addr].scratchSize];
         _V_ ::fprintf(stderr, "USB scratch area set to %u \n", m_Endpoint[addr].scratchSize);
      }
      else if (bInt) {
         // Interrupt transfer (has no IO buffer attached)
         m_Endpoint[addr].epType = INTERRUPT;
         m_Endpoint[addr].epSize  = epd->wMaxPacketSize;  // the EP size
         _V_ ::fprintf(stderr, "Assign Endpoint 0x%02x Interrupt, Size %u", addr, m_Endpoint[addr].epSize);
         if(bIn) { // IN
            m_Endpoint[addr].epNumber    = addr | USB_ENDPOINT_IN;
            m_Endpoint[addr].directionIN = true;  _V_ ::fprintf(stderr, " In \n");
         }
         else { // OUT
            m_Endpoint[addr].epNumber     = addr | USB_ENDPOINT_OUT;
            m_Endpoint[addr].directionOUT = true; _V_ ::fprintf(stderr, " Out \n");
         }
         if (m_Endpoint[addr].epSize<32) {
            m_Endpoint[addr].scratchSize=4*32;  // smaller EPs
         }
         else {
            m_Endpoint[addr].scratchSize=4*64;
         }
         m_Endpoint[addr].pScratchArea = new unsigned char [m_Endpoint[addr].scratchSize];
         _V_ ::fprintf(stderr, "USB scratch area set to %u \n", m_Endpoint[addr].scratchSize);
      }
      else {
         ::fprintf(stderr, "UxUsb_LibUsb::AssignEndpoints - Endpoint 0x%2x is unhandled type\n", addr);
      }
   }

   return USB_OK;
}


///////////////////////////////////////////////////////////////////////////////
///
bool UxUsb_LibUsb::DeviceAvailable()
{
	return m_devOK;
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Read through EP0 (USB Control Command)
/// @return A status as GenUsbErrors
int UxUsb_LibUsb::ControlRead(unsigned char reqCode, unsigned short value, unsigned short index, unsigned char *b, unsigned short& len)
{
	int err = USB_OK;
	int lenr = len;

   if (m_devOK) {

      lenr = usb_control_msg(m_DevHandle,
         USB_ENDPOINT_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
            reqCode, value, index, (char*)b, len, m_rxTimeout);

		if (lenr==USBLIB_TIMEOUT) {
			::fprintf(stderr, "UxUsb_LibUsb::ControlRead - USB timeout (%lu)\n", m_rxTimeout);
			err = USB_TIMEOUT;
		}
		else if (lenr<0) {
			::fprintf(stderr, "UxUsb_LibUsb::ControlRead - USB Error %d %s\n", errno, usb_strerror());
			err = USB_USB_CONTROL_RW_ERR;
		}
      if(len != lenr){
         err = USB_USB_BAD_LENGTH;
         ::fprintf(stderr, "UxUsb_LibUsb::ControlRead - bad len %d (%d)\n", lenr, len);
      }
	}
	else{
		err = USB_USB_DEV_NA;
      ::fprintf(stderr, "UxUsb_LibUsb::ControlRead - USB device not available\n");
	}

	return err;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Write through EP0 (USB Control Command)
/// @return A status as GenUsbErrors
int UxUsb_LibUsb::ControlWrite(unsigned char reqCode, unsigned short value, unsigned short index, unsigned char *b, unsigned short len)
{
	int err = USB_OK;
	int lenw = len;

   if (m_devOK) {

      lenw = usb_control_msg(m_DevHandle,
         USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
            reqCode, value, index, (char*)b, len, m_txTimeout);

		if (lenw==USBLIB_TIMEOUT) {
			::fprintf(stderr, "UxUsb_LibUsb::ControlWrite - USB timeout (%lu)\n", m_txTimeout);
			err = USB_TIMEOUT;
		}
		else if (lenw<0) {
			::fprintf(stderr, "UxUsb_LibUsb::ControlWrite - USB Error %d %s\n", errno, usb_strerror());
			err = USB_USB_CONTROL_RW_ERR;
		}
      if(len != lenw){
         err = USB_USB_BAD_LENGTH;
         ::fprintf(stderr, "UxUsb_LibUsb::ControlWrite - bad len %d (%d)\n", lenw, len);
      }
	}
	else{
		err = USB_USB_DEV_NA;
      ::fprintf(stderr, "UxUsb_LibUsb::ControlWrite - USB device not available\n");
	}

	return err;
}


/// USB Interface
/// @brief Read from EPn len bytes into buffer b
/// @return A status as GenUsbErrors
int UxUsb_LibUsb::Read(unsigned short ep, unsigned long& len, unsigned char *b, unsigned long offset)
{
   if (ep>=USB_MAXENDPOINTS) {
      return USB_USB_WRONG_ENDPOINT;
   }
   else if (m_Endpoint[ep].epType==BULK) {
      return BulkRead(ep, len, b, offset);
   }
   else if (m_Endpoint[ep].epType==INTERRUPT) {
      return IntRead(ep, len, b);
   }
   else {
      return USB_USB_WRONG_ENDPOINT;
   }
}

/// @brief Write to EPn len bytes from buffer b
/// @return A status as GenUsbErrors
int UxUsb_LibUsb::Write(unsigned short ep, unsigned long len, unsigned char *b)
{
   if (ep>=USB_MAXENDPOINTS) {
      return USB_USB_WRONG_ENDPOINT;
   }
   else if (m_Endpoint[ep].epType==BULK) {
      return BulkWrite(ep, len, b);
   }
   else if (m_Endpoint[ep].epType==INTERRUPT) {
      return IntWrite(ep, len, b);
   }
   else {
      return USB_USB_WRONG_ENDPOINT;
   }
}


/// @brief Read from EPn only one byte into buffer b
/// @return A status as GenUsbErrors
int UxUsb_LibUsb::ReadByte(unsigned short ep, unsigned char& b)
{
   unsigned char buf[5];
   unsigned long lread = 1;
   int retVal = this->Read(ep, lread, buf);
   if (lread==1) b= buf[0];
   return retVal;
}

/// @brief Write to EPn only one byte from buffer b
/// @return A status as GenUsbErrors
int UxUsb_LibUsb::WriteByte(unsigned short ep, unsigned char b)
{
   unsigned char buf[5]; buf[0]=b;
   int retVal = this->Write(ep, 1, buf);
   return retVal;
}



///////////////////////////////////////////////////////////////////////////////
/// @brief Bulk Read from EPn len bytes into buffer b, packet offset applied
int UxUsb_LibUsb::BulkRead(unsigned short ep, unsigned long& len, unsigned char* b, unsigned long offset)
{
	int err = USB_OK;
   int ept = m_Endpoint[ep].epNumber;
   unsigned long lenr = 0;
   unsigned long rest = len; len = 0;
   unsigned char* pDst = b;

   if (!m_devOK) {
      ::fprintf(stderr, "UxUsb_LibUsb::BulkRead - USB device not available\n");
      return USB_USB_DEV_NA; //ERROR EXIT
   }

   if (!m_Endpoint[ep].directionIN) ept=-1; // must be IN

   if(ept<=0){
      ::fprintf(stderr, "UxUsb_LibUsb::BulkRead - invalid endpoint given %d\n", ep);
      return USB_USB_ENDPOINT_NA; //ERROR EXIT
   }

   if(m_Endpoint[ep].epType!=BULK){
      ::fprintf(stderr, "UxUsb_LibUsb::BulkRead - invalid endpoint type given %d\n", ep);
      return USB_USB_WRONG_ENDPOINT; //ERROR EXIT
   }

   // first try to get from EP buffer
   lenr = m_Endpoint[ep].pInBuffer->GetBytes(pDst, rest);
   rest-=lenr; pDst+=lenr;

   // already served?
   if (rest<=0) {
      len = lenr;
      return USB_OK; //SUCCESS EXIT
   }

   // read more from USB

   unsigned char* buf;
   int xlen;

   xlen = m_Endpoint[ep].scratchSize; // read into local scratch as much as possible
   buf  = m_Endpoint[ep].pScratchArea;

   // real read from USB device
   xlen = usb_bulk_read(m_DevHandle, ept, (char*)buf, xlen, m_rxTimeout);

   if (xlen==USBLIB_TIMEOUT) {
      // USB timeout it is...
      _V_ ::fprintf(stderr, "UxUsb_LibUsb::BulkRead - USB timeout %lu ms\n", m_rxTimeout);
      return USB_TIMEOUT; // TIMEOUT EXIT - No data available
   }
   else if(xlen<0) {
      ::fprintf(stderr, "UxUsb_LibUsb::BulkRead - USB errno %d -> %s\n", errno, usb_strerror());
      return USB_USB_BULK_RW_ERR; //ERROR EXIT
   }


   // this is a null read or something read
   if ( (xlen>0) && (offset>0) ) {
      // reading is a bit strange..
      // some chips (FTDI) return additional status info at the beginning of every USB packet
      // so we have to adjust accordingly
      size_t urbs     = xlen / m_Endpoint[ep].epSize; // how many packets
      size_t urb_rest = xlen % m_Endpoint[ep].epSize; // rest
      size_t cpLen    = m_Endpoint[ep].epSize - offset;
      size_t stuffed;
      // now copy that number of packages into dest minus offset for each packet
      for (size_t i=0; i<urbs; i++) {
         buf+=offset;        // shift ptr
         stuffed = m_Endpoint[ep].pInBuffer->PutBytes(buf, cpLen);   // stuff them
         if (cpLen>stuffed) {
            ::fprintf(stderr, "UxUsb_LibUsb::BulkRead - Error - cannot store all bytes (loc a)\n");
            return USB_SW_ERR; //ERROR EXIT
         }
         buf+=cpLen;
      }
      if (urb_rest>offset) {
         cpLen = urb_rest - offset;
         // copy the last - may be incomplete packet
         buf+=offset;        // shift ptr
         stuffed = m_Endpoint[ep].pInBuffer->PutBytes(buf, cpLen);   // stuff them - can be 0bytes
         if (cpLen>stuffed) {
            ::fprintf(stderr, "UxUsb_LibUsb::BulkRead - Error - cannot store all bytes (loc b)\n");
            return USB_SW_ERR; //ERROR EXIT
         }
      }

   }
   else if (xlen>0) {
      // regular read something (no offset)
      size_t stuffed = m_Endpoint[ep].pInBuffer->PutBytes(buf, xlen);   // stuff them
      if (xlen>int(stuffed)) {
         ::fprintf(stderr, "UxUsb_LibUsb::BulkRead - Error - cannot store all bytes (loc c)\n");
         return USB_SW_ERR; //ERROR EXIT
      }
   }
   else {
      ; // xlen=0 read ??! returned an empty USB URB
   }

   // again get from buffer for the receiver
   lenr += m_Endpoint[ep].pInBuffer->GetBytes(pDst, rest);  // can be 0 due to the offset kind of reading
   len = lenr;
//   ::fprintf(stderr, "UxUsb_LibUsb::BulkRead - Info- len %d - lenr %d\n", len, lenr);
   return err;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Bulk Write to EPn len bytes from buffer b
int UxUsb_LibUsb::BulkWrite(unsigned short ep, unsigned long len, unsigned char *b)
{
   int err = USB_OK;
   int ept = m_Endpoint[ep].epNumber;
   unsigned long lenw = 0;
   unsigned long rest = len;
   unsigned char* pSrc = b;

   if (!m_devOK) {
      ::fprintf(stderr, "UxUsb_LibUsb::BulkWrite - USB device not available\n");
      return USB_USB_DEV_NA; //ERROR EXIT
   }

   if (!m_Endpoint[ep].directionOUT) ept=-1; // must be IN

   if(ept<=0){
      ::fprintf(stderr, "UxUsb_LibUsb::BulkWrite - invalid endpoint given %d\n", ep);
      return USB_USB_ENDPOINT_NA; //ERROR EXIT
   }

   if(m_Endpoint[ep].epType!=BULK){
      ::fprintf(stderr, "UxUsb_LibUsb::BulkWrite - invalid endpoint type given %d\n", ep);
      return USB_USB_WRONG_ENDPOINT; //ERROR EXIT
   }

   const unsigned long BuffSize = m_Endpoint[ep].scratchSize;
   unsigned char* buf = m_Endpoint[ep].pScratchArea;
   int xlen, blen;
   int nullWrite = MAX_NULLWRITE;

   while (rest>0) {
      // first stuff the bytes into our buffer
      lenw = m_Endpoint[ep].pOutBuffer->PutBytes(pSrc, rest);
      rest-=lenw; pSrc+=lenw; // adjust - if not all fits we have a rest

      // try to write the internal buffer in chuncks
      while ( (m_Endpoint[ep].pOutBuffer->Level()>0) && (err == USB_OK) ) {
         // load from IO buffer
         blen = m_Endpoint[ep].pOutBuffer->GetBytes(buf, BuffSize); // determine amount to write (available)

         while (nullWrite>0) {
            // real write to USB device
            xlen = usb_bulk_write(m_DevHandle, ept, (char*)buf, blen, m_txTimeout);  // write to USB

            if (xlen==USBLIB_TIMEOUT) {
               // USB timeout it is...
               ::fprintf(stderr, "UxUsb_LibUsb::BulkWrite - USB timeout %lu ms; trying again %d\n", m_txTimeout, nullWrite);
               err = USB_TIMEOUT;
               nullWrite--;
            }
            else if(xlen<0){
               ::fprintf(stderr, "UxUsb_LibUsb::BulkWrite - USB Error %d %s\n", errno, usb_strerror());
               return USB_USB_BULK_RW_ERR; // ERROR EXIT
            }
            else {
               err = USB_OK; // reset err on write
               // assuming we have written all.... No check so far!!
               break; // from while nullwrite with OK
            }
         }//while trys
         // here we have an ERR status

      }//while level
   }//while rest

	return err; // OK or TIMEOUT
}



///////////////////////////////////////////////////////////////////////////////
/// @brief Bulk Read from EPn len bytes into buffer b
int UxUsb_LibUsb::IntRead(unsigned short ep, unsigned long& len, unsigned char* b)
{
	int err = USB_OK;
   int ept = m_Endpoint[ep].epNumber;
   unsigned long rest = len; len = 0;

   if (!m_devOK) {
      ::fprintf(stderr, "UxUsb_LibUsb::IntRead - USB device not available\n");
      return USB_USB_DEV_NA; //ERROR EXIT
   }

   if (!m_Endpoint[ep].directionIN) ept=-1; // must be IN

   if(ept<=0){
      ::fprintf(stderr, "UxUsb_LibUsb::IntRead - invalid endpoint given %d\n", ep);
      return USB_USB_ENDPOINT_NA; //ERROR EXIT
   }

   if(m_Endpoint[ep].epType!=INTERRUPT){
      ::fprintf(stderr, "UxUsb_LibUsb::IntRead - invalid endpoint type given %d\n", ep);
      return USB_USB_WRONG_ENDPOINT; //ERROR EXIT
   }

   unsigned char*  buf = m_Endpoint[ep].pScratchArea; // use internal buffer
   const unsigned long BuffSize = m_Endpoint[ep].scratchSize;
   int xlen, blen; // current read size

   while (rest>0) {
      // still something to read ?
      blen = (rest>BuffSize) ? BuffSize : rest; // determine amount to read in this round

      xlen = usb_interrupt_read(m_DevHandle, ept, (char*)buf, blen, m_rxTimeout);

      if (xlen==USBLIB_TIMEOUT) {
         // USB timeout it is...
         _V_ ::fprintf(stderr, "UxUsb_LibUsb::IntRead - USB timeout %lu ms\n", m_rxTimeout);
         return USB_TIMEOUT; // TIMEOUT EXIT - No data available
      }
      else if(xlen<0) {
         // int read failed
         ::fprintf(stderr, "UxUsb_LibUsb::IntRead - USB Error %d %s\n", errno, usb_strerror());
         return USB_USB_INT_RW_ERR;  // ERROR EXIT
      }
      else if (xlen>0) {
         ::memcpy(&b[rest], buf, xlen); // get byte to receiver
         rest -= xlen;                  // adjust counter
      }
      else  {
         ; // xlen returns 0 bytes read ??!
      }
   }

   return err; // OK
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Bulk Write to EPn len bytes from buffer b
int UxUsb_LibUsb::IntWrite(unsigned short ep, unsigned long len, unsigned char* b)
{
   int err = USB_OK;
   int ept = m_Endpoint[ep].epNumber;
   unsigned long rest = len;

   if (!m_devOK) {
      ::fprintf(stderr, "UxUsb_LibUsb::IntWrite - USB device not available\n");
      return USB_USB_DEV_NA; //ERROR EXIT
   }

   if (!m_Endpoint[ep].directionOUT) ept=-1; // must be IN

   if(ept<=0){
      ::fprintf(stderr, "UxUsb_LibUsb::IntWrite - invalid endpoint given %d\n", ep);
      return USB_USB_ENDPOINT_NA; //ERROR EXIT
   }

   if(m_Endpoint[ep].epType!=INTERRUPT){
      ::fprintf(stderr, "UxUsb_LibUsb::IntWrite - invalid endpoint type given %d\n", ep);
      return USB_USB_WRONG_ENDPOINT; //ERROR EXIT
   }

   const unsigned long BuffSize = m_Endpoint[ep].scratchSize;
   unsigned char* buf = m_Endpoint[ep].pScratchArea;
   int xlen, blen;
   int nullWrite = MAX_NULLWRITE;

   while ( (rest>0) && (err == USB_OK) ) {
      // still something to write ?
      blen = (rest>BuffSize) ? BuffSize : rest; // determine amount to write
      ::memcpy(buf, &b[len-rest], blen);

      while (nullWrite>0) {

         xlen = usb_interrupt_write(m_DevHandle, ept, (char*)buf, blen, m_txTimeout);  // write to USB

         if (xlen==USBLIB_TIMEOUT) {
            // USB timeout it is...
            ::fprintf(stderr, "UxUsb_LibUsb::IntWrite - USB timeout %lu ms; trying again %d\n", m_txTimeout, nullWrite);
            err = USB_TIMEOUT;
            nullWrite--;
         }
         else if(xlen<0){
            ::fprintf(stderr, "UxUsb_LibUsb::IntWrite - USB Error %d %s\n", errno, usb_strerror());
            return USB_USB_INT_RW_ERR; // ERROR EXIT
         }
         else if (xlen>0) {
            err = USB_OK;  // reset err status on any wrtite
            rest -= xlen;      // adjust counters and pointers
            break;             // from while nullwrite with OK
         }
         else {
            ; // xlen returns 0 bytes written ??!
         }
      }//while trys
      // here we have an ERR status
   }

   return err; // OK or TIMEOUT
}


///////////////////////////////////////////////////////////////////////////////
///
int UxUsb_LibUsb::DeviceInfo()
{
  int err = USB_OK;
  //String s;

  if(!m_devOK){
    err = USB_USB_DEV_NA;
    ::fprintf(stderr, "UxUsb_LibUsb::DeviceInfo - OPEN First!\n");
  }
  else{
	::fprintf(stderr, "UxUsb_LibUsb::DeviceInfo -  ConfigCount: %d\n", m_USBDevice->descriptor.bNumConfigurations );
    for(int c=0; c<m_USBDevice->descriptor.bNumConfigurations; c++){
      struct usb_config_descriptor* cfg = &(m_USBDevice->config[c]);

      // \todo Write to the Status report, not directly print.
      ::fprintf(stderr, " bLength: 0x%x\n",cfg->bLength);
      ::fprintf(stderr, " bDescriptorType: %d\n",cfg->bDescriptorType);
      ::fprintf(stderr, " wTotalLength: %d (0x%x)\n",cfg->wTotalLength,cfg->wTotalLength);
      ::fprintf(stderr, " bNumInterfaces: %d\n",cfg->bNumInterfaces);
      ::fprintf(stderr, " bConfigurationValue: %d\n",cfg->bConfigurationValue);
      ::fprintf(stderr, " iConfiguration: %d\n",cfg->iConfiguration);
      ::fprintf(stderr, " bmAttributes: 0x%x\n",cfg->bmAttributes);
      ::fprintf(stderr, " MaxPower: %d\n",cfg->MaxPower);
      ::fprintf(stderr, " **********************************\n");

      for (int i=0; i<cfg->interface->num_altsetting; i++) {
        struct usb_interface_descriptor* ifc = &(cfg->interface->altsetting[i]);
        //::fprintf(stderr, "Interface Descriptor:" + String(i+1));
        ::fprintf(stderr, "  --------------------------------\n");
        ::fprintf(stderr, "  bLength: 0x%x\n",ifc->bLength);
        ::fprintf(stderr, "  bDescriptorType: %d\n",ifc->bDescriptorType);
        ::fprintf(stderr, "  bInterfaceNumber: %d\n",ifc->bInterfaceNumber);
        ::fprintf(stderr, "  bAlternateSetting: %d\n",ifc->bAlternateSetting);
        ::fprintf(stderr, "  bNumEndpoints: %d\n",ifc->bNumEndpoints);
        ::fprintf(stderr, "  bInterfaceClass: %d\n",ifc->bInterfaceClass);

        for (int e=0; e<ifc->bNumEndpoints; e++) {
          struct usb_endpoint_descriptor* ept = &(ifc->endpoint[e]);
          //::fprintf(stderr, "EndPoint Descriptor: " + String(e+1));
          ::fprintf(stderr, "  --------------------------------\n");
          ::fprintf(stderr, "  bLength: 0x%x\n",ept->bLength);
          ::fprintf(stderr, "  bDescriptorType: %d\n",ept->bDescriptorType);
          ::fprintf(stderr, "  bEndpointAddress: 0x%x\n",ept->bEndpointAddress);
          ::fprintf(stderr, "  bmAttributes: 0x%x\n",ept->bmAttributes);
          ::fprintf(stderr, "  wMaxPacketSize: %d\n",ept->wMaxPacketSize);
          ::fprintf(stderr, "  bInterval: %d\n",ept->bInterval);
          ::fprintf(stderr, "  **********************************\n");
        }
      }
    }
  }

  return err;
}



///////////////////////////////////////////////////////////////////////////////
/// @brief Handle the struct EndPointDescT

UxUsb_LibUsb::EndPointDescT::EndPointDescT()
: pScratchArea(NULL)
, pInBuffer(NULL)
, pOutBuffer(NULL)
{
   Init();
}

void UxUsb_LibUsb::EndPointDescT::Init()
{
   epNumber=-1; epSize=0; epType=UNDEF;
   if (pScratchArea) delete pScratchArea; pScratchArea=NULL; scratchSize=0;
   if (pInBuffer) delete pInBuffer; pInBuffer = NULL;
   if (pOutBuffer) delete pOutBuffer; pOutBuffer = NULL;
   directionIN=false; directionOUT=false;
}
