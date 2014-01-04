
#include "ArtDrvHscW32DLL.h"

// Artemis Windows DLL interface
#include "ArtemisHSCAPI.h"
#define ARTEMISDLLNAME "ArtemisHSC.dll"

ArtDrvHscW32DLL* ArtDrvHscW32DLL::pinstance = 0;// initialize pointer
ArtDrvHscW32DLL* ArtDrvHscW32DLL::Instance ()
{
 if (pinstance == 0)  // is it the first call?
 {
   pinstance = new ArtDrvHscW32DLL(); // create sole instance
 }
 return pinstance; // address of sole instance
}


// connects to first available
ArtDrvHscW32DLL::ArtDrvHscW32DLL()
: m_dllLoaded(false)
, m_hCam(NULL)
{
	if (!::ArtemisHscLoadDLL(ARTEMISDLLNAME))
	{
		return;  // ERROR EXIT
	}
	m_dllLoaded = true;
}

ArtDrvHscW32DLL::~ArtDrvHscW32DLL()
{
   if (m_dllLoaded) ::ArtemisHscUnLoadDLL();
}


// Get USB Identifier of Nth USB device. Return false if no such device.
// pName must be at least 40 chars long.
bool ArtDrvHscW32DLL::DeviceName(unsigned short Device, char *pName)
{
   if ( !m_dllLoaded ) return false; // bail if no DLL

   return ::ArtemisHscDeviceName((int)Device, pName);
}

// Get USB Serial number of Nth USB device. Return false if no such device.
// pName must be at least 40 chars long.
bool ArtDrvHscW32DLL::DeviceSerial(unsigned short Device, char *pName)
{
   if ( !m_dllLoaded ) return false; // bail if no DLL

   return ::ArtemisHscDeviceSerial((int)Device, pName);
}

// Return true if Nth USB device exists and is a camera.
bool ArtDrvHscW32DLL::DeviceIsCamera(unsigned short Device)
{
   if ( !m_dllLoaded ) return false; // bail if no DLL

   return ::ArtemisHscDeviceIsCamera((int)Device);
}

// Connect to given device. If Device=-1, connect to first available
// Returns handle if connected as requested, else NULL
bool ArtDrvHscW32DLL::Connect(unsigned short Device)
{
   if ( !m_dllLoaded ) return false; // bail if no DLL

	// when the cam queue is still full we may have to try several times to connect
   int errCnt=10;
   do {
	   m_hCam = ::ArtemisHscConnect((int)Device);
	   if (!m_hCam) {
         errCnt--;
         ReconnectUSB();
         Sleep(250); // wait a while
	   }
   }while(errCnt>0 && m_hCam==NULL);

   return IsConnected();
}

ARTEMISERROR ArtDrvHscW32DLL::ReconnectUSB()
{
   if ( !m_dllLoaded ) return ARTEMIS_NOT_CONNECTED; // bail if no DLL
   if ( !m_hCam ) return ARTEMIS_NOT_CONNECTED; // bail if no cam at all

   return (ARTEMISERROR)::ArtemisHscReconnectUSB(m_hCam);
}


// Return the last FT USB error condition seen
// Calling this function clears the internal error state (FT_OK=0)
int ArtDrvHscW32DLL::DiagnosticUSBError()
{
   if ( !m_dllLoaded ) return 1; //FT_INVALID_HANDLE; // bail if no DLL
   if ( !m_hCam ) return 1; //FT_INVALID_HANDLE; // bail if no cam at all

   return ::ArtemisHscDiagnosticUSBError(m_hCam);
}

// Ping the camera, return the result. -1 on error.
int ArtDrvHscW32DLL::DiagnosticPing(int send)
{
   if ( !m_dllLoaded ) return -1;
   if ( !m_hCam ) return -1;

   return ::ArtemisHscDiagnosticPing(m_hCam, send);
}



// Disconnect from given device.
// Returns true if disconnected as requested
bool ArtDrvHscW32DLL::Disconnect()
{
   if ( !m_dllLoaded ) return false; // bail if no DLL
   if ( !m_hCam ) return false; // bail if no cam at all

   bool retVal = ::ArtemisHscDisconnect(m_hCam);
   m_hCam = NULL;
   return retVal;
}


// Returns true if currently connected to a device
bool ArtDrvHscW32DLL::IsConnected() const
{
   if ( !m_dllLoaded ) return false; // bail if no DLL
   if ( !m_hCam ) return false; // bail if no cam at all

   return ::ArtemisHscIsConnected(m_hCam);
}

