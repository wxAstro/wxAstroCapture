
#include "wxFitsOut.h"
#include "../../version.h"


/// ////////////////////////////////////////////////////////////////////
/// IMPLEMENT fitsHeaderCls
// implements a FITS header record
// which consist of a number of header 'cards'
// a header card is defined as 36 header records with 80 bytes each (i.e. 2880 bytes)
// retrieval will return empty records to complete a card with 36 records each
class fitsHeaderCls
{
public:
   virtual ~fitsHeaderCls();
   void CreateBasicHeader(const wxSize& frameSize, size_t nAxis);

   wxFitsKeywordList& KeywordListRef() {return m_hRecords;};
   const wxFitsKeywordList& KeywordList() const {return m_hRecords;};

   size_t GetFitsHeaderByteSize() const;
   const char* HRecString(size_t hrPos);

private:
   //wxFitsKeyword::ArrayOfFitsKeywords  m_hRecords;
   wxFitsKeywordList  m_hRecords;
   char m_hrBuffer[81];  // inplace return of a hrec string
};

// Implementation of fitsHeaderCls

fitsHeaderCls::~fitsHeaderCls()
{
   m_hRecords.clear();
}

// returns the space needed to write the collected keywords
// i.e. the number of needed cards * 2880
size_t fitsHeaderCls::GetFitsHeaderByteSize() const
{
   int cardsUsed = (int)(m_hRecords.size() / 36) + 1;
   return cardsUsed * 36*80;
}

// returns the collected records as char strings, then empty ones until a card is complete
// it returns 8bit char strings ready to dump to the FITS file
// if the card is complete it returns NULL
const char* fitsHeaderCls::HRecString(size_t hrPos)
{
   if (hrPos<m_hRecords.size()) {
      strncpy(m_hrBuffer, m_hRecords[hrPos].KeywordString().fn_str(), 80);
      m_hrBuffer[80] = 0; // make sure we have an ASCIIZ string
      return m_hrBuffer;
   }
   else if (hrPos< ((size_t)(m_hRecords.size() / 36) + 1)*36) {
      // beyond the keywords but still within a card
      memset(m_hrBuffer, 0x20, 80); // return Blanks (according to standard)
      m_hrBuffer[80] = 0; // make sure we have an ASCIIZ string
      return m_hrBuffer;
   }
   else {
      return NULL;
   }

}

void fitsHeaderCls::CreateBasicHeader(const wxSize& frameSize, size_t nAxis)
{
   wxFitsKeyword k;

   // start with an empty list
   m_hRecords.clear();

   k.SetKeyword(wxFitsKeyword::EF_SIMPLE, true);
   m_hRecords += k;
   k.SetKeyword(wxFitsKeyword::EF_BITPIX, (long)16, wxT("U_SHORT, # bits storing pix values"));
   m_hRecords += k;

   if (nAxis==2)
      k.SetKeyword(wxFitsKeyword::EF_NAXIS, (long)nAxis, wxT("2d plane image"));
   else
      k.SetKeyword(wxFitsKeyword::EF_NAXIS, (long)nAxis, wxT("3x 2d planar rgb color image"));

   m_hRecords += k;
   k.SetKeyword(wxFitsKeyword::EF_NAXIS1, (long)frameSize.GetWidth(), wxT("# of pixels/row"));
   m_hRecords += k;
   k.SetKeyword(wxFitsKeyword::EF_NAXIS2, (long)frameSize.GetHeight(), wxT("# of rows (also # of scan lines)"));
   m_hRecords += k;
   if (nAxis==3) {
      k.SetKeyword(wxFitsKeyword::EF_NAXIS3, (long)3, wxT("# color planes seq: R G B"));
      m_hRecords += k;
   }
   k.SetKeyword(wxFitsKeyword::EF_BSCALE, (double)1.0, wxT("real = fits-value*BSCALE+BZERO"));
   m_hRecords += k;
   k.SetKeyword(wxFitsKeyword::EF_BZERO, (double)32768.0, wxT("real = fits-value*BSCALE+BZERO"));
   m_hRecords += k;

/*BM:20090620 - added to dynamic section of FITS keywords
   wxDateTime dt = wxDateTime::Now();
   dt.MakeUTC();
   wxString dts = dt.FormatISODate() + wxT("T") + dt.FormatISOTime();
   k.SetKeyword(wxFitsKeyword::EF_DATE_OBS, dts, wxT("UTC Date the file was written"));
   m_hRecords += k;
*/
   wxString dts = wxT("UTC");
   k.SetKeyword(wxFitsKeyword::EF_TIMESYS, dts, wxT("Time system used"));
   m_hRecords += k;


   wxString swVersion;
   swVersion << wxT("wxAstroCapture V ") << wxT(MyAppVer);
   k.SetKeyword(wxFitsKeyword::EF_SWCREATE, swVersion);
   m_hRecords += k;
}



