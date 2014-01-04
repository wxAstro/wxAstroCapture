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

#include "GenericImageMem.h"

#include <memory.h>

/// //////////////////////////////////////////////////////////////////////
/// GenericImageMem
///

GenericImageMemBase::GenericImageMemBase()
: m_x(0), m_y(0), m_width(0), m_height(0)
, m_memOK(false)
, m_byteSize(0)
, m_pByteMem(NULL)
{
}

GenericImageMemBase::~GenericImageMemBase()
{
   ClearMem();
}

GenericImageMemBase::GenericImageMemBase(size_t nBytes)
: m_memOK(false)
, m_byteSize(0)
, m_pByteMem(NULL)
{
   CreateMem(nBytes);
}

// Free the memory
void GenericImageMemBase::ClearMem()
{
#ifndef INDI
   // NOTE we use malloc/free as this is used when transfering mem to wxImage :-(
   if (m_pByteMem) ::free(m_pByteMem);
#else
   if (m_pByteMem) delete m_pByteMem;
#endif
   m_pByteMem = NULL; m_byteSize=0; m_memOK=false;

}

// Allocates the memory
void GenericImageMemBase::CreateMem(size_t nBytes)
{
   if (nBytes> m_byteSize) {
      ClearMem();
#ifndef INDI
   // NOTE we use malloc/free as this is used when transfering mem to wxImage :-(
      m_pByteMem= reinterpret_cast<unsigned char*>(::malloc(nBytes));
#else
      m_pByteMem= new unsigned char[nBytes];
#endif
   }

   if (m_pByteMem) {
      m_byteSize=nBytes;
      m_memOK=true;
   }
}

void GenericImageMemBase::CopyFrom(const unsigned char* bytePtr, size_t nBytes, ECopyMode cmode)
{
   CreateMem(nBytes);

   if (m_memOK) {
      switch (cmode) {
         case EYM_Reverse: {
            const unsigned char* sPtr=bytePtr+nBytes;
            unsigned char* dPtr=m_pByteMem;
            for (size_t i=0; i<nBytes; i++) {*dPtr=*(--sPtr); ++dPtr;};
         } break;

         case EYM_Forward:
         default: {
            ::memcpy(m_pByteMem, bytePtr, nBytes);

         }
      }//switch
   }
}

void GenericImageMemBase::SetFrameRect(int x, int y, int width, int height)
{
   m_x=x; m_y=y;
   m_width=width; m_height=height;
}

void GenericImageMemBase::SetFrameRect(const GenericImageMemBase* imageMem)
{
   if (imageMem) {
      imageMem->GetFrameRect(m_x, m_y, m_width, m_height);
   }
}

void GenericImageMemBase::GetFrameRect(int& x, int& y, int& width, int& height) const
{
   x=m_x; y=m_y;
   width=m_width; height=m_height;
}

void GenericImageMemBase::GetFrameRect(unsigned short& x, unsigned short& y, unsigned short& width, unsigned short& height) const
{
   x=(unsigned short)(m_x); y=(unsigned short)(m_y);
   width=(unsigned short)(m_width); height=(unsigned short)(m_height);
}


#ifndef INDI
// provides a WX compliant interface
void GenericImageMemBase::SetFrameRect(const wxRect frameRect)
{
   m_x=frameRect.x; m_y=frameRect.y;
   m_width=frameRect.width; m_height=frameRect.height;
}

wxRect GenericImageMemBase::FrameRect() const
{
   return wxRect(m_x, m_y, m_width, m_height);
}
#endif

// Detach the mem pointer - freeing mem is up to the caller now !!
// used to transfer memory to wxImage
unsigned char* GenericImageMemBase::DetatchMemPtr()
{
   if (m_pByteMem) {
      unsigned char* tmp = m_pByteMem;
      m_pByteMem = NULL;
      m_byteSize = 0; m_memOK=false;
      return tmp;
   }
   else {
      return NULL;
   }
}

/// //////////////////////////////////////////////////////////////////////
/// WordImageMem
///
WordImageMem::WordImageMem(size_t nWords)
: GenericImageMemBase(nWords*2)
{
}

void WordImageMem::CopyFrom(const unsigned short* wordPtr, size_t nWords, ECopyMode cmode)
{
   CreateMem(nWords*sizeof(unsigned short));

   if (MemOK()) {
      switch (cmode) {
         case EYM_Reverse: {
            const unsigned short* sPtr=wordPtr+nWords;
            unsigned short* dPtr=WordMemPtrRef();
            for (size_t i=0; i<nWords; i++) {*dPtr=*(--sPtr); ++dPtr;};
         } break;

         case EYM_Forward:
         default: {
            ::memcpy(WordMemPtrRef(), wordPtr, nWords*sizeof(unsigned short));

         }
      }//switch
   }
}

// Detach the mem pointer - freeing mem is up to the caller now !!
// used to transfer memory to wxImage
unsigned short* WordImageMem::DetatchMemPtr()
{
   return reinterpret_cast<unsigned short*>(GenericImageMemBase::DetatchMemPtr());
}

