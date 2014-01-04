#include "wxGuider.h"
#include "wxGuidingConfig.h"

wxGuider::wxGuider()
: m_interval_ms(1000) // default
, m_ra_dz(1.0)
, m_ra_k(20.0)
, m_ra_q(40.0)
, m_dec_dz(1.0)
, m_dec_k(30.0)
, m_dec_q(40.0)
, m_guiding_active(false)
{
   wxGuidingConfig::restoreGuiderTuningParams(this);
}

wxGuider::~wxGuider()
{
   wxGuidingConfig::saveGuiderTuningParams(this);
}

void wxGuider::setTuningDefaults()
{
   // "factory" defaults
   m_ra_dz  = 1.0;
   m_ra_k   = 20.0;
   m_ra_q   = 40.0;
   m_dec_dz = 1.0;
   m_dec_k  = 30.0;
   m_dec_q  = 40.0;
}


   // set current guide interval
bool wxGuider::setGuideInterval(size_t interval_ms)
{
   m_interval_ms = interval_ms;
   return true;
}

   // return current guide interval
size_t wxGuider::guideInterval()
{
   return m_interval_ms;
}

void wxGuider::setRaDz(double ra_dz)
{
   m_ra_dz = ra_dz;
}

void wxGuider::setRaK(double ra_k)
{
   m_ra_k = ra_k;
}

void wxGuider::setRaQ(double ra_q)
{
   m_ra_q = ra_q;
}


double wxGuider::raDz()
{
   return m_ra_dz;
}

double wxGuider::raK()
{
   return m_ra_k;
}

double wxGuider::raQ()
{
   return m_ra_q;
}


void wxGuider::setDecDz(double dec_dz)
{
   m_dec_dz = dec_dz;
}

void wxGuider::setDecK(double dec_k)
{
   m_dec_k = dec_k;
}

void wxGuider::setDecQ(double dec_q)
{
   m_dec_q = dec_q;
}


double wxGuider::decDz()
{
   return m_dec_dz;
}

double wxGuider::decK()
{
   return m_dec_k;
}

double wxGuider::decQ()
{
   return m_dec_q;
}

void wxGuider::set_guiding_active(bool active)
{
   m_guiding_active = active;
}

bool wxGuider::guiding_active()
{
   return m_guiding_active;
}
