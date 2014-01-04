


#ifndef _WXCAMERA_H__
#define _WXCAMERA_H__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wxCameraTypes.h"
#include "wxHistogram.h"
#include "wxCamCoords.h"

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxEVT_WXAC_TIMER, -1)
DECLARE_EVENT_TYPE(wxEVT_WXAC_FRAME, -1)
END_DECLARE_EVENT_TYPES()


class wxImageStore;

class wxCamera  : public wxScrolledWindow {
public:
   wxCamera();
   wxCamera(wxWindow* parent, wxWindowID id= -1);
   virtual ~wxCamera();


   // wxCamera has the only implementation for those
   void wxWarning(const wxString& warning);

   // we are cheating here
   static wxCamera* Construct(ECType camType, wxWindow* parent, wxWindowID id= -1);

   // wxCamera has no default implementation for those !!
   virtual wxString VideoTechnology() const =0; // a short string "WDM", "V4L2" etc.

   virtual bool IsWebcam() const =0;
   virtual bool IsArtcam() const =0;

   // report the abilities of a certain camera
   virtual bool HasDriverDialogs() const =0;
   virtual bool HasZooming() const =0;
   virtual bool HasBinning() const =0;
   virtual bool HasSubframe() const =0;
   virtual bool HasCooling() const =0;
   virtual bool HasWarmup() const =0;
   virtual bool SupportsGuiding() const =0;

   // call intrinsic dialogs
   virtual void ShowFormatDialog() =0;
   virtual void ShowSourceDialog() =0;

   // wxWebcam has a default implementation for those !!
   virtual bool ConnectCamera(const wxString& camera) =0;

   virtual bool IsConnected() const;
   virtual void DisconnectCamera() =0;

   // set the camera format (binning factor) 1... max supported
   virtual bool SetBinning(unsigned short binFactor) =0;
   // get the cameras max binning factor
   virtual unsigned short MaxBinning() const =0;

   // set the subframe format (binning factor 1 assumed)
   //  needs two points of a rectangle that makes the subframe in pixel coords
   //  the subframe might get adjusted if binning changes
   //  switching back to the max frame is done by setting all points to 1
   virtual bool SetSubframe(const wxPoint& start, const wxPoint& end) =0;
//   virtual bool SetSubframe(unsigned short startX, unsigned short startY,
//                            unsigned short endX, unsigned short endY) =0;

   // Set the subframe to reflect full frame and redraw
   virtual void SetFullFrame() =0;
   virtual void Refresh();

   virtual size_t EnumerateCameras(wxArrayString& cameras) =0;

   // the replies from the Camera functiions applies to the current active camera
   virtual wxString CameraName() const =0;
   virtual wxString CameraSerial() const =0;
   virtual wxString CameraStatus() const =0;

   // start or batch capture process
   virtual bool RunBatchCapture();

   // Image storage handling
   virtual bool StartCapture(const wxString& dirpath,
                             EVideoSampleFormat format,
                             EContainerFormat container,
                             bool batchCapture=false) =0;
   virtual bool IsCapturing() const;

   virtual void StopCapture() =0;
   virtual void PauseCapture() =0;        //20080819:BM new
   virtual void ContinueCapture() =0;     //20080819:BM new
   virtual bool IsCapturePausing() const; //20080819:BM new

   virtual size_t  FramesCaptured() const; // returns frames captured since StartCapture if active

   // abort the current take and start a new exposure (don't capture the aborted one)
   virtual void RestartExposure() =0;

   // define the duration of the LE
   virtual void SetLEduration(unsigned long milliseconds) =0;

   // path to camera in wxConfig
   virtual wxString ConfigPath(const wxString& slotname) =0;

   virtual void ImageProperties(double& gamma, bool& autoPoint, double& blackPoint, double& whitePoint);
   virtual void SetImageProperties(double gamma, bool autoPoint, double blackPoint, double whitePoint);

   // liveImage returns a wxImage containing the "streaming" image
   // This should work in LE mode, normal mode, all platforms
   virtual wxImage* liveImage() const =0;
   virtual bool     liveImageInverted() const =0; // true when image is upside down
   virtual wxSize liveSize() const=0;
   virtual int liveWidth() const {return liveSize().GetWidth();}
   virtual int liveHeight() const {return liveSize().GetHeight();}

   virtual wxImage* canvasImage() const =0;
   virtual wxSize canvasSize() const =0;
   virtual int canvasWidth() const  {return canvasSize().GetWidth();}
   virtual int canvasHeight() const  {return canvasSize().GetHeight();}

   // cooling support
   virtual ECoolingStatus CoolingStatus() const = 0;
   virtual float CoolingSetpoint() const =0;
   virtual float Temperature() const =0;
   virtual bool  SetTemperature(float temp) =0;
   virtual bool  SetWarmup() =0;

