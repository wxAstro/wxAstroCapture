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
 * @file           Atik16Thread.h
 *****************************************************************************
 * class Atik16Thread
 *
 * Astronomy USB Appliance driver for Linux and Win32
 * The camera reader thread for Artemis/ATIK cameras with USB 1.1 interface
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


#ifndef _ATIK16THREAD_H_
#define _ATIK16THREAD_H_

#include "ArtTypes.h"   // enums and structs of the Atik environment

#include "ArtTimeHelper.h"
#include "Atik16Cmd.h"  // Atik commands


class Atik16Drv;
class Atik16Thread: public ArtWorker
{
public:
   Atik16Thread(Atik16Drv* caller, const Atik16Cmd::AtikDevDescRec& dev);
   virtual ~Atik16Thread();

   //! thread interface
   void Entry();

   //! external control interface

   //! Start a timeout (exposure time)
   //! @param howLong  number of milliseconds to wait until calling back
   void TimedWait(long durationMs);

   //! Start image reading
   //! @param where    the location to copy the image into
   void Aquire(ArtSampleType& where);

   //! Prematurely stop exposure
   void StopExposure();

   //! Prematurely abort exposure
   void AbortExposure();

   //! Prematurely stop thread execution
   void AbortThread();

   //! external status interface
   // - not mutexed as we assume this is an atomar function
	bool IsAborted() {return m_aborted;};
   long WaitTimeRemaining() {return m_waitMsRemaining;};
   bool IsReadError() {return m_readErrorDetected;};

private:
   Atik16Thread(); // we don't expose this one

	void OnThreadExit();
	void OnReadEnd(long dlBps);
	void OnWaitEnd(int endCode);
	void OnTrigger();

	Atik16Drv*                  m_pCaller;  // caller ptr
   Atik16Cmd::AtikDevDescRec   m_dev;      // IO board ptr

   // some status vars
	int            m_exitCode;
	bool           m_aborted;
	long           m_aquiringLength; // >0 = how many BYTES
	long           m_waitMsRemaining;
   bool           m_abortExposure;     // true if we have to kill the take
   bool           m_readErrorDetected;

   ArtTimeHelper  m_timeHelper;
	long           m_elapsedMs;

	unsigned char* m_buffer;

};

#endif  // _ATIK16THREAD_H_
