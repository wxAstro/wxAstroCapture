
// BM:20090806 - buffered AVILIB

#include "avilib.h"

/// //////////////////////////////////////////
/// switch to use buffered IO
#ifdef AVIBUFFERED_IO

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/wfstream.h>

#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#ifdef __UNIX__
#include <unistd.h>
#ifndef O_BINARY
#define O_BINARY 0
#endif
#endif

#if defined(_WXMSW_ )
#include <io.h>
#pragma warning (disable:4996) // This function or variable may be unsafe.
#endif


#include "inttypes.h"
#include "byteswap.h"
#include "riff.h"  // get those AVI RIFF strucs

CAviBuffer::CAviBuffer(long p_width, long p_height)
{
    m_videoFmt.width = p_width;
    m_videoFmt.height = p_height;
    m_videoFmt.fmtid = CAviLib::VIDEO_NONE;

    m_videoBuffer.size = 0;
    m_videoBuffer.data = NULL;
}
CAviBuffer::~CAviBuffer()
{
}

void CAviBuffer::SetRGB() {
    m_videoFmt.fmtid=CAviLib::VIDEO_RGB24_NATIVE;
};
void CAviBuffer::SetYUV() {
    m_videoFmt.fmtid=CAviLib::VIDEO_YUV420_NATIVE;
};
void CAviBuffer::SetYUY2() {
    m_videoFmt.fmtid=CAviLib::VIDEO_YUY2_NATIVE;
};
void CAviBuffer::SetYVYU() {
    m_videoFmt.fmtid=CAviLib::VIDEO_YVYU_NATIVE;
};
void CAviBuffer::SetUYVY() {
    m_videoFmt.fmtid=CAviLib::VIDEO_UYVY_NATIVE;
};
void CAviBuffer::SetY800() {
    m_videoFmt.fmtid=CAviLib::VIDEO_Y800_NATIVE;
};



class avi_handle {
public:
	/* file name+handle */
	char   file[MAXPATHLEN];
	wxFileOutputStream*        fileOutStream;
	wxBufferedOutputStream*    bufOutStream;

	iovec *vec;

	/* format */
	CAviLib::video_fmt video;

	/* headers */
	AVI_HDR        avi_hdr;
	AVIX_HDR       avix_hdr;
	AVI_HDR_ODML   avi_hdr_odml;
	AVI_HDR_VIDEO  avi_hdr_video;
	AVI_DATA       avi_data;
	CHUNK_HDR      frame_hdr;
	CHUNK_HDR      idx_hdr;

	/* statistics -- first chunk */
	int    frames;		// adds up the number of frames
	off_t  hdr_size;	// adds up the size of all chunks
	off_t  data_size;	// adds up the size of data

	/* statistics -- current chunk */
	int    bigfile;
	int    framesx;
	off_t  avix_start;
	off_t  datax_size;

	/* statistics -- total */
	int    frames_total;

	/* frame index */
	struct IDX_RECORD *idx_array;
	int    idx_index, idx_count;
	off_t  idx_offset;
	off_t  idx_size;

	avi_handle();
};
avi_handle::avi_handle()
: fileOutStream(NULL), bufOutStream(NULL), vec(NULL), idx_array(NULL) {};


// code derived from xawtv-3.95 - write-avi.c
// removed audio and MPEG support
// added YUV I420 handling
// BM20070520

/*
 * M$ vidcap avi video+audio layout
 *
 * riff avi
 *   list hdrl       header
 *     avih          avi header
 *     list strl     video stream header
 *       strh
 *       strf
 *     list strl     audio stream header
 *       strh
 *       strf
 *     istf          ??? software
 *     idit          ??? timestamp
 *   yunk            ??? 4k page pad
 *   list movi       data
 *     00db          video data
 *     yunk          ??? 4k page pad
 *     [ ... ]
 *     01wb          audio data
 *     [ ... ]
 *   idx1            video frame index
 *
 */

// fps are given in multiples of 10
// i.e. 1 frame per second -> fp10s = 10
//      1 frame every 10 seconds -> fp10s = 1
#define FPS_MULT 10