/// ////////////////////////////////////////////////////////////////////
/// IMPLEMENT fitsDataCls

class fitsDataCls
{
public:
   fitsDataCls();
   virtual ~fitsDataCls();

   // used with native Pels
   void SetPel(const wxFitsOut::FitsPelType* pPel, size_t nPels, size_t pelPos);
   size_t AddPel(const wxFitsOut::FitsPelType* pPel, size_t nPels);

   // used with uchar Pels
   void SetPel(const wxUint8* pPel, size_t nPels, size_t pelPos);
   size_t AddPel(const wxUint8* pPel, size_t nPels);

   // used with ushort Pels
   void SetPel(const wxUint16* pPel, size_t nPels, size_t pelPos);
   size_t AddPel(const wxUint16* pPel, size_t nPels);

   // memory management
   // allocate nPels number of pels (NOT bytes)
   bool Allocate(size_t nPels);
   // deallocate mem and reset variables
   void Drop();
   // prepare the data area for writing as FITS image
   // MUST be called before retrieving the contents
   void Finalize();

   // return the used memory size in bytes
   size_t GetFitsDataByteSize() const {return m_memSize;};
   // return a pointer to the fits data memory
   const void* GetFitsDataPointer() const {return m_pDataMem;};

private:
   size_t   m_nPels;     // # of pels desired
   wxUint8* m_pDataMem;
   size_t   m_memSize;   // # of bytes allocated
   size_t   m_nextPelPos;
   const short  m_cSizeofPel;
   const wxFitsOut::FitsPelType   m_cZero;  // the BZERO value for USHORT
};


// Implementation of fitsDataCls

fitsDataCls::fitsDataCls()
: m_nPels(0)
, m_pDataMem(NULL)
, m_memSize(0)
, m_nextPelPos(0)
, m_cSizeofPel(sizeof(wxFitsOut::FitsPelType))
, m_cZero(-32768)
{
}

fitsDataCls::~fitsDataCls()
{
   this->Drop();
}

bool fitsDataCls::Allocate(size_t nPels)
{
   this->Drop();
   m_nPels = nPels;
   m_memSize = m_nPels * m_cSizeofPel;
   m_pDataMem = new wxUint8 [m_memSize];
   return (m_pDataMem!=NULL);
}

void fitsDataCls::Drop()
{
   if (m_pDataMem) delete [] m_pDataMem;
   m_pDataMem=NULL; m_memSize = 0; m_nPels = 0;
   m_nextPelPos = 0;
}

// prepare the data area for writing as FITS image
// FITS takes big endian format so we may have to swap all
void fitsDataCls::Finalize()
{
   if (!m_pDataMem) return;
   if (!m_nPels) return;

   wxFitsOut::FitsPelType* sdPtr = (wxFitsOut::FitsPelType*)m_pDataMem;
   switch (m_cSizeofPel) {
      case 1: {
         // nothing to do
         break;
      }
      case 2: {
         for (size_t i=0; i<m_nPels; i++) {
            *sdPtr = wxINT16_SWAP_ON_LE(*sdPtr); sdPtr++;
         }
         break;
      }
      case 4: {
         for (size_t i=0; i<m_nPels; i++) {
            *sdPtr = wxINT32_SWAP_ON_LE(*sdPtr); sdPtr++;
         }
         break;
      }
      default: ;
   }// switch
}


// Native type Pels - no conversion needed
void fitsDataCls::SetPel(const wxFitsOut::FitsPelType* pPel, size_t nPels, size_t pelPos)
{
   if (!m_pDataMem) return;
   if (!pPel) return;

   const wxFitsOut::FitsPelType* sptr = pPel;
   wxFitsOut::FitsPelType* dptr = (wxFitsOut::FitsPelType*)m_pDataMem; dptr+=pelPos;
   for (size_t i=0; i<nPels; i++) {
      *dptr++ = *sptr++;
   }
}

