

#include "wxImageStore.h"

#include "../wxFactory.h"
#include "../wxMessageArea.h"

// Stream things
#include <wx/wfstream.h>
#include <wx/zipstrm.h>

// AVI things
#include "avi/avilib.h"

// FITS things
#include "fits/wxFitsOutputStream.h"
#include "wxBMP2Fits.h"
#include "wxYP162Fits.h"

// name things
#include "../wxNamePolicy.h"

/// Implement the generic Image Store

wxImageStore::wxImageStore()
: m_readyForCapture(false)
, m_fps(1)
{
}

/// Implement the AVI Image Store

wxImageStoreAVI::wxImageStoreAVI()
: wxImageStore()
, m_aviHandler(new CAviLib())
, m_aviBuffer(NULL)
{
}

wxImageStoreAVI::~wxImageStoreAVI()
{
   this->Close();
}

bool wxImageStoreAVI::Create(const wxString& dirPath,
                        EVideoSampleFormat sampleFormat, EVideoConversion   sampleConversion,
                        const wxSize& frameSize, unsigned short fps)
{
   if (m_aviBuffer) {
      wxArrayString messages;
      messages.Add(wxT("Ooops - software error!"));
      messages.Add(wxString::Format(wxT("Software error in: %s, Line >> %u"),__FILE__, __LINE__ ));
      wxMessageArea::wxMessageSink(messages,wxT("Image Store - AVI Create"));
      return false;   // ERROR EXIT - already created
   }

   m_readyForCapture = false;

   // create new buffer
   m_frameSize = frameSize;
   m_aviBuffer = new CAviBuffer(m_frameSize.GetWidth(), m_frameSize.GetHeight());

   //20100822:BM - fixed bug in AVI create for RGB
   switch(sampleFormat) {
      case EVF_RGB24: { m_aviBuffer->SetRGB(); break; }
      case EVF_YUV : {
         switch(sampleConversion) {
            case EVC_I420_RGB24 : { m_aviBuffer->SetYUV(); break; }
            case EVC_YUY2_RGB24 : { m_aviBuffer->SetYUY2(); break; }
            case EVC_YVYU_RGB24 : { m_aviBuffer->SetYVYU(); break; }
            case EVC_UYVY_RGB24 : { m_aviBuffer->SetUYVY(); break; }
            case EVC_Y800_RGB24 : { m_aviBuffer->SetY800(); break; }
            default       : { m_aviBuffer->SetYUV();  }
         }
         break;
      }
      //20100822:BM - add error msg for wrong AVI create
      case EVF_YP16 : { //Art format 16bit not likely to fit an AVI and should not land here...
         wxArrayString messages;
         messages.Add(wxT("Ooops - software error!"));
         messages.Add(wxString::Format(wxT("Software error in: %s, Line >> %u"),__FILE__, __LINE__ ));
         wxMessageArea::wxMessageSink(messages,wxT("Image Store - AVI Create from Art Camera"));
         return false;   // ERROR EXIT -
      }
      default       : { m_aviBuffer->SetYUV();  }
   }

   wxNamePolicyContainer* cpolicy = wxF()->getNamePolicyContainer();

   m_storeName.Clear();
   m_storeName.AssignDir(dirPath);
   if ( ! m_storeName.IsDirWritable() ) {
      wxArrayString messages;
      messages.Add(wxT("Cannot write to folder or it does not exist!"));
      messages.Add(m_storeName.GetFullPath());
      wxMessageArea::wxMessageSink(messages,wxT("Image Store - Create AVI"));
      return false;  // ERROR EXIT cannot write to dir or it does not exist
   }
   m_storeName.SetName(cpolicy->GetName());
   m_storeName.SetExt(wxT("avi"));
   if ( m_storeName.FileExists() ) {
      wxArrayString messages;
      messages.Add(wxT("File already exists - we don't overwrite!"));
      messages.Add(m_storeName.GetFullName());
      wxMessageArea::wxMessageSink(messages,wxT("Image Store - Create AVI"));
      return false;  // ERROR EXIT file already exists
   }

   m_fps = fps;
   m_readyForCapture = m_aviHandler->avi_open(m_storeName.GetFullPath(), m_aviBuffer->m_videoFmt, fps);
   if (m_readyForCapture) cpolicy->CommitName();

   return m_readyForCapture;
}


