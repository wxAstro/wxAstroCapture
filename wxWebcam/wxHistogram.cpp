
#include "wxHistogram.h"

wxHistogram::wxHistogram()
: m_channel(Lchan)
, m_monoImage(true)
, m_OK(false)
, m_pivot(0)
, m_vLow(0)
, m_vHigh(0)
, m_vMax(0)
, m_scale(1.0)
{
   m_histo[Lchan].resize(128); // fixed size
   m_histo[Bchan].resize(128); // fixed size
   m_histo[Gchan].resize(128); // fixed size
   m_histo[Rchan].resize(128); // fixed size

}

wxHistogram::~wxHistogram()
{
}

bool wxHistogram::OK() const
{
   return m_OK;
}

int wxHistogram::Size() const
{
   return m_histo[Lchan].size();
}

void wxHistogram::SetChannel(FrameChannel channel)
{
   m_channel = channel;
}

wxHistogram::FrameChannel wxHistogram::Channel() const
{
   return m_channel;
}

unsigned long  wxHistogram::Value(wxUint16 pixel_value,  bool scale)
{
   return Value(m_channel, pixel_value,scale);
}

wxUint16 wxHistogram::MinExposure() const
{
   return MinExposure(m_channel);
}

wxUint16 wxHistogram::MaxExposure() const
{
   return MaxExposure(m_channel);
}


unsigned long  wxHistogram::Value(FrameChannel channel, wxUint16 pixel_value,  bool scale)
{
   if (m_monoImage) {
      // mono has only Lchan
      if( pixel_value > m_histo[Lchan].size()-1 ) return 0;
      if(scale) return (unsigned long)((m_histo[Lchan])[pixel_value]*m_scale);
      else return (m_histo[Lchan])[pixel_value];
   }
   else {
      if( pixel_value > m_histo[channel].size()-1 ) return 0;
      if(scale) return (unsigned long)((m_histo[channel])[pixel_value]*m_scale);
      else return (m_histo[channel])[pixel_value];
   }
}

wxUint16 wxHistogram::MinExposure(FrameChannel channel) const
{
   if (m_monoImage) {
      // mono has only Lchan
      return m_eMin[Lchan];
   }
   else {
      return m_eMin[channel];
   }
}

wxUint16 wxHistogram::MaxExposure(FrameChannel channel) const
{
   if (m_monoImage) {
      // mono has only Lchan
      return m_eMax[Lchan];
   }
   else {
      return m_eMax[channel];
   }
}


// assuming a mono image, size is w*h
// The histogram has 128 buckets
void wxHistogram::ComputeHist(const wxUint16* imgPtr, size_t imgSize)
{
   m_OK = false;
   m_monoImage = true;

   if (imgPtr==NULL) return;

   m_eMin[Lchan] = 256*256-1;
   m_eMax[Lchan] = 0;

   // histogram scale factor to ensure consistent range for different size frames
   m_scale = 120*160/double(imgSize);

   // pointer to 16bit image data
   const wxUint16* inpix = imgPtr;

   // reset histogram
   for(size_t i=0;i<m_histo[Lchan].size(); i++) {
       (m_histo[Lchan])[i] = 0;
   }

   // establish pointers
   for(size_t i=0; i<imgSize; i++) {
      const wxUint16 yVal = *inpix++;
      // also get min/max here
      m_eMin[Lchan] = (yVal < m_eMin[Lchan])? yVal : m_eMin[Lchan];
      m_eMax[Lchan] = (yVal > m_eMax[Lchan])? yVal : m_eMax[Lchan];
      // increment the histogram
      (m_histo[Lchan])[yVal/(2*256)]++;
   }//for

   m_OK = true;
}

// assuming a BGR image, size is w*h
// The histogram has 128 buckets
void wxHistogram::ComputeHist(const wxUint8* imgPtr, size_t imgSize)
{
   m_OK = false;
   m_monoImage = false;

   if (imgPtr==NULL) return;

   m_eMin[Lchan] = 256-1; m_eMin[Bchan] = 256-1; m_eMin[Gchan] = 256-1; m_eMin[Rchan] = 256-1;
   m_eMax[Lchan] = 0; m_eMax[Bchan] = 0; m_eMax[Gchan] = 0; m_eMax[Rchan] = 0;

   // histogram scale factor to ensure consistent range for different size frames
   m_scale = 120*160/double(imgSize);

   // pointer to 16bit image data
   const wxUint8* inpix = imgPtr;

   // reset histogram
   for(size_t i=0;i<m_histo[Lchan].size(); i++) {
       (m_histo[Lchan])[i] = 0; (m_histo[Bchan])[i] = 0; (m_histo[Gchan])[i] = 0; (m_histo[Rchan])[i] = 0;
   }

   // establish pointers
   for(size_t i=0; i<imgSize; i++) {
      const wxUint16 bVal = *inpix++;
      const wxUint16 gVal = *inpix++;
      const wxUint16 rVal = *inpix++;

      wxUint16 y((wxUint16)(0.299*rVal + 0.587*gVal + 0.114*bVal));
      const wxUint16 yVal = (y>255) ? 255 : y; // clip this one
      // also get min/max here
      m_eMin[Bchan] = (bVal < m_eMin[Bchan])? bVal : m_eMin[Bchan];
      m_eMax[Bchan] = (bVal > m_eMax[Bchan])? bVal : m_eMax[Bchan];
      m_eMin[Gchan] = (gVal < m_eMin[Gchan])? gVal : m_eMin[Gchan];
      m_eMax[Gchan] = (gVal > m_eMax[Gchan])? gVal : m_eMax[Gchan];
      m_eMin[Rchan] = (rVal < m_eMin[Rchan])? rVal : m_eMin[Rchan];
      m_eMax[Rchan] = (rVal > m_eMax[Rchan])? rVal : m_eMax[Rchan];
      m_eMin[Lchan] = (yVal < m_eMin[Lchan])? yVal : m_eMin[Lchan];
      m_eMax[Lchan] = (yVal > m_eMax[Lchan])? yVal : m_eMax[Lchan];
      // increment the histogram
      (m_histo[Bchan])[bVal/2]++;
      (m_histo[Gchan])[gVal/2]++;
      (m_histo[Rchan])[rVal/2]++;
      (m_histo[Lchan])[yVal/2]++;
   }//for
   m_OK = true;
}

