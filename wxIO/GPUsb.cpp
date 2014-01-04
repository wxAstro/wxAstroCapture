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
 * @file           GPUsb.cpp
 *****************************************************************************
 * class GPUsb
 *
 * Shoestring Astronomy Appliance driver for the GPUSB product
 *
 * Implements a simple OS independent API for the GPUSB device
 *
 * This is part of the Guiding Driver Library of wxAstroCapture
 *
 * Copyright (C) 2009 Martin Burri  (bm98@burri-web.org)
 *
 * Pls see notes in SAUSB_LibUsb for information about requirements
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

#include "GPUsb.h"

// OS dependent include
#ifdef WIN32
 #include "GPUsbW32.h"
#else // LINUX
 #include "GPUsbUX.h"
#endif

/// Copied from ShoestringGPUSB_DLL.h
// These are possible return values for the shutter status in GPUSB_RAMStatus and GPUSB_Status
// They are also the possible input values for the ram parameter of GPUSB_SetAll
const int GPUsb::GPUSB_RAM_ASSERTED = 2;
const int GPUsb::GPUSB_RAM_DEASSERTED = 1;

// These are possible return values for the shutter status in GPUSB_RAPStatus and GPUSB_Status
// They are also the possible input values for the rap parameter of GPUSB_SetAll
const int GPUsb::GPUSB_RAP_ASSERTED = 2;
const int GPUsb::GPUSB_RAP_DEASSERTED = 1;

// These are possible return values for the shutter status in GPUSB_DecMStatus and GPUSB_Status
// They are also the possible input values for the decm parameter of GPUSB_SetAll
const int GPUsb::GPUSB_DECM_ASSERTED = 2;
const int GPUsb::GPUSB_DECM_DEASSERTED = 1;

// These are possible return values for the shutter status in GPUSB_DecPStatus and GPUSB_Status
// They are also the possible input values for the decp parameter of GPUSB_SetAll
const int GPUsb::GPUSB_DECP_ASSERTED = 2;
const int GPUsb::GPUSB_DECP_DEASSERTED = 1;

// These are possible return values for the LED status in GPUSB_LEDStatus and GPUSB_Status
// They are also the possible input values for the led parameter of GPUSB_SetAll
const int GPUsb::GPUSB_LED_ON_RED = 4;
const int GPUsb::GPUSB_LED_ON_GREEN = 3;
const int GPUsb::GPUSB_LED_OFF_RED = 2;
const int GPUsb::GPUSB_LED_OFF_GREEN = 1;


/// @brief returns a new driver instance (allows OS independent code)
/// NOTE: caller is responsible to delete the received object
GPUsb* GPUsb::Create()
{
#ifdef WIN32
	return new GPUsbW32();
#else // LINUX
	return new GPUsbUX();
#endif
}

GPUsb::GPUsb()
: m_board_present(false)
{
}

bool GPUsb::BoardAvailable()
{
   return m_board_present; // maintained in OS specific part
}

bool GPUsb::Open()
{
   GPUSB_Close();
   GPUSB_Open();

   GPUSB_LEDGreen();
   GPUSB_LEDOn();

   return m_board_present;
}

void GPUsb::Close()
{
   GPUSB_Close();
}

// trivialized byte interface
// it assumes the b0..b3 --> RA-, RA+, DEC-, DEC+
//  and bit = 1 is enabled i.e. the line is driven and guiding takes place
bool GPUsb::SetValue(unsigned short val)
{
   bool retVal = SetRADEC(bool((val&EGV_RAn)==EGV_RAn), bool((val&EGV_RAp)==EGV_RAp),
                          bool((val&EGV_DECn)==EGV_DECn), bool((val&EGV_DECp)==EGV_DECp));

   return retVal;
}

bool GPUsb::GetValue(unsigned short& val)
{
   bool ran, rap, den, dep, led, ledRed;

   bool retVal =  BoardValues(ran, rap, den, dep, led, ledRed); // get from device
   // derive bits
   val = (unsigned short)( ((ran) ? EGV_RAn:0) + ((rap) ? EGV_RAp:0) + ((den) ? EGV_DECn:0) + ((dep) ? EGV_DECp:0) );
   return retVal;
}


bool GPUsb::BoardValues(bool& ran, bool& rap, bool& decn, bool& decp, bool& led, bool& ledRed)
{
   int ram_status, rap_status, decm_status, decp_status, led_status;

   bool retVal = GPUSB_Status(&ram_status, &rap_status, &decm_status, &decp_status, &led_status);
   ran=(ram_status==GPUSB_RAM_ASSERTED);
   rap=(rap_status==GPUSB_RAP_ASSERTED);
   decn=(decm_status==GPUSB_DECM_ASSERTED);
   decp=(decp_status==GPUSB_DECP_ASSERTED);
   led=   (led_status==GPUSB_LED_ON_RED) || (led_status==GPUSB_LED_ON_GREEN);
   ledRed=(led_status==GPUSB_LED_ON_RED) || (led_status==GPUSB_LED_OFF_RED);

   return retVal;
}

//! LED policy:
//! LED is off if port is opened but not active
//! LED is green if port is active
//! LED blinks red if guiding command is issued
//! LED stays red on error
bool GPUsb::SetRADEC(bool ran, bool rap, bool decn, bool decp)
{
   if (ran && rap) return false;   // shall not Guide both RA- AND RA+
   if (decn && decp) return false; // shall not Guide both DEC- AND DEC+

   bool retVal = true;

   bool gLed = (ran || rap || decn || decp); // show red led for any guiding command
   int ra_m, ra_p, de_m, de_p, led;  // IF values

   ra_m= (ran)  ? GPUSB_RAM_ASSERTED  : GPUSB_RAM_DEASSERTED;
   ra_p= (rap)  ? GPUSB_RAP_ASSERTED  : GPUSB_RAP_DEASSERTED;
   de_m= (decn) ? GPUSB_DECM_ASSERTED : GPUSB_DECM_DEASSERTED;
   de_p= (decp) ? GPUSB_DECP_ASSERTED : GPUSB_DECP_DEASSERTED;
   led = (gLed) ? GPUSB_LED_ON_RED  : GPUSB_LED_ON_GREEN;

   retVal = GPUSB_SetAll(ra_m, ra_p, de_m, de_p, led);

   return retVal;
}
