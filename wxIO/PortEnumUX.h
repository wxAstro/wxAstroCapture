
#ifndef _PORTENUMUX_H__
#define _PORTENUMUX_H__


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


// Singleton Instance helper
#define PORT_ENUMERATOR( vname )  PortEnumUX& (vname) = PortEnumUX::Instance()

class wxPortDescr;
class PortEnumUX
{
public:
	static PortEnumUX& Instance();
	virtual ~PortEnumUX();

public:
   //! get the port with index out of our list
	const wxPortDescr* GetPort(size_t cfIndex) const;
	//! enumerate the serial and parallel ports
	bool  EnumeratePorts();
	//! the number of items in the port list
	size_t  NumItems() {return m_numPorts;};


private:
	PortEnumUX();
	PortEnumUX(const PortEnumUX&);

   bool EnumerateParport(size_t devNr, long& addr, wxString& name, wxString& path);
   bool EnumerateHWSerport(size_t devNr, long& addr, wxString& name, wxString& path);
   bool EnumerateUSBSerport(size_t devNr, long& addr, wxString& name, wxString& path);
   bool EnumerateGPUSBport(wxString& name, wxString& path);
   bool EnumerateLXUSBport(wxString& name, wxString& path);
   bool IsArtCCDCamera(const wxString& basePath);

	void DropAll();


	static const size_t MAXITEM = 20;   // holds a maximum # of ports
	size_t            m_numPorts;
	wxPortDescr*      m_pPorts[MAXITEM];
	wxPortDescr*      m_nullPort;
};

#endif  //_PORTENUMUX_H__


