/**

  Shoestring Appliance - Control Library

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
 * @file           LXUsb.cpp
 *****************************************************************************
 * class GPUsb
 *
 * Shoestring Astronomy Appliance driver for the LXUSB product
 *
 * Implements a simple OS independent API for the LXUSB device
 *
 * This is part of the Webcam Driver Library of wxAstroCapture
 *
 * Copyright (C) 2009 Martin Burri  (bm98@burri-web.org)
 *
 *<hr>
 *
 * @b Project      wxAstroCapture<br>
 *
 * @author         M. Burri
 * @date           10-Jan-2009
 *
 *****************************************************************************
 *<hr>
 * @b Updates
 * - dd-mmm-yyyy V. Name: Description
 *
 *****************************************************************************/

#include "LXUsb.h"

// OS dependent include
#ifdef WIN32
 #include "LXUsbW32.h"
#else // LINUX
 #include "LXUsbUX.h"
#endif

/// Copied from ShoestringLXUSB_DLL.h
// These are possible return values for the shutter status in LXUSB_Frame1Status and LXUSB_Status
// They are also the possible input values for the frame1 parameter of LXUSB_SetAll
const int LXUsb::LXUSB_FRAME1_ASSERTED = 2;
const int LXUsb::LXUSB_FRAME1_DEASSERTED = 1;

// These are possible return values for the shutter status in LXUSB_Frame2Status and LXUSB_Status
// They are also the possible input values for the frame2 parameter of LXUSB_SetAll
const int LXUsb::LXUSB_FRAME2_ASSERTED = 2;
const int LXUsb::LXUSB_FRAME2_DEASSERTED = 1;

// These are possible return values for the shutter status in LXUSB_ShutterStatus and LXUSB_Status
// They are also the possible input values for the shutter parameter of LXUSB_SetAll
const int LXUsb::LXUSB_SHUTTER_ASSERTED = 2;
const int LXUsb::LXUSB_SHUTTER_DEASSERTED = 1;

// These are possible return values for the shutter status in LXUSB_CCDAmpStatus and LXUSB_Status
// They are also the possible input values for the ccdamp parameter of LXUSB_SetAll
const int LXUsb::LXUSB_CCDAMP_ASSERTED = 2;
const int LXUsb::LXUSB_CCDAMP_DEASSERTED = 1;

// These are possible return values for the LED status in LXUSB_LEDStatus and LXUSB_Status
// They are also the possible input values for the led parameter of LXUSB_SetAll
const int LXUsb::LXUSB_LED_ON_RED = 4;
const int LXUsb::LXUSB_LED_ON_GREEN = 3;
const int LXUsb::LXUSB_LED_OFF_RED = 2;
const int LXUsb::LXUSB_LED_OFF_GREEN = 1;



/// @brief returns a new driver instance (allows OS independent code)
/// NOTE: caller is responsible to delete the received object
LXUsb* LXUsb::Create()
{
#ifdef WIN32
	return new LXUsbW32();
#else // LINUX
	return new LXUsbUX();
#endif
}

LXUsb::LXUsb()
: m_board_present(false)
{
}

bool LXUsb::BoardAvailable()
{
   return m_board_present; // maintained in OS specific part
}

bool LXUsb::Open()
{
   LXUSB_Close();
   LXUSB_Open();
   return m_board_present;
}

void LXUsb::Close()
{
   LXUSB_Close();
}


// trivialized byte interface
// it assumes the b0..b3 --> Frame1, Frame2, Shutter, Amp
//  and bit = 1 is enabled (same logic as the parallel port has)
bool LXUsb::SetValue(unsigned short val)
{
   bool retVal = SetControls(bool((val&ECV_F1)==ECV_F1), bool((val&ECV_F2)==ECV_F2),
                             bool((val&ECV_Shutter)==ECV_Shutter), bool((val&ECV_Amp)==ECV_Amp));
   return retVal;
}

bool LXUsb::GetValue(unsigned short& val)
{
   bool f1, f2, shut, amp, led, ledRed;
   bool retVal =  BoardValues(f1, f2, shut, amp, led, ledRed); // get from device
   val = (unsigned short)( ((f1) ? ECV_F1:0) + ((f2) ? ECV_F2:0) + ((shut) ? ECV_Shutter:0) + ((amp) ? ECV_Amp:0) );
   return retVal;
}


bool LXUsb::BoardValues(bool& f1, bool& f2, bool& shutter, bool& amp, bool& led, bool& ledRed)
{
   int frame1_status, frame2_status, shutter_status, ccdamp_status, led_status;

   bool retVal = LXUSB_Status(&frame1_status, &frame2_status, &shutter_status, &ccdamp_status, &led_status);
   f1=(frame1_status==LXUSB_FRAME1_ASSERTED);
   f2=(frame2_status==LXUSB_FRAME2_ASSERTED);
   shutter=(shutter_status==LXUSB_SHUTTER_ASSERTED);
   amp=(ccdamp_status==LXUSB_CCDAMP_ASSERTED);
   led=   (led_status==LXUSB_LED_ON_RED) || (led_status==LXUSB_LED_ON_GREEN);
   ledRed=(led_status==LXUSB_LED_ON_RED) || (led_status==LXUSB_LED_OFF_RED);

   return retVal;
}


//  and bit = 1 is enabled (same logic as the parallel port has)
// which in LXUSB terms means the line is DEASSERTED
bool LXUsb::SetControls(bool f1, bool f2, bool shutter, bool amp)
{
   bool retVal = true;

   bool gLed = !(f1 && f2 && shutter && amp); // show red led for any cam control command i.e. one is false

   int f_1, f_2, s_hut, a_mp, led;  // IF values

   f_1= (f1)  ? LXUSB_FRAME1_DEASSERTED  : LXUSB_FRAME1_ASSERTED;
   f_2= (f2)  ? LXUSB_FRAME2_DEASSERTED  : LXUSB_FRAME2_ASSERTED;
   s_hut= (shutter) ? LXUSB_SHUTTER_DEASSERTED : LXUSB_SHUTTER_ASSERTED;
   a_mp= (amp) ? LXUSB_CCDAMP_DEASSERTED : LXUSB_CCDAMP_ASSERTED;
   led = (gLed) ? LXUSB_LED_ON_RED  : LXUSB_LED_ON_GREEN;

   retVal = LXUSB_SetAll(f_1, f_2, s_hut, a_mp, led);

   return retVal;
}


