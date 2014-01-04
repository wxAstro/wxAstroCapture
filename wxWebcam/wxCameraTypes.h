
#ifndef _WXCAMERATYPES_H__
#define _WXCAMERATYPES_H__

// Defines some very basic type, enums, strucs
// for the webcap capture code

// Macro that makes a FOURCC from four characters.
#define wmcFOURCC( ch0, ch1, ch2, ch3 )   \
		( (unsigned long)(unsigned char)(ch0) | ( (unsigned long)(unsigned char)(ch1) << 8 ) | \
		( (unsigned long)(unsigned char)(ch2) << 16 ) | ( (unsigned long)(unsigned char)(ch3) << 24 ) )

// YUV FourCC format IDs
#define wxAC_FOURCC_RGB      wmcFOURCC('R', 'G', 'B', ' ')  /* 24 bit pix RGB planes - only for ref*/
#define wxAC_FOURCC_YUY2     wmcFOURCC('Y', 'U', 'Y', '2')  /* 16bit/pix YUV 4:2:2 interleaved YUV 16bit/pixel*/
#define wxAC_FOURCC_YUYV     wmcFOURCC('Y', 'U', 'Y', 'V')  /* 16bit/pix YUV same as above */
#define wxAC_FOURCC_YVYU     wmcFOURCC('Y', 'V', 'Y', 'U')  /* 16bit/pix YVU 4:2:2 interleaved YVU 16bit/pixel*/
#define wxAC_FOURCC_UYVY     wmcFOURCC('U', 'Y', 'V', 'Y')  /* 16bit/pix UVY 4:2:2 interleaved UVY 16bit/pixel*/
#define wxAC_FOURCC_I420     wmcFOURCC('I', '4', '2', '0')  /* 12bit/pix YUV 4:2:0 planar Y then 1/2 U, 1/2 V planes*/
#define wxAC_FOURCC_IYUV     wmcFOURCC('I', 'Y', 'U', 'V')  /* 12bit/pix YUV 4:2:0 same as above */
#define wxAC_FOURCC_YU12     wmcFOURCC('Y', 'U', '1', '2')  /* 12bit/pix YUV 4:2:0 same as above */
#define wxAC_FOURCC_Y800     wmcFOURCC('Y', '8', '0', '0')  /* 8bit/pix Y planar 8bit grey */
#define wxAC_FOURCC_GREY     wmcFOURCC('G', 'R', 'E', 'Y')  /* 8bit/pix Y planar 8bit same as above */
#define wxAC_FOURCC_BY8      wmcFOURCC('B', 'Y', '8', ' ') /* 8bit/pix 8  BGBG.. GRGR..(don't know the seq) */
#define wxAC_FOURCC_Y16      wmcFOURCC('Y', '1', '6', ' ') /* 16bit/pix Y planar 16bit grey */
// special for Artemis
#define wxAC_FOURCC_YP16    wmcFOURCC('Y', 'P', '1', '6')   /* my own for the ArtCam - should change into Y16 once */


// the event sent to the app window when a sample was processed
// pls. call OnCapture() to allow processing
#define WM_CAPTURE      (WM_USER + 630)

// define the Double Map Type to use
//#define DMTYPE_DEF long  // the double map type
//#define DMTYPE_MAX LONG_MAX
#define DMTYPE_DEF short  // the double map type is short
#define DMTYPE_MAX SHRT_MAX

#define WXAC_PEVENT_ID_RT  0x10040  // realtime paint event
#define WXAC_PEVENT_ID_LE  0x10041  // long exposure paint event

#if defined (__UNIX__)
   // just have this here for the time beeing
   typedef struct tagBITMAPINFOHEADER{
         unsigned long  biSize; // -> sizeof(BITMAPINFOHEADER) is 40
         long           biWidth;
         long           biHeight;
         unsigned short biPlanes;
         unsigned short biBitCount;
         unsigned long  biCompression;
         unsigned long  biSizeImage;
         long           biXPelsPerMeter;
         long           biYPelsPerMeter;
         unsigned long  biClrUsed;
         unsigned long  biClrImportant;
   } BITMAPINFOHEADER, *PBITMAPINFOHEADER;

   // defines biCompression of BITMAPINFOHEADER
   #define BI_RGB  0
#endif


// kind of implementation of wxAstroCapture
enum EOSKind
{
   EOSK_Unknown   = 0,
   EOSK_Win32     = 1,
   EOSK_Linux     = 2,
};

