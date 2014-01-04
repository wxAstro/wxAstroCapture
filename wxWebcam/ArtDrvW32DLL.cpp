
#include "ArtDrvW32DLL.h"

// Artemis Windows DLL interface
#include "ArtemisCCDAPI.h"
#define ARTEMISDLLNAME "ArtemisCCD.dll"

ArtDrvW32DLL* ArtDrvW32DLL::pinstance = 0;// initialize pointer
ArtDrvW32DLL* ArtDrvW32DLL::Instance ()
{
 if (pinstance == 0)  // is it the first call?
 {
   pinstance = new ArtDrvW32DLL(); // create sole instance
 }
 return pinstance; // address of sole instance
}


// connects to first available
ArtDrvW32DLL::ArtDrvW32DLL()
: m_dllLoaded(false)
, m_hCam(NULL)
{
	if (!::ArtemisLoadDLL(ARTEMISDLLNAME))
	{
		return;  // ERROR EXIT
	}
	m_dllLoaded = true;
}

ArtDrvW32DLL::~ArtDrvW32DLL()
{
   if (m_dllLoaded) ::ArtemisUnLoadDLL();
}


// Get USB Identifier of Nth USB device. Return false if no such device.
// pName must be at least 40 chars long.
bool ArtDrvW32DLL::DeviceName(unsigned short Device, char *pName)
{
   if ( !m_dllLoaded ) return false; // bail if no DLL

   return ::ArtemisDeviceName((int)Device, pName);
}

// Get USB Serial number of Nth USB device. Return false if no such device.
// pName must be at least 40 chars long.
bool ArtDrvW32DLL::DeviceSerial(unsigned short Device, char *pName)
{
   if ( !m_dllLoaded ) return false; // bail if no DLL

   return ::ArtemisDeviceSerial((int)Device, pName);
}

// Return true if Nth USB device exists and is a camera.
bool ArtDrvW32DLL::DeviceIsCamera(unsigned short Device)
{
   if ( !m_dllLoaded ) return false; // bail if no DLL

   return ::ArtemisDeviceIsCamera((int)Device);
}

// Connect to given device. If Device=-1, connect to first available
// Returns handle if connected as requested, else NULL
bool ArtDrvW32DLL::Connect(unsigned short Device)
{
   if ( !m_dllLoaded ) return false; // bail if no DLL

	// when the cam queue is still full we may have to try several times to connect
   int errCnt=10;
   do {
	   m_hCam = ::ArtemisConnect((int)Device);
	   if (!m_hCam) {
         errCnt--;
         ReconnectUSB();
         Sleep(250); // wait a while
	   }
   }while(errCnt>0 && m_hCam==NULL);

   return IsConnected();
}

ARTEMISERROR ArtDrvW32DLL::ReconnectUSB()
{
   if ( !m_dllLoaded ) return ARTEMIS_NOT_CONNECTED; // bail if no DLL
   if ( !m_hCam ) return ARTEMIS_NOT_CONNECTED; // bail if no cam at all

   return (ARTEMISERROR)::ArtemisReconnectUSB(m_hCam);
}


// Return the last FT USB error condition seen
// Calling this function clears the internal error state (FT_OK=0)
int ArtDrvW32DLL::DiagnosticUSBError()
{
   if ( !m_dllLoaded ) return 1; //FT_INVALID_HANDLE; // bail if no DLL
   if ( !m_hCam ) return 1; //FT_INVALID_HANDLE; // bail if no cam at all

   return ::ArtemisDiagnosticUSBError(m_hCam);
}

// Ping the camera, return the result. -1 on error.
int ArtDrvW32DLL::DiagnosticPing(int send)
{
   if ( !m_dllLoaded ) return -1;
   if ( !m_hCam ) return -1;

   return ::ArtemisDiagnosticPing(m_hCam, send);
}



// Disconnect from given device.
// Returns true if disconnected as requested
bool ArtDrvW32DLL::Disconnect()
{
   if ( !m_dllLoaded ) return false; // bail if no DLL
   if ( !m_hCam ) return false; // bail if no cam at all

   bool retVal = ::ArtemisDisconnect(m_hCam);
   m_hCam = NULL;
   return retVal;
}