size_t fitsDataCls::AddPel(const wxFitsOut::FitsPelType* pPel, size_t nPels)
{
   if (!m_pDataMem) return m_nextPelPos;
   if (!pPel) return m_nextPelPos;

   this->SetPel(pPel, nPels, m_nextPelPos);
   m_nextPelPos += nPels;

   return m_nextPelPos;
}

// UCHAR type Pels add the ZERO base (0 gets  -32768 as signed short)
void fitsDataCls::SetPel(const wxUint8* pPel, size_t nPels, size_t pelPos)
{
   if (!m_pDataMem) return;
   if (!pPel) return;

   const wxUint8* sptr = pPel;
   wxFitsOut::FitsPelType* dptr = (wxFitsOut::FitsPelType*)m_pDataMem; dptr+=pelPos;
   wxFitsOut::FitsPelType pel;
   for (size_t i=0; i<nPels; i++) {
      pel = (wxFitsOut::FitsPelType)*sptr + m_cZero;
      *dptr++ = pel; sptr++;
   }
}

size_t fitsDataCls::AddPel(const wxUint8* pPel, size_t nPels)
{
   if (!m_pDataMem) return m_nextPelPos;
   if (!pPel) return m_nextPelPos;

   this->SetPel(pPel, nPels, m_nextPelPos);
   m_nextPelPos += nPels;

   return m_nextPelPos;
}

// USHORT type Pels add the ZERO base (0 gets  -32768 as signed short)
void fitsDataCls::SetPel(const wxUint16* pPel, size_t nPels, size_t pelPos)
{
   if (!m_pDataMem) return;
   if (!pPel) return;

   const wxUint16* sptr = pPel;
   wxFitsOut::FitsPelType* dptr = (wxFitsOut::FitsPelType*)m_pDataMem; dptr+=pelPos;
   wxFitsOut::FitsPelType pel;
   for (size_t i=0; i<nPels; i++) {
      pel = (wxFitsOut::FitsPelType)((long)*sptr + m_cZero); // have to go through long to avoid overflow
      *dptr++ = pel; sptr++;
   }
}

size_t fitsDataCls::AddPel(const wxUint16* pPel, size_t nPels)
{
   if (!m_pDataMem) return m_nextPelPos;
   if (!pPel) return m_nextPelPos;

   this->SetPel(pPel, nPels, m_nextPelPos);
   m_nextPelPos += nPels;

   return m_nextPelPos;
}


/// ////////////////////////////////////////////////////////////////////
/// IMPLEMENT fitsHandleCls

// carries the needed vars and some utilities for the wxFitsOut class
class fitsHandleCls
{
   friend class wxFitsOut; // nobody else should use this one

private:
   fitsHandleCls();
   virtual ~fitsHandleCls();

   bool Error() const {return m_fError;};
   void SetStatus(int status) {m_fError = m_fError || (status!=0);};

   bool SanityChecks(const wxSize& frameSize, size_t nAxis);
   // allocate space for the complete image and return the ptr to it (or NULL if it fails)
   void* Allocate();
   // deallocate mem and reset vars
   void Drop();

   // defined the primary HDU
   fitsHeaderCls m_fHeader;   // the primary header
   fitsDataCls   m_fData;     // the primary data

   // memory management for output
   wxUint8* m_pFitsData;   // main data ptr of final Fits item
   size_t  m_fitsDataLength;

   //
   bool m_fError;   // true is ERROR

   wxSize   m_frameSize;
   size_t   m_size;      // w*h
   size_t   m_nAxis;     // ..
   wxDateTime m_dateObs;  //BM:20090620 - added
   float    m_expoSeconds;
   float    m_ccdTemperature;
   size_t   m_frameNo;   // FRAMENO
   wxFitsKeywordList m_localKwdList; // the local keywords to add (persistent while obj lives)
};

// Implementation of fitsHandleCls

fitsHandleCls::fitsHandleCls()
: m_pFitsData(NULL), m_fitsDataLength(0), m_fError(false)
, m_size(0), m_nAxis(0), m_dateObs(wxDateTime::UNow())
, m_expoSeconds(0.0), m_ccdTemperature(100.0), m_frameNo(0)
{
}

