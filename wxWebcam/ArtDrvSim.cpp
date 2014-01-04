
#include "ArtDrvSim.h"

#include "wx/thread.h"
#include <wx/stdpaths.h>
#include <wx/dir.h>


// define the simulation DL rate in kByte / sec (real cam is ~ 300)
#define ART_USB1_KBSEC   350
// define the simulation DL rate in kByte / sec (real cam is ~ 1500)
#define ART_USB2_KBSEC   1500

// the allowed temp gradient of the CCD chip (K/sec)
#define ART_T_GRADIENT  0.2
// the spec what the cooler can cool down (K)
#define ART_DELTA_T     33.0
// Simulation has this ambient temperature (°C)
#define ART_AMBIENT_T   10.0

// define some multiple used items here
#define ART429_FILE  "art429sim"    // ATK-16
#define ART285_FILE  "art285sim"    // ATK-16HR

#define ART424_FILE  "art424sim"    // ATK-16IC
#define ART415_FILE  "art415sim"    // ATK-16IC-S

#define ART4021_FILE "art4021sim"   // ART 4021
#define ART11002_FILE "art11002sim" // ART 11002


//! A small thread for the simulation
class ArtDrvSim;
class ArtSimThread : public wxThread
{
public:
   ArtSimThread(ArtDrvSim* pCaller);
   ~ArtSimThread();
public:
   //! thread interface
   ExitCode Entry();
   //! Prematurely stop thread execution
   void AbortThread();

private:
   ArtSimThread(){};

   void srandom(unsigned long seed);
   long random(void);
   double random1(void);
   long random_r(unsigned long *ctx);
   long do_random(unsigned long *ctx);

   ArtDrvSim*     m_pCaller;
	int            m_exitCode;
	long           m_dlTime;
	wxLongLong     m_lastTickTime;
   wxLongLong     m_endReadTime;
   // RND
   static const long RANDOM_MAX;
   unsigned long  m_next;

};

const long ArtSimThread::RANDOM_MAX =  0x7FFFFFFF;

ArtSimThread::ArtSimThread(ArtDrvSim* pCaller)
: m_pCaller(pCaller)
, m_exitCode(0)
, m_dlTime(0)
, m_next(1)
{
   m_lastTickTime = ::wxGetLocalTimeMillis();
}

ArtSimThread::~ArtSimThread()
{
}

long ArtSimThread::do_random(unsigned long *ctx)
{
	/*
	 * Compute x = (7^5 * x) mod (2^31 - 1)
	 * without overflowing 31 bits:
	 *      (2^31 - 1) = 127773 * (7^5) + 2836
	 * From "Random number generators: good ones are hard to find",
	 * Park and Miller, Communications of the ACM, vol. 31, no. 10,
	 * October 1988, p. 1195.
	 */
   const long RANDOM_MAX =  0x7FFFFFFF;
	long hi, lo, x;

	hi = *ctx / 127773L;
	lo = *ctx % 127773L;
	x = 16807L * lo - 2836L * hi;
	if (x <= 0)
		x += 0x7fffffffL;
	return ((*ctx = x) % ((unsigned long)RANDOM_MAX + 1));
}


long ArtSimThread::random_r(unsigned long *ctx)
{
	return do_random(ctx);
}

long ArtSimThread::random(void)
{
	return do_random(&m_next);
}

double ArtSimThread::random1(void)
{
   long r = do_random(&m_next);
	return ((double)r/(double)RANDOM_MAX);
}

void ArtSimThread::srandom(unsigned long seed)
{
	m_next = seed;
}

