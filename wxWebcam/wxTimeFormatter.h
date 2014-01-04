
/// Formats RGB24 and YUV bitmaps wxDateTime via wxTimeFont
//
//  e.g. 12.04.2009 13:22:23.52
//  or   04/12/2009 13:22:23.52


#ifndef _WXTIMEFORMATTER_H_
#define _WXTIMEFORMATTER_H_


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wxCameraTypes.h" // enums and structs of the capture environment
#include "wxTimeFont.h"

class wxTimeFormatter
{
public:
   //! @brief Render a timestamp into an 8bit BMP either RGB24 or YUV is supported
   static void DrawTimeStamp(const PBITMAPINFOHEADER pBmpHeader, const wxUint8* pBmpData,
                      const wxDateTime tStamp, wxTimeFont::EFontSize fontSize);

   //! @brief Render a timestamp into a 16bit YP16 type of image map with given gray levels
   static void DrawTimeStamp(const wxSize& frameSize, const wxUint16* pYData,
                      const wxDateTime tStamp, wxUint16 blackLevel, wxUint16 whiteLevel,
                      wxTimeFont::EFontSize fontSize);
private:
};

#endif // _WXTIMEFORMATTER_H_
