
// the very basic capture objects are defined here

#include "ArtBase.h"

#include "ArtDrv.h"        // driver prototype
#include "Atik16Drv.h"     // the Linux USB FTDI245 driver for Artemis/ARTEMIS cameras
#include "ArtDrvW32DLL.h"  // the Windows orignal Artemis CCD DLL driver
#include "ArtDrvSim.h"     // an Artemis/ARTEMIS camera simulator


#include <memory.h>

// the temperature query intervall allowed [sec]
#define TEMP_INTERVAL  5
// the -+delta from setpoint to realtemp which is said to be on target °C*100
//  50 means abs(temp-set)<= 0.5°C is OK
#define TEMP_OK_DELTA  50

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  ArtBase     - an class that defines one of the cameras supported
//

//! static init

// the Artemis driver itself is implemented as sigleton (derived interface from ArtDrv class)

// here we decide on the driver
// right now we attach the ArtemisDLL in Win and the SIO or USB in UX


#if defined(WIN32)
   EOSKind ArtBase::m_implementation = EOSK_Win32;
   ArtDrv* ArtBase::m_camCcdDrv = ArtDrvW32DLL::Instance();    // Artemis DLL driver Win32
//   ArtDrv* ArtBase::m_camHscDrv = ArtDrvHscW32DLL::Instance(); // Artemis DLL driver Win32
   ArtDrv* ArtBase::m_camHscDrv = NULL;
   ArtDrv* ArtBase::m_camSimDrv = ArtDrvSim::Instance();       // Simulation
#else
   EOSKind ArtBase::m_implementation = EOSK_Linux;
   ArtDrv* ArtBase::m_camCcdDrv = Atik16Drv::Instance();       // Artemis Driver Linux
//   ArtDrv* ArtBase::m_camHscDrv = ArtDrvEzFX2::Instance();   // Artemis EzFX2 driver Linux
   ArtDrv* ArtBase::m_camHscDrv = NULL;
   ArtDrv* ArtBase::m_camSimDrv = ArtDrvSim::Instance();       // Simulation
#endif

	size_t ArtBase::m_numCameras = 0;
   ArtDevice*      ArtBase::m_pCameras[ArtBase::MAXITEM] = {0};
	const ArtDevice ArtBase::m_nullCamera;  // a dummy cam entry
	ArtDevice       ArtBase::m_artCamera;   // a generic device

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Delete enumeration list
void ArtBase::DropAll()
{
	for (size_t i=0; i<MAXITEM; i++)
	{
		if (m_pCameras[i])
			delete m_pCameras[i]; m_pCameras[i] = NULL;
	}
	m_numCameras = 0;
}

