// DoubleMap.cpp: implementation of the DoubleMap class.
//
//////////////////////////////////////////////////////////////////////

#include "DoubleMap.h"
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if defined(__UNIX__)


#include <malloc.h>
#include <math.h>
#include <limits.h>



//////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////

#define PELMAX (255)
#define PELMIN (0)
#define PELNUL (0)  // type conform zero

#ifndef CLIP
#define CLIP(x) ( (unsigned char)(( x>PELMAX) ? PELMAX : ( ( x<PELMIN) ? PELMIN : x)) )
#endif

#ifndef CLIPN
#define CLIPN(x, min, max) ( (x>max) ? max : ( (x<min) ? min : x) )
#endif

#ifndef MIN
#define MIN(x, y) (x < y ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x, y) (x > y ? (x) : (y))
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DoubleMap::DoubleMap()
: m_width(0)
, m_scanwidth(0)
, m_pixsize(3)
, m_height(0)
, m_size(0)
, m_mapFormat(EVF_Unknown)
, m_bitDepth(0)
, m_numSamples(0)
, m_tracked(false)
, m_dx(0.0F)
, m_dy(0.0F)
, m_darkProcessing(false)
, m_darkScale((float)0.8)
, m_register(false)
, m_pMap(NULL)
, m_pDarkMap(NULL)
, m_pNewMap(NULL)
{
}

DoubleMap::~DoubleMap()
{
	if (m_pMap)
		delete [] m_pMap;
	if (m_pDarkMap)
		delete [] m_pDarkMap;
	if (m_pNewMap)
		delete [] m_pNewMap;

}

// NOTE the double map ist organised as 3 consecutive planes of x*y dimension
// ordered G R B within one block of memory
// bitmaps arrive in the usual x*y map with BGR ordered Pixels




bool DoubleMap::CreateFrom(long width, long height, EVideoSampleFormat mapFormat)
{
bool hr = true;
//BEGIN
    if (mapFormat!=EVF_YUV && mapFormat!=EVF_RGB24) {
        return false; // ERROR EXIT cannot create other maps
    }

	if (m_pMap) {
		delete [] m_pMap;	// drop existing
		m_pMap = NULL; m_size = 0;
	}
	if (m_pDarkMap)
		delete [] m_pDarkMap;
	if (m_pNewMap)
		delete [] m_pNewMap;

    m_mapFormat = mapFormat;

	m_pixsize = (m_mapFormat==EVF_YUV)? 1 : 3; // YUV or RGB type bitmap
    m_bitDepth = (m_mapFormat==EVF_YUV)? 12 : 24; // YUV or RGB type bitmap

	m_width  = (short)width;
	m_scanwidth = ((m_width*m_pixsize + 3) >> 2 ) << 2;  // DWORD alignement required
	m_scanwidth = (m_mapFormat==EVF_YUV)? m_width : m_scanwidth; // YUV = width!!

	m_height = (short)height;
	m_size   = m_width * m_height * m_bitDepth/8;
	m_dx = 0.0F; m_dy = 0.0F;

	m_pMap = new  DMTYPE[m_size];
	m_pDarkMap = new  DMTYPE[m_size];
	m_pNewMap = new  DMTYPE[m_size];

	memset(m_pMap, 0, m_size*sizeof(DMTYPE) );
	memset(m_pDarkMap, 0, m_size*sizeof(DMTYPE) );
	memset(m_pNewMap, 0, m_size*sizeof(DMTYPE) );
	m_numSamples = 0.0F;

	return hr;
}//END CreateFrom

