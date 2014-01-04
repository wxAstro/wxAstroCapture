/***************************************************************
 * Name:      wxWebcamDBApp.cpp
 * Purpose:   Code for Application Class
 * Author:    ca ()
 * Created:   2007-04-11
 * Copyright: ca ()
 * License:
 **************************************************************/

#ifdef WX_PRECOMP //
#include "wx_pch.h"
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "wxWebcamDBApp.h"
#include <wx/stdpaths.h>
#include <wx/dir.h>

//(*AppHeaders
#include "wxWebcamDBMain.h"
#include <wx/image.h>
//*)

IMPLEMENT_APP(wxWebcamDBApp);

bool wxWebcamDBApp::OnInit()
{
	//(*AppInitialize
	bool wxsOK = true;
	wxInitAllImageHandlers();
	if ( wxsOK )
	{
		wxWebcamDBFrame* Frame = new wxWebcamDBFrame(0);
		Frame->Show();
		SetTopWindow(Frame);
	}
	//*)
	return wxsOK;

}