void wxHistogram::PivotDev(wxUint16& pivot, wxUint16& vlow, wxUint16& vhig, wxUint16& max)
{
   pivot = m_pivot;
   vlow = m_vLow;
   vhig = m_vHigh;
   max = m_vMax;
}


void wxHistogram::ComputePivotDev(const wxUint16* imgPtr, size_t imgSize)
{
   // pivot = average pixel value
   // vlow = estimated low pixel value to give decent looking high contrast image
   // vhig = estimated high pixel value to give decent looking high contrast image
   // max  = indication of range for pivot, vlow, vhig, i.e. [0,max]

   m_vMax = 256*256-1;

   // pointer to 16bit image data
   const wxUint16* inpix = imgPtr;

   // number of pixels
   size_t npix = imgSize;
   int lval(m_vMax),hval(0);

   // compute pivot = average value
   double sumpix = 0;
   for(size_t ipix=0; ipix<npix; ipix++) {
      const wxUint16& pixval = *inpix++;
      sumpix += pixval;
      if(pixval>0) {
         lval = (pixval < lval)? pixval : lval;
         hval = (pixval > hval)? pixval : hval;
      }
   }
   m_pivot = (wxUint16)(sumpix/npix);

   // compute high and low averages
   inpix = imgPtr;
   double sumlow = 0; size_t nlow=0;
   double sumhig = 0; size_t nhig=0;
   for(size_t ipix=0; ipix<npix; ipix++) {
      const wxUint16& pixval = *inpix++;
      if(pixval>0) {
         if(pixval > m_pivot) {
            sumhig += pixval;
            nhig++;
         }
         else if(pixval < m_pivot) {
            sumlow += pixval;
            nlow++;
         }
      }
   }

   size_t plow = size_t((nlow > 0)? sumlow/nlow : 0);
   if(nhig == 0) {
      sumhig = hval;
      nhig  = 1;
   }
   size_t phig = size_t(0.75*(m_pivot + sumhig/nhig));

   double slow=3.0;
   int vLow  = int(m_pivot - slow*(m_pivot - plow));
   int vHigh = phig;

   m_vLow  = (vLow <   0)? 0   : vLow;
   m_vHigh = (vHigh > m_vMax)? m_vMax : vHigh;
}


void wxHistogram::ComputePivotDev(const wxUint8* imgPtr, size_t imgSize)
{
   // pivot = average pixel value
   // vlow = estimated low pixel value to give decent looking high contrast image
   // vhig = estimated high pixel value to give decent looking high contrast image
   // max  = indication of range for pivot, vlow, vhig, i.e. [0,max]

   m_vMax = 256-1;

   // pointer to 16bit image data
   const wxUint8* inpix = imgPtr;

   // number of pixels
   size_t npix = imgSize * 3; // BGR image
   int lval(m_vMax),hval(0);

   // compute pivot = average value
   double sumpix = 0;
   for(size_t ipix=0; ipix<npix; ipix++) {
      const wxUint8& pixval = *inpix++;
      sumpix += pixval;
      if(pixval>0) {
         lval = (pixval < lval)? pixval : lval;
         hval = (pixval > hval)? pixval : hval;
      }
   }
   m_pivot = (wxUint16)(sumpix/npix);

   // compute high and low averages
   inpix = imgPtr;
   double sumlow = 0; size_t nlow=0;
   double sumhig = 0; size_t nhig=0;
   for(size_t ipix=0; ipix<npix; ipix++) {
      const wxUint8& pixval = *inpix++;
      if(pixval>0) {
         if(pixval > m_pivot) {
            sumhig += pixval;
            nhig++;
         }
         else if(pixval < m_pivot) {
            sumlow += pixval;
            nlow++;
         }
      }
   }

   size_t plow = size_t((nlow > 0)? sumlow/nlow : 0);
   if(nhig == 0) {
      sumhig = hval;
      nhig  = 1;
   }
   int phig = int(0.75*(m_pivot + sumhig/nhig));

   double slow=3.0;
   int vLow  = int(m_pivot - slow*(m_pivot - plow));
   int vHigh = phig;

   m_vLow  = (vLow <   0)? 0   : vLow;
   m_vHigh = (vHigh > m_vMax)? m_vMax : vHigh;
}

bool wxHistogram::MonoImage() const
{
   return m_monoImage;
}

double wxHistogram::SizeScaleFactor() const
{
   return m_scale;
}
