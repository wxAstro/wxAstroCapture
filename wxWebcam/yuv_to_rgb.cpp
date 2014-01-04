/*
 * yuv_to_rgb.c
 *
 * Most of the code that follows was taken from the OV511 driver.
 *
 * Copyright (c) 1999-2004 Mark W. McClelland
 * Support for OV519, OV8610 Copyright (c) 2003 Joerg Heckenbach
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 */

/*
   Carsten A. Arnholm, 05 April 2007
   * commented out  #include <linux/videodev.h>
   * commented out  #include get_depth function
   * changed force_rgb default to be 1 as it gave proper colour with Philips ToUcam
   * added #pragma for MSVC compiler to remove warning C4244
*/

#define OV511_ALLOW_CONVERSION

//#include <linux/videodev.h>

#include "yuv_to_rgb.h"

/* Pixel count * 3 bytes for RGB */
#define MAX_FRAME_SIZE(w, h) ((w) * (h) * 3)
/* Pixel count * bytes per YUV420 pixel (1.5) */
//#define MAX_FRAME_SIZE(w, h) ((w) * (h) * 3 / 2)

//static int fix_rgb_offset = 0;
static int force_rgb = 1;

#if defined(_WXMSW_ )
    // warning C4244: '=' : conversion from 'int' to 'wxUint8', possible loss of data
    #pragma warning (disable : 4244)
#endif

/* Returns number of bits per pixel (regardless of where they are located;
 * planar or not), or zero for unsupported format.
 */

/*
static inline int
get_depth(int palette)
{
	switch (palette) {
	case VIDEO_PALETTE_GREY:    return 8;
	case VIDEO_PALETTE_YUV420:  return 12;
	case VIDEO_PALETTE_YUV420P: return 12; // Planar
	case VIDEO_PALETTE_RGB565:  return 16;
	case VIDEO_PALETTE_RGB24:   return 24;
	case VIDEO_PALETTE_YUV422:  return 16;
	case VIDEO_PALETTE_YUYV:    return 16;
	case VIDEO_PALETTE_YUV422P: return 16; // Planar
	default:		    return 0;  // Invalid format
	}
}
*/

/**********************************************************************
 *
 * Color correction functions
 *
 **********************************************************************/

/* Copies a 64-byte segment at pIn to an 8x8 block at pOut. The width of the
 * image at pOut is specified by w.
 */
static inline void
make_8x8(wxUint8 *pIn, wxUint8 *pOut, int w)
{
	wxUint8 *pOut1 = pOut;
	int x, y;

	for (y = 0; y < 8; y++) {
		pOut1 = pOut;
		for (x = 0; x < 8; x++) {
			*pOut1++ = *pIn++;
		}
		pOut += w;
	}
}


/*
 * Turn a YUV4:2:0 block into an RGB block
 *
 * Video4Linux seems to use the blue, green, red channel
 * order convention-- rgb[0] is blue, rgb[1] is green, rgb[2] is red.
 *
 * Color space conversion coefficients taken from the excellent
 * http://www.inforamp.net/~poynton/ColorFAQ.html
 * In his terminology, this is a CCIR 601.1 YCbCr -> RGB.
 * Y values are given for all 4 pixels, but the U (Pb)
 * and V (Pr) are assumed constant over the 2x2 block.
 *
 * To avoid floating point arithmetic, the color conversion
 * coefficients are scaled into 16.16 fixed-point integers.
 * They were determined as follows:
 *
 *	double brightness = 1.0;  (0->black; 1->full scale)
 *	double saturation = 1.0;  (0->greyscale; 1->full color)
 *	double fixScale = brightness * 256 * 256;
 *	int rvScale = (int)(1.402 * saturation * fixScale);
 *	int guScale = (int)(-0.344136 * saturation * fixScale);
 *	int gvScale = (int)(-0.714136 * saturation * fixScale);
 *	int buScale = (int)(1.772 * saturation * fixScale);
 *	int yScale = (int)(fixScale);
 */