void wxImageStoreAVI::Close()
{
   if(m_readyForCapture) {
       m_aviHandler->avi_close();
   }
   if (m_aviBuffer) delete m_aviBuffer; m_aviBuffer = NULL;

   m_frameSize = wxSize(0,0);
   m_readyForCapture = false;
   m_storeName.Clear();
   m_fps = 1;
}


bool wxImageStoreAVI::AddBMP(const PBITMAPINFOHEADER pBmpHeader, const wxUint8* pBmpData,
                        size_t sampleSize, wxDateTime WXUNUSED(dateObs), float WXUNUSED(expoSeconds), float WXUNUSED(temperature))
{
   // sanity
   if ( (!pBmpHeader) || (!pBmpData) || (!sampleSize) || (!m_aviBuffer) ) {
      wxArrayString messages;
      messages.Add(wxT("Ooops - software error!"));
      messages.Add(wxString::Format(wxT("Software error in: %s, Line >> %u"),__FILE__, __LINE__ ));
      wxMessageArea::wxMessageSink(messages,wxT("Image Store - AVI AddBMP"));
      return false;
   }

   // setup buffer and dump BGR bitmap into the file
   m_aviBuffer->m_videoBuffer.data = pBmpData;
   m_aviBuffer->m_videoBuffer.size = sampleSize;

   int retVal = m_aviHandler->avi_video(m_aviBuffer->m_videoBuffer);       // write a sample
   return (retVal==0);
}

// add a YP16 (Y channel plane 16bit)
bool wxImageStoreAVI::AddYP16(const wxSize& WXUNUSED(frameSize),
                    const wxUint16* WXUNUSED(pYData), size_t WXUNUSED(sampleSize),
                    wxDateTime WXUNUSED(dateObs), float WXUNUSED(expoSeconds), float WXUNUSED(temperature))
{
   wxArrayString messages;
   messages.Add(wxT("Ooops - software error!"));
   messages.Add(wxString::Format(wxT("Software error in: %s, Line >> %u"),__FILE__, __LINE__ ));
   wxMessageArea::wxMessageSink(messages,wxT("Image Store - AVI AddYP16"));
   return false;
}


/// Implement the Zipped FITS Image Store

wxImageStoreZippedFITS::wxImageStoreZippedFITS()
: wxImageStore()
, m_outStream(NULL)
, m_zipStream(NULL)
, m_fitsStream(NULL)
, m_imageCount(0)
{
}

wxImageStoreZippedFITS::~wxImageStoreZippedFITS()
{
   this->Close();
}

bool wxImageStoreZippedFITS::Create(const wxString& dirPath,
                        EVideoSampleFormat WXUNUSED(sampleFormat), EVideoConversion   WXUNUSED(sampleConversion),
                        const wxSize& frameSize, unsigned short fps)
{
   if (m_fitsStream) {
      wxArrayString messages;
      messages.Add(wxT("Ooops - software error!"));
      messages.Add(wxString::Format(wxT("Software error in: %s, Line >> %u"),__FILE__, __LINE__ ));
      wxMessageArea::wxMessageSink(messages,wxT("Image Store - ZippedFITS Create"));
      return false;   // ERROR EXIT - already created
   }

   m_readyForCapture = false;

   // create new buffer
   m_frameSize = frameSize;

   wxNamePolicyContainer* cpolicy = wxF()->getNamePolicyContainer();

   m_storeName.Clear();
   m_storeName.AssignDir(dirPath);
   if ( ! m_storeName.IsDirWritable() ) {
      wxArrayString messages;
      messages.Add(wxT("Cannot write to folder or it does not exist!"));
      messages.Add(m_storeName.GetFullPath());
      wxMessageArea::wxMessageSink(messages,wxT("Image Store - ZippedFITS Create"));
      return false;  // ERROR EXIT cannot write to dir or it does not exist
   }
   m_storeName.SetName(cpolicy->GetName());
   m_storeName.SetExt(wxT("zip"));
   if ( m_storeName.FileExists() ) {
      wxArrayString messages;
      messages.Add(wxT("File already exists - we don't overwrite!"));
      messages.Add(m_storeName.GetFullName());
      wxMessageArea::wxMessageSink(messages,wxT("Image Store - ZippedFITS Create"));
      return false;  // ERROR EXIT file already exists
   }

   m_fps = fps;

   // create stream chain
   m_outStream = new wxFFileOutputStream(m_storeName.GetFullPath());
   if (!m_outStream) return false; // ERROR EXIT cannot create outfile
   m_zipStream = new wxZipOutputStream(*m_outStream);
   if (!m_zipStream) {
      delete m_outStream;
      wxArrayString messages;
      messages.Add(wxT("Ooops - Cannot create ZIP file!"));
      messages.Add(wxString::Format(wxT("Cannot create error in: %s, Line >> %u"),__FILE__, __LINE__ ));
      wxMessageArea::wxMessageSink(messages,wxT("Image Store - ZippedFITS Create"));
      return false; // ERROR EXIT cannot create outfile
   }
   m_fitsStream = new wxFitsOutputStream(*m_zipStream);
   if (!m_fitsStream) {
      delete m_zipStream;
      delete m_outStream;
      wxArrayString messages;
      messages.Add(wxT("Ooops - Cannot create ZIP file!"));
      messages.Add(wxString::Format(wxT("Cannot create error in: %s, Line >> %u"),__FILE__, __LINE__ ));
      wxMessageArea::wxMessageSink(messages,wxT("Image Store - ZippedFITS Create"));
      return false; // ERROR EXIT cannot create outfile
   }

   m_imageCount = 0;
   m_readyForCapture = true;
   if (m_readyForCapture) cpolicy->CommitName();

   return m_readyForCapture;
}


