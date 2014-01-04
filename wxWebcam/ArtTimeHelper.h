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
 * @file           ArtTimeHelper.h
 *****************************************************************************
 * class ArtTimeHelper
 *
 * Astronomy USB Appliance driver for Linux and Win32
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


#ifndef _ARTTIMEHELPER_H_
#define _ARTTIMEHELPER_H_

#include <sys/time.h>

//! @brief  Implements a timer for camera exposure handling
class ArtTimeHelper
{
public:
   ArtTimeHelper();
   virtual ~ArtTimeHelper();

   // set starttime as now
   void StartNow();

   // provide an endtime in n milliseconds
   void EndInMSec(unsigned long msToGo);

   // provide an endtime in f seconds
   void EndInSec(double sToGo);

   // returns true if proposed endtime is reached
   bool HasEnded();

   // assumes now as endtime for Elapsed returns
   void EndNow();

   // return the elapsed milliseconds
   unsigned long ElapsedMSec();
   // return the elapsed seconds
   double ElapsedSec();

   // return the remaining milliseconds (stable 0 after EndNow is called)
   unsigned long RemainingMSec();
   // return the remaining seconds (stable 0 after EndNow is called)
   double RemainingSec();

private:
   void NowMs(long long &now);

   long long m_startMs;
   long long m_endMs;
   timeval   m_initTime;
};

#endif //  _ARTTIMEHELPER_H_