//
// just copy and convert the Bitmap into a DMTYPE NewMap
//  subtract dark if there is one to be used
bool DoubleMap::Bitmap2NewMap(unsigned char* pBitmap)
{
bool hr = true;
register long x,y;
DMTYPE* ptrD = m_pDarkMap;
DMTYPE* ptrN = m_pNewMap;
unsigned char* sPtr;
//BEGIN
	if (!m_pMap) {
		return false;
	}
    if (!pBitmap)
        return false;  // not even a valid pointer

	if (m_darkProcessing)
	{
	    switch (m_mapFormat) {
	        case EVF_YUV:
                // YUV - just copy the contents
                sPtr = pBitmap;
                for (y=0; y<m_height*m_width; y++) {
                        *ptrN++ = (DMTYPE)*sPtr++  - (DMTYPE)(*ptrD++ * m_darkScale);
                }//y
                // U Plane half width, half height
                for (y=0; y<m_height*m_width/4; y++) {
                        *ptrN++ = (DMTYPE)*sPtr++  - (DMTYPE)(*ptrD++ * m_darkScale);
                }//y
                // V Plane half width, half height
                for (y=0; y<m_height*m_width/4; y++) {
                        *ptrN++ = (DMTYPE)*sPtr++  - (DMTYPE)(*ptrD++ * m_darkScale);
                }//y
                break;
	        case EVF_RGB24:
                for (y=0; y<m_height; y++) {// take care of stride (scanlength vs. width)
                    for (x=1; x<(m_width*m_pixsize); x+=m_pixsize) {//G
                        *ptrN++ = (DMTYPE)pBitmap[y*m_scanwidth+x]  - (DMTYPE)(*ptrD++ * m_darkScale);
                    }
                }//y
                for (y=0; y<m_height; y++) {// take care of stride (scanlength vs. width)
                    for (x=2; x<(m_width*m_pixsize); x+=m_pixsize) {//R
                        *ptrN++ = (DMTYPE)pBitmap[y*m_scanwidth+x]  - (DMTYPE)(*ptrD++ * m_darkScale);
                    }
                }//y
                for (y=0; y<m_height; y++) {// take care of stride (scanlength vs. width)
                    for (x=0; x<(m_width*m_pixsize); x+=m_pixsize) {//B
                        *ptrN++ = (DMTYPE)pBitmap[y*m_scanwidth+x]  - (DMTYPE)(*ptrD++ * m_darkScale);
                    }
                }//y
                break;
	        default:
                return false;
	    }//switch
	}
	else {
	    switch (m_mapFormat) {
	        case EVF_YUV:
                // YUV - just copy the contents
                sPtr = pBitmap;
                for (y=0; y<m_height*m_width; y++) {
                        *ptrN++ = (DMTYPE)*sPtr++;
                }//y
                // U Plane half width, half height
                for (y=0; y<m_height*m_width/4; y++) {
                        *ptrN++ = (DMTYPE)*sPtr++;
                }//y
                // V Plane half width, half height
                for (y=0; y<m_height*m_width/4; y++) {
                        *ptrN++ = (DMTYPE)*sPtr++;
                }//y

	        break;
	        case EVF_RGB24:
                for (y=0; y<m_height; y++) {
                    for (x=1; x<(m_width*m_pixsize); x+=m_pixsize) {//G
                        *ptrN++ = (DMTYPE)pBitmap[y*m_scanwidth+x];
                    }
                }//y
                for (y=0; y<m_height; y++) {
                    for (x=2; x<(m_width*m_pixsize); x+=m_pixsize) {//R
                        *ptrN++ = (DMTYPE)pBitmap[y*m_scanwidth+x];
                    }
                }//y
                for (y=0; y<m_height; y++) {
                    for (x=0; x<(m_width*m_pixsize); x+=m_pixsize) {//B
                        *ptrN++ = (DMTYPE)pBitmap[y*m_scanwidth+x];
                    }
                }//y
	        break;
	        default:
                return false;
	    }//switch

	}//if, else

	return hr;
}//Bitmap2NewMap



bool DoubleMap::AddBitmap(unsigned char* pBitmap)
{
bool hr = true;
register long i;
DMTYPE* ptr = m_pMap;
DMTYPE* ptrN = m_pNewMap;
//BEGIN
	if ( !(hr = Bitmap2NewMap(pBitmap)) )
		return false;

	// add NewMap to Map
	for (i=0; i<m_size; i++) {
		*ptr++ += *ptrN++;
	}
	m_numSamples += 1.0F; // increment sample counter
	m_dx = 0.0F; m_dy = 0.0F;

	return hr;
}//END AddBitmap