/* LIMIT: convert a 16.16 fixed-point value to a byte, with clipping. */
#define LIMIT(x) ((x)>0xffffff?0xff: ((x)<=0xffff?0:((x)>>16)))

static inline void
move_420_block(int yTL, int yTR, int yBL, int yBR, int u, int v,
	       int rowPixels, wxUint8 * rgb, int bits)
{
	const int rvScale = 91881;
	const int guScale = -22553;
	const int gvScale = -46801;
	const int buScale = 116129;
	const int yScale  = 65536;
	int r, g, b;

	g = guScale * u + gvScale * v;
	if (force_rgb) {
		r = buScale * u;
		b = rvScale * v;
	} else {
		r = rvScale * v;
		b = buScale * u;
	}

	yTL *= yScale; yTR *= yScale;
	yBL *= yScale; yBR *= yScale;

	if (bits == 24) {
		/* Write out top two pixels */
		rgb[0] = LIMIT(b+yTL); rgb[1] = LIMIT(g+yTL);
		rgb[2] = LIMIT(r+yTL);

		rgb[3] = LIMIT(b+yTR); rgb[4] = LIMIT(g+yTR);
		rgb[5] = LIMIT(r+yTR);

		/* Skip down to next line to write out bottom two pixels */
		rgb += 3 * rowPixels;
		rgb[0] = LIMIT(b+yBL); rgb[1] = LIMIT(g+yBL);
		rgb[2] = LIMIT(r+yBL);

		rgb[3] = LIMIT(b+yBR); rgb[4] = LIMIT(g+yBR);
		rgb[5] = LIMIT(r+yBR);
	} else if (bits == 16) {
		/* Write out top two pixels */
		rgb[0] = ((LIMIT(b+yTL) >> 3) & 0x1F)
			| ((LIMIT(g+yTL) << 3) & 0xE0);
		rgb[1] = ((LIMIT(g+yTL) >> 5) & 0x07)
			| (LIMIT(r+yTL) & 0xF8);

		rgb[2] = ((LIMIT(b+yTR) >> 3) & 0x1F)
			| ((LIMIT(g+yTR) << 3) & 0xE0);
		rgb[3] = ((LIMIT(g+yTR) >> 5) & 0x07)
			| (LIMIT(r+yTR) & 0xF8);

		/* Skip down to next line to write out bottom two pixels */
		rgb += 2 * rowPixels;

		rgb[0] = ((LIMIT(b+yBL) >> 3) & 0x1F)
			| ((LIMIT(g+yBL) << 3) & 0xE0);
		rgb[1] = ((LIMIT(g+yBL) >> 5) & 0x07)
			| (LIMIT(r+yBL) & 0xF8);

		rgb[2] = ((LIMIT(b+yBR) >> 3) & 0x1F)
			| ((LIMIT(g+yBR) << 3) & 0xE0);
		rgb[3] = ((LIMIT(g+yBR) >> 5) & 0x07)
			| (LIMIT(r+yBR) & 0xF8);
	}
}

#ifdef NOT_USED
/*
 * fixFrameRGBoffset--
 * My camera seems to return the red channel about 1 pixel
 * low, and the blue channel about 1 pixel high. After YUV->RGB
 * conversion, we can correct this easily. OSL 2/24/2000.
 */