   // image histogram support
   wxHistogram& Histogram();

   virtual bool imagePointfromDC(const wxPoint& pointDC, wxRealPoint& imagePoint); // convert to image from DC

   // enable reticle on/off
   virtual void EnableReticle(bool show);
   virtual bool isReticleEnabled() const;

   virtual double zoomFactor() const;
   virtual void unZoom(); // reset zoom factor to 1.0 and redraw
   virtual void ZoomRedraw(double zoomFact, bool redraw = true) =0;
   virtual bool IsMousetracking() const;

   // returns current video sample format
   virtual EVideoSampleFormat VideoSampleFormat() = 0;

protected:

   virtual void OnDrawOverlay(bool in_paint_event);
   virtual void OnDrawOverlay(wxDC& dc) = 0; // temporarily pure
   virtual void OnDrawReticle(wxDC& dc, bool zoomed_view = false);
   virtual void OnDrawImage(wxDC& dc, wxImage* image, bool zoomed_view = false);
   // OnDrawRubberband is part of overlay drawing
   virtual void OnDrawRubberband(wxDC& dc);
   virtual void OnDrawCapturedFrames(wxDC& dc);

   // the following is used for managing zooming
   virtual void OnMouseWheel(wxMouseEvent& event);
   virtual void OnMouseMove(wxMouseEvent& event);

   virtual void OnMouseLeftDown(wxMouseEvent& event);
   virtual void OnMouseLeftUp(wxMouseEvent& event);

   virtual void OnMouseDClick(wxMouseEvent& event);
   virtual void OnMouseRightDown(wxMouseEvent& event);
   virtual void OnMouseRightUp(wxMouseEvent& event);
   virtual void OnMouseEnterWindow(wxMouseEvent& event);
   virtual void OnMouseLeaveWindow(wxMouseEvent& event);
   virtual void FinalizeSubframe();

protected:
   wxWindow*      m_parent;           // parent window
   wxImageStore*  m_imageStore;     // an image store handler

   bool           m_isConnected;
   bool           m_isCapturing;
   bool           m_isCapturePausing;  //20080819:BM new
   bool           m_captureError;      //20080525:BM new
   bool           m_batchCapture;      //20090329:BM new
   size_t         m_frames_captured;

   float          m_gamma;
   bool           m_autoPoint;
   long           m_blackPoint;
   long           m_whitePoint;

   wxCamCoords*   m_camCoord;       // coordinate system helper
   double         m_zoomfact;       // zoom factor, 1.0=not zoomed <1.0 means smaller image

   class FmtChangeRec {
   public:
      FmtChangeRec() { Init(); };
      void Init() {fmtBinChangePending=false; fmtSubChangePending=false; bin=0; s=wxPoint(0,0); e=s;}
      void Reset() {fmtBinChangePending=false; fmtSubChangePending=false; }
      bool NeedsBinChange() const {return fmtBinChangePending;}
      bool NeedsSubChange() const {return fmtSubChangePending;}
      void SetBinning(unsigned short binning) {bin=binning; fmtBinChangePending=true;}
      void SetSubframe(const wxPoint& start, const wxPoint& end)
                        {s=start; e=end;  fmtSubChangePending=true;}
      unsigned short bin;
      wxPoint s, e;
   private:
      bool fmtBinChangePending;
      bool fmtSubChangePending;
   };
   FmtChangeRec   m_fmtChange;


   // class that allows setting points incrementally
   // and makes sure its dims are always positive (w and h)
   class wxAcRect {
   public:
      wxRect R, origR;
//      int  x, y, ox, oy;
//      int  w, h, ow, oh;
      void SetFirstPoint(const wxPoint& pt1, wxCamCoords& coSytem);
      void SetSecondPoint(const wxPoint& pt2, wxCamCoords& coSytem);

      wxRect  Rectangle() const {return R;}
      wxRect  OrigRectangle() const {return origR;}
      wxPoint StartPoint() const {return R.GetLeftTop();}
      wxPoint EndPoint() const {return R.GetRightBottom();}

   private:
      wxPoint m_firstPoint;
   };
   wxAcRect       m_mouseRect;
   bool           m_mouseTracked;   // true if the right button is pressed in the imagearea
   bool           m_leftScroll;     // true if scrolling using left mouse button
   wxPoint        m_leftScrollPos;

private:
  // typedef std::vector<int> Histogram;
  // Histogram        m_histo; // length=128, value=count
  // FrameChannel     m_channel; // channel used for histogram
   wxHistogram      m_histo;
   bool             m_overlayReticle; // crosshairs

private:
   DECLARE_ABSTRACT_CLASS(wxCamera)  /// ??? still needed ?
};




#endif  // _WXCAMERA_H__

