
#ifndef _PORTENUMW32_H__
#define _PORTENUMW32_H__


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


// Singleton Instance helper
#define PORT_ENUMERATOR( vname )  PortEnumW32& (vname) = PortEnumW32::Instance()

class wxPortDescr;
class PortEnumW32
{
public:
	static PortEnumW32& Instance();
	virtual ~PortEnumW32();

public:
	const wxPortDescr* GetPort(size_t cfIndex) const;
	bool  EnumeratePorts();
	size_t  NumItems() {return m_numPorts;};


private:
	PortEnumW32();
	PortEnumW32(const PortEnumW32&);

	void DropAll();
   void EnumParallel(unsigned long& port_items);
   void EnumSerial(unsigned long& port_items);
   void EnumGPUSB(unsigned long& port_items);
   void EnumLXUSB(unsigned long& port_items);

	static const size_t MAXITEM = 20;   // holds a maximum # of ports
	size_t            m_numPorts;
	wxPortDescr*      m_pPorts[MAXITEM];
	wxPortDescr*      m_nullPort;
};

#endif  //_PORTENUMW32_H__

