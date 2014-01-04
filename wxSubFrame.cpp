#include "wxSubFrame.h"
#include <wx/utils.h>
#include <wx/timer.h>

// === helper class image2d

/*
   image2d.
   This extremely lightweight image class just borrows the wxImage data pointer temporarily,
   and provides a convenient wrapper around it to allow fast, easy and cheap access to pixels.
*/


class image2d {
public:
   image2d(unsigned char* image, int xdim, int ydim) : m_image(image),m_xdim(xdim),m_ydim(ydim) {}
   unsigned char* operator()(const int& x,const int& y) const { return m_image + y*m_xdim*3 + x*3 + m_chan; }
   int xdim() const {return int(m_xdim);}
   int ydim() const {return int(m_ydim);}
   int pixel_id(const int& x,const int& y) const { return y*m_xdim + x;}
private:
   unsigned char* m_image;
   int m_xdim;
   int m_ydim;
   static const int m_chan = 1;  // 0=R, 1=G, 2=B
};

// === end helper class

unsigned char  wxSubFrame::m_limit=100;
double         wxSubFrame::m_angle = 0;
double         wxSubFrame::m_imageScale=1.0;

const double pi = 4.0*atan(1.0);

double wxSubFrame::cameraAngle()
{
   return 180*m_angle/pi;
}

void wxSubFrame::setCameraAngle(double degrees)
{
   m_angle = pi*degrees/180.0;
}

double wxSubFrame::imageScale()
{
   return m_imageScale;
}

void wxSubFrame::setImageScale(double imageScale)
{
   m_imageScale = imageScale;
}

unsigned char wxSubFrame::cutoffLimit()
{
   return m_limit;
}

void wxSubFrame::setCutoffLimit(unsigned char limit)
{
   m_limit = limit;
}

wxSubFrame::wxSubFrame()
: m_timestamp_ms(0)
, m_locked(false)
, m_inverted(false)
{}

void wxSubFrame::setFrameInfo(const wxSize& fullsiz       // full frame size
                             ,const wxSize& subsiz        // subframe size
                             ,const wxRealPoint& subcen)  // subframe centre point in full frame
{
  m_fullsiz = fullsiz;
  m_subsiz  = subsiz;
  m_lockcen = subcen;
  m_curcen  = subcen;
  m_timestamp_ms = 0;
  m_locked = false;
}

wxSubFrame::~wxSubFrame()
{}

unsigned long wxSubFrame::timestamp_ms()
{
   return m_timestamp_ms;
}

// extract guiding subframe from fullframe
bool wxSubFrame::copySubFrame(const wxImage& fullframe, bool inverted)
{
   int fw  = fullframe.GetWidth();
   int fh  = fullframe.GetHeight();

   // if image has been inverted since we last saw it, we must invert the coords here
   if(m_locked && (m_inverted != inverted)) {
      m_curcen.y  = fh - m_curcen.y;
      m_lockcen.y = fh - m_lockcen.y;
   }

   m_inverted = inverted;

   // calculate lower left position of subframe in source
   // make sure low end of rectangle is within full frame
   int xlo = int(m_curcen.x - m_subsiz.x/2);
   int ylo = int(m_curcen.y - m_subsiz.y/2);

   if(xlo < 0)xlo = 0;
   if(ylo < 0)ylo = 0;

   // check high end of rectangle
   int xhi = xlo + m_subsiz.x;
   int yhi = ylo + m_subsiz.y;
   if( xhi >= fw) xlo = fw - (m_subsiz.x+1);
   if( yhi >= fh) ylo = fh - (m_subsiz.y+1);


   // record the actual centre position of the subframe
   m_curcen.x = xlo + m_subsiz.x/2;
   m_curcen.y = ylo + m_subsiz.y/2;

   // actually compute final rectangle and copy out the subframe
   wxPoint p1(xlo,ylo);
   wxRect rect(p1,m_subsiz);

   m_subframe = fullframe.GetSubImage(rect);
   if(inverted) {

      // The subframe must be flipped
      int w = m_subframe.GetWidth();
      int h = m_subframe.GetHeight();
      int line_size = 3*w;
      unsigned char* pix = m_subframe.GetData();
      unsigned char* cur = pix;
      unsigned char* mir = cur + line_size*(h-1);
      unsigned char* swp = new unsigned char[line_size];
      for(int iline=0; iline<h/2; iline++) {
         memcpy(swp,cur,line_size);
         memcpy(cur,mir,line_size);
         memcpy(mir,swp,line_size);
         cur += line_size;
         mir -= line_size;
      }
      delete[] swp;

 //     m_subframe = m_subframe.Mirror(false);
   }
/*
   m_subframe.SaveFile(wxT("subframe.bmp"),wxBITMAP_TYPE_BMP);
   exit(0);
*/
   // set the timestamp of the subframe
   m_timestamp_ms = computeTimeStampMillis();
   return true;
}

