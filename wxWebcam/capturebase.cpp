
// the very basic capture objects are defined here

#include "capturebase.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implement a CapDeviceEntry
//
CapDeviceEntry::CapDeviceEntry()
{
    m_DevEntry.Init();
}
// The device index
size_t CapDeviceEntry::DevIndex(void) const {return m_DevEntry.FIndex;};
// The friendly device name (any two cams of the same brand have the same name)
const wxString& CapDeviceEntry::DevName(void) const {return m_DevEntry.FName;};
// The MS serial assigned (serves to distinguis between cams of the same brand)
const wxString& CapDeviceEntry::DevSerial(void) const {return m_DevEntry.DevSerial;};
// The device path such as /dev/videoN
const wxString& CapDeviceEntry::DevPath(void) const {return m_DevEntry.DevPath;};

void CapDeviceEntry::Init(size_t pFIndex, const wxString& pFName, const wxString& pDevSerial, const wxString& pDevPath)
{
    // reset the struct i.e. the internal store
    m_DevEntry.Init();

    // check if we got a proper item - then copy contents to expose
   m_DevEntry.FIndex = pFIndex;

   m_DevEntry.FName = pFName;
   m_DevEntry.DevSerial = pDevSerial;
   m_DevEntry.DevPath = pDevPath;
}//Init




