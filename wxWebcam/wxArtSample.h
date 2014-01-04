
#ifndef _WXARTSAMPLE_H__
#define _WXARTSAMPLE_H__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


#include "ArtTypes.h" // enums and structs of the Artemis environment

#include "GenericImageMem.h" //20080829BM used to avoid double copy

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Declare an Image sampled with an Artemis Camera
//
// This class is used to return a captured image from the stream
// it is always 16bit per pel unsigned
//
// this class uses malloc and free to be compatible with wxImage SetData()
//
class wxArtSample
{
public:
	wxArtSample();
	virtual ~wxArtSample();

public:

   // NOTE (all coords are unbinned camera pixels)
	// returns an image xpos within the original frame
   wxRect SampleRect() const {return m_subRect;}
   wxSize SampleSize() const {return m_subRect.GetSize();}
   wxPoint SamplePosition() const {return m_subRect.GetPosition();}

   wxDateTime TimeStamp() const {return m_uTimeStamp;}; //BM:20090620 - added
   void MarkTimeStamp(); //BM:20090620 - added

	// returns the applied binning
	unsigned short SampleXBin() const {return m_xbin;};
	// returns the applied binning
	unsigned short SampleYBin() const {return m_ybin;};

   bool FullFrame() const {return m_fullFrame;};
	// returns the eposure time used
	float ExposureTime() const {return m_exposureTime;};

   // define the sample to be dropped while processing
   void DropSample() {m_droppedSample=true;};
   // return the dropped status of the sample
   bool DroppedSample() const {return m_droppedSample;};

	// Y Data access
	// NOTES: the bmiHeader has biBitCount = 16, no Palette, biCompression = WXACFOURCC_Y16B
	// return the ptr to the sample data of the Y plane
	const wxUint16* SampleYPtr() const;
   size_t SampleSizeY() const; //Pels
   size_t SampleByteSizeY() const; //BYTES

   // converts the image to a plain 8bit bitmap with applied gamma
   // minIn, maxIn refer to the input clipping (-1 means the calculated values are taken)
   //
   bool ConvertToBMP(float p_gamma, long p_minIn=-1, long p_maxIn=-1);
   // detaches the RGB(BGR) pointer from this obj i.e. will not be freed on destroy
   // this is used when the sample is Set into a wxImage
   unsigned char* DetachBMP();

   // init sample and transfer memory ownership
   // pls note that wxArtSample now owns the imageMem !!!
	void Init(bool fullFrame, unsigned short binx, unsigned short biny,
                  WordImageMem* imageMem, float expoTime);

   bool histogram_pivot_dev(int& pivot, int& vlow, int& vhig,int& max);

private:
   void CleanUp();

	wxRect          m_subRect;
	unsigned short  m_xbin;
	unsigned short  m_ybin;
   bool            m_fullFrame;
   bool            m_droppedSample;  // true if this sample has to be dropped

   float           m_exposureTime;   // the real expotime (seconds)

   WordImageMem*   m_pImageMem;      //20080829BM used to avoid double copy
	ByteImageMem*   m_pImageBMP;      //memobject of the BGR map when converted

	wxDateTime       m_uTimeStamp;   //BM:20090619 - add running timestamp

};


#endif  // _WXARTSAMPLE_H__

