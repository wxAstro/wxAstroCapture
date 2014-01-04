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
 * @file           Atik16Thread.cpp
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
 *
 * From State           Action               To State
 *--------------------------------------------------------------
 * CAMERA_IDLE          idle loop 250ms      CAMERA_IDLE
 *                      TimedWait(msec)      CAMERA_EXPOSING
 *
 * CAMERA_EXPOSING      timer ends or StopExposure()
 * --> Callback - ProcessTimeElapsed()
 *
 *                      Aquire(what)         CAMERA_READING
 *
 * CAMERA_READING
 * --> Callback - ProcessTriggerRDCCD()
 * --> Callback - ProcessTriggerDownload(0%)
 *                      setup download       CAMERA_DOWNLOADING
 *
 * CAMERA_DOWNLOADING
 * --> Callback - ProcessTriggerDownload(dl%)
 *                 ..... multiple times
 *
 * --> Callback - ProcessImageRead(readErr, mSec, bps)
 *                      read ok              CAMERA_IDLE
 * or
 * --> Callback - ProcessImageRead(readErr, mSec, -1)
 *                      read error           CAMERA_IDLE
 *
 *
 * CAMERA_EXPOSING      AbortExposure()      CAMERA_FLUSHING
 * CAMERA_READING       AbortExposure()      CAMERA_FLUSHING
 * CAMERA_DOWNLOADING   AbortExposure()      CAMERA_FLUSHING
 *
 * CAMERA_FLUSHING
 * --> Callback - ProcessImageRead(readErr, mSec, -1)
 *                      read ok              CAMERA_IDLE
 *                      read error           CAMERA_IDLE
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

#include "Atik16Thread.h"

/// There is a derived version used for the INDI framework
///  which has some files in another place than wxAstroCapture
#ifdef INDI
   #include "GenericDebHelp.h"
#else
   #include "../GenericDebHelp.h"
#endif

#include "Atik16Drv.h"
#include "Ftd245IO.h"

#include <assert.h>

// default timeouts for the FTDI boards
#define CAM_RX_TIMEOUT   1500
#define CAM_TX_TIMEOUT   5000

// local shortcuts
typedef unsigned char  _byte;
typedef unsigned short _word;
typedef unsigned long  _dword;

// invalid creator
Atik16Thread::Atik16Thread()
: m_pCaller(NULL)
, m_exitCode(0)
, m_aborted(true)
, m_aquiringLength(0)
, m_waitMsRemaining(0)
, m_abortExposure(true)
, m_readErrorDetected(false)
, m_buffer(NULL)
{
}


Atik16Thread::Atik16Thread(Atik16Drv* caller, const Atik16Cmd::AtikDevDescRec& dev)
: m_pCaller(caller)
, m_exitCode(0)
, m_aborted(false)
, m_aquiringLength(0)
, m_waitMsRemaining(0)
, m_abortExposure(false)
, m_readErrorDetected(false)
, m_buffer(NULL)
{
   m_dev = dev;
//   m_lastTickTime = ::wxGetLocalTimeMillis();
}

Atik16Thread::~Atik16Thread()
{
}

