

#ifndef YUV_TO_RGB_H
#define YUV_TO_RGB_H

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

struct ov511_frame {
  //	int framenum;		/* Index of this frame */
  //	wxUint8 *data;	/* Frame buffer */
  //	wxUint8 *tempdata; /* Temp buffer for multi-stage conversions */
  //    wxUint8 *rawdata;	/* Raw camera data buffer */
  //    wxUint8 *compbuf;	/* Temp buffer for decompressor */

  //	int depth;		/* Bytes per pixel */
	int width;		/* Width application is expecting */
	int height;		/* Height application is expecting */

  	int rawwidth;		/* Actual width of frame sent from camera */
  	int rawheight;		/* Actual height of frame sent from camera */

   //	int sub_flag;		/* Sub-capture mode for this frame? */
  //	 unsigned int format;	/* Format for this frame */
   //	int compressed;		/* Is frame compressed? */

   //	volatile int grabstate;	/* State of grabbing */
  //	 int scanstate;		/* State of scanning */

   //	int bytes_recvd;	/* Number of image bytes received from camera */

   //	long bytes_read;	/* Amount that has been read() */

   //	wait_queue_head_t wq;	/* Processes waiting */

   //	int snapshot;		/* True if frame was a snapshot */
 };

/* Converts from YUV420 raw to planar YUV420 */
void
yuv420raw_to_yuv420p(struct ov511_frame *frame,
		     wxUint8 *pIn0, wxUint8 *pOut0);

/* Converts from planar YUV420 to RGB24. */
void
yuv420p_to_rgb(struct ov511_frame *frame,
	       wxUint8 *pIn0, wxUint8 *pOut0, int bits);

#endif  //YUV_TO_RGB_H