fitsHandleCls::~fitsHandleCls()
{
   this->Drop();
}

bool fitsHandleCls::SanityChecks(const wxSize& frameSize, size_t nAxis)
{
   m_frameSize = wxSize(0,0); m_size=0; m_nAxis=0;

   if (m_pFitsData) return false; // already allocated - use Drop() before calling here

   if (frameSize.GetWidth()<1 || frameSize.GetWidth()>FITS_MAX_WIDTH) return false;
   if (frameSize.GetHeight()<1 || frameSize.GetHeight()>FITS_MAX_HEIGHT) return false;
   if (nAxis<2 || nAxis>3) return false;

   m_frameSize = frameSize;
   m_nAxis = nAxis;
   m_size = m_frameSize.GetWidth() * m_frameSize.GetHeight();

   //OK end
   SetStatus(0);
   return true;
}

void* fitsHandleCls::Allocate()
{
   this->Drop();
   m_fitsDataLength = m_fHeader.GetFitsHeaderByteSize() + m_fData.GetFitsDataByteSize();
   m_pFitsData = new wxUint8 [m_fitsDataLength];
   if (m_pFitsData) memset(m_pFitsData, 0x20, m_fitsDataLength); // fill with blanks

   return m_pFitsData;
}

void fitsHandleCls::Drop()
{
   if (m_pFitsData) delete [] m_pFitsData;
   m_pFitsData = NULL;
   m_fitsDataLength = 0;
}


/// ////////////////////////////////////////////////////////////////////
/// IMPLEMENT wxFitsOut

// public wxFitsOut implemetation
// deals with keywords and raw image data of type:
//  - wxUint8 (8bit)
//  - wxUint16 (16bit) (TODO)
wxFitsOut::wxFitsOut()
: m_hFits(new fitsHandleCls()) // the handle is valid while this object lives
{
}

wxFitsOut::~wxFitsOut()
{
   delete m_hFits;
}

// return the local KeywordList for manipulation
wxFitsKeywordList& wxFitsOut::KeywordListRef() const
{
   return m_hFits->m_localKwdList;
}

// return the local KeywordList for read only
const wxFitsKeywordList& wxFitsOut::KeywordList() const
{
   return m_hFits->m_localKwdList;
}

// set the date/ time of the obyervation which is added for each image as FITS keyword
void wxFitsOut::SetDateObs(wxDateTime dateObs) //BM:20090620 - added for better timestamp
{
   m_hFits->m_dateObs = dateObs;
}

// set the exposure time which is added for each image as FITS keyword
void wxFitsOut::SetExposureSeconds(float expoSeconds)
{
   m_hFits->m_expoSeconds = expoSeconds;
}

// set the CCD temperature which is added for each image as FITS keyword
void wxFitsOut::SetCCDTemperature(float temperature)
{
   m_hFits->m_ccdTemperature = temperature;
}

void wxFitsOut::SetFrameNo(size_t frameNo)
{
   m_hFits->m_frameNo = frameNo;
}


// creates a fits file with given dimensions
// and adds appropriate header keywords
bool wxFitsOut::Create(const wxSize& frameSize, size_t nAxis)
{
   if (!m_hFits) return false;  // don't have a handle ??
   if (m_hFits->Error()) return false;  // error condition

   m_hFits->Drop();  // empty everything

   // check and set the dimensions in handle
   if (!m_hFits->SanityChecks(frameSize, nAxis)) return false;

   // prepare the header (and initial FITS keywords)
   m_hFits->m_fHeader.CreateBasicHeader(frameSize, nAxis);

   // prepare the data area
   long s = frameSize.GetWidth()*frameSize.GetHeight(); if (nAxis==3) s*=3;
   m_hFits->m_fData.Allocate(s);

   return (!m_hFits->Error());
}

// mem access to get the Fitsdata out
size_t wxFitsOut::GetFitsSize() const
{
   return m_hFits->m_fitsDataLength;
}

const void* wxFitsOut::GetFitsPointer() const
{
   return m_hFits->m_pFitsData;
}


