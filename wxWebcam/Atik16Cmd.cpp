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
 * @file           Atik16Cmd.cpp
 *****************************************************************************
 * class Atik16Cmd
 *
 * Astronomy USB Appliance driver for Linux and Win32
 * The basic strucs and commands for Artemis/ATIK cameras with USB 1.1 interface
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

#include "Atik16Cmd.h"

/// There is a derived version used for the INDI framework
///  which has some files in another place than wxAstroCapture
#ifdef INDI
   #include "GenericDebHelp.h"
#else
   #include "../GenericDebHelp.h"
#endif

#include "Ftd245IO.h"

#include <stdlib.h>
#include <iostream>
#include <iomanip>

#include <assert.h>

using namespace std;

// local shortcuts
typedef unsigned char  _byte;
typedef unsigned short _word;
typedef unsigned long  _dword;

// defines the 3 chars leading any command
#define CMD_ 'C','M','D'

enum ECamCapsBuffer
{// buffer pos of named item
   CCE_VMINOR     = 0,  // ProtocolRevMin Byte
   CCE_VMAJOR     = 1,  // ProtocolRevMaj Byte
   CCE_ID         = 2,  // CamType; Null terminated string
   CCE_USERID     = 42, // Manufacturer; Null terminated string (following the ID)
   // reposition field counter here
   CCE_FLAGS_LO   = 0,  // Flags Lo Byte (NOTE:  slightly different from CCD)
   CCE_FLAGS_LO_FIFO        = 0x01, // Flags Lo B0 = FIFO fitted
   CCE_FLAGS_LO_INTERLACED  = 0x02, // Flags Lo B1 = Interlaced CCD
   CCE_FLAGS_HI   = 1,  // Flags Hi Byte (Not used)
   CCE_TPIX_X     = 2,  // TotalPixelsX Word LO/HI
   CCE_TPIX_Y     = 4,  // TotalPixelsY Word LO/HI
   CCE_PIXSIZE_X  = 6,  // PixelSizeX Word LO/HI (size in microns * 100 i.e 740=7.40u)
   CCE_PIXSIZE_Y  = 8,  // PixelSizeY Word LO/HI (size in microns * 100 i.e 740=7.40u)
   CCE_MAXBIN_X   = 10, // MaxBinX Word LO/HI
   CCE_MAXBIN_Y   = 12, // MaxBinY Word LO/HI (0xff = only symmetric binning available -> Y=Xbin)
   CCE_WELLCAP    = 14, // WellCap Word LO/HI (cap in 1k e units i.e. 60 = 60'000 e)
};

enum ECamReadBuffer
{// buffer pos of named item
   CRE_C    = 0   ,  //
   CRE_M          ,  //
   CRE_D          ,  //
   CRE_COMMAND    ,  // Command Byte
   CRE_EXPO       ,  // Exposure Time Byte
                           // Exposure Time
                           // 0=exposure timed on PC; the CCD is read without being cleared first
                           // 1-9 Exposure timed in the camera. Value = exposure time in mS
                           // 10-255 Exposure timed in camera. Exposure time = (Value-9)*10ms
   CRE_XBIN       ,  // XBin Byte
   CRE_YBIN       ,  // YBin Byte
   CRE_XSTART_LO  ,  // XstartLo Byte Start position X Lo Byte
   CRE_XSTART_HI  ,  // XstartHi Byte Start position X Hi Byte
   CRE_YSTART_LO  ,  // YstartLo Byte Start position Y Lo Byte
   CRE_YSTART_HI  ,  // YstartHi Byte Start position Y Hi Byte
   CRE_NUMX_LO    ,  // NumXLo Number or X pixels to read Lo byte (CCD)
   CRE_NUMX_HI    ,  // NumXHi Number or X pixels to read Hi byte (CCD)
   CRE_NUMY_LO    ,  // NumYLo Number or Y pixels to read Lo byte (CCD)
   CRE_NUMY_HI    ,  // NumYHi Number or Y pixels to read Hi byte (CCD)
   CRE_IMGCFG_LO  ,  // ImageConfigLo Flags 2 see EImageConfigBits EICB_xyz
   CRE_IMGCFG_HI  ,  // ImageConfigHi Flags 1 see EImageConfigBits EICB_xyz

