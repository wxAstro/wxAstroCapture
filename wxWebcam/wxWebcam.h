// wxWebcam.h: interface for the wxWebcam class.
//
//////////////////////////////////////////////////////////////////////

#ifndef WXWEBCAM_H
#define WXWEBCAM_H

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/config.h>

#include "captypes.h"
#include "capturebase.h"
#include "wxCamera.h"
#include "wxWebcamTimer.h"
#include "wxWebcamTimerCB.h"

#include "../wxIO/wxPortManager.h"


class wxConfig;
class CaptureBase;
class wxFitsKeywordList;
//* LE stuff
class wxLongExpoPort;
class wxPort;
class LiveImageWeb;
class wxWebSample;

class wxWebcam  : public wxCamera, public wxWebcamTimerCB {
public:
   wxWebcam();
   wxWebcam(wxWindow* parent, wxWindowID id= -1);
   virtual ~wxWebcam();

   // we are cheating here
   static wxWebcam* Construct(wxWindow* parent, wxWindowID id= -1);

   // wxWebcam has no default implementation for those !!
   virtual wxString VideoTechnology() const =0; // a short string "WDM", "V4L2" etc.

   bool IsWebcam() const {return true;};
   bool IsArtcam() const {return false;};

   // report the abilities of a certain camera (default webcam answers)
   virtual bool HasDriverDialogs() const =0;
   virtual bool HasZooming() const {return true;};  //20080525:BM support more features
   virtual bool HasBinning() const {return false;};
   virtual bool HasSubframe() const {return true;}; //20080525:BM support more features
   virtual bool HasCooling() const {return false;};
   virtual bool HasWarmup() const {return false;};
   virtual bool SupportsGuiding() const {return true;};

   // call intrinsic dialogs
   virtual void ShowFormatDialog() =0;
   virtual void ShowSourceDialog() =0;


   // wxWebcam has a default implementation for those !!
   virtual bool ConnectCamera(const wxString& camera);

   virtual void DisconnectCamera();

   // Webcams do not really support binning so leave it alone
   // set the camera format (binning factor) 1... max supported
   static bool SetDefaultBinning(unsigned short WXUNUSED(binFactor)){return false;};
   static unsigned short DefaultBinning() { return 1;};
   // set the camera format (binning factor) 1... max supported
   bool SetBinning(unsigned short WXUNUSED(binFactor)){return false;};
   // get the cameras max binning factor
   unsigned short MaxBinning() const { return 1;};

   // set the subframe format (binning factor 1 assumed)
   //  needs two points of a rectangle that makes the subframe in pixel coords
   //  the subframe might get adjusted if binning changes
   //  switching back to the max frame is done by setting all points to 0 (zero)
   virtual bool SetSubframe(const wxPoint& start, const wxPoint& end);
   virtual void SetFullFrame();

   virtual size_t EnumerateCameras(wxArrayString& cameras);

   // the replies from the Camera functiions applies to the current active camera
   virtual wxString CameraName() const;
   virtual wxString CameraSerial() const;
   virtual wxString CameraStatus() const;

   // Image storage handling
   virtual bool StartCapture(const wxString& dirpath,
                             EVideoSampleFormat format,
                             EContainerFormat container,
                             bool batchCapture);
   virtual void StopCapture();
   virtual void PauseCapture();        //20080819:BM new
   virtual void ContinueCapture();     //20080819:BM new

   // abort the current take and start a new exposure (don't capture the aborted one)
   virtual void RestartExposure();

   // returns true when LE is ongoing
   virtual bool IsLExposing() const;
   // en/disables LE capturing
   virtual bool SwitchLE(bool enabled);
   // define how many images are ignored after switching CCD readout on (default=1)
   virtual void SetCaptureDelay(size_t numImages) {m_capDelay = numImages;};
   // define the duration of the LE
   virtual void SetLEduration(unsigned long milliseconds) {m_leMilliseconds = milliseconds;};

   // wxWebcamTimerCB callback implementation
   // regular timer event at the nominal pace
   virtual void TimerEvent();
   // special timer event
   virtual void TimerXEvent();
   // calls back the AmpLeadTime before the exposure ends
   virtual void TriggerExposureEvent(bool aborted);
   // calls back if the exposure ends
   virtual void ReadExposureEvent(bool aborted);
   // returns seconds left until exposure ends
   virtual void ProgressEvent(unsigned long secondsLeft);

   // cooling support
   virtual ECoolingStatus CoolingStatus() const {return ECS_NotAvailable;};
   virtual float CoolingSetpoint() const {return 99.9f;};
   virtual float Temperature() const {return 99.9f;};
   virtual bool  SetTemperature(float WXUNUSED(temp)) {return false;};
   virtual bool  SetWarmup()  {return false;};


