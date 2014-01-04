

#include "wxPortManager.h"
#include "wxPort.h"

#include <vector>


/// wxPortHandle

//! A port handle maintains the relevant port information
//! provides the interfaces only to the the port manager (PM)
//! received with POpen() and used to access the port's interface through PM
wxPortHandle::wxPortHandle()
: m_port(NULL)
, m_portHID(0)
, m_users(0)
, m_mask(0)
{
}

wxPortHandle::wxPortHandle(size_t portHID, wxPort* port)
: m_port(port)
, m_portHID(portHID)
, m_users(0)
, m_mask(0)
{
}


bool wxPortHandle::IsPortParallel() const
{
   if (NULL==m_port) return false;
   return m_port->IsPortParallel();
}

bool wxPortHandle::IsPortSerial() const
{
   if (NULL==m_port) return false;
   return m_port->IsPortSerial();
}


bool wxPortHandle::IsPortGPUSB() const//BM:20081226 added
{
   if (NULL==m_port) return false;
   return m_port->IsPortGPUSB();
}


bool wxPortHandle::IsPortLXUSB() const//BM:20081226 added
{
   if (NULL==m_port) return false;
   return m_port->IsPortLXUSB();
}


bool wxPortHandle::IsPortLXLED() const//BM:20081226 added
{
   if (NULL==m_port) return false;
   return m_port->IsPortLXLED();
}


bool wxPortHandle::IsPortDummy() const
{
   if (NULL==m_port) return true;
   return m_port->IsPortDummy();
}

const wxString wxPortHandle::s_nullString = wxString(wxT(""));

const wxString& wxPortHandle::PortName() const
{
   if (NULL==m_port) return s_nullString;
   return m_port->PortName();
}




/// wxPortManager

wxPortManager::wxPortManager()
: m_portHID_Next(1)
{
 //... perform necessary instance initializations
   EnumeratePorts();
}

wxPortManager::~wxPortManager()
{
   m_portsManaged.ClearList();
   m_ports.clear();
}

void wxPortManager::EnumeratePorts()
{
   m_portsManaged.ClearList();

   wxPort::PortEntryVector pv;
   wxPort::EnumeratePorts(pv);
   for (size_t i=0; i<pv.size(); i++) {
      m_portsManaged.AddPortDesc(pv[i]);
   }//for
}


/// working with port descriptors

//! returns a port descriptor with name
const wxPortDescr& wxPortManager::PortDescriptor(const wxString& portName) const
{
   return m_portsManaged.PortDescr(portName);
}

//! returns the first port descriptor
const wxPortDescr& wxPortManager::PortDescriptorFirst() const
{
   return m_portsManaged.PortDescrFirst();
}

//! returns a port descriptor list
wxPortDescrList& wxPortManager::PortDescriptors()
{
   return m_portsManaged;
}


/// working with ports

//! Open a port for usage
wxPortHandle wxPortManager::POpen(const wxPortDescr& portDesc, wxUint16 mask)
{
   // before using the list we must acquire the mutex
   wxMutexLocker lock(m_mutexPortIO);

   // see if we use this port already
   PORTMAP_ITERATOR iter;
   for (iter=m_ports.begin(); iter!=m_ports.end(); iter++) {
      // search for the same port name
      if ( portDesc.PortName().IsSameAs( ((*iter).second).Port()->PortName() , false) ){
         // we use this one already
         ((*iter).second).AddUser(); // add refcount to the internal obj
         // make a copy
         wxPortHandle c((*iter).second);
         c.SetMask(mask); // the mask is maintained only in the returned obj
         return c;      // and return it as handle
      }
      else {
         // not found
         continue;
      }
   }
   // did not found it - allocate and return it

   // create a new port from the descriptor
   wxPort* xport = wxPort::Construct(portDesc);
   size_t thisPort = m_portHID_Next++;
   // create a manager handle
   wxPortHandle x(thisPort, xport);
   x.AddUser();
   m_ports[thisPort] = x; // copy to the managers map

   // prepare the handle for the user
   x.SetMask(mask); // the mask is maintained only in the returned obj
   return x; // and return as copy
}

//! Reopens a released port - must have been Opened before
bool wxPortManager::PReopen(wxPortHandle& pHandle)
{
   PORTMAP_ITERATOR iter;
   if ( (iter=m_ports.find(pHandle.PortHID())) != m_ports.end() ) {
      // having found the handle in our internal list
      ; // continue
   }
   else {
      // cannot find the port handle in our list ???
      wxASSERT(false);
      return false;
   }
   if ( ! pHandle.PortValid() ) return false;
   // before using the list we must acquire the mutex
   wxMutexLocker lock(m_mutexPortIO);
   return (wxPort::IPAC_Success==pHandle.Port()->PReopen());
}

//! Releases the port but not the handle (use PReopen() to use it again)
void wxPortManager::PRelease(wxPortHandle& pHandle)
{
   PORTMAP_ITERATOR iter;
   if ( (iter=m_ports.find(pHandle.PortHID())) != m_ports.end() ) {
      // having found the handle in our internal list
      ; // continue
   }
   else {
      // cannot find the port handle in our list ???
      wxASSERT(false);
      return;
   }
   if ( ! pHandle.PortValid() ) return;
   // before using the list we must acquire the mutex
   wxMutexLocker lock(m_mutexPortIO);
   pHandle.Port()->PRelease();
}