void wxImageStoreZippedFITS::Close()
{
   if(m_readyForCapture) {
      m_zipStream->Close();
   }
   if (m_fitsStream) {
      delete m_fitsStream; m_fitsStream = NULL;
      delete m_zipStream;  m_zipStream = NULL;
      delete m_outStream;  m_outStream = NULL;
   }

   m_frameSize = wxSize(0,0);
   m_readyForCapture = false;
   m_storeName.Clear();
   m_fps = 1;
   m_imageCount = 0;
}


bool wxImageStoreZippedFITS::AddBMP(const PBITMAPINFOHEADER pBmpHeader, const wxUint8* pBmpData,
                              size_t sampleSize, wxDateTime dateObs, float expoSeconds, float temperature)
{
   // sanity
   if ( (!pBmpHeader) || (!pBmpData) || (!sampleSize) || (!m_fitsStream) ) {
      wxArrayString messages;
      messages.Add(wxT("Ooops - software error!"));
      messages.Add(wxString::Format(wxT("Software error in: %s, Line >> %u"),__FILE__, __LINE__ ));
      wxMessageArea::wxMessageSink(messages,wxT("Image Store - ZippedFITS AddBMP"));
      return false;
   }

   wxNamePolicyFile* fpolicy = wxF()->getNamePolicyFile();

   wxBMP2Fits fits; // the FITS format handler
   // setup buffer and dump BGR bitmap into the file
   fits.AddBMP(pBmpHeader, pBmpData);
   fits.SetDateObs(dateObs); //BM:20090620 - added
   fits.SetExposureSeconds(expoSeconds);   // FITS needs exposure time in header
   fits.SetCCDTemperature(temperature);    // FITS needs ccd temp in header
   fits.SetFrameNo(fpolicy->GetIncrement());// FITS needs frame number in header
   fits.KeywordListRef() = m_kwdList;      // FITS can have more user keywords in header
   fits.Finalize();

   wxString fname = fpolicy->GetName();  fname+= wxT(".fits");

   m_imageCount++;

   // stream the content as a zip entry
   m_zipStream->PutNextEntry(fname);
   m_fitsStream->Write(fits);
   m_zipStream->CloseEntry();

   bool retVal = (m_zipStream->GetLastError()==wxSTREAM_NO_ERROR);
   if (retVal) fpolicy->CommitName();

   return retVal;
}

// add a YP16 (Y channel plane 16bit)
bool wxImageStoreZippedFITS::AddYP16(const wxSize& frameSize,
                               const wxUint16* pYData, size_t sampleSize,
                               wxDateTime dateObs, float expoSeconds, float temperature)
{
   // sanity
   if ( (!pYData) || (!sampleSize) || (!m_fitsStream) ) {
      wxArrayString messages;
      messages.Add(wxT("Ooops - software error!"));
      messages.Add(wxString::Format(wxT("Software error in: %s, Line >> %u"),__FILE__, __LINE__ ));
      wxMessageArea::wxMessageSink(messages,wxT("Image Store - ZippedFITS AddYP16"));
      return false;
   }

   wxNamePolicyFile* fpolicy = wxF()->getNamePolicyFile();

   wxYP162Fits fits; // the FITS format handler
   // setup buffer and dump BGR bitmap into the file
   fits.AddYP16(frameSize, pYData);
   fits.SetDateObs(dateObs); //BM:20090620 - added
   fits.SetExposureSeconds(expoSeconds);  // FITS needs exposure time in header
   fits.SetCCDTemperature(temperature);   // FITS needs ccd temp in header
   fits.SetFrameNo(fpolicy->GetIncrement());// FITS needs frame number in header
   fits.KeywordListRef() = m_kwdList;     // FITS can have more user keywords in header
   fits.Finalize();

   wxString fname = fpolicy->GetName(); fname+= wxT(".fits");

   m_imageCount++;

   // stream the content as a zip entry
   m_zipStream->PutNextEntry(fname);
   m_fitsStream->Write(fits);
   m_zipStream->CloseEntry();

   bool retVal = (m_zipStream->GetLastError()==wxSTREAM_NO_ERROR);
   if (retVal) fpolicy->CommitName();

   return retVal;
}

