
#include "wxPortDescr.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implement a wxPortDescr
//
wxPortDescr::wxPortDescr()
{
    m_PortDescr.Init();
}
// The port index
size_t wxPortDescr::PortIndex(void) const {return m_PortDescr.FIndex;};
// The friendly port name
const wxString& wxPortDescr::PortName(void) const {return m_PortDescr.FName;};
// The HW port address (win)
long wxPortDescr::PortAddress(void) const {return m_PortDescr.PortAdr;};
// The device path such as /dev/parallel0 (Linux)
const wxString& wxPortDescr::PortPath(void) const {return m_PortDescr.PortPath;};

bool wxPortDescr::IsGuidePort() const {return m_PortDescr.isGuidePort;};//BM:20081228 added
bool wxPortDescr::IsLxPort() const {return m_PortDescr.isLxPort;};//BM:20081228 added

bool wxPortDescr::IsPortParallel() const {return m_PortDescr.isParallel;};
bool wxPortDescr::IsPortSerial() const {return m_PortDescr.isSerial;};
bool wxPortDescr::IsPortGPUSB() const {return m_PortDescr.isUSB_GPUSB;};//BM:20081227 added
bool wxPortDescr::IsPortLXUSB() const {return m_PortDescr.isUSB_LXUSB;};//BM:20081227 added
bool wxPortDescr::IsPortLXLED() const {return m_PortDescr.isUSB_LXLED;};//BM:20081227 added
bool wxPortDescr::IsPortDummy() const {return m_PortDescr.isDummy;};

void wxPortDescr::Init(size_t pFIndex, const wxString& pFName, const long pPortAdr,
               const wxString& pPortPath,
               bool guidePort, bool lxPort, //BM:20081228 added
               bool serPort, bool parPort,
               bool gpusbPort, bool lxusbPort, bool lxledPort)
{
   // reset the struct i.e. the internal store
   m_PortDescr.Init();

    // check if we got a proper item - then copy contents to expose
   m_PortDescr.FIndex = pFIndex;

   m_PortDescr.FName = pFName;
   m_PortDescr.PortAdr = pPortAdr;
   m_PortDescr.PortPath = pPortPath;

   m_PortDescr.isGuidePort = guidePort;//BM:20081228 added
   m_PortDescr.isLxPort    = lxPort;   //BM:20081228 added

   m_PortDescr.isSerial    = serPort;
   m_PortDescr.isParallel  = parPort;
   m_PortDescr.isUSB_GPUSB = gpusbPort;//BM:20081227 added
   m_PortDescr.isUSB_LXUSB = lxusbPort;//BM:20081227 added
   m_PortDescr.isUSB_LXLED = lxledPort;//BM:20081227 added
   m_PortDescr.isDummy     = ! (serPort || parPort || gpusbPort || lxusbPort || lxledPort);
}


/// wxPortDescrList  - a list of port descriptors

wxPortDescrList::wxPortDescrList()
{
}

wxPortDescrList::~wxPortDescrList()
{
   ClearList();
}


const wxPortDescr& wxPortDescrList::PortDescr(const wxString& pName) const
{
   std::string pn(pName.fn_str());
   PORTDESCMAP::const_iterator citer;

   if ( (citer=m_portDescriptors.find(pn)) != m_portDescriptors.end() ) {
      return (*citer).second;
   }
   else {
      return m_nullDescriptor;
   }
}

const wxPortDescr& wxPortDescrList::PortDescrFirst() const
{
   PORTDESCMAP::const_iterator citer = m_portDescriptors.begin();

   if ( citer != m_portDescriptors.end() ) {
      return (*citer).second;
   }
   else {
      return m_nullDescriptor;
   }
}

void wxPortDescrList::AddPortDesc(wxPortDescr descriptor)
{
   std::string pn(descriptor.PortName().fn_str());
   m_portDescriptors[pn] = descriptor;
}


void wxPortDescrList::RemovePortDesc(const wxString& pName)
{
   std::string pn(pName.fn_str());
   iterator iter;

   if ( (iter=m_portDescriptors.find(pn)) != m_portDescriptors.end() ) {
      m_portDescriptors.erase(iter);
   }
}

void wxPortDescrList::ClearList()
{
   m_portDescriptors.clear();
}