// camera type
enum ECType
{
   ECT_Unknown   = 0,
   ECT_Webcam    = 1,
   ECT_Artcam    = 2,
};

// kind of synch for OnCaptureRequired
enum ESYNMode
{
   ESYN_None      = 0,
   ESYN_Tick      = 1,	// only the sample pace
   ESYN_Capture   = 2,  // also need to gather a sample
   ESYN_All       = 3,
};

// defines the kind of transfor to apply when collecting images
enum ECaptureMathKind
{
	EMK_Mean        = 0,
	EMK_Stretch     = 1,
	EMK_Sum         = 2,
	EMK_Log         = 3,
	EMK_HistStretch = 4,
	EMK_DropStretch = 5,
	EMK_Algo_7      = 6,    // reserve
	EMK_Algo_8      = 7,
	EMK_Algo_9      = 8,
	EMK_Algo_10     = 9
};

// Basic video/image format type
enum EVideoSampleFormat
{
	EVF_Unknown     = -1,
	EVF_YUV         = 0,  // equals to any YUV format
	EVF_RGB24       = 1,  // BGR24 or RGB24 type
	EVF_YP16        = 50, // equals to YP16 Artemis format
};

enum EContainerFormat
{
	ECF_AVI         = 0, // AVI container
	ECF_ZIP_FITS    = 1, // ZIP container with FITS files
	ECF_FILES_FITS  = 2, //20080821:BM File container with FITS files
};

// defines the conversion done with the Filter
enum EVideoConversion
{
	EVC_UNKNOWN         = -1,   // no conversion possible
	EVC_RGB24_RGB24     = 0,    // regular convert RGB24 to BGR24
	EVC_I420_RGB24      = 1,    // regular convert I420(IYUV) to BGR24
	EVC_YUY2_RGB24      = 2,    // regular convert YUY2  to BGR24 this is the same as YUYV but better known by codecs
	EVC_YVYU_RGB24      = 3,    // regular convert YVYU  to BGR24
	EVC_UYVY_RGB24      = 4,    // regular convert UYVY  to BGR24
	EVC_Y800_RGB24      = 5,    // regular convert Y800  to BGR24
	EVC_BY8_RGB24       = 6,    // bayer convert BY8  to BGR24
	EVC_CFA_RGB24       = 7,    // convert Y (Bayer) CFA to RGB24 (only available with native=I420(YUV))
	EVC_Y16             = 50,   // 16bit Y raw
};

// define the CCD mapping of the Bayer pattern
enum EVideoPixelOrder
{
	EVO_GB = 0,
	EVO_BG = 1,
	EVO_GR = 2,
	EVO_RG = 3
};

// defines the type of streaming that is active right now
enum EGraphStreamingType
{
	GST_Off           = 0,  // no capture streaming
	GST_Stream        = 1,  // capture streaming it will call CaptureEvent()
	GST_Collect       = 2,  // collecting is subsampling images, will call CaptureEvent()
	GST_Classify      = 3   // from each frame min and max intensity is calculated --> ClassifyEvent()
};

// If there is extended property access we usually know the chipset of the device
enum EKnownChipsetCat
{
	CSX_Unknown       = 0,  // we don't know more - may be it supports some basic properties
	CSX_SAA8112       = 1,  // Philips Vesta type of CCD webcams
	CSX_SAA8116       = 3,  // Philips ToUCam breed (Logitech, Creative etc.)
	CSX_CREALIVEULTRA = 4,  // USB2.0 Creative Live Ultra CCD cam
	CSX_LOGIFUSION    = 5,  // Logitec Fusion USB2.0 series of CCD cams
	CSX_TRUST5400     = 6,  // Trust 5400 USB2
	CSX_MSLIVECIN     = 7,  // Microsoft Live Cam Cinema
	// Arttypes
	CSX_ArtCCD_PX     = 56,    // Artemis icx285 progressive CCD type cam (Atik as well)
	CSX_ArtCCD_IL     = 57,    // Artemis icx429 interlaced CCD type cam (Atik as well)
	CSX_ArtHSC        = 60,    // Artemis HSC type cam (pro line)
};

// cooling support
enum ECoolingStatus {
   ECS_NotAvailable = -1,     // there is no status information
   ECS_Off          = 0,      // cooling is off
   ECS_Cooling      = 1,      // cooling is active and on it's way to reach the setpoint
   ECS_Holding      = 2,      // cooling is active and holding the setpoint
   ECS_Warmup       = 3,      // cooling is active but in a controled warmup mode (cannot be interrupted)

};

#endif // _WXCAMERATYPES_H__


