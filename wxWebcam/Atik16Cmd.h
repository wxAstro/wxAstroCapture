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
 * @file           Atik16Cmd.h
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


#ifndef _ATIK16CMD_H_
#define _ATIK16CMD_H_

#include "ArtTypes.h" // enums and structs of the Atik/Artemis environment

#include <string>

class Ftd245IO;

class Atik16Cmd
{
public:
	Atik16Cmd();
	virtual ~Atik16Cmd();

public:

   enum ECamType {
      ECT_Unknown,
      ECT_ART285,
      ECT_ART429,
      ECT_Art424, // ATK-16IC
      ECT_Art415, // ATK-16IC-S
   };
   struct AtikCamCapsRec
   {
      AtikCamCapsRec() {Init();};
      void Init() {
         ProtocolRevMin=0;  ProtocolRevMaj=0;
         ID_String.clear(); UserID_String.clear();
         Interlaced=false;  FIFOfitted=false; ExtTrigger=false;
         Preview=false;     Subsample=false;  FanControl=false;
         TotalPixelsX=0;    TotalPixelsY=0;
         PixelSizeX=0.0;    PixelSizeY=0.0;
         SensorSizeX=0.0;   SensorSizeY=0.0;
         MaxBinX=0;         MaxBinY=0;          SyncBinning=true;
         WellCap=0;         CamType=ECT_Unknown; Gain = 0.0;
         QEPrct = 0;        Noise = 0;
         SensorType.clear();CamModel.clear();
      };
      unsigned char  ProtocolRevMin; unsigned char  ProtocolRevMaj;
      std::string    ID_String;      std::string    UserID_String;
      bool           Interlaced;     bool           FIFOfitted;
      bool           ExtTrigger;     bool           Preview;
      bool           Subsample;      bool           FanControl;
      unsigned short TotalPixelsX;   unsigned short TotalPixelsY;
      float          PixelSizeX;     float PixelSizeY; // microns
      bool           SyncBinning;   // Y and X bin level cannot be different
      float          SensorSizeX;    float          SensorSizeY;  // mm
      unsigned short MaxBinX;        unsigned short MaxBinY;
      unsigned long  WellCap;
      ECamType       CamType;        std::string    CamModel;
      unsigned short QEPrct;
      unsigned short Noise;
      float          Gain;
      std::string    SensorType;
   };

   struct AtikCamExposureRec
   {
      AtikCamExposureRec() {Init();};
      void Init() {
         // setting some useful defaults
         Exposure = 2;                   // 10's of ms if 0 no shutter
         XBin = 1;         YBin = 1;
         XStart = 100;     YStart = 100; // start of pixels 16bit subframe definition
         SetNum(200,200);                // num of pixels 16bit subframe definition
         ImageConfig = EICB_CTPall | EICB_IpcsMode; // standard caps cam independent
      };
      // use either SetEnd or SetNum to maintain all numbers!!
      void SetEnd(unsigned short xend, unsigned short yend) {
         XEnd = xend; YEnd = yend;  NumX = XEnd-XStart; NumY = YEnd-YStart;
      };
      void SetNum(unsigned short numx, unsigned short numy) {
         NumX = numx; NumY = numy;  XEnd = XStart+NumX; YEnd = YStart+NumY;
      };
      size_t ImageBytes(){return (2*(size_t)NumX*(size_t)NumY / (size_t)XBin / (size_t)YBin);};
      size_t ImagePixels(){return ((size_t)NumX*(size_t)NumY / (size_t)XBin / (size_t)YBin);};

      unsigned long  Exposure;      // ms if 0 no shutter
      unsigned short XBin;          // binning level
      unsigned short YBin;
      unsigned short XStart;        // start of pixels 16bit subframe definition 0-based
      unsigned short YStart;
      unsigned short XEnd;          // end pos of 16bit subframe definition not including
      unsigned short YEnd;          //    = XStart+ NumX or YStart + NumY
      unsigned short NumX;          // num of pixels 16bit subframe definition
      unsigned short NumY;          //    = XEnd - XStart or YEnd - YStart
      unsigned short ImageConfig;   // see EImageConfigBits EICBH_xyz
      AtikCamCapsRec CamCaps;       // copy of the cam caps record
   };

   struct AtikDevDescRec{
      AtikDevDescRec() {Init();};
      void Init() {
         // setting some useful defaults
         ftd=NULL;
      };
      bool DevOK()   const {return (NULL!=ftd);};
      bool DevNULL() const {return (NULL==ftd);};

      Ftd245IO*    ftd;
   };



    //! All Firmware calls - to be sent via USB interface

   //! Reset   Resets the Camera and everything
   static bool CMD_RESET(const AtikDevDescRec& dev);

   static bool CMD_PICPing(const AtikDevDescRec& dev);
   static bool CMD_AppPing(const AtikDevDescRec& dev, unsigned char &pingVal);
   static bool CMD_FIFO_Fitted(const AtikDevDescRec& dev, bool& fifofitted);
   static bool CMD_RequestCAPS(const AtikDevDescRec& dev, AtikCamCapsRec &caps);

   static bool CMD_Amplifier(const AtikDevDescRec& dev, bool ampState);
   static bool CMD_ClearCCD(const AtikDevDescRec& dev);
   static bool CMD_Shutter(const AtikDevDescRec& dev);

   //! read in the serial no - provide at least 7 char buffer
   static bool CMD_GetSerialNo(const AtikDevDescRec& dev, unsigned char serNo[]);
   //! switch all guiding off
   static bool CMD_GuideOff(const AtikDevDescRec& dev);
   //! send bytes to the external IO port (max 16 char)
   static bool CMD_SendExtPort(const AtikDevDescRec& dev, const unsigned char data[], unsigned char dataLen);

   // prepare the ReadCCD Buffer from camExp
   // NOTE: camExp content (frame size) is adjusted if needed for even sizes
   static void PrepReadCCD(AtikCamExposureRec& camExp, unsigned char rccd[]);
   // send the rccd command buffer
   static bool CMD_ReadCCD(const AtikDevDescRec& dev,  unsigned char rccd[]);
   static bool ReadImageBytes(const AtikDevDescRec& dev, unsigned char buf[], size_t bufSize, size_t& bRead, unsigned short mSeconds);

   static const unsigned long SHORTEXPOLIMIT = 2460;  // ms for in cam exposure

private:

};

#endif  // _ATIK16CMD_H_