/// Implement the FITS Image Store

wxImageStoreFITS::wxImageStoreFITS()
: wxImageStore()
, m_outStream(NULL)
, m_fitsStream(NULL)
, m_imageCount(0)
{
}

wxImageStoreFITS::~wxImageStoreFITS()
{
   this->Close();
}

bool wxImageStoreFITS::Create(const wxString& dirPath,
                        EVideoSampleFormat WXUNUSED(sampleFormat), EVideoConversion   WXUNUSED(sampleConversion),
                        const wxSize& frameSize, unsigned short fps)
{
   if (m_fitsStream) {
      wxArrayString messages;
      messages.Add(wxT("Ooops - software error!"));
      messages.Add(wxString::Format(wxT("Software error in: %s, Line >> %u"),__FILE__, __LINE__ ));
      wxMessageArea::wxMessageSink(messages,wxT("Image Store - FITS Create"));
      return false;   // ERROR EXIT - already created
   }

   m_readyForCapture = false;

   // create new buffer
   m_frameSize = frameSize;

   wxNamePolicyContainer* cpolicy = wxF()->getNamePolicyContainer();

   m_storeName.Clear();
   m_storeName.AssignDir(dirPath);
   if ( ! m_storeName.IsDirWritable() ) {
      wxArrayString messages;
      messages.Add(wxT("Cannot write to folder or it does not exist!"));
      messages.Add(m_storeName.GetFullPath());
      wxMessageArea::wxMessageSink(messages,wxT("Image Store - FITS Create"));
      return false;  // ERROR EXIT cannot write to dir or it does not exist
   }
   m_storeName.AppendDir(cpolicy->GetName());  // this is a folder name

   m_fps = fps;
   m_imageCount = 0;
   m_readyForCapture = m_storeName.Mkdir(0777, wxPATH_MKDIR_FULL);
   if (m_readyForCapture) cpolicy->CommitName();

   return m_readyForCapture;
}


void wxImageStoreFITS::Close()
{

   m_frameSize = wxSize(0,0);
   m_readyForCapture = false;
   m_storeName.Clear();
   m_fps = 1;
   m_imageCount = 0;
}


bool wxImageStoreFITS::AddBMP(const PBITMAPINFOHEADER pBmpHeader, const wxUint8* pBmpData,
                              size_t sampleSize, wxDateTime dateObs, float expoSeconds, float temperature)
{
   // sanity
   if ( (!pBmpHeader) || (!pBmpData) || (!sampleSize) ) {
      wxArrayString messages;
      messages.Add(wxT("Ooops - software error!"));
      messages.Add(wxString::Format(wxT("Software error in: %s, Line >> %u"),__FILE__, __LINE__ ));
      wxMessageArea::wxMessageSink(messages,wxT("Image Store - FITS AddBMP"));
      return false;
   }

   wxNamePolicyFile* fpolicy = wxF()->getNamePolicyFile();

   wxBMP2Fits fits; // the FITS format handler
   // setup buffer and dump BGR bitmap into the file
   fits.AddBMP(pBmpHeader, pBmpData);
   fits.SetDateObs(dateObs); //BM:20090620 - added
   fits.SetExposureSeconds(expoSeconds);  // FITS needs exposure time in header
   fits.SetCCDTemperature(temperature);   // FITS needs ccd temp in header
   fits.SetFrameNo(fpolicy->GetIncrement());// FITS needs frame number in header
   fits.KeywordListRef() = m_kwdList;     // FITS can have more user keywords in header
   fits.Finalize();

   wxFileName fname;
   fname.AssignDir(m_storeName.GetPath());
   fname.SetName(fpolicy->GetName());
   fname.SetExt(wxT("fits"));

   m_imageCount++;

   m_outStream = new wxFFileOutputStream(fname.GetFullPath());
   if (!m_outStream) {
      wxArrayString messages;
      messages.Add(wxT("Ooops - Cannot create FITS file!"));
      messages.Add(fname.GetFullPath());
      messages.Add(wxString::Format(wxT("Cannot create error in: %s, Line >> %u"),__FILE__, __LINE__ ));
      wxMessageArea::wxMessageSink(messages,wxT("Image Store - FITS AddBMP"));
      return false; // ERROR EXIT cannot create outfile
   }
   m_fitsStream = new wxFitsOutputStream(*m_outStream);
   if (!m_fitsStream) {
      wxArrayString messages;
      messages.Add(wxT("Ooops - Cannot create FITS stream!"));
      messages.Add(wxString::Format(wxT("Cannot create error in: %s, Line >> %u"),__FILE__, __LINE__ ));
      wxMessageArea::wxMessageSink(messages,wxT("Image Store - FITS AddBMP"));
      delete m_outStream;
      return false; // ERROR EXIT cannot create outfile
   }

   m_fitsStream->Write(fits);
   bool retVal = (m_fitsStream->GetLastError()==wxSTREAM_NO_ERROR);
   m_fitsStream->Close();
   if (m_fitsStream) {
      delete m_fitsStream; m_fitsStream = NULL;
      delete m_outStream;  m_outStream = NULL;
   }
   if (retVal) fpolicy->CommitName();

   return retVal;
}

