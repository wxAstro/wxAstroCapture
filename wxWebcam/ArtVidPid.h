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
 * @file           AtikVidPid.h
 *****************************************************************************
 * defines only
 *
 * Astronomy USB Appliance driver - USB Vid and Pid defines
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

#ifndef _ATIKVIDPID_H_
#define _ATIKVIDPID_H_


// the FTDI Vendor ID
#define FTDI_VENDOR_VID           "0403"
#define FTDI_VENDOR_VID_x        0x0403

/*
* Definitions for Artemis astronomical USB based cameras
* Check it at http://www.artemisccd.co.uk/
*/
#define FTDI_ARTEMIS_PID          "DF28"  /* All Artemis Cameras */
#define FTDI_ARTEMIS_PID_x       0xDF28   /* All Artemis Cameras */
#define FTDI_MINIART_PID          "DF34"  /* Mini Artemis Cameras */
#define FTDI_MINIART_PID_x       0xDF34   /* Mini Artemis Cameras */
/*
* Definitions for ATIK Instruments astronomical USB based cameras
* Check it at http://www.atik-instruments.com/
*/
#define FTDI_ATIK_ATK16_PID       "DF30"  /* ATIK ATK-16 Grayscale Camera */
#define FTDI_ATIK_ATK16_PID_x    0xDF30  /* ATIK ATK-16 Grayscale Camera */
#define FTDI_ATIK_ATK16HR_PID     "DF31"  /* ATIK ATK-16HR Grayscale Camera */
#define FTDI_ATIK_ATK16HR_PID_x  0xDF31  /* ATIK ATK-16HR Grayscale Camera */
#define FTDI_ATIK_ATK16C_PID      "DF32"  /* ATIK ATK-16C Colour Camera */
#define FTDI_ATIK_ATK16C_PID_x   0xDF32  /* ATIK ATK-16C Colour Camera */
#define FTDI_ATIK_ATK16HRC_PID    "DF33"  /* ATIK ATK-16HRC Colour Camera */
#define FTDI_ATIK_ATK16HRC_PID_x 0xDF33  /* ATIK ATK-16HRC Colour Camera */

/*
* More definitions for ATIK Instruments astronomical USB based cameras
* newer line of ATIK IC16 cameras
* Check it at http://www.atik-instruments.com/
*/
#define FTDI_ATIK_ATK16IC_PID      "DF35"  /* ATIK ATK-16IC Grayscale Camera */
#define FTDI_ATIK_ATK16IC_PID_x   0xDF35  /* ATIK ATK-16IC Grayscale Camera */
#define FTDI_ATIK_ATK16ICC_PID     "DF36"  /* ATIK ATK-16IC-C Colour Camera */
#define FTDI_ATIK_ATK16ICC_PID_x  0xDF36  /* ATIK ATK-16IC-C Colour Camera */
#define FTDI_ATIK_ATK16ICS_PID     "DF2C"  /* ATIK ATK-16IC-S Grayscale Camera */
#define FTDI_ATIK_ATK16ICS_PID_x  0xDF2C  /* ATIK ATK-16IC-S Grayscale Camera */
#define FTDI_ATIK_ATK16ICSC_PID    "DF2D"  /* ATIK ATK-16IC-S-C Colour Camera */
#define FTDI_ATIK_ATK16ICSC_PID_x 0xDF2D  /* ATIK ATK-16IC-S-C Colour Camera */


#endif // _ATIKVIDPID_H_

