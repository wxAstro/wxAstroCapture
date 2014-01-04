#ifndef _RIFF_H__
#define _RIFF_H__

#include "byteswap.h"

#if BYTE_ORDER == BIG_ENDIAN
# define AVI_SWAP2(a) SWAP2((a))
# define AVI_SWAP4(a) SWAP4((a))
#else
# define AVI_SWAP2(a) (a)
# define AVI_SWAP4(a) (a)
#endif


/* Description of a basic AVI file

struct _RIFF   / * "RIFF" * /
{
    struct _AVICHUNK   / * "AVI " * /
    {
        struct _LISTHEADERCHUNK   / * "hdrl" * /
        {
            AVIHEADER AviHeader;     / * "avih" * /
            struct _LISTHEADERCHUNK  / * "strl" * /
            {
                AVISTREAMHEADER	StreamHeader; / * "strh" * /
                AVISTREAMFORMAT	StreamFormat; / * "strf" * /
                AVISTREAMDATA	StreamData;   / * "strd" * /
            }
        }
        struct _LISTMOVIECHUNK  / * "movi" * /
        {
            struct _LISTRECORDCHUNK  / * "rec " * /
            {
                / * Subchunk 1 * /
                / * Subchunk 2 * /
                / * Subchunk N * /
            }
        }
        struct _AVIINDEXCHUNK  / * "idx1" * /
        {
            / * Index data * /
        }
    }
}
*/


#define AVIF_HASINDEX                   0x10

// Basic RIFF Format Chunk Header
/* i.e.
const CAviLib::CHUNK_HDR CAviLib::frame_hdr = {
	{'0','0','d','b'}, 0
};
*/
struct CHUNK_HDR {
    wxUint8             id[4];
    wxUint32            size;
};


// The first mandatory LIST chunk contains the main AVI header subchunk and has the identifier hdrl.
// The information in the header subchunk defines the format of the entire AVI chunk.
// The hdrl chunk must appear as the first chunk within the AVI chunk. The format of the header subchunk is the following:
struct RIFF_avih {
    wxUint32 us_frame;          // Time delay between frames, amount of delay between frames in microseconds.
    wxUint32 bps;               // byte/s overall - Data rate of the AVI data in bytes per second.
    wxUint32 unknown1;          // PaddingGranularity specifies the multiple size of padding used in the data in bytes.
		// When used, the value of this field is typically 2048.
    wxUint32 flags;				// Data parameters
		// Flags contains parameter settings specific to the AVI file and its data.
		// The parameters correspond to the bit values of the Flags field as follows:
		// Bit  4 AVI chunk contains an index subchunk (idx1).
		// Bit  5 Use the index data to determine how to read the AVI data, rather than the physical order of the chunks with the RIFF file.
		// Bit  8 AVI file is interleaved.
		// Bit 16 AVI file is optimized for live video capture.
		// Bit 17 AVI file contains copyrighted data.
    wxUint32 frames;            // TotalNumberOfFrames indicates the total number of frames of video data stored in the movi subchunk.
    wxUint32 init_frames;       // NumberOfInitialFrames specifies the number of frames in the file before the actual AVI data.
		//For non-interleaved data this value is 0
    wxUint32 streams;			// Number of data streams in chunk
    wxUint32 bufsize;           // suggested buffer size - Minimum playback buffer size
    wxUint32 width;				// Width of video frame in pixels
    wxUint32 height;			// Height of video frame in pixels
    wxUint32 scale;				// Unit used to measure time
		// TimeScale is the unit used to measure time in this chunk.
		// It is used with DataRate to specify the time scale that the stream will use.
		// For video streams, this value should be the frame rate and typically has a value of 30.
    wxUint32 rate;				//DataRate is divided by the TimeScale value to calculate the number of samples per second.
    wxUint32 start;				// StartTime is the starting time of the AVI data and is usually 0.
    wxUint32 length;			// DataLength is the size of the AVI chunk in the units specified by the TimeScale value.
};

/* i.e.
const CAviLib::AVI_HDR CAviLib::avi_hdr = {
	{'R','I','F','F'}, 0, {'A','V','I',' '},
	{'L','I','S','T'}, 0, {'h','d','r','l'},
	{'a','v','i','h'}, AVI_SWAP4(sizeof(RIFF_avih)), {0}
};
*/
struct AVI_HDR {
	CHUNK_HDR	cRiff;		wxUint8	riff_type[4];
	CHUNK_HDR	cHdrl;		wxUint8	hdrl_type[4];
	CHUNK_HDR	cAvihdr;	RIFF_avih	avih;
};