// add a YP16 (Y channel plane 16bit)
bool wxImageStoreFITS::AddYP16(const wxSize& frameSize,
                               const wxUint16* pYData, size_t sampleSize,
                               wxDateTime dateObs, float expoSeconds, float temperature)
{
   // sanity
   if ( (!pYData) || (!sampleSize) ) {
      wxArrayString messages;
      messages.Add(wxT("Ooops - software error!"));
      messages.Add(wxString::Format(wxT("Software error in: %s, Line >> %u"),__FILE__, __LINE__ ));
      wxMessageArea::wxMessageSink(messages,wxT("Image Store - FITS AddYP16"));
      return false;
   }

   wxNamePolicyFile* fpolicy = wxF()->getNamePolicyFile();

   wxYP162Fits fits; // the FITS format handler
   // setup buffer and dump BGR bitmap into the file
   fits.AddYP16(frameSize, pYData);
   fits.SetDateObs(dateObs); //BM:20090620 - added
   fits.SetExposureSeconds(expoSeconds);  // FITS needs exposure time in header
   fits.SetCCDTemperature(temperature);   // FITS needs ccd temp in header
   fits.SetFrameNo(fpolicy->GetIncrement());// FITS needs frame number in header
   fits.KeywordListRef() = m_kwdList;     // FITS can have more user keywords in header
   fits.Finalize();

   wxFileName fname;
   fname.AssignDir(m_storeName.GetPath());
   fname.SetName(fpolicy->GetName());
   fname.SetExt(wxT("fits"));

   m_imageCount++;

   m_outStream = new wxFFileOutputStream(fname.GetFullPath());
   if (!m_outStream) {
      wxArrayString messages;
      messages.Add(wxT("Ooops - Cannot create FITS file!"));
      messages.Add(fname.GetFullPath());
      messages.Add(wxString::Format(wxT("Cannot create error in: %s, Line >> %u"),__FILE__, __LINE__ ));
      wxMessageArea::wxMessageSink(messages,wxT("Image Store - FITS AddYP16"));
      return false; // ERROR EXIT cannot create outfile
   }
   m_fitsStream = new wxFitsOutputStream(*m_outStream);
   if (!m_fitsStream) {
      wxArrayString messages;
      messages.Add(wxT("Ooops - Cannot create FITS stream!"));
      messages.Add(wxString::Format(wxT("Cannot create error in: %s, Line >> %u"),__FILE__, __LINE__ ));
      wxMessageArea::wxMessageSink(messages,wxT("Image Store - FITS AddYP16"));
      delete m_outStream;
      return false; // ERROR EXIT cannot create outfile
   }

   m_fitsStream->Write(fits);
   bool retVal = (m_fitsStream->GetLastError()==wxSTREAM_NO_ERROR);
   m_fitsStream->Close();
   if (m_fitsStream) {
      delete m_fitsStream; m_fitsStream = NULL;
      delete m_outStream;  m_outStream = NULL;
   }
   if (retVal) fpolicy->CommitName();

   return retVal;
}


