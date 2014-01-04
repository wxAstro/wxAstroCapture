

#ifndef _WXFITS_H__
#define _WXFITS_H__


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wxFitsKeyword.h"

// hard limits for the internal image buffer
#define FITS_MAX_WIDTH  4096
#define FITS_MAX_HEIGHT 4096


/// DECLARE wxFitsOut
// holds a FITS image of a give type in mem
// type is currently USHORT
// note this implements exactly ONE HDU where data is an image
// either as single 2d image bw or 3 consecutive 2d image planes for RGB types

// Intended use:
// Derive a new class from wxFitsOut with an appropriate image interface
// add additionally needed keywords or provide interfaces for that purpose
class fitsHandleCls;
class wxFitsOut
{
public:
   wxFitsOut();
   virtual ~wxFitsOut();

public:
   // the native datatype used to write FITS image data
   // Note: FITS requires signed integers to be written
   //       Unsigned input types will be scaled to get the full range (BZERO, BSCALE)
   typedef wxInt16 FitsPelType;

   // set the date/ time of the obyervation which is added for each image as FITS keyword
   void SetDateObs(wxDateTime dateObs); //BM:20090620 - added for better timestamp
   // set the exposure time which is added for each image as FITS keyword
   void SetExposureSeconds(float expoSeconds);
   // set the CCD temperature which is added for each image as FITS keyword
   void SetCCDTemperature(float temperature);
   // set the FrameNo which is added for each image as FITS keyword
   void SetFrameNo(size_t frameNo);

   // the local Fits Keywordlist remains persistent for multiple image dumps
   // return the local KeywordList for manipulation
   wxFitsKeywordList& KeywordListRef() const;
   // return the local KeywordList for read only
   const wxFitsKeywordList& KeywordList() const;

   // Output processing

   // put all together and make it available
   // it does add the required FITS entries and adds the KeywordList
   // after them - finally the END keyword is added automatically
   // MUST be called before retrieving the contents
   bool Finalize();

   // mem access to get the Fitsdata out
   size_t GetFitsSize() const;
   const void* GetFitsPointer() const;

protected:
   // only to be used by derived classes for low level image handling

   // creates a fits file with given dimensions
   // and adds appropriate header keywords
   bool Create(const wxSize& frameSize, size_t nAxis=2);

   // stream like adding either ...

   // uchar variants
   // a complete image i.e. w*h pels (w*h values)
   bool AddImage(const wxUint8* pImage);
   // an image stride i.e. width pels (w values)
   bool AddImageStride(const wxUint8* pImageStride);
   // an image Pel i.e. a pel (one value)
   bool AddImagePel(const wxUint8* pImagePel);

   // ushort variants
   // a complete image i.e. w*h pels (w*h values)
   bool AddImage(const wxUint16* pImage);
   // an image stride i.e. width pels (w values)
   bool AddImageStride(const wxUint16* pImageStride);
   // an image Pel i.e. a pel (one value)
   bool AddImagePel(const wxUint16* pImagePel);

private:
   fitsHandleCls* m_hFits;
};

#endif  // _WXFITS_H__