//! Task execution comes here
// NOTE: for interlaced cams
//       for exposures < 2460 ms the cam will aquire an exposure for each frame
//       i.e. 2.4 sec expo for F1 then send one frame then 2.4 sec for F2 then send the frame
//       i.e. the receiver thread must have enough time to capture the second one too
void Atik16Thread::Entry()
{
   // Sanity check
   if ( !m_pCaller ) {
      return; // immediately - this is messy - we cannot even reach our creator
   }

   // Thread actions
   const long WAIT_INTERVALL = 250;  // milliseconds

   // INIT
	m_exitCode = 0;         // task result
	bool done = false;      // task status
	bool readData = false;  // read status
	bool linger = false;    // something left to trash

   long toRead = 0;        // what do we have to read
   long nRead = 0;         // how much is left
	long readSize = 1;      // best read chuncksize
	long readCount= 0;      // counts the already read bytes

	const long InitNullReads = 20; // allow enough null reads (-> 20 sec)
	long nullReads = 0;     // read=0 counter to allow cam lag times

   unsigned char* pBuffer = NULL; // will hold the destination ptr

   ArtTimeHelper bpsTimer; // in read timer
   long Bps;               // Bytes per second value (DL rate)

   // Sanity check
   if ( m_dev.DevNULL() ) {
      m_exitCode = 2;
      done = true;
   }

   // Init FTD
   m_dev.ftd->SetDefaultBuffer();
   m_dev.ftd->SetTimeouts(CAM_RX_TIMEOUT, CAM_TX_TIMEOUT);
   m_pCaller->ProcessThreadStart(); // and down to the caller

   /* ----------------------------------------------------------- */
   while(!done)
   {
      if (m_aborted) {
         // task was aborted by some
         m_exitCode = 1;
         done = true;  // thread will die now
      }
      else {
         // process cam states
         switch (m_pCaller->m_camState) {
            case CAMERA_IDLE: {
               ;
            }// IDLE task
            break; // nothing to do ..

            case CAMERA_EXPOSING: {
               // the cam is now waiting for the exposure to end - next is either READING or FLUSHING
               m_waitMsRemaining = m_timeHelper.RemainingMSec(); //BM:20090410 - bugfix not showing EXP time
               if (m_abortExposure) {
                  // waiting ended with abort
                  OnWaitEnd(m_waitMsRemaining); // call with what's left
               }
               else if ( m_timeHelper.HasEnded() ) {
                  // waiting ended
                  OnWaitEnd(0);           // process below with 0 left
               }// expo done
               else {
                  ; // just wait
               }
            }// EXPO task
            break;

            case CAMERA_READING: {
               // the cam is reading the CCD - next is DOWNLOADING
               // not yet in reading state - now reading starts
               pBuffer = (unsigned char*)m_buffer; // where to write ??
               // note: this either reads the camera
               //       or does short exposures i.e. up to ~2.5 secs inplace
               m_elapsedMs = m_timeHelper.ElapsedMSec(); // real end duration just before CCD is read
               m_pCaller->ProcessTriggerRDCCD();         // issue the read command down to the camera

               // reading is done - we prepare DL
               toRead = m_aquiringLength; // get aquire size
               nRead  = toRead;           // set remaining amout
               linger = false;

               nullReads = InitNullReads;   // number of errors with read == 0 allowed
               readCount = 0;       // read counter (to calc DL percents)
               readSize = m_dev.ftd->GetAskSize();  // optimal ask size according to manual
               readData = true;     //reading now
               m_readErrorDetected = false;

               m_dev.ftd->SetTimeouts(1000, CAM_TX_TIMEOUT); // timeout - 20 retries in the thread
               m_pCaller->ProcessTriggerDownload(0);  // init DL with 0% done and update State
               bpsTimer.StartNow();                   // get the time to calc Bps later
               // triggers download processing
               if (m_abortExposure) {
                  m_pCaller->SetCameraState(CAMERA_FLUSHING); // immediate flush
               }
               else {
                  m_pCaller->SetCameraState(CAMERA_DOWNLOADING);
               }
            }// READING task
            break;

            case CAMERA_FLUSHING: {
               // just readout the cam if needed - next is IDLE again
               if (readData) {
                  // read remaining bytes from camera
                  // we cannot cancel data collection, once started
                  // so we have to read what's left
                  nullReads = InitNullReads;      // number of errors with read == 0 allowed
                  linger = (nRead>0) ? true : false; // something left
                  while (linger && (nullReads>0))
                  {
                     // do not longer gather data but read the buffers from USB if needed
                     // just read the rest (hogging)
                     size_t read;
                     if ( m_dev.ftd->ReadBYTEsNIL(read) ) {
                        nRead -= read;
                     }
                     else {
                        ::fprintf(stderr, "Atik16Thread - Error reading from camera while flushing !\n");
                        nullReads = 0; // force error exit
                        m_readErrorDetected = true;
                     }

                     if (read<=0) {
                        nullReads--; // make sure to get to an end once
                     }
                     else {
                        nullReads = InitNullReads; // reset nullreads after getting some
                     }
                     linger = (nRead>0) ? true : false; // something left
                  }
                  nRead = 0; // left to read is forced to 0 now
                  linger = false; // not longer
                  readData = false; // not longer
               }// was reading

               OnReadEnd(-1);
            }// FLUSHING task
            break;

            case CAMERA_DOWNLOADING: {
               // the cam is now downloading the image data - next is IDLE
               if (nRead>0) {
                  // still something to read
                  size_t read = 0;
                  // get data in chunks that should be optimal
                  if ( m_dev.ftd->ReadBYTEs(pBuffer, (nRead>=readSize)? readSize : nRead, read) ) {
                     // we have read something
                     if (read>0) {
                        // Regular Read: got something
                        readCount += read; nRead -= read;
                        pBuffer+=read; // advance dest ptr
                        if (toRead>0) m_pCaller->ProcessTriggerDownload((readCount *100) / toRead); // update DL status
                        nullReads = InitNullReads;  // reset Read Error counter once we got something
                        // continue with next round
                     }
                     else {
                        // Zero Read: read=0 - try more read
                        // again - here we may have to wait for the second 2.46 sec exposure - so be patient...
                        nullReads--;
                        _V_ ::fprintf(stdout, "Atik16Thread - Read 0 bytes: %ld - got %ld waitingfor %ld \n", nullReads, readCount, nRead);
                        if (nullReads<=0) {
                           // too many 0-reads, bail out - cam seems not longer responding (or we expected too much...)
                           ::fprintf(stderr, "Atik16Thread - too many 0 byte reads from camera!\n");
#ifdef _DEBUG
                           ::fprintf(stderr, "Atik16Thread - DEBUG continues!\n");
                           readData = false; // not longer
                           m_readErrorDetected = false; // we have a read error condition now
                           nRead=0;
                           OnReadEnd(0); // call my creator with success for debug
#else
                           readData = false; // not longer
                           m_readErrorDetected = true; // we have a read error condition now
                           nRead=0;
                           OnReadEnd(-1); // call my creator with failure
#endif
                           // this will terminate DL
                        }
                     }
                  }// read OK
                  else {
                     // IO Read Error: abort
                     ::fprintf(stderr, "Atik16Thread - Error reading from camera during download!\n");
                     readData = false; // not longer
                     m_readErrorDetected = true; // we have a read error condition now
                     nRead=0;
                     OnReadEnd(-1); // call my creator with failure
                     // this will terminate DL
                  }// read error
               }//if something to read

               else {
                  // regular read end
                  readData = false; // not longer
                  long dlTime =  long(bpsTimer.ElapsedMSec());
                  if (dlTime!=0) Bps = (toRead*1000) / dlTime; //bytes / sec
                  else           Bps = 1;
                  m_readErrorDetected = false;
                  OnReadEnd(Bps); // call my creator to indicate read end
                  // this will terminate DL
               }// DL done
            }// DOWNLOADING task
            break;

            default: {
            // other state such as CAMERA_ERROR
            // just waiting
            ;
            }//default
         }//switch

         if (!readData) m_dev.ftd->Sleep_ms(WAIT_INTERVALL);  // now wait a while if not reading

      }//not aborted
   }// thread loop

   OnThreadExit();      // call my creator to expect to shutdown things
}


