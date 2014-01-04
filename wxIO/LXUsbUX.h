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
 * @file           LXUsbUX.h
 *****************************************************************************
 * class LXUsbUX
 *
 * Shoestring Astronomy Appliance driver for the LXUSB product 
 * Linux upper level driver
 *
 * Defines the Linux upper level driver
 *  this class is derived from the interface class LXUsb
 *  it implements the API style interface which is OS dependent
 *  This is the Linux implementation using SAUSB_LibUsb
 * 
 * This is part of the Webcam Driver Library of wxAstroCapture
 *
 * Copyright (C) 2009 Martin Burri  (bm98@burri-web.org)
 * 
 * Pls see notes in SAUSB_LibUsb.h for information about requirements
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

#ifndef _LXUSBW32_H_
#define _LXUSBW32_H_

#include "LXUsb.h"

class SAUSB_LibUsb; // forward declaration

class LXUsbUX : public LXUsb
{
public:
   LXUsbUX();
   virtual ~LXUsbUX();

   /// @brief Sleep for an amount of milliseconds
   void Sleep_ms(unsigned short  mseconds); // OS independent sleep

public:
   /// LXUSB Win API style interface
   // Copied from ShoestringLXUSB_DLL.h
    bool LXUSB_Open( void );
    bool LXUSB_Close( void );
    bool LXUSB_Reset( void );
    bool LXUSB_Frame1Assert( void );
    bool LXUSB_Frame1Deassert( void );
    bool LXUSB_Frame2Assert( void );
    bool LXUSB_Frame2Deassert( void );
    bool LXUSB_ShutterAssert( void );
    bool LXUSB_ShutterDeassert( void );
    bool LXUSB_CCDAmpAssert( void );
    bool LXUSB_CCDAmpDeassert( void );
    bool LXUSB_AllControlDeassert( void );
    bool LXUSB_LEDOn( void );
    bool LXUSB_LEDOff( void );
    bool LXUSB_LEDRed( void );
    bool LXUSB_LEDGreen( void );
    bool LXUSB_SetAll(  int frame1, int frame2, int shutter, int ccdamp, int led );
    bool LXUSB_Frame1Status( int *status );
    bool LXUSB_Frame2Status( int *status );
    bool LXUSB_ShutterStatus( int *status );
    bool LXUSB_CCDAmpStatus( int *status );
    bool LXUSB_LEDStatus( int *status );
    bool LXUSB_Status( int *frame1_status, int *frame2_status, int *shutter_status, int *ccdamp_status, int *led_status );

private:
   SAUSB_LibUsb* m_p_lxusb; // lower level driver instance
   unsigned char m_ctrl;    // actual local data byte
};

#endif // _LXUSBW32_H_
