#include "wxStar.h"
#include <cmath>

wxStar::wxStar()
: m_x(0)
, m_y(0)
, m_mag(0.0)
{}

wxStar::wxStar(double x, double y, double mag)
: m_x(x)
, m_y(y)
, m_mag(mag)
{}

wxStar::~wxStar()
{}

wxStar& wxStar::operator=(const wxStar& other)
{
   m_x = other.m_x;
   m_y = other.m_y;
   m_mag = other.m_mag;
   return *this;
}

int wxStar::xint() const { return int(m_x+0.5);}
int wxStar::yint() const { return int(m_y+0.5);}
double wxStar::x() const { return m_x;}
double wxStar::y() const { return m_y;}
double wxStar::mag() const { return m_mag;}

bool wxStar::add(const wxStar& other)
{
   // calculate the combined centre of gravity for the two stars
   double Mx = m_x*m_mag  +  other.m_x*other.m_mag;
   double My = m_y*m_mag  +  other.m_y*other.m_mag;
   double F  = m_mag + other.m_mag;

   if(fabs(F) > 0.0) {
      m_x = Mx/F;
      m_y = My/F;
      m_mag = F;
      return true;
   }
   return false;
}

wxRealPoint wxStar::point()
{
   return wxRealPoint(m_x,m_y);
}
