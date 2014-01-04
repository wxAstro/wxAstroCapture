

#include "PortEnumUX.h"

#include "wxPort.h"

#include <wx/tokenzr.h>


#include "../debtools.h"

#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/txtstrm.h>
#include <wx/wfstream.h>

#include "ShoestringVidPid.h"
#include "SAUSB_LibUsb.h"


////////////////////////////////////////////////////////////////////////////////////////////
// class implementation

// The Enumerator Singleton

// Singleton
PortEnumUX& PortEnumUX::Instance()
{
	static PortEnumUX instance;
	return instance;
}


PortEnumUX::PortEnumUX()
: m_numPorts(0)
{
	for (size_t i=0; i<MAXITEM; i++)
	{
		m_pPorts[i] = NULL;
	}
	m_nullPort = new wxPortDescr();
	// just start with all enumerated
	EnumeratePorts();
}

PortEnumUX::~PortEnumUX()
{
	DropAll();
	delete m_nullPort;
}

void PortEnumUX::DropAll()
{
	for (size_t i=0; i<MAXITEM; i++)
	{
		if (m_pPorts[i])
			delete m_pPorts[i]; m_pPorts[i] = NULL;
	}
	m_numPorts = 0;
}



const wxPortDescr* PortEnumUX::GetPort(size_t cfIndex) const
{

	if (cfIndex>=m_numPorts)
	{
		return m_nullPort;
	}

	return m_pPorts[cfIndex];
}


// (Re)-Enumerate the Port devices
bool PortEnumUX::EnumeratePorts()
{
   DPRINT("\nPortEnumUX::EnumeratePorts - Starting Port detection ...\n");

   size_t num_items = 0;
   wxArrayString output, error;

	DropAll();
   // Dummy Port
   m_pPorts[num_items] = new wxPortDescr();
   m_pPorts[num_items]->Init(num_items, wxT("!None!"), 0, wxT("n.a."),
                              true, true, false,false,false,false,false);//Gx and Lx supported (to show it)
   num_items++;


   size_t devNr;
   long addr; wxString name, path;

   // parallel ones
   for (devNr=0; devNr<10; devNr++) {
      if (num_items>=MAXITEM) break;
      if ( ! EnumerateParport(devNr, addr, name, path) )
         continue; // try next
      m_pPorts[num_items] = new wxPortDescr();
      m_pPorts[num_items]->Init(num_items, name, addr, path, true,  true, // +guide +LX
                                                             false, true, false, false, false);
      num_items++;
   }

   // serial ones HW ports
   for (devNr=0; devNr<10; devNr++) {
      if (num_items>=MAXITEM) break;
      if ( ! EnumerateHWSerport(devNr, addr, name, path) )
         continue; // try next
      m_pPorts[num_items] = new wxPortDescr();
      m_pPorts[num_items]->Init(num_items, name, addr, path, true,  true, // +guide +LX
                                                             true, false, false, false, false);
      num_items++;
   }

   // serial ones USB ports
   for (devNr=0; devNr<10; devNr++) {
      if (num_items>=MAXITEM) break;
      if ( ! EnumerateUSBSerport(devNr, addr, name, path) )
         continue; // try next
      m_pPorts[num_items] = new wxPortDescr();
      m_pPorts[num_items]->Init(num_items, name, addr, path, true,  true, // +guide +LX
                                                             true, false, false, false, false);
      num_items++;
   }

   //BM:20090104 - added
   // USB ports for Shoestring GPUSB
   if ( EnumerateGPUSBport(name, path) ) {
      m_pPorts[num_items] = new wxPortDescr();
      m_pPorts[num_items]->Init(num_items, name, 0, path, true, false, // +guide -LX
                                                             false, false, true, false, false);
      num_items++;
   }

   // USB ports for Shoestring LXUSB
   if ( EnumerateLXUSBport(name, path) ) {
      m_pPorts[num_items] = new wxPortDescr();
      m_pPorts[num_items]->Init(num_items, name, 0, path, false, true, // -guide +LX
                                                             false, false, false, true, false);
      num_items++;
   }

#ifdef _DEBUG
   // TTY serial output for testing purposes
   m_pPorts[num_items] = new wxPortDescr();
   name = wxT("z ptyr0 -> ttyr0"); path = wxT("/dev/ptyr0"); addr =0;
   m_pPorts[num_items]->Init(num_items, name, addr, path, true, false, // +guide -LX
                                                          true, false, false, false, false);
   num_items++;
#endif

   m_numPorts = num_items;
   return true;
}