// Re-Enumerate all devices
bool ArtBase::EnumerateArtCameras()
{
   DropAll();

   if (m_camCcdDrv) {
      // check ArtCCD devices
      for (size_t i=ACT_ArtCCD_FIRST; i<ACT_ArtCCD_LAST; i++) {
      // enumarate MAXITEM   FTDI devices and look which is a camera
         if ( m_camCcdDrv->DeviceIsCamera(i) ) {
            char devName[50]; devName[0] = 0;
            char devSerial[50]; devSerial[0] = 0;
            m_camCcdDrv->DeviceName(i, devName);  // min 40 length is required according to manual
            m_camCcdDrv->DeviceSerial(i, devSerial);

            wxString friendly_name = wxString::FromAscii((char*)devName);
            if (friendly_name.length()>0) {
               m_pCameras[m_numCameras] = new ArtDevice();
               wxString dev_serial = wxString::FromAscii((char*)devSerial);

               m_pCameras[m_numCameras]->Init(i, friendly_name, dev_serial, m_camCcdDrv);
               m_numCameras++;
            }
         }
      }//for
   }//endif

   if (m_camHscDrv) {
      // check ArtHSC devices
      for (size_t i=ACT_ArtHSC_FIRST; i<ACT_ArtHSC_LAST; i++) {
      // enumarate MAXITEM   FTDI devices and look which is a camera
         if ( m_camHscDrv->DeviceIsCamera(i) ) {
            char devName[50]; devName[0] = 0;
            char devSerial[50]; devSerial[0] = 0;
            m_camHscDrv->DeviceName(i, devName);  // min 40 length is required according to manual
            m_camHscDrv->DeviceSerial(i, devSerial);

            wxString friendly_name = wxString::FromAscii((char*)devName);
            if (friendly_name.length()>0) {
               m_pCameras[m_numCameras] = new ArtDevice();
               wxString dev_serial = wxString::FromAscii((char*)devSerial);

               m_pCameras[m_numCameras]->Init(i, friendly_name, dev_serial, m_camHscDrv);
               m_numCameras++;
            }
         }
      }//for
   }//endif

   if (m_camSimDrv) {
      // check ArtSIM devices
      for (size_t i=ACT_ArtSIM_FIRST; i<ACT_ArtSIM_LAST; i++) {
      // enumarate MAXITEM   FTDI devices and look which is a camera
         if ( m_camSimDrv->DeviceIsCamera(i) ) {
            char devName[50]; devName[0] = 0;
            char devSerial[50]; devSerial[0] = 0;
            m_camSimDrv->DeviceName(i, devName);  // min 40 length is required according to manual
            m_camSimDrv->DeviceSerial(i, devSerial);

            wxString friendly_name = wxString::FromAscii((char*)devName);
            if (friendly_name.length()>0) {
               m_pCameras[m_numCameras] = new ArtDevice();
               wxString dev_serial = wxString::FromAscii((char*)devSerial);

               m_pCameras[m_numCameras]->Init(i, friendly_name, dev_serial, m_camSimDrv);
               m_numCameras++;
            }
         }
      }//for
   }//endif
   return (true);
}

// Get a generic device descriptor of device cfIndex
const ArtDevice& ArtBase::ArtDevEntry(size_t cfIndex)
{
   if (cfIndex>=m_numCameras)
      return m_nullCamera;

   m_artCamera.Init(m_pCameras[cfIndex]->DevIndex(),
                    m_pCameras[cfIndex]->DevName(),
                    m_pCameras[cfIndex]->DevSerial(),
                    m_pCameras[cfIndex]->PCamDriver());
   return m_artCamera;
}



//! instance specific interfaces
// This is the constructor of a class that has been exported.
// see dscapture.h for the class definition
ArtBase::ArtBase()
: m_lastHResult(true)
, m_lastTemp(100)
, m_lastTempCall(0)
, m_lastInfo(COE_NULL_FLAG)
, m_lastInfoCall(0)
, m_chipset(CSX_Unknown)
{
   // Init the transfer sample
   m_lSample.ready = false;
   m_lSample.sampleDropped = true;
   m_lSample.fullFrame = false;
   m_lSample.binX = 1;  m_lSample.binY = 1;
   m_lSample.imageMem=NULL;
   m_lSample.dataLength = 0;

   // init the Artemis property record of the controlled cam
   m_artProps.Protocol = -1;
	m_artProps.nPixelsX = 0;        m_artProps.nPixelsY = 0;
	m_artProps.PixelMicronsX = 0.0; m_artProps.PixelMicronsY = 0.0;
	m_artProps.ccdflags = 0;        m_artProps.cameraflags = 0;
	m_artProps.Description[0] = 0;  m_artProps.Manufacturer[0] = 0;
}


ArtBase::~ArtBase()
{
   // Drop ownership
   DropControlledDevice();
   // Drop allocated sample space
   if (m_lSample.imageMem) delete [] m_lSample.imageMem;
}

// error tracking: return the last HRESULT from a call
bool ArtBase::LastHResult()
{
	return m_lastHResult;
}

wxString ArtBase::VideoTechnology()
{
	if (!m_cam.IsDeviceControlled()) {
	   return wxT("Unknown");
	}
	wxString vt;
	switch (m_cam.PCamDriver()->VideoTechnology()) {
	   case ArtDrv::EAV_ArtCcd_Dll: return wxT("ARTCCD");
	   case ArtDrv::EAV_ArtHsc_Dll: return wxT("ARTHSC");
	   case ArtDrv::EAV_ArtSio:     return wxT("ARTSIO");
	   case ArtDrv::EAV_ArtLibUsb:  return wxT("ARTUSB");
	   case ArtDrv::EAV_ArtSim:     return wxT("ARTSIM");
	}
   return wxT("Unknown");
}