// add a dark image that is subtracted from each added image
// if the map is a NULL ptr dark subtraction is switched off
// dimension must fit otherwise it breaks....
bool DoubleMap::AddDarkMap(unsigned char* pBitmap)
{
bool hr = true;
register long i;
DMTYPE* ptrD = m_pDarkMap;
DMTYPE* ptrN = m_pNewMap;

//BEGIN
	m_darkProcessing = false;
    if (!pBitmap)
		return hr;  // dark processing is switched off

	hr = Bitmap2NewMap(pBitmap);
	if ( !(hr) )
		return hr;

	// copy NewMap to DarkMap
	for (i=0; i<m_size; i++) {
		*ptrD++ = *ptrN++;
	}
	// don't switch it on before loading the NewMap (otherwise it is darkprocessed too)
	m_darkProcessing = true; // finally
	hr = true;

	return hr;
}//AddDarkMap


// merge the stored image with the new one
// the new one weights the factor of the outcome
// the value range of this Map should remain within 0..255 and the sample count = 1
// when the first sample is put in as AddBitmap
bool DoubleMap::MergeBitmap(double factor, unsigned char* pBitmap)
{
bool hr = true;
register long i;
DMTYPE* ptr = m_pMap;
DMTYPE* ptrN = m_pNewMap;
float  f;
//BEGIN
	hr = Bitmap2NewMap(pBitmap);
	if ( !(hr) )
		return hr;

	f = (float)factor;
	f = (f<0.001F) ? 0.001F : f;
	f = (f>1.000F) ? 1.000F : f;

	// add map
	for (i=0; i<m_size; i++) {
		// slight round up due to -0.4 instead of 0.5
//		*ptr = (DMTYPE)ceil( (1.0F - f) * (float)*ptr + (f * (float)*ptrN) - 0.4F);
		*ptr = (DMTYPE)ceil( (1.0F - f) * (float)*ptr + ( (float)*ptrN) - 0.4F);
		m_numSamples = m_numSamples * (1.0F - f) + 1; // increment sample counter
		ptr++; ptrN++;
	}

	return hr;
}//END MergeBitmap



// return the mean value of the stored Map
bool DoubleMap::GetMean(unsigned char* pBitmap)
{
bool hr = true;
register long x,y;
DMTYPE bVal;
DMTYPE* ptr = m_pMap;
//BEGIN
	if (!m_pMap)
		return false;
    if (!pBitmap)
        return false;  // not even a valid pointer

    switch (m_mapFormat) {
        case EVF_YUV:
            for (y=0; y<m_size; y++) {
                    bVal = (DMTYPE)(*ptr / m_numSamples); ptr++;// mean
                    pBitmap[y] = (unsigned char)CLIP(bVal);
            }//y
            break;

        case EVF_RGB24:
            for (y=0; y<m_height; y++) {
                for (x=1; x<m_width*m_pixsize; x+=m_pixsize) {//G
                    bVal = (DMTYPE)(*ptr / m_numSamples); ptr++;// mean
                    pBitmap[y*m_scanwidth+x] = (unsigned char)CLIP(bVal);
                }
            }//y
            for (y=0; y<m_height; y++) {
                for (x=2; x<m_width*m_pixsize; x+=m_pixsize) {//R
                    bVal = (DMTYPE)(*ptr / m_numSamples); ptr++; // mean
                    pBitmap[y*m_scanwidth+x] = (unsigned char)CLIP(bVal);
                }
            }//y
            for (y=0; y<m_height; y++) {
                for (x=0; x<m_width*m_pixsize; x+=m_pixsize) {//B
                    bVal = (DMTYPE)(*ptr / m_numSamples); ptr++; // mean
                    pBitmap[y*m_scanwidth+x] = (unsigned char)CLIP(bVal);
                }
            }//y
            break;
        default:
            ;
    }//switch

	return hr;
}//END GetMean



bool DoubleMap::GetSum(unsigned char* pBitmap)
{
bool hr = true;
//BEGIN
	if (!m_pMap)
		return false;
    if (!pBitmap)
        return false;  // not even a valid pointer

	zz_DumpToBitmap(pBitmap);

	return hr;
}//END GetSum



