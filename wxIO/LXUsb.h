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
 * @file           LXUsb.h
 *****************************************************************************
 * class GPUsb
 *
 * Shoestring Astronomy Appliance driver for the LXUSB product
 *
 * Implements a simple OS independent API for the LXUSB device
 * Defines the native API which is implemented by OS dependent code
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

#ifndef _LXUSB_H_
#define _LXUSB_H_

class LXUsb
{
friend class LXUsbUX;  // allow usage of private items
friend class LXUsbW32; // allow usage of private items

public:
   /// @brief returns a new driver instance (allows OS independent code)
   /// NOTE: caller is responsible to delete the received object
  static LXUsb*   Create();

  /// @brief Sleep for an amount of milliseconds
  virtual void Sleep_ms(unsigned short  mseconds) =0; // OS independent sleep

public:
   /// LXUSB Win API style interface
   // Copied from ShoestringLXUSB_DLL.h

   // These are possible return values for the shutter status in LXUSB_Frame1Status and LXUSB_Status
   // They are also the possible input values for the frame1 parameter of LXUSB_SetAll
   static const int LXUSB_FRAME1_ASSERTED;
   static const int LXUSB_FRAME1_DEASSERTED;

   // These are possible return values for the shutter status in LXUSB_Frame2Status and LXUSB_Status
   // They are also the possible input values for the frame2 parameter of LXUSB_SetAll
   static const int LXUSB_FRAME2_ASSERTED;
   static const int LXUSB_FRAME2_DEASSERTED;

   // These are possible return values for the shutter status in LXUSB_ShutterStatus and LXUSB_Status
   // They are also the possible input values for the shutter parameter of LXUSB_SetAll
   static const int LXUSB_SHUTTER_ASSERTED;
   static const int LXUSB_SHUTTER_DEASSERTED;

   // These are possible return values for the shutter status in LXUSB_CCDAmpStatus and LXUSB_Status
   // They are also the possible input values for the ccdamp parameter of LXUSB_SetAll
   static const int LXUSB_CCDAMP_ASSERTED;
   static const int LXUSB_CCDAMP_DEASSERTED;

   // These are possible return values for the LED status in LXUSB_LEDStatus and LXUSB_Status
   // They are also the possible input values for the led parameter of LXUSB_SetAll
   static const int LXUSB_LED_ON_RED;
   static const int LXUSB_LED_ON_GREEN;
   static const int LXUSB_LED_OFF_RED;
   static const int LXUSB_LED_OFF_GREEN;

   // implemented by the OS specific part
   virtual bool LXUSB_Open( void ) =0;
   virtual bool LXUSB_Close( void ) =0;
   virtual bool LXUSB_Reset( void ) =0;
   virtual bool LXUSB_Frame1Assert( void ) =0;
   virtual bool LXUSB_Frame1Deassert( void ) =0;
   virtual bool LXUSB_Frame2Assert( void ) =0;
   virtual bool LXUSB_Frame2Deassert( void ) =0;
   virtual bool LXUSB_ShutterAssert( void ) =0;
   virtual bool LXUSB_ShutterDeassert( void ) =0;
   virtual bool LXUSB_CCDAmpAssert( void ) =0;
   virtual bool LXUSB_CCDAmpDeassert( void ) =0;
   virtual bool LXUSB_AllControlDeassert( void ) =0;
   virtual bool LXUSB_LEDOn( void ) =0;
   virtual bool LXUSB_LEDOff( void ) =0;
   virtual bool LXUSB_LEDRed( void ) =0;
   virtual bool LXUSB_LEDGreen( void ) =0;
   virtual bool LXUSB_SetAll( int frame1, int frame2, int shutter, int ccdamp, int led ) =0;
   virtual bool LXUSB_Frame1Status( int *status ) =0;
   virtual bool LXUSB_Frame2Status( int *status ) =0;
   virtual bool LXUSB_ShutterStatus( int *status ) =0;
   virtual bool LXUSB_CCDAmpStatus( int *status ) =0;
   virtual bool LXUSB_LEDStatus( int *status ) =0;
   virtual bool LXUSB_Status( int *frame1_status, int *frame2_status, int *shutter_status, int *ccdamp_status, int *led_status ) =0;



public:
   /// Trivial byte style interface

   /// @brief Returns true the device is OK and usable
   bool Open();

   /// @brief Returns if the device is OK and usable
   /// @return Returns true if the device is OK and opened
   bool BoardAvailable();

   /// @brief Closes the device
   void Close();

   /// it assumes the b0..b3 --> Frame1, Frame2, Shutter, Amp (LED handling is intrinsic)
   /// and bit = 1 is enabled i.e. the item is asserted (reading out, shutter, amp works)

   /// @brief The bits of the trivial interface
   enum ECVALS {
      ECV_None    =0,
      ECV_F1      =1, // Frame 1
      ECV_F2      =2, // Frame 2
      ECV_Shutter =4, // Shutter
      ECV_Amp     =8, // CCD Amplifier
      ECV_All     =15,
   };

   /// @brief Asserts the webcam control line when the bit is 1
   bool SetValue(unsigned short val);
   /// @brief Returns the control bits
   bool GetValue(unsigned short& val);

protected:
   LXUsb();

private:
   /// note: true = signal is enabled
   bool BoardValues(bool& f1, bool& f2, bool& shutter, bool& amp, bool& led, bool& ledRed);
   bool SetControls(bool f1, bool f2, bool shutter, bool amp);

   bool m_board_present;
};

#endif // _LXUSB_H_
