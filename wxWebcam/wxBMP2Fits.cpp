

#include "wxBMP2Fits.h"

#include "wxCameraTypes.h"  // BITMAPINFOHEADER for Linux


/// ////////////////////////////////////////////////////////////////////
/// IMPLEMENT wxBMP2Fits

// takes one BMP without palette i.e. data follows header immediately
bool wxBMP2Fits::AddBMP(const void* bmp)
{
   BITMAPINFOHEADER* pBmih = (BITMAPINFOHEADER*)bmp;
   void*    pbData = (void*)(pBmih+1);

   return AddBMP((void*)pBmih, pbData);
}

// takes Info Header and BMP data
bool wxBMP2Fits::AddBMP(const void* bmpHeader, const void* bmpData)
{

   BITMAPINFOHEADER* pBmih = (BITMAPINFOHEADER*)bmpHeader;
   wxUint8*    pbData = const_cast<wxUint8*>( reinterpret_cast<const wxUint8*>(bmpData));

   size_t nAxis  = (pBmih->biBitCount==24)?3:2; // YUV or RGB ?
   size_t width  = pBmih->biWidth;
   size_t height = abs(pBmih->biHeight); // bottumup BMP deliver negative heights

   // start FITS generation
   wxFitsOut::Create(wxSize(width,height), nAxis);

   wxFitsKeyword k;
   wxString txt(wxT("Fits image created by wxAstroCapture - wxBMP2Fits"));
   k.SetKeyword(wxFitsKeyword::EF_HISTORY, txt);
   KeywordListRef() += k;

   wxUint8 stride[FITS_MAX_WIDTH];  // gets one scanline
   wxUint8* sPtr;
   wxUint8* dPtr;

   if ( pBmih->biBitCount==8 || pBmih->biBitCount==12 ) {
      // MONO or YUV plane
      // dump one plane (upside down) only Y for YUV
      size_t scanwidth  = ((width + 3) & ~3); // DWORD align (BMP stride)
      for (long y=long(height)-1; y>=0; y--) {
         sPtr = pbData + y*scanwidth; // source start
         dPtr = stride;
         for (size_t x=0; x<width; x++) {
            *dPtr++ = *sPtr++;
         }
         wxFitsOut::AddImageStride(stride);
      }
   }
   else if ( pBmih->biBitCount==16 ) {
      // YUV 4:2:2
      // dump Y values, depends on compression
      unsigned scanwidth  = ((width*2 + 3) & ~3); // DWORD align (BMP stride)
      switch (pBmih->biCompression) {
         case wxAC_FOURCC_YUY2:  // image is Y0-U0-Y1-V0 ...
         case wxAC_FOURCC_YUYV:  // image is Y0-U0-Y1-V0 ...
         case wxAC_FOURCC_YVYU: {// image is Y0-V0-Y1-U0 ...
            for (long y=long(height)-1; y>=0; y--) {
               sPtr = pbData + y*scanwidth; // source start
               dPtr = stride;
               for (size_t x=0; x<width; x++) {
                  *dPtr++ = *sPtr++; sPtr++;  // skip U or V
               }
               wxFitsOut::AddImageStride(stride);
            }
            break;
         }
         case wxAC_FOURCC_UYVY: {// image is U0-Y0-V0-Y1 ...
            for (long y=long(height)-1; y>=0; y--) {
               sPtr = pbData + y*scanwidth + 1; // source start / skip leading U0
               dPtr = stride;
               for (size_t x=0; x<width; x++) {
                  *dPtr++ = *sPtr++; sPtr++;  // skip U or V
               }
               wxFitsOut::AddImageStride(stride);
            }
            break;
         }
         default:
            ;
      }//switch
   }
   else if (pBmih->biBitCount==24) {
      // BGR COLOR plane
      size_t scanwidth  = ((width*3 + 3) & ~3); // DWORD align (BMP stride)
      // dump three planes RGB order /BMP is BGR
//      for (size_t y=0; y<height; y++) {
      //BM:20090621 - copy upsidedown as above !!
      for (long y=long(height)-1; y>=0; y--) {
         sPtr = pbData + y*scanwidth +2; // source start R
         dPtr = stride;
         for (size_t x=0; x<width; x++) {
            *dPtr++ = *sPtr; sPtr+=3;
         }
         wxFitsOut::AddImageStride(stride);
      }

      for (size_t y=0; y<height; y++) {
         sPtr = pbData + y*scanwidth + 1; // source start G
         dPtr = stride;
         for (size_t x=0; x<width; x++) {
            *dPtr++ = *sPtr; sPtr+=3;
         }
         wxFitsOut::AddImageStride(stride);
      }

      for (size_t y=0; y<height; y++) {
         sPtr = pbData + y*scanwidth + 0; // source start B
         dPtr = stride;
         for (size_t x=0; x<width; x++) {
            *dPtr++ = *sPtr; sPtr+=3;
         }
         wxFitsOut::AddImageStride(stride);
      }
   }
   else {
      return false;
   }
   return true;
}



