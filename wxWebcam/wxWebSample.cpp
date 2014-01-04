

#include "wxWebSample.h"
#include "GenericImageMem.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implement a wxWebSample
//
wxWebSample::wxWebSample()
: m_fullFrame(false)
, m_droppedSample(false)
, m_sampleSizeBGR(0)
, m_pImageBGR(NULL)
, m_sampleSizeYUV(0)
, m_pImageYUV(NULL)
{
   CleanUp();
}

wxWebSample::~wxWebSample()
{
   CleanUp();
}


// swaps the R and B pixel values in the BGR map
void wxWebSample::ConvertBGR2RGB()
{
   if ( !m_pImageBGR ) return;

   // convert BGR map to RGB map for display
   wxUint8 uc;
   wxUint8* rgbPtr=m_pImageBGR->ByteMemPtrRef();
   for (int i=0; i<m_subRect.GetWidth()*m_subRect.GetHeight(); i++) {
      uc = rgbPtr[0]; rgbPtr[0]=rgbPtr[2]; rgbPtr[2]=uc; // swap R and B
      ++rgbPtr;++rgbPtr;++rgbPtr; // next Pixel
   }
 }

// detaches the RGB(BGR) pointer from this obj i.e. will not be freed on destroy
// this is used when the sample is Set into a wxImage
unsigned char* wxWebSample::DetachBMP()
{
   if (m_pImageBGR) {
      unsigned char* tx = reinterpret_cast<unsigned char*>(m_pImageBGR->DetatchMemPtr());
      delete m_pImageBGR; m_pImageBGR=NULL; // not longer used
      return tx;
   }
   else {
      return NULL;
   }
}

// return the ptr to header of the bitmap (ptr to BITMAPINFOHEADER)
const PBITMAPINFOHEADER wxWebSample::SampleHeaderBGRPtr() const
{
   return const_cast<const PBITMAPINFOHEADER>(&m_pHeaderBGR);
}

// return the ptr to header of the bitmap (ptr to BITMAPINFOHEADER)
const PBITMAPINFOHEADER wxWebSample::SampleHeaderYUVPtr() const
{
   return const_cast<const PBITMAPINFOHEADER>(&m_pHeaderYUV);
}


// RGB24 Data access
// NOTES: the bmiHeader has biBitCount = 24, no Palette,
// return the ptr to the sample data of the bitmap
const wxUint8* wxWebSample::SampleBGRPtr() const
{
   if (m_pImageBGR) return m_pImageBGR->ByteMemPtr();
   else            return NULL;
}

size_t wxWebSample::SampleSizeBgr() const
{
   if (m_pImageBGR) return m_pImageBGR->ByteSize();
   else            return 0;
}


// YUV Data access
// NOTES: the bmiHeader has biBitCount = 12, no Palette, biCompression = WMCFOURCC_I420 (I420)
// return the ptr to the sample data of the Y plane
const wxUint8* wxWebSample::SampleYUVPtr() const
{
   if (m_pImageYUV) return m_pImageYUV->ByteMemPtr();
   else            return NULL;
}

size_t wxWebSample::SampleSizeYuv() const
{
   if (m_pImageYUV) return m_pImageYUV->ByteSize();
   else            return 0;
}

//BM:20090620 - added
void wxWebSample::MarkTimeStamp()
{
   m_uTimeStamp = wxDateTime::UNow(); //BM:20090619 - init running timestamp
}


