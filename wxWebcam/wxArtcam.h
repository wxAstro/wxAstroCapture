
#ifndef __WXARTCAM_H__
#define __WXARTCAM_H__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/config.h>

// The base class to implement a wxWidgets aware Artemis  CCD and HSC type cameras
//  CCD = Art285/429/255 and Atik branded ones
// HSC = HighRes Artemis cameras (not supported yet)

// derives from a wxScrolledWindow where the image is shown and a timer class
// this class implements a much as possible but is not intended nor able to be instantiated on it's own
// the proper camera class derives from this class

#include "ArtTypes.h"
#include "ArtBase.h"

#include "wxCamera.h"

#include "wxArtcamTimer.h"
#include "wxArtcamTimerCB.h"

#include <vector>

class wxConfig;
class ArtBase;
class wxArtSample;
class LiveImageArt;
class wxFitsKeywordList;

class wxArtcam  : public wxCamera, public wxArtcamTimerCB {
public:
   wxArtcam();
   wxArtcam(wxWindow* parent, wxWindowID id= -1);
   virtual ~wxArtcam();

   // we are cheating here
   static wxArtcam* Construct(wxWindow* parent, wxWindowID id= -1);

   // wxArtcam has no default implementation for those !!
   wxString VideoTechnology() const; // a short string "WDM", "V4L2" etc.

   bool IsWebcam() const {return false;};
   bool IsArtcam() const {return true;};

   // report the abilities of a certain camera (default artcam answers)
   bool HasDriverDialogs() const {return false;}
   bool HasZooming() const {return true;};
   bool HasBinning() const {return true;};
   bool HasSubframe() const {return true;};
   bool HasCooling() const;
   bool HasWarmup() const;
   bool SupportsGuiding() const {return false;}

   // call intrinsic dialogs
   void ShowFormatDialog() {return;}
   void ShowSourceDialog() {return;}

   // wxArtcam has a default implementation for those !!
   bool ConnectCamera(const wxString& camera);

   void DisconnectCamera();

   // set the camera format (binning factor) 1... max supported
   static bool SetDefaultBinning(unsigned short binFactor);
   static unsigned short DefaultBinning();
   bool SetBinning(unsigned short binFactor);
   // get the cameras max binning factor
   unsigned short MaxBinning() const;

   // set the subframe format (binning factor 1 assumed)
   //  needs two points of a rectangle that makes the subframe in pixel coords
   //  the subframe might get adjusted if binning changes
   //  switching back to the max frame is done by setting all points to 1
   bool SetSubframe(const wxPoint& start, const wxPoint& end);
   // Set the subframe to reflect full frame and redraw
   void SetFullFrame();

   size_t EnumerateCameras(wxArrayString& cameras);

   // the replies from the Camera functiions applies to the current active camera
   wxString CameraName() const;
   wxString CameraSerial() const;
   wxString CameraStatus() const;

   // Image storage handling
   bool StartCapture(const wxString& dirpath,
                     EVideoSampleFormat format,
                     EContainerFormat container,
                     bool batchCapture);
   void StopCapture();
   void PauseCapture();        //20080819:BM new
   void ContinueCapture();     //20080819:BM new

   // abort the current take and start a new exposure (don't capture the aborted one)
   void RestartExposure();

   // define the duration of the LE
   void SetLEduration(unsigned long milliseconds);

   // wxArtcamTimerCB callback implementation
   // regular timer event at the nominal pace
   void TimerEvent();
   // calls back if the exposure ends
   void ReadExposureEvent(bool aborted);
   // returns seconds left until exposure ends
   void ProgressEvent(unsigned long secondsLeft);

   // cooling support
   ECoolingStatus CoolingStatus() const;
   float CoolingSetpoint() const;
   float Temperature() const;
   bool  SetTemperature(float temp);
   bool  SetWarmup();

   // called via paintEvent either from sample grabber when a new sample
   // was written into the wxImage used as framebuffer
   // or while LE capture to show the LE sample
   void OnDraw(wxPaintEvent& event);
   void OnSize(wxSizeEvent& event);

   // path to camera in wxConfig
   wxString ConfigPath(const wxString& slotname);

   void SetImageProperties(double gamma, bool autoPoint, double blackPoint, double whitePoint);

   // liveImage returns a wxImage containing the "streaming" image
   // This should work in LE mode, normal mode, all platforms
   wxImage* liveImage() const;
   bool  liveImageInverted() const; // true when image is upside down
   wxSize liveSize() const;

   wxImage* canvasImage() const;
   wxSize canvasSize() const;

   // the following is used for managing zooming
   void ZoomRedraw(double zoomFact, bool redraw = true);

   EVideoSampleFormat VideoSampleFormat();

protected:
   void CaptureHandling();
   bool StartPreview();
   bool SelectCamera(const wxString& camera);
   bool StartExpoCycle();

   ArtBase* ds();

   void OnDrawOverlay(wxDC& dc);
   void OnDrawImage(wxDC& dc, wxImage* image, bool zoomed_view);
   //void OnDrawReticle(wxDC& dc, bool zoomed_view = false);

   void FinalizeSubframe();

protected:
   ArtBase*       m_ds;               // Artemis interface

   wxArtSample*   m_pArtSample;

   void UpdateImage(const wxSize& size, bool fullFrame);
   LiveImageArt*  m_liveImages;      // the live image container

   EVideoSampleFormat m_sampleFormat;
   int            m_driver_idx;        // current  camera index, or -1 if none


   // factor to hold binning when no artemis connected
   static unsigned short m_default_binFactor;

   wxArtcamTimer  m_timer;          // the general and LE timer object

   unsigned long  m_leMilliseconds; // the LExposure time
   bool           m_startupLE;      // true if we have to start a LE sequence (synched)

   bool           m_cooledCam;
   bool           m_warmupCam;

private:
   void LoadKeywordList(wxFitsKeywordList* kwdList);

protected:
   DECLARE_EVENT_TABLE()
};

#endif //__WXARTCAM_H__
