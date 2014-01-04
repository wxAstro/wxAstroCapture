
#ifndef _WXPORTDESCR_H__
#define _WXPORTDESCR_H__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <map>
#include <string>


class wxPortDescr
{
public:
// Allow initialization of content by owner class
friend class PortEnumW32;
friend class PortEnumUX;

    // constructor inits the content as an invalid device (index = -1)
	wxPortDescr();
	virtual ~wxPortDescr(){};

	// The port index
	size_t PortIndex(void) const;
	// The friendly port name
	const wxString& PortName(void) const;
	// The HW port address (win)
	long PortAddress(void) const;
	// The device path such as /dev/parallel0 (Linux)
	const wxString& PortPath(void) const;

	bool IsGuidePort() const;//BM:20081227 added
	bool IsLxPort() const;//BM:20081227 added

	bool IsPortParallel() const;
	bool IsPortSerial() const;
	bool IsPortGPUSB() const;//BM:20081227 added
	bool IsPortLXUSB() const;//BM:20081227 added
	bool IsPortLXLED() const;//BM:20081227 added
	bool IsPortDummy() const;

private:
	// hidden from general access, only the provider can use it
	//BM:20081228 more tags added for better support
	void Init(size_t pFIndex, const wxString& pFName, const long pPortAdr,
                  const wxString& pPortPath,
                  bool guidePort, bool lxPort,
                  bool serPort, bool parPort, bool gpusbPort, bool lxusbPort, bool lxledPort);
	// Capture Device attribute storage
	struct SPortDescr
	{
		SPortDescr(){ Init();}
		void Init(){
			FIndex=0;
			FName.Clear(); PortAdr=0; PortPath.Clear();
			isGuidePort=true; isLxPort=true;
			isSerial=false; isParallel=false;
			isUSB_GPUSB=false; isUSB_LXUSB=false; isUSB_LXLED=false;
			isDummy=true;
		}

		size_t FIndex;       // The index of the entry
		wxString FName;      // The ports friendly name - show this one
		long PortAdr;        // HW port address
		wxString PortPath;   // The path /dev/parallel0
		bool isGuidePort, isLxPort; //BM:20081227 added
		bool isSerial, isParallel;
		bool isUSB_GPUSB, isUSB_LXUSB, isUSB_LXLED;//BM:20081227 added
		bool isDummy;
	};

	SPortDescr  m_PortDescr; // the real storage};
};


/// wxPortDescrList  - a list of port descriptors

class wxPortDescrList
{
friend class wxPortManager;  // can add/remove elements

public:
   wxPortDescrList();
   virtual ~wxPortDescrList();

private:
   typedef std::map<std::string, wxPortDescr> PORTDESCMAP;

public:
   // kind of enumeration
   typedef PORTDESCMAP::iterator port_iterator;
   port_iterator port_begin(){return m_portDescriptors.begin();};
   port_iterator port_end(){return m_portDescriptors.end();};

	const wxPortDescr& PortDescrFirst() const;
	const wxPortDescr& PortDescr(const wxString& pName) const;
	const wxPortDescr& PortDescr(port_iterator iter) const {return (*iter).second;};


private:
   typedef PORTDESCMAP::iterator iterator;

   void AddPortDesc(wxPortDescr descriptor);
   void RemovePortDesc(const wxString& pName);
   void ClearList();

   PORTDESCMAP    m_portDescriptors;
   wxPortDescr    m_nullDescriptor;
};


#endif // _WXPORTDESCR_H__