// Retrieve the current camera state
ARTEMISCAMERASTATE ArtDrvHscW32DLL::CameraState() const
{
   if ( !m_dllLoaded ) return CAMERA_ERROR; // bail if no DLL
   if ( !m_hCam ) return CAMERA_ERROR; // bail if no cam at all

   return (ARTEMISCAMERASTATE)::ArtemisHscCameraState(m_hCam);
}

// Fills in pProp with camera properties
ARTEMISERROR ArtDrvHscW32DLL::Properties(ARTEMISPROPERTIES &pProp)
{
   if ( !m_dllLoaded ) return ARTEMIS_NOT_CONNECTED; // bail if no DLL
   if ( !m_hCam ) return ARTEMIS_NOT_CONNECTED; // bail if no cam at all

   return (ARTEMISERROR)::ArtemisHscProperties(m_hCam, &pProp);
}

// Set download thread to high or normal priority
ARTEMISERROR ArtDrvHscW32DLL::HighPriority(bool bHigh)
{
   if ( !m_dllLoaded ) return ARTEMIS_NOT_CONNECTED; // bail if no DLL
   if ( !m_hCam ) return ARTEMIS_NOT_CONNECTED; // bail if no cam at all

   return (ARTEMISERROR)::ArtemisHscHighPriority(m_hCam, bHigh);
}

// Get the pos and size of imaging subframe
void ArtDrvHscW32DLL::Subframe(unsigned short &x, unsigned short &y, unsigned short &w, unsigned short &h) const
{
   if ( !m_dllLoaded ) return; // bail if no DLL
   if ( !m_hCam ) return; // bail if no cam at all

   int ix=x, iy=y, iw=w, ih=h;
   ::ArtemisHscGetSubframe(m_hCam, &ix, &iy, &iw, &ih);
   x=(unsigned short)ix; y=(unsigned short)iy; w=(unsigned short)iw; h=(unsigned short)ih;
}

// set the pos and size of imaging subframe inunbinned coords
ARTEMISERROR ArtDrvHscW32DLL::SetSubframe(unsigned short x, unsigned short y, unsigned short w, unsigned short h)
{
   if ( !m_dllLoaded ) return ARTEMIS_NOT_CONNECTED; // bail if no DLL
   if ( !m_hCam ) return ARTEMIS_NOT_CONNECTED; // bail if no cam at all

   return (ARTEMISERROR)::ArtemisHscSubframe(m_hCam, (int)x, (int)y, (int)w, (int)h);
}

// Set the start x,y coords for imaging subframe., X,Y in unbinned coordinates
ARTEMISERROR ArtDrvHscW32DLL::SetSubframePos(unsigned short x, unsigned short y)
{
   if ( !m_dllLoaded ) return ARTEMIS_NOT_CONNECTED; // bail if no DLL
   if ( !m_hCam ) return ARTEMIS_NOT_CONNECTED; // bail if no cam at all

   return (ARTEMISERROR)::ArtemisHscSubframePos(m_hCam, (int)x, (int)y);
}

// Set the width and height of imaging subframe, W,H in unbinned coordinates
ARTEMISERROR ArtDrvHscW32DLL::SetSubframeSize(unsigned short w, unsigned short h)
{
   if ( !m_dllLoaded ) return ARTEMIS_NOT_CONNECTED; // bail if no DLL
   if ( !m_hCam ) return ARTEMIS_NOT_CONNECTED; // bail if no cam at all

   return (ARTEMISERROR)::ArtemisHscSubframeSize(m_hCam, (int)w, (int)h);
}

// Get the x,y binning factors
void ArtDrvHscW32DLL::Bin(unsigned short& x, unsigned short& y) const
{
   if ( !m_dllLoaded ) return; // bail if no DLL
   if ( !m_hCam ) return; // bail if no cam at all

   int ix=x, iy=y;
   ::ArtemisHscGetBin(m_hCam, &ix, &iy);
   x=(unsigned short)ix; y=(unsigned short)iy;
}

// Set the x,y binning factors
ARTEMISERROR ArtDrvHscW32DLL::SetBin(unsigned short x, unsigned short y)
{
   if ( !m_dllLoaded ) return ARTEMIS_NOT_CONNECTED; // bail if no DLL
   if ( !m_hCam ) return ARTEMIS_NOT_CONNECTED; // bail if no cam at all

   return (ARTEMISERROR)::ArtemisHscBin(m_hCam, (int)x, (int)y);
}