// Enable Access to the device with cfIndex
// set to -1,0..n
//  0 .. n enable access to device m
bool ArtBase::ControlDevice(long cfIndex)
{
	// if we had one controlled before, delete it
   DropControlledDevice();

   // make use of it by submitting the controlled item
   if ( cfIndex>=long(m_numCameras) ) return false;

   // prepare the entry used to control the camera
   m_cam.Init(m_pCameras[cfIndex]->DevIndex(),
              m_pCameras[cfIndex]->DevName(),
              m_pCameras[cfIndex]->DevSerial(),
              m_pCameras[cfIndex]->PCamDriver());

   // take ownership by creating a cameradriver for the desired device
   m_cam.ControlDevice( m_cam.PCamDriver()->Connect(m_cam.DevIndex()));
   m_lastHResult = (m_cam.IsDeviceControlled() && m_cam.PCamDriver()->DiagnosticUSBError()==0 );   // succeeded ?

   if ( m_lastHResult &&  m_cam.PCamDriver()->IsConnected() ) {
      // init cam if it has been successfully connected
      m_cam.PCamDriver()->Properties(m_artProps);  // accord. to manual this has to be done first
  	   m_chipset = (HasCCDInterlaced()) ? CSX_ArtCCD_IL : CSX_ArtCCD_PX;
      m_cam.PCamDriver()->SetBin(1,1);
   }
   else {
      m_cam.PCamDriver()->ReconnectUSB(); // may be this helps the DLL to connnect
      m_artProps.Protocol = -1;
      m_lastHResult = false;
   }
   return m_lastHResult;
}

void ArtBase::DropControlledDevice()
{
	// if we had one controlled before, delete it
   if (m_cam.IsDeviceControlled()) {
      m_cam.PCamDriver()->AbortExposure();  // just in case
      m_cam.PCamDriver()->Disconnect();
      // set controlled cam to a NULL camera
      m_cam.ControlDevice(false);   // drop control over device, drops driver too
	   m_cam.Init(m_nullCamera.DevIndex(), m_nullCamera.DevName(), m_nullCamera.DevSerial(), m_nullCamera.PCamDriver());
	   m_chipset = CSX_Unknown;
   }
}


// Get a device descriptor of the controlled device
ArtDevice& ArtBase::ControlledArtDevEntry()
{
   return m_cam;
}

const EKnownChipsetCat  ArtBase::Chipset() const
{
   return m_chipset;
}


bool ArtBase::HasFifo()
{
   if (m_artProps.Protocol!=-1)
      return ( ARTEMIS_PROPERTIES_CAMERAFLAGS_FIFO
                  == (m_artProps.cameraflags & ARTEMIS_PROPERTIES_CAMERAFLAGS_FIFO) );
   else
      return false;
}

bool ArtBase::HasExternalTrigger()
{
   if (m_artProps.Protocol!=-1)
      return ( ARTEMIS_PROPERTIES_CAMERAFLAGS_EXT_TRIGGER
                  == (m_artProps.cameraflags & ARTEMIS_PROPERTIES_CAMERAFLAGS_EXT_TRIGGER) );
   else
      return false;
}

bool ArtBase::HasCCDInterlaced()
{
   if (m_artProps.Protocol!=-1)
      return ( ARTEMIS_PROPERTIES_CCDFLAGS_INTERLACED
                  == (m_artProps.ccdflags & ARTEMIS_PROPERTIES_CCDFLAGS_INTERLACED) );
   else
      return false;
}

// cooling support
bool ArtBase::HasCooling()
{
   if (m_artProps.Protocol!=-1)
      return ( ARTEMIS_PROPERTIES_CAMERAFLAGS_COOLING
                  == (m_artProps.cameraflags & ARTEMIS_PROPERTIES_CAMERAFLAGS_COOLING) );
   else
      return false;
}

