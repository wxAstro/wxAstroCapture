/**

  Astronomy USB Appliance - Control Library

Copyright (C) 2009 Martin Burri  (bm@burri-web.org)

This program is free software licensed under LGPL;
you can redistribute it and/or modify it under the terms of
the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License,
or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/**
 * @file           ArtTypes.h
 *****************************************************************************
 * enums, structs, typdefs and prototypes
 *
 * Astronomy USB Appliance driver
 *
 * This is part of the Driver Library of wxAstroCapture
 *
 * Copyright (C) 2009 Martin Burri  (bm98@burri-web.org)
 *
 *<hr>
 *
 * @b Project      wxAstroCapture<br>
 *
 * @author         M. Burri
 * @date           01-Feb-2009
 *
 *****************************************************************************
 *<hr>
 * @b Updates
 * - dd-mmm-yyyy V. Name: Description
 *
 *****************************************************************************/

#ifndef _ARTTYPES_H_
#define _ARTTYPES_H_

#include <stddef.h>

#include "GenericImageMem.h"


//////////////////////////////////////////////////////////////////////////
//
// Interface functions for Atik/Artemis CCD Camera Library
//

/// mostly copied from interface DLL (don't change !!)
//! Artemis/ATIK API defs copied from SDK File

//Error codes

enum ARTEMISERROR
{
	ARTEMIS_OK = 0,
	ARTEMIS_INVALID_PARAMETER,
	ARTEMIS_NOT_CONNECTED,
	ARTEMIS_NOT_IMPLEMENTED,
	ARTEMIS_NO_RESPONSE,
};

//Other enumeration types
enum ARTEMISPRECHARGEMODE
{
	PRECHARGE_NONE = 0,     // Precharge ignored
	PRECHARGE_ICPS,         // In-camera precharge subtraction
	PRECHARGE_FULL,         // Precharge sent with image data
};

// Camera State
enum ARTEMISCAMERASTATE
{
	CAMERA_ERROR = -1,
	CAMERA_IDLE = 0,
	CAMERA_WAITING,
	CAMERA_EXPOSING,
	CAMERA_READING,
	CAMERA_DOWNLOADING,
	CAMERA_FLUSHING,
};

// Parameters for ArtemisSendMessage
enum ARTEMISSENDMSG
{
	ARTEMIS_LE_LOW       =0,
	ARTEMIS_LE_HIGH      =1,
	ARTEMIS_GUIDE_NORTH  =10,
	ARTEMIS_GUIDE_SOUTH  =11,
	ARTEMIS_GUIDE_EAST   =12,
	ARTEMIS_GUIDE_WEST   =13,
	ARTEMIS_GUIDE_STOP   =14,
};

// Parameters for ArtemisGet/SetProcessing
// These must be powers of 2.
enum ARTEMISPROCESSING
{
	ARTEMIS_PROCESS_NULL       =0,   // using none sometimes
	ARTEMIS_PROCESS_LINEARISE  =1,   // compensate for JFET nonlinearity
	ARTEMIS_PROCESS_VBE        =2, // adjust for 'Venetian Blind effect'
};

// Parameters for ArtemisSetUpADC
enum ARTEMISSETUPADC
{
	ARTEMIS_SETUPADC_MODE      =0,
	ARTEMIS_SETUPADC_OFFSETR   =(1<<10),
	ARTEMIS_SETUPADC_OFFSETG   =(2<<10),
	ARTEMIS_SETUPADC_OFFSETB   =(3<<10),
	ARTEMIS_SETUPADC_GAINR     =(4<<10),
	ARTEMIS_SETUPADC_GAING     =(5<<10),
	ARTEMIS_SETUPADC_GAINB     =(6<<10),
};

enum ARTEMISPROPERTIESCCDFLAGS
{
	ARTEMIS_PROPERTIES_CCDFLAGS_NULL       =0, // no flag
	ARTEMIS_PROPERTIES_CCDFLAGS_INTERLACED =1, // CCD is interlaced type
	ARTEMIS_PROPERTIES_CCDFLAGS_DUMMY      =0x7FFFFFFF // force size to 4 bytes
};
enum ARTEMISPROPERTIESCAMERAFLAGS
{
	ARTEMIS_PROPERTIES_CAMERAFLAGS_NULL          =0, // no flag
	ARTEMIS_PROPERTIES_CAMERAFLAGS_FIFO          =1, // Camera has readout FIFO fitted
	ARTEMIS_PROPERTIES_CAMERAFLAGS_EXT_TRIGGER   =2, // Camera has external trigger capabilities
	ARTEMIS_PROPERTIES_CAMERAFLAGS_PREVIEW       =4, // Camera can return preview data
	ARTEMIS_PROPERTIES_CAMERAFLAGS_SUBSAMPLE     =8, // Camera can return subsampled data
	ARTEMIS_PROPERTIES_CAMERAFLAGS_COOLING       =0x100,  // Added for wxCA: Camera has cooling capabilities
	ARTEMIS_PROPERTIES_CAMERAFLAGS_WARMUP        =0x200,  // Added for wxCA: Camera has warmup capabilities
	ARTEMIS_PROPERTIES_CAMERAFLAGS_DUMMY         =0x7FFFFFFF // force size to 4 bytes
};


