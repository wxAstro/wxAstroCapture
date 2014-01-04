#ifndef WXSTAR_H
#define WXSTAR_H

/*
    wxStar is used to hold information of placement and
    "magnitude" of a star during autoguiding.
*/
#include <wx/gdicmn.h>

class wxStar{
public:
   wxStar();
   wxStar(double x, double y, double mag);
   virtual ~wxStar();

   wxStar& operator=(const wxStar& other);

   // this star consumes the added star
   bool add(const wxStar& other);

   // return the position and magnitude of the star
   double x() const;
   double y() const;
   double mag() const;

   // return star as convenient point
   wxRealPoint point();

   // return rounded off integer positions
   int xint() const;
   int yint() const;
private:
   double m_x;
   double m_y;
   double m_mag;
};



#endif // WXSTAR_H
