// wxArtcam.cpp: implementation of the wxArtcam class.
//
//////////////////////////////////////////////////////////////////////

#include "wxArtcam.h"

#include <wx/dcbuffer.h>

#include "wxImageStore.h"
#include "wxArtCoords.h"
#include "wxTimeFormatter.h"

#include "../wxFactory.h"
#include "../wxMessageArea.h"
#include "../BatchJobManager.h"

#include "../GenericDebHelp.h"


BEGIN_EVENT_TABLE(wxArtcam, wxCamera)
   EVT_SIZE(wxArtcam::OnSize)
   EVT_PAINT(wxArtcam::OnDraw)
   EVT_MOUSEWHEEL(wxArtcam::OnMouseWheel)
   EVT_RIGHT_DCLICK(wxArtcam::OnMouseDClick)
   EVT_RIGHT_DOWN(wxArtcam::OnMouseRightDown)
   EVT_RIGHT_UP(wxArtcam::OnMouseRightUp)
   EVT_LEFT_DOWN(wxArtcam::OnMouseLeftDown)
   EVT_LEFT_UP(wxArtcam::OnMouseLeftUp)
   EVT_MOTION(wxArtcam::OnMouseMove)
   EVT_ENTER_WINDOW(wxArtcam::OnMouseEnterWindow)
   EVT_LEAVE_WINDOW(wxArtcam::OnMouseLeaveWindow)
END_EVENT_TABLE()


unsigned short wxArtcam::m_default_binFactor = 2;

wxArtcam* wxArtcam::Construct(wxWindow* parent, wxWindowID id)
{

   return new wxArtcam(parent,id);
}

/// NOTE:
// If refering to frames and sizes care must be taken.
// When switching the binning, (sub)framesize for the next exposure
//  there is still one running under the previous settings.
// To get the dimensions of the current sample one has to use the values given in the m_pArtSample obj
//
// If a size/binning changes the fact is stored in the m_fmtChange record
//  and applied to the camera in StartPreview() just before a new exposure is started
//  also a note is taken to create a new wxImage to store the screen image if the
//  exposure ends and new data arrives (see below).
//
// When the exposure ends, an image for a sample is allocated in CaptureHandling() where
//  we use the dimensions given in m_pArtSample which are most accurate values for the sample at hand.
//  (we don't know how the camera changes our wishes of the binning,framesize as it cannot
//   process any values i.e. an interlaced cam cannot bin x3 or x5, for binned images the subframe
//   start/endpoints have to be aligned)
//
// Any image manipulation such as subframe selection, binning relates therefore
//  to the image show on screen and not to the one which is in progress
//  or the one to be taken next cycle !!
//
// In the end we have to use coordinates of the show Image and m_pArtSample
//  while the user interacts with the screen
//  Also when mapping a new image to screen we may use those values.
//  There is a short time when a new sample is taken and the image data are copied to
//  the live image where things mix up (may we should introduce an exclusive section there)
//
// The camera m_ds->SubFrame()  dimensions just show the current state of the _next_ sample to be taken
//
// END NOTE

// helper class for handling of live images
class LiveImageArt
{
public:
   LiveImageArt();
   ~LiveImageArt();

   bool UpdateImage(const wxSize& size, bool fullFrame);
   void SetChangeImage();

   // return the pointer to the real image
   wxImage*  ImagePtr() const;
   // return the ptr to the Background image
   wxImage* BackgPtr() const; // might be NULL !!

   // set/get internal state
   void SetFrameSize(const wxSize& size);

   void SetSubFrame(bool sub);
   bool SubFrame() const;
   void SetSubFramePos(const wxPoint& leftTop);
   // returns validity of Image
   bool ImageOK() const;
   bool BackgImageOK() const;

   // Returns the full frame size
   wxSize Size() const;
   int Width() const {return Size().GetWidth();}
   int Height() const {return Size().GetHeight();}
   // returns the subframe pos
   wxPoint SubFrameLeftTop() const;
   int SubFrameLeft() const {return SubFrameLeftTop().x;}
   int SubFrameTop() const {return SubFrameLeftTop().y;}

private:
   wxImage*    m_pImage;            // instance of image to hold the still image either full or subframe
   wxImage*    m_pBackImage;        // the background map which is a x binned fullframe
   bool        m_imageChangeDue;    // true if image change is needed for the next data
   bool        m_drawSubframe;      // true if only a subframe has to be drawn
   wxPoint     m_subFrameLeftTop;
   wxSize      m_fullSize;         // the fullframe size
};
LiveImageArt::LiveImageArt()
: m_pImage(NULL)
, m_pBackImage(NULL)
, m_imageChangeDue(false)
, m_drawSubframe(false)
{
   m_pImage = new wxImage(640,480);
   m_pBackImage = new wxImage(640,480);
}

LiveImageArt::~LiveImageArt()
{
   if (m_pImage) delete m_pImage;
   if (m_pBackImage) delete m_pBackImage;
}

// return the pointer to the real image
wxImage*  LiveImageArt::ImagePtr() const {return m_pImage;};

// return the ptr to the Background image
wxImage* LiveImageArt::BackgPtr() const {return (m_drawSubframe)?m_pBackImage:NULL;}; // might be NULL !!