// bit depth of the formats must match EVideoFormat !!
const unsigned int ng_vfmt_to_depth[CAviLib::VIDEO_FMT_COUNT] = {
    0,               /* unused   */
    8,               /* RGB8     */
    8,               /* GRAY8    */
    16,              /* RGB15 LE */
    16,              /* RGB16 LE */
    16,              /* RGB15 BE */
    16,              /* RGB16 BE */
    24,              /* BGR24 TD   */
    24,              /* BGR24 BU   */
    32,              /* BGR32    */
    24,              /* RGB24    */
    32,              /* RGB32    */
    16,              /* LUT2     */
    32,              /* LUT4     */
    16,              /* YUY2 4:2:2 */
    16,              /* YVYU 4:2:2 */
    16,              /* YUV422P  */
    12,              /* YUV420P TD */
    12,              /* YUV420P BU */
    0,               /* MJPEG    */
    0,               /* JPEG     */
    16,              /* UYVY 4:2:2 */
    8,               /* Y800 8bit planar */
};

// Macro that makes a FOURCC from four characters.
#define wmcFOURCC( ch0, ch1, ch2, ch3 )				\
		( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |	\
		( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )

// YUV formats.
#define wxAC_FOURCC_YUYV     wmcFOURCC('Y', 'U', 'Y', '2')  /* 16bit/pix YUV 4:2:2  as YUYV*/
#define wxAC_FOURCC_YVYU     wmcFOURCC('Y', 'V', 'Y', 'U')  /* 16bit/pix YVU 4:2:2 */
#define wxAC_FOURCC_UYVY     wmcFOURCC('U', 'Y', 'V', 'Y')  /* 16bit/pix UVY 4:2:2 */
#define wxAC_FOURCC_I420     wmcFOURCC('I', '4', '2', '0')  /* 12bit/pix YUV 4:2:0 */
#define wxAC_FOURCC_Y800     wmcFOURCC('Y', '8', '0', '0')  /* 8bit/pix Y planar 8bit */
#define wxAC_FOURCC_BY8      wmcFOURCC('B', 'Y', '8', '\0') /* 8bit/pix 8  BGBG.. GRGR..(don't know the seq) */
#define wxAC_FOURCC_Y16      wmcFOURCC('Y', '1', '6', '\0') /* 16bit/pix Y planar 16bit */
/*
#define WMCFOURCC_UYVY  0x59565955
#define WMCFOURCC_YUY2		0x32595559
#define WMCFOURCC_YVYU		0x55595659
#define WMCFOURCC_YVU9		0x39555659
#define WMCFOURCC_YV12		0x32315659
#define WMCFOURCC_I420		0x30323449
#define WMCFOURCC_IYUV		0x56555949
*/

// file sze limits
#if 0 /* debugging */
# define LIMIT_OPENDML      (1024*1024)
#else
# define LIMIT_OPENDML (2000*1024*1024) // seems 2GB
#endif


#define size_strl_vids (sizeof(RIFF_strh) + \
			sizeof(RIFF_strf_vids) + \
			4*5)

// Implement the const objs
const AVI_HDR avi_hdr = {
	{{'R','I','F','F'}, 0},	{'A','V','I',' '},
	{{'L','I','S','T'}, 0},	{'h','d','r','l'},
	{{'a','v','i','h'}, AVI_SWAP4(sizeof(RIFF_avih))},	{0}
};

const AVIX_HDR avix_hdr = {
	{{'R','I','F','F'}, 0}, {'A','V','I','X'},
	{{'L','I','S','T'}, 0}, {'m','o','v','i'},
};

const AVI_HDR_VIDEO avi_hdr_video = {
	{{'L','I','S','T'}, AVI_SWAP4(size_strl_vids)},          {'s','t','r','l'},
	{{'s','t','r','h'}, AVI_SWAP4(sizeof(RIFF_strh))},      {{'v','i','d','s'}},
	{{'s','t','r','f'}, AVI_SWAP4(sizeof(RIFF_strf_vids))}, {0}
};

const AVI_HDR_ODML avi_hdr_odml = {
	{{'L','I','S','T'}, AVI_SWAP4(sizeof(wxUint32) + 4*3)},  {'o','d','m','l'},
	{{'d','m','l','h'}, AVI_SWAP4(sizeof(wxUint32))},
};

const AVI_DATA avi_data = {
	{{'L','I','S','T'}, 0},  {'m','o','v','i'},
};

