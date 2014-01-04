

#include "wxArtCoords.h"

wxArtCoords::wxArtCoords()
: wxCamCoords()
{
}

wxArtCoords::~wxArtCoords()
{
}

// transformations
//

wxCamPoint wxArtCoords::GetPoint(EPointOrigin org)
{
   wxCamPoint newPt=m_point;
   switch (m_ptOrigin) {
      // transform FROM:
      case EPO_CCD: // CCD original pixels
         switch (org) {
            // transform TO:
            case EPO_CCD:
               ; // p'=p
            break;
            case EPO_Sample:
               // p'= B*p
               newPt.x = m_point.x/m_bfx;
               newPt.y = m_point.y/m_bfy;
            break;
            case EPO_Image:
               // p'= MB*p
               newPt.x = m_ccdw/m_bfx - m_point.x/m_bfx -1.0;
               newPt.y = m_ccdh/m_bfy - m_point.y/m_bfy -1.0;
            break;
            case EPO_DC:
               // p'= ZMB*p
               newPt.x = (m_ccdw/m_bfx - m_point.x/m_bfx -1.0) *m_zfx;
               newPt.y = (m_ccdh/m_bfy - m_point.y/m_bfy -1.0) *m_zfy;
            break;
         }//switch
         break;

      case EPO_Sample:
         switch (org) {
            // transform TO:
            case EPO_CCD:
               // p'= Bi*p
               newPt.x = m_point.x *m_bfx;
               newPt.y = m_point.y *m_bfy;
            break;
            case EPO_Sample:
               ; // p'=p
            break;
            case EPO_Image:
               // p'= M*p
               newPt.x = m_ccdw/m_bfx - m_point.x -1.0;
               newPt.y = m_ccdh/m_bfy - m_point.y -1.0;
            break;
            case EPO_DC:
               // p'= ZM*p
               newPt.x = (m_ccdw/m_bfx - m_point.x -1.0) *m_zfx;
               newPt.y = (m_ccdh/m_bfy - m_point.y -1.0) *m_zfy;
            break;
         }//switch
         break;

      case EPO_Image:
         switch (org) {
            // transform TO:
            case EPO_CCD:
               // p'= BiM*p
               newPt.x = m_ccdw - m_point.x *m_bfx -1.0;
               newPt.y = m_ccdh - m_point.y *m_bfy -1.0;
            break;
            case EPO_Sample:
               // p'= M*p
               newPt.x = m_ccdw/m_bfx - m_point.x -1.0;
               newPt.y = m_ccdh/m_bfy - m_point.y -1.0;
            break;
            case EPO_Image:
               ; // p'=p
            break;
            case EPO_DC:
               // p'= Z*p
               newPt.x = m_point.x *m_zfx;
               newPt.y = m_point.y *m_zfy;
            break;
         }//switch
         break;

      case EPO_DC:
         switch (org) {
            // transform TO:
            case EPO_CCD:
               // p'= BiMZi*p
               newPt.x = m_ccdw - m_point.x/m_zfx *m_bfx -1.0;
               newPt.y = m_ccdh - m_point.y/m_zfy *m_bfy -1.0;
            break;
            case EPO_Sample:
               // p'= MZi*p
               newPt.x = m_ccdw/m_bfx - m_point.x/m_zfx -1.0;
               newPt.y = m_ccdh/m_bfy - m_point.y/m_zfy -1.0;
            break;
            case EPO_Image:
               // p'= Zi*p
               newPt.x = m_point.x/m_zfx;
               newPt.y = m_point.y/m_zfy;
            break;
            case EPO_DC:
               ; // p'=p
            break;
         }//switch
         break;
   }//switch

   return newPt;
}

// returning the full size translated to org
wxSize wxArtCoords::GetFullSize(EPointOrigin org)
{
   if (org==EPO_CCD) {
      // no mirroing when asked for native CCD size - return values directly
      return wxSize(int(m_ccdw), int(m_ccdh));
   }
   else {
      // this is a bit cheating - we know here that CCD is mirrored so we
      // ask for the opposite point of 0/0 which is maxX/maxY i.e. the size-1
      wxCamPoint size(0, 0);
      SetPoint(size, EPO_CCD);
      size = GetPoint(org); size.IncBy(1.0);
      return wxSize(size.IntPoint().x, size.IntPoint().y);
   }
}