bool DoubleMap::GetStretched(unsigned char* pBitmap)
{
bool hr = true;
register long x,y;
DMTYPE bVal;
float iScale;
DMTYPE max=0, min=DMTYPE_MAX;
long planeSize = m_width*m_height;
DMTYPE* ptr = m_pMap;

//BEGIN
	if (!m_pMap)
		return false;
    if (!pBitmap)
        return false;  // not even a valid pointer

    switch (m_mapFormat) {
        case EVF_YUV: {
            // get min/max of Y
            for (x=0; x<planeSize; x++) {
                max = MAX(*ptr, max); min = MIN(*ptr, min); ptr++;
            }

            // get the scale factor
            iScale = (max - min) / 254.0F; if (iScale<=0.0) iScale = 1.0F;

            ptr = m_pMap;
            // squeeze the image into the Bitmap
            for (y=0; y<planeSize; y++) {
                    bVal = (DMTYPE)( (*ptr - min) / iScale ); ptr++; // G histogram of stretch
                    pBitmap[y] = CLIP(bVal);
            }//y
            // mean of U and V
            ptr = &m_pMap[planeSize];
            for (y=planeSize; y<m_size; y++) {
                    bVal = (DMTYPE)(*ptr / m_numSamples); ptr++;// mean
                    pBitmap[y] = (unsigned char)CLIP(bVal);
            }//y
            break;
        }
        case EVF_RGB24: {
            // get min/max
            for (x=0; x<m_size; x++) {
                max = MAX(*ptr, max); min = MIN(*ptr, min); ptr++;
            }

            // get the scale factor
            iScale = (max - min) / 254.0F; if (iScale<=0.0) iScale = 1.0F;

            DMTYPE* gPtr = m_pMap;
            DMTYPE* rPtr = m_pMap + planeSize;
            DMTYPE* bPtr = m_pMap + planeSize + planeSize;
            // squeeze the image into the Bitmap
            for (y=0; y<m_height; y++) {
                for (x=0; x<m_width*m_pixsize; x+=m_pixsize) { // each pel
                    bVal = (DMTYPE)( (*gPtr - min) / iScale ); gPtr++; // G histogram of stretch
                    pBitmap[y*m_scanwidth+x+1] = CLIP(bVal);
                    bVal = (DMTYPE)( (*rPtr - min) / iScale ); rPtr++; //R histogram of stretch
                    pBitmap[y*m_scanwidth+x+2] = CLIP(bVal);
                    bVal = (DMTYPE)( (*bPtr - min) / iScale ); bPtr++; //B histogram of stretch
                    pBitmap[y*m_scanwidth+x] = CLIP(bVal);
                }//for
            }//y
            break;
        }
        case EVF_Unknown:
        default:
            ;
    }//switch
	return hr;
}//END GetStretched