/// //////////////////////////////////////////////////////////////////////
wxThread::ExitCode ArtSimThread::Entry()
{
   const long WAIT_INTERVALL = 250;  // milliseconds
   m_dlTime = 2000;

	m_exitCode = 0;         // task result
	bool done = false;      // task status
   unsigned long dlRate = m_pCaller->m_dlRate;
   // Sanity check
   if ( !m_pCaller ) {
      m_exitCode = 2;
      done = true;
   }

    /* ----------------------------------------------------------- */
    while(!done)
    {
      // wxThread obligation
      if ( this->TestDestroy() ) {
         // must terminate prematurely
         this->AbortThread();
      }
      if (m_pCaller->m_aborted) {
         // task was aborted
         m_exitCode = 1;
         done = true;  // thread will die now
      }
      else {

         // this timeslot
         wxLongLong now = ::wxGetLocalTimeMillis();
         long dt = (now-m_lastTickTime).ToLong();

         // cooler handling
         int noise = (int)(random1() * 100.0) - 50;
         if (m_pCaller->m_coolingBaseFlags) {
            if (m_pCaller->m_warmingUp) {
               m_pCaller->m_actualTemp += (int)(ART_T_GRADIENT/1000.0*100.0 * dt); // (K/sec) /1000ms *100 ArtTempUnits
               if ( m_pCaller->m_actualTemp> (ART_AMBIENT_T*100) ) {
                  m_pCaller->m_actualTemp = int(ART_AMBIENT_T*100 +noise);
                  m_pCaller->m_warmingUp=false;  // done
               }
            }
            else if (m_pCaller->m_coolingOn) {
               if (m_pCaller->m_setpCtrl) {
                  m_pCaller->m_actualTemp -= (int)(ART_T_GRADIENT/1000.0*100.0 * dt);
                  if ( m_pCaller->m_actualTemp<=m_pCaller->m_setpoint ) {
                     m_pCaller->m_actualTemp=m_pCaller->m_setpoint +noise;  // setpoint reached
                  }
               }
               else {
                  // no setpoint cooler
                  ; // dont'know yet
               }
            }
            else {
               m_pCaller->m_actualTemp = int(ART_AMBIENT_T*100 +noise);
            }
         }//end cooling support

         // image handling
         if (m_pCaller->m_camState == CAMERA_IDLE) {
            ; // nothing to do ..
         }// IDLE task

         else if (m_pCaller->m_camState == CAMERA_FLUSHING) {
            // the real cam would DL the data but not return an image
            // set DL status
            m_pCaller->m_dlPercent = 100;
            m_pCaller->m_abortExposure = false; // not longer
            // set true even it's not copied - abort just don't catch the image but retains the state machine
            m_pCaller->m_artSample.ready = true; // read done now
            // next state
            m_pCaller->m_camState = CAMERA_IDLE;
         }// FLUSH task

         else if (m_pCaller->m_camState == CAMERA_DOWNLOADING) {
            if ( (m_endReadTime - now).ToLong()<=0 ) {
               // DL done

               // copy requested image data into m_artSample
               wxUint16* srcPtr;
               wxUint16* dstPtr = m_pCaller->m_artSample.imageMem->WordMemPtrRef();

               size_t binx = m_pCaller->m_artSample.binX;
               size_t ccdw = m_pCaller->m_camProps.nPixelsX;
               size_t biny = m_pCaller->m_artSample.binY;
               // convert from binned pixels
               size_t eWidth = m_pCaller->m_artSample.imageMem->FrameRect().width * binx;   // ccd pixels
               size_t eHeight = m_pCaller->m_artSample.imageMem->FrameRect().height * biny; // ccd pixels
               size_t eX = m_pCaller->m_artSample.imageMem->FrameRect().x;      // ccd pixels
               size_t eY = m_pCaller->m_artSample.imageMem->FrameRect().y;      // ccd pixels
               float  lastExp = m_pCaller->m_lastExpoDuration;

               for (size_t y=eY; y<eY+eHeight; y+=biny) {
                  for (size_t x=eX; x<eX+eWidth; x+=binx) {
                     unsigned long pVal = 0;
                     // binning
                     for (size_t yy=0; yy<biny; yy++) {
                        // set the srcPtr to the proper scan line
                        srcPtr = &m_pCaller->m_imageData[(y+yy)*ccdw + x];
                        for (size_t xx=0; xx<binx; xx++) {
                           // integrate row pixels according to the bin size
                           // scale the values based on exposure length
                           pVal += (unsigned long)( lastExp*(*srcPtr++) );
                        }// for xx
                     }// for yy
                     // cut binning result to word
                     *dstPtr++ = (wxUint16)( (pVal<0xffff)?pVal:0xffff );
                  }// for x
               }// for y

               // set DL status
               m_pCaller->m_dlPercent = 100;
               m_pCaller->m_artSample.ready = true; // read done now
               // next state
               m_pCaller->m_camState = CAMERA_IDLE;
            }// DL done
            else {
               // progress DL
               m_pCaller->m_dlPercent = 100 - (100*(m_endReadTime - now).ToLong() / m_dlTime);
            }
         }// DL task

         else if (m_pCaller->m_camState == CAMERA_EXPOSING) {
            if ( (m_pCaller->m_endExpoTime - now).ToLong()<=0 ) {
               // expo done - get image
               //PREPARE READ CCD
               m_pCaller->m_dlPercent = 0;
               m_dlTime = m_pCaller->m_artSample.dataLength / dlRate; // bytes / kByte/sec  =  ms download time
               m_endReadTime = now + m_dlTime;  // 2 sec read for now
               // next state
               if (m_pCaller->m_abortExposure) {
                  m_pCaller->m_camState = CAMERA_FLUSHING; // send data to NIL
               }
               else {
                  m_pCaller->m_camState = CAMERA_DOWNLOADING;
               }
            }// expo done
         }// EXPO task

         else {
            // just waiting
            ;
         }
         m_lastTickTime = now;
         this->Sleep(WAIT_INTERVALL);  // now wait a while
      } // not aborted
   }// thread loop

   Exit(&m_exitCode);   // exit thread
   return &m_exitCode;  // thread completed successfully
}

