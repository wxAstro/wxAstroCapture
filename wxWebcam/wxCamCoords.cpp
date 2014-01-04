

#include "wxCamCoords.h"

wxCamCoords::wxCamCoords()
: m_ptOrigin(EPO_CCD)
, m_ccdw(640.0)
, m_ccdh(480.0)
, m_bfx(1.0)
, m_bfy(1.0)
, m_zfx(1.0)
, m_zfy(1.0)
{
}

wxCamCoords::~wxCamCoords()
{
}

// transformations
//


// returning the full size (default implementation)
wxSize wxCamCoords::GetFullSize(EPointOrigin org)
{
   if (org==EPO_CCD) {
      // no mirroing when asked for native CCD size - return values directly
      return wxSize(m_ccdw, m_ccdh);
   }
   else {
      wxCamPoint size(m_ccdw, m_ccdh);
      SetPoint(size, EPO_CCD);
      size  = GetPoint(org); size.IncBy(1.0);
      return wxSize(size.IntPoint().x, size.IntPoint().y);
   }
}

void wxCamCoords::SetPoint(const wxCamPoint& pt, EPointOrigin org)
{
   m_point=pt;
   m_ptOrigin=org;
}

void wxCamCoords::SetCCDSize(unsigned short w, unsigned short h)
{
   m_ccdw=double(w);
   m_ccdh=double(h);
}

void wxCamCoords::SetBinFactor(unsigned short  bf)
{
   m_bfx=double(bf);
   m_bfy=double(bf);
}
void wxCamCoords::SetBinFactor(unsigned short  bfx, unsigned short  bfy)
{
   m_bfx=double(bfx);
   m_bfy=double(bfy);
}

void wxCamCoords::SetZoomFactor(double zf)
{
   m_zfx=zf;
   m_zfy=zf;
}
void wxCamCoords::SetZoomFactor(double zfx, double zfy)
{
   m_zfx=zfx;
   m_zfy=zfy;
}