static void
fixFrameRGBoffset(struct ov511_frame *frame)
{
	int x, y;
	int rowBytes = frame->width*3, w = frame->width;
	wxUint8 *rgb = frame->data;
	const int shift = 1;  /* Distance to shift pixels by, vertically */

	/* Don't bother with little images */
	if (frame->width < 400)
		return;

	/* This only works with RGB24 */
	if (frame->format != VIDEO_PALETTE_RGB24)
		return;

	/* Shift red channel up */
	for (y = shift; y < frame->height; y++)	{
		int lp = (y-shift)*rowBytes;     /* Previous line offset */
		int lc = y*rowBytes;             /* Current line offset */
		for (x = 0; x < w; x++)
			rgb[lp+x*3+2] = rgb[lc+x*3+2]; /* Shift red up */
	}

	/* Shift blue channel down */
	for (y = frame->height-shift-1; y >= 0; y--) {
		int ln = (y + shift) * rowBytes;  /* Next line offset */
		int lc = y * rowBytes;            /* Current line offset */
		for (x = 0; x < w; x++)
			rgb[ln+x*3+0] = rgb[lc+x*3+0]; /* Shift blue down */
	}
}
#endif

/**********************************************************************
 *
 * Format conversion
 *
 **********************************************************************/


/*
 * For YUV 4:2:0 images, the data show up in 384 byte segments.
 * The first 64 bytes of each segment are U, the next 64 are V.  The U and
 * V are arranged as follows:
 *
 *      0  1 ...  7
 *      8  9 ... 15
 *           ...
 *     56 57 ... 63
 *
 * U and V are shipped at half resolution (1 U,V sample -> one 2x2 block).
 *
 * The next 256 bytes are full resolution Y data and represent 4 squares
 * of 8x8 pixels as follows:
 *
 *      0  1 ...  7    64  65 ...  71   ...  192 193 ... 199
 *      8  9 ... 15    72  73 ...  79        200 201 ... 207
 *           ...              ...                    ...
 *     56 57 ... 63   120 121 ... 127   ...  248 249 ... 255
 *
 * Note that the U and V data in one segment represent a 16 x 16 pixel
 * area, but the Y data represent a 32 x 8 pixel area. If the width is not an
 * even multiple of 32, the extra 8x8 blocks within a 32x8 block belong to the
 * next horizontal stripe.
 *
 * If dumppix module param is set, _parse_data just dumps the incoming segments,
 * verbatim, in order, into the frame. When used with vidcat -f ppm -s 640x480
 * this puts the data on the standard output and can be analyzed with the
 * parseppm.c utility I wrote.  That's a much faster way for figuring out how
 * these data are scrambled.
 */

/* Converts from raw, uncompressed segments at pIn0 to a YUV420P frame at pOut0.
 *
 * FIXME: Currently only handles width and height that are multiples of 16
 */
void
yuv420raw_to_yuv420p(struct ov511_frame *frame,
		     wxUint8 *pIn0, wxUint8 *pOut0)
{
	int k, x, y;
	wxUint8 *pIn, *pOut, *pOutLine;
	const unsigned int a = frame->rawwidth * frame->rawheight;
	const unsigned int w = frame->rawwidth / 2;

	/* Copy U and V */
	pIn = pIn0;
	pOutLine = pOut0 + a;
	for (y = 0; y < frame->rawheight - 1; y += 16) {
		pOut = pOutLine;
		for (x = 0; x < frame->rawwidth - 1; x += 16) {
			make_8x8(pIn, pOut, w);
			make_8x8(pIn + 64, pOut + a/4, w);
			pIn += 384;
			pOut += 8;
		}
		pOutLine += 8 * w;
	}

	/* Copy Y */
	pIn = pIn0 + 128;
	pOutLine = pOut0;
	k = 0;
	for (y = 0; y < frame->rawheight - 1; y += 8) {
		pOut = pOutLine;
		for (x = 0; x < frame->rawwidth - 1; x += 8) {
			make_8x8(pIn, pOut, frame->rawwidth);
			pIn += 64;
			pOut += 8;
			if ((++k) > 3) {
				k = 0;
				pIn += 128;
			}
		}
		pOutLine += 8 * frame->rawwidth;
	}
}