bool  ArtBase::HasWarmup()
{
   if (m_artProps.Protocol!=-1)
      return ( ARTEMIS_PROPERTIES_CAMERAFLAGS_WARMUP
                  == (m_artProps.cameraflags & ARTEMIS_PROPERTIES_CAMERAFLAGS_WARMUP) );
   else
      return false;
}

ECoolingStatus ArtBase::CoolingStatus()
{
	if (!m_cam.IsDeviceControlled()) {
		return ECS_NotAvailable;
	}
	else if ( m_cam.PCamDriver()->DiagnosticUSBError()!=0 ) {
		return ECS_NotAvailable;
   }
   else {
      long t = ::wxGetLocalTime();
      if ((t-m_lastInfoCall)>=TEMP_INTERVAL) {
         // we shall not query the temperature too often !!
         // shares the calling pace with the other get info call
         int level, minlvl, maxlvl;
         m_cam.PCamDriver()->CoolingInfo(m_lastInfo, level, minlvl, maxlvl, m_lastSetpoint);
      }
      if (!(m_lastInfo&COE_CCooling))    return ECS_NotAvailable;
      else if (m_lastInfo&COE_SWarmUp)   return ECS_Warmup;
      else if (m_lastInfo&COE_SCooling)  {
         Temperature(); // just call in case it was not called shortly before
         if (abs(m_lastTemp-m_lastSetpoint)<=TEMP_OK_DELTA) return ECS_Holding;
         else                                               return ECS_Cooling;
      }
      else return ECS_Off;
   }
}

float ArtBase::CoolingSetpoint()
{
	if (!m_cam.IsDeviceControlled()) {
		return 99.9f;
	}
	else if ( m_cam.PCamDriver()->DiagnosticUSBError()!=0 ) {
		return 99.9f;
   }
   else {
      long t = ::wxGetLocalTime();
      if ((t-m_lastInfoCall)>=TEMP_INTERVAL) {
         // we shall not query the temperature too often !!
         // shares the calling pace with the other get info call
         int level, minlvl, maxlvl;
         m_cam.PCamDriver()->CoolingInfo(m_lastInfo, level, minlvl, maxlvl, m_lastSetpoint);
      }
      return (float)m_lastSetpoint / 100.0;
   }
}


float ArtBase::Temperature()
{
	if (!m_cam.IsDeviceControlled()) {
		return -99.0;
	}
	else if ( m_cam.PCamDriver()->DiagnosticUSBError()!=0 ) {
		return -99.0;
   }
   else {
      long t = ::wxGetLocalTime();
      if ((t-m_lastTempCall)>=TEMP_INTERVAL) {
         // we shall not query the temperature too often !!
         m_cam.PCamDriver()->TemperatureSensorInfo(1, m_lastTemp);
         m_lastTempCall = t;
      }
      return (float)m_lastTemp / 100.0;
   }
}

bool  ArtBase::SetTemperature(float temp)
{
	m_lastHResult = true; // init with OK
	if (!m_cam.IsDeviceControlled()) {
		m_lastHResult = false;
	}
	else if ( m_cam.PCamDriver()->DiagnosticUSBError()!=0 ) {
		m_lastHResult = false;
   }
   else {
      int sp = (int)(temp*100.0);
		int retVal = m_cam.PCamDriver()->SetCooling(sp);
      m_lastHResult = (ARTEMIS_OK==retVal);
   }
	return m_lastHResult;
}

bool  ArtBase::SetWarmup()
{
	m_lastHResult = true; // init with OK
	if (!m_cam.IsDeviceControlled()) {
		m_lastHResult = false;
	}
	else if ( m_cam.PCamDriver()->DiagnosticUSBError()!=0 ) {
		m_lastHResult = false;
   }
   else {
		int retVal = m_cam.PCamDriver()->CoolerWarmUp();
      m_lastHResult = (ARTEMIS_OK==retVal);
   }
	return m_lastHResult;
}


