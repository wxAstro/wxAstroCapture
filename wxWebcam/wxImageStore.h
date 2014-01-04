
#ifndef _WXIMAGESTORE_H__
#define _WXIMAGESTORE_H__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include <wx/filename.h>

#include "wxCameraTypes.h" // enums and structs of the capture environment
#include "fits/wxFitsKeyword.h"

/// Defines a generic Image Store
class wxImageStore
{
public:
   wxImageStore();
   virtual ~wxImageStore(){};

public:
   // Those must be implemented in derived classes

   // create a store of a specific type
   virtual bool Create(const wxString& dirPath,
                        EVideoSampleFormat sampleFormat, EVideoConversion   sampleConversion,
                        const wxSize& frameSize, unsigned short fps = 1) =0;
   // close the image store
   virtual void Close() =0;

   // add a BMP image to the store
   //BM:20090620 - added dateObs
   virtual bool AddBMP(const PBITMAPINFOHEADER pBmpHeader, const wxUint8* pBmpData,
                        size_t sampleSize, wxDateTime dateObs, float expoSeconds, float temperature=100.0) =0;

   // add a YP16 (Y channel plane 16bit)
   //BM:20090620 - added dateObs
   virtual bool AddYP16(const wxSize& frameSize, const wxUint16* pYData,
                        size_t sampleSize, wxDateTime dateObs, float expoSeconds, float temperature=100.0) =0;

public:
   // assigns a local list of FITS type keywords - but could be used for other store types too
   void SetKeywordList(const wxFitsKeywordList& kwdList) {m_kwdList = kwdList;};

   // retrieve with and height from Create() call
   wxSize Size() const {return m_frameSize;}

   // return the generated store name
   wxString StoreName() const {return m_storeName.GetPath();};
   // returns true if the store was created successfully and is accepting images
   bool IsReadyForCapture() const {return m_readyForCapture;};

protected:
   wxSize               m_frameSize;
   bool                 m_readyForCapture;
   wxFileName           m_storeName;
   unsigned short       m_fps;
   wxFitsKeywordList    m_kwdList;
};


/// Defines the AVI Image Store
class CAviLib;
class CAviBuffer;

class wxImageStoreAVI : public wxImageStore
{
public:
   wxImageStoreAVI();
   virtual ~wxImageStoreAVI();

public:
   bool Create(const wxString& dirPath,
                        EVideoSampleFormat sampleFormat, EVideoConversion   sampleConversion,
                        const wxSize& frameSize, unsigned short fps = 1);
   void Close();

   // add a BMP image to the store
   //BM:20090620 - added dateObs
   bool AddBMP(const PBITMAPINFOHEADER pBmpHeader, const wxUint8* pBmpData,
               size_t sampleSize, wxDateTime dateObs, float expoSeconds, float temperature=100.0);

   // add a YP16 (Y channel plane 16bit) - NOT IMPLEMENTED
   //BM:20090620 - added dateObs
   bool AddYP16(const wxSize& frameSize, const wxUint16* pYData,
                size_t sampleSize, wxDateTime dateObs, float expoSeconds, float temperature=100.0);

private:
   CAviLib*     m_aviHandler;       // AVI streamer
   CAviBuffer*  m_aviBuffer;        // a framebuffer for AVI streaming
};


/// Defines the FITS Image Store

class wxFFileOutputStream;
class wxFitsOutputStream;

class wxImageStoreFITS : public wxImageStore
{
public:
   wxImageStoreFITS();
   virtual ~wxImageStoreFITS();

public:
   bool Create(const wxString& dirPath,
                        EVideoSampleFormat sampleFormat, EVideoConversion   sampleConversion,
                        const wxSize& frameSize, unsigned short fps = 1);
   void Close();

   // add a BMP image to the store
   //BM:20090620 - added dateObs
   bool AddBMP(const PBITMAPINFOHEADER pBmpHeader, const wxUint8* pBmpData,
               size_t sampleSize, wxDateTime dateObs, float expoSeconds, float temperature=100.0);

   // add a YP16 (Y channel plane 16bit)
   //BM:20090620 - added dateObs
   bool AddYP16(const wxSize& frameSize, const wxUint16* pYData,
                size_t sampleSize, wxDateTime dateObs, float expoSeconds, float temperature=100.0);

private:
   wxFFileOutputStream*  m_outStream;
   wxFitsOutputStream*   m_fitsStream;

   size_t                  m_imageCount;
};


/// Defines the Zipped FITS Image Store

class wxFFileOutputStream;
class wxZipOutputStream;
class wxFitsOutputStream;

class wxImageStoreZippedFITS : public wxImageStore
{
public:
   wxImageStoreZippedFITS();
   virtual ~wxImageStoreZippedFITS();

public:
   bool Create(const wxString& dirPath,
                        EVideoSampleFormat sampleFormat, EVideoConversion   sampleConversion,
                        const wxSize& frameSize, unsigned short fps = 1);
   void Close();

   // add a BMP image to the store
   //BM:20090620 - added dateObs
   bool AddBMP(const PBITMAPINFOHEADER pBmpHeader, const wxUint8* pBmpData,
               size_t sampleSize, wxDateTime dateObs, float expoSeconds, float temperature=100.0);

   // add a YP16 (Y channel plane 16bit)
   //BM:20090620 - added dateObs
   bool AddYP16(const wxSize& frameSize, const wxUint16* pYData,
                size_t sampleSize, wxDateTime dateObs, float expoSeconds, float temperature=100.0);

private:
   wxFFileOutputStream*  m_outStream;
   wxZipOutputStream*    m_zipStream;
   wxFitsOutputStream*   m_fitsStream;

   size_t                  m_imageCount;
};

#endif  // _WXIMAGESTORE_H__


