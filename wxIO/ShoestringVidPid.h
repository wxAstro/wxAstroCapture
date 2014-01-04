// VidPid.h: Defines the known USB VID,PID in one place
//
//////////////////////////////////////////////////////////////////////


#ifndef _VIDPID_H__
#define _VIDPID_H__


// the FTDI Vendor ID
#define FTDI_VENDOR_VID         "0403"

/*
* Definitions for Artemis astronomical USB based cameras
* Check it at http://www.artemisccd.co.uk/
*/
#define FTDI_ARTEMIS_PID        "DF28"  /* All Artemis Cameras */
#define FTDI_MINIART_PID        "DF34"  /* Mini Artemis Cameras */
/*
* Definitions for ATIK Instruments astronomical USB based cameras
* Check it at http://www.atik-instruments.com/
*/
#define FTDI_ATIK_ATK16_PID     "DF30"  /* ATIK ATK-16 Grayscale Camera */
#define FTDI_ATIK_ATK16HR_PID   "DF31"  /* ATIK ATK-16HR Grayscale Camera */
#define FTDI_ATIK_ATK16C_PID    "DF32"  /* ATIK ATK-16C Colour Camera */
#define FTDI_ATIK_ATK16HRC_PID  "DF33"  /* ATIK ATK-16HRC Colour Camera */

/*
* More definitions for ATIK Instruments astronomical USB based cameras
* newer line of ATIK IC16 cameras
* Check it at http://www.atik-instruments.com/
*/
#define FTDI_ATIK_ATK16IC_PID   "DF35"  /* ATIK ATK-16IC Grayscale Camera */
#define FTDI_ATIK_ATK16ICC_PID  "DF36"  /* ATIK ATK-16IC-C Colour Camera */
#define FTDI_ATIK_ATK16ICS_PID  "DF2C"  /* ATIK ATK-16IC-S Grayscale Camera */
#define FTDI_ATIK_ATK16ICSC_PID "DF2D"  /* ATIK ATK-16IC-S-C Colour Camera */



//BM:20081226 added
// the Shoestring Vendor ID
#define SSTRING_VENDOR_VID         "134A"
/*
* Definitions for Shoestring astronomical USB based appliances
*/
#define SSTRING_GPUSB_PID        "9020"  /* GPUSB ST4 like guider interface */
#define SSTRING_LXUSB_PID        "9022"  /* LXUSB SC modded webcam control IF */

#endif // _VIDPID_H__