// computes a millisecond based time stamp for this time instant
unsigned long wxSubFrame::computeTimeStampMillis()
{
   // based on BM-20090411 - fixup the clock issue

   /*
      why clock() is unreliable
      ========================
      i) it is the CPU time in cpu clocks used by the program so far and NOT the
      clock time the program runs as the name suggests
      ii) the man page says that CLOCKS_PER_SEC must be 1000000 to be POSIX
      compliant independent of the resolution
      Which means that Linux ist POSIX compliant and Win does not bother.

      Instead we use wxGetLocalTimeMillis() which returns time since 1.1.1970
      in milliseconds. This is absolute time and using wxLongLong. We don't want or
      need absolute time. We just use the lower part of the _int64 which is
      ts.GetLo(), it returns the remainder of the division ts / 2^32 or the
      low unsigned int of that number - that is where the ms counting takes place,
      the High part is only to cope with the 1.1.1970 timespan in ms.

      The result is a timestamp in ms units that is able to represent at least
      49.7 day long time spans.
   */

   wxLongLong ts= wxGetLocalTimeMillis();
   unsigned long timestamp = ts.GetLo(); //return the lower part only
   return timestamp;
}


bool wxSubFrame::FindStar(wxStar& subframe_star)
{
   DonePixels done_pixels,done_paint;

   bool success = false;

   // scan the frame for the highest magnitude pixel
   // and paint the star in that position

   image2d subframe(m_subframe.GetData(),m_subframe.GetWidth(),m_subframe.GetHeight());
   int xdim = subframe.xdim();
   int ydim = subframe.ydim();
   for(int y=1; y<ydim-1; y++) {
      for(int x=1; x<xdim-1; x++) {

         // get current pixel
         unsigned char* pixel = subframe(x,y);

         // check to see if pixel value is over cutoff limit
         if(*pixel > m_limit) {

            // Find the local peak
            unsigned long peak_mag=0;
            int x_peak=-1;
            int y_peak=-1;
            FindPeak(subframe,done_pixels,x,y,peak_mag,x_peak,y_peak);

            // initialise a star from the peak value and mark the pixel location as painted
            wxStar local_star(x_peak,y_peak,peak_mag);
            done_paint.insert(subframe.pixel_id(x_peak,y_peak));

            int recursion = 0;
            PaintStar(subframe,local_star,x_peak,y_peak, done_paint,recursion);
            if(local_star.mag() > subframe_star.mag()) {

               // this was a star of greater magnitude
               subframe_star = local_star;
               success = true;
            }
         }
      }
   }

   return success;
}


void wxSubFrame::PaintStar(const image2d& subframe, wxStar& star, int x, int y, DonePixels& done, int& recursion)
{
   // if recursion is too extreme, give up.
   // The program will eventually crash with stack overflow if this protection is not in place
   ++recursion;
   if(recursion < 64) {

      int xdim = subframe.xdim();
      int ydim = subframe.ydim();

      // look one pixel in all directions from the given point
      for(int yy=y-1; yy<=y+1; yy++) {

         if(yy <     0)continue;
         if(yy >= ydim)continue;

         for(int xx=x-1; xx<=x+1; xx++) {

            if(xx <    0)continue;
            if(xx >= xdim)continue;

            // if we have seen this pixel before, skip it
            int key = subframe.pixel_id(xx,yy);
            if(done.find(key) != done.end()) continue;
            done.insert(key);

            // ok, unseen pixel, so add it to our star if it is above limit
            unsigned char* pixel = subframe(xx,yy);
            if(*pixel > m_limit) {
               star.add(wxStar(xx,yy,*pixel));

               // continue painting
               PaintStar(subframe,star,xx,yy,done,recursion);
            }
         }
      }
   }
   --recursion;

}