// use proc filesystem to check for parports
// <http://www.gelato.unsw.edu.au/lxr/source/Documentation/parport.txt>
bool PortEnumUX::EnumerateParport(size_t devNr, long& addr, wxString& name, wxString& path)
{
   DPRINT1("PortEnumUX::EnumerateParport - detecting parport%d now ", int(devNr));
   wxString bPath(wxT("/proc/sys/dev/parport"));
   if ( ! wxDir::Exists(bPath) ) {
      DPRINT1("- base path %s does not exist\n", (const char*)bPath.fn_str());
      return false;
   }
   path = wxString::Format(wxT("/parport%d"), (int)devNr);
   bPath += path;
   if ( ! wxDir::Exists(bPath) ) {
      DPRINT("- does not exist\n");
      return false;
   }

   // now get the base address
   wxFFileInputStream* input;
   wxTextInputStream* text;
   bPath += wxT("/base-addr");
   if ( ! wxFileName::FileExists(bPath) ) {
      DPRINT("- base-addr file does not exist - not a parport that can be identified\n");
      return false;
   }
   input = new wxFFileInputStream(bPath);
   if ( ! input->IsOk() ) {
      DPRINT("- error cannot read base-addr file - not a parport that can be identified\n");
      delete input;
      return false;
   }
   text = new wxTextInputStream( *input , wxT(" \t"));
// get something like:  888 \t  0
   (text->ReadWord()).ToLong(&addr, 0);
   delete text; delete input;

   name = wxString::Format(wxT("parport%d (0x%x)"), int(devNr), int(addr));
   path = wxT("/dev") + path;
   DPRINT1("- found: %s\n", (const char*)name.fn_str());
   return true;
}


// use sys filesystem to check for parallel
// /sys/class/ppdev/parport?/device/resources
/* content is like:
* state = active
* io 0x378-0x37f
* irq 7
*/

/*** SEEMS NOT TO DETECT PCI etc. type of parportx (only ppdev)
 *** switching back to old style now
 ***BM:20090426
//BM:20090104 - new version similar to serial port
bool PortEnumUX::EnumerateParport(size_t devNr, long& addr, wxString& name, wxString& path)
{
   wxString bPath(wxT("/sys/class/ppdev"));
   if ( ! wxDir::Exists(bPath) ) {
      DPRINT1("PortEnumUX::EnumerateParport - base path %s does not exist\n", (const char*)bPath.fn_str());
      return false;
   }
   path = wxString::Format(wxT("/parport%d"), int(devNr));
   bPath += path;
   if ( ! wxDir::Exists(bPath) ) {
      DPRINT("- does not exist\n");
      return false;
   }

   // now get the base address
   wxFFileInputStream* input;
   wxTextInputStream* text;
   bPath += wxT("/device/resources");
   if ( ! wxFileName::FileExists(bPath) ) {
      DPRINT("error - resources file does not exist - not a HW port\n");
      return false;
   }
   input = new wxFFileInputStream(bPath);
   if ( ! input->IsOk() ) {
      DPRINT("error - cannot read resources file - not a HW port\n");
      delete input;
      return false;
   }
   text = new wxTextInputStream( *input );
   text->ReadLine();text->ReadWord();
   (text->ReadWord()).ToLong(&addr, 0);
   delete text; delete input;

   name = wxString::Format(wxT("parport%d (0x%x)"), int(devNr), int(addr));
   path = wxT("/dev") + path;
   return true;
}

*/


