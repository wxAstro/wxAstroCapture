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
 * @file           GenericImageMem.h
 *****************************************************************************
 * class GenericImageMemBase
 * class ByteImageMem
 * class WordImageMem
 *
 * Astronomy USB Appliance driver for Linux and Win32
 * A frame helper class
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
 * @date           01-Feb-2009
 *
 *****************************************************************************
 *<hr>
 * @b Updates
 * - dd-mmm-yyyy V. Name: Description
 *
 *****************************************************************************/

#ifndef _GENERICIMAGEMEM_H_
#define _GENERICIMAGEMEM_H_


#ifndef INDI  // using wxWidgets
   // For compilers that support precompilation, includes "wx/wx.h".
   #include "wx/wxprec.h"

   #ifndef WX_PRECOMP
   #include "wx/wx.h"
   #endif
#endif

#include <stddef.h>

class GenericImageMemBase
{
public:
   enum ECopyMode {
      EYM_Forward=0, // copies from byte at loc[0] to loc[0]
      EYM_Reverse,   // copies from byte at loc[n] to loc[0]
   };

public:
   GenericImageMemBase();
   virtual ~GenericImageMemBase();

public:

   bool   MemOK() const {return m_memOK;};
   size_t ByteSize() const {return m_byteSize;};
   const  unsigned char* ByteMemPtr() const {return m_pByteMem;};
   unsigned char* ByteMemPtrRef() const {return m_pByteMem;};

   // Detach the mem pointer - freeing mem is up to the caller now !!
   // used to transfer memory to wxImage
   unsigned char* DetatchMemPtr();

   // copy n bytes from bytePtr, allocates mem if needed
   void CopyFrom(const unsigned char* bytePtr, size_t nBytes, ECopyMode cmode=EYM_Forward);


   // rect is pos=unbinned coords, size=real size (binned pixels)
   void SetFrameRect(int x, int y, int width, int height);
   void SetFrameRect(const GenericImageMemBase* imageMem);
   void GetFrameRect(int& x, int& y, int& width, int& height) const;
   void GetFrameRect(unsigned short& x, unsigned short& y, unsigned short& width, unsigned short& height) const;
   int x() const {return m_x;};
   int y() const {return m_y;};
   int width() const {return m_width;};
   int height() const {return m_height;};
#ifndef INDI  // using wxWidgets
   // provides a WX compliant interface
   void SetFrameRect(const wxRect frameRect);
   wxRect FrameRect() const;
#endif

protected:
   GenericImageMemBase(size_t nBytes);

   // NOTE we use malloc/free as this is used when transfering mem to wxImage :-(
   // Allocates the memory
   void CreateMem(size_t nBytes);
   // Free the memory
   void ClearMem();

private:
   int m_x, m_y, m_width, m_height; // transports the image properties
//   wxRect   m_frameRect;
   bool           m_memOK;
   size_t         m_byteSize;
   unsigned char* m_pByteMem;
};


class ByteImageMem : public GenericImageMemBase
{
public:
   ByteImageMem() : GenericImageMemBase(){};
   ByteImageMem(size_t nBytes) :GenericImageMemBase(nBytes) {};

};



class WordImageMem : public GenericImageMemBase
{
public:
   WordImageMem() : GenericImageMemBase(){};
   WordImageMem(size_t nWords);

public:
   size_t WordSize() const {return ByteSize() / sizeof(unsigned short);};
   const unsigned short* WordMemPtr() const {return reinterpret_cast<const unsigned short*>(ByteMemPtr());};
   unsigned short* WordMemPtrRef() const {return reinterpret_cast<unsigned short*>(ByteMemPtrRef());};

   // Detach the mem pointer - freeing mem is up to the caller now !!
   // used to transfer memory to wxImage
   unsigned short* DetatchMemPtr();

   // copy n words from wordPtr, allocates mem if needed
   void CopyFrom(const unsigned short* wordPtr, size_t nWords, ECopyMode cmode=EYM_Forward);
};


#endif // _GENERICIMAGEMEM_H_
