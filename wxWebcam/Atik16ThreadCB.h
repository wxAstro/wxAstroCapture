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
 * @file           Atik16ThreadCB.h
 *****************************************************************************
 * abstract class Atik16ThreadCB
 *
 * Astronomy USB Appliance driver for Linux and Win32
 * The camera reader thread callback protorype
 *  for Artemis/ATIK cameras with USB 1.1 interface
 *  User of the Thread have to be derived from this class
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

#ifndef _ATIK16THREADCB_H_
#define _ATIK16THREADCB_H_

#include "ArtTypes.h" // enums and structs of the Atik environment

class Atik16ThreadCB
{

public:
    Atik16ThreadCB() {};
    virtual ~Atik16ThreadCB() {};

   //! thread interface
   virtual ArtSampleType* GetImageBuffer()  =0;
   //! Callback if timer expires
   virtual void ProcessTimeElapsed() =0;
   //! Callback if CCD Readout is needed (thread does not issue CCD commands)
   virtual void ProcessTriggerRDCCD() =0;
   //! Callback to indicate download percents
   virtual void ProcessTriggerDownload(long downloadPercent) =0;
   //! Callback to indicate the image was read and can be processed now
   virtual void ProcessImageRead(bool readError, long realExpotime, long downloadBps) =0;

   //! Callback when the worker is about to start
   virtual void ProcessThreadStart() =0;
   //! Callback when the worker is about to end
   virtual void ProcessThreadEnd(int exitCode) =0;

};


#endif  // _ATIK16THREADCB_H_