//////////////////////////////////////////////////////////////////////
// for the thread only
//////////////////////////////////////////////////////////////////////

// Indicates the timer has expired
// endCode = 0 is regular timer end
// endCode <>0 is timer has been stopped before end
void Atik16Thread::OnWaitEnd(int endCode)
{
// REMARK: beware of the context ... this is a callback of the HighPrio Thread

	// reset states
   m_waitMsRemaining = 0; // time remaining

   if (m_abortExposure) {
	   // exposure aborted
      _V_ ::fprintf(stdout, "Atik16Thread::OnWaitEnd() - aborted ms left: %d\n", (int)endCode);
      m_pCaller->SetCameraState(CAMERA_FLUSHING); // send data to NIL and report if finished
   }
	else {
      assert(endCode==0);    // sw bug

	   // proper end exposure time - caller will trigger readout now
      _V_ ::fprintf(stdout, "Atik16Thread::OnWaitEnd() - end of exposure\n");
		m_pCaller->ProcessTimeElapsed(); // and down to the caller
	}
}

// Indicate reading has ended
void Atik16Thread::OnReadEnd(long dlBps)
{
   _V_ ::fprintf(stdout, "Atik16Thread::OnReadEnd()\n");

// REMARK: beware of the context ... this is a callback of the HighPrio Thread
	// reset states
	m_aquiringLength = 0;

   // reset FT interface
   m_dev.ftd->SetDefaultBuffer();
   m_dev.ftd->SetTimeouts(CAM_RX_TIMEOUT, CAM_TX_TIMEOUT);

   // we have to call this one in any case to indicate end of a cycle
   m_pCaller->ProcessImageRead(m_readErrorDetected, m_elapsedMs, dlBps); // and down to the caller
   if ( m_readErrorDetected ) {
      _V_ ::fprintf(stdout, "Atik16Thread::OnReadEnd() - read error detected - cam state = IDLE\n");
   }
   else {
      _V_ ::fprintf(stdout, "Atik16Thread::OnReadEnd() - regular end of take - cam state = IDLE\n");
   }
   m_pCaller->SetCameraState(CAMERA_IDLE);

   m_abortExposure = false;  // reset
}

