
#ifndef _WXPORTMANAGER_H__
#define _WXPORTMANAGER_H__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wxPortDescr.h"
#include "wxPort.h"

#include <string>
#include <map>


/// wxPortHandle

//! A port handle maintains the relevant port information
//! provides the interfaces only to the the port manager (PM)
//! received with POpen() and used to access the port's interface through PM

class wxPort;
class wxPortHandle
{
friend class wxPortManager; // only the manager is allowed to access the port internals

public:
   wxPortHandle();
   wxPortHandle(size_t portHID, wxPort* port);
   // default copy constructor and operator'=' should to right - so it is omitted here

public:
   //! returns true if the port obj can be used
   bool PortValid() const;
   bool IsPortParallel() const;
   bool IsPortSerial() const;
   bool IsPortGPUSB() const;  //BM:20081226 added
   bool IsPortLXUSB() const;  //BM:20081226 added
   bool IsPortLXLED() const;  //BM:20081226 added
   bool IsPortDummy() const;

   const wxString& PortName() const;

private:
   //! returns the port object ptr
   wxPort* Port();

   //! returns the handleID (used to track orphaned ports)
   size_t PortHID() const;
   //! returns true if the port is still in use (refcounts>0)
   bool HasUsers() const;
   //! add a user (++ refcount)
   void AddUser();
   //! remove a user (-- refcount) - once the refcount becomes 0 the handle is dropped
   void RemoveUser();

   //! the the users bitmask which are the bits that access is possible
   bool SetMask(wxUint16 mask);
   //! retrieve that mask
   wxUint16 Mask() const;

   //! deletes references to a real port and therefore disable access through this object
   void DisablePortHandle();

private:
   wxPort*        m_port;      // ptr to real port obj
   size_t         m_portHID;   // the handleID
   size_t         m_users;     // the refcount
   wxUint16       m_mask;      // the bitmask asked for with POpen()
   static const wxString s_nullString;
};
//! returns true if the port obj can be used
inline bool wxPortHandle::PortValid() const {return ( (NULL!=m_port) && (m_portHID>0) );}
//! returns the port object ptr
inline wxPort* wxPortHandle::Port() {return m_port;}
//! returns the handleID (used to track orphaned ports)
inline size_t wxPortHandle::PortHID() const {return m_portHID;}
//! returns true if the port is still in use (refcounts)
inline bool wxPortHandle::HasUsers() const {return (m_users>0);}
//! add a user (increment refcount)
inline void wxPortHandle::AddUser() {m_users++;}
//! remove a user (decrement refcount) - once the refcount becomes 0 the handle is dropped
inline void wxPortHandle::RemoveUser() { if (m_users>0) m_users--;}
inline bool wxPortHandle::SetMask(unsigned short mask){m_mask = mask; return true;}
inline wxUint16 wxPortHandle::Mask() const {return m_mask;}
//! disable this portHandle
inline void wxPortHandle::DisablePortHandle() {m_port=NULL; m_portHID=0;}




//! The Port Manager should be allocated once per application only

class wxPortDescrList;
class wxPortManager
{
public:
   wxPortManager();
   virtual ~wxPortManager();

public:

   /// working with port descriptors

   void EnumeratePorts();

   //! returns a port descriptor with name
   const wxPortDescr& PortDescriptor(const wxString& portName) const;
   //! returns the first port descriptor
   const wxPortDescr& PortDescriptorFirst() const;
   //! returns a port descriptor list
   wxPortDescrList& PortDescriptors();

   /// working with ports

   //! Open a port for usage, returning a handle
   wxPortHandle POpen(const wxPortDescr& portDesc, unsigned short mask);
   //! Reset the bits used with this port
   bool PSetMask(wxPortHandle& pHandle, unsigned short mask);
   //! Close port an release it
   void PClose(wxPortHandle& pHandle);

   //! Reopens a released port - must have been Opened before
   bool PReopen(wxPortHandle& pHandle);
   //! Releases the port but not the handle (use PReopen() to use it again)
   void PRelease(wxPortHandle& pHandle);

   //! Read from port
   bool PGet(wxPortHandle& pHandle, unsigned short& pValue);
   //! Write to port
   bool PPut(wxPortHandle& pHandle, unsigned short pValue);

   //! Set the ports bauderate (effective for serial RX/TX only)
   bool PSetBaudrate(wxPortHandle& pHandle, wxPort::EBaudRate baudrate);
   //! Set the ports handshake (effective for serial RX/TX only)
   bool PSetHandshake(wxPortHandle& pHandle, wxPort::EHandshake handshake);
   //! Set the rw timeout (if supported)
   bool PSetTimeout(wxPortHandle& pHandle, long toInMS, long toOutMS);

   //! Receive a byte (effective for serial RX only)
   bool PRx(wxPortHandle& pHandle, unsigned char& pValue);
   //! Send a byte (effective for serial TX only)
   bool PTx(wxPortHandle& pHandle, unsigned char pValue);

private:
   // disallow actions on this object
   wxPortManager(const wxPortManager&);
   wxPortManager& operator= (const wxPortManager&);

private:
   wxMutex        m_mutexPortIO;

   // holds the list of managed i.e. known ports
   wxPortDescrList  m_portsManaged;

   // defines the used ports which is a wxPortHandle for each used port
   typedef std::map<size_t, wxPortHandle>  PORTMAP;
   typedef PORTMAP::iterator  PORTMAP_ITERATOR;
   PORTMAP   m_ports;         // allocates the real ports used
   size_t    m_portHID_Next;  // next handleID to be used, starts with 1 (0=unused)
};


#endif // _WXPORTMANAGER_H__