bool DoubleMap::GetHistStreched(unsigned char* pBitmap)
{
bool hr = true;
register long x,y;
DMTYPE  bVal;
float iScale;
long  hist[256];
double relHist[256], hScale;
DMTYPE max=0, min=DMTYPE_MAX;
long planeSize = m_width*m_height;
//BEGIN
	if (!m_pMap)
		return false;
    if (!pBitmap)
        return false;  // not even a valid pointer

    switch (m_mapFormat) {
        case EVF_YUV: {
            // reset Y-Map pointers
            DMTYPE* ptr = m_pMap;
            // get min/max of Y
            for (x=0; x<planeSize; x++) {
                max = MAX(*ptr, max); min = MIN(*ptr, min); ptr++;
            }
            iScale = (max - min) / 254.0F;

            if (iScale>0.001) { // not a zero image
                memset(hist, 0, sizeof(hist));
                memset(relHist, 0, sizeof(relHist));

                // reset Y-Map pointer
                ptr = m_pMap;
                // squeeze the image into the Bitmap
                for (y=0; y<planeSize; y++) {
                        bVal = (DMTYPE)( (*ptr - min) / iScale ); ptr++; // G histogram of stretch
                        pBitmap[y] = CLIP(bVal);

                        // calc histogram of the stretched image (range 0 .. 255)
                        bVal = (DMTYPE)pBitmap[y];
                        hist[(int)CLIP(bVal)]++;
                }//y
                // calc the sum of the relative contribution for each histogram bin
                hScale = 1.0 / (double)planeSize; // contribution part of a pixel
                relHist[0] = hist[0] * hScale;  // relative contribution
                for (x=1; x<256; x++) {
                    relHist[x] = relHist[x-1] + hist[x] * hScale;  // summ of relative contribution
                    relHist[x] = (relHist[x]> 1.0) ? (DMTYPE)1 : relHist[x]; // must not exceed 1.0
                }
                // now rescale the bitmap again
                for (y=0; y<planeSize; y++) {
                        bVal = (DMTYPE)(pBitmap[y] * relHist[pBitmap[y]]);
                        pBitmap[y] = CLIP(bVal);
                }//y
                // mean of U and V
                ptr = &m_pMap[planeSize];
                for (y=planeSize; y<m_size; y++) {
                        bVal = (DMTYPE)(*ptr / m_numSamples); ptr++;// mean
                        pBitmap[y] = (unsigned char)CLIP(bVal);
                }//y
            }
            else { // just dump
                zz_DumpToBitmap(pBitmap);
            }
            break;
        }
        case EVF_RGB24: {
            // reset RGB-Map pointers
            DMTYPE* gPtr = m_pMap;
            DMTYPE* rPtr = m_pMap + planeSize;
            DMTYPE* bPtr = m_pMap + planeSize + planeSize;
            // get min/max of the pixel mean i.e. (R+G+B) / 3
            for (x=0; x<planeSize; x++) {
                bVal = (*gPtr++ + *rPtr++ + *bPtr++) / 3;
                max = MAX(bVal, max);  min = MIN(bVal, min);
            }
            iScale = (max - min) / 254.0F;

            if (iScale>0.001) { // not a zero image
                memset(hist, 0, sizeof(hist));
                memset(relHist, 0, sizeof(relHist));

                // reset RGB-Map pointers
                gPtr = m_pMap;
                rPtr = m_pMap + planeSize;
                bPtr = m_pMap + planeSize + planeSize;
                // squeeze the image into the Bitmap
                for (y=0; y<m_height; y++) {
                    for (x=0; x<m_width*m_pixsize; x+=m_pixsize) { // each pel
                        bVal = (DMTYPE)( (*gPtr - min) / iScale ); gPtr++; // G histogram of stretch
                        pBitmap[y*m_scanwidth+x+1] = CLIP(bVal);
                        bVal = (DMTYPE)( (*rPtr - min) / iScale ); rPtr++; //R histogram of stretch
                        pBitmap[y*m_scanwidth+x+2] = CLIP(bVal);
                        bVal = (DMTYPE)( (*bPtr - min) / iScale ); bPtr++; //B histogram of stretch
                        pBitmap[y*m_scanwidth+x] = CLIP(bVal);

                        // calc histogram of the stretched image (range 0 .. 255)
                        bVal = (DMTYPE)((pBitmap[y*m_scanwidth+x]
                                            + pBitmap[y*m_scanwidth+x+1]
                                            + pBitmap[y*m_scanwidth+x+2]) / 3.0);
                        hist[(int)CLIP(bVal)]++;
                    }
                }//y
                // calc the sum of the relative contribution for each histogram bin
                hScale = 1.0 / (double)planeSize; // contribution part of a pixel
                relHist[0] = hist[0] * hScale;  // relative contribution
                for (x=1; x<256; x++) {
                    relHist[x] = relHist[x-1] + hist[x] * hScale;  // summ of relative contribution
                    relHist[x] = (relHist[x]> 1.0) ? (DMTYPE)1 : relHist[x]; // must not exceed 1.0
                }
                // now rescale the bitmap again
                for (y=0; y<m_height; y++) {
                    for (x=0; x<m_width*m_pixsize; x++) {
                        bVal = (DMTYPE)(pBitmap[y*m_scanwidth+x] * relHist[pBitmap[y*m_scanwidth+x]]);
                        pBitmap[y*m_scanwidth+x] = CLIP(bVal);
                    }
                }//y
            }
            else { // just dump
                zz_DumpToBitmap(pBitmap);
            }
            break;
        }
        case EVF_Unknown:
        default:
            ;
    }//switch


	return hr;
}//END GetHistStreched




