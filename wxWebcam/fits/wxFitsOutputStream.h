

#ifndef _WXFITSOUTPUTSTREAM_H__
#define _WXFITSOUTPUTSTREAM_H__


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

class wxFitsOut;
class wxFitsOutputStream : public wxOutputStream
{
public:
   wxFitsOutputStream(wxOutputStream& s);

   // write the Fits obj to the stream
   wxOutputStream&  Write(const wxFitsOut& fits);

protected:
    wxOutputStream &m_output;

};

#endif // _WXFITSOUTPUTSTREAM_H__
