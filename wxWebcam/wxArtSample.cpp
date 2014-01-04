

#include "wxArtSample.h"
#include "GenericImageMem.h"


#include "../debtools.h"   // Debug tools


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implement a wxArtSample
//
wxArtSample::wxArtSample()
: m_xbin(1)
, m_ybin(1)
, m_fullFrame(false)
, m_droppedSample(false)
, m_pImageMem(NULL)
, m_pImageBMP(NULL)
{
}

wxArtSample::~wxArtSample()
{
   CleanUp();
}

// Y Data access
// NOTES: the bmiHeader has biBitCount = 16, no Palette, biCompression = WXACFOURCC_Y16B
// return the ptr to the sample data of the Y plane
const wxUint16* wxArtSample::SampleYPtr() const
{
   if (m_pImageMem) return m_pImageMem->WordMemPtr();
   else            return NULL;
}

size_t wxArtSample::SampleSizeY() const
{
   if (m_pImageMem) return m_pImageMem->WordSize();
   else            return 0;
}

size_t wxArtSample::SampleByteSizeY() const
{
   if (m_pImageMem) return m_pImageMem->ByteSize();
   else            return 0;
}

// converts the image to a plain 3*8bit mono but RGB sized bitmap with applied gamma
bool wxArtSample::ConvertToBMP(float p_gamma, long p_minIn, long p_maxIn)
{
   long min, max;
	double minIn, maxIn, minOut, maxOut, gammaOut;
   double b,m, vv;

   if ( !m_pImageMem ) return false;// ERROR EXIT no image

   min = 0;
   max = 65535;

   // get the in range and gamma arguments
   minIn = double( (p_minIn>=0)? p_minIn:min );
   maxIn = double( (p_maxIn>=0)? p_maxIn:max );

   maxOut = 254.5;   minOut  = 0.0; // defines the output pixel value range (full 8bit) (prevent fp rounding issue)
   gammaOut = (p_gamma<0.1) ? 0.1 : ( (p_gamma>9.99) ? 9.99 : p_gamma); // clip arg range

   // basic equation is Out = m * In + b
   //  where Out is in the range of USHORT (0..2^16-1)
   //  and In is an arbitrary range based on the Input datatype

   // solved Eq is:
   // m = (maxOut - minOut) / (maxIn-minIn)
   // b = minOut - m*minIn
   m = (maxOut - minOut) / (maxIn - minIn);
   b = minOut - m * minIn;

   // define some helpers here
   size_t sw = m_subRect.GetWidth();      // dst coords
   size_t sh = m_subRect.GetHeight();     // dst coords

   // prepare BMP datastore
   wxUint16 scanwidth = sw*3;
   //scanwidth = (scanwidth +3) & ~3; // DWORD align if needed (?? dont know for wxImage)
   if (m_pImageBMP) delete m_pImageBMP;
   m_pImageBMP = new ByteImageMem(sh*scanwidth); // allocate and RGB map (for wxImage - must be malloc!!)
   if (!m_pImageBMP->MemOK()) return false; // ERROR EXIT no memory

   const wxUint16* inPtr = m_pImageMem->WordMemPtr();
   wxUint8* outPtr;
   wxUint8  outVal;
   for (size_t y=0; y<sh; y++) {
      outPtr = m_pImageBMP->ByteMemPtrRef() + y*scanwidth;
      for (size_t x=0; x<sw; x++) {
         vv = (double)*inPtr++;
         // clip input
         vv = (vv<minIn) ? minIn : ((vv>maxIn) ? maxIn : vv);
         // scale with out = m * In + b; then apply gamma based on Out range
         //vv = m * vv + b; // plain linear conversion
         vv = maxOut *  pow( ((m * vv + b) / maxOut), 1.0/gammaOut);
         // clip calculated pixel at out range
         outVal = wxUint8( (vv<minOut) ? minOut : ((vv>maxOut) ? maxOut : vv) );
         *outPtr++ = outVal; *outPtr++ = outVal; *outPtr++ = outVal; // set into RGB
      }//cols
   }//rows
   // now we have an RBG map that suits the wxImage data ptr
   return true;
}

// detaches the RGB(BGR) pointer from this obj i.e. will not be freed on destroy
// this is used when the sample is Set into a wxImage
unsigned char* wxArtSample::DetachBMP()
{
   if (m_pImageBMP) {
      unsigned char* tx = reinterpret_cast<unsigned char*>(m_pImageBMP->DetatchMemPtr());
      delete m_pImageBMP; m_pImageBMP=NULL; // not longer used
      return tx;
   }

   // if we reach this point, there is nothing to detach
   return NULL;
}

//BM:20090620 - added
void wxArtSample::MarkTimeStamp()
{
   m_uTimeStamp = wxDateTime::UNow(); //BM:20090619 - init running timestamp
}

// init sample and transfer memory ownership
// pls note that wxArtSample now owns the imageMem !!!
void wxArtSample::Init(bool fullFrame, unsigned short binx, unsigned short biny,
                           WordImageMem* imageMem, float expoTime)
{
   CleanUp();

   m_fullFrame = fullFrame;
   m_xbin = binx; m_ybin = biny;

   m_exposureTime = expoTime;

   // set the image ptr
   m_pImageMem = imageMem;
   if (m_pImageMem) m_subRect = m_pImageMem->FrameRect();
   else m_subRect = wxRect(0,0,0,0);
}

void wxArtSample::CleanUp()
{
   m_subRect = wxRect(0,0,0,0);
   m_xbin = 1;  m_ybin = 1;
   m_fullFrame = false;
   m_droppedSample = false;

   if (m_pImageMem) delete m_pImageMem; m_pImageMem = NULL;
   if (m_pImageBMP) delete m_pImageBMP; m_pImageBMP = NULL; // just in case it was not detached
}

bool wxArtSample::histogram_pivot_dev(int& pivot, int& vlow, int& vhig,int& max)
{
   max = 65535;

   if (!m_pImageMem) return false;

   // pointer to 16bit image data
   const wxUint16* inpix = m_pImageMem->WordMemPtr();

   // number of pixels
   int npix = m_subRect.GetWidth()*m_subRect.GetHeight();
   int lval(max),hval(0);

   // compute pivot = average value
   double sumpix = 0;
   for(int ipix=0; ipix<npix; ipix++) {
      const wxUint16& pixval = *inpix++;
      sumpix += pixval;
      if(pixval>0) {
         lval = (pixval < lval)? pixval : lval;
         hval = (pixval > hval)? pixval : hval;
      }
   }
   pivot = int(sumpix/npix);

   // compute high and low averages
   inpix = m_pImageMem->WordMemPtr();
   double sumlow = 0; int nlow=0;
   double sumhig = 0; int nhig=0;
   for(int ipix=0; ipix<npix; ipix++) {
      const wxUint16& pixval = *inpix++;
      if(pixval>0) {
         if(pixval > pivot) {
            sumhig += pixval;
            nhig++;
         }
         else if(pixval < pivot) {
            sumlow += pixval;
            nlow++;
         }
      }
   }

   int plow = int( (nlow > 0)? sumlow/nlow : 0 );
   if(nhig == 0) {
      sumhig = hval;
      nhig  = 1;
   }
   int phig = int( 0.75*(pivot + sumhig/nhig) );

   int dlow = pivot - plow;
   double slow=3.0;
   vlow = int( pivot - slow*dlow );
   vhig = phig;

   vlow = (vhig <   0)? 0   : vlow;
   vhig = (vhig > max)? max : vhig;

   return true;
}