// thread must abort itself
void ArtSimThread::AbortThread()
{
	// will make the thread to terminate
	m_pCaller->m_aborted = true; // thread will end now (in a while)
}// Abort







//! Implement the real SIM Driver interface

ArtDrvSim* ArtDrvSim::pinstance = 0;// initialize pointer
ArtDrvSim* ArtDrvSim::Instance ()
{
 if (pinstance == 0)  // is it the first call?
 {
   pinstance = new ArtDrvSim(); // create sole instance
 }
 return pinstance; // address of sole instance
}

ArtDrvSim::ArtDrvSim()
: m_connected(false)
, m_abortExposure(false)
, m_camState(CAMERA_ERROR)
, m_pSimThread(NULL)
, m_aborted(false)
, m_simFilename(wxT(""))
, m_ccdsx(0)
, m_ccdsy(0)
, m_ccdw(0)
, m_ccdh(0)
, m_binx(0)
, m_biny(0)
, m_maxBin(6)
, m_ampSwitched(false)
, m_lastExpoDuration(1.0)
, m_dlPercent(0)
, m_dlRate(1)
, m_imageData(NULL)
, m_coolingBaseFlags(COE_NULL_FLAG)
, m_actualTemp(int(ART_AMBIENT_T*100))
, m_setpoint(int(ART_AMBIENT_T*100))
, m_warmingUp(false)
, m_coolingOn(false)
, m_setpCtrl(false)
{
   SetSimProperties(ACT_ArtFTD_00); // default is invalid to avoid excessive mem alloc if not used
   // clean record
	m_artSample.binX = 1; m_artSample.binY = 1;
	m_artSample.imageMem = NULL;
	m_artSample.dataLength = 0;
	m_artSample.ready = false;
}



ArtDrvSim::~ArtDrvSim()
{
   Disconnect();
	if (m_artSample.imageMem) delete m_artSample.imageMem;
   if (m_imageData) delete m_imageData;
}


