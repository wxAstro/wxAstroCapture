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
 * @file           IOBuffer.cpp
 *****************************************************************************
 * class IOBuffer
 *
 * Astronomy USB Appliance driver for Linux using libUsb
 *
 * A supporting buffer class
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
 * @date           19-Sep-2009
 *
 *****************************************************************************
 *<hr>
 * @b Updates
 * - dd-mmm-yyyy V. Name: Description
 *
 *****************************************************************************/

#include "IOBuffer.h"

#include <memory.h>
#include <string.h>  // memmove


// local shortcuts
typedef unsigned char  _byte;
typedef unsigned short _word;
typedef unsigned long  _dword;

//!/////////////////////////////////////////////////////////////////////////
// Helper Class for buffer management

IOBuffer::IOBuffer(size_t size)
: m_buffer(new unsigned char[size])
, m_pBuffer(NULL)
, m_size(size)
, m_level(0)
, m_available(0)
{
   Init();
}

IOBuffer::~IOBuffer()
{
   if (m_buffer) delete m_buffer;
}

void IOBuffer::Init()
{
   m_level = 0;
   m_available = m_size;
}

// level and remaining is maintained when accessing the buffer -> always valid

//!@brief Returns the size of the buffer
size_t IOBuffer::Size() {return m_size;};

//!@brief Returns the current filling level
size_t IOBuffer::Level() {return m_level;};

//!@brief Returns what is still availble (Size-Level)
size_t IOBuffer::Available() {return m_available;};

//!@brief Retrieve bytes from the buffer from the beginning of the buffer
size_t IOBuffer::GetBytes(_byte *dst, size_t amount)
{
   if (amount==0) return 0;  // paranoid..
   if (m_level==0) return 0; // this is possible however

   size_t lenRead = (amount>m_level) ? m_level: amount;
   ::memcpy(dst, m_buffer, lenRead);

   m_level-=lenRead;  // we have less now...
   if (m_level>0) {
      // as we don't maintain a circular buffer we have to shift the bytes once they are read
      m_pBuffer=m_buffer;  // at start again
      m_pBuffer+=lenRead;     // at not returned start
      ::memmove(m_buffer, m_pBuffer, m_level); // move at beginning
   }
   m_available+=lenRead;  // adjust
   return lenRead;
}

//!@brief Fill bytes into the buffer appending them at the end
size_t IOBuffer::PutBytes(const _byte *src, size_t amount)
{
   if (amount==0) return 0;
   if (m_available==0) return 0;

   size_t lenWrite = (amount>m_available) ? m_available: amount;
   m_pBuffer=m_buffer; // at start again
   m_pBuffer+=m_level; // at insertpt
   ::memcpy(m_pBuffer, src, lenWrite);
   m_level+=lenWrite; m_available-=lenWrite;  // adjust

   return lenWrite;
}

