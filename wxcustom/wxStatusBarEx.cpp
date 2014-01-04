#include "wxStatusBarEx.h"

#ifndef _WXMSW_

#include <wx/wx.h>
#include <wx/dcclient.h>

BEGIN_EVENT_TABLE(wxStatusBarEx,wxStatusBar)
	EVT_PAINT(wxStatusBarEx::OnPaint)
END_EVENT_TABLE()

wxStatusBarEx::wxStatusBarEx(wxWindow* parent, wxWindowID id, long style, const wxString& name)
: wxStatusBar(parent,id,style,name)
{
   //ctor
}

wxStatusBarEx::~wxStatusBarEx()
{
   //dtor
}

/// this code is based on wxStatusBarGeneric::OnPaint
void wxStatusBarEx::OnPaint(wxPaintEvent& WXUNUSED(event) )
{
    wxPaintDC dc(this);

    if (GetFont().Ok())
        dc.SetFont(GetFont());

    dc.SetBackgroundMode(wxTRANSPARENT);

    /// this is added to allow the text to be sensible in the status bar
    /// when using red theme
    dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));

    for (int i = 0; i < m_nFields; i ++)
        DrawField(dc, i);
}

#endif
