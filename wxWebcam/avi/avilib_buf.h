
#ifndef _AVILIB_BUF_H__
#define _AVILIB_BUF_H__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// BM:20090806 - buffered AVILIB
// forward declare so we don't have to expose its internals
class avi_handle_buf;
class CAviLib_Buf {
public:
	CAviLib_Buf();
	virtual ~CAviLib_Buf();

	enum EVideoFormat { /*** are only supported */
		VIDEO_NONE          = 0,
//		VIDEO_RGB08         = 1,  /* bt848 dithered */
//		VIDEO_GRAY          = 2,
		VIDEO_RGB15_LE      = 3,  /* 15 bpp little endian */
		VIDEO_RGB16_LE      = 4,  /* 16 bpp little endian */
		VIDEO_RGB15_BE      = 5,  /* 15 bpp big endian */
		VIDEO_RGB16_BE      = 6,  /* 16 bpp big endian */
		VIDEO_BGR24_TD      = 7,  /*** bgrbgrbgrbgr (LE) topdown also RGB type*/
		VIDEO_BGR24_BU      = 8,  /*** bgrbgrbgrbgr (LE) bottomup also RGB type*/
//		VIDEO_BGR32         = 9,  /* bgr-bgr-bgr- (LE) */
//		VIDEO_RGB24         =10,  /* rgbrgbrgbrgb (BE) */
//		VIDEO_RGB32         =11,  /* -rgb-rgb-rgb (BE) */
//		VIDEO_LUT2          =12,  /* lookup-table 2 byte depth */
//		VIDEO_LUT4          =13,  /* lookup-table 4 byte depth */
		VIDEO_YUYV_TD       =14,  /*** 4:2:2 */
		VIDEO_YVYU_TD       =15,  /*** 4:2:2 */
//		VIDEO_YUV422P       =16,  /* YUV 4:2:2 (planar) */
		VIDEO_YUV420_TD     =17,  /*** YUV 4:2:0 (planar)  topdown */
		VIDEO_YUV420_BU     =18,  /*** YUV 4:2:0 (planar) bottomup */
//		VIDEO_MJPEG			=19,  /* MJPEG (AVI) */
//		VIDEO_JPEG			=20,  /* JPEG (JFIF) */
		VIDEO_UYVY_TD       =21,  /*** 4:2:2 */
		VIDEO_Y800_TD       =22,  /*** 8bit planar */
		VIDEO_FMT_COUNT     =23,
#if BYTE_ORDER == BIG_ENDIAN
		VIDEO_RGB15_NATIVE  = VIDEO_RGB15_BE,
		VIDEO_RGB16_NATIVE  = VIDEO_RGB16_BE,
#else
		VIDEO_RGB15_NATIVE  = VIDEO_RGB15_LE,
		VIDEO_RGB16_NATIVE  = VIDEO_RGB16_LE,
#endif
#ifdef __UNIX__
		VIDEO_RGB24_NATIVE   = VIDEO_BGR24_BU,
		VIDEO_YUV420_NATIVE  = VIDEO_YUV420_TD,
		VIDEO_YUYV_NATIVE    = VIDEO_YUYV_TD,
		VIDEO_YVYU_NATIVE    = VIDEO_YVYU_TD,
		VIDEO_UYVY_NATIVE    = VIDEO_UYVY_TD,
		VIDEO_Y800_NATIVE    = VIDEO_Y800_TD,
#else
//		VIDEO_RGB24_NATIVE   = VIDEO_BGR24_TD, // Win RGB24 is now returned like the Linux one
		VIDEO_RGB24_NATIVE   = VIDEO_BGR24_BU,
		VIDEO_YUV420_NATIVE  = VIDEO_YUV420_TD,
#endif
	};

	// the format descriptor
	struct video_fmt {
		EVideoFormat    fmtid;         /* VIDEO_* */
		unsigned int    width;
		unsigned int    height;
	};

	// a videobuffer to submit
	struct video_buf {
		size_t          size;
		const wxUint8   *data;
	};


	// Opens an AVI file
	// return true if success (fails if file open already)
	// params: filename, a video descriptor, the fcc type, the fps rate*30
	// fps are given in multiples of 10
	// i.e. 1 frame per second -> fp10s = 10
	//      1 frame every 10 seconds -> fp10s = 1
	bool avi_open(const wxString &filename, const video_fmt &video, int fp10s);

	// write a Frame Chunk
	// return 0 for success
	int avi_video(const video_buf &buf);

	// Close the AVI file
	// write the Index and rewrite the header with final size data
	// return 0 for success
	int avi_close();

private:

	// some internals
	void avi_addindex(avi_handle_buf *h, char *fourcc, int flags, int chunksize);
	void avi_writeindex(avi_handle_buf *h);
	void avi_bigfile(avi_handle_buf *h, int last);
	void avi_write_header(avi_handle_buf *h);

private:
    avi_handle_buf* m_handle; // NULL when file not open
};

class CAviBuffer_Buf
{
public:

    CAviBuffer_Buf(long p_width, long p_height);
    virtual ~CAviBuffer_Buf();
    void SetRGB();
    void SetYUV();
    void SetYUYV();
    void SetYVYU();
    void SetUYVY();
    void SetY800();
    CAviLib_Buf::video_fmt  m_videoFmt;
    CAviLib_Buf::video_buf  m_videoBuffer;

};

#endif // _AVILIB_BUF_H__