   CRE_SIZE         // MUST be the last
};

enum EArtCommands  // CCD COMMANDS
{
   CCMD_Reset           = 0,   // Par: none Ret: none
                                // Complete reset of PIC incl USB (reenumerates USB!!!)

   CCMD_RequestCAPS     = 1,   // Par: none
                               // Ret: Infoblock (ECamCapsBuffer)
                                // returns caps record

   CCMD_Amp             = 2,   // Par: BYTE - 0=Amp Off, 1=AmpOn
                               // Ret: none
                                // If Byte <>0 then CCD Amplifier turned on, else turned off.

   CCMD_ADC             = 3,   // Par: BYTE 0=ADC off, 1=ADC on
                               // Ret: none - Set the cs line to the ADC.
                                 // !! Already in Read command included

   CCMD_ClearCCD        = 4,   // Par: none Ret: none
                                // Clears the CCD including readout regs

   CCMD_Shutter         = 5,   // Par: none Ret: none
                                // Fires the electronic shutter to start the exposure

   CCMD_ReadCCD         = 6,   // Par: Parameterblock (ECamReadBuffer) - The single command read CCD
                               // Ret: image array
                                // Returns NUMX / XBin  * NUMY / YBin  16 bit words as image array (CCD)

   CCMD_App_Ping        = 80,  // Par: BYTE
                               // Ret: BYTE
                                // Returns the byte sent

   CCMD_ClockCCD        = 18,  // Par: none Ret: none
                                // Set the camera into a loop clocking the CCD but sending no data back.

   CCMD_FIFO_Fitted     = 89,  // Par: none
                               // Ret: BYTE - Returns 1 if yes else 0.

   CCMD_Ping            = 253, // Par: none Ret: BYTE - read via I2C until 0xff

   CCMD_GetSerialNo     = 254,  // Par: none Ret: 6bytes serial + 1 byte HW (0=beta/1=HWversion)
                                // Returns the cameras serial No and HW rev index

   CCMD_GuideOff        = 24,   // Par: none Ret: none
                                // Switches all guiding off

   CCMD_SendExtPort     = 11,   // Par: none Ret: 1*byte (01=OK)
                                // Sends bytes to expansion port

};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
static void RetX(const char s[], bool rVal)
{
#ifdef _DEBUG
   if (!rVal)
      cout << s << " - returns: " << rVal << endl;
#endif
}


Atik16Cmd::Atik16Cmd()
{
}

Atik16Cmd::~Atik16Cmd()
{
}


//! Reset
// Resets the Camera and everything
bool Atik16Cmd::CMD_RESET(const AtikDevDescRec& dev)
{
	_V_ ::fprintf(stdout, "Atik16Cmd::CMD_RESET()\n");
	bool retVal;
	size_t bWritten;

	_byte hsc[] = { CMD_, CCMD_Reset};
   retVal = dev.ftd->WriteBYTEs(hsc, sizeof(hsc), bWritten); RetX("WriteBYTEs (CCMD_Reset)", retVal);
   dev.ftd->Sleep_ms(100);

	return retVal;
}

//! PIC Ping
/*
Returns 100 dec
 returns BYTE  (100 dec)
*/
bool Atik16Cmd::CMD_PICPing(const AtikDevDescRec& dev)
{
	_V_ ::fprintf(stdout, "Atik16Cmd::CMD_PICPing()\n");
	bool retVal;
	size_t bWritten;
   _byte pingVal = 0;

   // info = "CMD" + Chr$(253)
	_byte hsc[] = { CMD_, CCMD_Ping};
   retVal = dev.ftd->WriteBYTEs(hsc, sizeof(hsc), bWritten); RetX("WriteBYTEs (CCMD_Ping)", retVal);
   dev.ftd->Sleep_ms(100);
	retVal = retVal && dev.ftd->ReadBYTE(pingVal);
	return retVal;
}