// The second mandatory LIST chunk contains the actual AVI data, has the identifier movi,
// and must appear as the second chunk within the AVI chunk.
// If the data is not interleaved, it is stored as a single block of data within the movi chunk itself.


/* i.e.
const CAviLib::AVIX_HDR CAviLib::avix_hdr = {
	{'R','I','F','F'}, 0, {'A','V','I','X'},
	{'L','I','S','T'}, 0, {'m','o','v','i'},
};
*/
struct AVIX_HDR {
    CHUNK_HDR	cRiff;		wxUint8	riff_type[4];
    CHUNK_HDR	cListData;	wxUint8	data_type[4];
};


struct RIFF_strh {
    wxUint8 type[4];      // Chunk identifier ("strl")
		//  Identifiers supported by the current version of the RIFF format are: vids for video data and auds for audio data.
    wxUint8 handler[4];	// DataHandler may contain a 4-character identifier specifying the preferred type of device to handle the data stream
    wxUint32 flags;				// Flags contains a set of bit flags use to indicate parameter settings related to the data.
    wxUint32 priority;			// Priority is set to 0.
    wxUint32 init_frames;       // InitialFrames indicates in seconds how far the audio is placed ahead of the video in interleaved data.
    wxUint32 scale;				// Unit used to measure time
		// TimeScale, DataRate, StartTime, DataLength, and SuggestedBufferSize all have the same function as the fields of the same names in the hdr1 chunk.
    wxUint32 rate;				// Data rate of playback
    wxUint32 start;				// Starting time of AVI data
    wxUint32 length;			// Size of AVI data chunk
    wxUint32 bufsize;           // Minimum playback buffer size
    wxUint32 quality;			// Quality is an integer in the range of 0 to 10,000, indicating the quality factor used to encode the sample.
    wxUint32 samplesize;		// SampleSize is the size of a single sample of data.
		// If this value is 0, the sample varies in size and each sample is stored in a separate subchunk.
		// If this value is non-zero, then all the samples are the same size and are stored in a single subchunk.
    /* XXX 16 bytes ? */
	// Immediately following the stream header is a stream format subchunk with the identifier strf.
	// This header describes the format of the stream data.
	// Its format varies depending on the type of data that is stored (audio or video). This subchunk is also required.
};



struct RIFF_strf_vids {       /* == BitMapInfoHeader */
    wxUint32 size;
    wxUint32 width;
    wxUint32 height;
    wxUint16 planes;
    wxUint16 bit_cnt;
    wxUint8 compression[4];
    wxUint32 image_size;
    wxUint32 xpels_meter;
    wxUint32 ypels_meter;
    wxUint32 num_colors;        /* used colors */
    wxUint32 imp_colors;        /* important colors */
    /* may be more for some codecs */
};

/*i.e.
const CAviLib::AVI_HDR_VIDEO CAviLib::avi_hdr_video = {
	{'L','I','S','T'}, AVI_SWAP4(size_strl_vids),          {'s','t','r','l'},
	{'s','t','r','h'}, AVI_SWAP4(sizeof(RIFF_strh)),      {{'v','i','d','s'}},
	{'s','t','r','f'}, AVI_SWAP4(sizeof(RIFF_strf_vids)), {0}
};
*/
struct AVI_HDR_VIDEO {
	CHUNK_HDR	cStrl;	wxUint8	      strl_type[4];
    CHUNK_HDR	cStrh;	RIFF_strh		strh;
    CHUNK_HDR	cStrf;	RIFF_strf_vids  strf;
};

/* i.e.
const CAviLib::AVI_HDR_ODML CAviLib::avi_hdr_odml = {
	{'L','I','S','T'}, AVI_SWAP4(sizeof(wxUint32) + 4*3),  {'o','d','m','l'},
	{'d','m','l','h'}, AVI_SWAP4(sizeof(wxUint32)),
};
*/
struct AVI_HDR_ODML {
    CHUNK_HDR	cOdml;	wxUint8	   odml_type[4];
    CHUNK_HDR	cDmlh;  wxUint32		total_frames;
};

/* i.e.
const CAviLib::AVI_DATA CAviLib::avi_data = {
	{'L','I','S','T'}, 0,  {'m','o','v','i'},
};
*/
struct AVI_DATA {
    CHUNK_HDR	cListData;	wxUint8 data_type[4];
    /* audio+video data follows */
};


struct IDX_RECORD {
    wxUint8             id[4];
    wxUint32            flags;
    wxUint32            offset;
    wxUint32            size;
};


#endif  // _RIFF_H__
