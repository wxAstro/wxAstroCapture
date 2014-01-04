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
 * @file           IOBuffer.h
 *****************************************************************************
 * class IOBuffer
 *
 * A supporting buffer class
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
 * @date           19-Sep-2009
 *
 *****************************************************************************
 *<hr>
 * @b Updates
 * - dd-mmm-yyyy V. Name: Description
 *
 *****************************************************************************/

#ifndef _IOBUFFER_H_
#define _IOBUFFER_H_

#include <stddef.h>

// helper class for internal buffering
class IOBuffer
{
public:

   IOBuffer(size_t size);
   virtual ~IOBuffer();

   //!@brief Returns the size of the buffer
   //!@return the size in bytes
   size_t Size();

   //!@brief Returns the current filling level
   //!@return the current fill level in bytes
   size_t Level();

   //!@brief Returns what is still availble (Size-Level)
   //!@return the free bytes that are available
   size_t Available();

   //!@brief Retrieve bytes from the buffer from the beginning of the buffer
   //!@param dst: the get target
   //!@param amount: the number of bytes to get
   //!@return the effective returned bytes
   size_t GetBytes(unsigned char *dst, size_t amount);

   //!@brief Fill bytes into the buffer appending them at the end
   //!@param src: the Put source
   //!@param amount: the number of bytes to put
   //!@return the effective buffered bytes
   size_t PutBytes(const unsigned char *src, size_t amount);

private:
   IOBuffer();
   IOBuffer(IOBuffer&);
   void Init();

   unsigned char     *m_buffer;     // holds the buffer
   unsigned char     *m_pBuffer;    // ptr within buffer
   size_t             m_size;
   size_t             m_level;      // bytes in buffer
   size_t             m_available;  // bytes left to buffer
};

#endif // _IOBUFFER_H_