//! Reset the bits used with this port
bool wxPortManager::PSetMask(wxPortHandle& pHandle, wxUint16 mask)
{
   PORTMAP_ITERATOR iter;
   if ( (iter=m_ports.find(pHandle.PortHID())) != m_ports.end() ) {
      // having found the handle in our internal list
      ; // continue
   }
   else {
      // cannot find the port handle in our list ???
      wxASSERT(false);
      return false;
   }
   // handle found
   return pHandle.SetMask(mask); // the mask is maintained only in the user obj
}


//! Close port an release it
void wxPortManager::PClose(wxPortHandle& pHandle)
{
   PORTMAP_ITERATOR iter;
   if ( (iter=m_ports.find(pHandle.PortHID())) != m_ports.end() ) {
      // having found the handle in our internal list
      ; // continue
   }
   else {
      // cannot find the port handle in our list ???
      wxASSERT(false);
      return;
   }
   // handle found
   pHandle.DisablePortHandle(); // disable the users handle

   // before using the list we must acquire the mutex
   wxMutexLocker lock(m_mutexPortIO);

   (*iter).second.RemoveUser(); // dec refcount in our managed handle
   if ( (*iter).second.HasUsers() ) {
     // there are still other users around, just leave
      return;
   }
   // no more users, close port and drop internal handle
   if ( (*iter).second.PortValid() ) {
      (*iter).second.Port()->PClose();
      delete (*iter).second.Port(); // we own wxPort, so delete it here
   }
   m_ports.erase(iter); // delete the handle entry as well
}


//! Read from port
bool wxPortManager::PGet(wxPortHandle& pHandle, wxUint16& pValue)
{
   if ( m_ports.find(pHandle.PortHID()) != m_ports.end() ) {
      ; // continue
   }
   else {
      // cannot find the port handle in our list ???
      wxASSERT(false);
      return false;
   }

   if ( ! pHandle.PortValid() ) return false;

   // before using the list we must acquire the mutex
   wxMutexLocker lock(m_mutexPortIO);
   return (wxPort::IPAC_Success==pHandle.Port()->PGet(pValue, pHandle.Mask()));
}

//! Write to port
bool wxPortManager::PPut(wxPortHandle& pHandle, wxUint16 pValue)
{
   if ( m_ports.find(pHandle.PortHID()) != m_ports.end() ) {
      ; // continue
   }
   else {
      // cannot find the port handle in our list ???
      wxASSERT(false);
      return false;
   }

   if ( ! pHandle.PortValid() ) return false;

   // before using the list we must acquire the mutex
   wxMutexLocker lock(m_mutexPortIO);
   return (wxPort::IPAC_Success==pHandle.Port()->PPut(pValue, pHandle.Mask()));
}



//! Set the ports bauderate (effective for serial RX/TX only)
bool wxPortManager::PSetBaudrate(wxPortHandle& pHandle, wxPort::EBaudRate baudrate)
{
   if ( m_ports.find(pHandle.PortHID()) != m_ports.end() ) {
      ; // continue
   }
   else {
      // cannot find the port handle in our list ???
      wxASSERT(false);
      return false;
   }

   if ( ! pHandle.PortValid() ) return false;

   // before using the list we must acquire the mutex
   wxMutexLocker lock(m_mutexPortIO);
   return (wxPort::IPAC_Success==pHandle.Port()->PSetBaudrate(baudrate));
}

//! Set the ports handshake (effective for serial RX/TX only)
bool wxPortManager::PSetHandshake(wxPortHandle& pHandle, wxPort::EHandshake handshake)
{
   if ( m_ports.find(pHandle.PortHID()) != m_ports.end() ) {
      ; // continue
   }
   else {
      // cannot find the port handle in our list ???
      wxASSERT(false);
      return false;
   }

   if ( ! pHandle.PortValid() ) return false;

   // before using the list we must acquire the mutex
   wxMutexLocker lock(m_mutexPortIO);
   return (wxPort::IPAC_Success==pHandle.Port()->PSetHandshake(handshake));
}

//! Set the rw timeout (if supported)
bool wxPortManager::PSetTimeout(wxPortHandle& pHandle, long toInMS, long toOutMS)
{
   if ( m_ports.find(pHandle.PortHID()) != m_ports.end() ) {
      ; // continue
   }
   else {
      // cannot find the port handle in our list ???
      wxASSERT(false);
      return false;
   }

   if ( ! pHandle.PortValid() ) return false;

   // before using the list we must acquire the mutex
   wxMutexLocker lock(m_mutexPortIO);
   return (wxPort::IPAC_Success==pHandle.Port()->PSetTimeout(toInMS, toOutMS));
}


//! Receive a byte (effective for serial RX only)
bool wxPortManager::PRx(wxPortHandle& pHandle, wxUint8& pValue)
{
   if ( m_ports.find(pHandle.PortHID()) != m_ports.end() ) {
      ; // continue
   }
   else {
      // cannot find the port handle in our list ???
      wxASSERT(false);
      return false;
   }

   if ( ! pHandle.PortValid() ) return false;

   // before using the list we must acquire the mutex
   wxMutexLocker lock(m_mutexPortIO);
   return (wxPort::IPAC_Success==pHandle.Port()->PRx(pValue));
}

//! Send a byte (effective for serial TX only)
bool wxPortManager::PTx(wxPortHandle& pHandle, wxUint8 pValue)
{
   if ( m_ports.find(pHandle.PortHID()) != m_ports.end() ) {
      ; // continue
   }
   else {
      // cannot find the port handle in our list ???
      wxASSERT(false);
      return false;
   }

   if ( ! pHandle.PortValid() ) return false;

   // before using the list we must acquire the mutex
   wxMutexLocker lock(m_mutexPortIO);
   return (wxPort::IPAC_Success==pHandle.Port()->PTx(pValue));
}

