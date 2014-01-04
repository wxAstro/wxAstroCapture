// DoubleMap.h: interface for the DoubleMap class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _DOUBLEMAP_H__
#define _DOUBLEMAP_H__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if defined(__UNIX__)

#include "captypes.h"

class DoubleMap
{
public:
	DoubleMap();
	virtual ~DoubleMap();

public:
 	typedef  DMTYPE_DEF DMTYPE; // class typedef for proper handling

public:
	// create a new Map with dimension
	bool CreateFrom(long width, long height, EVideoSampleFormat mapFormat = EVF_RGB24);
	// add pixels from new bitmap
	bool AddBitmap(unsigned char* bitmap);
	// apply a darkmap
	bool AddDarkMap(unsigned char* pBitmap);
	// merge dark and image
	bool MergeBitmap(double factor, unsigned char *pBitmap);

	// returning maps

	// a stretched based on min, max of each channel limited by range
	bool GetStretched(double histRange, unsigned char *pBitmap);
	// a log streched
	bool GetLog(unsigned char* pBitmap);
	// a histogram stretched
	bool GetHistStreched(unsigned char* pBitmap);
	// a stretched based on min, max of each channel
	bool GetStretched(unsigned char* pBitmap);
	// just the summed image
	bool GetSum(unsigned char* pBitmap);
	// apply mean calc before returning it
	bool GetMean(unsigned char *pBitmap);

	// access to underlying data
	bool GetMapData(long* mapSize, DMTYPE* *pBitmap);

	// the # samples collected in the map
	long GetNumSamples();

private:
	bool Bitmap2NewMap(unsigned char* pBitmap);
	void zz_DumpToBitmap(unsigned char* pBitmap);

	short   m_width;
	short   m_scanwidth;        // bitmap width (dword sized)
	short   m_pixsize;          // size of a bitmap pixel in bytes
	short   m_height;
	long    m_size;
	EVideoSampleFormat m_mapFormat;
	short   m_bitDepth;
	float   m_numSamples;
	bool    m_tracked;
	float   m_dx;
	float   m_dy;               // last displacement

	bool    m_darkProcessing;   // dark enabled
	float   m_darkScale;        // how much of the dark is subtracted

	bool    m_register;

	DMTYPE* m_pMap;            // cumulative result map
	DMTYPE* m_pDarkMap;        // a dark image to subtract
	DMTYPE* m_pNewMap;         // any new map that is collected goes here


};

#endif // __UNIX__
#endif // _DOUBLEMAP_H__
