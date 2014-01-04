
#ifndef _WXWEBSAMPLE_H__
#define _WXWEBSAMPLE_H__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "captypes.h" // enums and structs of the capture environment

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Declare a wxWebSample
//
// This class is used to return a captured image from the stream
// it is always 8bit per pel unsigned (0 .. 255)
//
// this class uses malloc and free to be compatible with wxImage SetData()
//
class ByteImageMem;

class wxWebSample
{
public:
	wxWebSample();
	virtual ~wxWebSample();

public:
   // NOTE (all coords are unbinned camera pixels)
	// returns an image xpos within the original frame
   wxRect SampleRect() const {return m_subRect;}
   wxSize SampleSize() const {return m_subRect.GetSize();}
   wxPoint SamplePosition() const {return m_subRect.GetPosition();}

   wxDateTime TimeStamp() const {return m_uTimeStamp;}; //BM:20090620 - added
   void MarkTimeStamp(); //BM:20090620 - added

   bool FullFrame() const {return m_fullFrame;};

   // define the sample to be dropped while processing
   void DropSample() {m_droppedSample=true;};
   // return the dropped status of the sample
   bool DroppedSample() const {return m_droppedSample;};



	// return the ptr to header of the bitmap (ptr to BITMAPINFOHEADER)
	const PBITMAPINFOHEADER SampleHeaderBGRPtr() const;
	// return the ptr to header of the bitmap (ptr to BITMAPINFOHEADER)
	const PBITMAPINFOHEADER SampleHeaderYUVPtr() const;

	// RGB24 Data access
	// NOTES: the bmiHeader has biBitCount = 24, no Palette,
	// return the ptr to the sample data of the bitmap
	const wxUint8* SampleBGRPtr() const;
   size_t SampleSizeBgr() const;

	// YUV Data access
	// NOTES: the bmiHeader has biBitCount = 12, no Palette, biCompression = WMCFOURCC_I420 (I420)
	// return the ptr to the sample data of the Y plane
	const wxUint8* SampleYUVPtr() const;
   size_t SampleSizeYuv() const;

   // swaps the R and B pixel values in the BGR map
   void ConvertBGR2RGB();
   // detaches the RGB(BGR) pointer from this obj i.e. will not be freed on destroy
   // this is used when the sample is Set into a wxImage
   wxUint8* DetachBMP();

   // init sample and transfer memory ownership
   // pls note that wxWebSample now owns the imageMem !!!
	void Init(bool fullFrame, EVideoConversion sampleFormat, ByteImageMem* pImageYuv, ByteImageMem* pImageBgr);

private:
   void CleanUp();

	wxRect           m_subRect;
   bool             m_fullFrame;
   bool             m_droppedSample;  // true if this sample has to be dropped

	size_t           m_sampleSizeBGR;
	BITMAPINFOHEADER m_pHeaderBGR; // pBITMAPINFOHEADER
	ByteImageMem*    m_pImageBGR;

	size_t           m_sampleSizeYUV;
	BITMAPINFOHEADER m_pHeaderYUV; // pBITMAPINFOHEADER
	ByteImageMem*    m_pImageYUV;

	wxDateTime       m_uTimeStamp;   //BM:20090619 - add running timestamp
};



#endif  // _WXWEBSAMPLE_H__