bool DoubleMap::GetStretched(double histRange, unsigned char* pBitmap)
{
bool hr = true;
register long x,y;
DMTYPE bVal;
float iScale;
DMTYPE max=0, min=DMTYPE_MAX;
long  hist[256]; long topMark = 0;
double relHist[256], hScale;
long planeSize = m_width*m_height;
//BEGIN
	if (!m_pMap)
		return false;
    if (!pBitmap)
        return false;  // not even a valid pointer

    switch (m_mapFormat) {
        case EVF_YUV: {
            // reset Y-Map pointers
            DMTYPE* ptr = m_pMap;
            // get min/max of Y
            for (x=0; x<planeSize; x++) {
                max = MAX(*ptr, max); min = MIN(*ptr, min); ptr++;
            }
            iScale = (max - min) / 254.0F;

            if (iScale>0.001) { // not a zero image
                memset(hist, 0, sizeof(hist));
                memset(relHist, 0, sizeof(relHist));

                // reset Y-Map pointers
                ptr = m_pMap;
                // squeeze the image into the Bitmap
                for (y=0; y<planeSize; y++) {
                        bVal = (DMTYPE)( (*ptr - min) / iScale ); ptr++; // G histogram of stretch
                        pBitmap[y] = CLIP(bVal);

                        // calc histogram of the stretched image (range 0 .. 255)
                        bVal = (DMTYPE)pBitmap[y];
                        hist[(int)CLIP(bVal)]++;
                }//y

                // calc the sum of the relative contribution for each histogram bin
                hScale = 1.0 / (double)planeSize; // contribution part of a pixel
                relHist[0] = hist[0] * hScale;  // relative contribution
                for (x=1; x<256; x++) {
                    relHist[x] = relHist[x-1] + hist[x] * hScale;  // summ of relative contribution
                    relHist[x] = (relHist[x]> 1.0) ? 1.0 : relHist[x]; // must not exceed 1.0
                    // set the topmark where the histogram reaches the given range
                    if ( relHist[x]>= histRange ) {
                        if (topMark==0) topMark = x; // should set only once
                    }
                }
                // now rescale the bitmap again with the new max
                iScale = ((topMark*iScale+min) - min) / 254.0F;
                // reset RGB-Map pointers
                ptr = m_pMap;
                // squeeze the image into the Bitmap
                for (y=0; y<planeSize; y++) {
                        bVal = (DMTYPE)( (*ptr - min) / iScale ); ptr++; // Y histogram of stretch
                        pBitmap[y] = CLIP(bVal);
                }//y

                // mean of U and V
                ptr = &m_pMap[planeSize];
                for (y=planeSize; y<m_size; y++) {
                        bVal = (DMTYPE)(*ptr / m_numSamples); ptr++;// mean
                        pBitmap[y] = (unsigned char)CLIP(bVal);
                }//y
            }
            else { // just dump
                zz_DumpToBitmap(pBitmap);
            }
            break;
        }
        case EVF_RGB24: {
            // reset RGB-Map pointers
            DMTYPE* gPtr = m_pMap;
            DMTYPE* rPtr = m_pMap + planeSize;
            DMTYPE* bPtr = m_pMap + planeSize + planeSize;
            // get min/max of the pixel mean i.e. (R+G+B) / 3
            for (x=0; x<planeSize; x++) {
                bVal = (*gPtr++ + *rPtr++ + *bPtr++) / 3;
                max = MAX(bVal, max);  min = MIN(bVal, min);
            }
            iScale = (max - min) / 254.0F;

            if (iScale>0.001) { // not a zero image
                memset(hist, 0, sizeof(hist));
                memset(relHist, 0, sizeof(relHist));

                // reset RGB-Map pointers
                gPtr = m_pMap;
                rPtr = m_pMap + planeSize;
                bPtr = m_pMap + planeSize + planeSize;
                // squeeze the image into the Bitmap
                for (y=0; y<m_height; y++) {
                    for (x=0; x<m_width*m_pixsize; x+=m_pixsize) { // each pel
                        bVal = (DMTYPE)( (*gPtr - min) / iScale ); gPtr++; // G histogram of stretch
                        pBitmap[y*m_scanwidth+x+1] = CLIP(bVal);
                        bVal = (DMTYPE)( (*rPtr - min) / iScale ); rPtr++; //R histogram of stretch
                        pBitmap[y*m_scanwidth+x+2] = CLIP(bVal);
                        bVal = (DMTYPE)( (*bPtr - min) / iScale ); bPtr++; //B histogram of stretch
                        pBitmap[y*m_scanwidth+x] = CLIP(bVal);

                        // calc histogram of the stretched image (range 0 .. 255)
                        bVal = (DMTYPE)((pBitmap[y*m_scanwidth+x]
                                            + pBitmap[y*m_scanwidth+x+1]
                                            + pBitmap[y*m_scanwidth+x+2]) / 3.0);
                        hist[(int)CLIP(bVal)]++;
                    }
                }//y

                // calc the sum of the relative contribution for each histogram bin
                hScale = 1.0 / (double)planeSize; // contribution part of a pixel
                relHist[0] = hist[0] * hScale;  // relative contribution
                for (x=1; x<256; x++) {
                    relHist[x] = relHist[x-1] + hist[x] * hScale;  // summ of relative contribution
                    relHist[x] = (relHist[x]> 1.0) ? 1.0 : relHist[x]; // must not exceed 1.0
                    // set the topmark where the histogram reaches the given range
                    if ( relHist[x]>= histRange ) {
                        if (topMark==0) topMark = x; // should set only once
                    }
                }
                // now rescale the bitmap again with the new max
                iScale = ((topMark*iScale+min) - min) / 254.0F;
                // reset RGB-Map pointers
                gPtr = m_pMap;
                rPtr = m_pMap + planeSize;
                bPtr = m_pMap + planeSize + planeSize;
                // squeeze the image into the Bitmap
                for (y=0; y<m_height; y++) {
                    for (x=0; x<m_width*m_pixsize; x+=m_pixsize) { // each pel
                        bVal = (DMTYPE)( (*gPtr - min) / iScale ); gPtr++; // G histogram of stretch
                        pBitmap[y*m_scanwidth+x+1] = CLIP(bVal);
                        bVal = (DMTYPE)( (*rPtr - min) / iScale ); rPtr++; //R histogram of stretch
                        pBitmap[y*m_scanwidth+x+2] = CLIP(bVal);
                        bVal = (DMTYPE)( (*bPtr - min) / iScale ); bPtr++; //B histogram of stretch
                        pBitmap[y*m_scanwidth+x] = CLIP(bVal);
                    }
                }//y
            }
            else { // just dump
                zz_DumpToBitmap(pBitmap);
            }
            break;
        }
        case EVF_Unknown:
        default:
            ;
    }//switch



	return hr;
}//END GetStretched


