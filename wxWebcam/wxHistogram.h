
#ifndef _WXHISTOGRAM_H__
#define _WXHISTOGRAM_H__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <vector>

class wxHistogram
{
public:
   wxHistogram();
   ~wxHistogram();

public:
   enum FrameChannel {Lchan=0, Rchan, Gchan, Bchan};

   int Size() const;
   bool OK() const;

   void SetChannel(FrameChannel channel);
   FrameChannel Channel() const;

   // current channel
   unsigned long  Value(wxUint16 pixel_value, bool scale=false);
   wxUint16 MinExposure() const;
   wxUint16 MaxExposure() const;

   // per channel
   unsigned long  Value(FrameChannel channel, wxUint16 pixel_value, bool scale=false);
   wxUint16 MinExposure(FrameChannel channel) const;
   wxUint16 MaxExposure(FrameChannel channel) const;

   void PivotDev(wxUint16& pivot, wxUint16& vlow, wxUint16& vhig, wxUint16& max);

   // assuming a mono image, size is w*h
   void ComputeHist(const wxUint16* imgPtr, size_t imgSize);
   // assuming a BGR image, size is w*h
   void ComputeHist(const wxUint8* imgPtr, size_t imgSize);

   // assuming a mono image, size is w*h
   void ComputePivotDev(const wxUint16* imgPtr, size_t imgSize);
   // assuming a BGR image, size is w*h
   void ComputePivotDev(const wxUint8* imgPtr, size_t imgSize);

   bool MonoImage() const ; // True when monochrome
   double SizeScaleFactor() const;

private:
   typedef std::vector<unsigned long> Histogram;
   Histogram      m_histo[4]; // length=128, value=count
   FrameChannel   m_channel; // channel used for histogram
   wxUint16       m_eMin[4];  // minimum pixel values, 4 channels according to FrameChannel
   wxUint16       m_eMax[4];  // maximum pixel values, 4 channels according to FrameChannel

   bool m_monoImage;
   bool m_OK;
   wxUint16       m_pivot;
   wxUint16       m_vLow;
   wxUint16       m_vHigh;
   wxUint16       m_vMax;

   double         m_scale;   // histogram frame size scaling factor
};

#endif // _WXHISTOGRAM_H__