enum EImageConfigBits
{
   // CCD(16) FLAGS
   //ImageConfigLo CCD usage
   EICB_CTP1        = 0x01,   //b0 = Charge Transfer Pulse 1
   EICB_CTP2        = 0x02,   //b1 = Charge Transfer Pulse 2
   EICB_CTPall      = 0x03,   //b0 + b1
   EICB_DeInterlace = 0x04,   //b2 = deInterlace (reads CCD in 2 goes masking b0 then b1)
   EICB_Precharge   = 0x08,   //b3 = PreCharge 1=send separate precharge values. (no not set b5 or b7)
   EICB_FocusMode   = 0x10,   //b4 = Focus Mode (not supported)
   EICB_IpcsMode    = 0x20,   //b5 = ICPS (In camera precharge subtraction mode)
   EICB_PcCheck     = 0x28,   //b3 + b5
   EICB_AmpOn       = 0x40,   //b6 = AmpSE (set to enable amp in self timed exposures)
   EICB_UseFIFO     = 0x80,   //b7 = Use FIFO

   // HSC(40) FLAGS
   //ImageConfigLo flags 2
   EICBH_Preview     = 0x01,  //b0 = Faster low quality read
   EICBH_Subsample   = 0x02,  //b1 = Instead of binning
   EICBH_FastRead    = 0x04,  //b2 = replaced by Preview

   //ImageConfigHi flags 1
   EICBH_CTP2        = 0x0100,   //b0 = Charge Transfer Pulse 1
   EICBH_CTP1        = 0x0200,   //b1 = Charge Transfer Pulse 2
   EICBH_CTPall      = 0x0300,   //b0 + b1 ; set for normal read
   EICBH_DeInterlace = 0x0400,   //b2 = deInterlace - not supported
   EICBH_AmpOn       = 0x0800,   //b3 = 1=amp off during camera timed exposures
   EICBH_StreamImg   = 0x1000,   //b4 = not supported
   EICBH_Overlap     = 0x2000,   //b5 = not supported
   EICBH_Oversample  = 0x4000,   //b6 = not supported
   EICBH_WaitexTrig  = 0x8000,   //b7 = wait for external trigger before exposing
   //
   EICB_reserved    = 0x7fff,    // force size to 2 bytes
};

typedef unsigned char CamCoolingInfoT;

enum ECamCoolingInfo
{// Info flag
   COE_NULL_FLAG  = 0,
   COE_CapsMask   = 0x1f, // b0-4 capabilities
   COE_CCooling   = 0x01, // b0 0 = no cooling 1=cooling
   COE_CControl   = 0x02, // b1 0 = always on 1= controllable
   COE_COnOff     = 0x04, // b2 0 = on/off control not available  1= on off cooling control
   COE_CPwrLevels = 0x08, // b3 0 = no selectable power levels 1= selectable power levels
   COE_SetPoint   = 0x10, // b4 0 = no temperature set point cooling 1= set point cooling

   COE_StatusMask = 0xe0, // b5-7 report what's actually happening
   COE_SWarmUp    = 0x20, // b5 0 =normal control 1=warming up
   COE_SCooling   = 0x40, // b6 0 =cooling off   1=cooling on
   COE_SSetPoint  = 0x80, // b7 0 = no set point control 1=set point control
   //
   COE_reserved   = 0x7fff, // force size to 2 bytes
};


enum EArtCameraType
{
   ACT_ArtCCD_FIRST=0,
   ACT_ArtFTD_00     =  ACT_ArtCCD_FIRST,
   ACT_ArtFTD_01,
   ACT_ArtFTD_02,
   ACT_ArtFTD_03,
   ACT_ArtFTD_04,
   ACT_ArtFTD_05,
   ACT_ArtFTD_06,
   ACT_ArtFTD_07,
   ACT_ArtFTD_08,
   ACT_ArtFTD_09,
   ACT_ArtCCD_LAST,

   ACT_ArtHSC_FIRST  =ACT_ArtCCD_LAST,
   ACT_ArtEzFX2      = ACT_ArtHSC_FIRST,
   ACT_ArtHSC_LAST,

   ACT_ArtSIM_FIRST  =ACT_ArtHSC_LAST,
   ACT_Art429        =ACT_ArtSIM_FIRST,
   ACT_Art285,
   ACT_Art424,
   ACT_Art415,
   ACT_Art4021,
   ACT_Art11002,
   ACT_ArtSIM_LAST,
   ACT_Art_LAST      =ACT_ArtSIM_LAST  // make sure it is the last one !!
};

//Structures

// camera/CCD properties
struct ARTEMISPROPERTIES
{
	int   Protocol;
	int   nPixelsX;
	int   nPixelsY;
	float PixelMicronsX;
	float PixelMicronsY;
	int   ccdflags;     // ARTEMISPROPERTIESCCDFLAGS
	int   cameraflags;  // ARTEMISPROPERTIESCAMERAFLAGS
	char  Description[40];
	char  Manufacturer[40];
};

typedef void* ArtemisHandle;


/// this is wxAstroCapture specific

//! @brief type of grabbed Sample (all coords are unbinned camera pixels)
struct ArtSampleType {
		bool            fullFrame;
		unsigned short  binX;
		unsigned short  binY;
		unsigned long   expotime; //milliseconds
      bool            ready;
      bool            toDeinterlace;  // must be deinterlaced before use
      bool            sampleDropped;   // true if this sample is not a valid one
      WordImageMem*   imageMem;     //20080829BM used to avoid double copy
		size_t          dataLength;   // memsize (BYTES) of the item
};

//! @brief generic Artemis camera worker thread prototype
class ArtWorker
{
public:
 virtual ~ArtWorker(){};
 void operator () () { Entry(); };
 virtual void Entry() = 0;
};

#endif  // _ARTTYPES_H_


