

#include "wxFitsOutputStream.h"

#include "wxFitsOut.h"


wxFitsOutputStream::wxFitsOutputStream(wxOutputStream& s)
: m_output(s)
{
}

wxOutputStream&  wxFitsOutputStream::Write(const wxFitsOut& fits)
{
   const wxUint8* sPtr = const_cast<wxUint8*>( reinterpret_cast<const wxUint8*>( fits.GetFitsPointer() ) );
   size_t      sLen = fits.GetFitsSize();
   size_t      sWrite = 1;
   while (sLen>0 && sWrite>0) {
      m_output.Write((void*)sPtr, sLen);
      sWrite = m_output.LastWrite();
      sLen -= sWrite; sPtr+=sWrite;
   }//while
   return *this;
}
