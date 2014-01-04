

#include "wxTimeFormatter.h"

#include <wx/datetime.h>


//! @brief Render a timestamp into an 8bit BMP either RGB24 or YUV is supported
void wxTimeFormatter::DrawTimeStamp(const PBITMAPINFOHEADER pBmpHeader, const wxUint8* pBmpData,
                      const wxDateTime tStamp, wxTimeFont::EFontSize fontSize)
{
   wxTimeFont TF(fontSize);  // create a specific font here

   unsigned short msec = tStamp.GetMillisecond(wxDateTime::UTC);
   wxString tstring = wxString::Format(wxT("%s.%03hu"), tStamp.Format(wxT("%Y-%m-%d %H:%M:%S"), wxDateTime::UTC).c_str(), msec);

   // allocate bitmap memory type for the string
   const unsigned int  w = TF.FontWidth();
   const unsigned int  h = TF.FontHeight();
   unsigned int        textmapwidth = tstring.Len()*w;

   // the offset of the timestamp in the image
   const unsigned int  X_OFFSET = 3;
   const unsigned int  Y_OFFSET = 3;

   if ( (textmapwidth+X_OFFSET) > (unsigned int)(pBmpHeader->biWidth) ) return; // image to small to carry timestamp
   if ( (h+Y_OFFSET) > (unsigned int)(pBmpHeader->biHeight) ) return; // image to small to carry timestamp

   unsigned int  x,y;
   wxUint8 *dstPtr, *srcPtr;

   // create a bitmap (array [row][col] --> [h][textmapwidth] for the string
   wxUint8 *textmap = new wxUint8[h*textmapwidth];

   // render the text into the temp bitmap
   for (x=0; x<tstring.Len(); x++) {
      // for each char
      const wxUint8* fontMap = TF.Letter(tstring[x]); // get the letter bitmap [row][col]
      if (fontMap) {
         // OK not NULL (i.e. we have a valid char)
         // copy each row of the letter into the proper map pos
         for (y=0; y<h; y++) {
            dstPtr = &textmap[y*textmapwidth];  // move to textmap row
            dstPtr+=x*w;            // then within row to char
            ::memcpy(dstPtr, &fontMap[y*w] ,w*sizeof(wxUint8)); // copy the characters scanline
         }
      }
   }
   // now blit the text birmap into the target bitmap
   wxUint8*    pbData = const_cast<wxUint8*>( reinterpret_cast<const wxUint8*>(pBmpData));

   if (pBmpHeader->biBitCount==24) {
      //BGR bitmap
      unsigned scanwidth  = ((pBmpHeader->biWidth*3 + 3) & ~3); // DWORD align (BMP stride)
      // each row will be copied into BGR pixels
      for (y=0; y<h; y++) {
         srcPtr = &textmap[y*textmapwidth];  // move to textmap row
         dstPtr = &pbData[(y+X_OFFSET)*scanwidth] + X_OFFSET; // start position of the text
         for (x=0; x<textmapwidth; x++) {
            *dstPtr++ = *(srcPtr+x);// copy the characters scanline into BGR
            *dstPtr++ = *(srcPtr+x);
            *dstPtr++ = *(srcPtr+x);
         }
      }
   }
   else if ( (pBmpHeader->biBitCount==8) || (pBmpHeader->biBitCount==12) ) {
      //YUV planar or plain mono bitmap - write directly into Y plane pixels
      unsigned scanwidth  = ((pBmpHeader->biWidth + 3) & ~3); // DWORD align (BMP stride)
      for (y=0;y<h; y++) {
         srcPtr = &textmap[y*textmapwidth];  // move to textmap row
         dstPtr = &pbData[(y+X_OFFSET)*scanwidth] + X_OFFSET; // start position of the text
         ::memcpy(dstPtr, srcPtr ,textmapwidth*sizeof(wxUint8)); // copy the characters scanline
      }
   }
   else if (pBmpHeader->biBitCount==16) {
      //YUV  4:2:2 interleave bitmap - write directly into interleaved Y plane pixels
      unsigned scanwidth  = ((pBmpHeader->biWidth*2 + 3) & ~3); // DWORD align (BMP stride)
      switch (pBmpHeader->biCompression) {
         case wxAC_FOURCC_YUY2:  // image is Y0-U0-Y1-V0 ...
         case wxAC_FOURCC_YUYV:  // image is Y0-U0-Y1-V0 ...
         case wxAC_FOURCC_YVYU: {// image is Y0-V0-Y1-U0 ...
            for (y=0;y<h; y++) {
               srcPtr = &textmap[y*textmapwidth];  // move to textmap row
               dstPtr = &pbData[(y+X_OFFSET)*scanwidth] + X_OFFSET*2; // start position of the text
               for (x=0; x<textmapwidth; x++) {
                  *dstPtr++ = *(srcPtr+x); dstPtr++;// skip U or V
               }
            }
            break;
         }
         case wxAC_FOURCC_UYVY: {// image is U0-Y0-V0-Y1 ...
            unsigned scanwidth  = ((pBmpHeader->biWidth*2 + 3) & ~3); // DWORD align (BMP stride)
            for (y=0;y<h; y++) {
               srcPtr = &textmap[y*textmapwidth];  // move to textmap row
               dstPtr = &pbData[(y+X_OFFSET)*scanwidth] + X_OFFSET*2 + 1; // skip leading U
               for (x=0; x<textmapwidth; x++) {
                  *dstPtr++ = *(srcPtr+x); dstPtr++;// skip U or V
               }
            }
            break;
         }
         default:
            ;
      }//switch

   }
   // get rid of tempdata
   delete[] textmap;

}

   //! @brief Render a timestamp into a 16bit YP16 type of image map with given gray levels