// Returns true if currently connected to a device
bool ArtDrvW32DLL::IsConnected() const
{
   if ( !m_dllLoaded ) return false; // bail if no DLL
   if ( !m_hCam ) return false; // bail if no cam at all

   return ::ArtemisIsConnected(m_hCam);
}

// Retrieve the current camera state
ARTEMISCAMERASTATE ArtDrvW32DLL::CameraState() const
{
   if ( !m_dllLoaded ) return CAMERA_ERROR; // bail if no DLL
   if ( !m_hCam ) return CAMERA_ERROR; // bail if no cam at all

   return (ARTEMISCAMERASTATE)::ArtemisCameraState(m_hCam);
}

// Fills in pProp with camera properties
ARTEMISERROR ArtDrvW32DLL::Properties(ARTEMISPROPERTIES &pProp)
{
   if ( !m_dllLoaded ) return ARTEMIS_NOT_CONNECTED; // bail if no DLL
   if ( !m_hCam ) return ARTEMIS_NOT_CONNECTED; // bail if no cam at all

   return (ARTEMISERROR)::ArtemisProperties(m_hCam, &pProp);
}

// Set download thread to high or normal priority
ARTEMISERROR ArtDrvW32DLL::HighPriority(bool bHigh)
{
   if ( !m_dllLoaded ) return ARTEMIS_NOT_CONNECTED; // bail if no DLL
   if ( !m_hCam ) return ARTEMIS_NOT_CONNECTED; // bail if no cam at all

   return (ARTEMISERROR)::ArtemisHighPriority(m_hCam, bHigh);
}

// Get the pos and size of imaging subframe
void ArtDrvW32DLL::Subframe(unsigned short &x, unsigned short &y, unsigned short &w, unsigned short &h) const
{
   if ( !m_dllLoaded ) return; // bail if no DLL
   if ( !m_hCam ) return; // bail if no cam at all

   int ix=x, iy=y, iw=w, ih=h;
   ::ArtemisGetSubframe(m_hCam, &ix, &iy, &iw, &ih);
   x=(unsigned short)ix; y=(unsigned short)iy; w=(unsigned short)iw; h=(unsigned short)ih;
}

// set the pos and size of imaging subframe inunbinned coords
ARTEMISERROR ArtDrvW32DLL::SetSubframe(unsigned short x, unsigned short y, unsigned short w, unsigned short h)
{
   if ( !m_dllLoaded ) return ARTEMIS_NOT_CONNECTED; // bail if no DLL
   if ( !m_hCam ) return ARTEMIS_NOT_CONNECTED; // bail if no cam at all

   return (ARTEMISERROR)::ArtemisSubframe(m_hCam, (int)x, (int)y, (int)w, (int)h);
}

// Set the start x,y coords for imaging subframe., X,Y in unbinned coordinates
ARTEMISERROR ArtDrvW32DLL::SetSubframePos(unsigned short x, unsigned short y)
{
   if ( !m_dllLoaded ) return ARTEMIS_NOT_CONNECTED; // bail if no DLL
   if ( !m_hCam ) return ARTEMIS_NOT_CONNECTED; // bail if no cam at all

   return (ARTEMISERROR)::ArtemisSubframePos(m_hCam, (int)x, (int)y);
}

// Set the width and height of imaging subframe, W,H in unbinned coordinates
ARTEMISERROR ArtDrvW32DLL::SetSubframeSize(unsigned short w, unsigned short h)
{
   if ( !m_dllLoaded ) return ARTEMIS_NOT_CONNECTED; // bail if no DLL
   if ( !m_hCam ) return ARTEMIS_NOT_CONNECTED; // bail if no cam at all

   return (ARTEMISERROR)::ArtemisSubframeSize(m_hCam, (int)w, (int)h);
}

// Get the x,y binning factors
void ArtDrvW32DLL::Bin(unsigned short& x, unsigned short& y) const
{
   if ( !m_dllLoaded ) return; // bail if no DLL
   if ( !m_hCam ) return; // bail if no cam at all

   int ix=x, iy=y;
   ::ArtemisGetBin(m_hCam, &ix, &iy);
   x=(unsigned short)ix; y=(unsigned short)iy;
}