// Implements a GammaLog response
bool DoubleMap::GetLog(unsigned char* pBitmap)
{
bool hr = true;
register long x,y;
DMTYPE bVal;
float logMax = (float)log(255.0);
float gammaI = (float)(1.0 / 0.35); // Gamma
float iScale;
DMTYPE max=0, min=DMTYPE_MAX;
DMTYPE* ptr = m_pMap;
long planeSize = m_width*m_height;
//BEGIN
	if (!m_pMap)
		return false;
    if (!pBitmap)
        return false;  // not even a valid pointer

    switch (m_mapFormat) {
        case EVF_YUV: {
            // reset Y-Map pointer
            DMTYPE *ptr = m_pMap;
            // get min/max of the pixel mean i.e. (R+G+B) / 3
            for (x=0; x<planeSize; x++) {
                bVal = *ptr++;
                max = MAX(bVal, max);  min = MIN(bVal, min);
            }
            iScale = (max - min) / (float)254.0;

            // reset Y-Map pointer
            ptr = m_pMap;
            if (iScale>0.001) { // not a zero image
                for (y=0; y<planeSize; y++) {
                        bVal = (DMTYPE)( (*ptr - min) / iScale ); ptr++; // stretch
                        bVal = (DMTYPE)(255.0F * (float)pow( (float)log(bVal + 1.0) / logMax, gammaI)); // gammaLog
                        pBitmap[y] = CLIP(bVal);
                }//y

                // mean of U and V
                ptr = &m_pMap[planeSize];
                for (y=planeSize; y<m_size; y++) {
                        bVal = (DMTYPE)(*ptr / m_numSamples); ptr++;// mean
                        pBitmap[y] = (unsigned char)CLIP(bVal);
                }//y
            }
            else { // just dump
                zz_DumpToBitmap(pBitmap);
            }
            break;
        }
        case EVF_RGB24: {
            // reset RGB-Map pointers
            DMTYPE *gPtr = m_pMap;
            DMTYPE *rPtr = m_pMap + planeSize;
            DMTYPE *bPtr = m_pMap + planeSize + planeSize;
            // get min/max of the pixel mean i.e. (R+G+B) / 3
            for (x=0; x<planeSize; x++) {
                bVal = (*gPtr++ + *rPtr++ + *bPtr++) / 3;
                max = MAX(bVal, max);  min = MIN(bVal, min);
            }
            iScale = (max - min) / (float)254.0;

            if (iScale>0.001) { // not a zero image
                for (y=0; y<m_height; y++) {
                    for (x=1; x<m_width*m_pixsize; x+=m_pixsize) { //G
                        bVal = (DMTYPE)( (*ptr - min) / iScale ); ptr++; // stretch
                        bVal = (DMTYPE)(255.0F * (float)pow( (float)log(bVal + 1.0) / logMax, gammaI)); // gammaLog
                        pBitmap[y*m_scanwidth+x] = CLIP(bVal);
                    }
                }//y
                for (y=0; y<m_height; y++) {
                    for (x=2; x<m_width*m_pixsize; x+=m_pixsize) { //R
                        bVal = (DMTYPE)( (*ptr - min) / iScale ); ptr++; // stretch
                        bVal = (DMTYPE)(255.0F * (float)pow( (float)log(bVal + 1.0) / logMax, gammaI)); // gammaLog
                        pBitmap[y*m_scanwidth+x] = CLIP(bVal);
                    }
                }//y
                for (y=0; y<m_height; y++) {
                    for (x=0; x<m_width*m_pixsize; x+=m_pixsize) { //B
                        bVal = (DMTYPE)( (*ptr - min) / iScale ); ptr++; // stretch
                        bVal = (DMTYPE)(255.0F * (float)pow( (float)log(bVal + 1.0) / logMax, gammaI)); // gammaLog
                        pBitmap[y*m_scanwidth+x] = CLIP(bVal);
                    }
                }//y
            }
            else { // just dump
                zz_DumpToBitmap(pBitmap);
            }
            break;
        }
        case EVF_Unknown:
        default:
        ;
    }//switch

	return hr;
}//END GetLog