//! Application Ping
/*
Returns the byte sent
  returns BYTE  (para loopback)
*/
bool Atik16Cmd::CMD_AppPing(const AtikDevDescRec& dev, _byte& pingVal)
{
	_V_ ::fprintf(stdout, "Atik16Cmd::CMD_AppPing()\n");
	bool retVal;
	size_t bWritten;

	_byte hsc[] = { CMD_, CCMD_App_Ping, pingVal };
   pingVal = ~pingVal; // trash result to make sure we get an error
   retVal = dev.ftd->WriteBYTEs(hsc, sizeof(hsc), bWritten); RetX("WriteBYTEs (CCMD_App_Ping)", retVal);
   dev.ftd->Sleep_ms(100);
	retVal = retVal && dev.ftd->ReadBYTE(pingVal);
	return retVal;
}

//! FIFO Fitted Query
/*
Returns 1 if FIFO is fitted, else 0
*/
bool Atik16Cmd::CMD_FIFO_Fitted(const AtikDevDescRec& dev, bool& fifofitted)
{
	_V_ ::fprintf(stdout, "Atik16Cmd::CMD_FIFO_Fitted()\n");
	bool retVal;
	size_t bWritten;
   _byte pingVal = 0;
   fifofitted = false;

   // info = "CMD" + Chr$(89)
	_byte hsc[] = { CMD_, CCMD_FIFO_Fitted};
   retVal = dev.ftd->WriteBYTEs(hsc, sizeof(hsc), bWritten); RetX("WriteBYTEs (CCMD_FIFO_Fitted)", retVal);
   dev.ftd->Sleep_ms(100);
	retVal = retVal && dev.ftd->ReadBYTE(pingVal);
	fifofitted = (pingVal!=0); // Returns 1 if FIFO is fitted, else 0
	return retVal;
}

//! Amplifier
/*
Sets the Vcc voltage level to the CCD at high (readout) or low (integration) levels.
0 = low level or Amp off
1 = High level or Amp on
*/
bool Atik16Cmd::CMD_Amplifier(const AtikDevDescRec& dev, bool ampState)
{
	_V_ ::fprintf(stdout, "Atik16Cmd::CMD_Amplifier(%d) \n", int(ampState));
	bool retVal;
	size_t bWritten;

	_byte hsc[] = { CMD_, CCMD_Amp, ((ampState)? 1 : 0) };
   retVal = dev.ftd->WriteBYTEs(hsc, sizeof(hsc), bWritten); RetX("WriteBYTEs (CCMD_Amp)", retVal);
   dev.ftd->Sleep_ms(100);
	return retVal;
}


//! Send Capabilities

//! Asks for a data block to the PC describing the capabilities of the camera. The code
//!  also does some initialisation and must be called before using the camera and after a reset.