const CHUNK_HDR frame_hdr = {
	{'0','0','d','b'}, 0
};
const CHUNK_HDR idx_hdr = {
	{'i','d','x','1'}, 0
};


#ifdef _DEBUG
	#include <iostream>
	#define  WPOS(x)   std::cerr << (x) << lseek(h->fd,0,SEEK_CUR) << std::endl
#else
	#define WPOS
#endif
/* ----------------------------------------------------------------------- */
/* idx1 frame index                                                        */

CAviLib::CAviLib()
: m_handle(0)
{
}

CAviLib::~CAviLib()
{
}

void CAviLib::avi_addindex(avi_handle *h, char *fourcc, int flags, int chunksize)
{
    if (h->idx_index == h->idx_count) {
		h->idx_count += 256;
		h->idx_array = (IDX_RECORD*) realloc(h->idx_array,h->idx_count*sizeof(IDX_RECORD));
    }
    memcpy(h->idx_array[h->idx_index].id, fourcc, 4);
    h->idx_array[h->idx_index].flags=AVI_SWAP4(flags);
    h->idx_array[h->idx_index].offset=AVI_SWAP4(h->idx_offset-h->hdr_size-8);
    h->idx_array[h->idx_index].size=AVI_SWAP4(chunksize);
    h->idx_index++;
    h->idx_offset += chunksize + sizeof(CHUNK_HDR);
}

void CAviLib::avi_writeindex(avi_handle *h)
{
    if (h->idx_index<=0) return; // no index to write
    /* write frame index */
    h->idx_hdr.size = AVI_SWAP4(h->idx_index * sizeof(IDX_RECORD));
    h->bufOutStream->Write(&h->idx_hdr, sizeof(CHUNK_HDR));
    h->bufOutStream->Write(h->idx_array, h->idx_index*sizeof(IDX_RECORD));
    h->idx_size += h->idx_index * sizeof(IDX_RECORD) + sizeof(CHUNK_HDR);

    /* update header */
    h->avi_hdr.avih.flags |= AVI_SWAP4(AVIF_HASINDEX);
}

void CAviLib::avi_bigfile(avi_handle* h, int last)
{
    off_t avix_end;

    if (h->bigfile) {
		/* finish this chunk */
		avix_end = h->bufOutStream->TellO();
		h->bufOutStream->SeekO(h->avix_start, wxFromStart);
		h->avix_hdr.cRiff.size = h->datax_size + 4*4;
		h->avix_hdr.cListData.size = h->datax_size + 4;
		// to file
      h->bufOutStream->Write(&h->avix_hdr, sizeof(AVIX_HDR));

		h->bufOutStream->SeekO(avix_end, wxFromStart);
		h->avix_start = avix_end;
    }
    else {
		h->avix_start = h->bufOutStream->TellO();
    }

    if (last)
		return;

    h->bigfile++;
    h->framesx = 0;
    h->datax_size = 0;
    h->bufOutStream->Write(&h->avix_hdr, sizeof(AVIX_HDR));
}

/* ----------------------------------------------------------------------- */

void CAviLib::avi_write_header(avi_handle* h)
{
    off_t curpos;

    /* fill in some statistic values ... */
    h->avi_hdr.cRiff.size        = AVI_SWAP4(h->hdr_size + h->data_size + h->idx_size);
    h->avi_hdr.cHdrl.size        = AVI_SWAP4(h->hdr_size - 4*5); // less leading elements
    h->avi_hdr.avih.frames       = AVI_SWAP4(h->frames);

    if (h->video.fmtid != VIDEO_NONE)
		h->avi_hdr_video.strh.length = AVI_SWAP4(h->frames);

    h->avi_data.cListData.size   = AVI_SWAP4(h->data_size);

    /* ... and write header again */
    curpos = h->bufOutStream->TellO();	// save location
    h->bufOutStream->SeekO(0, wxFromStart); // -->BOF

   h->bufOutStream->Write(&h->avi_hdr, sizeof(AVI_HDR));// basic header
	if (h->video.fmtid != VIDEO_NONE) {
      h->bufOutStream->Write(&h->avi_hdr_video,sizeof(AVI_HDR_VIDEO));// v header
	}
	// there was audio...
	if (h->video.fmtid != VIDEO_NONE) {
		h->avi_hdr_odml.total_frames = h->frames_total;
      h->bufOutStream->Write(&h->avi_hdr_odml,sizeof(AVI_HDR_ODML));// odml header
    }

    h->bufOutStream->Write(&h->avi_data, sizeof(AVI_DATA));  // data header

	// move to stored location
    h->bufOutStream->SeekO(curpos, wxFromStart);
}