// init and capture an image
bool ArtBase::CaptureImage(bool WXUNUSED(enabled), unsigned long milliseconds)
{
	m_lastHResult = true; // init with OK

	if ( !m_cam.IsDeviceControlled() ) {
		m_lastHResult = false;
	}
	else if ( m_cam.PCamDriver()->DiagnosticUSBError()!=0 ) {
		m_lastHResult = false;
   }
   else if (-1==m_cam.PCamDriver()->DiagnosticPing(42) ) {
		m_lastHResult = false;
   }
   else {
      m_lSample.sampleDropped = false;
   	// Don't switch amplifier off for short exposures
   	m_cam.PCamDriver()->SetAmplifierSwitched((milliseconds>1000));
   	// Start the exposure
   	int retVal = m_cam.PCamDriver()->StartExposure(((float)milliseconds*0.001f));
      m_lastHResult = (ARTEMIS_OK==retVal);
   }
	return m_lastHResult;
}


// query the current type of streaming
const ARTEMISCAMERASTATE ArtBase::CaptureStatus() const
{
	if (!m_cam.IsDeviceControlled()) {
		return CAMERA_ERROR;
	}
	else if ( m_cam.PCamDriver()->DiagnosticUSBError()!=0 ) {
		return CAMERA_ERROR;
   }
   else {
      return (ARTEMISCAMERASTATE)m_cam.PCamDriver()->CameraState();
   }
}

// Return time remaining in current exposure, in seconds
void ArtBase::TimeRemaining(bool& exposing, float& timeRemaining)
{
   exposing = true; timeRemaining = 0.0;

	if (!m_cam.IsDeviceControlled()) {
		return;
	}
	else if ( m_cam.PCamDriver()->DiagnosticUSBError()!=0 ) {
		return;
   }
   else {
      timeRemaining = m_cam.PCamDriver()->ExposureTimeRemaining();
      // not longer exposing??
      if ( (timeRemaining<0000.1) && ( CAMERA_DOWNLOADING==m_cam.PCamDriver()->CameraState() ) ) {
         exposing=false;
         timeRemaining = (100 - m_cam.PCamDriver()->DownloadPercent());
         timeRemaining = (timeRemaining<0) ? 0 : timeRemaining; // we don't want neg numbers..
      }
      return;
   }
}

// Abort exposure, if one is in progress
void ArtBase::AbortExposure()
{
   DropCapturedSample();     // mark as dropped
	if (!m_cam.IsDeviceControlled()) {
		return;
	}
	else if ( m_cam.PCamDriver()->DiagnosticUSBError()!=0 ) {
		return;
   }
   m_cam.PCamDriver()->StopExposure(); // kill camera op
}


// fills the submitted sample object with data
void ArtBase::CapturedSample(wxArtSample& sample)
{
	// clear
	sample.Init(false,1,1,NULL,0.0);

	if (!m_cam.IsDeviceControlled()) {
	   sample.DropSample(); // mark dropped
		return;
	}
	else if ( m_cam.PCamDriver()->DiagnosticUSBError()!=0 ) {
	   sample.DropSample(); // mark dropped
		return;
   }
   else {
      // transfer sample from internal buffer to the outgoing buffer (copy right now)
      // also here the direction is CCD ie start is > end therefore -size+1
      if (m_lSample.ready ) {
         sample.Init(m_lSample.fullFrame, m_lSample.binX, m_lSample.binY,
                        m_lSample.imageMem, m_lSample.expotime/1000.0);
         // init detaches the current sample
         m_lSample.imageMem = NULL;
      }
      if (m_lSample.sampleDropped) sample.DropSample(); // mark dropped if needed
      return;
   }
}


// just reply to ignore that sample and continue
void ArtBase::DropCapturedSample()
{
   m_lSample.ready = false;
   m_lSample.sampleDropped = true;      // mark as dropped
}

// query the camera video ccd original dimension (BIN=1)
void ArtBase::CcdDimension(wxUint16& width, wxUint16& height) const
{
   width = m_artProps.nPixelsX; height = m_artProps.nPixelsY;
}