void ArtDrvSim::SetSimProperties(EArtCameraType camType)
{
   if (m_imageData) delete m_imageData;
   m_imageData = NULL;

   switch (camType) {
   case ACT_Art11002:
      m_camProps.cameraflags = ARTEMIS_PROPERTIES_CAMERAFLAGS_PREVIEW
                              + ARTEMIS_PROPERTIES_CAMERAFLAGS_SUBSAMPLE
                              + ARTEMIS_PROPERTIES_CAMERAFLAGS_COOLING
                              + ARTEMIS_PROPERTIES_CAMERAFLAGS_WARMUP;
      m_camProps.ccdflags = ARTEMIS_PROPERTIES_CCDFLAGS_NULL;
      m_coolingBaseFlags = (ECamCoolingInfo)(COE_CCooling | COE_CControl | COE_SetPoint); // returns ArtHSC cooling
      strcpy(&(m_camProps.Description[0]), "Art 11002 Sim");
      strcpy(&(m_camProps.Manufacturer[0]), "Homebrew");
      m_camProps.nPixelsX = 4008;
      m_camProps.nPixelsY = 2672;
      m_camProps.PixelMicronsX = 9.0 ;
      m_camProps.PixelMicronsY = 9.0;
      m_camProps.Protocol = 100;
      m_imageData = new wxUint16 [m_camProps.nPixelsX*m_camProps.nPixelsY];
      m_dlRate= ART_USB2_KBSEC;
      break;

   case ACT_Art4021:
      m_camProps.cameraflags = ARTEMIS_PROPERTIES_CAMERAFLAGS_PREVIEW
                              + ARTEMIS_PROPERTIES_CAMERAFLAGS_SUBSAMPLE
                              + ARTEMIS_PROPERTIES_CAMERAFLAGS_COOLING
                              + ARTEMIS_PROPERTIES_CAMERAFLAGS_WARMUP;
      m_camProps.ccdflags = ARTEMIS_PROPERTIES_CCDFLAGS_NULL;
      m_coolingBaseFlags = (ECamCoolingInfo)(COE_CCooling | COE_CControl | COE_SetPoint); // returns ArtHSC cooling
      strcpy(&(m_camProps.Description[0]), "Art 4021 Sim");
      strcpy(&(m_camProps.Manufacturer[0]), "Homebrew");
      m_camProps.nPixelsX = 2048;
      m_camProps.nPixelsY = 2048;
      m_camProps.PixelMicronsX = 7.4f ;
      m_camProps.PixelMicronsY = 7.4f;
      m_camProps.Protocol = 100;
      m_imageData = new wxUint16 [m_camProps.nPixelsX*m_camProps.nPixelsY];
      m_dlRate= ART_USB2_KBSEC;
      break;

   case ACT_Art424:
      m_camProps.cameraflags = ARTEMIS_PROPERTIES_CAMERAFLAGS_NULL;
      m_camProps.ccdflags = ARTEMIS_PROPERTIES_CCDFLAGS_NULL;
      strcpy(&(m_camProps.Description[0]), "Atk-16ic Sim");
      strcpy(&(m_camProps.Manufacturer[0]), "Homebrew");
      m_camProps.nPixelsX = 659;
      m_camProps.nPixelsY = 494;
      m_camProps.PixelMicronsX = 7.4f ;
      m_camProps.PixelMicronsY = 7.4f;
      m_camProps.Protocol = 100;
      m_imageData = new wxUint16 [m_camProps.nPixelsX*m_camProps.nPixelsY];
      m_dlRate= ART_USB1_KBSEC;
      break;

   case ACT_Art415:
      m_camProps.cameraflags = ARTEMIS_PROPERTIES_CAMERAFLAGS_NULL;
      m_camProps.ccdflags = ARTEMIS_PROPERTIES_CCDFLAGS_NULL;
      strcpy(&(m_camProps.Description[0]), "Atk-16ic-s Sim");
      strcpy(&(m_camProps.Manufacturer[0]), "Homebrew");
      m_camProps.nPixelsX = 782;
      m_camProps.nPixelsY = 582;
      m_camProps.PixelMicronsX = 8.3f ;
      m_camProps.PixelMicronsY = 8.3f;
      m_camProps.Protocol = 100;
      m_imageData = new wxUint16 [m_camProps.nPixelsX*m_camProps.nPixelsY];
      m_dlRate= ART_USB1_KBSEC;
      break;

   case ACT_Art285:
      m_camProps.cameraflags = ARTEMIS_PROPERTIES_CAMERAFLAGS_FIFO;
      m_camProps.ccdflags = ARTEMIS_PROPERTIES_CCDFLAGS_NULL;
      strcpy(&(m_camProps.Description[0]), "Art 285 Sim");
      strcpy(&(m_camProps.Manufacturer[0]), "Homebrew");
      m_camProps.nPixelsX = 1392;
      m_camProps.nPixelsY = 1040;
      m_camProps.PixelMicronsX = 6.45f ;
      m_camProps.PixelMicronsY = 6.45f;
      m_camProps.Protocol = 100;
      m_imageData = new wxUint16 [m_camProps.nPixelsX*m_camProps.nPixelsY];
      m_dlRate= ART_USB1_KBSEC;
      break;

   case ACT_Art429:
      m_camProps.cameraflags = ARTEMIS_PROPERTIES_CAMERAFLAGS_NULL;
      m_camProps.ccdflags = ARTEMIS_PROPERTIES_CCDFLAGS_INTERLACED;
      strcpy(&(m_camProps.Description[0]), "Art 429 Sim");
      strcpy(&(m_camProps.Manufacturer[0]), "Homebrew");
      m_camProps.nPixelsX = 752;
      m_camProps.nPixelsY = 582;
      m_camProps.PixelMicronsX = 8.6f ;
      m_camProps.PixelMicronsY = 8.3f;
      m_camProps.Protocol = 100;
      m_imageData = new wxUint16 [m_camProps.nPixelsX*m_camProps.nPixelsY];
      m_dlRate= ART_USB1_KBSEC;
      break;

   default:
      m_camProps.cameraflags = ARTEMIS_PROPERTIES_CAMERAFLAGS_NULL;
      m_camProps.ccdflags = ARTEMIS_PROPERTIES_CCDFLAGS_NULL;
      strcpy(&(m_camProps.Description[0]), "INVALID Sim");
      strcpy(&(m_camProps.Manufacturer[0]), "Homebrew");
      m_camProps.nPixelsX = 10;
      m_camProps.nPixelsY = 10;
      m_camProps.PixelMicronsX = 1.0 ;
      m_camProps.PixelMicronsY = 1.0;
      m_camProps.Protocol = 100;
      m_imageData = new wxUint16 [m_camProps.nPixelsX*m_camProps.nPixelsY];
      m_dlRate= ART_USB1_KBSEC;
      break;
   }//switch
}


bool ArtDrvSim::ReadImage(const wxFileName &simName)
{
   FILE* ifi;
   if (!simName.IsOk()) return false;

   if ( (ifi = ::fopen(simName.GetFullPath().fn_str(), "rb"))==NULL) {
      // open fails...
      return false;
   }
   size_t fLength = m_camProps.nPixelsX*m_camProps.nPixelsY * 2;
   size_t fRead = ::fread(m_imageData, 1, fLength, ifi);
   ::fclose(ifi);
   return (fLength==fRead);
}


