

#include "wxCamera.h"

#include "../wxFactory.h"

#include <wx/dcbuffer.h>

#include "wxWebcam.h"
#include "wxArtcam.h"

#include "wxImageStore.h"
#include "../wxMessageArea.h"
#include "../BatchJobManager.h"

wxCamera* wxCamera::Construct(ECType camType, wxWindow* parent, wxWindowID id)
{
   switch (camType)
   {
      case ECT_Artcam:
         return (wxCamera*) wxArtcam::Construct(parent,id);
      break;

      case ECT_Unknown:
      case ECT_Webcam:
      default:
         return (wxCamera*) wxWebcam::Construct(parent,id);
   }//switch
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_ABSTRACT_CLASS(wxCamera,wxScrolledWindow)


wxCamera::wxCamera()
: wxScrolledWindow()
, m_imageStore(NULL)
, m_isConnected(false)
, m_isCapturing(false)
, m_isCapturePausing(false)
, m_captureError(false)
, m_batchCapture(false)
, m_frames_captured(0)
, m_gamma(1.0)
, m_autoPoint(true)
, m_blackPoint(0)
, m_whitePoint(0) // set properly in specific constructor
, m_camCoord(NULL)
, m_zoomfact(1.0)
, m_mouseTracked(false)
, m_leftScroll(false)
, m_overlayReticle(false)
{
 //  m_histo.resize(128);
   m_histo.SetChannel(wxHistogram::Lchan);
}

//!20080907BM: slightly larger initial area to allow scrollers to vanish at zoom 1.0
wxCamera::wxCamera(wxWindow* parent, wxWindowID id)
: wxScrolledWindow(parent, id, wxDefaultPosition, wxSize(650,490), wxTAB_TRAVERSAL|wxFULL_REPAINT_ON_RESIZE/*|wxEXPAND|wxSHAPED*/)
, m_parent(parent)
, m_imageStore(NULL)
, m_isConnected(false)
, m_isCapturing(false)
, m_isCapturePausing(false)
, m_captureError(false)
, m_batchCapture(false)
, m_frames_captured(0)
, m_gamma(1.0)
, m_autoPoint(true)
, m_blackPoint(0)
, m_whitePoint(256)
, m_camCoord(NULL)
, m_zoomfact(1.0)
, m_mouseTracked(false)
, m_leftScroll(false)
, m_overlayReticle(false)
{
 //  m_histo.resize(128);
   m_histo.SetChannel(wxHistogram::Lchan);
}

wxCamera::~wxCamera()
{
   if (m_camCoord) delete m_camCoord; m_camCoord=NULL;
   if (m_imageStore) delete m_imageStore; m_imageStore=NULL;

}


void wxCamera::wxWarning(const wxString &warning)
{
    wxMessageBox(warning);
}


// new histogram interface
wxHistogram& wxCamera::Histogram()
{
   return m_histo;
}

bool  wxCamera::IsConnected() const
{
   return m_isConnected;
}

bool wxCamera::IsCapturing() const
{
   return m_isCapturing;
}

bool wxCamera::IsCapturePausing() const
{
   return m_isCapturePausing;
}


size_t wxCamera::FramesCaptured() const
{
   return m_frames_captured;
}

double wxCamera::zoomFactor() const
{
   return m_zoomfact;
}

void wxCamera::unZoom()
{
   ZoomRedraw(1.0);
}


void wxCamera::EnableReticle(bool show)
{
   m_overlayReticle = show;
   OnDrawOverlay(false);
}

bool wxCamera::isReticleEnabled() const
{
   return m_overlayReticle;
}

// common part of this interface
void wxCamera::ImageProperties(double& gamma, bool& autoPoint, double& blackPoint, double& whitePoint)
{
   gamma = m_gamma; autoPoint = m_autoPoint;
   double bp = (double)m_blackPoint, wp = (double)m_whitePoint;
   blackPoint = bp; whitePoint = wp;
}

void wxCamera::SetImageProperties(double gamma, bool autoPoint, double blackPoint, double whitePoint)
{
   m_gamma = (float)gamma;
   m_autoPoint = autoPoint;
   m_blackPoint = (long)blackPoint; m_whitePoint = (long)whitePoint;
}

// convert to image from DC
bool wxCamera::imagePointfromDC(const wxPoint& pointDC, wxRealPoint& pointImage)
{
   wxImage* image = liveImage();
   if(!image)return false;

   // this is outside a paint event so use wxClientDC !!!
   wxClientDC dc(this);
   PrepareDC(dc);
   wxCoord width,height;
   dc.GetSize(&width, &height);
   double scale_x = double(width)/image->GetWidth();
   double scale_y = double(height)/image->GetHeight();

   if(liveImageInverted()) {
      pointImage.x = pointDC.x/scale_x;
      pointImage.y = (height - pointDC.y)/scale_y;
   }
   else {
      pointImage.x = pointDC.x/scale_x;
      pointImage.y = pointDC.y/scale_y;
   }
   return true;
}

void wxCamera::Refresh()
{
   m_parent->Refresh();
}


// This is the standard wxCamera OnDrawOverlay version
// it gets the cameras live image, derives the DC
// then calls OnDrawOverlay(dc) which is always camera specific (no generic version available)
// Note that this function may be overidden on some cameras and/or platforms
//  especially if using subframes as this one does not know about it
void wxCamera::OnDrawOverlay(bool in_paint_event)
{
   wxImage* image = liveImage();
   if(image->Ok()) {
      if(in_paint_event) {
         // this is inside a paint event so use paintDC
         wxAutoBufferedPaintDC dc(this); //, wxBUFFER_VIRTUAL_AREA);
         PrepareDC(dc); // even so the Buffered should not require Prepare this one does
         OnDrawOverlay(dc);
      }
      else {
         // this is outside a paint event so use wxClientDC !!!
         wxClientDC dc(this);  // outside OnPaint events
         PrepareDC(dc);
         wxSize   iSize(image->GetWidth(), image->GetHeight());
         wxBufferedDC bdc(&dc, iSize);
         OnDrawOverlay(bdc);
      }
   }
}

void wxCamera::OnDrawImage(wxDC& dc, wxImage* image, bool zoomed_view)
{

   if(image && image->IsOk()) {

      // get the size of the client area
      wxCoord width,height;
      dc.GetSize(&width, &height);

      if(zoomed_view) {
         // scale the image based the zoom factor

         int vir_width = (int)(image->GetWidth()*zoomFactor());
         int vir_height = (int)(image->GetHeight()*zoomFactor());

         dc.DrawBitmap(image->Scale(vir_width,vir_height),0,0);
         dc.SetBrush(*wxBLACK_BRUSH);
         dc.DrawRectangle(0,vir_height,width,height);
         dc.DrawRectangle(vir_width,0,width,height);

      }
      else {
         // scale the image after the client area
         if((width== image->GetWidth()) && (height == image->GetHeight()) ) {
            // m_wximage and device context have same size, draw unscaled
            dc.DrawBitmap(*image,0,0);
         }
         else {
            // m_wximage and device context have different sizes, draw a scaled image.
            dc.DrawBitmap(image->Scale(width,height),0,0);
         }
      }
   }
}


void wxCamera::OnDrawReticle(wxDC& dc, bool WXUNUSED(zoomed_view))
{
   if(!IsConnected()) return;
   if(!isReticleEnabled()) return;

   wxCamPoint size(canvasWidth(), canvasHeight()); // scale to backg image
   m_camCoord->SetPoint(size, wxCamCoords::EPO_Image);
   size = m_camCoord->GetPoint(wxCamCoords::EPO_DC);

   short h2=short(size.y*0.5), w2=short(size.x*0.5);
   short diam = (h2 < w2)? h2 : w2;
   short rrad1 = diam/3;
   short rrad2 = rrad1*2;
   short cx = short(size.x/2);
   short cy = short(size.y/2);

   dc.SetPen(*wxRED_PEN);
   dc.SetBrush(*wxTRANSPARENT_BRUSH);
   dc.DrawCircle(cx, cy, rrad1);
   dc.DrawCircle(cx, cy, rrad2);
   dc.DrawLine(cx-w2, cy, cx+w2, cy);
   dc.DrawLine(cx, cy-h2, cx, cy+h2);
   dc.SetBrush(wxNullBrush);
   dc.SetPen(wxNullPen);
}

void wxCamera::OnDrawRubberband(wxDC& dc)
{
   if(!IsConnected()) return;
   if(!IsMousetracking())return;

   dc.SetPen(*wxGREEN_PEN);
   dc.SetBrush(*wxTRANSPARENT_BRUSH);
   // draws in mouse coordinates on DC
   dc.DrawRectangle(m_mouseRect.OrigRectangle());
   // reset resources used
   dc.SetBrush(wxNullBrush);
   dc.SetPen(wxNullPen);
}


void wxCamera::OnDrawCapturedFrames(wxDC& dc)
{
   if(!IsConnected()) return;
   if (!IsCapturing()) return;
   if (! wxF()->getNamePolicyFile() ) return;


   // this is dependent of the kind of container (could be done better I presume)
   size_t frameC;
   if ( wxF()->containerFormat()==ECF_AVI ) {
      frameC = m_frames_captured;
   }
   else {
      frameC = wxF()->getNamePolicyFile()->GetIncrement();
   }

   wxString label = wxT("");
   // get info about counter font size or whether it is turned off
   wxFactory::FrameCounterSize font_size = wxF()->FrameCounterFontSize();
   if(font_size != wxFactory::CNT_OFF) {
      label = wxString::Format(wxT("# %u"), frameC);
   }
   else {
      font_size = wxFactory::CNT_SMALL; // we use this also for the timestamp
   };
   int point_size = int(font_size);

   EVideoSampleFormat video_format = VideoSampleFormat();
   if(EVF_YUV == video_format) {
      // we may have to draw the time indicator as a label
      wxFactory::FrameTimeStampSize fts_size = wxF()->FrameTimeStampFontSize();
      if(fts_size != wxFactory::FTS_OFF) {
         wxDateTime now = wxDateTime::UNow().ToUTC();
         wxDateTime::wxDateTime_t ms = now.GetMillisecond();
         if(label.length() > 0) {
            label += wxT(" - ");
         }
         else {
            point_size = 2*int(fts_size);
         }
         label += wxString::Format(wxT("%.2d:%.2d:%.2d.%.3d"), (int)now.GetHour(),(int)now.GetMinute(),(int)now.GetSecond(), ms);
      }
   }

   if(label.length() > 0) {
      wxFont fnt = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
      dc.SetTextForeground(wxColour(255, 80, 0));
      fnt.SetPointSize(point_size);
      dc.SetFont(fnt);

      //20080823:BM draw the label top left independent of scrolling
      int xx(0),yy(0);
      CalcUnscrolledPosition(5, 25+ wxF()->FpsFontSize()+point_size,&xx,&yy); // wxWidgets supplied helper
      dc.DrawText(label, xx, yy);
   }
}



/// //////////////////////////////////////////////////////////////////////////
/// MOUSE EVENTHANDLER SECTION
/// //////////////////////////////////////////////////////////////////////////

bool wxCamera::IsMousetracking() const
{
   return m_mouseTracked;
}


void wxCamera::OnMouseWheel(wxMouseEvent& event)
{
    //20080825:CA Zoomed guiding not supported
   if (wxF()->guider()->guiding_active())return;

   wxUint16 w,h;

   // get the real image dims (could be subframe which affects maxzoom only!!!)
   w = liveImage()->GetWidth();
   h = liveImage()->GetHeight();

   // get the client size
   wxClientDC dc(this);  // outside paint event use wxClientDC !!!
   PrepareDC(dc);
   wxCoord width,height;
   dc.GetSize(&width, &height);

   // do not zoom more than 5 times
   double max_zoom = 5.0*double(width)/double(w);
   if(max_zoom < 1.0)max_zoom=1.0;

   double rot = event.GetWheelRotation()/event.GetWheelDelta();
   if(rot == 0) {
      m_zoomfact = 1.0;
   }
   else  {
      m_zoomfact += rot/20.;
      if(m_zoomfact<0.05)m_zoomfact=0.05;
      if(m_zoomfact>max_zoom)m_zoomfact=max_zoom;
   }

   // redraw zoomed with scrollbars etc
   ZoomRedraw(m_zoomfact);
}

void wxCamera::OnMouseLeftDown(wxMouseEvent& event)
{
   m_leftScroll = true;

   // convert to unscrolled coordinates
   wxPoint pos = event.GetPosition();
   int xx(0),yy(0);
   CalcUnscrolledPosition(pos.x,pos.y,&xx,&yy); // wxWidgets supplied helper
   m_leftScrollPos = wxPoint(xx,yy);

   // the skip is needed here, or else mouse wheel scrolling will not work on Win
   event.Skip(true);
}

void wxCamera::OnMouseLeftUp(wxMouseEvent& event)
{

   m_leftScroll = false;
   event.Skip(true);
}

// standard size (fullframe)
void wxCamera::OnMouseDClick(wxMouseEvent& WXUNUSED(event))
{
   SetFullFrame();
}

void wxCamera::OnMouseMove(wxMouseEvent& event)
{
   if (m_mouseTracked) {
      if (event.RightIsDown()) {

         // convert to unscrolled coordinates
         wxPoint pos = event.GetPosition();
         int xx(0),yy(0);
         CalcUnscrolledPosition(pos.x,pos.y,&xx,&yy); // wxWidgets supplied helper
         xx = (xx > 0)? xx : 0;
         yy = (yy > 0)? yy : 0;

         pos.x = xx;
         pos.y = yy;
         m_mouseRect.SetSecondPoint(pos, *m_camCoord); // DC coords

         // we refresh the window, causing OnDrawOverlay to be triggered, which again
         // causes OnDrawRubberband
         wxWindow::Refresh(false);
      }
   }
   else if(m_leftScroll) {

      // convert mouse point to unscrolled coordinates
      wxPoint pos = event.GetPosition();

      int xx(0),yy(0);
      CalcUnscrolledPosition(pos.x,pos.y,&xx,&yy); // wxWidgets supplied helper

      // compute difference between current and original
      // mouse position in unscrolled dc coordinates.
      // This defines the relative scrolling needed
      int dx = m_leftScrollPos.x - xx;
      int dy = m_leftScrollPos.y - yy;

      // compute unscrolled dc coordinates of upper left dc point
      int sx,sy;
      CalcUnscrolledPosition(0,0,&sx,&sy);

      // now we can compute the absolute scrolling
      Scroll(sx+dx,sy+dy);
   }
   else {
      event.Skip(true); // needed otherwise others don't see any event...
   }

}

void wxCamera::OnMouseRightDown(wxMouseEvent& event)
{
   m_mouseTracked=true;

   // convert to unscrolled coordinates
   wxPoint pos = event.GetPosition();
   int xx(0),yy(0);
   CalcUnscrolledPosition(pos.x,pos.y,&xx,&yy); // wxWidgets supplied helper
   pos.x = xx;
   pos.y = yy;

   m_mouseRect.SetFirstPoint(pos, *m_camCoord); // DC coords
}

void wxCamera::OnMouseRightUp(wxMouseEvent& event)
{
   if (m_mouseTracked) {

      // convert to unscrolled coordinates
      wxPoint pos = event.GetPosition();
      int xx(0),yy(0);
      CalcUnscrolledPosition(pos.x,pos.y,&xx,&yy); // wxWidgets supplied helper
      xx = (xx > 0)? xx : 0;
      yy = (yy > 0)? yy : 0;

      pos.x = xx;
      pos.y = yy;

      m_mouseRect.SetSecondPoint(pos, *m_camCoord); // DC coords

      FinalizeSubframe();
   }
   else {
      event.Skip(true);
   }
   m_mouseTracked=false;
}

void wxCamera::OnMouseEnterWindow(wxMouseEvent& WXUNUSED(event))
{
   // grab focus when mouse hovers over window
   SetFocus();
}

void wxCamera::OnMouseLeaveWindow(wxMouseEvent& event)
{
   if (m_mouseTracked) {

      wxPoint pos = event.GetPosition();
      int xx(0),yy(0);
      CalcUnscrolledPosition(pos.x,pos.y,&xx,&yy); // wxWidgets supplied helper

      bool outside = false;

      // if the mouse is dragged outside the CCD, we may have to
      // finalise the rectangle at the boundary

      // map from Image to CCD coordinates
      wxCamPoint point(xx, yy);
      m_camCoord->SetPoint(point, wxCamCoords::EPO_Image);
      point = m_camCoord->GetPoint(wxCamCoords::EPO_CCD);
      if(point.x < 0) {
         point.y = 0;
         outside = true;
      }
      if(point.y < 0) {
         point.y = 0;
         outside = true;
      }

      if(!outside) {
         wxSize fsPt = m_camCoord->GetFullSize(wxCamCoords::EPO_DC);
         if(point.x > fsPt.x) {
            point.x = fsPt.x;
            outside = true;
         }
         if(point.y > fsPt.y) {
            point.y = fsPt.y;
            outside = true;
         }
      }

      if(outside)FinalizeSubframe();
   }
   else {
      event.Skip(true);
   }

   // don't hold on to the focus when leaving the window
   GetParent()->SetFocus();
}

void wxCamera::FinalizeSubframe()
{
   m_mouseTracked=false;
}

/// //////////////////////////////////////////////////////////////////////////
/// MOUSE TRACKING SECTION
/// //////////////////////////////////////////////////////////////////////////


// Just saves the first point of a selected rectangle
void wxCamera::wxAcRect::SetFirstPoint(const wxPoint& pt1, wxCamCoords& coSytem)
{
   wxPoint newP = pt1;
   // check boundaries of the real image
   wxSize fsPt = coSytem.GetFullSize(wxCamCoords::EPO_DC);
   if (newP.x<0) newP.x=0;             if (newP.y<0) newP.y=0;
   if (newP.x>=fsPt.x) newP.x=fsPt.x;  if (newP.y>=fsPt.y) newP.y=fsPt.y;

   m_firstPoint = newP;
   origR.SetTopLeft(m_firstPoint);
}

// Receives the second point of the rectangle and calculates
// the rect. dimensions based on the actual zoomlevel
// and retains the original world coords of the image in ox,oy,ow,oh
void wxCamera::wxAcRect::SetSecondPoint(const wxPoint& pt2, wxCamCoords& coSytem)
{
   wxPoint newP = pt2;
   // check boundaries of the real image
   wxSize fsPt = coSytem.GetFullSize(wxCamCoords::EPO_DC);
   if (newP.x<0) newP.x=0;             if (newP.y<0) newP.y=0;
   if (newP.x>=fsPt.x) newP.x=fsPt.x;  if (newP.y>=fsPt.y) newP.y=fsPt.y;

   // save original mouse points
   origR.SetBottomRight(newP);

   // make always a positive oriented rectangle
   wxCamPoint sPt, ePt;
   if (m_firstPoint.x<newP.x) {
      if (m_firstPoint.y<newP.y) {
         sPt.SetPoint(m_firstPoint.x, m_firstPoint.y);
         ePt.SetPoint(newP.x, newP.y);
      }
      else { // y exchanged
         sPt.SetPoint(m_firstPoint.x, newP.y);
         ePt.SetPoint(newP.x, m_firstPoint.y);
      }
   }
   else { // x exchanged
      if (m_firstPoint.y<newP.y) {
         sPt.SetPoint(newP.x, m_firstPoint.y);
         ePt.SetPoint(m_firstPoint.x, newP.y);
      }
      else { // y exchanged
         sPt.SetPoint(newP.x, newP.y);
         ePt.SetPoint(m_firstPoint.x, m_firstPoint.y);
      }
   }
   // scale gathered points according to current scale
   coSytem.SetPoint(sPt, wxCamCoords::EPO_DC);
   sPt = coSytem.GetPoint(wxCamCoords::EPO_Image);

   coSytem.SetPoint(ePt, wxCamCoords::EPO_DC);
   ePt = coSytem.GetPoint(wxCamCoords::EPO_Image);
   // set mouse rectangle
   R.SetTopLeft(sPt.IntPoint());
   R.SetBottomRight(ePt.IntPoint());
}

bool wxCamera::RunBatchCapture()
{
   wxString folder = wxF()->captureFolder();
   EVideoSampleFormat sformat = wxF()->sampleFormat();
   EContainerFormat cformat = wxF()->containerFormat();
   BatchJobManager* bjm = wxF()->batchJobManager();

   // if there are jobs waiting, start the processing
   if(bjm->JobAvailable()) {

      // activate the batch job manager
      bjm->setActive(true);

      // get the current active job
      BatchJob& bj = bjm->ActiveJob();
      if(bj.FramesRemaining() <= 0) {
         // here we end with a new job that has no frames left ???
         wxArrayString messages;
         messages.Add(wxT("Empty Batch Job encountered - please report this as bug...."));
         wxMessageArea::wxMessageSink(messages,wxT("Camera - RunBatchCapture"));
         return false;
      }
      else {
         // set the exposure time
         unsigned long msec = (unsigned long)(1000*bj.ExpoTime());
         SetLEduration(msec);

         // CA 20090331 Looks like we have to restart exposure here to get the correct
         // exposure length of first batch frame
         RestartExposure();

         // actually start the capture process
         return StartCapture(folder,sformat,cformat, true); //BM:20090329 - submit batch tag // CA: 20090411: handle return value
         // the management of the subsequent frames and exposure lengths, is
         // handled in CaptureHandling()
         // return value true means we submitted a job
      }
   }

   // nothing available
   return false;
}