// Get the max x,y binning factors
void ArtDrvHscW32DLL::MaxBin(unsigned short& x, unsigned short& y)
{
   if ( !m_dllLoaded ) return; // bail if no DLL
   if ( !m_hCam ) return; // bail if no cam at all
   int xb=x, yb=y;
   ::ArtemisHscGetMaxBin(m_hCam, &xb, &yb);
   x=(unsigned short)xb; y=(unsigned short)yb;
}

// Return true if amp switched off during exposures
bool ArtDrvHscW32DLL::AmplifierSwitched() const
{
   if ( !m_dllLoaded ) return false; // bail if no DLL
   if ( !m_hCam ) return false; // bail if no cam at all

   return (ARTEMISERROR)::ArtemisHscGetAmplifierSwitched(m_hCam);
}

// Set whether amp is switched off during exposures
ARTEMISERROR ArtDrvHscW32DLL::SetAmplifierSwitched(bool bSwitched)
{
   if ( !m_dllLoaded ) return ARTEMIS_NOT_CONNECTED; // bail if no DLL
   if ( !m_hCam ) return ARTEMIS_NOT_CONNECTED; // bail if no cam at all

   return (ARTEMISERROR)::ArtemisHscSetAmplifierSwitched(m_hCam, bSwitched);
}

// Set whether amp is switched off during exposures
ARTEMISERROR ArtDrvHscW32DLL::SetAmp(bool bOn)
{
   if ( !m_dllLoaded ) return ARTEMIS_NOT_CONNECTED; // bail if no DLL
   if ( !m_hCam ) return ARTEMIS_NOT_CONNECTED; // bail if no cam at all

   return (ARTEMISERROR)::ArtemisHscAmplifier(m_hCam, bOn);
}


// Return duration of last exposure, in seconds
float ArtDrvHscW32DLL::LastExposureDuration() const
{
   if ( !m_dllLoaded ) return 0.0; // bail if no DLL
   if ( !m_hCam ) return 0.0; // bail if no cam at all

   return ::ArtemisHscLastExposureDuration(m_hCam);
}

// Start an exposure
ARTEMISERROR ArtDrvHscW32DLL::StartExposure(float Seconds)
{
   if ( !m_dllLoaded ) return ARTEMIS_NOT_CONNECTED; // bail if no DLL
   if ( !m_hCam ) return ARTEMIS_NOT_CONNECTED; // bail if no cam at all

   // workaround DLL bug
   int x,y,w,h,binx,biny;
   ::ArtemisHscGetSubframe(m_hCam, &x, &y, &w, &h);
   ::ArtemisHscGetBin(m_hCam, &binx, &biny);
   long imageSize = 2 * w * h / binx / biny;
   if (Seconds>2.4)
      m_msDLtimeExpected = imageSize / 230; // we assume DLspeed of 280 bytes/ms (280kB/sec) for expo>2.4sec
   else
      m_msDLtimeExpected = imageSize / 230 + Seconds*1500; // double exposure takes longer

   m_msDLtimeExpected = (m_msDLtimeExpected<=0) ? 1000 : m_msDLtimeExpected; // Div0 safety
   m_msStartDLtime = ::wxGetLocalTimeMillis();
   m_msStartDLtime = m_msStartDLtime + (1000*Seconds); // start DL time
   return (ARTEMISERROR)::ArtemisHscStartExposure(m_hCam, Seconds);
}

// Return time remaining in current exposure, in seconds
float ArtDrvHscW32DLL::ExposureTimeRemaining() const
{
   if ( !m_dllLoaded ) return 0.0; // bail if no DLL
   if ( !m_hCam ) return 0.0; // bail if no cam at all

   return ::ArtemisHscExposureTimeRemaining(m_hCam);
}

