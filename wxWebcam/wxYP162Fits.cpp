

#include "wxYP162Fits.h"

#include "wxCameraTypes.h"  // BITMAPINFOHEADER for Linux


/// ////////////////////////////////////////////////////////////////////
/// IMPLEMENT wxBMP2Fits

// takes Info Header and BMP data
bool wxYP162Fits::AddYP16(const wxSize& frameSize, const void* yData)
{

   wxUint16* pbData = const_cast<wxUint16*>(reinterpret_cast<const wxUint16*>(yData));

   size_t nAxis  = 2;

   // start FITS generation
   wxFitsOut::Create(frameSize, nAxis);

   wxFitsKeyword k;
   wxString txt(wxT("Fits image created by wxAstroCapture - wxYP162Fits"));
   k.SetKeyword(wxFitsKeyword::EF_HISTORY, txt);
   KeywordListRef() += k;

   size_t width  = frameSize.GetWidth();
   size_t height  = frameSize.GetHeight();
   wxUint16 stride[FITS_MAX_WIDTH];  // gets one scanline
   wxUint16* sPtr;
   wxUint16* dPtr;

   // dump one plane (upside down)
   for (long y=height-1; y>=0; y--) {
      sPtr = pbData + y*width; // source start
      dPtr = stride;
      for (size_t x=0; x<width; x++) {
         *dPtr++ = *sPtr++;
      }
      wxFitsOut::AddImageStride(stride);
   }
   return true;
}



