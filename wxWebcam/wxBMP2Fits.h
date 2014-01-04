

#ifndef _WXBMP2FITS_H__
#define _WXBMP2FITS_H__


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "fits/wxFitsOut.h"


/// DECLARE wxBMP2Fits

// Derived from wxFits
// provides a Win BMP to FITS format conversion class
class wxBMP2Fits : public wxFitsOut
{
public:
   // takes one BMP without palette i.e. data follows header immediately
   bool AddBMP(const void* bmp);

   // takes Info Header and BMP data
   bool AddBMP(const void* bmpHeader, const void* bmpData);

};


#endif  // _WXBMP2FITS_H__
