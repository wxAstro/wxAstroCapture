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
 * @file           ArtTimeHelper.cpp
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

#include "ArtTimeHelper.h"

#include <time.h>
#include <assert.h>

// local shortcuts
typedef unsigned char      _byte;
typedef unsigned short     _word;
typedef unsigned long      _dword;
typedef unsigned long long _qword;
typedef long long          _int64;

ArtTimeHelper::ArtTimeHelper()
{
   ::gettimeofday(&m_initTime, NULL);

   m_startMs=0;
   m_endMs=0;
}

ArtTimeHelper::~ArtTimeHelper()
{
}

void ArtTimeHelper::NowMs(long long &now)
{
   timeval _now; ::gettimeofday(&_now, NULL);
   // must take care of data type (tv_sec is large already)
   now=(long long)(_now.tv_sec) * (long long)(1000);
   now+=_now.tv_usec/1000;
}

// set starttime as now
void ArtTimeHelper::StartNow()
{
   NowMs(m_startMs);
   m_endMs = 0; // disable timing when newly started
}

// provide an endtime in n milliseconds
void ArtTimeHelper::EndInMSec(unsigned long msToGo)
{
   m_endMs = m_startMs + msToGo;
}

// provide an endtime in f seconds
void ArtTimeHelper::EndInSec(double sToGo)
{
   _dword ms = _dword(sToGo * 1000.0);
   EndInMSec(ms); // just use the ms routine
}

// returns true if proposed endtime is reached
bool ArtTimeHelper::HasEnded()
{
   // check if we measure time (m_endTime>0)
   if (m_endMs>0) {
      _int64 now; NowMs(now);
      if (now>=m_endMs) {
         // end has passed already
         return true;
      }
   }
   return false; // no timing or not reached yet
}

// assumes now as endtime for Elapsed returns
void ArtTimeHelper::EndNow()
{
  NowMs(m_endMs); // catch current
}

// return the elapsed milliseconds
unsigned long ArtTimeHelper::ElapsedMSec()
{
   _int64 now; NowMs(now);
   if (now>m_startMs) {
      _int64 diff = now - m_startMs;
      return _dword(diff);
   }
   return 0;
}

// return the elapsed seconds
double ArtTimeHelper::ElapsedSec()
{
   _dword elapsedMs = ElapsedMSec();
   return (double(elapsedMs) / 1000.0);
}

// return the remaining milliseconds (stable 0 after EndNow is called)
unsigned long ArtTimeHelper::RemainingMSec()
{
   _int64 now; NowMs(now);
   if (m_endMs>now) {
      // still timing
      _int64 diff = m_endMs - now;
      return _dword(diff);
   }
   // timer has ended
   return 0;
}

// return the remaining seconds (stable 0 after EndNow is called)
double ArtTimeHelper::RemainingSec()
{
   _dword remainingMs = RemainingMSec();
   return (double(remainingMs) / 1000.0);
}