// indicates the thread will exit after returning
void Atik16Thread::OnThreadExit()
{
// REMARK: beware of the context ... this is a callback of the HighPrio Thread
	// reset states
	m_waitMsRemaining = 0;
	m_aquiringLength = 0;

	if (m_exitCode != 0) // not a regular end
		m_aborted = true;

	m_pCaller->ProcessThreadEnd(m_exitCode); // and down to the caller

   // after returning from here the thread proc ends !!!
}


//////////////////////////////////////////////////////////////////////
// for the owner
//////////////////////////////////////////////////////////////////////

// must now wait a given time
// it will report the end of wait through the callback ProcessTimeElapsed()
void Atik16Thread::TimedWait(long durationMs) // milliseconds
{
   _V_ ::fprintf(stdout, "Atik16Thread::TimedWait()\n");

	if (!m_aborted) {
      assert(m_pCaller->CameraState()==CAMERA_IDLE);

		m_abortExposure = false;  // reset
	   m_timeHelper.StartNow();
	   m_timeHelper.EndInMSec(durationMs);
		m_waitMsRemaining = m_timeHelper.RemainingMSec();
      m_elapsedMs = 0;  // set back to zero
		// camstate is maintained in the owner obj to have it also
		//  when the task is not yet existing
		m_pCaller->SetCameraState(CAMERA_EXPOSING);
	}
	else {
      ::fprintf(stderr, "Atik16Thread::TimedWait() - thread is aborted, cannot setup new take\n");
	   assert(false);;
	}
}

// must now aquire data
// it will report the end of read through the callback
void Atik16Thread::Aquire(ArtSampleType& where)
{
   _V_ ::fprintf(stdout, "Atik16Thread::Aquire()\n");

	if (!m_aborted && !m_abortExposure) {
      assert(m_pCaller->CameraState()==CAMERA_EXPOSING);

      // setup from parameter
		m_buffer = (unsigned char*)where.imageMem->ByteMemPtrRef();
		m_aquiringLength = where.dataLength; // >0 = how many BYTES

      m_pCaller->SetCameraState(CAMERA_READING);
	}
	else {
      ::fprintf(stderr, "Atik16Thread::TimedWait() - thread or exposure is aborted, cannot aquire now\n");
	   assert(false);;
	}
}

// Prematurely stop exposure
void Atik16Thread::StopExposure()
{
   _V_ ::fprintf(stdout, "Atik16Thread::StopExposure()\n");

   if (m_pCaller->CameraState()==CAMERA_EXPOSING) {
      m_timeHelper.EndNow(); // will end timer and immediately trigger DL
   }
}

// Prematurely abort exposure
void Atik16Thread::AbortExposure()
{
   _V_ ::fprintf(stdout, "Atik16Thread::AbortExposure()\n");

   if (m_pCaller->CameraState()==CAMERA_EXPOSING) {
      m_abortExposure=true;   // will end timer and proceed to FLUSHING
   }
   else if (m_pCaller->CameraState()==CAMERA_READING) {
      m_abortExposure=true;   // will proceed to FLUSHING
   }
   else if (m_pCaller->CameraState()==CAMERA_DOWNLOADING) {
      m_pCaller->SetCameraState(CAMERA_FLUSHING); // changes DL to FLUSHING
   }
}

// thread must abort itself
void Atik16Thread::AbortThread()
{
   _V_ ::fprintf(stdout, "Atik16Thread::AbortThread()\n");

	// will make the thread to terminate
   m_abortExposure=true;
	m_aborted = true; // thread will end now (in a while)
}