// Get USB Identifier of Nth USB device. Return false if no such device.
// pName must be at least 40 chars long.
bool ArtDrvSim::DeviceName(unsigned short Device, char *pName)
{
   switch (Device) {
   case ACT_Art11002:
      ::strcpy(pName, "Art 11002 Sim");
      return true;
      break;
   case ACT_Art4021:
      ::strcpy(pName, "Art 4021 Sim");
      return true;
      break;
   case ACT_Art424:
      ::strcpy(pName, "Atk-16ic Sim");
      return true;
      break;
   case ACT_Art415:
      ::strcpy(pName, "Atk-16ic-s Sim");
      return true;
      break;
   case ACT_Art429:
      ::strcpy(pName, "Art 429 Sim");
      return true;
      break;
   case ACT_Art285:
      ::strcpy(pName, "Art 285 Sim");
      return true;
      break;
   default:
      ::strcpy(pName, "n.a.");
      return false;
      break;
   }// switch
}

// Get USB Serial number of Nth USB device. Return false if no such device.
// pName must be at least 40 chars long.
bool ArtDrvSim::DeviceSerial(unsigned short Device, char *pName)
{
   const char SimDev[] = "X12345";
   ::strcpy(pName, SimDev);
   bool retVal = false;
   retVal = retVal || (Device==ACT_Art429);
   retVal = retVal || (Device==ACT_Art285);
   retVal = retVal || (Device==ACT_Art424);
   retVal = retVal || (Device==ACT_Art415);
   retVal = retVal || (Device==ACT_Art4021);
   retVal = retVal || (Device==ACT_Art11002);
   return retVal;
}

// checks if the image file exists in UserData or current dir
// returns true if the file exists and the simName is containing the filename
bool ArtDrvSim::CheckSimFile(wxFileName &simName, unsigned short Device)
{
   // look for sim files in GetUserDataDir()
   // Return the directory for the user-dependent application data files:
   // Unix: ~/.appname
   // Windows: C:\Documents and Settings\username\Application Data\appname
   wxStandardPaths px;
   // if we did not found a file we check the current dir for compatibility reasons
   switch (Device) {
   case ACT_Art429:
      simName.Assign(px.GetUserDataDir(), wxT(ART429_FILE), wxT("raw"));
      if (simName.IsOk()) if (simName.FileExists()) return true;
      simName.Assign(wxT("."), wxT(ART429_FILE), wxT("raw"));
      if (simName.IsOk()) if (simName.FileExists()) return true;
      break;
   case ACT_Art285:
      simName.Assign(px.GetUserDataDir(), wxT(ART285_FILE), wxT("raw"));
      if (simName.IsOk()) if (simName.FileExists()) return true;
      simName.Assign(wxT("."), wxT(ART285_FILE), wxT("raw"));
      if (simName.IsOk()) if (simName.FileExists()) return true;
      break;
   case ACT_Art424:
      simName.Assign(px.GetUserDataDir(), wxT(ART424_FILE), wxT("raw"));
      if (simName.IsOk()) if (simName.FileExists()) return true;
      simName.Assign(wxT("."), wxT(ART424_FILE), wxT("raw"));
      if (simName.IsOk()) if (simName.FileExists()) return true;
      break;
   case ACT_Art415:
      simName.Assign(px.GetUserDataDir(), wxT(ART415_FILE), wxT("raw"));
      if (simName.IsOk()) if (simName.FileExists()) return true;
      simName.Assign(wxT("."), wxT(ART415_FILE), wxT("raw"));
      if (simName.IsOk()) if (simName.FileExists()) return true;
      break;
   case ACT_Art4021:
      simName.Assign(px.GetUserDataDir(), wxT(ART4021_FILE), wxT("raw"));
      if (simName.IsOk()) if (simName.FileExists()) return true;
      simName.Assign(wxT("."), wxT(ART4021_FILE), wxT("raw"));
      if (simName.IsOk()) if (simName.FileExists()) return true;
      break;
   case ACT_Art11002:
      simName.Assign(px.GetUserDataDir(), wxT(ART11002_FILE), wxT("raw"));
      if (simName.IsOk()) if (simName.FileExists()) return true;
      simName.Assign(wxT("."), wxT(ART11002_FILE), wxT("raw"));
      if (simName.IsOk()) if (simName.FileExists()) return true;
      break;
   default:
      ;
   }// switch
   simName.Clear();
   return false;
}