/* returns
	BYTE	ProtocolRevMin
	BYTE	ProtocolRevMaj
	ASCIIZ	ID Null terminated string
	ASCIIZ	User ID
	BYTE	Flags Lo
			Flags Lo b0 = FIFO fitted
					 b1 = Interlaced CCD
	BYTE	Flags Hi
			Flags Hi Not used
	BYTE*2 (lo,hi) TotalPixelsX
	BYTE*2 (lo,hi) TotalPixelsY
	BYTE*2 (lo,hi) PixelSizeX
	BYTE*2 (lo,hi) PixelSizeY
	BYTE*2 (lo,hi) MaxBinX
	BYTE*2 (lo,hi) MaxBinY
	BYTE*2 (lo,hi) WellCap
	BYTE	FIFO Fitted Byte
*/
bool Atik16Cmd::CMD_RequestCAPS(const AtikDevDescRec& dev, AtikCamCapsRec &caps)
{
   _V_ ::fprintf(stdout, "Atik16Cmd::CMD_RequestCAPS() \n");
	bool retVal;
	size_t bWritten, bRead;
	_byte buf[64]; // enough !!??( may not exceed 40 according to manual

	_byte hsc[] = { CMD_, CCMD_RequestCAPS };

   caps.Init();   // RESET CAPS

   retVal = dev.ftd->WriteBYTEs(hsc, sizeof(hsc), bWritten); RetX("WriteBYTEs (CCMD_RequestCAPS)", retVal);

	retVal = retVal && dev.ftd->ReadBYTEs(&buf[0], 2, bRead);
	if ( !retVal || (bRead!=2) )
		return false; // could not read enough - bailing out now

	caps.ProtocolRevMin = buf[CCE_VMINOR];
	caps.ProtocolRevMaj = buf[CCE_VMAJOR];
 //  assert(caps.ProtocolRevMin==15); assert(caps.ProtocolRevMaj==2); /// REMOVE FOR PRODUCTION
   _V_ ::fprintf(stdout, "  Camera protocol detected: V %hhd.%hhd \n", caps.ProtocolRevMaj, caps.ProtocolRevMin);

   // get ID
	_byte r; _byte* p = buf; size_t cnt=0;
	do {
		retVal = retVal && dev.ftd->ReadBYTE(r);
		*p++ = r; cnt++;
	} while (r>0 && cnt<sizeof(buf));
	if (!retVal)
		return false; // could not read something - bailing out now

	caps.ID_String.assign((char*)buf, cnt);

	// get User ID (Manufacturer)
	p = buf; cnt=0;
	do {
		retVal = retVal && dev.ftd->ReadBYTE(r);
		*p++ = r; cnt++;
	} while (r>0 && cnt<sizeof(buf));
	if (!retVal)
		return false; // could not read something - bailing out now

	caps.UserID_String.assign((char*)buf, cnt);

	retVal = retVal && dev.ftd->ReadBYTEs(&buf[0], 16, bRead);
	retVal = retVal && (16==bRead);
	if (!retVal)
		return false; // could not read something - bailing out now

   caps.Interlaced = ( ((buf[CCE_FLAGS_LO]) & CCE_FLAGS_LO_INTERLACED) ) ? true : false;
   caps.FIFOfitted = ( ((buf[CCE_FLAGS_LO]) & CCE_FLAGS_LO_FIFO) ) ? true : false;
   caps.TotalPixelsX = (_word)(*(_word*)&buf[CCE_TPIX_X]);
   caps.TotalPixelsY = (_word)(*(_word*)&buf[CCE_TPIX_Y]);
   caps.PixelSizeX = (*(_word*)&buf[CCE_PIXSIZE_X]) / 100.0;
   caps.PixelSizeY = (*(_word*)&buf[CCE_PIXSIZE_Y]) / 100.0;
   caps.MaxBinX = *(_word*)&buf[CCE_MAXBIN_X];
   caps.MaxBinY = *(_word*)&buf[CCE_MAXBIN_Y];
   caps.SyncBinning = (0xff==caps.MaxBinY)?true:false; // y binning must be equal to x binning
   caps.WellCap = (_dword)(*(_word*)&buf[CCE_WELLCAP]) * 1000;
   // we guess the camera type according to the x pixels
   //! @todo does not yet handle the colour types of any type properly
   if (caps.TotalPixelsX==1392) {
      caps.CamType    = ECT_ART285;
      caps.CamModel   = "Art 285";
      caps.SensorType = "ICX-285";  // from specs
      caps.QEPrct     = 50;        // from datasheet
      caps.Noise      = 8;         // from datasheet
      caps.WellCap    = 18000; // from datasheet
      caps.Gain       = 0.45;     // from datasheet
      caps.SensorSizeX = 8.98;     // from datasheet
      caps.SensorSizeY = 6.7;     // from datasheet
   }
   else if (caps.TotalPixelsX==659 ) {
      caps.CamType    = ECT_Art424;
      caps.CamModel   = "ATK-16IC";
      caps.SensorType = "ICX-424";  // from specs
      caps.QEPrct     = 66;        // ??
      caps.Noise      = 7;         // from datasheet
      caps.WellCap    = 24000; // ??
      caps.Gain       = 0.45;     // ??
      caps.SensorSizeX = 4.88;     // from datasheet
      caps.SensorSizeY = 3.66;     // from datasheet
   }
   else if (caps.TotalPixelsX==782 ) {
      caps.CamType    = ECT_Art415;
      caps.CamModel   = "ATK-16IC-S";
      caps.SensorType = "ICX-415";  // from specs
      caps.QEPrct     = 50;        // ?
      caps.Noise      = 6;         // from datasheet
      caps.WellCap    = 55000; // ??
      caps.Gain       = 1.3;     // ??
      caps.SensorSizeX = 6.49;     // from datasheet
      caps.SensorSizeY = 4.83;     // from datasheet
   }
   else if (caps.TotalPixelsX==752 ) {
      caps.CamType    = ECT_ART429;
      caps.CamModel   = "Art 429";
      caps.SensorType = "ICX-429";  // from specs
      caps.QEPrct     = 55;        // from datasheet
      caps.Noise      = 11;         // from datasheet
      caps.WellCap    = 55000; // from datasheet
      caps.Gain       = 1.3;     // from datasheet
      caps.SensorSizeX = 6.47;     // from datasheet
      caps.SensorSizeY = 4.83;     // from datasheet
   }

	return retVal;
}