// query the camera video frame original dimension (BIN=1)
void ArtBase::CamDimension(wxUint16& width, wxUint16& height) const
{
	if (!m_cam.IsDeviceControlled()) {
		return;
	}
	else if ( m_cam.PCamDriver()->DiagnosticUSBError()!=0 ) {
		return;
   }
   else {
      wxUint16 x,y,wid,hgt;
      m_cam.PCamDriver()->Subframe(x, y, wid, hgt);
      // acc. to manual the dimensions are in real camera pixels i.e. before binning
      width = wid;
      height = hgt;
   }
}


// return true pixels of the sample
void ArtBase::FrameDimension(wxUint16& width, wxUint16& height) const
{
	width = 0, height = 0;
	if (!m_cam.IsDeviceControlled()) {
		return;
	}
	else if ( m_cam.PCamDriver()->DiagnosticUSBError()!=0 ) {
		return;
   }
   else {
      wxUint16 x,y,wid,hgt,binx,biny;
      m_cam.PCamDriver()->Subframe(x, y, wid, hgt);
      m_cam.PCamDriver()->Bin(binx, biny);
      // acc. to manual the dimensions are in real camera pixels i.e. before binning
      width = wid / binx;
      height = hgt / biny;
   }
}

// Set the camera binning format
bool ArtBase::SetFormat(wxUint16 binning)
{
	m_lastHResult = true; // init with OK

	if (!m_cam.IsDeviceControlled()) {
		m_lastHResult = false;
	}
	else if ( m_cam.PCamDriver()->DiagnosticUSBError()!=0 ) {
		m_lastHResult = false;
   }
   else {
      wxUint16 realBin = (binning>MaxFormat()) ? MaxFormat() : binning;
      if ( (Chipset()==CSX_ArtCCD_IL) && (realBin==3 || realBin==5) ) realBin--; // Interlaced cannot odd binning
      int retVal = m_cam.PCamDriver()->SetBin(realBin, realBin);
      m_lastHResult = (ARTEMIS_OK==retVal);
   }

	return m_lastHResult;
}

wxUint16 ArtBase::MaxFormat()
{
	if (!m_cam.IsDeviceControlled()) {
		return 1;
	}
	wxUint16 y,x, maxB;
	m_cam.PCamDriver()->MaxBin(x,y);
	maxB = (x>y)?y:x;
   return maxB;
}

wxUint16 ArtBase::BinFormat()
{
	if (!m_cam.IsDeviceControlled()) {
		return 1;
	}
	wxUint16 y,x, maxB;
	m_cam.PCamDriver()->Bin(x,y);
	maxB = (x>y)?y:x;
   return maxB;
}

// set the subframe format (binning factor 1 assumed)
//  needs two points of a rectangle that makes the subframe in pixel coords
//  the subframe might get adjusted if binning changes
//  switching back to the max frame is done by setting all points to 1
bool ArtBase::SetSubframe(wxUint16 startX, wxUint16 startY,
                              wxUint16 endX, wxUint16 endY)
{
	m_lastHResult = true; // init with OK
   int retVal;

	if (!m_cam.IsDeviceControlled()) {
		m_lastHResult = false;
	}
	else if ( m_cam.PCamDriver()->DiagnosticUSBError()!=0 ) {
		m_lastHResult = false;
   }
   else {
      m_lSample.fullFrame=false;
      // the subframe on the CCD is mirrored
      // i.e. startX is bottom right and therefore usually > endX
      // we have to supply a positve oriented subframe with point and width
      // this reorientation is done here
      wxUint16 ccdsx, ccdsy, ccdw, ccdh;
      // handle reversed boxes
      if (startX<endX) {
         ccdsx = startX; ccdw = endX - startX +1; // pt inclusive
         // this happens sometimes...
         ccdw=2; // bail out with too small rectangle
      }
      else {
         ccdsx = endX; ccdw = startX - endX +1; // pt inclusive
      }
      if (startY<endY) {
         ccdsy = startY; ccdh = endY - startY + 1; // pt inclusive
         // this happens sometimes...
         ccdh=2; // bail out with too small rectangle
      }
      else {
         ccdsy = endY; ccdh = startY - endY + 1; // pt inclusive
      }
      // handle max format box (all dims are equal == 1 pixel image)
      ARTEMISPROPERTIES prop;
      m_cam.PCamDriver()->Properties(prop);
      if (ccdw==1 && ccdh==1) {
         ccdsx=0; ccdsy=0; ccdw=prop.nPixelsX; ccdh=prop.nPixelsY;
         m_lSample.fullFrame=true;
      }
      // handle too small size boxes
      if (ccdw<10 || ccdh<10) {
         m_lastHResult = false;
      }
      else {
         // seems OK - send values
         retVal = m_cam.PCamDriver()->SetSubframe(ccdsx, ccdsy, ccdw, ccdh);
         m_lSample.fullFrame=(m_lSample.fullFrame || ( (ccdw==prop.nPixelsX) && (ccdh==prop.nPixelsY) ));
         m_lastHResult = (ARTEMIS_OK==retVal);
      }
   }

	return m_lastHResult;
}


