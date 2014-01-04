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
 * @file           GPUsb.h
 *****************************************************************************
 * class GPUsb
 *
 * Shoestring Astronomy Appliance driver for the GPUSB product
 *
 * Implements a simple OS independent API for the GPUSB device
 * Defines the native API which is implemented by OS dependent code
 * 
 * This is part of the Guiding Driver Library of wxAstroCapture
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


#ifndef _GPUSB_H_
#define _GPUSB_H_

class GPUsb
{
friend class GPUsbUX;  // allow usage of private items
friend class GPUsbW32; // allow usage of private items

public:
   /// @brief returns a new driver instance (allows OS independent code)
   /// NOTE: caller is responsible to delete the received object
  static GPUsb*   Create();

  /// @brief Sleep for an amount of milliseconds
  virtual void Sleep_ms(unsigned short  mseconds) =0; // OS independent sleep

public:
   /// GPUSB Win API style interface
   // Copied from ShoestringGPUSB_DLL.h

   // These are possible return values for the shutter status in GPUSB_RAMStatus and GPUSB_Status
   // They are also the possible input values for the ram parameter of GPUSB_SetAll
   static const int GPUSB_RAM_ASSERTED;
   static const int GPUSB_RAM_DEASSERTED;

   // These are possible return values for the shutter status in GPUSB_RAPStatus and GPUSB_Status
   // They are also the possible input values for the rap parameter of GPUSB_SetAll
   static const int GPUSB_RAP_ASSERTED;
   static const int GPUSB_RAP_DEASSERTED;

   // These are possible return values for the shutter status in GPUSB_DecMStatus and GPUSB_Status
   // They are also the possible input values for the decm parameter of GPUSB_SetAll
   static const int GPUSB_DECM_ASSERTED;
   static const int GPUSB_DECM_DEASSERTED;

   // These are possible return values for the shutter status in GPUSB_DecPStatus and GPUSB_Status
   // They are also the possible input values for the decp parameter of GPUSB_SetAll
   static const int GPUSB_DECP_ASSERTED;
   static const int GPUSB_DECP_DEASSERTED;

   // These are possible return values for the LED status in GPUSB_LEDStatus and GPUSB_Status
   // They are also the possible input values for the led parameter of GPUSB_SetAll
   static const int GPUSB_LED_ON_RED;
   static const int GPUSB_LED_ON_GREEN;
   static const int GPUSB_LED_OFF_RED;
   static const int GPUSB_LED_OFF_GREEN;

   // implemented by the OS specific part
   virtual bool GPUSB_Open( void ) =0;
   virtual bool GPUSB_Close( void ) =0;
   virtual bool GPUSB_Reset( void ) =0;
   virtual bool GPUSB_RAMAssert( void ) =0;
   virtual bool GPUSB_RAMDeassert( void ) =0;
   virtual bool GPUSB_RAPAssert( void ) =0;
   virtual bool GPUSB_RAPDeassert( void ) =0;
   virtual bool GPUSB_DecMAssert( void ) =0;
   virtual bool GPUSB_DecMDeassert( void ) =0;
   virtual bool GPUSB_DecPAssert( void ) =0;
   virtual bool GPUSB_DecPDeassert( void ) =0;
   virtual bool GPUSB_AllDirDeassert( void ) =0;
   virtual bool GPUSB_LEDOn( void ) =0;
   virtual bool GPUSB_LEDOff( void ) =0;
   virtual bool GPUSB_LEDRed( void ) =0;
   virtual bool GPUSB_LEDGreen( void ) =0;
   virtual bool GPUSB_SetAll( int ram, int rap, int decm, int decp, int led ) =0;
   virtual bool GPUSB_RAMStatus( int *status ) =0;
   virtual bool GPUSB_RAPStatus( int *status ) =0;
   virtual bool GPUSB_DecMStatus( int *status ) =0;
   virtual bool GPUSB_DecPStatus( int *status ) =0;
   virtual bool GPUSB_LEDStatus( int *status ) =0;
   virtual bool GPUSB_Status( int *ram_status, int *rap_status, int *decm_status, int *decp_status, int *led_status ) =0;



public:
   /// Trivial byte style interface

   /// @brief Returns true the device is OK and usable
   bool Open();

   /// @brief Returns if the device is OK and usable
   /// @return Returns true if the device is OK and opened
   bool BoardAvailable();

   /// @brief Closes the device
   void Close();

   /// it assumes the bits b0..b3 --> RA-, RA+, DEC-, DEC+ (LED handling is intrinsic)
   /// and bit = 1 is enabled i.e. the line is driven and guiding takes place

   /// @brief The bits of the trivial interface
   enum EGVALS {
      EGV_None =0,
      EGV_RAn  =1,  // RA-
      EGV_RAp  =2,  // RA+
      EGV_DECn =4,  // DEC-
      EGV_DECp =8,  // DEC+
      EGV_All  =15,
   };

   /// @brief Asserts the guiding line when the bit is 1
   bool SetValue(unsigned short val);
   /// @brief Returns the guiding bits
   bool GetValue(unsigned short& val);

protected:
   GPUsb();

private:
   /// note: true = signal is enabled
   bool BoardValues(bool& ran, bool& rap, bool& decn, bool& decp, bool& led, bool& ledRed);
   bool SetRADEC(bool ran, bool rap, bool decn, bool decp);

   bool m_board_present;
};

#endif // _GPUSB_H_