//! ClearCCD
// Clears the CCD including readout regs
bool Atik16Cmd::CMD_ClearCCD(const AtikDevDescRec& dev)
{
   _V_ ::fprintf(stdout, "Atik16Cmd::CMD_ClearCCD() \n");

	bool retVal;
	size_t bWritten;

	_byte hsc[] = { CMD_, CCMD_ClearCCD };

   retVal = dev.ftd->WriteBYTEs(hsc, sizeof(hsc), bWritten); RetX("WriteBYTEs (CCMD_ClearCCD)", retVal);
   dev.ftd->Sleep_ms(100);

	return retVal;
}

//! Shutter
// Fires the electronic shutter to start the exposure
bool Atik16Cmd::CMD_Shutter(const AtikDevDescRec& dev)
{
   _V_ ::fprintf(stdout, "Atik16Cmd::CMD_Shutter() \n");

	bool retVal;
	size_t bWritten;

	_byte hsc[] = { CMD_, CCMD_Shutter };

   retVal = dev.ftd->WriteBYTEs(hsc, sizeof(hsc), bWritten); RetX("WriteBYTEs (CCMD_Shutter)", retVal);
   dev.ftd->Sleep_ms(100);

if (!retVal) fprintf(stderr, ".. Failed \n");
	return retVal;
}


bool Atik16Cmd::CMD_GetSerialNo(const AtikDevDescRec& dev, unsigned char serNo[])
{
   _V_ ::fprintf(stdout, "Atik16Cmd::CMD_GetSerialNo() \n");

	bool retVal;
	size_t bWritten, bRead;

	_byte hsc[] = { CMD_, CCMD_GetSerialNo };

   retVal = dev.ftd->WriteBYTEs(hsc, sizeof(hsc), bWritten); RetX("WriteBYTEs (CCMD_GetSerialNo)", retVal);

	retVal = retVal && dev.ftd->ReadBYTEs(&serNo[0], 7, bRead);
	if (!retVal)
		return false; // could not read something - bailing out now

   retVal = retVal && (bRead==7); // must read 7 bytes
#ifdef _DEBUG
   _V_ ::fprintf(stdout, "Serial (hex): ");
   for (int i=0; i<6; i++) {
      _V_ ::fprintf(stdout, "%02x-", serNo[i]);
   }
      _V_ ::fprintf(stdout, "HW-index %02x ", serNo[6]);
   _V_ ::fprintf(stdout, "\n");
#endif

	return retVal;
}


