 // wxWebcam.cpp: implementation of the wxWebcam class.
//
//////////////////////////////////////////////////////////////////////

#include "wxWebcam.h"
#include "wxWebcamWin32DS.h"
#include "wxWebcamV4L2.h"


#include <wx/dcbuffer.h>
#include <wx/font.h>
#include <wx/datetime.h>

#include "wxImageStore.h"
#include "wxWebCoords.h"
#include "wxTimeFormatter.h"

//* LE stuff
#include "../wxIO/wxLongExpoPort.h"
#include "../wxIO/wxPort.h"
#include "../wxFactory.h"
#include "../wxMessageArea.h"
#include "../BatchJobManager.h"


#include "../GenericDebHelp.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_ABSTRACT_CLASS(wxWebcam,wxScrolledWindow)

unsigned short wxWebcam::m_default_binFactor = 1;

wxWebcam* wxWebcam::Construct(wxWindow* parent, wxWindowID id)
{
#ifdef WIN32
    return new wxWebcamWin32DS(parent,id);
#else  // LINUX
     return new wxWebcamV4L2(parent,id);
#endif
}


// helper class for handling of live images
class LiveImageWeb
{
public:
   LiveImageWeb();
   ~LiveImageWeb();

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
   // returns the subframe props
   wxSize SubFrameSize() const;
   int SubFrameWidth() const {return SubFrameSize().GetWidth();}
   int SubFrameHeight() const {return SubFrameSize().GetHeight();}
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



LiveImageWeb::LiveImageWeb()
: m_pImage(NULL)
, m_pBackImage(NULL)
, m_imageChangeDue(false)
, m_drawSubframe(false)
{
   m_pImage = new wxImage(640,480);
   m_pBackImage = new wxImage(640,480);
}

LiveImageWeb::~LiveImageWeb()
{
   if (m_pImage) delete m_pImage;
   if (m_pBackImage) delete m_pBackImage;
}

// return the pointer to the real image
wxImage*  LiveImageWeb::ImagePtr() const {return m_pImage;};

// return the ptr to the Background image
wxImage* LiveImageWeb::BackgPtr() const {return (m_drawSubframe)?m_pBackImage:NULL;}; // might be NULL !!

// set/get internal state
void LiveImageWeb::SetFrameSize(const wxSize& size) {m_fullSize=size;};
void LiveImageWeb::SetSubFrame(bool sub) {m_drawSubframe=sub;}
bool LiveImageWeb::SubFrame() const {return m_drawSubframe;}

void LiveImageWeb::SetSubFramePos(const wxPoint& leftTop) {m_subFrameLeftTop=leftTop;}

// returns validity of Image
bool LiveImageWeb::ImageOK() const { return (m_pImage)?m_pImage->Ok():false; }
bool LiveImageWeb::BackgImageOK() const { return (m_pBackImage)?m_pBackImage->Ok():false; }


// return image props
wxSize LiveImageWeb::Size() const
{
   return (m_drawSubframe)? wxSize(m_pBackImage->GetWidth(), m_pBackImage->GetHeight()) \
                          : wxSize(m_pImage->GetWidth(), m_pImage->GetHeight());
}
wxPoint LiveImageWeb::SubFrameLeftTop() const {return m_subFrameLeftTop;};
wxSize LiveImageWeb::SubFrameSize() const
{
   return  wxSize(m_pImage->GetWidth(), m_pImage->GetHeight());
}

// triggers a change at the next Update call
void LiveImageWeb::SetChangeImage() {
   m_imageChangeDue = true; // we have to change next time
}

// checks if a change is due - it destroys the old and creates a new one
bool LiveImageWeb::UpdateImage(const wxSize& size, bool fullFrame)
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
// Construction/Destruction  wxWebcam
//////////////////////////////////////////////////////////////////////

wxWebcam::wxWebcam()
: wxCamera()
, m_ds(NULL)
, m_timer(NULL)
, m_pLongExpoPort(NULL)
, m_overlayGuideBox(0,0,0,0)
, m_overlayGuideBoxDraw(false)
{}

wxWebcam::wxWebcam(wxWindow* parent, wxWindowID id)
: wxCamera(parent, id)
, m_ds(NULL)
, m_pProperties(NULL)
, m_pWebSample(new wxWebSample)
, m_pLiveImages(new LiveImageWeb)
, m_preview(false)
, m_dropSample(false)
, m_sampleFormat(EVF_Unknown)
, m_driver_idx(-1)
, m_timer(this)
, m_pLongExpoPort(NULL)
, m_isLExposing(false)
, m_capDelay(1)
, m_leMilliseconds(2000)
, m_doReadoutLE(false)
, m_lxSecLeft(0)
, m_overlayGuideBox(0,0,0,0)
, m_overlayGuideBoxDraw(false)
, m_fps(0)
, m_dps(0)
{

#ifdef _DEBUG
  DebHelp::SetVerboseLog();
#endif

/// IN ANY CASE FOR THE TIME BEEING
  if (wxGetEnv(wxT("wxAC_LOG"), NULL) ) DebHelp::SetVerboseLog();
  if (wxGetEnv(wxT("wxAC_DEBUG"), NULL) ) DebHelp::SetVerboseDebug();


   SetBackgroundStyle(wxBG_STYLE_CUSTOM); // used with double buffered drawing (see manual)
   m_camCoord = new wxWebCoords;
   m_whitePoint = 0xff; // webcam WP is 8bit sample
}

wxWebcam::~wxWebcam()
{
   // disconnect the camera
   DisconnectCamera();

   if (m_pProperties) delete m_pProperties;
   if (m_ds) delete m_ds;
   if (m_pWebSample) delete m_pWebSample;
   if (m_pLiveImages) delete m_pLiveImages;
}


size_t wxWebcam::EnumerateCameras(wxArrayString& cameras)
{
   // simply make an array of camera names to choose from
  // DO NOT CLEAR CONTENTS HERE cameras.clear();
   m_ds->EnumerateCapFilter();
   int num_items =  m_ds->NumItems();
   if ( num_items >0 ) {
      for(int item=0; item<num_items; item++) {
         const CapDeviceEntry& descr = m_ds->CapDevEntry(item);
         // add camera friendly name to list
         cameras.Add(descr.DevName());
      }
   }
   return cameras.size();
}

bool wxWebcam::SelectCamera(const wxString& camera)
{
   // set index and connect to the camera corresponding to "camera"
   m_driver_idx = -1;
   m_ds->EnumerateCapFilter();
   size_t num_items =  m_ds->NumItems();
   for(size_t item=0; item<num_items; item++) {

      // get camera friendly name
      const CapDeviceEntry& descr = m_ds->CapDevEntry(item);
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
bool wxWebcam::ConnectCamera(const wxString& camera)
{
   // make sure that LE is NOT checked
   m_doReadoutLE = false;
   //* TODO
   // may be other generic stuff
   //* TODO
   // make sure the given camera is selected
   SelectCamera(camera);
   if ( m_driver_idx>=0 && (m_driver_idx<(int)m_ds->NumItems()) ) {

      // create the capture graph, and start previewing
      bool retVal = false;
      // this depends on implementation right now
      if (m_ds->Implementation()==EOSK_Win32) {
         retVal = m_ds->CreateNew((long)GetHandle());
      }
      else if (m_ds->Implementation()==EOSK_Linux) {
         retVal = m_ds->CreateNew((long)GetHandle());
      }
      if (retVal){
         m_ds->SetFormat(1); // must be 1 (fullsize) to get the real max ccd dimensions
         m_timer.Start(wxF()->PerformanceLevel());   //20100703:BM - from user setting- triggers readout of cam
         m_isLExposing = false;
         bool retval = StartPreview();
         if (retVal) {
            unsigned short ccdw, ccdh;
            m_ds->FrameDimension(ccdw, ccdh);
            m_camCoord->SetCCDSize(ccdw, ccdh); // should be set only once on connect
         }
         // apply selected default property settings if any
         // must do this after completing the preview setup (it also declares the connection true)
         RestoreDefaultSlot(wxFactory::singleton()->config());
         m_captureError=false;
         return retval;
      }
   }
   return false; // ERROR EXIT
}

bool wxWebcam::RestoreDefaultSlot(wxConfig* config)
{
   if(!config)return false;

   bool retval = false;
   wxString defaultSlot;
   wxString OldPath = config->GetPath();
   config->SetPath(ConfigPath(_T("")));
   if(config->Read(_T("DefaultSlot"),&defaultSlot,_T(""))) {
      // there is a default slot, but does it really exist?
      if(config->HasGroup(defaultSlot)) {
         // ok the default slot exists!
         // restore it in the camera
         config->SetPath(ConfigPath(defaultSlot));
         retval = RestoreFromConfig(config);
      }
   }
   config->SetPath(OldPath);
   return retval;
}

void wxWebcam::DisconnectCamera()
{
   m_ds->CaptureStream(false, 0); // we can use CaptureStream or Collect to stop it
   StopCapture();
   SwitchLE(false);

   m_ds->Stop();
   m_timer.Stop();
   m_isConnected = false;

   m_ds->DropControlledDevice(); // drop cam object
//   m_streamImage.Destroy();

   cancelGuideBox();

   // this will destroy the current image (reveals as black screen)
   m_pLiveImages->SetChangeImage();
   m_pLiveImages->SetSubFrame(true);
   m_pLiveImages->SetSubFramePos(wxPoint(0,0));
   m_pLiveImages->UpdateImage(wxSize(640,480), true);
   Refresh();
}


wxString wxWebcam::CameraName() const
{
   wxString name = _T("No camera selected");
   if(m_driver_idx >= 0) {

      const CapDeviceEntry& descr = m_ds->CapDevEntry(m_driver_idx);
      name = descr.DevName();
   }

   return  name;
}

wxString wxWebcam::CameraStatus() const
{
   wxString status = _T(" (") + VideoTechnology();
   if(IsConnected()) {
      if (m_isLExposing) {
         status += wxT(": LX - ");
         status << m_lxSecLeft;
      }
      else {
         status += wxT(": STREAMING");
      }
      if(IsCapturing()) {
         if (m_captureError) status += wxT(" : CAP-ERROR");
         else                status += wxT(" : CAPTURING");

      }
   }
   else {
      status += wxT(": DISCONNECTED");
   }
   status += wxT(")");
   return status;
}


wxString wxWebcam::CameraSerial() const
{
   wxString txt = wxT("<no serial>");
   if(m_driver_idx >= 0) {

      const CapDeviceEntry& descr = m_ds->CapDevEntry(m_driver_idx);
      txt = descr.DevSerial();
   }
   return txt;
}


// start the cam streaming in previously set LE mode
bool wxWebcam::StartPreview()
{
   m_lxSecLeft = 0;

   // start the image stream

   // handle format changes
   if ( m_fmtChange.NeedsBinChange() ) {
      // bin change has higer prio otherwise things get messed up
//      m_ds->SetFormat(m_fmtChange.bin);     // NO BINNING WITH WEBCAMS

      // we have to set the new FullFrameSize when binning changes
      // using a copy here, we shall not yet change anything in the global coord object
      wxWebCoords co(*dynamic_cast<wxWebCoords*>(m_camCoord)); co.SetBinFactor(m_fmtChange.bin); // new binfactor
      wxSize size(co.GetFullSize(wxCamCoords::EPO_Image));
      m_pLiveImages->SetFrameSize(size); // preset new framesize
      m_fmtChange.Reset(); // reset format change tracker
   }
   else if ( m_fmtChange.NeedsSubChange() ) {
      // we cannot change sub if bin was changed therefore else if
      m_ds->SetSubframe(m_fmtChange.s.x, m_fmtChange.s.y, m_fmtChange.e.x, m_fmtChange.e.y); // CCD coords
      m_fmtChange.Reset(); // reset format change tracker
   }

   // trigger to create a new image
   m_pLiveImages->SetChangeImage(); // creates a new image when needed (first time write)

   // start the image stream
   m_isConnected = m_ds->Run();

   if (m_isConnected) {
      if (m_isLExposing) {
         // Always set camera to 5fps on startup in case previous
         // run was locked due to 15fps problem.
         // 5fps is also useful for long exposure
         int fps_5 = 0;
         properties()->SetProperty_FPS(fps_5);
      }
      m_preview = !m_isLExposing;
      // start capturing images through the sample grabber
      return StartExpoCycle();   // now the timer is calling back to grab and display images
   }
   else {
      return false;
   }
}


/// //////////////////////////////////////////////////////////////////////////
/// IMAGE CAPTURING SECTION
/// //////////////////////////////////////////////////////////////////////////

// start storing images into whatever storage type is requested
bool wxWebcam::StartCapture(const wxString& dirpath,
                            EVideoSampleFormat format,
                            EContainerFormat container,
                            bool batchCapture)
{
   m_isCapturing = false; // don't start sample collection in timer callback too early
   m_isCapturePausing = false;
   m_captureError = false;
   m_batchCapture = batchCapture; //BM:20090329 - new support plain capture too
   m_frames_captured = 0; //BM:20090329 - reset counter ??!!

   if (m_imageStore) delete m_imageStore; m_imageStore = NULL;

   // store type depends on Setup parameter (later)
   if(container == ECF_AVI)            m_imageStore = new wxImageStoreAVI();
   else if(container == ECF_ZIP_FITS)  m_imageStore = new wxImageStoreZippedFITS();
   else if(container == ECF_FILES_FITS) m_imageStore = new wxImageStoreFITS(); //20080821:BM new format
   else {
      wxArrayString messages;
      messages.Add(wxT("Ooops - an unknown container format"));
      messages.Add(wxT("Please report it as 'bug' with the following information:"));
      messages.Add(wxString::Format(wxT("Unknown container format in: %s, Line >> %u"),__FILE__, __LINE__ ));
      wxMessageArea::wxMessageSink(messages,wxT("Web Camera - Start Capture"));

      return false; // unknown container format
   }

   // now start capture setup
   m_sampleFormat = format;
   m_frames_captured = 0;

   // start capturing with a play rate of 1FPSec
   int fps = 5;
   // the fudge factor is set so that replay takes about
   // the same elapsed time as orignal capture
   // for some reason 10fps only gives 7fps captured
   int fudge_factor=10;

   switch(properties()->Property_FPS()) {
      case 0: { fps = 5 ; fudge_factor=10; break; }
      case 1: { fps = 10; fudge_factor=7; break; }
      case 2: { fps = 15; fudge_factor=10; break; }
      case 3: { fps = 20; fudge_factor=10; break; }
      case 4: { fps = 25; fudge_factor=10; break; }
      case 5: { fps = 30; fudge_factor=10; break; }
      default: { }
   }
   if (m_ds->Implementation()==EOSK_Win32) {
      fudge_factor=10;  // Must do this better
   }

   unsigned short w,h;
   m_ds->FrameDimension(w,h);
   // get dims from real image
   wxImage* image = liveImage();
   wxSize frameSize(image->GetWidth(), image->GetHeight());


   m_imageStore->Create(dirpath, m_sampleFormat, m_ds->WCFConversion(), frameSize, fps*fudge_factor);

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

      m_dropSample = true;  // we have to drop one to make sure we do not mix up formats in the store
      m_isCapturing = true; // enable sample grabbing in the callback NOW
   }
   else {
      // failed to open the storage handler
      delete m_imageStore; m_imageStore=NULL; //20101018:BM - fix leaving non null ptr...
   }
   return m_isCapturing;
}

// close the storage handler
void wxWebcam::StopCapture()
{
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
void wxWebcam::PauseCapture()
{
   if (! m_isCapturing) return;      // not capturing
   if (m_isCapturePausing) return;   // already pausing ..

   m_isCapturePausing=true;
}

//20080819:BM new
void wxWebcam::ContinueCapture()
{
   if (! m_isCapturing) return;        // not capturing
   if (! m_isCapturePausing) return;   // not pausing ..

   m_isCapturePausing=false;
}


// deals with the OnCapture Event when a new sample is ready
//  to be retrieved from stream
//    get a sample from the grabber
//    store it if needed
//    show it in GUI through bitBlt of wxImage
//    restart the capture cycle depending on mode (LE or not)
void wxWebcam::CaptureHandling()
{

   long capResult = m_ds->OnCapture(); // allow the camcode to get the samples

   if (capResult>0) {
      //intermediate report e.g. for progress meter
      ;
   }
   else if (capResult==0) {


       // capture sample ready
      if (m_isLExposing) {
         _V_ ::fprintf(stderr, "wxWebcam::CaptureHandling() - capture LE sample \n");
         if(m_pLongExpoPort)m_pLongExpoPort->EndExposure();  // finish LE cycle
      }
      else {
         // if not LE we measure the time since start cycle
         m_realExpoTime = m_timer.GetRealExposureSeconds();
      }

      // cs will allocated memory which can be used to submit a sample
      // m_artSample will allocated memory which can be used to submit a sample
      if (!m_pWebSample) {
         wxArrayString messages;
         messages.Add(wxT("Ooops - software error detected - exit and restart program - sorry!"));
         messages.Add(wxT("Please report it as 'bug' with the following information:"));
         messages.Add(wxString::Format(wxT("null ptr in: %s, Line >> %u"),__FILE__, __LINE__ ));
         wxMessageArea::wxMessageSink(messages,wxT("Web Camera - CaptureHandling"));
         return;
      }
      m_ds->CapturedSample(*m_pWebSample);  // we _must_ collect the sample if it is ready

     m_lxSecLeft=0; //BM:20090104 - added
     //BM:20090104 - add event when reaching 0
      {
         wxCommandEvent    cmdEvent(wxEVT_WXAC_TIMER, GetId());
         cmdEvent.SetEventObject(this);

         ::wxPostEvent(m_parent->GetEventHandler(), cmdEvent);
      }

      // Store the sample if needed
      if ( (!m_dropSample)  && m_isCapturing && (!m_isCapturePausing) && (!m_captureError) ) {
         // setup buffer and dump YUV bitmap into the file
         if (m_sampleFormat==EVF_YUV) {
            //20080525:BM must check if YUV is available at all (could be not if RGB streaming is up e.g. USB2 cams)
            if (m_pWebSample->SampleSizeYuv()>0) {
               //BM:20090621 - added timestamp handling - added submit size too
               //CA:20090623 - Added dependency on setting
               wxFactory::FrameTimeStampSize fts_size = wxF()->FrameTimeStampFontSize();
               switch (fts_size) {
                  case wxFactory::FTS_SMALL:
                  wxTimeFormatter::DrawTimeStamp(m_pWebSample->SampleHeaderYUVPtr(), m_pWebSample->SampleYUVPtr(),
                                                 m_pWebSample->TimeStamp(), wxTimeFont::EFS_Small);
                  break;
                  case wxFactory::FTS_MEDIUM:
                  wxTimeFormatter::DrawTimeStamp(m_pWebSample->SampleHeaderYUVPtr(), m_pWebSample->SampleYUVPtr(),
                                                 m_pWebSample->TimeStamp(), wxTimeFont::EFS_Mid);
                  break;
                  case wxFactory::FTS_OFF:
                  default:
                    ; // just nothing
               }//switch

               m_imageStore->AddBMP(m_pWebSample->SampleHeaderYUVPtr(), m_pWebSample->SampleYUVPtr(),
                                    m_pWebSample->SampleSizeYuv(), m_pWebSample->TimeStamp(), m_realExpoTime);
            }
            else { //20080525:BM Complain about improper setting... (cannot save RGB into a YUV AVI)
               wxArrayString messages;
               messages.Add(wxT("Options-Settings is YUV420 but only RGB is available !"));
               messages.Add(wxT("Please change Setting to RGB24 and restart Capture."));
               wxMessageArea::wxMessageSink(messages,wxT("Web Camera - CaptureHandling"));
               m_captureError=true;
            }
         }
         else if (m_sampleFormat==EVF_RGB24) {
            //BM:20090621 - added timestamp handling - added submit size too
            //CA:20090623 - Added dependency on setting
            wxFactory::FrameTimeStampSize fts_size = wxF()->FrameTimeStampFontSize();
            switch (fts_size) {
               case wxFactory::FTS_SMALL:
               wxTimeFormatter::DrawTimeStamp(m_pWebSample->SampleHeaderBGRPtr(), m_pWebSample->SampleBGRPtr(),
                                              m_pWebSample->TimeStamp(), wxTimeFont::EFS_Small);
               break;
               case wxFactory::FTS_MEDIUM:
               wxTimeFormatter::DrawTimeStamp(m_pWebSample->SampleHeaderBGRPtr(), m_pWebSample->SampleBGRPtr(),
                                             m_pWebSample->TimeStamp(), wxTimeFont::EFS_Mid);
               break;
               case wxFactory::FTS_OFF:
               default:
                 ; // just nothing
            }//switch

            // setup buffer and dump BGR bitmap into the file
            m_imageStore->AddBMP(m_pWebSample->SampleHeaderBGRPtr(), m_pWebSample->SampleBGRPtr(),
                                    m_pWebSample->SampleSizeBgr(), m_pWebSample->TimeStamp(), m_realExpoTime);
         }
         else {
            wxArrayString messages;
            messages.Add(wxT("Ooops - Unknown capture format (bug?) - Restart Capture and complain if it persists!"));
            messages.Add(wxT("Please report it as 'bug' with the following information:"));
            messages.Add(wxString::Format(wxT("Unknown capture format in: %s, Line >> %u"),__FILE__, __LINE__ ));
            wxMessageArea::wxMessageSink(messages,wxT("Web Camera - CaptureHandling"));
            m_captureError=true;
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
      m_dropSample = false; // this is a one shot trigger

      // calc histogram as long as BGR is still available
      Histogram().ComputeHist(m_pWebSample->SampleBGRPtr(), m_pWebSample->SampleSizeBgr()/3);

      // Prepare to Paint the captured sample if in LE mode
      wxPaintEvent    paintEvent; // call owner to redraw
      paintEvent.SetId(WXAC_PEVENT_ID_LE);

      // manage images
      // get the right image (create a new if needed)
      UpdateImage(m_pWebSample->SampleSize(), m_pWebSample->FullFrame());
      if (m_pLiveImages->ImageOK()) {
         m_pWebSample->ConvertBGR2RGB();    //wsImage needs a RGB ordered bitmap
         m_pLiveImages->ImagePtr()->SetData(m_pWebSample->DetachBMP()); // wxImage now owns the bitmap data
         // again our image coords are top/left and the CCD is bottom right oriented..
         // X/YPos are on the binned image (as if it was a 1/bin sized image)

         //! the one and only place to set the proper binning
         //  from a brand new live image which is about to be shown
         //! NO BINNING for webcams...

         // translate from artSample to image space
         //  to DC it will be translated OnDraw as we don't know the zoom here
         wxCamPoint point(m_pWebSample->SamplePosition());
         m_camCoord->SetPoint(point, wxCamCoords::EPO_Sample);
         point = m_camCoord->GetPoint(wxCamCoords::EPO_Image);

         m_pLiveImages->SetSubFramePos(point.IntPoint());
         ZoomRedraw(m_zoomfact);
      }

      ::wxPostEvent(GetEventHandler(), paintEvent);
      // causes a paint event through the wx messaging system

      // Restart cycle
      if ( !StartExpoCycle() ){
         if (m_isLExposing) {
            wxArrayString messages;
            messages.Add(wxT("Interrupted Long Exposure Cycle - please try to restart LE."));
            wxMessageArea::wxMessageSink(messages,wxT("Web Camera - CaptureHandling"));
         }
         else {
         }
      }
   }
   else { //<0 error
      if (m_isLExposing) {
         // error report, something failed
         wxArrayString messages;
         messages.Add(wxT("Ooops - Long Exposure Cycle Error - Restart Capture and complain if it persists!"));
         messages.Add(wxT("Please report it as 'bug' with the following information:"));
         messages.Add(wxString::Format(wxT("Long Exposure Cycle Error in: %s, Line >> %u"),__FILE__, __LINE__ ));
         wxMessageArea::wxMessageSink(messages,wxT("Web Camera - CaptureHandling"));
         SwitchLE(false);
      }
   }
}


/// //////////////////////////////////////////////////////////////////////////
/// CAM EXPOSURE SECTION
/// //////////////////////////////////////////////////////////////////////////

// starts one exposure cycle
// handling depends wether we are in LE or not
bool wxWebcam::StartExpoCycle()
{
   if (m_fmtChange.NeedsBinChange() || m_fmtChange.NeedsSubChange() ) {
      // this calls StartExpoCycle() again so we have to leave this with the returned value
      bool retVal = StartPreview();
      return retVal;
   }
   else {
      if (m_isLExposing) {
         // starts the new LE cycle
         if ( !m_pLongExpoPort->PreStartExposure() ) {
            ::fprintf(stderr, "wxWebcam::StartExpoCycle() - PreStartExposure failed \n");
            return false;  // ERROR EXIT
         }
         // now if there is a shutter pre cycle we have to setup another 300ms for that
         if ( m_pLongExpoPort->IsCycleShutter() ) {
            return m_timer.StartTimerX(300);  // the time is hardcoded -> must be at least one 5fps round
         }
         else {
            return m_timer.StartTimerX(1);
         }
      }
      else {
         // non LE - just trigger sample collection
         m_timer.InitRealExposureSeconds();   // tag the exposure to read the real time exposure
         m_pWebSample->MarkTimeStamp(); //BM:20090621 - load current time into the sample
         return m_ds->CaptureStream(true, 0); // start one capture cycle - no countdown
      }
   }
}

// abort the current take and start a new exposure (don't capture the aborted one)
void wxWebcam::RestartExposure()
{
   ; // not supported right now
}

bool wxWebcam::IsLExposing() const
{
   return m_isLExposing;
}

// en/disables LE capturing
bool wxWebcam::SwitchLE(bool enabled)
{
   if ( enabled == m_isLExposing ) return true; // nothing to do


   // now we have either to switch LE on or off
   if (enabled) { //was OFF target ON
      _V_ ::fprintf(stderr, "wxWebcam::SwitchLE() - enable LE \n");

      if ( !UseLEPort(wxF()->lePort()) ) {
         ::fprintf(stderr,"wxWebcam::SwitchLE() - using a long exposure port failed\n");
         return false; // cannot - invalid LE obje
      }
      // switch Realtime display off
      m_preview = false;
//      m_wximage= m_streamImage.Copy(); // avoid garbage image when switching to LE
      if(m_pLongExpoPort)m_pLongExpoPort->LXmodeOn(); // go to LE Mode
      // this has to be synced in CaptureHandling
      m_isLExposing = true;
   }
   else { //was ON target OFF
      _V_ ::fprintf(stderr, "wxWebcam::SwitchLE() - disable LE \n");

      m_timer.AbortExposure(); // calls into timer callback with abort flag
      if(m_pLongExpoPort)m_pLongExpoPort->LXmodeOff();
      m_isLExposing = false;
      // back to non LE streaming
      // switch Realtime display on
      SetFullFrame();   // reset frame to fullframe
//      StartExpoCycle(); // restart preview capture cycle - no countdown
      m_preview = true;
      UseLEPort(NULL);
   }

   return true;
}

/// //////////////////////////////////////////////////////////////////////////
/// WEBCAM READOUT SECTION
/// //////////////////////////////////////////////////////////////////////////

// the regular pacemaker
void wxWebcam::TimerEvent()
{
   ESYNMode syn = m_ds->OnCaptureRequired();
    // Data processing:
    // check if a sample has been deployed
   if (syn>=ESYN_Capture)
      CaptureHandling();

   // sync readout through the timer event to get a consistent timing
   if ( (m_doReadoutLE) && (syn>=ESYN_Tick) ) {
      m_doReadoutLE = false;  // one shot trigger
      // enables readout via port
      if(m_pLongExpoPort)m_pLongExpoPort->ReadExposure();
      m_realExpoTime = m_timer.GetRealExposureSeconds(); // read just after CCD readout starts
      // get the image (# is a setup item)
      m_ds->CaptureStream(true, m_capDelay);
   }
}

/// //////////////////////////////////////////////////////////////////////////
/// WEBCAM LONG EXPOSURE SECTION
/// //////////////////////////////////////////////////////////////////////////

// calls back if the shutter cycle has ended
// used for LE only
void wxWebcam::TimerXEvent()
{
   // here we start the real timed exposure
   if ( !m_timer.StartExposure(m_leMilliseconds) ) {
      if(m_pLongExpoPort)m_pLongExpoPort->EndExposure();
      StartExpoCycle(); // restart preview capture cycle - no countdown
      return;  // ERROR EXIT
   }
   if(m_pLongExpoPort)m_pLongExpoPort->StartExposure();
   m_pWebSample->MarkTimeStamp(); //BM:20090621 - load current time into the sample
   m_timer.InitRealExposureSeconds();   // tag the exposure to read the real time exposure
}

// calls back the AmpLeadTime before the exposure ends
void wxWebcam::TriggerExposureEvent(bool aborted)
{
   if (aborted) {
      if(m_pLongExpoPort)m_pLongExpoPort->EndExposure();
      StartExpoCycle(); // restart preview capture cycle - no countdown
   }
   else {
      if(m_pLongExpoPort)m_pLongExpoPort->TriggerExposure();  // would call amp on of set so
   }
}

// calls back if the exposure ends
void wxWebcam::ReadExposureEvent(bool aborted)
{
   if (aborted) {
      if(m_pLongExpoPort)m_pLongExpoPort->EndExposure();
      StartExpoCycle(); // restart preview capture cycle - no countdown
   }
   else {
      m_doReadoutLE = true;
      // now this will trigger a readout later
   }
}

// returns seconds left until exposure ends
void wxWebcam::ProgressEvent(unsigned long secondsLeft)
{
   //BM:20090104 - add event each time the second changes
   if (secondsLeft!=m_lxSecLeft) {
      wxCommandEvent    cmdEvent(wxEVT_WXAC_TIMER, GetId());
      cmdEvent.SetEventObject(this);

      ::wxPostEvent(m_parent->GetEventHandler(), cmdEvent);
   }
   m_lxSecLeft = secondsLeft;
}

/// //////////////////////////////////////////////////////////////////////////
/// IMAGE DRAWING SECTION
/// //////////////////////////////////////////////////////////////////////////

wxImage* wxWebcam::liveImage() const
{
   // return a pointer to the relevant image buffer
   // i.e. a buffer guaranteed to contain the most recent live camera image
   wxImage* image = m_pLiveImages->ImagePtr();
   return image;
}

wxImage* wxWebcam::frameImage() const
{
   return m_pLiveImages->ImagePtr();
}

// default response
// can be overwritten by platform specific implementation
bool wxWebcam::liveImageInverted() const
{
   return false;
}

wxSize wxWebcam::liveSize() const
{
   if (liveImage())
      return wxSize(liveImage()->GetWidth(), liveImage()->GetHeight());
   else
     return wxSize(0,0);

}

wxImage* wxWebcam::canvasImage() const
{
   if ( m_pLiveImages->BackgPtr() ) return m_pLiveImages->BackgPtr();
   else                            return m_pLiveImages->ImagePtr(); //20080819:BM fix return image if no back is there
}

wxSize wxWebcam::canvasSize() const
{
   if (canvasImage())
      return wxSize(canvasImage()->GetWidth(), canvasImage()->GetHeight());
   else
     return wxSize(0,0);

}

void wxWebcam::SetImageProperties(double gamma, bool autoPoint, double blackPoint, double whitePoint)
{
   wxCamera::SetImageProperties(gamma, autoPoint, blackPoint, whitePoint);

   // TODO implement for Webcam
}

// checks if a change is due - it destroys the old and creates a new one
void wxWebcam::UpdateImage(const wxSize& size, bool fullFrame)
{
   bool changed = m_pLiveImages->UpdateImage(size, fullFrame);
   if ( changed ) {
      // enable support for images larger than the client area
      // by providing scrollbars. This is managed by ZoomRedraw.
      ZoomRedraw(m_zoomfact,true);
   }
}

// prepares the scrolling after zoom change and redraws via wxWidgets
void wxWebcam::ZoomRedraw(double zoomFact, bool redraw)
{
   // actual frame dimensions
   wxUint16 w,h;
   // returns always the fullframe size
   w = m_pLiveImages->Width();
   h = m_pLiveImages->Height();
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

   // redraw through wxWidgets event handler
   if(redraw)wxWindow::Refresh(false);
}


// Main Draw Routine
// called via paintEvent also from Linux sample grabber when a new sample
// was written into the wxImage used as framebuffer
// or from CaptureHandling to show a new LE capture
// or jsut from the wx infrastructure
void wxWebcam::OnDraw(wxPaintEvent& event)
{
   if ( m_isLExposing ) {
      // here we land when LExposing - so we have a wxImage to draw on all platforms

      wxASSERT(!m_preview);  // shall not be in Preview here

      // we _must_ show the m_wximage when in LE mode
      wxImage* image = liveImage();

      // we use buffered DCs to avoid flicker if possible

      // called directly from capture handling
      if (event.GetId()==WXAC_PEVENT_ID_LE) {
         // this is outside a paint event so use wxClientDC !!!
         if(m_pLiveImages->ImageOK()) {
            wxClientDC ndc(this);
            PrepareDC(ndc);
            wxSize   iSize(image->GetWidth(), image->GetHeight());
            wxBufferedDC dc(&ndc, iSize);  // outside OnPaint events
            OnDrawOverlay(dc);
         }
         else {
            // we are called by capture handler but dont have a valid image
            // just do nothing here
         }
      }//if called from capture handling

      // called via regular PAINT event
      else {
         // this is inside a paint event so use paintDC
         if(image->Ok()) {
//            wxAutoBufferedPaintDC dc(this); //, wxBUFFER_VIRTUAL_AREA );
//            PrepareDC(dc); // even so the Buffered should not require Prepare this one does
                             //(it is due to implementation as typedef when it resolves a plain PaintDC)
            //! seems that Linux GTK2 does not well autoBuffer things so we use explicite BufferedDC
            wxBufferedPaintDC dc(this, wxBUFFER_VIRTUAL_AREA );
            OnDrawOverlay(dc);
         }
         else {
            // we don't have a valid image to draw right now
            event.Skip();  // allow others - GUI will not be responsive otherwise.....
         }
      }//else
   }
   else {
      // not in LE mode - must be in Paint Event then
      // let others do some painting
      event.Skip();

      // then draw decorations if needed
      OnDrawOverlay(true);
   }
}


// This is called when in NON LE MODE
// this routine assumes to write to a wxImage as liveimage
// Note that this function may be overidden on some platforms
void wxWebcam::OnDrawOverlay(bool in_paint_event)
{
   // This is the standard wxWidgets OnDrawOverlay version
   // Note that this function may be overidden on some platforms
   wxImage* image = liveImage();
   if(image && image->Ok()) {
      if(in_paint_event) {
         // this is inside a paint event so use paintDC
         // 99% of the time it goes here (when not in LE)

//         wxAutoBufferedPaintDC dc(this); //, wxBUFFER_VIRTUAL_AREA );
//         PrepareDC(dc); // even so the Buffered should not require Prepare this one does
                          //(it is due to implementation as typedef when it resolves a plain PaintDC)
         //! seems that Linux GTK2 does not well autoBuffer things so we use explicite BufferedDC
         wxBufferedPaintDC dc(this, wxBUFFER_VIRTUAL_AREA );
         OnDrawOverlay(dc);
      }
      else {
         // this is outside a paint event so use wxClientDC !!!
         wxClientDC ndc(this);
         PrepareDC(ndc);
         wxSize   iSize(m_pLiveImages->Width(), m_pLiveImages->Height());
         wxBufferedDC dc(&ndc, iSize);  // outside OnPaint events
         OnDrawOverlay(dc);
      }
   }
}

// This is called when in LE MODE
// the proper DC is already sumbitted
void wxWebcam::OnDrawOverlay(wxDC& dc)
{
   // draw image and overlay symbols using the already prepared DC
   wxCoord width,height;
   dc.GetSize(&width, &height);

   wxImage* image = liveImage();

   OnDrawImage(dc,image, true);
   OnDrawReticle(dc,true);
   OnDrawRubberband(dc);
   OnDrawGuidebox(dc);
   if (m_isCapturing) OnDrawCapturedFrames(dc);
   OnDrawFps(dc);
}


// Overwrites the wxCamera provided one
void wxWebcam::OnDrawImage(wxDC& dc, wxImage* image, bool zoomed_view)
{

   if( m_pLiveImages && m_pLiveImages->ImageOK() ) {

      // get the size of the client area
      wxCoord width,height;
      dc.GetSize(&width, &height);

      if(zoomed_view) {
         // scale the image based the zoom factor
         int vir_width, vir_height;

         if (m_pLiveImages->SubFrame()) {
            // we have to draw a background image
            // map from Image to DC coordinates
            wxCamPoint size(m_pLiveImages-> Width(), m_pLiveImages->Height());
            m_camCoord->SetPoint(size, wxCamCoords::EPO_Image);
            size = m_camCoord->GetPoint(wxCamCoords::EPO_DC);

            vir_width  = int(size.x);
            vir_height = int(size.y);
            if ( (vir_width==m_pLiveImages->Width()) && (vir_height==m_pLiveImages->Height()) ) {
               // does not need scaling
               dc.DrawBitmap(*m_pLiveImages->BackgPtr(),0,0);
            }
            else {
               // needs scaling
               dc.DrawBitmap(m_pLiveImages->BackgPtr()->Scale(vir_width,vir_height),0,0);
            }
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
         wxCamPoint size(m_pLiveImages->SubFrameWidth(), m_pLiveImages->SubFrameHeight());
         m_camCoord->SetPoint(size, wxCamCoords::EPO_Image);
         size = m_camCoord->GetPoint(wxCamCoords::EPO_DC);

         vir_width  = int(size.x);
         vir_height = int(size.y);

         wxCamPoint point(m_pLiveImages->SubFrameLeft(), m_pLiveImages->SubFrameTop());
         m_camCoord->SetPoint(point, wxCamCoords::EPO_Image);
         point = m_camCoord->GetPoint(wxCamCoords::EPO_DC);

         int vir_left = int(point.x);
         int vir_top  = int(point.y);
         if ( (vir_width==m_pLiveImages->SubFrameWidth()) && (vir_height==m_pLiveImages->SubFrameHeight()) ) {
            // does not need scaling
            dc.DrawBitmap(*m_pLiveImages->ImagePtr(), vir_left, vir_top);
         }
         else {
            // needs scaling
            dc.DrawBitmap(m_pLiveImages->ImagePtr()->Scale(vir_width,vir_height), vir_left, vir_top);
         }
         if (m_pLiveImages->SubFrame()) {
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
         wxCamera::OnDrawImage(dc, image, false);
      }
   }
}

void wxWebcam::OnDrawGuidebox(wxDC& dc)
{
   if(!IsConnected()) return;
   if(!m_overlayGuideBoxDraw) return;

   // the guidebox is specified in m_wximage coordinates and must be scaled relative to DC
   // draw image and overlay symbols using the already prepared DC
   wxCoord width,height;
   dc.GetSize(&width, &height);

   wxImage* image = liveImage();

   double scale_x = double(width)/image->GetWidth();
   double scale_y = double(height)/image->GetHeight();

   scale_x = 1.0;  //20080825:CA Zoomed guiding not supported
   scale_y = 1.0;  //20080825:CA Zoomed guiding not supported

   wxPoint pos;
   pos.x = (int)( (double)m_overlayGuideBox.GetPosition().x*scale_x );
   pos.y = (int)( (double)m_overlayGuideBox.GetPosition().y*scale_y );
   wxSize size;
   size.SetWidth((int)(m_overlayGuideBox.GetSize().GetWidth()*scale_x) );
   size.SetHeight((int)(m_overlayGuideBox.GetSize().GetHeight()*scale_y) );
   wxRect scaled_box(pos,size);

   dc.SetPen(*wxWHITE_PEN);
   dc.SetBrush(*wxTRANSPARENT_BRUSH);
   dc.DrawRectangle(scaled_box);
   dc.DrawLine((wxCoord)(m_overlayGuideLine[0].x*scale_x),(wxCoord)(m_overlayGuideLine[0].y*scale_y),
               (wxCoord)(m_overlayGuideLine[1].x*scale_x),(wxCoord)(m_overlayGuideLine[1].y*scale_y));
   // reset resources used
   dc.SetBrush(wxNullBrush);
   dc.SetPen(wxNullPen);
}


void wxWebcam::setGuideBox(const wxRealPoint& lockpos, // locked position in image coordinates
                           const wxRealPoint& curpos,  // current position in image coordinates
                           const wxSize& size)         // size in image poxels
{
   wxPoint curpos2;
   curpos2.x = (int)((double)curpos.x + 0.5 - (double)size.GetWidth()*0.5);
   curpos2.y = (int)((double)curpos.y + 0.5 - (double)size.GetHeight()*0.5);
   m_overlayGuideBox = wxRect(curpos2,size);

   m_overlayGuideLine[0] = lockpos;
   m_overlayGuideLine[1] = curpos;

   m_overlayGuideBoxDraw = true;
   OnDrawOverlay(false);
}

void wxWebcam::cancelGuideBox()
{
   m_overlayGuideBoxDraw = false;
   OnDrawOverlay(false);
}

bool wxWebcam::isGuiding() const
{
   return m_overlayGuideBoxDraw;
}


void wxWebcam::OnDrawFps(wxDC& dc)
{
   if(!IsConnected()) return;

   wxString label = wxT("");
   // get info about counter font size or whether it is turned off
   wxFactory::FrameCounterSize font_size = wxF()->FpsFontSize();
   if(font_size != wxFactory::CNT_OFF) {
      label = wxString::Format(wxT("FPS %3.0f - DPS %3.0f"), m_fps, m_dps);
   }
   int point_size = int(font_size);

   if(label.length() > 0) {
      wxFont fnt = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
      dc.SetTextForeground(wxColour(255, 80, 0));
      fnt.SetPointSize(point_size);
      dc.SetFont(fnt);

      //20080823:BM draw the label top left independent of scrolling
      int xx(0),yy(0);
      CalcUnscrolledPosition(5,20,&xx,&yy); // wxWidgets supplied helper
      dc.DrawText(label, xx, yy);
   }
}



/// //////////////////////////////////////////////////////////////////////////
/// SUBFRAME SECTION
/// //////////////////////////////////////////////////////////////////////////

void wxWebcam::SetFullFrame()
{
   m_fmtChange.SetSubframe(wxPoint(1,1), wxPoint(1,1));
   // this will only set a note to change but does not change immediately
}

// set the subframe format (binning factor 1 assumed)
//  needs two points of a rectangle that makes the subframe in pixel coords
//  the subframe might get adjusted if binning changes
//  switching back to the max frame is done by setting all points to 0 (zero)
bool wxWebcam::SetSubframe(const wxPoint& start, const wxPoint& end)
{
//   return false;  // NOT YET SUPPORTED
   // we have to prepare for that
   if (m_isCapturing) return false; // not allowed while capturing though...

   m_fmtChange.SetSubframe(start, end);
   return true;
}

void wxWebcam::FinalizeSubframe()
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
void wxWebcam::LoadKeywordList(wxFitsKeywordList* kwdList)
{
   if (!kwdList) return;


   wxFactory* factory = wxFactory::singleton();
   factory->getFitsKeywordList(*kwdList);

   wxFitsKeywordEx kx;
   kx.CameraSettingKwd(SettingString()); *kwdList += kx; // this will not track setting changes while capturing !!!!
}


// returns a string nn;nn;..; of the current webcam % settings (.. means not readable)
// the order is derived from the enum sequence in captypes.h
wxString wxWebcam::SettingString()
{
   if(!IsConnected())return wxT("not available");

   wxString setting;
   bool percent = true;
   DeviceProperties* prop = properties();

   if(prop->HasProperty_Brightness() == VCS_both) {
      setting << prop->Property_Brightness(percent) << wxT(";");
   } else setting << wxT("..;");

   if(prop->HasProperty_Contrast() == VCS_both) {
      setting << prop->Property_Contrast(percent) << wxT(";");
   } else setting << wxT("..;");

   if(prop->HasProperty_Hue() == VCS_both) {
      setting << prop->Property_Hue(percent) << wxT(";");
   } else setting << wxT("..;");

   if(prop->HasProperty_Saturation() == VCS_both) {
      setting << prop->Property_Saturation(percent) << wxT(";");
   } else setting << wxT("..;");

   if(prop->HasProperty_Sharpness() == VCS_both) {
      setting << prop->Property_Sharpness(percent) << wxT(";");
   } else setting << wxT("..;");

   if(prop->HasProperty_Gamma() == VCS_both) {
      setting << prop->Property_Gamma(percent) << wxT(";");
   } else setting << wxT("..;");

   if(prop->HasProperty_ColorEnable() == VCS_both) {
      setting << prop->Property_ColorEnable() << wxT(";");
   } else setting << wxT("..;");

   if(prop->HasProperty_BacklightCompensation() == VCS_both) {
      setting << prop->Property_BacklightCompensation(percent) << wxT(";");
   } else setting << wxT("..;");

   if(prop->HasProperty_WhiteBalanceMode() == VCS_both) {
      setting << prop->Property_WhiteBalanceMode() << wxT(";");
   } else setting << wxT("..;");

   if(prop->HasProperty_WhiteBalRed() == VCS_both) {
      setting << prop->Property_WhiteBalRed(percent) << wxT(";");
   } else setting << wxT("..;");

   if(prop->HasProperty_WhiteBalBlue() == VCS_both) {
      setting << prop->Property_WhiteBalBlue(percent) << wxT(";");
   } else setting << wxT("..;");

   if(prop->HasProperty_FlickerFree() == VCS_both) {
      setting << prop->Property_FlickerFree() << wxT(";");
   } else setting << wxT("..;");

   if(prop->HasProperty_Flicker50_60() == VCS_both) {
      setting << prop->Property_Flicker50_60(percent) << wxT(";");
   } else setting << wxT("..;");

   if(prop->HasProperty_FPS() == VCS_both) {
      setting << prop->Property_FPS() << wxT(";");
   } else setting << wxT("..;");

   if(prop->HasProperty_Exposure() == VCS_both) {
      setting << prop->Property_Exposure() << wxT(";");
   } else setting << wxT("..;");

   if(prop->HasProperty_Gain() == VCS_both) {
      setting << prop->Property_Gain(percent) << wxT(";");
   } else setting << wxT("..;");

   if(prop->HasProperty_FlipHorizontal() == VCS_both) {
      setting << prop->Property_FlipHorizontal() << wxT(";");
   } else setting << wxT("..;");

   if(prop->HasProperty_FlipVertical() == VCS_both) {
      setting << prop->Property_FlipVertical() << wxT(";");
   } else setting << wxT("..;");

   if(prop->HasProperty_AutoExposure() == VCS_both) {
      setting << prop->Property_AutoExposure() << wxT(";");
   } else setting << wxT("..;");

   if(prop->HasProperty_AutoAll() == VCS_both) {
      setting << prop->Property_AutoAll() << wxT(";");
   } else setting << wxT("..;");

   return setting;
}


/// //////////////////////////////////////////////////////////////////////////
/// WEBCAM CONFIG SECTION
/// //////////////////////////////////////////////////////////////////////////

wxString wxWebcam::ConfigPath(const wxString& slotname)
{
   wxString path = _T("/cameras/") + CameraName() +_T("(")+  CameraSerial() + _T(")");
   if(slotname.Length() > 0) {
      path += _T("/") + slotname;
   }
   return path;
}

bool wxWebcam::SaveToConfig(wxConfig* config)
{
   // Here, we can store property values directly into the config object
   // Only properties with both read and write support are considered

   if(!IsConnected())return false;

   bool percent = true;
   DeviceProperties* prop = properties();

   if(prop->HasProperty_Brightness() == VCS_both) {
      config->Write(_T("Brightness"),prop->Property_Brightness(percent));
   }

   if(prop->HasProperty_Contrast() == VCS_both) {
      config->Write(_T("Contrast"),prop->Property_Contrast(percent));
   }

   if(prop->HasProperty_Hue() == VCS_both) {
      config->Write(_T("Hue"),prop->Property_Hue(percent));
   }

   if(prop->HasProperty_Saturation() == VCS_both) {
      config->Write(_T("Saturation"),prop->Property_Saturation(percent));
   }

   if(prop->HasProperty_Sharpness() == VCS_both) {
      config->Write(_T("Sharpness"),prop->Property_Sharpness(percent));
   }

   if(prop->HasProperty_Gamma() == VCS_both) {
      config->Write(_T("Gamma"),prop->Property_Gamma(percent));
   }

   if(prop->HasProperty_ColorEnable() == VCS_both) {
      config->Write(_T("ColorEnable"),prop->Property_ColorEnable());
   }

   if(prop->HasProperty_BacklightCompensation() == VCS_both) {
      config->Write(_T("BacklightCompensation"),prop->Property_BacklightCompensation(percent));
   }

   if(prop->HasProperty_WhiteBalanceMode() == VCS_both) {
      config->Write(_T("WhiteBalanceMode"),prop->Property_WhiteBalanceMode());
   }

   if(prop->HasProperty_WhiteBalRed() == VCS_both) {
      config->Write(_T("WhiteBalRed"),prop->Property_WhiteBalRed(percent));
   }

   if(prop->HasProperty_WhiteBalBlue() == VCS_both) {
      config->Write(_T("WhiteBalBlue"),prop->Property_WhiteBalBlue(percent));
   }

   if(prop->HasProperty_FlickerFree() == VCS_both) {
      config->Write(_T("FlickerFree"),prop->Property_FlickerFree());
   }

   if(prop->HasProperty_Flicker50_60() == VCS_both) {
      config->Write(_T("Flicker50_60"),prop->Property_Flicker50_60(percent));
   }

   if(prop->HasProperty_FPS() == VCS_both) {
      config->Write(_T("FPS"),prop->Property_FPS());
   }

   if(prop->HasProperty_Exposure() == VCS_both) {
      config->Write(_T("Exposure"),prop->Property_Exposure());
   }

   if(prop->HasProperty_Gain() == VCS_both) {
      config->Write(_T("Gain"),prop->Property_Gain(percent));
   }

   if(prop->HasProperty_FlipHorizontal() == VCS_both) {
      config->Write(_T("FlipHorizontal"),prop->Property_FlipHorizontal());
   }

   if(prop->HasProperty_FlipVertical() == VCS_both) {
      config->Write(_T("FlipVertical"),prop->Property_FlipVertical());
   }

   if(prop->HasProperty_AutoExposure() == VCS_both) {
      config->Write(_T("AutoExposure"),prop->Property_AutoExposure());
   }

   if(prop->HasProperty_AutoAll() == VCS_both) {
      config->Write(_T("AutoAll"),prop->Property_AutoAll());
   }

   return true;
}


bool wxWebcam::RestoreFromConfig(wxConfig* config)
{
   if(!IsConnected())return false;

   bool percent = true;
   DeviceProperties* prop = properties();

   // === PRIORITISED SEQUENCE FIRST

   if(prop->HasProperty_FPS() == VCS_both) {
      long value=0;
      if(config->Read(_T("FPS"),&value))prop->SetProperty_FPS(value);
   }

   if(prop->HasProperty_Exposure() == VCS_both) {
      long value=0;
      if(config->Read(_T("Exposure"),&value))prop->SetProperty_Exposure(value);
   }

   if(prop->HasProperty_Gain() == VCS_both) {
      long value=0;
      if(config->Read(_T("Gain"),&value))prop->SetProperty_Gain(value,percent);
   }

   // === END PRIORITSED SEQUENCE

   if(prop->HasProperty_Brightness() == VCS_both) {
      long value=0;
      if(config->Read(_T("Brightness"),&value))prop->SetProperty_Brightness(value,percent);
   }

   if(prop->HasProperty_Contrast() == VCS_both) {
      long value=0;
      if(config->Read(_T("Contrast"),&value))prop->SetProperty_Contrast(value,percent);
   }

   if(prop->HasProperty_Hue() == VCS_both) {
      long value=0;
      if(config->Read(_T("Hue"),&value))prop->SetProperty_Hue(value,percent);
   }

   if(prop->HasProperty_Saturation() == VCS_both) {
      long value=0;
      if(config->Read(_T("Saturation"),&value))prop->SetProperty_Saturation(value,percent);
   }

   if(prop->HasProperty_Sharpness() == VCS_both) {
      long value=0;
      if(config->Read(_T("Sharpness"),&value))prop->SetProperty_Sharpness(value,percent);
   }

   if(prop->HasProperty_Gamma() == VCS_both) {
      long value=0;
      if(config->Read(_T("Gamma"),&value))prop->SetProperty_Gamma(value,percent);
   }

   if(prop->HasProperty_ColorEnable() == VCS_both) {
      long value=0;
      if(config->Read(_T("ColorEnable"),&value))prop->SetProperty_ColorEnable((value)? true : false);
   }

   if(prop->HasProperty_BacklightCompensation() == VCS_both) {
      long value=0;
      if(config->Read(_T("BacklightCompensation"),&value))prop->SetProperty_BacklightCompensation(value,percent);
   }

   if(prop->HasProperty_WhiteBalanceMode() == VCS_both) {
      long value=0;
      if(config->Read(_T("WhiteBalanceMode"),&value))prop->SetProperty_WhiteBalanceMode(value);
   }

   if(prop->HasProperty_WhiteBalRed() == VCS_both) {
      long value=0;
      if(config->Read(_T("WhiteBalRed"),&value))prop->SetProperty_WhiteBalRed(value,percent);
   }

   if(prop->HasProperty_WhiteBalBlue() == VCS_both) {
      long value=0;
      if(config->Read(_T("WhiteBalBlue"),&value))prop->SetProperty_WhiteBalBlue(value,percent);
   }

   if(prop->HasProperty_FlickerFree() == VCS_both) {
      long value=0;
      if(config->Read(_T("FlickerFree"),&value))prop->SetProperty_FlickerFree((value)? true : false);
   }

   if(prop->HasProperty_Flicker50_60() == VCS_both) {
      long value=0;
      if(config->Read(_T("Flicker50_60"),&value))prop->SetProperty_Flicker50_60((value)? true : false);
   }

   if(prop->HasProperty_FlipHorizontal() == VCS_both) {
      long value=0;
      if(config->Read(_T("FlipHorizontal"),&value))prop->SetProperty_FlipHorizontal((value)? true : false);
   }

   if(prop->HasProperty_FlipVertical() == VCS_both) {
      long value=0;
      if(config->Read(_T("FlipVertical"),&value))prop->SetProperty_FlipVertical((value)? true : false);
   }

   // === AUTO items shall be set as the final properties

   if(prop->HasProperty_AutoExposure() == VCS_both) {
      long value=0;
      if(config->Read(_T("AutoExposure"),&value))prop->SetProperty_AutoExposure((value)? true : false);
   }

   if(prop->HasProperty_AutoAll() == VCS_both) {
      long value=0;
      if(config->Read(_T("AutoAll"),&value))prop->SetProperty_AutoAll((value)? true : false);
   }

   return true;
}


// submit the IO port handle to be used with LE
// TODO may be the assignement must be handled restrictiv i.e. not while doing LE or so
bool wxWebcam::UseLEPort(wxLongExpoPort* lePort)
{
   m_pLongExpoPort = lePort;
   return (m_pLongExpoPort!=NULL);
}


wxLongExpoPort* wxWebcam::LongExpoPort()
{
   return m_pLongExpoPort;
}

CaptureBase* wxWebcam::ds()
{
   return m_ds;
}


/// //////////////////////////////////////////////////////////////////////////
/// WEBCAM PROPOERTY SECTION
/// //////////////////////////////////////////////////////////////////////////

wxWebcam::DeviceProperties* wxWebcam::properties()
{
   return m_pProperties;
}

wxWebcam::DeviceProperties::DeviceProperties(wxWebcam* camera)
: m_camera(camera)
{}

// Query for availablity and type if support for each property
EVidCapPropertySupport wxWebcam::DeviceProperties::HasProperty_Brightness()
{
   EVidCapPropertySupport ds_support = m_camera->ds()->PropertyHas(VCP_Brightness);
   return ((EVidCapPropertySupport)ds_support);}

EVidCapPropertySupport wxWebcam::DeviceProperties::HasProperty_Contrast()
{
   EVidCapPropertySupport ds_support = m_camera->ds()->PropertyHas(VCP_Contrast);
   return ((EVidCapPropertySupport)ds_support);
}

EVidCapPropertySupport wxWebcam::DeviceProperties::HasProperty_Hue()
{
   EVidCapPropertySupport ds_support = m_camera->ds()->PropertyHas(VCP_Hue);
   return ((EVidCapPropertySupport)ds_support);
}

EVidCapPropertySupport wxWebcam::DeviceProperties::HasProperty_Saturation()
{
   EVidCapPropertySupport ds_support = m_camera->ds()->PropertyHas(VCP_Saturation);
   return ((EVidCapPropertySupport)ds_support);
}

EVidCapPropertySupport wxWebcam::DeviceProperties::HasProperty_Sharpness()
{
   EVidCapPropertySupport ds_support = m_camera->ds()->PropertyHas(VCP_Sharpness);
   return ((EVidCapPropertySupport)ds_support);
}

EVidCapPropertySupport wxWebcam::DeviceProperties::HasProperty_Gamma()
{
   EVidCapPropertySupport ds_support = m_camera->ds()->PropertyHas(VCP_Gamma);
   return ((EVidCapPropertySupport)ds_support);
}

EVidCapPropertySupport wxWebcam::DeviceProperties::HasProperty_ColorEnable()
{
   EVidCapPropertySupport ds_support = m_camera->ds()->PropertyHas(VCP_ColorEnable);
   return ((EVidCapPropertySupport)ds_support);
}

EVidCapPropertySupport wxWebcam::DeviceProperties::HasProperty_BacklightCompensation()
{
   EVidCapPropertySupport ds_support = m_camera->ds()->PropertyHas(VCP_BacklightCompensation);
   return ((EVidCapPropertySupport)ds_support);
}

EVidCapPropertySupport wxWebcam::DeviceProperties::HasProperty_WhiteBalanceMode()
{
   EVidCapPropertySupport ds_support = m_camera->ds()->PropertyHas(VCP_WhiteBalanceMode);
   return ((EVidCapPropertySupport)ds_support);
}

EVidCapPropertySupport wxWebcam::DeviceProperties::HasProperty_WhiteBalRed()
{
   EVidCapPropertySupport ds_support = m_camera->ds()->PropertyHas(VCP_WhiteBalRed);
   return ((EVidCapPropertySupport)ds_support);
}

EVidCapPropertySupport wxWebcam::DeviceProperties::HasProperty_WhiteBalBlue()
{
   EVidCapPropertySupport ds_support = m_camera->ds()->PropertyHas(VCP_WhiteBalBlue);
   return ((EVidCapPropertySupport)ds_support);
}

EVidCapPropertySupport wxWebcam::DeviceProperties::HasProperty_FlickerFree()
{
   EVidCapPropertySupport ds_support = m_camera->ds()->PropertyHas(VCP_FlickerFree);
   return ((EVidCapPropertySupport)ds_support);
}

EVidCapPropertySupport wxWebcam::DeviceProperties::HasProperty_Flicker50_60()
{
   EVidCapPropertySupport ds_support = m_camera->ds()->PropertyHas(VCP_Flicker50_60);
   return ((EVidCapPropertySupport)ds_support);
}

EVidCapPropertySupport wxWebcam::DeviceProperties::HasProperty_FPS()
{
   EVidCapPropertySupport ds_support = m_camera->ds()->PropertyHas(VCP_FPS);
   return ((EVidCapPropertySupport)ds_support);
}

EVidCapPropertySupport wxWebcam::DeviceProperties::HasProperty_Exposure()
{
   EVidCapPropertySupport ds_support = m_camera->ds()->PropertyHas(VCP_Exposure);
   return ((EVidCapPropertySupport)ds_support);
}

EVidCapPropertySupport wxWebcam::DeviceProperties::HasProperty_Gain()
{
   EVidCapPropertySupport ds_support = m_camera->ds()->PropertyHas(VCP_Gain);
   return ((EVidCapPropertySupport)ds_support);
}

EVidCapPropertySupport wxWebcam::DeviceProperties::HasProperty_FlipHorizontal()
{
   EVidCapPropertySupport ds_support = m_camera->ds()->PropertyHas(VCP_FlipHorizontal);
   return ((EVidCapPropertySupport)ds_support);
}

EVidCapPropertySupport wxWebcam::DeviceProperties::HasProperty_FlipVertical()
{
   EVidCapPropertySupport ds_support = m_camera->ds()->PropertyHas(VCP_FlipVertical);
   return ((EVidCapPropertySupport)ds_support);
}

EVidCapPropertySupport wxWebcam::DeviceProperties::HasProperty_AutoExposure()
{
   EVidCapPropertySupport ds_support = m_camera->ds()->PropertyHas(VCP_AutoExposure);
   return ((EVidCapPropertySupport)ds_support);
}

EVidCapPropertySupport wxWebcam::DeviceProperties::HasProperty_AutoAll()
{
   EVidCapPropertySupport ds_support = m_camera->ds()->PropertyHas(VCP_AutoAll);
   return ((EVidCapPropertySupport)ds_support);
}


// retrieve raw raw/percent values, or bool
long wxWebcam::DeviceProperties::Property_Brightness(bool percent)
{
   return m_camera->ds()->Property(VCP_Brightness,percent);
}

long wxWebcam::DeviceProperties::Property_Contrast(bool percent)
{
   return m_camera->ds()->Property(VCP_Contrast,percent);
}

long wxWebcam::DeviceProperties::Property_Hue(bool percent)
{
   return m_camera->ds()->Property(VCP_Hue,percent);
}

long wxWebcam::DeviceProperties::Property_Saturation(bool percent)
{
   return m_camera->ds()->Property(VCP_Saturation,percent);
}

long wxWebcam::DeviceProperties::Property_Sharpness(bool percent)
{
   return m_camera->ds()->Property(VCP_Sharpness,percent);
}

long wxWebcam::DeviceProperties::Property_Gamma(bool percent)
{
   return m_camera->ds()->Property(VCP_Gamma,percent);
}

bool wxWebcam::DeviceProperties::Property_ColorEnable()
{
   long raw = m_camera->ds()->Property(VCP_ColorEnable,false);
   return (raw!=0);
}

long wxWebcam::DeviceProperties::Property_BacklightCompensation(bool percent)
{
   return m_camera->ds()->Property(VCP_BacklightCompensation,percent);
}

long wxWebcam::DeviceProperties::Property_WhiteBalanceMode()
{
   return m_camera->ds()->Property(VCP_WhiteBalanceMode,false);
}

long wxWebcam::DeviceProperties::Property_WhiteBalRed(bool percent)
{
   return m_camera->ds()->Property(VCP_WhiteBalRed,percent);
}

long wxWebcam::DeviceProperties::Property_WhiteBalBlue(bool percent)
{
   return m_camera->ds()->Property(VCP_WhiteBalBlue,percent);
}

bool wxWebcam::DeviceProperties::Property_FlickerFree()
{
   long raw = m_camera->ds()->Property(VCP_FlickerFree,false);
   return (raw!=0);
}

long wxWebcam::DeviceProperties::Property_Flicker50_60(bool WXUNUSED(percent))
{
   // (0=50Hz, 255=60Hz)
   long enable50 = m_camera->ds()->Property(VCP_Flicker50_60,false);
   return (enable50==0)? 0 : 1;
}

long wxWebcam::DeviceProperties::Property_FPS()
{
   return m_camera->ds()->Property(VCP_FPS,false);
}

long wxWebcam::DeviceProperties::Property_Exposure()
{
   return m_camera->ds()->Property(VCP_Exposure,false);
}

long wxWebcam::DeviceProperties::Property_Gain(bool percent)
{
   return m_camera->ds()->Property(VCP_Gain,percent);
}

bool wxWebcam::DeviceProperties::Property_FlipHorizontal()
{
   long flip = m_camera->ds()->Property(VCP_FlipHorizontal,false);
   return (flip!=0);
}

bool wxWebcam::DeviceProperties::Property_FlipVertical()
{
   long flip = m_camera->ds()->Property(VCP_FlipVertical,false);
   return (flip!=0);
}

bool wxWebcam::DeviceProperties::Property_AutoExposure()
{
   long autoexp = m_camera->ds()->Property(VCP_AutoExposure,false);
   return (autoexp!=0);
}

bool wxWebcam::DeviceProperties::Property_AutoAll()
{
   long autoall = m_camera->ds()->Property(VCP_AutoAll,false);
   return (autoall!=0);
}


// set property values, as raw/percent value, or bool
void wxWebcam::DeviceProperties::SetProperty_Brightness(long propValue, bool percent)
{
   m_camera->ds()->SetProperty(VCP_Brightness,propValue,percent);
}

void wxWebcam::DeviceProperties::SetProperty_Contrast(long propValue, bool percent )
{
   m_camera->ds()->SetProperty(VCP_Contrast,propValue,percent);
}

void wxWebcam::DeviceProperties::SetProperty_Hue(long propValue, bool percent )
{
   m_camera->ds()->SetProperty(VCP_Hue,propValue,percent);
}

void wxWebcam::DeviceProperties::SetProperty_Saturation(long propValue, bool percent )
{
   m_camera->ds()->SetProperty(VCP_Saturation,propValue,percent);
}

void wxWebcam::DeviceProperties::SetProperty_Sharpness(long propValue, bool percent )
{
   m_camera->ds()->SetProperty(VCP_Sharpness,propValue,percent);
}

void wxWebcam::DeviceProperties::SetProperty_Gamma(long propValue, bool percent )
{
   m_camera->ds()->SetProperty(VCP_Gamma,propValue,percent);
}

void wxWebcam::DeviceProperties::SetProperty_ColorEnable(bool enable )
{
   m_camera->ds()->SetProperty(VCP_ColorEnable,(enable)? 1 : 0,false);
}

void wxWebcam::DeviceProperties::SetProperty_BacklightCompensation(long propValue, bool percent )
{
   m_camera->ds()->SetProperty(VCP_BacklightCompensation,propValue,percent);
}

void wxWebcam::DeviceProperties::SetProperty_WhiteBalanceMode(long propValue)
{
   m_camera->ds()->SetProperty(VCP_WhiteBalanceMode,propValue,false);
}

void wxWebcam::DeviceProperties::SetProperty_WhiteBalRed(long propValue, bool percent )
{
   m_camera->ds()->SetProperty(VCP_WhiteBalRed,propValue,percent);
}

void wxWebcam::DeviceProperties::SetProperty_WhiteBalBlue(long propValue, bool percent )
{
   m_camera->ds()->SetProperty(VCP_WhiteBalBlue,propValue,percent);
}

void wxWebcam::DeviceProperties::SetProperty_FlickerFree(bool enable)
{
   m_camera->ds()->SetProperty(VCP_FlickerFree,(enable)? 1 : 0,false);
}

void wxWebcam::DeviceProperties::SetProperty_Flicker50_60(bool enable50)
{
   // (0=50Hz, 255=60Hz)
   m_camera->ds()->SetProperty(VCP_Flicker50_60,(enable50)? 0 : 255,false);
}

void wxWebcam::DeviceProperties::SetProperty_FPS(long propValue)
{
   m_camera->ds()->SetProperty(VCP_FPS,propValue,false);
}

void wxWebcam::DeviceProperties::SetProperty_Exposure(long propValue)
{
   m_camera->ds()->SetProperty(VCP_Exposure,propValue,false);
}

void wxWebcam::DeviceProperties::SetProperty_Gain(long propValue, bool percent )
{
   m_camera->ds()->SetProperty(VCP_Gain,propValue,percent);
}

void wxWebcam::DeviceProperties::SetProperty_FlipHorizontal(bool enable )
{
   m_camera->ds()->SetProperty(VCP_FlipHorizontal,(enable)? 1 : 0,false);
}

void wxWebcam::DeviceProperties::SetProperty_FlipVertical(bool enable )
{
   m_camera->ds()->SetProperty(VCP_FlipVertical,(enable)? 1 : 0,false);
}

void wxWebcam::DeviceProperties::SetProperty_AutoExposure( bool enable )
{
   m_camera->ds()->SetProperty(VCP_AutoExposure,(enable)? 1 : 0,false);
}

void wxWebcam::DeviceProperties::SetProperty_AutoAll( bool enable )
{
   m_camera->ds()->SetProperty(VCP_AutoAll,(enable)? 1 : 0,false);
}


void wxWebcam::DeviceProperties::PropertyMinMax_Brightness(long& min, long& max, long& step)
{
   min   = m_camera->ds()->PropertyMin(VCP_Brightness);
   max   = m_camera->ds()->PropertyMax(VCP_Brightness);
   step = m_camera->ds()->PropertyStep(VCP_Brightness);
}

void wxWebcam::DeviceProperties::PropertyMinMax_Contrast(long& min, long& max, long& step)
{
   min   = m_camera->ds()->PropertyMin(VCP_Contrast);
   max   = m_camera->ds()->PropertyMax(VCP_Contrast);
   step = m_camera->ds()->PropertyStep(VCP_Contrast);
}

void wxWebcam::DeviceProperties::PropertyMinMax_Hue(long& min, long& max, long& step)
{
   min   = m_camera->ds()->PropertyMin(VCP_Hue);
   max   = m_camera->ds()->PropertyMax(VCP_Hue);
   step = m_camera->ds()->PropertyStep(VCP_Hue);
}

void wxWebcam::DeviceProperties::PropertyMinMax_Saturation(long& min, long& max, long& step)
{
   min   = m_camera->ds()->PropertyMin(VCP_Saturation);
   max   = m_camera->ds()->PropertyMax(VCP_Saturation);
   step = m_camera->ds()->PropertyStep(VCP_Saturation);
}

void wxWebcam::DeviceProperties::PropertyMinMax_Sharpness(long& min, long& max, long& step)
{
   min   = m_camera->ds()->PropertyMin(VCP_Sharpness);
   max   = m_camera->ds()->PropertyMax(VCP_Sharpness);
   step = m_camera->ds()->PropertyStep(VCP_Sharpness);
}

void wxWebcam::DeviceProperties::PropertyMinMax_Gamma(long& min, long& max, long& step)
{
   min   = m_camera->ds()->PropertyMin(VCP_Gamma);
   max   = m_camera->ds()->PropertyMax(VCP_Gamma);
   step = m_camera->ds()->PropertyStep(VCP_Gamma);
}

void wxWebcam::DeviceProperties::PropertyMinMax_BacklightCompensation(long& min, long& max, long& step)
{
   min   = m_camera->ds()->PropertyMin(VCP_BacklightCompensation);
   max   = m_camera->ds()->PropertyMax(VCP_BacklightCompensation);
   step = m_camera->ds()->PropertyStep(VCP_BacklightCompensation);
}

void wxWebcam::DeviceProperties::PropertyMinMax_WhiteBalanceMode(long& min, long& max, long& step)
{
   min   = m_camera->ds()->PropertyMin(VCP_WhiteBalanceMode);
   max   = m_camera->ds()->PropertyMax(VCP_WhiteBalanceMode);
   step = m_camera->ds()->PropertyStep(VCP_WhiteBalanceMode);
}

void wxWebcam::DeviceProperties::PropertyMinMax_WhiteBalRed(long& min, long& max, long& step)
{
   min   = m_camera->ds()->PropertyMin(VCP_WhiteBalRed);
   max   = m_camera->ds()->PropertyMax(VCP_WhiteBalRed);
   step = m_camera->ds()->PropertyStep(VCP_WhiteBalRed);
}

void wxWebcam::DeviceProperties::PropertyMinMax_WhiteBalBlue(long& min, long& max, long& step)
{
   min   = m_camera->ds()->PropertyMin(VCP_WhiteBalBlue);
   max   = m_camera->ds()->PropertyMax(VCP_WhiteBalBlue);
   step = m_camera->ds()->PropertyStep(VCP_WhiteBalBlue);
}

void wxWebcam::DeviceProperties::PropertyMinMax_FPS(long& min, long& max, long& step)
{
   min   = m_camera->ds()->PropertyMin(VCP_FPS);
   max   = m_camera->ds()->PropertyMax(VCP_FPS);
   step = m_camera->ds()->PropertyStep(VCP_FPS);
}

void wxWebcam::DeviceProperties::PropertyMinMax_Exposure(long& min, long& max, long& step)
{
   min   = m_camera->ds()->PropertyMin(VCP_Exposure);
   max   = m_camera->ds()->PropertyMax(VCP_Exposure);
   step = m_camera->ds()->PropertyStep(VCP_Exposure);
}

void wxWebcam::DeviceProperties::PropertyMinMax_Gain(long& min, long& max, long& step)
{
   min   = m_camera->ds()->PropertyMin(VCP_Gain);
   max   = m_camera->ds()->PropertyMax(VCP_Gain);
   step = m_camera->ds()->PropertyStep(VCP_Gain);
}

void wxWebcam::DeviceProperties::Settings_LoadUser()
{
   // 		VCP_Setting					= 23, // =0 Load User, =1 Save User, =2 Load Factory
   long load_user=0;
   m_camera->ds()->SetProperty(VCP_Setting,load_user,false);
}

void wxWebcam::DeviceProperties::Settings_SaveUser()
{
   long save_user=1;
   m_camera->ds()->SetProperty(VCP_Setting,save_user,false);
}

void wxWebcam::DeviceProperties::Settings_LoadFactory()
{
   long load_factory=2;
   m_camera->ds()->SetProperty(VCP_Setting,load_factory,false);
}

EVideoSampleFormat wxWebcam::VideoSampleFormat()
{
   return m_sampleFormat;
}
