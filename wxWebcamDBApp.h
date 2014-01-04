/***************************************************************
 * Name:      wxWebcamDBApp.h
 * Purpose:   Defines Application Class
 * Author:    ca ()
 * Created:   2007-04-11
 * Copyright: ca ()
 * License:   
 **************************************************************/
 
#ifndef WXWEBCAMDBAPP_H
#define WXWEBCAMDBAPP_H

#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

class wxWebcamDBApp : public wxApp
{
	public:
		virtual bool OnInit();
};

#endif // wxWebcamDBAPP_H
