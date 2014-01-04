
#ifndef _WXCAMCOORDS_H__
#define _WXCAMCOORDS_H__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

//20080726:BM derived from wx base type now
class wxCamPoint : public wxRealPoint
{
public:
//   double X;
//   double Y;
   wxCamPoint() :wxRealPoint(){}
   wxCamPoint(const wxPoint& pt) :wxRealPoint(pt.x, pt.y){}
   wxCamPoint(double xx, double yy) :wxRealPoint(xx,yy){}
   wxCamPoint(wxUint16 xui, wxUint16 yui) :wxRealPoint(double(xui),double(yui)){}
   wxCamPoint(int xi, int yi) :wxRealPoint(double(xi),double(yi)){}
   void SetPoint(const wxPoint& pt) {*this=wxCamPoint(pt);}
   void SetPoint(wxUint16 xui, wxUint16 yui) {x=double(xui); y=double(yui);}
   wxPoint IntPoint() const {return wxPoint(int(x), int(y));}
   void IncBy(double i=1.0) {x+=i; y+=i;}
   void DecBy(double i=1.0) {x-=i; y-=i;}
};

class wxCamCoords
{
public:
	wxCamCoords();
	virtual ~wxCamCoords();

public:
   // set the various variables used to transform the coordinates
   void SetCCDSize(wxUint16 w, wxUint16 h);

   void SetBinFactor(wxUint16 bf);
   void SetBinFactor(wxUint16 bfx, wxUint16 bfy);

   void SetZoomFactor(double zf);
   void SetZoomFactor(double zfx, double zfy);

   enum EPointOrigin {
      EPO_CCD = 0,
      EPO_Sample,
      EPO_Image,
      EPO_DC,
   };
   void SetPoint(const wxCamPoint& pt, EPointOrigin org);
   void SetPoint(const wxPoint& pt, EPointOrigin org) {SetPoint(wxCamPoint(pt.x, pt.y) , org);}

   // transformations (may/shall be cam dependent)
   virtual wxCamPoint GetPoint(EPointOrigin org) =0;
   virtual wxPoint GetIntPoint(EPointOrigin org) {return GetPoint(org).IntPoint();}
   virtual wxSize GetFullSize(EPointOrigin org);

protected:
   wxCamPoint     m_point;
   EPointOrigin   m_ptOrigin;
   double         m_ccdw, m_ccdh;
   double         m_bfx,  m_bfy;
   double         m_zfx,  m_zfy;
};


#endif  // _WXCAMCOORDS_H__

