
#ifndef _WXWEBCOORDS_H__
#define _WXWEBCOORDS_H__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wxCamCoords.h"

class wxWebCoords : public wxCamCoords
{
public:
	wxWebCoords();
	virtual ~wxWebCoords();

public:
   // transformations
   wxCamPoint GetPoint(EPointOrigin org);
   wxSize GetFullSize(EPointOrigin org);
};


#endif  // _WXWEBCOORDS_H__