// set/get internal state
void LiveImageArt::SetFrameSize(const wxSize& size) {m_fullSize=size;};
void LiveImageArt::SetSubFrame(bool sub) {m_drawSubframe=sub;}
bool LiveImageArt::SubFrame() const {return m_drawSubframe;}

void LiveImageArt::SetSubFramePos(const wxPoint& leftTop) {m_subFrameLeftTop=leftTop;}

// returns validity of Image
bool LiveImageArt::ImageOK() const { return (m_pImage)?m_pImage->Ok():false; }
bool LiveImageArt::BackgImageOK() const { return (m_pBackImage)?m_pBackImage->Ok():false; }


// return image props
wxSize LiveImageArt::Size() const
{
   return (m_drawSubframe)? wxSize(m_pBackImage->GetWidth(), m_pBackImage->GetHeight()) \
                          : wxSize(m_pImage->GetWidth(), m_pImage->GetHeight());
}
wxPoint LiveImageArt::SubFrameLeftTop() const {return m_subFrameLeftTop;};

// triggers a change at the next Update call
void LiveImageArt::SetChangeImage() {
   m_imageChangeDue = true; // we have to change next time
}

// checks if a change is due - it destroys the old and creates a new one
bool LiveImageArt::UpdateImage(const wxSize& size, bool fullFrame)
{
   if (m_imageChangeDue) {
      if (fullFrame) {
         // set new framesize - might have bin changed
         SetFrameSize(size);
      }

      if (!fullFrame) {
         // next frame is subframe
         if (!m_drawSubframe) {
            // was full frame before - take a copy before it is gone
            if (m_pImage->Ok()) {
               *m_pBackImage = m_pImage->Blur(5);
            }
         }
         else {
            // was also subframe before
            // .. but we check if we have a changed fullframe size i.e. bin change
            if (   (m_pBackImage->GetWidth()!=m_fullSize.GetWidth())
                || (m_pBackImage->GetHeight()!=m_fullSize.GetHeight()) ) {
               // yes it has changed; as we are in subframe already we just create a new back background image
               // at the next occasion we get the blur copy above
               m_pBackImage->Create(m_fullSize.GetWidth(), m_fullSize.GetHeight());
            }
         }
      }
      // now create the new live Image according to arguments
      m_pImage->Create(size.GetWidth(), size.GetHeight());
      m_drawSubframe = (! fullFrame);
      m_imageChangeDue = false; // not longer
      return true;
   }
   else {
      return false;
   }
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// should never be called ...
wxArtcam::wxArtcam()
: wxCamera()
, m_ds(NULL)
, m_pArtSample(NULL)
, m_liveImages(NULL)
, m_timer(NULL)
, m_cooledCam(false)
, m_warmupCam(false)
{
}

// regular constructor of the camera
wxArtcam::wxArtcam(wxWindow* parent, wxWindowID id)
: wxCamera(parent, id)
, m_ds(NULL)
, m_pArtSample(new wxArtSample())
, m_liveImages(new LiveImageArt)
, m_sampleFormat(EVF_Unknown)
, m_driver_idx(-1)
, m_timer(this)
, m_leMilliseconds(2000)
, m_startupLE(false)
, m_cooledCam(false)
, m_warmupCam(false)
{
#ifdef _DEBUG
  DebHelp::SetVerboseLog();
#endif

/// IN ANY CASE FOR THE TIME BEEING
  if (wxGetEnv(wxT("wxAC_LOG"), NULL) ) DebHelp::SetVerboseLog();
  if (wxGetEnv(wxT("wxAC_DEBUG"), NULL) ) DebHelp::SetVerboseDebug();


   SetBackgroundStyle(wxBG_STYLE_CUSTOM); // used with double buffered drawing (see manual)
   m_camCoord = new wxArtCoords;
   m_whitePoint = 0xffff; // artemis WP is 16bit sample
   m_ds = new ArtBase();
}

wxArtcam::~wxArtcam()
{
   // disconnect the camera
   StopCapture();
   DisconnectCamera();
   if (m_ds) delete m_ds; m_ds = NULL;
   if (m_liveImages) delete m_liveImages;
   if (m_pArtSample) delete m_pArtSample;
}

wxString wxArtcam::VideoTechnology() const
{
   return m_ds->VideoTechnology();
}

size_t wxArtcam::EnumerateCameras(wxArrayString& cameras)
{
   // simply make an array of camera names to choose from
 // Do not clear contents here ! cameras.clear();
   ArtBase::EnumerateArtCameras();
   int num_items =  ArtBase::NumItems();
   if ( num_items >0 ) {
      for(int item=0; item<num_items; item++) {
         const ArtDevice& descr = ArtBase::ArtDevEntry(item);
         // add camera friendly name to list
         cameras.Add(descr.DevName());
      }
   }
   return cameras.size();
}

bool wxArtcam::SelectCamera(const wxString& camera)
{
   // set index and connect to the camera corresponding to "camera"

   m_driver_idx = -1;
   // reenumerate
   ArtBase::EnumerateArtCameras();
   int num_items =  ArtBase::NumItems();
   for(int item=0; item<num_items; item++) {
      // get camera friendly name
      const ArtDevice& descr = ArtBase::ArtDevEntry(item);
      wxString name = descr.DevName();
      if(name == camera) {
         // ok, we have a match, connect to that camera
         if(m_ds->ControlDevice(item)) {
           m_driver_idx = item;
         }
         break;
      }
   }
   return (m_driver_idx >= 0);
}

// intended to be called prior to OS specific connect cam
bool wxArtcam::ConnectCamera(const wxString& camera)
{
   // make sure the given camera is selected
   if ( SelectCamera(camera) ) {
      m_ds->SetFormat(m_default_binFactor);
      SetFullFrame();
      m_timer.Start(100);     // start the cam timer obj with N ms polling
      bool retVal = StartPreview();
      if (retVal) {
         unsigned short ccdw, ccdh;
         m_ds->CcdDimension(ccdw, ccdh);
         m_camCoord->SetCCDSize(ccdw, ccdh); // should be set only once on connect
         m_cooledCam=m_ds->HasCooling();
         m_warmupCam=m_ds->HasWarmup();
         m_isConnected = true;
      }
      return retVal;
   }
   return false; // ERROR EXIT
}

void wxArtcam::DisconnectCamera()
{
   m_timer.Stop();
   m_isConnected = false;

   m_cooledCam = false;
   m_warmupCam = false;

   m_ds->DropControlledDevice(); // drop cam object

   // this will destroy the current image (reveals as black screen)
   m_liveImages->SetChangeImage();
   m_liveImages->SetSubFrame(true);
   m_liveImages->SetSubFramePos(wxPoint(0,0));
   m_liveImages->UpdateImage(wxSize(640,480), true);
   Refresh();
}


wxString wxArtcam::CameraName() const
{
   wxString name = _T("No camera selected");
   if(m_ds) {
      const ArtDevice& descr = m_ds->ControlledArtDevEntry();
      name = descr.DevName();
   }

   return  name;
}


wxString wxArtcam::CameraSerial() const
{
   wxString txt = wxT("<no serial>");
   if(m_ds) {
      const ArtDevice& descr = m_ds->ControlledArtDevEntry();
      txt = descr.DevSerial();
   }
   return txt;
}


wxString wxArtcam::CameraStatus() const
{
   wxString status = _T(" (") + VideoTechnology();
   if(IsConnected()) {
      bool exposing;  float timeRemaining;
      m_ds->TimeRemaining(exposing, timeRemaining);
      if (exposing) status += wxT(": EXP - ");
      else          status += wxT(": DL% - ");
      status << long(timeRemaining);
      if(IsCapturing()) {
         status += wxT(" : CAPTURING");
      }
   }
   else {
      status += wxT(": DISCONNECTED");
   }
   status += wxT(")");
   return status;
}

   // cooling support
bool wxArtcam::HasCooling() const
{
   return m_cooledCam;
}

bool  wxArtcam::HasWarmup() const
{
   return m_warmupCam;
}

ECoolingStatus wxArtcam::CoolingStatus() const
{
   if ( !m_ds ) return ECS_NotAvailable;
   return m_ds->CoolingStatus();
}

float wxArtcam::CoolingSetpoint() const
{
   if ( !m_ds ) return 99.9f;
   return m_ds->CoolingSetpoint();
}

float wxArtcam::Temperature() const
{
   if ( !m_ds ) return 99.9f;
   return m_ds->Temperature();
}

bool  wxArtcam::SetTemperature(float temp)
{
   if ( !m_ds ) return false;
   return m_ds->SetTemperature(temp);
}

bool  wxArtcam::SetWarmup()
{
   if ( !m_warmupCam ) return false;
   if ( !m_ds ) return false;
   return m_ds->SetWarmup();
}

// set default binning, to be used with next connect
bool wxArtcam::SetDefaultBinning(unsigned short binFactor)
{
   m_default_binFactor = binFactor;

   return true;
}

unsigned short wxArtcam::DefaultBinning()
{
   return m_default_binFactor;
}

// set the camera format (binning factor) 1... max supported
bool wxArtcam::SetBinning(unsigned short binFactor)
{
   // we have to prepare for that
   if (m_isCapturing) return false; // not allowed while capturing though...

   m_fmtChange.SetBinning( (binFactor>MaxBinning()) ? MaxBinning() : binFactor );
   SetDefaultBinning(m_fmtChange.bin);
   return true;
}

// get the cameras max binning factor
unsigned short wxArtcam::MaxBinning() const
{
   if ( !m_ds ) return 1;
   return m_ds->MaxFormat();
}



// start the cam streaming (sort of)
bool wxArtcam::StartPreview()
{
   if ( !m_ds ) return false;

   // start the image stream

   // handle format changes
   if ( m_fmtChange.NeedsBinChange() ) {
      // bin change has higer prio otherwise things get messed up
      m_ds->SetFormat(m_fmtChange.bin);

      // we have to set the new FullFrameSize when binning changes
      // using a copy here, we shall not yet change anything in the global coord object
      wxArtCoords co(*dynamic_cast<wxArtCoords*>(m_camCoord)); co.SetBinFactor(m_fmtChange.bin); // new binfactor
      wxSize size(co.GetFullSize(wxCamCoords::EPO_Image));
      m_liveImages->SetFrameSize(size); // preset new framesize
      m_fmtChange.Reset(); // reset format change tracker
   }
   else if ( m_fmtChange.NeedsSubChange() ) {
      // we cannot change sub if bin was changed therefore else if
      m_ds->SetSubframe(m_fmtChange.s.x, m_fmtChange.s.y, m_fmtChange.e.x, m_fmtChange.e.y); // CCD coords
      m_fmtChange.Reset(); // reset format change tracker
   }

   // trigger to create a new image
   m_liveImages->SetChangeImage(); // creates a new image when needed (first time write)

   m_isConnected = true;
   return StartExpoCycle();   // now the timer is calling back to grab and display images
}



// start storing images into whatever storage type is requested
bool wxArtcam::StartCapture(const wxString& dirpath,
                            EVideoSampleFormat WXUNUSED(format),
                            EContainerFormat container,
                            bool batchCapture)

{
   if ( !m_isConnected ) return false;

   m_isCapturing = false; // don't start sample collection in timer callback too early
   m_isCapturePausing = false;
   m_captureError = false;
   m_batchCapture = batchCapture; //BM:20090329 - new support plain capture too
   m_frames_captured = 0; //BM:20090329 - reset counter ??!!

   if (m_imageStore) delete m_imageStore; m_imageStore = NULL;

   // store type depends on Setup parameter (later)
   if(container == ECF_ZIP_FITS) {
       m_imageStore = new wxImageStoreZippedFITS();
   }
   else if(container == ECF_FILES_FITS) {
      m_imageStore = new wxImageStoreFITS(); //20080821:BM new format
   }
   else if(container == ECF_AVI) {
      wxArrayString messages;
      messages.Add(wxT("The AVI container is not supported for Artemis capture"));
      messages.Add(wxT("Please choose a FITS type container in Settings."));
      wxMessageArea::wxMessageSink(messages,wxT("Artemis Camera - Start Capture"));
      return false; // unsupported container format
   }
   else {
      wxArrayString messages;
      messages.Add(wxT("Ooops - an unknown container format"));
      messages.Add(wxT("Please report it as 'bug' with the following information:"));
      messages.Add(wxString::Format(wxT("Unknown container format in: %s, Line >> %u"),__FILE__, __LINE__ ));
      wxMessageArea::wxMessageSink(messages,wxT("Artemis Camera - Start Capture"));

      return false; // unknown container format
   }

   // now start capture setup
   m_sampleFormat = EVF_YP16; // overwrite format given
   m_frames_captured = 0;

   int fps = 1;

   wxUint16 w,h;
   m_ds->FrameDimension(w,h); // here we need the real frame size of the cam image data
   m_imageStore->Create(dirpath, m_sampleFormat, EVC_Y16, wxSize(w,h), fps);

   if ( m_imageStore->IsReadyForCapture() ) {
      {//keyword handling
         // we use a list to add the image header given by the user
         // depending on the choosen store the keywords will be added in a proper way (..or not at all)
         wxFitsKeywordList* kwList = new wxFitsKeywordList(); // create on heap
         // now fill the list with keywords
         LoadKeywordList(kwList);
         // finally submit the header for this capture run
         m_imageStore->SetKeywordList(*kwList);
         delete kwList;
      }//end keyword handling

      m_isCapturing = true; // enable sample grabbing in the callback NOW
   }
   else {
      // failed to open the storage handler
      delete m_imageStore;
   }
   return m_isCapturing;
}


// close the storage handler
void wxArtcam::StopCapture()
{
   if ( !m_isConnected ) return;

   if(m_isCapturing) {
      if (m_imageStore) {
         m_imageStore->Close();
         delete m_imageStore;
         m_imageStore=NULL;
      }
   }
   m_isCapturing = false;
   m_isCapturePausing = false; // drop this one too
   m_captureError = false;
   m_batchCapture = false;
}

//20080819:BM new
void wxArtcam::PauseCapture()
{
   if (! m_isCapturing) return;      // not capturing
   if (m_isCapturePausing) return;   // already pausing ..

   m_isCapturePausing=true;
}

//20080819:BM new
void wxArtcam::ContinueCapture()
{
   if (! m_isCapturing) return;        // not capturing
   if (! m_isCapturePausing) return;   // not pausing ..

   m_isCapturePausing=false;
}


// abort the current take and start a new exposure (don't capture the aborted one)
void wxArtcam::RestartExposure()
{
   m_ds->AbortExposure();     // goes through OnCapture()
}


void wxArtcam::SetLEduration(unsigned long milliseconds)
{
   m_leMilliseconds = milliseconds;
}


// deals with the OnCapture Event when a new sample is ready
//  to be retrieved from stream
//    get a sample from the grabber
//    store it if needed
//    show it in GUI through bitBlt of wxImage
//    restart the capture cycle depending on mode (LE or not)
void wxArtcam::CaptureHandling()
{
   static wxUint16 blackLevel, whiteLevel; //BM:20090621 -  retain the levels throughout the existence


   if ( !m_isConnected ) return;

   long capResult = m_ds->OnCapture(); // allow the camcode to get the samples

   if (capResult>0) {
      //intermediate report e.g. for progress meter
      ;
   }
   else if (capResult==0) {
      // capture sample ready

      // m_pArtSample will allocated memory which can be used to submit a sample
      if (!m_pArtSample) {
         wxArrayString messages;
         messages.Add(wxT("Ooops - software error detected - exit and restart program - sorry!"));
         messages.Add(wxT("Please report it as 'bug' with the following information:"));
         messages.Add(wxString::Format(wxT("null ptr in: %s, Line >> %u"),__FILE__, __LINE__ ));
         wxMessageArea::wxMessageSink(messages,wxT("Artemis Camera - CaptureHandling"));
         return;
      }
      m_ds->CapturedSample(*m_pArtSample);  // we _must_ collect the sample if it is ready

      //BM:20090104 - add event when reaching 0
      {
         wxCommandEvent    cmdEvent(wxEVT_WXAC_TIMER, GetId());
         cmdEvent.SetEventObject(this);

         ::wxPostEvent(m_parent->GetEventHandler(), cmdEvent);
      }

      // Store the sample if needed
      if ( !m_pArtSample->DroppedSample() ) {
         if (m_isCapturing && (!m_isCapturePausing) && (!m_captureError) ) {
            // setup buffer and dump YUV bitmap into the file
            if (m_sampleFormat==EVF_YP16) {
               float ccdTemp = (m_ds->HasCooling()) ? m_ds->Temperature(): 100.0; // 100 will not report in FITS header
               //BM:20090621 - added timestamp handling - added submit size too
               //CA:20090623 - Added dependency on setting
               wxFactory::FrameTimeStampSize fts_size = wxF()->FrameTimeStampFontSize();
               switch (fts_size) {
                  case wxFactory::FTS_SMALL:
                     wxTimeFormatter::DrawTimeStamp(m_pArtSample->SampleSize(), m_pArtSample->SampleYPtr(),
                                                    m_pArtSample->TimeStamp(), blackLevel, whiteLevel,
                                                    wxTimeFont::EFS_Small);
                     break;
                  case wxFactory::FTS_MEDIUM:
                     wxTimeFormatter::DrawTimeStamp(m_pArtSample->SampleSize(), m_pArtSample->SampleYPtr(),
                                                    m_pArtSample->TimeStamp(), blackLevel, whiteLevel,
                                                    wxTimeFont::EFS_Mid);
                     break;
                  case wxFactory::FTS_OFF:
                  default:
                    ; // just nothing
               }//switch

               m_imageStore->AddYP16(m_pArtSample->SampleSize(),
                                    m_pArtSample->SampleYPtr(), m_pArtSample->SampleByteSizeY(),
                                    m_pArtSample->TimeStamp(), m_pArtSample->ExposureTime(), ccdTemp);
            }
            else {
               // TODO error handling if captured wrong or no format
            }
            m_frames_captured++;
            //BM:20090329 - only if batch handling is active
            if ( m_batchCapture ) {
               //BM:20090119 - add batch handling - part 1 finish old job
               BatchJobManager* bjm = wxF()->batchJobManager();
               if(bjm->JobAvailable()) {
                  BatchJob& bj = bjm->ActiveJob();
                  bj.OneFrameDone();
                  if(bj.FramesRemaining() <= 0) {
                       bjm->CommitJob();
                  }
                  // now we have either to continue the job or will get a new job later
               }
               //BM:20090104 - add event each time a frame is taken
               {
                  wxCommandEvent    cmdEvent(wxEVT_WXAC_FRAME, GetId());
                  cmdEvent.SetEventObject(this);

                  ::wxPostEvent(m_parent->GetEventHandler(), cmdEvent);
               }

               //BM:20090119 - add batch handling - part 2 start new job
               if(bjm->JobAvailable()) {
                  BatchJob& bj = bjm->ActiveJob();
                  if(bj.FramesRemaining() > 0) {
                     // time could have changed with a new job
                     unsigned long msec = (unsigned long)(1000*bj.ExpoTime());
                     SetLEduration(msec);
                  }
                  else {
                     // here we end with a new job that has no frames left ???
                     wxArrayString messages;
                     messages.Add(wxT("Empty Batch Job encountered - please report this as bug...."));
                     wxMessageArea::wxMessageSink(messages,wxT("Web Camera - CaptureHandling"));
                     StopCapture();  // no job anymore....
                  }
               }
               else {
                  StopCapture();  // no job anymore....
               }
            }// if batchHandling
         }// is capturing into storage

         wxPaintEvent    paintEvent; // call owner to redraw
         paintEvent.SetId(WXAC_PEVENT_ID_LE);
         // calc histogram
         Histogram().ComputePivotDev(m_pArtSample->SampleYPtr(), m_pArtSample->SampleSizeY());
         Histogram().ComputeHist(m_pArtSample->SampleYPtr(), m_pArtSample->SampleSizeY());
         blackLevel = Histogram().MinExposure(); //BM20090621 - get timestamp drawing levels here
         whiteLevel = Histogram().MaxExposure(); // .. we assume no BIG changes between images and use the last one above
         unsigned short p, vl, vh, m;
         Histogram().PivotDev(p, vl, vh, m);
         if (m_autoPoint) m_pArtSample->ConvertToBMP(m_gamma, vl, vh);
         else             m_pArtSample->ConvertToBMP(m_gamma, m_blackPoint, m_whitePoint);

         // manage images
         // get the right image (create a new if needed)
         UpdateImage(m_pArtSample->SampleSize(), m_pArtSample->FullFrame());
         if (m_liveImages->ImageOK()) {
            m_liveImages->ImagePtr()->SetData(m_pArtSample->DetachBMP()); // wxImage now owns the bitmap data
            // again our image coords are top/left and the CCD is bottom right oriented..
            // X/YPos are on the binned image (as if it was a 1/bin sized image)

            //! the one and only place to set the proper binning
            //  from a brand new live image which is about to be shown
            m_camCoord->SetBinFactor(m_pArtSample->SampleXBin(), m_pArtSample->SampleYBin());

            // translate from artSample to image space
            //  to DC it will be translated OnDraw as we don't know the zoom here
            wxCamPoint point(m_pArtSample->SamplePosition());
            m_camCoord->SetPoint(point, wxCamCoords::EPO_Sample);
            point = m_camCoord->GetPoint(wxCamCoords::EPO_Image);

            m_liveImages->SetSubFramePos(point.IntPoint());
            ZoomRedraw(m_zoomfact);
         }

         ::wxPostEvent(GetEventHandler(), paintEvent);
         // causes a paint event through the wx messaging system
      }// if not dropped

      // Restart cycle
      StartExpoCycle();
   }
   // error conditions
   else if (capResult==-1) {
      wxArrayString messages;
      messages.Add(wxT("Ooops - USB IO error - try disconnect and reconnect camera!"));
      messages.Add(wxString::Format(wxT("USB IO error in: %s, Line >> %u"),__FILE__, __LINE__ ));
      wxMessageArea::wxMessageSink(messages,wxT("Artemis Camera - CaptureHandling"));
   }
   else if (capResult==-2) {
      wxArrayString messages;
      messages.Add(wxT("Ooops - could not catch image - try disconnect and reconnect camera!"));
      messages.Add(wxString::Format(wxT("Could not catch image in: %s, Line >> %u"),__FILE__, __LINE__ ));
      wxMessageArea::wxMessageSink(messages,wxT("Artemis Camera - CaptureHandling"));
   }
   else if (capResult==-3) {
      wxArrayString messages;
      messages.Add(wxT("Ooops - software error detected - exit and restart program - sorry!"));
      messages.Add(wxString::Format(wxT("Software error detected in: %s, Line >> %u"),__FILE__, __LINE__ ));
      wxMessageArea::wxMessageSink(messages,wxT("Artemis Camera - CaptureHandling"));
   }
   else if (capResult==-4) {
      wxArrayString messages;
      messages.Add(wxT("Ooops - USB IO error - try disconnect and reconnect camera!"));
      messages.Add(wxString::Format(wxT("USB IO error in: %s, Line >> %u"),__FILE__, __LINE__ ));
      wxMessageArea::wxMessageSink(messages,wxT("Artemis Camera - CaptureHandling"));
   }
   else { //other error
      wxArrayString messages;
      messages.Add(wxT("Ooops - software error detected - exit and restart program - sorry!"));
      messages.Add(wxString::Format(wxT("Software error detected in: %s, Line >> %u"),__FILE__, __LINE__ ));
      wxMessageArea::wxMessageSink(messages,wxT("Artemis Camera - CaptureHandling"));
   }
}

/// //////////////////////////////////////////////////////////////////////////
/// CAM EXPOSURE SECTION
/// //////////////////////////////////////////////////////////////////////////

// starts one exposure cycle
// handling depends wether we are in LE or not
bool wxArtcam::StartExpoCycle()
{
   if ( !m_isConnected ) return false;

   if (m_fmtChange.NeedsBinChange() || m_fmtChange.NeedsSubChange() ) {
      // this calls StartExpoCycle() again so we have to leave this with the returned value
      bool retVal = StartPreview();
      return retVal;
   }
   else {
      m_pArtSample->MarkTimeStamp(); //BM:20090621 - load current time into the sample
      bool retVal  = m_ds->CaptureImage(true, m_leMilliseconds);
      if (!retVal) {
         DisconnectCamera();
         wxArrayString messages;
         messages.Add(wxT("Ooops - camera error - try disconnect and reconnect camera!"));
         messages.Add(wxString::Format(wxT("Camera error in: %s, Line >> %u"),__FILE__, __LINE__ ));
         wxMessageArea::wxMessageSink(messages,wxT("Artemis Camera - StartExpoCycle"));
      }
      return retVal;
   }
}


// the regular pacemaker
void wxArtcam::TimerEvent()
{
   ESYNMode syn = m_ds->OnCaptureRequired();
    // Data processing:
    // check if a sample has been deployed
   if (syn==ESYN_None) {
      if ( CAMERA_ERROR==m_ds->CaptureStatus() ) {
         DisconnectCamera();
         wxArrayString messages;
         messages.Add(wxT("Ooops - USB IO error camera disconnected - try to reconnect camera!"));
         messages.Add(wxString::Format(wxT("USB IO error in: %s, Line >> %u"),__FILE__, __LINE__ ));
         wxMessageArea::wxMessageSink(messages,wxT("Artemis Camera - TimerEvent"));
      }
   }

   if (syn>=ESYN_Capture) {
      CaptureHandling();
   }

   ::wxWakeUpIdle();    // added to make the status line update more actual
}


// returns seconds left until exposure ends
void wxArtcam::ProgressEvent(unsigned long WXUNUSED(secondsLeft))
{
   int i=0;
   i++;
}

ArtBase* wxArtcam::ds()
{
   return m_ds;
}


wxString wxArtcam::ConfigPath(const wxString& slotname)
{
   wxString path = _T("/cameras/") + CameraName() +_T("(")+  CameraSerial() + _T(")");
   if(slotname.Length() > 0) {
      path += _T("/") + slotname;
   }
   return path;
}


/// //////////////////////////////////////////////////////////////////////////
/// IMAGE DRAWING SECTION
/// //////////////////////////////////////////////////////////////////////////

wxImage* wxArtcam::liveImage() const
{
   // return a pointer to the relevant image buffer
   // i.e. a buffer guaranteed to contain the most recent live camera image
   return m_liveImages->ImagePtr();
}

bool wxArtcam::liveImageInverted() const
{
   return false;
}

wxSize wxArtcam::liveSize() const
{
   if (liveImage())
      return wxSize(liveImage()->GetWidth(), liveImage()->GetHeight());
   else
     return wxSize(0,0);

}

wxImage* wxArtcam::canvasImage() const
{
   if ( m_liveImages->BackgPtr() ) return m_liveImages->BackgPtr();
   else                            return m_liveImages->ImagePtr(); //20080819:BM fix return image if no back is there
}

wxSize wxArtcam::canvasSize() const
{
   if (canvasImage())
      return wxSize(canvasImage()->GetWidth(), canvasImage()->GetHeight());
   else
     return wxSize(0,0);

}

void wxArtcam::SetImageProperties(double gamma, bool autoPoint, double blackPoint, double whitePoint)
{
   wxCamera::SetImageProperties(gamma, autoPoint, blackPoint, whitePoint);

   if (!m_pArtSample) return;

   wxPaintEvent    paintEvent; // call owner to redraw
   paintEvent.SetId(WXAC_PEVENT_ID_LE);
   if (m_autoPoint) {
      // get auto values
      unsigned short p, vl, vh, m;
      Histogram().PivotDev(p, vl, vh, m);
      m_blackPoint = vl; m_whitePoint = vh;
   }
   if ( m_pArtSample->ConvertToBMP(m_gamma, m_blackPoint, m_whitePoint) ) {
      // only if Convert was successful
      if (m_liveImages->ImageOK()) {
         m_liveImages->ImagePtr()->SetData(m_pArtSample->DetachBMP()); // wxImage now owns the bitmap data
      }
      ::wxPostEvent(GetEventHandler(), paintEvent);
      // causes a paint event through the wx messaging system
   }
}

// checks if a change is due - it destroys the old and creates a new one
void wxArtcam::UpdateImage(const wxSize& size, bool fullFrame)
{
   bool changed = m_liveImages->UpdateImage(size, fullFrame);
   if ( changed ) {
      // enable support for images larger than the client area
      // by providing scrollbars. This is managed by ZoomRedraw.
      ZoomRedraw(m_zoomfact,true);
   }
}

void wxArtcam::ZoomRedraw(double zoomFact, bool redraw)
{
   // actual frame dimensions
   wxUint16 w,h;
   // returns always the fullframe size
   w = m_liveImages->Width();
   h = m_liveImages->Height();
   m_zoomfact = zoomFact;
   //! here is the one an only point to set the zoomfactor
   m_camCoord->SetZoomFactor(m_zoomfact);

   // map from Image to DC coordinates
   wxCamPoint size(w,h);
   m_camCoord->SetPoint(size, wxCamCoords::EPO_Image);
   size = m_camCoord->GetPoint(wxCamCoords::EPO_DC);

   int vir_w = int(size.x);
   int vir_h = int(size.y);

   SetVirtualSizeHints(vir_w/10, vir_h/10, vir_w, vir_h);
   SetVirtualSize(vir_w,vir_h);

   // client size
   wxClientDC dc(this);  // outside paint event use wxClientDC !!!
   PrepareDC(dc);
   wxCoord width,height;
   // get the size of the drawing area
   dc.GetSize(&width, &height);

   // set scrollbars on when client area is smaller than virtual image in one or both directions
   wxSize csize = wxSize(width,height);
   int rate_h = (vir_w > csize.GetWidth())? 1:0;
   int rate_w = (vir_h > csize.GetHeight())? 1:0;
   SetScrollRate(rate_h,rate_w);

   // redraw
   if(redraw)wxWindow::Refresh(false);
}

void wxArtcam::OnSize(wxSizeEvent& WXUNUSED(event))
{
//    m_ds->ScaleVideoOutput();
}

void wxArtcam::OnDraw(wxPaintEvent& event)
{
   // we _must_ show the m_wximage when in LE mode

   // called directly from capture handling
   if (event.GetId()==WXAC_PEVENT_ID_LE) {
      if(m_liveImages->ImageOK()) {
         wxClientDC ndc(this);  // outside paint event use wxClientDC !!!
         PrepareDC(ndc);
         wxSize   iSize(liveWidth(), liveHeight()); // scale (sub)frame
         wxBufferedDC dc(&ndc, iSize);  // outside OnPaint events
         OnDrawOverlay(dc);
      }
      else {
         // we are called by capture handler but dont have a valid image
         // just do nothing here
      }
   }
   else {
      // called via regular PAINT event
      if(m_liveImages->ImageOK()) {
         wxAutoBufferedPaintDC dc(this); //, wxBUFFER_VIRTUAL_AREA );  // inside OnPaint events
         PrepareDC(dc); // even so the Buffered should not require Prepare this one does
         OnDrawOverlay(dc);
      }
      else {
         // we don't have a valid image to draw right now
         event.Skip();  // allow others - GUI will not be responsive otherwise.....
      }
   }//else
}

void wxArtcam::OnDrawOverlay(wxDC& dc)
{
   // draw overlay symbols using the already prepared DC
   wxCoord width,height;
   dc.GetSize(&width, &height);
   wxImage* image = liveImage();

   OnDrawImage(dc,image,true); // we always draw the image (removed callers DrawImage call)
   OnDrawReticle(dc,true);
   OnDrawRubberband(dc);
   if (m_isCapturing) OnDrawCapturedFrames(dc); //20080823:BM draw label for Art too
}

void wxArtcam::OnDrawImage(wxDC& dc, wxImage* image, bool zoomed_view)
{

   if(image && image->IsOk()) {

      // get the size of the client area
      wxCoord width,height;
      dc.GetSize(&width, &height);

      if(zoomed_view) {
         // scale the image based the zoom factor
         int vir_width, vir_height;

         if (m_liveImages->SubFrame()) {
            // we have to draw a background image
            // map from Image to DC coordinates
            wxCamPoint size(m_liveImages->Width(), m_liveImages->Height());
            m_camCoord->SetPoint(size, wxCamCoords::EPO_Image);
            size = m_camCoord->GetPoint(wxCamCoords::EPO_DC);

            vir_width  = int(size.x);
            vir_height = int(size.y);

            dc.DrawBitmap(m_liveImages->BackgPtr()->Scale(vir_width,vir_height),0,0);
            // clean undrawn area
            dc.SetPen(*wxBLACK_PEN);
            dc.SetBrush(*wxBLACK_BRUSH);
            dc.DrawRectangle(0,vir_height,width,height);
            dc.DrawRectangle(vir_width,0,width,height);
            // reset resources used
            dc.SetBrush(wxNullBrush);
            dc.SetPen(wxNullPen);
         }

         // map from Image to DC coordinates
         wxCamPoint size(liveImage()->GetWidth(), liveImage()->GetHeight());
         m_camCoord->SetPoint(size, wxCamCoords::EPO_Image);
         size = m_camCoord->GetPoint(wxCamCoords::EPO_DC);

         vir_width  = int(size.x);
         vir_height = int(size.y);

         wxCamPoint point(m_liveImages->SubFrameLeft(), m_liveImages->SubFrameTop());
         m_camCoord->SetPoint(point, wxCamCoords::EPO_Image);
         point = m_camCoord->GetPoint(wxCamCoords::EPO_DC);

         int vir_left = int(point.x);
         int vir_top  = int(point.y);
         dc.DrawBitmap(image->Scale(vir_width,vir_height), vir_left, vir_top);
         if (m_liveImages->SubFrame()) {
            // we draw a rectangle around the subframe
            dc.SetPen(*wxCYAN_PEN);
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.DrawRectangle(vir_left, vir_top, vir_width,vir_height);
         }
         else {
            // clean undrawn area if there was no background to draw
            dc.SetPen(*wxBLACK_PEN);
            dc.SetBrush(*wxBLACK_BRUSH);
            dc.DrawRectangle(0,vir_height,width,height);
            dc.DrawRectangle(vir_width,0,width,height);
         }
         // reset resources used
         dc.SetBrush(wxNullBrush);
         dc.SetPen(wxNullPen);
     }
      else {
         wxASSERT(false);
      }
   }
}


/// //////////////////////////////////////////////////////////////////////////
/// SUBFRAME SECTION
/// //////////////////////////////////////////////////////////////////////////


void wxArtcam::SetFullFrame()
{
   m_fmtChange.SetSubframe(wxPoint(1,1), wxPoint(1,1));
   // this will only set a note to change but does not change immediately
}

// set the subframe format (binning factor 1 assumed)
//  needs two points of a rectangle that makes the subframe in pixel coords
//  the subframe might get adjusted if binning changes
//  switching back to the max frame is done by setting all points to 1
bool wxArtcam::SetSubframe(const wxPoint& start, const wxPoint& end)
{
   // we have to prepare for that
   if (m_isCapturing) return false; // not allowed while capturing though...

   m_fmtChange.SetSubframe(start, end);
   return true;
}

void wxArtcam::FinalizeSubframe()
{
  // map from Image to CCD coordinates (mouseRect calculates de-zoomed Image coords)
   wxCamPoint startPoint(m_mouseRect.StartPoint());
   m_camCoord->SetPoint(startPoint, wxCamCoords::EPO_Image);
   startPoint = m_camCoord->GetPoint(wxCamCoords::EPO_CCD);

   wxCamPoint endPoint(m_mouseRect.EndPoint());
   m_camCoord->SetPoint(endPoint, wxCamCoords::EPO_Image);
   endPoint = m_camCoord->GetPoint(wxCamCoords::EPO_CCD);

   m_fmtChange.SetSubframe(startPoint.IntPoint(), endPoint.IntPoint());
   ZoomRedraw(m_zoomfact);

   wxCamera::FinalizeSubframe();
}

/// //////////////////////////////////////////////////////////////////////////
/// FITS SECTION
/// //////////////////////////////////////////////////////////////////////////

// used to populate the keywordlist from setting dialog
void wxArtcam::LoadKeywordList(wxFitsKeywordList* kwdList)
{
   if (!kwdList) return;

   wxFactory* factory = wxFactory::singleton();
   factory->getFitsKeywordList(*kwdList);
}

EVideoSampleFormat wxArtcam::VideoSampleFormat()
{
   return m_sampleFormat;
}