// init sample and transfer memory ownership
// pls note that wxWebSample now owns the imageMem !!!
void wxWebSample::Init(bool fullFrame, EVideoConversion sampleFormat, ByteImageMem* pImageYuv, ByteImageMem* pImageBgr)
{
   CleanUp();

   m_fullFrame = fullFrame;

   // set the image ptr
   m_pImageBGR = pImageBgr;
   m_pImageYUV = pImageYuv;
   if (pImageBgr) m_subRect = pImageBgr->FrameRect();
   else m_subRect = wxRect(0,0,0,0);

	size_t iSize(0), sw(0), sh(0);

   // clear bmih
   ::memset(&m_pHeaderYUV, 0, sizeof(BITMAPINFOHEADER));
   m_pHeaderYUV.biSize = sizeof(BITMAPINFOHEADER);
   m_pHeaderYUV.biWidth = (long)sw;
   m_pHeaderYUV.biHeight = (long)sh;

   //20080518:BM native can be RGB in this case we don't have a YUV at hand
   if (pImageYuv) {
      // get target size BMP
      sw = pImageYuv->FrameRect().width;
      sh = pImageYuv->FrameRect().height;
      size_t iYSize = 0, iUVSize = 0;
      switch (sampleFormat) {
         case EVC_I420_RGB24:
         {
            iYSize = sw * sh;
            iUVSize =  (sw>>1) * (sh>>1);
            // create a bitmapinfoheader for the YUV sample
            // calculate the image size
            iSize =  iYSize + 2*iUVSize;
            m_pHeaderYUV.biSizeImage = iSize;
            m_pHeaderYUV.biBitCount = 12;   m_pHeaderYUV.biCompression = wxAC_FOURCC_I420;
            break;
         }

         case EVC_YUY2_RGB24:
          {
            // create a bitmapinfoheader for the YUV 4:2:2 sample
            // calculate the image size
            iSize =  sw * sh * 2;
            m_pHeaderYUV.biSizeImage = iSize;
            m_pHeaderYUV.biBitCount = 16;   m_pHeaderYUV.biCompression = wxAC_FOURCC_YUY2;
            break;
         }
         case EVC_YVYU_RGB24:
          {
            // create a bitmapinfoheader for the YUV 4:2:2 sample
            // calculate the image size
            iSize =  sw * sh * 2;
            m_pHeaderYUV.biSizeImage = iSize;
            m_pHeaderYUV.biBitCount = 16;   m_pHeaderYUV.biCompression = wxAC_FOURCC_YVYU;
            break;
         }
         case EVC_UYVY_RGB24:
          {
            // create a bitmapinfoheader for the YUV 4:2:2 sample
            // calculate the image size
            iSize =  sw * sh * 2;
            m_pHeaderYUV.biSizeImage = iSize;
            m_pHeaderYUV.biBitCount = 16;   m_pHeaderYUV.biCompression = wxAC_FOURCC_UYVY;
            break;
         }

         case EVC_Y800_RGB24:
          {
            // create a bitmapinfoheader for the Y800 sample
            // calculate the image size
            iSize =  sw * sh ;
            m_pHeaderYUV.biSizeImage = iSize;
            m_pHeaderYUV.biBitCount = 8;   m_pHeaderYUV.biCompression = wxAC_FOURCC_Y800;
            break;
         }

         default:
          {
            // create a bitmapinfoheader for the Y800 sample
            // calculate the image size
            iSize =  1;
            m_pHeaderYUV.biSizeImage = iSize;
            m_pHeaderYUV.biBitCount = 8;   m_pHeaderYUV.biCompression = wxAC_FOURCC_Y800;
            break;
         }
      }

      // now set the bmih struct
      m_pHeaderYUV.biWidth = (long)sw;
      m_pHeaderYUV.biHeight = (long)sh;
      m_pHeaderYUV.biPlanes = 1;
   }
   m_sampleSizeYUV = iSize;


   // create a bitmapinfoheader for the RGB24 sample
      // clear bmih
   ::memset(&m_pHeaderBGR, 0, sizeof(BITMAPINFOHEADER));
   m_pHeaderBGR.biSize = sizeof(BITMAPINFOHEADER);
   m_pHeaderBGR.biWidth = (long)sw;
   m_pHeaderBGR.biHeight = (long)sh;
   iSize = 0;

   if (m_pImageBGR) {

      // calculate the image size
      size_t sw = m_pImageBGR->FrameRect().width;
      size_t sh = m_pImageBGR->FrameRect().height;
      size_t scan_sw = (sw*3+3) & ~3;        // dst coords scan width in bytes WORD aligned
      iSize =  scan_sw*sh; // BMP has 3 bytes per pixel - i.e. use scanwidth

      // now set the bmih struct
      m_pHeaderBGR.biWidth = (long)sw;
      m_pHeaderBGR.biHeight = (long)sh;
      m_pHeaderBGR.biPlanes = 1;
      m_pHeaderBGR.biBitCount = 24;
      m_pHeaderBGR.biSizeImage = iSize;

   }
   m_sampleSizeBGR = iSize;
}//Init


void wxWebSample::CleanUp()
{
   m_subRect = wxRect(0,0,0,0);
   m_fullFrame = false;
   m_droppedSample = false;

   ::memset(&m_pHeaderYUV, 0, sizeof(BITMAPINFOHEADER));
   if (m_pImageYUV) delete m_pImageYUV; m_pImageYUV = NULL;
   m_sampleSizeYUV = 0;

   ::memset(&m_pHeaderBGR, 0, sizeof(BITMAPINFOHEADER));
   if (m_pImageBGR) delete m_pImageBGR; m_pImageBGR = NULL;
   m_sampleSizeBGR = 0;
}


