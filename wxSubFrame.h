#ifndef WXSUBFRAME_H
#define WXSUBFRAME_H

#include <wx/image.h>
#include "wxStar.h"
class image2d;

#include <set>

/*
    wxSubFrame represents the subframe of the guiding box.
    It contains autoguiding algorithms.

    It has info on full frame and subframe sizes, plus subframe centre.
    It owns a copy of the subframe, assigned using copySubFrame on each guiding interval.
    The guidestar offset and magnitude is computed using the guide function.

    It is the responsibility of the caller to call copySubFrame and guide on each guiding interval.
    The wxStar returned from guide can be used to generate telescope guiding commands.
*/

class wxSubFrame {
public:
   wxSubFrame();

   void setFrameInfo(const wxSize& fullsiz       // full frame size
                    ,const wxSize& subsiz        // subframe size
                    ,const wxRealPoint& subcen); // subframe centre point in full frame

   virtual ~wxSubFrame();

   // extract guiding subframe from fullframe
   bool copySubFrame(const wxImage& fullframe,   // reference to live full frame
                     bool inverted);             // true when full frame is inverted, i.e. upside down.

   unsigned long timestamp_ms();                // time stamp of last copied subframe in milliseconds

   // find the guidestar and return its position in arcseconds relative to the locked position.
   // Camera angle and image scale will be taken into account.
   // If the guidestar is unlocked, a lock will be established.
   // The function returns false if the guidestar was not found in the subframe
   bool guide(wxStar& offset_star);

   // return up-to-date centre point of subframe in fullframe
   const wxRealPoint& centre() const;
   const wxSize&      size() const;

   // return lock position (in full frame)
   const wxRealPoint& lockpos() const;

   // Guide camera angle relative to RA, DEC
   // The angle is measured in degrees and is positive for a counterclockwise camera rotation
   static double cameraAngle();
   static void setCameraAngle(double degrees);

   // Guide camera imagescale [arcseconds/pixel]
   static double imageScale();
   static void setImageScale(double imageScale);

   // Background cutoff limit
   // Only pixels with values greater than this value will be considered
   static unsigned char cutoffLimit();
   static void setCutoffLimit(unsigned char limit);

private:
   typedef std::set<int> DonePixels;

   // find peak pixel in subframe, starting at x, y. Return coords relative to subframe lower left corner
   bool FindPeak(const image2d& subframe, DonePixels& done_pixels, int x, int y, unsigned long& peak_mag, int& xmax, int& ymax);

   // Paint the area around x, y. Return accumulated star, relative to subframe lower left
   void PaintStar(const image2d& subframe, wxStar& star, int x, int y, DonePixels& done, int& recursion);

   // If successful, FindStar returns a star with subframe (x,y) coordinates
   bool FindStar(wxStar& subframe_star);

   // return the current guide star offset, relative to locked position (not subframe coords)
   bool getOffset(wxRealPoint& offset);

   // lock onto and center subframe on guide star, returns true on success
   bool lock(wxStar& offset_star);

   // updateFrameCentre takes a star with subframe (x,y) coordinates,
   // it updates the m_curcen member variable to match
   void updateFrameCentre(wxStar& subframe_star);

   // computes a millisecond based time stamp for this time instant
   unsigned long computeTimeStampMillis();

private:
   wxSize         m_fullsiz;      // full frame image dimensions in pixels
   wxSize         m_subsiz;       // requested subframe dimensions in pixels

   wxImage        m_subframe;     // current subframe pixel data
   wxRealPoint    m_curcen;       // current subframe centre position (in full frame)
   unsigned long  m_timestamp_ms; // time stamp in ms of last copied subframe //BM-20090411 - fixup the clock issue // CA-20090412- use ms

   wxRealPoint    m_lockcen;      // original locked subframe centre position (in full frame)
   bool           m_locked;       // true when locked on a guidestar

   bool           m_inverted;     // true when last fullframe was inverted

   static unsigned char  m_limit; // lower cutoff limit
   static double  m_angle;        // camera angle in radians (counterclockwise)
   static double  m_imageScale;   // ["/pixel]

};

#endif // WXSUBFRAME_H