bool DoubleMap::GetMapData(long* mapSize, DMTYPE* *pBitmap)
{
	*mapSize = m_size;
	*pBitmap = m_pMap;
	return true;
}//END GetMapPtr

long DoubleMap::GetNumSamples()
{
	return (long)m_numSamples;
}//END GetNumSamples


// just copy the contents of the Map into the bitmap
void DoubleMap::zz_DumpToBitmap(unsigned char* pBitmap)
{
register int x,y;
DMTYPE* ptr = m_pMap;

//BEGIN
    switch (m_mapFormat) {
        case EVF_YUV:
            for (y=0; y<m_size; y++) {
                pBitmap[y] = CLIP(*ptr); ptr++;
            }//y
            break;
        case EVF_RGB24:
            for (y=0; y<m_height; y++) {
                for (x=1; x<m_width*m_pixsize; x+=m_pixsize) { //G
                    pBitmap[y*m_scanwidth+x] = CLIP(*ptr); ptr++;
                }
            }//y
            for (y=0; y<m_height; y++) {
                for (x=2; x<m_width*m_pixsize; x+=m_pixsize) { //R
                    pBitmap[y*m_scanwidth+x] = CLIP(*ptr); ptr++;
                }
            }//y
            for (y=0; y<m_height; y++) {
                for (x=0; x<m_width*m_pixsize; x+=m_pixsize) { //B
                    pBitmap[y*m_scanwidth+x] = CLIP(*ptr); ptr++;
                }
            }//y
            break;
        default:
            return;
    }//switch
}//zz_DumpToBitmap

#endif // __UNIX__