bool Atik16Cmd::CMD_GuideOff(const AtikDevDescRec& dev)
{
   _V_ ::fprintf(stdout, "Atik16Cmd::CMD_GuideOff() \n");

	bool retVal;
	size_t bWritten;

	_byte hsc[] = { CMD_, CCMD_GuideOff };

   retVal = dev.ftd->WriteBYTEs(hsc, sizeof(hsc), bWritten); RetX("WriteBYTEs (CCMD_GuideOff)", retVal);
   dev.ftd->Sleep_ms(100);

if (!retVal) fprintf(stderr, ".. Failed \n");
	return retVal;
}

bool Atik16Cmd::CMD_SendExtPort(const AtikDevDescRec& dev, const unsigned char data[], unsigned char dataLen)
{
   _V_ ::fprintf(stdout, "Atik16Cmd::CMD_SendExtPort() \n");

	bool retVal;
	size_t bWritten;
	_byte r;
	_byte hscX[20]; _byte* p_hascX = &hscX[0];

	_byte hsc[] = { CMD_, CCMD_SendExtPort };

   if ( dataLen>(sizeof(hscX)-sizeof(hsc)) ) return false; // cannot write so much

   // copy command
	for (unsigned i=0; i<sizeof(hsc); i++) {
	   *p_hascX++ = hsc[i];
	}
	// copy parameter
	for (unsigned i=0; i<dataLen; i++) {
	   *p_hascX++ = data[i];
	}

   retVal = dev.ftd->WriteBYTEs(hscX, sizeof(hsc)+dataLen, bWritten); RetX("WriteBYTEs (CMD_SendExtPort)", retVal);
   dev.ftd->Sleep_ms(100);
   retVal = retVal && dev.ftd->ReadBYTE(r);
	if (!retVal)
		return false; // could not read something - bailing out now
   retVal = retVal && (r==1); // must get a 1

	return retVal;
}


//! Read CCD
/*
The single command read CCD gives access to all the cameras read out modes. The
command will turn on the ADC, turn on the amp and for self timed exposures clear the CCD.
Returns NUMX / XBin  * NUMY / YBin  16 bit words as image array (CCD)
*/

// send the rccd command buffer
// returns WORDs of image (low byte then high byte)
bool Atik16Cmd::CMD_ReadCCD(const AtikDevDescRec& dev, unsigned char rccd[])
{
   _V_ ::fprintf(stdout, "Atik16Cmd::CMD_ReadCCD() \n");
#ifdef _DEBUG
   _V_ ::fprintf(stdout, "Buffer (hex): ");
   for (int i=0; i<CRE_SIZE; i++) {
      _V_ ::fprintf(stdout, "%02x ", rccd[i]);
   }
   _V_ ::fprintf(stdout, "\n");
#endif
	bool retVal;
	size_t bWritten;

   retVal = dev.ftd->WriteBYTEs(rccd, CRE_SIZE, bWritten); RetX("WriteBYTEs (CCMD_ReadCCD)", retVal);
   dev.ftd->Sleep_ms(100);

if (!retVal) fprintf(stderr, ".. Failed \n");
  return retVal;
}