void wxTimeFormatter::DrawTimeStamp(const wxSize& frameSize, const wxUint16* pYData,
                      const wxDateTime tStamp, wxUint16 blackLevel, wxUint16 whiteLevel,
                      wxTimeFont::EFontSize fontSize)
{
   wxTimeFont TF(fontSize);  // create a specific font here

   unsigned short msec = tStamp.GetMillisecond(wxDateTime::UTC);
   wxString tstring = wxString::Format(wxT("%s.%03hu"), tStamp.Format(wxT("%Y-%m-%d %H:%M:%S"), wxDateTime::UTC).c_str(), msec);

   // allocate bitmap memory type for the string
   const unsigned int  w = TF.FontWidth();
   const unsigned int  h = TF.FontHeight();
   unsigned int        textmapwidth = tstring.Len()*w;

   // the offset of the timestamp in the image
   const unsigned int  X_OFFSET = 3;
   const unsigned int  Y_OFFSET = 3;

   if ( (textmapwidth+X_OFFSET) > (unsigned int)(frameSize.GetWidth()) ) return; // image to small to carry timestamp
   if ( (h+Y_OFFSET) > (unsigned int)(frameSize.GetHeight()) ) return; // image to small to carry timestamp

   unsigned int  x,y;
   wxUint16 *dstPtr, *srcPtr;

   // create a bitmap (array [row][col] --> [h][textmapwidth] for the string
   wxUint16 *textmap = new wxUint16[h*textmapwidth];

   // render the text into the temp bitmap
   for (x=0; x<tstring.Len(); x++) {
      // for each char
      const wxUint8* fontMap = TF.Letter(tstring[x]); // get the letter bitmap [row][col]
      if (fontMap) {
         // OK not NULL (i.e. we have a valid char)
         // copy each row of the letter into the proper map pos
         for (y=0; y<h; y++) {
            dstPtr = &textmap[y*textmapwidth];  // move to textmap row
            dstPtr+=x*w;            // then within row to char
            // copy the characters scanline; pixels here
            for (unsigned int p=0; p<w; p++) {
               *dstPtr++ = (fontMap[y*w+p]==0) ? blackLevel : whiteLevel; // pixelvalue in source are 000 or 255 only
            }
         }
      }
   }
   // now blit the text birmap into the target bitmap
   wxUint16*    pbData = const_cast<wxUint16*>( reinterpret_cast<const wxUint16*>(pYData));

   //YUV bitmap - write directly into plane pixels
   unsigned scanwidth  = frameSize.GetWidth();
   for (y=0;y<h; y++) {
      srcPtr = &textmap[y*textmapwidth];  // move to textmap row
      dstPtr = &pbData[(y+X_OFFSET)*scanwidth] + X_OFFSET; // start position of the text
      ::memcpy(dstPtr, srcPtr ,textmapwidth*sizeof(wxUint16)); // copy the characters scanline
   }
   // get rid of tempdata
   delete[] textmap;
}