// use sys filesystem to check for serial
// /sys/class/tty/ttyS?/device/resources
/* content is like:
* state = active
* io 0x2f8-0x2ff
* irq 3
*/

bool PortEnumUX::EnumerateHWSerport(size_t devNr, long& addr, wxString& name, wxString& path)
{
   DPRINT1("PortEnumUX::EnumerateSerport - detecting ttyS%d now ", int(devNr));
   wxString bPath(wxT("/sys/class/tty"));
   if ( ! wxDir::Exists(bPath) ) {
      DPRINT1("- base path %s does not exist\n", (const char*)bPath.fn_str());
      return false;
   }
   path = wxString::Format(wxT("/ttyS%d"), int(devNr));
   bPath += path;
   if ( ! wxDir::Exists(bPath) ) {
      DPRINT("- does not exist\n");
      return false;
   }

   // now get the base address
   wxFFileInputStream* input;
   wxTextInputStream* text;
   bPath += wxT("/device/resources");
   if ( ! wxFileName::FileExists(bPath) ) {
      DPRINT("- resources file does not exist - not a serialport that can be identified\n");
      return false;
   }
   input = new wxFFileInputStream(bPath);
   if ( ! input->IsOk() ) {
      DPRINT("- error cannot read resources file - not a serialport that can be identified\n");
      delete input;
      return false;
   }
   text = new wxTextInputStream( *input );
   text->ReadLine();text->ReadWord();
   (text->ReadWord()).ToLong(&addr, 0);
   delete text; delete input;

   name = wxString::Format(wxT("ttyS%d (0x%x)"), int(devNr), int(addr));
   path = wxT("/dev") + path;
   DPRINT1("- found: %s\n", (const char*)name.fn_str());
   return true;
}

// use sys filesystem to check for serial USB devices
// NOTE we cannot differentiate wether it is a serial port or something else....
// /sys/bus/usb/drivers/*/*-*/ttyUSB?
bool PortEnumUX::EnumerateUSBSerport(size_t devNr, long& addr, wxString& name, wxString& path)
{
   wxArrayString output, error;

   DPRINT1("PortEnumUX::EnumerateUSBSerport - detecting ttyUSB%d now ", int(devNr));

   wxString cmd(wxT("/bin/sh -c 'find -P /sys/bus/usb/drivers/*/*-*/ttyUSB"));
   cmd += wxString::Format(wxT("%d"), (int)devNr);   cmd += wxT(" -maxdepth 0'");
   // finds something like:   /sys/bus/usb/drivers/driver/1-2:1.0/ttyUSB1
   int code = wxExecute(cmd, output, error);
   if (code) {
      DPRINT("- does not exist\n");
      return false;  // shell exec returned error (most likely "file not found" - which is OK
   }

   size_t count = output.GetCount();
   if ( !count ) {
      DPRINT("- does not exist\n");
      return  false; // nothing found ??
   }
   wxString driver = output[0]; driver.Replace(wxT("/sys/bus/usb/drivers/"), wxT(""));
   driver = driver.BeforeFirst(wxT('/'));

   // Check if it is the Art Cam and forget it then
   if ( 0==driver.CmpNoCase(wxT("ftdi_sio"))) {
      // driver fits - check further
      //    /sys/bus/usb/devices/n-m/ .. idVendor, idProduct  are interesting
      wxString xpath = output[0].BeforeFirst(wxT(':'));
      xpath.Replace(wxT("drivers"), wxT("devices"));
      xpath.Replace(wxT("ftdi_sio/"), wxT(""));
      // should be like  /sys/bus/usb/devices/n-m    now
      if ( wxDir::Exists(xpath) ) {
         if ( IsArtCCDCamera(xpath) ) {
            DPRINT("- does not exist\n");
            return false;  // it is Art cam not a serial port
         }
      }
   }

   path = wxString::Format(wxT("/ttyUSB%d"), (int)devNr);
   addr = 0;
   name = wxString::Format(wxT("ttyUSB%d (%s)"), devNr, driver.c_str());
   path = wxT("/dev") + path;
   DPRINT1("- found: %s\n", (const char*)name.fn_str());
   return true;
}