////////////////////////////////////////////////////////////


// Opens an AVI file
// return true if success
// params: filename, a video descriptor, the fcc type, the fps rate*30
bool CAviLib::avi_open(const wxString &filename,
	                          const video_fmt &video,
							  int fp10s)
{
	if(m_handle)return false;   // alredy allocated

    // avi_video_priv moved to function local
	// descriptor of the AVI file we want to write
	struct avi_video_priv {
		char      handler[4];
		char      compress[4];
		int       bytesperpixel;
	};

    // setup some vide capabilities based on video format
    avi_video_priv  pvideo = {{0}};
    switch (video.fmtid) {
        case VIDEO_RGB15_LE:
        case VIDEO_RGB15_BE:
        case VIDEO_RGB16_LE:
        case VIDEO_RGB16_BE:
            pvideo.bytesperpixel = 2;
            break;
        case VIDEO_BGR24_TD:
        case VIDEO_BGR24_BU:
            pvideo.bytesperpixel = 3;
            break;
        case VIDEO_YUV420_TD:
        case VIDEO_YUV420_BU:
            pvideo.bytesperpixel = 1;
            strncpy(pvideo.compress, "I420", 4);
            break;
        case VIDEO_YUY2_TD:
            pvideo.bytesperpixel = 2;
            strncpy(pvideo.compress, "YUY2", 4);
            break;
        case VIDEO_YVYU_TD:
            pvideo.bytesperpixel = 2;
            strncpy(pvideo.compress, "YVYU", 4);
            break;
        case VIDEO_UYVY_TD:
            pvideo.bytesperpixel = 2;
            strncpy(pvideo.compress, "UYVY", 4);
            break;
        case VIDEO_Y800_TD:
            pvideo.bytesperpixel = 1;
            break;
        case VIDEO_NONE:
        default:
        ;
    }//switch



	avi_handle* h = 0;

	int i,frame_bytes,depth,streams,rate,us_frame,v_rate;

	// get a new handle
    if (filename.IsEmpty())
		return false; // no filename

	if (NULL == (h = (avi_handle*) malloc(sizeof(*h))))
		return false; // no mem

    /* init */
    memset(h,0,sizeof(avi_handle));

	// the received descriptor
	h->video         = video;
	// RIFF strucs
    h->avi_hdr       = avi_hdr;
    h->avix_hdr      = avix_hdr;
    h->avi_hdr_odml  = avi_hdr_odml;
    h->avi_hdr_video = avi_hdr_video;
    h->avi_data      = avi_data;
    h->frame_hdr     = frame_hdr;
    h->idx_hdr       = idx_hdr;
	// a frame ptr array
    h->vec           = (iovec*) malloc(sizeof(iovec) * video.height*2); // max used so far  for YUV

    strncpy(h->file, filename.fn_str() , MAXPATHLEN-1);
    h->fileOutStream = new wxFileOutputStream(filename);
    if ( ! h->fileOutStream->IsOk()) {
		fprintf(stderr,"Creating  wxFileOutputStream %s: %d\n",h->file, h->fileOutStream->GetLastError());
		free(h);
    	m_handle = 0;
		return false; // error opening file
    }
    h->bufOutStream = new wxBufferedOutputStream(*h->fileOutStream);
    /* general */
    streams = 0;
    rate = 0; v_rate=0;
    if (h->video.fmtid != VIDEO_NONE) {
		// having VIDEO
		const int framesize = pvideo.bytesperpixel * h->video.width*h->video.height;
		streams++;
		v_rate=(framesize * fp10s)/FPS_MULT; // video rate per second
		rate += v_rate; // total rate per second
//      h->bufOutStream->GetOutputStreamBuffer()->SetBufferIO(v_rate * 1); // set buffer size for n sec video
      h->bufOutStream->GetOutputStreamBuffer()->SetBufferIO(framesize+sizeof(CHUNK_HDR)); // set buffer size for one frame

		h->avi_hdr.avih.width       = AVI_SWAP4(h->video.width);
		h->avi_hdr.avih.height      = AVI_SWAP4(h->video.height);
    }//if

    us_frame = (long long)1000000 / fp10s * FPS_MULT;
    h->avi_hdr.avih.us_frame    = AVI_SWAP4(us_frame);
    h->avi_hdr.avih.bps         = AVI_SWAP4(rate);
    h->avi_hdr.avih.streams     = AVI_SWAP4(streams);
	// to file
    h->bufOutStream->Write(&h->avi_hdr, sizeof(AVI_HDR));  //basic header
    h->hdr_size += h->bufOutStream->LastWrite();

    /* video */
    if (h->video.fmtid != VIDEO_NONE) {
		depth = ng_vfmt_to_depth[h->video.fmtid];
		frame_bytes = h->video.width * h->video.height * depth /8 ;
		h->avi_hdr.avih.bufsize	 = frame_bytes;
		// strh  setup stream header
		for (i = 0; i < 4; i++) {
			h->avi_hdr_video.strh.handler[i]     = pvideo.handler[i];
		}
		h->avi_hdr_video.strh.scale      = AVI_SWAP4(FPS_MULT);	// use n second scale
		h->avi_hdr_video.strh.rate       = AVI_SWAP4(fp10s);	// sets the display rate
		h->avi_hdr_video.strh.bufsize	 = frame_bytes;
		// strf  setup stream frame (bitmapinfoheader)
		h->avi_hdr_video.strf.size       = AVI_SWAP4(sizeof(avi_hdr_video.strf));
		h->avi_hdr_video.strf.width      = AVI_SWAP4(h->video.width);
		h->avi_hdr_video.strf.height     = AVI_SWAP4(h->video.height);
		h->avi_hdr_video.strf.planes     = AVI_SWAP2(1);
		h->avi_hdr_video.strf.bit_cnt    = AVI_SWAP2(depth ? depth : 24);
		for (i = 0; i < 4; i++) {
			h->avi_hdr_video.strf.compression[i] = pvideo.compress[i];
		}
		h->avi_hdr_video.strf.image_size = AVI_SWAP4(frame_bytes);
		// prepare frame chunk
		h->frame_hdr.size                = AVI_SWAP4(frame_bytes);

		// to file
      h->bufOutStream->Write(&h->avi_hdr_video, sizeof(AVI_HDR_VIDEO));  // v header
		h->hdr_size += h->bufOutStream->LastWrite();
    }//if

	if (h->video.fmtid != VIDEO_NONE) {
		// to file
      h->bufOutStream->Write(&h->avi_hdr_odml, sizeof(AVI_HDR_ODML));  // odml header
		h->hdr_size += h->bufOutStream->LastWrite();
	}
	// header part completed

    /* data */
	// to file  - data header chunk
    h->bufOutStream->Write(&h->avi_data, sizeof(AVI_DATA));  // data header
    if ( h->bufOutStream->LastWrite()<=0 ) {	// data header
		fprintf(stderr,"Write AVI_DATA %s: %d\n",h->file, h->bufOutStream->GetLastError());
		free(h);
    	m_handle = 0;
		return false; // error writing to file
    }
    h->data_size  = 4; // initial size without data (type field)

	// prepare index stuff
    h->idx_index  = 0;
    h->idx_offset = h->hdr_size + sizeof(AVI_DATA);

	// finally rewrite the header with actual data
    avi_write_header(h);

	m_handle = h;
	return (m_handle!=0);
}