bool wxSubFrame::FindPeak(const image2d& subframe, DonePixels& done_pixels, int x, int y, unsigned long& peak_mag, int& xmax, int& ymax)
{
   int xdim = subframe.xdim();
   int ydim = subframe.ydim();

   // initial values
   peak_mag = *subframe(x,y);
   xmax = x;
   ymax = y;

   // mark this pixel as visited
   done_pixels.insert(subframe.pixel_id(x,y));

   // check neigbour pixels for larger values
   const int delta=1;
   for(int yy=y-delta; yy<=y+delta; yy++) {

      // don't look outside frame
      if(yy<0)continue;
      if(yy>=ydim)continue;

      for(int xx=x-delta; xx<=x+delta; xx++) {

         // don't look outside frame
         if(xx<0)continue;
         if(xx>=xdim)continue;

         if(xx!=x && yy!=y) {
            unsigned char* neighbour = subframe(xx,yy);
            if(*neighbour > peak_mag) {

               // check that this pixel has not been visited before
               if(done_pixels.find(subframe.pixel_id(x,y)) == done_pixels.end()) {
                  // new peak value found, recurse
                  FindPeak(subframe, done_pixels,xx,yy,peak_mag,xmax,ymax);
               }
            }
         }
      }
   }
   return true;
}

bool wxSubFrame::lock(wxStar& offset_star)
{
   if(!m_locked) {

      wxStar star;
      if(FindStar(star)) {

         updateFrameCentre(star);

         // the lock position is also the current position initially
         m_lockcen = m_curcen;

         // just return a zero offset star
         offset_star = wxStar(0,0,star.mag());

         m_locked = true;
      }
   }
   return m_locked;
}

void wxSubFrame::updateFrameCentre(wxStar& subframe_star)
{
   // values for half width and half height
   double hsdimx = m_subsiz.x/2;
   double hsdimy = m_subsiz.y/2;

   double sign = (m_inverted)? -1.0 : 1.0;

   // convert the star subframe coordinates to fullframe coordinates
   double test_x = m_curcen.x + subframe_star.x() - hsdimx;
   double test_y = m_curcen.y + sign*subframe_star.y() - sign*hsdimy;

   // will the subframe extend beyond the fullframe border if we center
   // the subframe on the found guide star?

   // adjust the subframe centre point so that the box
   // does not go beyond the fullframe
   if( (test_x-hsdimx) <            0) test_x = hsdimx;
   if( (test_x+hsdimx) >= m_fullsiz.x) test_x = m_fullsiz.x-hsdimx-1;

   if( (test_y-hsdimy) <            0) test_y = hsdimy;
   if( (test_y+hsdimy) >= m_fullsiz.y) test_y = m_fullsiz.y-hsdimy-1;

   // set the computed centre position
   m_curcen.x = test_x;
   m_curcen.y = test_y;
}


bool wxSubFrame::getOffset(wxRealPoint& offset)
{
   if(m_locked) {
      offset = m_curcen - m_lockcen;

      double sign = (m_inverted)? 1.0 : -1.0;

      // allow for camera rotation
      double cosang = m_imageScale*cos(m_angle);
      double sinang = m_imageScale*sin(m_angle);

      double ra  = cosang*offset.x - sign*sinang*offset.y;
      double dec = sinang*offset.x + sign*cosang*offset.y;
      offset.x = ra;
      offset.y = dec;

      return true;
   }
   return false;
}

// find the guidestar and return its position in pixels relative to the locked position.
// If the guidestar is unlocked, a lock will be established.
// The function returns false if the guidestar was not found in the subframe
bool wxSubFrame::guide(wxStar& offset_star)
{
   if(!m_locked) {
      m_locked = lock(offset_star);
   }
   else {
      wxStar framestar;
      if(FindStar(framestar)) {
         updateFrameCentre(framestar);

         // compute the RA, DEC offset
         wxRealPoint offset;
         getOffset(offset);
         offset_star = wxStar( offset.x,offset.y,framestar.mag());
      }
      else {

         // guide star lost, sound a bell
         wxBell();
         return false;
      }
   }

   return m_locked;
}

const wxRealPoint& wxSubFrame::centre() const
{
   return m_curcen;
}

const wxSize& wxSubFrame::size() const
{
   return m_subsiz;
}

const wxRealPoint& wxSubFrame::lockpos() const
{
   return m_lockcen;
}
