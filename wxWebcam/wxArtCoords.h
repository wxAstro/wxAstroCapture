
#ifndef _WXARTCOORDS_H__
#define _WXARTCOORDS_H__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wxCamCoords.h"

class wxArtCoords : public wxCamCoords
{
public:
	wxArtCoords();
	virtual ~wxArtCoords();

public:
   // transformations
   wxCamPoint GetPoint(EPointOrigin org);
   wxSize GetFullSize(EPointOrigin org);
};


#endif  // _WXARTCOORDS_H__