// Return true if Nth USB device exists and is a camera.
bool ArtDrvSim::DeviceIsCamera(unsigned short Device)
{
   wxFileName fName;
   return CheckSimFile(fName, Device);
}

// Connect to given device.
// Sets the internal handle if connected as requested
// returns true when successfull otherwise false
bool ArtDrvSim::Connect(unsigned short Device)
{
   if (m_connected)
      return false;
   // basically DeviceIsCamera() also sets the member var of the simFilename
   if (!CheckSimFile(m_simFilename, Device)) return false;

   // this sets the cameras properties
   SetSimProperties((EArtCameraType)Device);

   if ( ! ReadImage(m_simFilename) ) {
       wxMessageBox(wxT("ArtSimCam: cannot find a proper image raw file - please check installation"));
      return false;
   }
   m_ccdsx = 0;
   m_ccdsy = 0;
   m_ccdw = m_camProps.nPixelsX;
   m_ccdh = m_camProps.nPixelsY;
   m_binx = 1;
   m_biny = 1;
	if (m_artSample.imageMem)
	{
		delete m_artSample.imageMem;
		m_artSample.imageMem = NULL;
		m_artSample.dataLength = 0;
	}

	// make a new image buffer if we know the ccd i.e. call this before reading the ccd
   m_artSample.dataLength = m_camProps.nPixelsX * m_camProps.nPixelsY; // WORDS full cam size
   m_artSample.imageMem = new WordImageMem(m_artSample.dataLength); // progressive CCD
   m_artSample.dataLength *= 2; // make BYTES (this is what is needed to be read
   m_artSample.ready = false;

   m_aborted = false;
   m_pSimThread = new ArtSimThread(this);
   m_pSimThread->Create();
   m_pSimThread->Run();   // let's run it

   m_abortExposure = false;
   m_connected = true;
   m_camState = CAMERA_IDLE;

   return m_connected;
}

ARTEMISERROR ArtDrvSim::ReconnectUSB()
{
   return ARTEMIS_OK;
}


// Return the last FT USB error condition seen
// Calling this function clears the internal error state (FT_OK=0)
int ArtDrvSim::DiagnosticUSBError()
{
   return 0;
}

// Ping the camera, return the result. -1 on error.
int ArtDrvSim::DiagnosticPing(int send)
{
   return send;
}

// Disconnect from given device.
// Returns true if disconnected as requested
bool ArtDrvSim::Disconnect()
{
   AbortExposure();

   m_connected=false;
   //m_warmingUp = (m_coolingOn) ? true:false; // switch to warmup if cooler was on
   wxASSERT(m_pSimThread);
   m_pSimThread->AbortThread();
   ::wxMilliSleep(500); // allow some time to end thread
   m_camState = CAMERA_IDLE;
   return !m_connected;
}


// Returns TRUE if currently connected to a device
bool ArtDrvSim::IsConnected() const
{
   return m_connected;
}

// Retrieve the current camera state
ARTEMISCAMERASTATE ArtDrvSim::CameraState() const
{
   return m_camState;
}

// Fills in pProp with camera properties
ARTEMISERROR ArtDrvSim::Properties(ARTEMISPROPERTIES &pProp)
{
   pProp = m_camProps;
   return ARTEMIS_OK;
}

// Set download thread to high or normal priority
ARTEMISERROR ArtDrvSim::HighPriority(bool WXUNUSED(bHigh))
{
   if (!m_connected) return ARTEMIS_NOT_CONNECTED;
   return ARTEMIS_NOT_IMPLEMENTED;
}

// Get the pos and size of imaging subframe
void ArtDrvSim::Subframe(wxUint16 &x, wxUint16 &y, wxUint16 &w, wxUint16 &h) const
{
   x=m_ccdsx; y=m_ccdsy; w=m_ccdw; h=m_ccdh;
}

// set the pos and size of imaging subframe inunbinned coords
ARTEMISERROR ArtDrvSim::SetSubframe(wxUint16 x, wxUint16 y, wxUint16 w, wxUint16 h)
{
   if (!m_connected) return ARTEMIS_NOT_CONNECTED;

   if ( (x+w) > m_camProps.nPixelsX ) return ARTEMIS_INVALID_PARAMETER;
   if ( (y+h) > m_camProps.nPixelsY ) return ARTEMIS_INVALID_PARAMETER;

   m_ccdsx = x; m_ccdsy = y;
   m_ccdw = w;  m_ccdh = h;
   return ARTEMIS_OK;
}