// prepare the ReadCCD Buffer from camExp
void Atik16Cmd::PrepReadCCD(AtikCamExposureRec& camExp, unsigned char rccd[])
{
	_word ccdsx, ccdsy, ccdh, ccdw, binx, biny;

  // setup the Read command buffer
   rccd[CRE_C] = 'C'; rccd[CRE_M] = 'M'; rccd[CRE_D] = 'D';
   rccd[CRE_COMMAND] = CCMD_ReadCCD;

	/*
	Exposure has 1ms items - translate to camera ones
		Exposure Time
			0 or >=SHORTEXPOLIMIT  exposure timed on PC so just send the data
			1-9 Exposure timed in the camera. Value = exposure time in mS
			10-255 Exposure timed in camera. Exposure time = (Value-9)*10ms
	*/
	if (camExp.Exposure==0) {
		rccd[CRE_EXPO] = (_byte)(0);
		camExp.ImageConfig &= ~EICB_AmpOn;  // clear short image
	}
	else if (camExp.Exposure<10) {
		rccd[CRE_EXPO] = (_byte)(camExp.Exposure & 0xff);
		camExp.ImageConfig |= EICB_AmpOn;
	}
	else if (camExp.Exposure<SHORTEXPOLIMIT) {
		rccd[CRE_EXPO] = (_byte)( ((camExp.Exposure/10)+9) & 0xff );
		camExp.ImageConfig |= EICB_AmpOn;
	}
	else { // SHORTEXPOLIMIT
		rccd[CRE_EXPO] = (_byte)(0);  // must have used external timing
		camExp.ImageConfig &= ~EICB_AmpOn;  // clear short image
	}

	binx  = camExp.XBin;   biny  = camExp.YBin;
	ccdsx = camExp.XStart; ccdsy = camExp.YStart;
	ccdw  = camExp.NumX;   ccdh  = camExp.NumY;

   // make sure ccdh and ccdsy gets even after binning and interlace
   ccdsx/=binx*2; ccdsx*=binx*2;
   ccdsy/=biny*2; ccdsy*=biny*2;
   ccdw/=binx*2; ccdw*=binx*2;
   ccdh/=biny*2; ccdh*=biny*2;
   camExp.XStart = ccdsx; // write back
   camExp.YStart = ccdsy; // write back
   camExp.SetNum(ccdw, ccdh); // write eff size back

   // setup the transfer buffer
	rccd[CRE_XBIN] = (_byte)(binx & 0xff);
	rccd[CRE_XSTART_LO] = (_byte)(ccdsx & 0xff);
	rccd[CRE_XSTART_HI] = (_byte)((ccdsx & 0xff00) >> 8);
	rccd[CRE_NUMX_LO] = (_byte)(ccdw & 0xff);
	rccd[CRE_NUMX_HI] = (_byte)((ccdw & 0xff00) >> 8);

	rccd[CRE_YBIN] = (_byte)(biny & 0xff);
	rccd[CRE_YSTART_LO] = (_byte)(ccdsy & 0xff);
	rccd[CRE_YSTART_HI] = (_byte)((ccdsy & 0xff00) >> 8);
	rccd[CRE_NUMY_LO] = (_byte)(ccdh & 0xff);
	rccd[CRE_NUMY_HI] = (_byte)((ccdh & 0xff00) >> 8);

   if (camExp.CamCaps.Interlaced) // interlaced CCD
	{
		// get the half height only
		ccdsy/=2; ccdh/=2;
		rccd[CRE_YSTART_LO] = (_byte)(ccdsy & 0xff);
		rccd[CRE_YSTART_HI] = (_byte)((ccdsy & 0xff00) >> 8);
		rccd[CRE_NUMY_LO] = (_byte)(ccdh & 0xff);
		rccd[CRE_NUMY_HI] = (_byte)((ccdh & 0xff00) >> 8);
		if ((camExp.ImageConfig & EICB_DeInterlace) != EICB_DeInterlace)
		{
			// the size is half too if not to be deinterlaced
			//camExp.ImageConfig &= (~CTP2);
         assert(false); // should not be there
		}
	}

	rccd[CRE_IMGCFG_LO] = (_byte)(camExp.ImageConfig & 0xff);
	rccd[CRE_IMGCFG_HI] = (_byte)((camExp.ImageConfig & 0xff00) >> 8);

	if ((camExp.ImageConfig & EICB_PcCheck) == EICB_Precharge) {// no IPCS
      assert(false); // should not be there
	}

	// once the thread is ready it will trigger the READCCD with the
	// parameters set up above  i.e. sending rccdBuf
}


bool Atik16Cmd::ReadImageBytes(const AtikDevDescRec& dev, unsigned char buf[], size_t bufSize, size_t& bRead, unsigned short mSeconds)
{
	dev.ftd->SetMaxBuffer(); // for reading the ccd
   bool retVal = dev.ftd->ReadBYTEs(buf, bufSize, bRead);

   if (!retVal) ::fprintf(stderr, "Atik16Cmd::ReadImageBytes() - Failed \n");
   return retVal;
}





