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


#include "Ftd245IOUX.h"

/// There is a derived version used for the INDI framework
///  which has some files in another place than wxAstroCapture
#ifdef INDI
   #include "GenericDebHelp.h"
#else
   #include "../GenericDebHelp.h"
#endif

#include "ARTUSB_LibUsb.h"

#include <fcntl.h>    // open(), fcntl() and their constants
#include <termios.h>  // termios and its constants
#include <unistd.h>   // write(), read(), getpid(), close()
#include <signal.h>   // sigaction and related functions and constants
#include <string.h>   // bzero()
#include <stdio.h>    // This is assumed, but best to include it explicitly.
#include <errno.h>    // errno
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>



///////////////////////////////////////////////////////////////////////////////
int Ftd245IOUX::xioctl(int device, int request, void* arg)
{
   // repeat the call to ioctl as long as the previous call was interrupted prematurely
   int r = 0;
   int tryx=10;
   do {
     r = ::ioctl(device,request,arg); tryx--;
   } while ( -1 == r && EINTR == errno && tryx );

   return r;
}

///////////////////////////////////////////////////////////////////////////////
// I can only count on getting at least one signal when a read is ready.
// Subsequent signals might be hidden when the handler is running
static int x_signals = 0;
static void signal_handler_IO(int status) {
  x_signals++;
}



///////////////////////////////////////////////////////////////////////////////
Ftd245IOUX::Ftd245IOUX()
: Ftd245IO()
, m_device(-1)
{
}

Ftd245IOUX::~Ftd245IOUX()
{
   ShutConnection();
}