// Set the x,y binning factors
ARTEMISERROR ArtDrvW32DLL::SetBin(unsigned short x, unsigned short y)
{
   if ( !m_dllLoaded ) return ARTEMIS_NOT_CONNECTED; // bail if no DLL
   if ( !m_hCam ) return ARTEMIS_NOT_CONNECTED; // bail if no cam at all

   return (ARTEMISERROR)::ArtemisBin(m_hCam, (int)x, (int)y);
}

// Get the max x,y binning factors
void ArtDrvW32DLL::MaxBin(unsigned short& x, unsigned short& y)
{
   x=4; y=4; // DLL cannot reply this values
}

// Return true if amp switched off during exposures
bool ArtDrvW32DLL::AmplifierSwitched() const
{
   if ( !m_dllLoaded ) return false; // bail if no DLL
   if ( !m_hCam ) return false; // bail if no cam at all

   return (ARTEMISERROR)::ArtemisGetAmplifierSwitched(m_hCam);
}

// Set whether amp is switched off during exposures
ARTEMISERROR ArtDrvW32DLL::SetAmplifierSwitched(bool bSwitched)
{
   if ( !m_dllLoaded ) return ARTEMIS_NOT_CONNECTED; // bail if no DLL
   if ( !m_hCam ) return ARTEMIS_NOT_CONNECTED; // bail if no cam at all

   return (ARTEMISERROR)::ArtemisSetAmplifierSwitched(m_hCam, bSwitched);
}

// Set whether amp is switched off during exposures
ARTEMISERROR ArtDrvW32DLL::SetAmp(bool bOn)
{
   if ( !m_dllLoaded ) return ARTEMIS_NOT_CONNECTED; // bail if no DLL
   if ( !m_hCam ) return ARTEMIS_NOT_CONNECTED; // bail if no cam at all

   return (ARTEMISERROR)::ArtemisAmplifier(m_hCam, bOn);
}

// Return duration of last exposure, in seconds
float ArtDrvW32DLL::LastExposureDuration() const
{
   if ( !m_dllLoaded ) return 0.0; // bail if no DLL
   if ( !m_hCam ) return 0.0; // bail if no cam at all

   return ::ArtemisLastExposureDuration(m_hCam);
}

// Start an exposure
ARTEMISERROR ArtDrvW32DLL::StartExposure(float Seconds)
{
   if ( !m_dllLoaded ) return ARTEMIS_NOT_CONNECTED; // bail if no DLL
   if ( !m_hCam ) return ARTEMIS_NOT_CONNECTED; // bail if no cam at all

   // workaround DLL bug
   int x,y,w,h,binx,biny;
   ::ArtemisGetSubframe(m_hCam, &x, &y, &w, &h);
   ::ArtemisGetBin(m_hCam, &binx, &biny);
   long imageSize = 2 * w * h / binx / biny;
   if (Seconds>2.4)
      m_msDLtimeExpected = imageSize / 230; // we assume DLspeed of 280 bytes/ms (280kB/sec) for expo>2.4sec
   else
      m_msDLtimeExpected = imageSize / 230 + Seconds*1500; // double exposure takes longer

   m_msDLtimeExpected = (m_msDLtimeExpected<=0) ? 1000 : m_msDLtimeExpected; // Div0 safety
   m_msStartDLtime = ::wxGetLocalTimeMillis();
   m_msStartDLtime = m_msStartDLtime + (1000*Seconds); // start DL time
   return (ARTEMISERROR)::ArtemisStartExposure(m_hCam, Seconds);
}

// Return time remaining in current exposure, in seconds
float ArtDrvW32DLL::ExposureTimeRemaining() const
{
   if ( !m_dllLoaded ) return 0.0; // bail if no DLL
   if ( !m_hCam ) return 0.0; // bail if no cam at all

   return ::ArtemisExposureTimeRemaining(m_hCam);
}