// put all together and make it available
// MUST be called before retrieving the contents
bool wxFitsOut::Finalize()
{
   // add dateObs keyword
   wxFitsKeywordEx k;
   k.DateObsKwd(m_hFits->m_dateObs);
   m_hFits->m_fHeader.KeywordListRef() += k;

   // add exposure keyword if set
   if (m_hFits->m_expoSeconds>0.0) {
      wxFitsKeywordEx k;
      k.ExposureTimeKwd(double(m_hFits->m_expoSeconds));
      m_hFits->m_fHeader.KeywordListRef() += k;
   }

   // add ccdTemp keyword if set
   if (m_hFits->m_ccdTemperature<90.0) {
      wxFitsKeywordEx k;
      k.TemperatureKwd(double(m_hFits->m_ccdTemperature));
      m_hFits->m_fHeader.KeywordListRef() += k;
   }

   // add frameNo keyword if set
   if (m_hFits->m_frameNo>0) {
      wxFitsKeywordEx k;
      k.FrameNumberKwd(m_hFits->m_frameNo);
      m_hFits->m_fHeader.KeywordListRef() += k;
   }

   // add user keywords
   m_hFits->m_fHeader.KeywordListRef() += m_hFits->m_localKwdList;

   // add the final END keyword
   m_hFits->m_fHeader.KeywordListRef() += wxFitsKeyword(wxFitsKeyword::EF_END);

   // allocate a bunch of memory to hold the complete FITS file
   wxUint8* dPtr = (wxUint8*)m_hFits->Allocate();
   if (!dPtr) return false; // ERROR EXIT -  mem allocation failed

   // get the header and write it out
   const char* pHRec; unsigned short i=0;
   pHRec = m_hFits->m_fHeader.HRecString(i);
   while (pHRec!=NULL) {
      memcpy(dPtr, pHRec, 80);
      dPtr+=80; i++; // next location
      pHRec = m_hFits->m_fHeader.HRecString(i);
   }//while

   // first do some chores needed before writing out
   m_hFits->m_fData.Finalize();
   // get the image data and write it
   const wxUint8* sPtr = (wxUint8*)m_hFits->m_fData.GetFitsDataPointer();
   if (sPtr) memcpy(dPtr, sPtr, m_hFits->m_fData.GetFitsDataByteSize());

   return true;
}

// uchar variants

// a complete image i.e. w*h pels (w*h values)
bool wxFitsOut::AddImage(const wxUint8* pImage)
{
   if (!m_hFits) return false;  // don't have a handle
   if (m_hFits->Error()) return false;  // error condition

   m_hFits->m_fData.AddPel(pImage, m_hFits->m_size);
   return (!m_hFits->Error());
}


// an image stride i.e. w pels (w values)
bool wxFitsOut::AddImageStride(const wxUint8* pImageStride)
{
   if (!m_hFits) return false;  // don't have a handle
   if (m_hFits->Error()) return false;  // error condition

   m_hFits->m_fData.AddPel(pImageStride, m_hFits->m_frameSize.GetWidth());
   return (!m_hFits->Error());
}


// an image Pel i.e. a pel (one value)
bool wxFitsOut::AddImagePel(const wxUint8* pImagePel)
{
   if (!m_hFits) return false;  // don't have a handle
   if (m_hFits->Error()) return false;  // error condition

   m_hFits->m_fData.AddPel(pImagePel, 1);
   return (!m_hFits->Error());
}

// ushort variants

// a complete image i.e. w*h pels (w*h values)
bool wxFitsOut::AddImage(const wxUint16* pImage)
{
   if (!m_hFits) return false;  // don't have a handle
   if (m_hFits->Error()) return false;  // error condition

   m_hFits->m_fData.AddPel(pImage, m_hFits->m_size);
   return (!m_hFits->Error());
}


// an image stride i.e. w pels (w values)
bool wxFitsOut::AddImageStride(const wxUint16* pImageStride)
{
   if (!m_hFits) return false;  // don't have a handle
   if (m_hFits->Error()) return false;  // error condition

   m_hFits->m_fData.AddPel(pImageStride, m_hFits->m_frameSize.GetWidth());
   return (!m_hFits->Error());
}


// an image Pel i.e. a pel (one value)
bool wxFitsOut::AddImagePel(const wxUint16* pImagePel)
{
   if (!m_hFits) return false;  // don't have a handle
   if (m_hFits->Error()) return false;  // error condition

   m_hFits->m_fData.AddPel(pImagePel, 1);
   return (!m_hFits->Error());
}