// Percentage downloaded
int ArtDrvHscW32DLL::DownloadPercent() const
{
   if ( !m_dllLoaded ) return 0; // bail if no DLL
   if ( !m_hCam ) return 0; // bail if no cam at all

   int dLpercent = ::ArtemisHscDownloadPercent(m_hCam);
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
ARTEMISERROR ArtDrvHscW32DLL::StopExposure()
{
   if ( !m_dllLoaded ) return ARTEMIS_NOT_CONNECTED; // bail if no DLL
   if ( !m_hCam ) return ARTEMIS_NOT_CONNECTED; // bail if no cam at all

   return (ARTEMISERROR)::ArtemisHscStopExposure(m_hCam);
}

// Abort exposure, if one is in progress
ARTEMISERROR ArtDrvHscW32DLL::AbortExposure()
{
   if ( !m_dllLoaded ) return ARTEMIS_NOT_CONNECTED; // bail if no DLL
   if ( !m_hCam ) return ARTEMIS_NOT_CONNECTED; // bail if no cam at all

   return (ARTEMISERROR)::ArtemisHscAbortExposure(m_hCam);
}

// Return true if an image is ready to be retrieved
bool ArtDrvHscW32DLL::ImageReady() const
{
   if ( !m_dllLoaded ) return false; // bail if no DLL
   if ( !m_hCam ) return false; // bail if no cam at all

   return ::ArtemisHscImageReady(m_hCam);
}


// Retrieve image dimensions and binning factors.
// x,y are actual CCD locations. w,h are pixel dimensions of image
void ArtDrvHscW32DLL::ImageData(unsigned short &x, unsigned short &y,
                                  unsigned short &w, unsigned short &h,
                                  unsigned short &binx, unsigned short &biny)
{
   if ( !m_dllLoaded ) return; // bail if no DLL
   if ( !m_hCam ) return; // bail if no cam at all

   int ix=x, iy=y, iw=w, ih=h, ibx=binx, iby=biny;
   ::ArtemisHscGetImageData(m_hCam, &ix, &iy, &iw, &ih, &ibx, &iby);
   x=(unsigned short)ix; y=(unsigned short)iy; w=(unsigned short)iw; h=(unsigned short)ih;
   binx=(unsigned short)ibx; biny=(unsigned short)iby;
}

// Return pointer to internal image buffer (actually wxUint16s)
const unsigned short* ArtDrvHscW32DLL::ImageBuffer() const
{
   if ( !m_dllLoaded ) return NULL; // bail if no DLL
   if ( !m_hCam ) return NULL; // bail if no cam at all

   return (unsigned short*) ::ArtemisHscImageBuffer(m_hCam);
}


// cooling support
void ArtDrvHscW32DLL::TemperatureSensorInfo(unsigned short sensor, int& temperature)
{
   if ( !m_dllLoaded ) return; // bail if no DLL
   if ( !m_hCam ) return; // bail if no cam at all
   int sr=sensor, tmp=temperature;
   ::ArtemisHscTemperatureSensorInfo(m_hCam, sr, &tmp);
   temperature=tmp;
}

void ArtDrvHscW32DLL::CoolingInfo(ECamCoolingInfo& flags, int& level, int& minlvl, int& maxlvl, int& setpoint)
{
   if ( !m_dllLoaded ) return; // bail if no DLL
   if ( !m_hCam ) return; // bail if no cam at all

   int flg=(int)flags, lvl=level, mlvl=minlvl, xlvl=maxlvl, spt=setpoint;
   ::ArtemisHscCoolingInfo(m_hCam, &flg, &lvl, &mlvl, &xlvl, &spt);
   flags=(ECamCoolingInfo)flg; level=lvl; minlvl=mlvl; maxlvl=xlvl; setpoint=spt;
}

ARTEMISERROR ArtDrvHscW32DLL::SetCooling(int setpoint)
{
   if ( !m_dllLoaded ) return ARTEMIS_NOT_CONNECTED; // bail if no DLL
   if ( !m_hCam ) return ARTEMIS_NOT_CONNECTED; // bail if no cam at all

   return (ARTEMISERROR)::ArtemisHscSetCooling(m_hCam, setpoint);
}

ARTEMISERROR ArtDrvHscW32DLL::SetCooling(double temperature)
{
   if ( !m_dllLoaded ) return ARTEMIS_NOT_CONNECTED; // bail if no DLL
   if ( !m_hCam ) return ARTEMIS_NOT_CONNECTED; // bail if no cam at all

   int setpoint = temperature *100.0;
   return (ARTEMISERROR)::ArtemisHscSetCooling(m_hCam, setpoint);
}

ARTEMISERROR ArtDrvHscW32DLL::CoolerWarmUp()
{
   if ( !m_dllLoaded ) return ARTEMIS_NOT_CONNECTED; // bail if no DLL
   if ( !m_hCam ) return ARTEMIS_NOT_CONNECTED; // bail if no cam at all

   return (ARTEMISERROR)::ArtemisHscCoolerWarmUp(m_hCam);
}


