
#ifndef _CAPTYPES_H__
#define _CAPTYPES_H__

#ifndef DSCAPLIB_EXPORTS
// this part will not be defined in the dscaplib.dll code

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wxCameraTypes.h" // enums and structs of the Camera environment

#else
#include "wxCameraTypes.h" // enums and structs of the Camera environment
#endif // !DSCAPLIB_EXPORTS


// Properties may provide one of the access types below (must be checked before using it!!)
enum EVidCapPropertySupport
{
	VCS_no      = 0,  // not supported
	VCS_get     = 1,  // get support
	VCS_set     = 2,  //	set support
	VCS_both    = 3   //	get+set support
} ;

// The extended WhiteBalance Mode
enum EWB_Mode{
    EWB_NotAvailable = -1, // if it is not available
    EWB_Tungsten     = 0,  // Warm or Indoor setting
    EWB_Outdoor      = 1,  // Sun or outdoor setting
    EWB_FL           = 2,  // Fluorescence light setting
    EWB_Freeze       = 3,  // Manual WB Red and Blue are working now
    EWB_Auto         = 4,  // Auto WB maintained by the camera (RB sliders are off)
} ;

// All the different properties that are available
// Not all devices may sopport all properties - to be checked with PropertyHas() before using it
// Also the value range differs from devices - check the range before setting values
 enum EVidCapProperty{
	// *** copied from VideoProcAmp (MSDN VideoProcAmpProperty Enumeration Page)
    // V4L2 Properties first
		VCP_BASE                    = 0,
		// used to enumerate independent from content - use this in for loops
		VCP_FirstElement            = VCP_BASE,

	//Specifies the brightness, also called the black level.
		VCP_Brightness              = VCP_BASE+0,//V4L2_CID_BRIGHTNESS, VideoProcAmp_Brightness
	//Specifies the contrast
		VCP_Contrast                = VCP_BASE+1,//V4L2_CID_CONTRAST, VideoProcAmp_Contrast
	//Specifies the hue
		VCP_Hue                     = VCP_BASE+2,//V4L2_CID_HUE, VideoProcAmp_Hue
	//Specifies the saturation
		VCP_Saturation              = VCP_BASE+3,//V4L2_CID_SATURATION, VideoProcAmp_Saturation
	//Specifies the sharpness.
		VCP_Sharpness               = VCP_BASE+4,//V4L2_CID_PRIVATE_CONTOUR, VideoProcAmp_Sharpness
	//Specifies the gamma
		VCP_Gamma                   = VCP_BASE+5,//V4L2_CID_GAMMA, VideoProcAmp_Gamma
		VCP_ColorEnable             = VCP_BASE+6,//V4L2_CID_PRIVATE_COLOUR_MODE, VideoProcAmp_ColorEnable
		VCP_WhiteBalance            = VCP_BASE+7,//n.a., VideoProcAmp_WhiteBalance
		VCP_BacklightCompensation   = VCP_BASE+8,//V4L2_CID_PRIVATE_BACKLIGHT, VideoProcAmp_BacklightCompensation

		VCP_WhiteBalanceMode        = VCP_BASE+9,  //VIDIOCPWC[SG]AWB, (0=Warm, 1=Outdoor, 2=FL, 3=Freeze,4=Auto)
		VCP_WhiteBalRed             = VCP_BASE+10, //V4L2_CID_RED_BALANCE range 0..65535, (red part 0..255)
		VCP_WhiteBalBlue            = VCP_BASE+11, //V4L2_CID_BLUE_BALANCE range 0..65535, (blue part 0..255)

		VCP_FlickerFree             = VCP_BASE+12, //V4L2_CID_PRIVATE_FLICKERLESS, (0=off  else on)
		VCP_Flicker50_60            = VCP_BASE+13, //n.a., (0=50Hz, 255=60Hz)

		VCP_FPS                     = VCP_BASE+14, //VIDIOC[SG]WIN, (0=5,1=10,2=15,3=20,4=25,5=30,6=60)
		VCP_Exposure                = VCP_BASE+15, //V4L2_CID_EXPOSURE, (0..10 / 0=1/5, 1=1/10, 2=1/25...)
		VCP_Gain                    = VCP_BASE+16, //V4L2_CID_GAIN, (0..63)

		VCP_FlipHorizontal          = VCP_BASE+17, //V4L2_CID_HFLIP
		VCP_FlipVertical            = VCP_BASE+18, //V4L2_CID_VFLIP

		VCP_AutoExposure            = VCP_BASE+19, //V4L2_CID_AUTOGAIN
		VCP_AutoAll                 = VCP_BASE+20, //???

		// used to enumerate independent from content - use this in for loops for common settings
		VCP_LastSetting             = VCP_AutoAll,

	// *** others
		VCP_ActualFrameRate         = VCP_BASE+21, //VIDIOCGWIN (vwin.flags & PWC_FPS_FRMASK) >> PWC_FPS_SHIFT

	// *** Control parameters (set only for some SAA chip cams)
		VCP_LED                     = VCP_BASE+22, //VIDIOCPWCxLED range = 0..25000, 16 on, =17 slow blink, =18 fast blink
		VCP_Setting                 = VCP_BASE+23, //V4L2_CID_PRIVATE_SAVE_USER, RESTORE_USER, RESTORE_FACTORY
		VCP_ExtTrigger              = VCP_BASE+24, //n.a.
		VCP_SWTrigger               = VCP_BASE+25, //n.a.

	// *** CamControl parameters (mostly set only)
		VCP_Pan                     = VCP_BASE+26, // camera's pan setting
		VCP_Tilt                    = VCP_BASE+27, // camera's tilt setting
		VCP_Roll                    = VCP_BASE+28, // camera's roll setting
		VCP_Zoom                    = VCP_BASE+29, // camera's zoom setting
		VCP_CamExposure             = VCP_BASE+30, // exposure setting (mostly not supported...)
		VCP_Iris                    = VCP_BASE+31, // camera's iris setting
		VCP_Focus                   = VCP_BASE+32, // camera's focus setting

		// used to enumerate independent from content - use this in for loops for all settings
		VCP_LastElement             = VCP_Focus,
		VCP_LastElement1            = VCP_LastElement+1,
} ;


	// type of grabbed Sample
struct CapSampleType {
		long     startX;
		long     startY;
		long     width;
		long     height;
		bool     fullFrame;
		bool     nativeIsRGB;      // true only if the native format is RGB24
	};


#endif // _CAPTYPES_H__