// Set the start x,y coords for imaging subframe., X,Y in unbinned coordinates
ARTEMISERROR ArtDrvSim::SetSubframePos(wxUint16 x, wxUint16 y)
{
   if (!m_connected) return ARTEMIS_NOT_CONNECTED;
   if ( (x+m_ccdw) > m_camProps.nPixelsX ) return ARTEMIS_INVALID_PARAMETER;
   if ( (y+m_ccdh) > m_camProps.nPixelsY ) return ARTEMIS_INVALID_PARAMETER;

   m_ccdsx = x; m_ccdsy = y;
   return ARTEMIS_OK;
}

// Set the width and height of imaging subframe, W,H in unbinned coordinates
ARTEMISERROR ArtDrvSim::SetSubframeSize(wxUint16 w, wxUint16 h)
{
   if (!m_connected) return ARTEMIS_NOT_CONNECTED;
   if ( (m_ccdsx+w) > m_camProps.nPixelsX  ) return ARTEMIS_INVALID_PARAMETER;
   if ( (m_ccdsy+h) > m_camProps.nPixelsY ) return ARTEMIS_INVALID_PARAMETER;

   m_ccdw = w;  m_ccdh = h;
   return ARTEMIS_OK;
}


// Get the x,y binning factors
void ArtDrvSim::Bin(wxUint16 &x, wxUint16 &y) const
{
   x = m_binx; y = m_biny;
}

// Set the x,y binning factors
ARTEMISERROR ArtDrvSim::SetBin(wxUint16 x, wxUint16 y)
{
   if (!m_connected) return ARTEMIS_NOT_CONNECTED;
   if ( x > m_maxBin ) return ARTEMIS_INVALID_PARAMETER;
   if ( y > m_maxBin ) return ARTEMIS_INVALID_PARAMETER;
   m_binx = x; m_biny = y;
   return ARTEMIS_OK;
}

// Get the max x,y binning factors
void ArtDrvSim::MaxBin(wxUint16& x, wxUint16& y)
{
   x=m_maxBin; y=m_maxBin;
}


// Return true if amp switched off during exposures
bool ArtDrvSim::AmplifierSwitched() const
{
   return m_ampSwitched;
}

// Set whether amp is switched off during exposures
ARTEMISERROR ArtDrvSim::SetAmplifierSwitched(bool bSwitched)
{
   if (!m_connected) return ARTEMIS_NOT_CONNECTED;
   m_ampSwitched = bSwitched;
   return ARTEMIS_OK;
}

// Set the CCD amplifier on or off
ARTEMISERROR ArtDrvSim::SetAmp(bool WXUNUSED(bOn))
{
   if (!m_connected) return ARTEMIS_NOT_CONNECTED;
   return ARTEMIS_OK;
}


// Return duration of last exposure, in seconds
float ArtDrvSim::LastExposureDuration() const
{
   return m_lastExpoDuration;
}


// Start an exposure
ARTEMISERROR ArtDrvSim::StartExposure(float Seconds)
{
   if (!m_connected) return ARTEMIS_NOT_CONNECTED;

   if (m_camState!=CAMERA_IDLE) return ARTEMIS_NO_RESPONSE;

   m_abortExposure = false; // reset

      // get this data as late as possible as it may have changed from setup
   // due to pixel adjustment for interlace and binning (cam does not like odd sizes)
   m_startExpoTime = ::wxGetLocalTimeMillis();
   m_lastExpoDuration = (float)m_expoDuration / 1000.0;
   m_expoDuration = (unsigned long)(Seconds * 1000.0);
   m_endExpoTime = m_startExpoTime + m_expoDuration;

   // make sure ccdh and ccdsx gets even after binning and interlace
   ///Sim behaves like ArtCcdCmd now
	m_ccdsx/=m_binx*2; m_ccdsx*=m_binx*2;
	m_ccdsy/=m_biny*2; m_ccdsy*=m_biny*2;
	m_ccdw/=m_binx*2; m_ccdw*=m_binx*2;
	m_ccdh/=m_biny*2; m_ccdh*=m_biny*2;


   m_artSample.ready = false;
   m_artSample.expotime = m_expoDuration;
   m_artSample.imageMem->SetFrameRect(wxRect(m_ccdsx, m_ccdsy, m_ccdw/m_binx, m_ccdh/m_biny));
   m_artSample.binX   = m_binx;   m_artSample.binY   = m_biny;
   m_artSample.dataLength = (m_ccdw/m_binx) * (m_ccdh/m_biny) *2; // bytes

   if (m_artSample.expotime<500) {
      // no timer, just let ReadCCD do the job
      ::wxMilliSleep(m_expoDuration);
      m_endExpoTime = ::wxGetLocalTimeMillis();

      // next state
      m_camState = CAMERA_EXPOSING;
   }
   else {
      // wait until m_endExpoTime is reached

      // next state
      m_camState = CAMERA_EXPOSING;
   }

   return ARTEMIS_OK;
}