bool PortEnumUX::EnumerateGPUSBport(wxString& name, wxString& path)
{
   DPRINT("PortEnumUX::EnumerateGPUSBport - detecting GPUSB now ");

   if ( SAUSB_LibUsb::DeviceAttached(SAUSB_LibUsb::SAUSB_GPUSB) ) {
      name = wxT("GPUSB");
      path = wxT("/dev/hid_gpusb_d:b");
      DPRINT1("- found: %s\n", (const char*)name.fn_str());
      return true;
   }
   DPRINT("- does not exist\n");
   return false;
}


bool PortEnumUX::EnumerateLXUSBport(wxString& name, wxString& path)
{
   DPRINT("PortEnumUX::EnumerateLXUSBport - detecting LXUSB now ");

   if ( SAUSB_LibUsb::DeviceAttached(SAUSB_LibUsb::SAUSB_LXUSB) ) {
      name = wxT("LXUSB");
      path = wxT("/dev/hid_lxusb_d:b");
      DPRINT1("- found: %s\n", (const char*)name.fn_str());
      return true;
   }
   DPRINT("- does not exist\n");
   return false;
}


bool PortEnumUX::IsArtCCDCamera(const wxString& basePath)
{
   wxString bPath = basePath;
   if (bPath.IsEmpty()) return false;

   // check if the port is owned by the FTDI - PID known
   //    /sys/bus/usb/devices/n-m/ .. idVendor, idProduct  are interesting

   // check USB VID

   wxFileInputStream* input;
   wxTextInputStream* text;

   input = new wxFileInputStream( bPath + wxT("/idVendor") ); if ( ! input->IsOk() ) {delete input; return false;};
   text = new wxTextInputStream( *input );
   wxString idVendor = text->ReadLine();
   delete text; delete input;

   if ( 0!=idVendor.CmpNoCase(wxT(FTDI_VENDOR_VID)) ) return false; // not sought VID

   // ok that is FTDI native VID - check product PIDs
   input = new wxFileInputStream( bPath + wxT("/idProduct") ); if ( ! input->IsOk() ) {delete input; return false;};
   text = new wxTextInputStream( *input );
   wxString idProduct = text->ReadLine();
   delete text; delete input;

   bool productKnown = false;
   if (0==idProduct.CmpNoCase(wxT(FTDI_ARTEMIS_PID)) ) productKnown=true;
   if (0==idProduct.CmpNoCase(wxT(FTDI_MINIART_PID)) ) productKnown=true;
   if (0==idProduct.CmpNoCase(wxT(FTDI_ATIK_ATK16_PID)) ) productKnown=true;
   if (0==idProduct.CmpNoCase(wxT(FTDI_ATIK_ATK16C_PID)) ) productKnown=true;
   if (0==idProduct.CmpNoCase(wxT(FTDI_ATIK_ATK16HR_PID)) ) productKnown=true;
   if (0==idProduct.CmpNoCase(wxT(FTDI_ATIK_ATK16HRC_PID)) ) productKnown=true;
   if (0==idProduct.CmpNoCase(wxT(FTDI_ATIK_ATK16IC_PID)) ) productKnown=true;
   if (0==idProduct.CmpNoCase(wxT(FTDI_ATIK_ATK16ICC_PID)) ) productKnown=true;
   if (0==idProduct.CmpNoCase(wxT(FTDI_ATIK_ATK16ICS_PID)) ) productKnown=true;
   if (0==idProduct.CmpNoCase(wxT(FTDI_ATIK_ATK16ICSC_PID)) ) productKnown=true;

   if ( ! productKnown ) return false; // not sought PID

   return true;
}