   // called via paintEvent either from sample grabber when a new sample
   // was written into the wxImage used as framebuffer
   // or while LE capture to show the LE sample
   virtual void OnDraw(wxPaintEvent& event);

   // the DeviceProperties class provides access to all the
   // camera controls, shutter speed, gain etc.
   // implemented via capturebase abstract class
   class DeviceProperties {
   public:

      DeviceProperties(wxWebcam* camera);
      virtual ~DeviceProperties() {}

      // Query for availablity and type if support for each property
      EVidCapPropertySupport HasProperty_Brightness();
      EVidCapPropertySupport HasProperty_Contrast();
      EVidCapPropertySupport HasProperty_Hue();
      EVidCapPropertySupport HasProperty_Saturation();
      EVidCapPropertySupport HasProperty_Sharpness();
      EVidCapPropertySupport HasProperty_Gamma();
      EVidCapPropertySupport HasProperty_ColorEnable();
      EVidCapPropertySupport HasProperty_BacklightCompensation();
      EVidCapPropertySupport HasProperty_WhiteBalanceMode();
      EVidCapPropertySupport HasProperty_WhiteBalRed();
      EVidCapPropertySupport HasProperty_WhiteBalBlue();
      EVidCapPropertySupport HasProperty_FlickerFree();
      EVidCapPropertySupport HasProperty_Flicker50_60();
      EVidCapPropertySupport HasProperty_FPS();
      EVidCapPropertySupport HasProperty_Exposure();
      EVidCapPropertySupport HasProperty_Gain();
      EVidCapPropertySupport HasProperty_FlipHorizontal();
      EVidCapPropertySupport HasProperty_FlipVertical();
      EVidCapPropertySupport HasProperty_AutoExposure();
      EVidCapPropertySupport HasProperty_AutoAll();

      // retrieve raw raw/percent values, or bool
      long Property_Brightness(bool percent);
      long Property_Contrast(bool percent);
      long Property_Hue(bool percent);
      long Property_Saturation(bool percent);
      long Property_Sharpness(bool percent);
      long Property_Gamma(bool percent);
      bool Property_ColorEnable();
      long Property_BacklightCompensation(bool percent);
      long Property_WhiteBalanceMode();
      long Property_WhiteBalRed(bool percent);
      long Property_WhiteBalBlue(bool percent);
      bool Property_FlickerFree();
      long Property_Flicker50_60(bool percent);
      long Property_FPS();
      long Property_Exposure();
      long Property_Gain(bool percent);
      bool Property_FlipHorizontal();
      bool Property_FlipVertical();
      bool Property_AutoExposure();
      bool Property_AutoAll();

      // set property values, as raw/percent value, or bool
      void SetProperty_Brightness(long propValue, bool percent);
      void SetProperty_Contrast(long propValue, bool percent );
      void SetProperty_Hue(long propValue, bool percent );
      void SetProperty_Saturation(long propValue, bool percent );
      void SetProperty_Sharpness(long propValue, bool percent );
      void SetProperty_Gamma(long propValue, bool percent );
      void SetProperty_ColorEnable(bool ebable );
      void SetProperty_BacklightCompensation(long propValue, bool percent );
      void SetProperty_WhiteBalanceMode(long propValue);
      void SetProperty_WhiteBalRed(long propValue, bool percent );
      void SetProperty_WhiteBalBlue(long propValue, bool percent );
      void SetProperty_FlickerFree(bool enable);
      void SetProperty_Flicker50_60(bool enable50);
      void SetProperty_FPS(long propValue);
      void SetProperty_Exposure(long propValue);
      void SetProperty_Gain(long propValue, bool percent );
      void SetProperty_FlipHorizontal(bool enable );
      void SetProperty_FlipVertical(bool enable );
      void SetProperty_AutoExposure( bool enable );
      void SetProperty_AutoAll( bool enable );

      // retrieve min, max and step for raw property value
      void PropertyMinMax_Brightness(long& min, long& max, long& step);
      void PropertyMinMax_Contrast(long& min, long& max, long& step);
      void PropertyMinMax_Hue(long& min, long& max, long& step);
      void PropertyMinMax_Saturation(long& min, long& max, long& step);
      void PropertyMinMax_Sharpness(long& min, long& max, long& step);
      void PropertyMinMax_Gamma(long& min, long& max, long& step);
      void PropertyMinMax_BacklightCompensation(long& min, long& max, long& step);
      void PropertyMinMax_WhiteBalanceMode(long& min, long& max, long& step);
      void PropertyMinMax_WhiteBalRed(long& min, long& max, long& step);
      void PropertyMinMax_WhiteBalBlue(long& min, long& max, long& step);
      void PropertyMinMax_FPS(long& min, long& max, long& step);
      void PropertyMinMax_Exposure(long& min, long& max, long& step);
      void PropertyMinMax_Gain(long& min, long& max, long& step);