// Return time remaining in current exposure, in seconds
float ArtDrvSim::ExposureTimeRemaining() const
{
   if (!m_connected) return 0.0;

   if (m_camState == CAMERA_EXPOSING)
      return (m_endExpoTime - ::wxGetLocalTimeMillis()).ToLong() / 1000.0;
   else
      return 0.0;
}


// Percentage downloaded
int ArtDrvSim::DownloadPercent() const
{
   return m_dlPercent;
}


// Prematurely end an exposure, collecting image data.
ARTEMISERROR ArtDrvSim::StopExposure()
{
   if (!m_connected) return ARTEMIS_NOT_CONNECTED;

   if (m_camState == CAMERA_EXPOSING) {
      m_endExpoTime = ::wxGetLocalTimeMillis(); // now
      m_expoDuration = (m_endExpoTime-m_startExpoTime).ToLong(); // real duration
      m_artSample.expotime = m_expoDuration;
   }

   return ARTEMIS_OK;
}


// Abort exposure, if one is in progress
ARTEMISERROR ArtDrvSim::AbortExposure()
{
   m_abortExposure = true;
   return StopExposure();
}



// Return true if an image is ready to be retrieved
// in SimCam this is the simulation thread as it is polled each 100ms from the owner
// so any timedependent things are done here
bool ArtDrvSim::ImageReady() const
{
   if (!m_connected) return false;

   return m_artSample.ready;
}


// Retrieve image dimensions and binning factors.
// x,y are actual CCD locations. w,h are pixel dimensions of image
void ArtDrvSim::ImageData(wxUint16 &x, wxUint16 &y,
                                  wxUint16 &w, wxUint16 &h,
                                  wxUint16 &binx, wxUint16 &biny)
{
   wxRect R;
   if (m_artSample.imageMem) R = m_artSample.imageMem->FrameRect();
   else R = wxRect(0,0,0,0);

   x = R.x;     y = R.y;
   w = R.width; h = R.height;
   binx = m_artSample.binX; biny = m_artSample.binY;
}


// Return pointer to internal image buffer (actually wxUint16s)
const wxUint16* ArtDrvSim::ImageBuffer() const
{
   if (!m_connected) return NULL;

   if (m_artSample.ready && m_artSample.imageMem)
      return m_artSample.imageMem->WordMemPtr();
   else
      return NULL;
}


// cooling support
void ArtDrvSim::TemperatureSensorInfo(wxUint16 WXUNUSED(sensor), int& temperature) // °C *100
{
   temperature=m_actualTemp;
}

void ArtDrvSim::CoolingInfo(ECamCoolingInfo& flags, int& WXUNUSED(level), int& WXUNUSED(minlvl), int& WXUNUSED(maxlvl), int& setpoint)
{
   flags = m_coolingBaseFlags;
   flags = (ECamCoolingInfo)(flags | ((m_warmingUp) ? COE_SWarmUp   : 0) );
   flags = (ECamCoolingInfo)(flags | ((m_coolingOn) ? COE_SCooling  : 0) );
   flags = (ECamCoolingInfo)(flags | ((m_setpCtrl)  ? COE_SSetPoint : 0) );
   setpoint = m_setpoint;
}

ARTEMISERROR ArtDrvSim::SetCooling(int setpoint)
{
   if (m_warmingUp) return ARTEMIS_INVALID_PARAMETER; // not allowed while warming up

   m_setpoint = setpoint;
   if (m_setpoint < (ART_AMBIENT_T-ART_DELTA_T)*100) m_setpoint = int( (ART_AMBIENT_T-ART_DELTA_T)*100 );
   if (m_setpoint > (ART_AMBIENT_T)*100) m_setpoint = int( (ART_AMBIENT_T)*100 );
   // switch SP cooling on
   m_coolingOn = true;
   m_setpCtrl = true;
   return ARTEMIS_OK;
}

ARTEMISERROR ArtDrvSim::SetCooling(double temperature)
{
   if (m_warmingUp) return ARTEMIS_INVALID_PARAMETER; // not allowed while warming up

   m_setpoint = temperature*100;
   if (m_setpoint < (ART_AMBIENT_T-ART_DELTA_T)*100) m_setpoint = int( (ART_AMBIENT_T-ART_DELTA_T)*100 );
   if (m_setpoint > (ART_AMBIENT_T)*100) m_setpoint = int( (ART_AMBIENT_T)*100 );
   // switch SP cooling on
   m_coolingOn = true;
   m_setpCtrl = true;
   return ARTEMIS_OK;
}

ARTEMISERROR ArtDrvSim::CoolerWarmUp()
{
   m_warmingUp = true;
   // switch SP cooling off
   m_setpCtrl = false;
   m_coolingOn = false;
   return ARTEMIS_OK;
}

