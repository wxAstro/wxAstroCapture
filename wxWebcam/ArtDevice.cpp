

#include "ArtDevice.h"

#include "ArtDrv.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implement a ArtDevice
//
ArtDevice::ArtDevice()
: m_camControlled(false)
{
    m_DevEntry.Init();
}

ArtDevice::~ArtDevice()
{
}

// The device index
long ArtDevice::DevIndex(void) const {return m_DevEntry.s_CIndex;};
// The friendly device name (any two cams of the same brand have the same name)
const wxString& ArtDevice::DevName(void) const {return m_DevEntry.s_CName;};
// The MS serial assigned (serves to distinguis between cams of the same brand)
const wxString& ArtDevice::DevSerial(void) const {return m_DevEntry.s_DevSerial;};

// Returns true if the device is controlled i.e. a driver is attached
bool ArtDevice::IsDeviceControlled() const {return (m_camControlled);};
// Set the driver when it is a controlled device entry
void ArtDevice::ControlDevice(bool pCamDrvOK)
{
   m_camControlled = pCamDrvOK && (m_DevEntry.s_pCamDriver);
}
// Allow others to use the driver
ArtDrv* ArtDevice::PCamDriver() const {return m_DevEntry.s_pCamDriver;};

// set device properties
void ArtDevice::Init(long pCIndex, const wxString& pCName, const wxString& pDevSerial, ArtDrv* pCamDriver)
{
    // reset the struct i.e. the internal store
   m_camControlled=false;
   m_DevEntry.Init();

   // check if we got a proper item - then copy contents to expose
   if ( pCIndex>=0 ) {
      m_DevEntry.s_CIndex = pCIndex;
      m_DevEntry.s_CName = pCName;
      m_DevEntry.s_DevSerial = pDevSerial;
      m_DevEntry.s_pCamDriver = pCamDriver;
   }
}//Init

