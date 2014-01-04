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
 * @file           GPUsbUX.h
 *****************************************************************************
 * class GPUsbUX
 *
 * Shoestring Astronomy Appliance driver for the GPUSB product 
 * Linux upper level driver
 *
 * Defines the Linux upper level driver
 *  this class is derived from the interface class GPUsb
 *  it implements the API style interface which is OS dependent
 *  This is the Linux implementation using SAUSB_LibUsb
 * 
 * This is part of the Guiding Driver Library of wxAstroCapture
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

#ifndef _GPUSBUX_H_
#define _GPUSBUX_H_

#include "GPUsb.h"

class SAUSB_LibUsb; // forward declaration

class GPUsbUX : public GPUsb
{
public:
   GPUsbUX();
   virtual ~GPUsbUX();

   /// @brief Sleep for an amount of milliseconds
   void Sleep_ms(unsigned short  mseconds); // OS independent sleep

public:
   /// GPUSB Win API style interface
   // Copied from ShoestringGPUSB_DLL.h - see API doc for details
   bool GPUSB_Open( void );
   bool GPUSB_Close( void );
   bool GPUSB_Reset( void );
   bool GPUSB_RAMAssert( void );
   bool GPUSB_RAMDeassert( void );
   bool GPUSB_RAPAssert( void );
   bool GPUSB_RAPDeassert( void );
   bool GPUSB_DecMAssert( void );
   bool GPUSB_DecMDeassert( void );
   bool GPUSB_DecPAssert( void );
   bool GPUSB_DecPDeassert( void );
   bool GPUSB_AllDirDeassert( void );
   bool GPUSB_LEDOn( void );
   bool GPUSB_LEDOff( void );
   bool GPUSB_LEDRed( void );
   bool GPUSB_LEDGreen( void );
   bool GPUSB_SetAll( int ram, int rap, int decm, int decp, int led );
   bool GPUSB_RAMStatus( int *status );
   bool GPUSB_RAPStatus( int *status );
   bool GPUSB_DecMStatus( int *status );
   bool GPUSB_DecPStatus( int *status );
   bool GPUSB_LEDStatus( int *status );
   bool GPUSB_Status( int *ram_status, int *rap_status, int *decm_status, int *decp_status, int *led_status );

private:
   SAUSB_LibUsb* m_p_gpusb; // lower level driver instance
   unsigned char m_ctrl;    // actual local data byte
};

#endif // _GPUSBUX_H_