void
yuv420p_to_rgb(struct ov511_frame *frame,
	       wxUint8 *pIn0, wxUint8 *pOut0, int bits)
{
	const int numpix = frame->width * frame->height;
	const int bytes = bits >> 3;
	int i, j, y00, y01, y10, y11, u, v;
	wxUint8 *pY = pIn0;
	wxUint8 *pU = pY + numpix;
	wxUint8 *pV = pU + numpix / 4;
	wxUint8 *pOut = pOut0;

	for (j = 0; j <= frame->height - 2; j += 2) {
		for (i = 0; i <= frame->width - 2; i += 2) {
			y00 = *pY;
			y01 = *(pY + 1);
			y10 = *(pY + frame->width);
			y11 = *(pY + frame->width + 1);
			u = (*pU++) - 128;
			v = (*pV++) - 128;

			move_420_block(y00, y01, y10, y11, u, v,
				       frame->width, pOut, bits);

			pY += 2;
			pOut += 2 * bytes;
		}
		pY += frame->width;
		pOut += frame->width * bytes;
	}
}

#ifdef NOT_USED
/* Converts from planar YUV420 to YUV422 (YUYV). */
static void
yuv420p_to_yuv422(struct ov511_frame *frame,
		  wxUint8 *pIn0, wxUint8 *pOut0)
{
	const int numpix = frame->width * frame->height;
	int i, j;
	wxUint8 *pY = pIn0;
	wxUint8 *pU = pY + numpix;
	wxUint8 *pV = pU + numpix / 4;
	wxUint8 *pOut = pOut0;

	for (i = 0; i < numpix; i++) {
		*pOut = *(pY + i);
		pOut += 2;
	}

	pOut = pOut0 + 1;
	for (j = 0; j <= frame->height - 2 ; j += 2) {
		for (i = 0; i <= frame->width - 2; i += 2) {
			int u = *pU++;
			int v = *pV++;

			*pOut = u;
			*(pOut+2) = v;
			*(pOut+frame->width*2) = u;
			*(pOut+frame->width*2+2) = v;
			pOut += 4;
		}
		pOut += (frame->width * 2);
	}
}
#endif

#ifdef NOT_USED
/* Converts pData from planar YUV420 to planar YUV422 **in place**. */
static void
yuv420p_to_yuv422p(struct ov511_frame *frame, wxUint8 *pData)
{
	const int numpix = frame->width * frame->height;
	const int w = frame->width;
	int j;
	wxUint8 *pIn, *pOut;

	/* Clear U and V */
	memset(pData + numpix + numpix / 2, 127, numpix / 2);

	/* Convert V starting from beginning and working forward */
	pIn = pData + numpix + numpix / 4;
	pOut = pData + numpix +numpix / 2;
	for (j = 0; j <= frame->height - 2; j += 2) {
		memmove(pOut, pIn, w/2);
		memmove(pOut + w/2, pIn, w/2);
		pIn += w/2;
		pOut += w;
	}

	/* Convert U, starting from end and working backward */
	pIn = pData + numpix + numpix / 4;
	pOut = pData + numpix + numpix / 2;
	for (j = 0; j <= frame->height - 2; j += 2) {
		pIn -= w/2;
		pOut -= w;
		memmove(pOut, pIn, w/2);
		memmove(pOut + w/2, pIn, w/2);
	}
}
#endif

#ifdef NOT_USED
#ifdef OV511_ALLOW_CONVERSION
/* Process raw YUV420 data into the format requested by the app. Conversion
 * between V4L formats is allowed.
 */