// returns true if a an OnCapture call is expected
// supports non event driven apps i.e. EventSink is not provided
ESYNMode ArtBase::OnCaptureRequired()
{
	if (!m_cam.IsDeviceControlled()) {
		return ESYN_None;
	}
   else if ( CAMERA_ERROR==m_cam.PCamDriver()->CameraState() ) {
		return ESYN_None;
   }
	else if ( m_cam.PCamDriver()->ImageReady() ) {
      // image is ready...
      if ( CAMERA_DOWNLOADING==m_cam.PCamDriver()->CameraState() ) {
         // but still downloading
         return ESYN_Tick;
      }
      else {
         // download required
         return ESYN_All;
      }
	}
	else {
      // just a simple tick from the timer
		return ESYN_Tick;
	}
}


// must be called to gather the sampled image when polled via OnCaptureRequired
//
// OnCapture returns 0 if a new sample must be collected
// positive numbers are capture events left until finished (not used for ArtCam)
// negative numbers are error indications
long ArtBase::OnCapture()
{
   m_lSample.ready = false;
	m_lastHResult = true; // init with OK

	if (!m_cam.IsDeviceControlled()) {
      m_lastHResult = false;
		return -3;  // ERROR EXIT - no handle
	}
	else if ( m_cam.PCamDriver()->DiagnosticUSBError()!=0 ) {
      m_lastHResult = false;
		return -4;  // ERROR EXIT - usb error
   }
	else {
   	// Get dimensions of image
   	wxUint16 x,y,wid,hgt,binx,biny;
   	m_cam.PCamDriver()->ImageData(x, y, wid, hgt, binx, biny);
      //--->  x,y are CCD pixel, w,h are binned pixel dimensions of image
      // Original Artemis API is defined as such

      const wxUint16* pimg = m_cam.PCamDriver()->ImageBuffer();
   	// acc. to manual the dimensions are in real camera pixels i.e. before binning
   	// the sample contains the real image dimensions

      //! One and only place to create the new Image Memory
      m_lSample.imageMem = new WordImageMem();
   	// as we get the image back the direction is still inverse due to the mirroring
   	// and our correction in SetSubframe
   	// to allow coordinate transform we will feedback the expected values
   	// and inverse the direction again (start is end and via versa)
   	m_lSample.imageMem->SetFrameRect(wxRect(x/binx+wid-1, y/biny+hgt-1, wid, hgt));
      m_lSample.binX = binx;      m_lSample.binY = biny;
      m_lSample.expotime = (unsigned long)(m_cam.PCamDriver()->LastExposureDuration()*1000.0);
      // load image from camera
      if (pimg)
      {
         size_t imgsize = wid * hgt;
         // image is in CCD readout order i.e. reversed
         m_lSample.imageMem->CopyFrom(pimg, imgsize, GenericImageMemBase::EYM_Reverse);
         m_lSample.dataLength = m_lSample.imageMem->ByteSize();
         m_lSample.ready = m_lSample.imageMem->MemOK();;
         return 0;  // OK EXIT
      }
	}

   m_lastHResult = false;
	if ( m_cam.PCamDriver()->CameraState()==CAMERA_ERROR ) {
      return -1;  // ERROR EXIT - CAMERA ERROR
	}
	else {
      return -2;  // ERROR EXIT - could not catch image
	}
}