// Percentage downloaded
int ArtDrvW32DLL::DownloadPercent() const
{
   if ( !m_dllLoaded ) return 0; // bail if no DLL
   if ( !m_hCam ) return 0; // bail if no cam at all

   int dLpercent = ::ArtemisDownloadPercent(m_hCam);
   if (dLpercent==0) {
      // Art reports nothing DL so far
       wxLongLong now = ::wxGetLocalTimeMillis(); // hold current time
       long msCurrentDLtime = (now - m_msStartDLtime).ToLong(); // ms since DL started
       msCurrentDLtime = (msCurrentDLtime<0) ? 0 : msCurrentDLtime;
       dLpercent = (int)((float)msCurrentDLtime / (float)m_msDLtimeExpected * 100.0);
   }
   return dLpercent;
}

// Prematurely end an exposure, collecting image data.
ARTEMISERROR ArtDrvW32DLL::StopExposure()
{
   if ( !m_dllLoaded ) return ARTEMIS_NOT_CONNECTED; // bail if no DLL
   if ( !m_hCam ) return ARTEMIS_NOT_CONNECTED; // bail if no cam at all

   return (ARTEMISERROR)::ArtemisStopExposure(m_hCam);
}

// Abort exposure, if one is in progress
ARTEMISERROR ArtDrvW32DLL::AbortExposure()
{
   if ( !m_dllLoaded ) return ARTEMIS_NOT_CONNECTED; // bail if no DLL
   if ( !m_hCam ) return ARTEMIS_NOT_CONNECTED; // bail if no cam at all

   return (ARTEMISERROR)::ArtemisAbortExposure(m_hCam);
}

// Return true if an image is ready to be retrieved
bool ArtDrvW32DLL::ImageReady() const
{
   if ( !m_dllLoaded ) return false; // bail if no DLL
   if ( !m_hCam ) return false; // bail if no cam at all

   return ::ArtemisImageReady(m_hCam);
}


// Retrieve image dimensions and binning factors.
// x,y are actual CCD locations. w,h are pixel dimensions of image
void ArtDrvW32DLL::ImageData(unsigned short &x, unsigned short &y,
                                  unsigned short &w, unsigned short &h,
                                  unsigned short &binx, unsigned short &biny)
{
   if ( !m_dllLoaded ) return; // bail if no DLL
   if ( !m_hCam ) return; // bail if no cam at all

   int ix=x, iy=y, iw=w, ih=h, ibx=binx, iby=biny;
   ::ArtemisGetImageData(m_hCam, &ix, &iy, &iw, &ih, &ibx, &iby);
   x=(unsigned short)ix; y=(unsigned short)iy; w=(unsigned short)iw; h=(unsigned short)ih;
   binx=(unsigned short)ibx; biny=(unsigned short)iby;
}

// Return pointer to internal image buffer (actually wxUint16s)
const unsigned short* ArtDrvW32DLL::ImageBuffer() const
{
   if ( !m_dllLoaded ) return NULL; // bail if no DLL
   if ( !m_hCam ) return NULL; // bail if no cam at all

   return (unsigned short*) ::ArtemisImageBuffer(m_hCam);
}



// cooling support
void ArtDrvW32DLL::TemperatureSensorInfo(unsigned short WXUNUSED(sensor), int& temperature) // °C *100
{
   temperature=-990;
}

void ArtDrvW32DLL::CoolingInfo(ECamCoolingInfo& flags, int& WXUNUSED(level), int& WXUNUSED(minlvl), int& WXUNUSED(maxlvl), int& WXUNUSED(setpoint))
{
   flags=(ECamCoolingInfo)0; // returns no cooling
}

ARTEMISERROR ArtDrvW32DLL::SetCooling(int WXUNUSED(setpoint))
{
   return ARTEMIS_NOT_IMPLEMENTED;
}

ARTEMISERROR ArtDrvW32DLL::SetCooling(double WXUNUSED(temperature))
{
   return ARTEMIS_NOT_IMPLEMENTED;
}

ARTEMISERROR ArtDrvW32DLL::CoolerWarmUp()
{
   return ARTEMIS_NOT_IMPLEMENTED;
}