///////////////////////////////////////////////////////////////////////////////
void Ftd245IOUX::ShutConnection()
{
   _V_ ::fprintf(stderr, "Ftd245IOUX::ShutConnection() - device closed.\n");
   if (m_device!=-1) ::close(m_device); m_device=-1;
   Ftd245IO::ShutConnection(); //       m_board_present = false;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief portable sleep command; take milliseconds as argument
void Ftd245IOUX::Sleep_ms(unsigned short  mseconds )
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
//! @brief  Check if the USB board is available (USB device is connected)
bool Ftd245IOUX::DeviceAvailable(void)
{
  return (m_device!=-1);
}

//////////////////////////////////////////////////////////////////////
//! @brief  Initialize and test of connection with the USB board
//! @param  int deviceIndex: relates to interface slots 0...9
//! @return bool: returns true if an FTDI board was found with the given deviceIndex
bool Ftd245IOUX::InitBoard(int deviceIndex)
{
   struct termios params;
   struct sigaction saio;

   // Setting up signal handler used by read() when doing asynchronous I/O
   sigset_t mask;
   saio.sa_handler = ::signal_handler_IO;
   ::sigemptyset(&mask);
   ::sigaddset(&mask, SIGIO);
   saio.sa_mask = mask;
   saio.sa_flags = SA_NODEFER;
   saio.sa_restorer = NULL;
   ::sigaction(SIGIO, &saio, NULL);

   _V_ ::fprintf(stderr, "Ftd245IOUX::InitBoard(%d)\n", deviceIndex);

   char devicename[50];
   bool retVal;

   if (m_device!=-1) ::close(m_device); m_device=-1;

   m_deviceIndex = -1;
   m_board_present = false;

   // retrieve the camera path
   retVal = ARTUSB_LibUsb::GetDevicePath(static_cast<ARTUSB_LibUsb::ARTUSB_LibUsb_Device>(deviceIndex), devicename);
   if ( !retVal ) {
      ::fprintf(stderr, "Ftd245IOUX::InitBoard() - device %s does not exist\n", devicename);
      return false; // ERROR EXIT - device does not exist
   }

   _V_ ::fprintf(stderr, "Ftd245IOUX::InitBoard() - device %s\n", devicename);

   m_device = ::open(devicename, O_RDWR | O_NOCTTY); // If O_ASYNC is set here, hangs read for lack of ownership
   if (m_device >= 0) {
      _V_ ::fprintf(stderr, "Ftd245IOUX::InitBoard() - device %s is open\n", devicename);

      // At this point only O_RDWR is set and the owner process pid = 0
      ::fcntl(m_device, F_SETOWN, ::getpid());  // Making this process own the file descriptor allows O_ASYNC to be set!!!

      ///NOTE signal handling is not enabled when the line below is commented out
      //      seems not required to get reads done with VMIN = 0 below
      //::fcntl(m_device, F_SETFL, O_ASYNC );   // Use signals - See man 2 open for explanation
      if (::tcgetattr(m_device, &params) < 0) {
         ::fprintf(stderr, "Ftd245IOUX::InitBoard() - %s - tcgetattr() failed, device closed!\n\n", strerror(errno));
         ::close(m_device); m_device = -1;
         return false; // ERROR EXIT - syscall failed
      }
      // the FTDI245BM is a USB-parallel-FIFO device
      // Transfers run as fast as the host or the module allows.
      ::bzero(&params, sizeof(params));
      params.c_iflag = IGNPAR;   // ignor parity errors
      params.c_oflag = 0;        // output handling is completely disabled
//      params.c_cflag = B460800 | CS8 | CLOCAL | CREAD; // seems we cannot affect baudrate (FT245 is a parallel device)
      params.c_cflag = CS8 | CLOCAL | CREAD; //  baudrate is set - will always report at max speed anyhow
      cfsetospeed(&params, B3000000);
      cfsetispeed(&params, B3000000);
        /* set input mode (non-canonical, no echo,...) */
      params.c_lflag = 0;        // local processing is completely disabled

      // blocking behaviour - we read with timeout not with #chars
      params.c_cc[VTIME] = 1; // wait 0.1s for an answer (will be changed by calling Timeout interface
      params.c_cc[VMIN] = 0;  // don't wait for any data

      ::tcflush(m_device, TCIFLUSH); // flush input buffers
      if (::tcsetattr(m_device, TCSANOW, &params) < 0)
      {
         ::fprintf(stderr, "Ftd245IOUX::InitBoard() - %s - tcsetattr() failed!\n", strerror(errno));
         return false; // ERROR EXIT - syscall failed
      }
      ::tcflush(m_device, TCIOFLUSH); // flush input and output buffers
      m_deviceIndex = deviceIndex;
      m_board_present = true;
   }
   else {
      ::fprintf(stderr, "Ftd245IOUX::InitBoard() - %s - cannot open device %s!\n", strerror(errno), devicename);
      ShutConnection();
   }

   return m_board_present;
}


//////////////////////////////////////////////////////////////////////
//! @brief  Read from the device
//! @param  void* lpvBuffer: the read buffer
//! @param  size_t dwBuffSize: the size of the read buffer
//! @param  size_t& lpdwBytesRead: variable to return the bytes read
//! @return State information (Ftd245IOState)
Ftd245IO::Ftd245IOState Ftd245IOUX::Read(void* lpvBuffer, size_t dwBuffSize, size_t& lpdwBytesRead)
{
   lpdwBytesRead = 0;
   if ( !m_board_present || m_device==0 ) return FTS_INVALID_HANDLE;

   int len = ::read(m_device, lpvBuffer, dwBuffSize);
//::fprintf(stderr, "Ftd245IOUX::Read() - ::read() returned %d\n", len);

   if (len<0) {
      // IO error
      ::fprintf(stderr, "Ftd245IOUX::Read() - %s\n", strerror(errno));
      ::fprintf(stderr, "... ERROR:  read() returned %d - device closed!\n", len);
      m_board_present = false; // not longer
      ::close(m_device); m_device = 0;
   }
   lpdwBytesRead = (len>=0)?len : 0;

   return (len>=0)?FTS_OK:FTS_IO_ERROR;
}

//////////////////////////////////////////////////////////////////////
//! @brief  Empty the read channel
void Ftd245IOUX::ClearInputChannel()
{
   ::tcflush(m_device, TCIFLUSH); // flush input buffers
}



//////////////////////////////////////////////////////////////////////
//! @brief  Write to the device
//! @param  void* lpvBuffer: the write buffer
//! @param  size_t dwBuffSize: the size of the write buffer
//! @param  size_t& lpdwBytes: variable to return the bytes written
//! @return State information (Ftd245IOState)
Ftd245IO::Ftd245IOState Ftd245IOUX::Write(void* lpvBuffer, size_t dwBuffSize, size_t& lpdwBytes)
{
   lpdwBytes = 0;
   if ( !m_board_present || m_device==0 ) return FTS_INVALID_HANDLE;

   int len = ::write(m_device, lpvBuffer, dwBuffSize);
//::fprintf(stderr, "Ftd245IOUX::Write() - ::write() returned %d\n", len);

   if (len<0) {
      // IO error
      ::fprintf(stderr, "Ftd245IOUX::Write() - %s\n", strerror(errno));
      ::fprintf(stderr, "... ERROR:  write() returned %d - device closed!\n", len);
      m_board_present = false; // not longer
      ::close(m_device); m_device = 0;
   }
   lpdwBytes = (len>=0)?len : 0;

   return (len>=0)?FTS_OK:FTS_IO_ERROR;
}


//////////////////////////////////////////////////////////////////////
//! @brief  Set the board timeouts
//! @param  unsigned long dwReadTimeout: read timeout in milliseconds
//! @param  unsigned long dwWriteTimeout: write timeout in milliseconds
//! @return State information (Ftd245IOState)
Ftd245IO::Ftd245IOState Ftd245IOUX::SetTimeouts(unsigned long dwReadTimeout, unsigned long dwWriteTimeout)
{
   if (!m_board_present) return FTS_INVALID_HANDLE;

   struct termios params;
   if (::tcgetattr(m_device, &params) < 0) {
      return FTS_IO_ERROR;
   }
   params.c_cc[VTIME] = dwReadTimeout / 100; // wait for an answer

   ::tcflush(m_device, TCIFLUSH); // flush input buffers
   if (::tcsetattr(m_device, TCSANOW, &params) < 0) {
      ::fprintf(stderr, "Ftd245IOUX::SetTimeouts() - %s\n", strerror(errno));
      ::fprintf(stderr, "... ERROR:  tcsetattr() failed!\n");
      return FTS_IO_ERROR;
   }
   return FTS_OK;
}

//////////////////////////////////////////////////////////////////////
//! @brief  Get the board timeouts
//! @param  unsigned long& dwReadTimeout: returns read timeout in milliseconds
//! @param  unsigned long& dwWriteTimeout: returns write timeout in milliseconds
void Ftd245IOUX::GetTimeouts(unsigned long& dwReadTimeout, unsigned long& dwWriteTimeout)
{
   if (!m_board_present) return;

   struct termios params;
   if (::tcgetattr(m_device, &params) < 0) {
      return;
   }
   dwReadTimeout = params.c_cc[VTIME] * 100; // wait for an answer
   dwWriteTimeout = params.c_cc[VTIME] * 100; // wait for an answer
}

//////////////////////////////////////////////////////////////////////
//! @brief  Get the size of the read queue
//! @param  size_t& lpdwAmountInRxQueue: variable to return the bytes in read queue
//! @return State information (Ftd245IOState)
Ftd245IO::Ftd245IOState Ftd245IOUX::GetQueueStatus(size_t& lpdwAmountInRxQueue)
{
   int bufcnt;
   xioctl(m_device, FIONREAD, &bufcnt);
   lpdwAmountInRxQueue = bufcnt;
   return FTS_OK;
}

//////////////////////////////////////////////////////////////////////
//! @brief  Set the transfer parameter
//! @param  size_t dwInTransferSize: read transfer size of the usb interface
//! @return State information (Ftd245IOState)
Ftd245IO::Ftd245IOState Ftd245IOUX::SetUSBParameters (size_t dwInTransferSize)
{
   return FTS_OK; // cannot in Linux
}