static void
ov51x_postprocess_yuv420(struct usb_ov511 *ov, struct ov511_frame *frame)
{
	/* Process frame->rawdata to frame->tempdata */
	if (frame->compressed)
		decompress(ov, frame, frame->rawdata, frame->tempdata);
	else
		yuv420raw_to_yuv420p(frame, frame->rawdata, frame->tempdata);

	/* Deinterlace frame, if necessary */
	if (ov->sensor == SEN_SAA7111A && frame->rawheight >= 480) {
		memcpy(frame->rawdata, frame->tempdata,
			MAX_RAW_DATA_SIZE(frame->width, frame->height));
		deinterlace(frame, RAWFMT_YUV420, frame->rawdata,
		            frame->tempdata);
	}

	/* Frame should be (width x height) and not (rawwidth x rawheight) at
         * this point. */

	/* Process frame->tempdata to frame->data */
	switch (frame->format) {
	case VIDEO_PALETTE_RGB565:
		yuv420p_to_rgb(frame, frame->tempdata, frame->data, 16);
		break;
	case VIDEO_PALETTE_RGB24:
		yuv420p_to_rgb(frame, frame->tempdata, frame->data, 24);
		break;
	case VIDEO_PALETTE_YUV422:
	case VIDEO_PALETTE_YUYV:
		yuv420p_to_yuv422(frame, frame->tempdata, frame->data);
		break;
	case VIDEO_PALETTE_YUV420:
	case VIDEO_PALETTE_YUV420P:
		memcpy(frame->data, frame->tempdata,
			MAX_RAW_DATA_SIZE(frame->width, frame->height));
		break;
	case VIDEO_PALETTE_YUV422P:
		/* Data is converted in place, so copy it in advance */
		memcpy(frame->data, frame->tempdata,
			MAX_RAW_DATA_SIZE(frame->width, frame->height));

		yuv420p_to_yuv422p(frame, frame->data);
		break;
	default:
		err("Cannot convert YUV420 to %s",
		    symbolic(v4l1_plist, frame->format));
	}

	if (fix_rgb_offset)
		fixFrameRGBoffset(frame);
}

#else /* if conversion not allowed */

/* Process raw YUV420 data into standard YUV420P */
static void
ov51x_postprocess_yuv420(struct usb_ov511 *ov, struct ov511_frame *frame)
{
	/* Deinterlace frame, if necessary */
	if (ov->sensor == SEN_SAA7111A && frame->rawheight >= 480) {
		if (frame->compressed)
			decompress(ov, frame, frame->rawdata, frame->tempdata);
		else
			yuv420raw_to_yuv420p(frame, frame->rawdata,
					     frame->tempdata);

		deinterlace(frame, RAWFMT_YUV420, frame->tempdata,
		            frame->data);
	} else {
		if (frame->compressed)
			decompress(ov, frame, frame->rawdata, frame->data);
		else
			yuv420raw_to_yuv420p(frame, frame->rawdata,
					     frame->data);
	}
}
#endif /* OV511_ALLOW_CONVERSION */
#endif

#ifdef NOT_USED
/* Post-processes the specified frame. This consists of:
 * 	1. Decompress frame, if necessary
 *	2. Deinterlace frame and scale to proper size, if necessary
 * 	3. Convert from YUV planar to destination format, if necessary
 * 	4. Fix the RGB offset, if necessary
 */
static void
ov51x_postprocess(struct usb_ov511 *ov, struct ov511_frame *frame)
{
	if (dumppix) {
		memset(frame->data, 0,
			MAX_DATA_SIZE(ov->maxwidth, ov->maxheight));
		PDEBUG(4, "Dumping %d bytes", frame->bytes_recvd);
		memcpy(frame->data, frame->rawdata, frame->bytes_recvd);
	} else {
		switch (frame->format) {
		case VIDEO_PALETTE_GREY:
			ov51x_postprocess_grey(ov, frame);
			break;
		case VIDEO_PALETTE_YUV420:
		case VIDEO_PALETTE_YUV420P:
#ifdef OV511_ALLOW_CONVERSION
		case VIDEO_PALETTE_RGB565:
		case VIDEO_PALETTE_RGB24:
		case VIDEO_PALETTE_YUV422:
		case VIDEO_PALETTE_YUYV:
		case VIDEO_PALETTE_YUV422P:
#endif
			ov51x_postprocess_yuv420(ov, frame);
			break;
		default:
			err("Cannot convert data to %s",
			    symbolic(v4l1_plist, frame->format));
		}
	}
}
#endif