      // load/save user and factory settings int "active area"
      void Settings_LoadUser();
      void Settings_SaveUser();
      void Settings_LoadFactory();

      wxWebcam* m_camera;
   };

   // Provide access to the camera properties
   DeviceProperties* properties();

   // save and restore current camera properties to the configuration
   // The config object is preconfigured for the current slot, so no navigation is required
   bool SaveToConfig(wxConfig* config);
   bool RestoreFromConfig(wxConfig* config);
   bool RestoreDefaultSlot(wxConfig* config);

   // provide access to the underlying long exposure control object.
   // This is not meant for general use (use the wxWebcam own LE interface functions!)
   // But it is is needed for setting up the GUI.
   wxLongExpoPort* LongExpoPort();

   // path to camera in wxConfig
   wxString ConfigPath(const wxString& slotname);
   // trigger whenever a configuration is updated outside this object
   void ConfigUpdated(){};

   // liveImage returns a wxImage containing the "streaming" image
   // This should work in LE mode, normal mode, all platforms
   virtual wxImage* liveImage() const;
   virtual bool     liveImageInverted() const; // true when image is upside down
   virtual wxImage* frameImage() const;
   virtual wxSize liveSize() const;

   virtual wxImage* canvasImage() const;
   virtual wxSize canvasSize() const;

   void SetImageProperties(double gamma, bool autoPoint, double blackPoint, double whitePoint);

   void ZoomRedraw(double zoomFact, bool redraw = true);

   // guiding feedback support
   // setGuideBox enables drawing of a guide box
   virtual bool isGuiding() const;
   virtual void setGuideBox(const wxRealPoint& lockpos, // locked position in image coordinates
                            const wxRealPoint& curpos,  // current position in image coordinates
                            const wxSize& size);        // size in image poxels
   virtual void cancelGuideBox();                       // disables guide box drawing


protected:
   virtual void CaptureHandling();
   virtual bool StartPreview();
   virtual bool SelectCamera(const wxString& camera);
   virtual bool StartExpoCycle();
   // Long Exposure Interface of the Webcam
   // submit the IO port to be used with LE
   bool UseLEPort(wxLongExpoPort* lePort);

   CaptureBase* ds();

   virtual void OnDrawOverlay(bool in_paint_event);
   virtual void OnDrawOverlay(wxDC& dc);
   virtual void OnDrawImage(wxDC& dc, wxImage* image, bool zoomed_view);
   virtual void OnDrawGuidebox(wxDC& dc);
   virtual void OnDrawFps(wxDC& dc); //20100703:BM added - show current FPS

   void FinalizeSubframe();

   EVideoSampleFormat VideoSampleFormat();

protected:
   CaptureBase*   m_ds;               // DirectShow interface
   DeviceProperties* m_pProperties;   // the device property object (allocated OS specific)

   wxWebSample*   m_pWebSample;

   void UpdateImage(const wxSize& size, bool fullFrame);
   LiveImageWeb*  m_pLiveImages;      // the live image container
   bool           m_preview;
   bool           m_dropSample;       // if true, drop sample

   EVideoSampleFormat m_sampleFormat;
   int            m_driver_idx;        // current  camera index, or -1 if none

   // factor to hold binning when no artemis connected
   static unsigned short m_default_binFactor;

   wxWebcamTimer  m_timer;      // the general and LE timer object

   wxPortHandle   m_ioPortHandle;   // WE own this one
   wxLongExpoPort*m_pLongExpoPort;
   bool           m_isLExposing;
   unsigned long  m_capDelay;    // how many images to wait before taking the LE image
   unsigned long  m_leMilliseconds; // the LExposure time
   bool           m_doReadoutLE;      // true if we have to start a LE sequence (synched)
   float          m_realExpoTime;     // the measured exposure time
   unsigned long  m_lxSecLeft;

   // various variables used on overlay drawing
   wxRealPoint    m_overlayGuideLine[2]; // indicator of guiding error
   wxRect         m_overlayGuideBox;     // A box (in image coordinates) showing guiding subframe
   bool           m_overlayGuideBoxDraw; // true when guide box shall be drawn

   wxFloat32       m_fps;  // actual fps (reported frames)
   wxFloat32       m_dps;  // actual dps (dropped frames)


private:
   void LoadKeywordList(wxFitsKeywordList* kwdList);
   wxString SettingString();


   DECLARE_ABSTRACT_CLASS(wxWebcam)  /// ??? still needed ?
};

#endif