// write a Frame Chunk
int CAviLib::avi_video(const video_buf &buf)
{
    avi_handle *h = m_handle;
    iovec *line;
    int size=0;

    size = (int)((buf.size + 3) & ~3); // DWORD align
    h->frame_hdr.size = AVI_SWAP4(size);
    h->bufOutStream->Write(&h->frame_hdr, sizeof(CHUNK_HDR));  // chunk header
    if ( h->bufOutStream->LastWrite()<=0 ) {
		fprintf(stderr,"Write CHUNK_HDR %s: %d\n",h->file, h->bufOutStream->GetLastError());
		return -1;
    }

	bool tdSelect=true; // topdown map selector
    switch (h->video.fmtid) {
		case VIDEO_BGR24_BU:
			tdSelect=false;

		case VIDEO_RGB15_LE:
		case VIDEO_BGR24_TD: {
			int y;
			const unsigned int bpl = h->video.width * ng_vfmt_to_depth[h->video.fmtid] / 8;
			h->avi_hdr_video.strh.quality  = 10000; // raw data

			// setup the vector array -
			if (tdSelect) {
				for (line = h->vec, y = 0; y < (int)h->video.height; line++, y++) {
					line->iov_base = ((wxUint8*)buf.data) + y * bpl;
					line->iov_len  = bpl;
				}
			}
			else {
				//this is for bottomup maps
				for (line = h->vec, y = (int)h->video.height-1; y >= 0; line++, y--) {
					line->iov_base = ((wxUint8*)buf.data) + y * bpl;
					line->iov_len  = bpl;
				}
			}

			iovec* vcx = h->vec; int wx, w = 0;
			for (y=0; y<(int)h->video.height; y++) {
            h->bufOutStream->Write(vcx->iov_base, (unsigned int)vcx->iov_len);
            wx = h->bufOutStream->LastWrite();
				if (-1 == wx)
					return -1;
				w += wx;
				vcx++;  // next scanrow
			}
			if (w!=size)
				return -1; // assumed write does not match real one

			break;
		}

		case VIDEO_YUV420_BU:
			tdSelect=false;

		case VIDEO_YUV420_TD: {
			h->avi_hdr_video.strh.quality  = 10000/4; // yuv data

			// I420 consist of a Y plane with full resolution and an U and V plane at 1/4 resolution
			int y;
			unsigned int bpl;
			const wxUint8 *yBuf=buf.data;
			const wxUint8 *uBuf=&yBuf[h->video.height * h->video.width];
			const wxUint8 *vBuf=&uBuf[h->video.height * h->video.width / 4];
			// setup the vector array -
			if (tdSelect) {
				bpl = h->video.width; // Y part - full width full height
				for (line = h->vec, y = 0; y < (int)h->video.height; line++, y++) {
					line->iov_base = ((wxUint8*)yBuf) + y * bpl;
					line->iov_len  = bpl;
				}
				bpl = h->video.width / 2; // U part - half width half height
				for (y = 0; y < ((int)h->video.height/2); line++, y++) {
					line->iov_base = ((wxUint8*)uBuf) + y * bpl;
					line->iov_len  = bpl;
				}
				bpl = h->video.width / 2; // V part - half width half height
				for (y = 0; y < ((int)h->video.height/2); line++, y++) {
					line->iov_base = ((wxUint8*)vBuf) + y * bpl;
					line->iov_len  = bpl;
				}
			}
			else {
				//this is for bottomup maps
				bpl = h->video.width; // Y part - full width full height
				for (line = h->vec, y = h->video.height-1; y >= 0; line++, y--) {
					line->iov_base = ((wxUint8*)yBuf) + y * bpl;
					line->iov_len  = bpl;
				}
				bpl = h->video.width / 2; // U part - half width half height
				for (y = (h->video.height/2)-1; y >= 0; line++, y--) {
					line->iov_base = ((wxUint8*)uBuf) + y * bpl;
					line->iov_len  = bpl;
				}
				bpl = h->video.width / 2; // V part - half width half height
				for (y = (h->video.height/2)-1; y >= 0; line++, y--) {
					line->iov_base = ((wxUint8*)vBuf) + y * bpl;
					line->iov_len  = bpl;
				}
			}
			/* writev - write a vector
			if (-1 == writev(h->fd,h->vec,h->video.height)) {
				fprintf(stderr,"writev %s: %s\n",h->file,strerror(errno));
				return -1;
			}
			*/
			iovec* vcx = h->vec; int wx, w = 0;
			// must write 2* height (Y , U and V)
			for (y=0; y<(int)h->video.height*2; y++) {
            h->bufOutStream->Write(vcx->iov_base, (unsigned int)vcx->iov_len);
            wx = h->bufOutStream->LastWrite();
				if (-1 == wx)
					return -1;
				w += wx;
				vcx++;		// next scanrow
			}
			if (w!=size)
				return -1; // assumed write does not match real one

			break;
		}
		case VIDEO_YUY2_TD:
		case VIDEO_YVYU_TD:
		case VIDEO_UYVY_TD: {
			int y;
			const unsigned int bpl = h->video.width * ng_vfmt_to_depth[h->video.fmtid] / 8;
			h->avi_hdr_video.strh.quality  = 10000 / 2; // yuyv data

			// setup the vector array -
			if (tdSelect) {
				for (line = h->vec, y = 0; y < (int)h->video.height; line++, y++) {
					line->iov_base = ((wxUint8*)buf.data) + y * bpl;
					line->iov_len  = bpl;
				}
			}
			else {
				//this is for bottomup maps
				for (line = h->vec, y = (int)h->video.height-1; y >= 0; line++, y--) {
					line->iov_base = ((wxUint8*)buf.data) + y * bpl;
					line->iov_len  = bpl;
				}
			}

			iovec* vcx = h->vec; int wx, w = 0;
			for (y=0; y<(int)h->video.height; y++) {
            h->bufOutStream->Write(vcx->iov_base, (unsigned int)vcx->iov_len);
            wx = h->bufOutStream->LastWrite();
				if (-1 == wx)
					return -1;
				w += wx;
				vcx++;  // next scanrow
			}
			if (w!=size)
				return -1; // assumed write does not match real one

			break;
		}

		case VIDEO_Y800_TD: {
			int y;
			const unsigned int bpl = h->video.width * ng_vfmt_to_depth[h->video.fmtid] / 8;
			h->avi_hdr_video.strh.quality  = 10000; // raw data

			// setup the vector array -
			if (tdSelect) {
				for (line = h->vec, y = 0; y < (int)h->video.height; line++, y++) {
					line->iov_base = ((wxUint8*)buf.data) + y * bpl;
					line->iov_len  = bpl;
				}
			}
			else {
				//this is for bottomup maps
				for (line = h->vec, y = (int)h->video.height-1; y >= 0; line++, y--) {
					line->iov_base = ((wxUint8*)buf.data) + y * bpl;
					line->iov_len  = bpl;
				}
			}

			iovec* vcx = h->vec; int wx, w = 0;
			for (y=0; y<(int)h->video.height; y++) {
            h->bufOutStream->Write(vcx->iov_base, (unsigned int)vcx->iov_len);
            wx = h->bufOutStream->LastWrite();
				if (-1 == wx)
					return -1;
				w += wx;
				vcx++;  // next scanrow
			}
			if (w!=size)
				return -1; // assumed write does not match real one

			break;
		}

		case VIDEO_NONE:
//		case VIDEO_RGB08:
//		case VIDEO_GRAY:
		case VIDEO_RGB16_LE:
		case VIDEO_RGB15_BE:
		case VIDEO_RGB16_BE:
//		case VIDEO_BGR32:
//		case VIDEO_RGB24:
//		case VIDEO_RGB32:
//		case VIDEO_LUT2:
//		case VIDEO_LUT4:
//		case VIDEO_YUV422P:
//		case VIDEO_MJPEG:
//		case VIDEO_JPEG:
		case VIDEO_FMT_COUNT:
		default:
            ; // not supported
    }//switch

    h->frames_total += 1;
    if (!h->bigfile) {
		avi_addindex(h, (char*)h->frame_hdr.id, 0x12, size);
		h->data_size  += size + sizeof(CHUNK_HDR);
		h->frames     += 1;
    } else
	{
		h->datax_size += size + sizeof(CHUNK_HDR);
		h->framesx    += 1;
    }
    if ((h->bigfile ? h->datax_size : h->data_size) > LIMIT_OPENDML)
		avi_bigfile(h, 0);

    return 0;
}

// Close the AVI file
// write the Index and rewrite the header with final size data
int CAviLib::avi_close()
{
    avi_handle *h = m_handle;

    /* write frame index */
    if (h->video.fmtid != VIDEO_NONE) {
		if (!h->bigfile) {
			avi_writeindex(h);
		} else {
			avi_bigfile(h,1);
			h->idx_size = 0;
		}
    }

    avi_write_header(h);

    h->bufOutStream->Sync();
    delete h->bufOutStream; h->bufOutStream=NULL;
    h->fileOutStream->Close();
    delete h->fileOutStream; h->fileOutStream=NULL;

    //close(h->fd);
    free(h->vec);
    free(h);
	m_handle = 0;
    return 0;
}

#endif // AVIBUFFERED_IO
