

#ifndef _WXYP162FITS_H__
#define _WXYP162FITS_H__


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "fits/wxFitsOut.h"


/// DECLARE wxYP162Fits

// Derived from wxFits
// provides a Win BMP to FITS format conversion class
class wxYP162Fits : public wxFitsOut
{
public:
   // takes Info Header and BMP data
   bool AddYP16(const wxSize& frameSize, const void* yData);

};


#endif  // _WXYP162FITS_H__
