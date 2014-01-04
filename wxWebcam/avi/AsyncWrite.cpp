/**
 * @file           AsyncWrite.cpp
 *****************************************************************************
 * class AsyncWrite
 *
 *
 * Copyright (C) 2009 Martin Burri  (bm98@burri-web.org)
 *
 *<hr>
 *
 * @b Project      wxAstroCapture<br>
 *
 * @author         M. Burri
 * @date           09-Aug-2009
 *
 *****************************************************************************
 *<hr>
 * @b Updates
 * - dd-mmm-yyyy V. Name: Description
 *
 *****************************************************************************/

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


#include "AsyncWrite.h"

#include <assert.h>

#include <wx/wfstream.h>

// local shortcuts
typedef unsigned char  _byte;
typedef unsigned short _word;
typedef unsigned long  _dword;


////////////////////////////////////////////////////////////////////
// AsyncIOBlock
////////////////////////////////////////////////////////////////////

AsyncIOBlock::AsyncIOBlock(size_t memSize)
: m_memBlock(NULL)
, m_memSize(0)
, m_seekSet(false)
, m_seekPos(0)
{
	// note memsize can be 0 - may be only a seek command is issued then
	if (memSize>0) {
		m_memSize = memSize;
		m_memBlock = static_cast<void*>(new _byte[m_memSize]);
		if (!m_memBlock) {
			m_memSize = 0; // no memory
		}
	}
}

AsyncIOBlock::~AsyncIOBlock()
{
	if (m_memBlock) delete [] static_cast<_byte*>(m_memBlock);
}

void AsyncIOBlock::SeekPos(size_t seekPosition)
{
	m_seekPos = seekPosition;
	m_seekSet = true; // have to seek write
}

void* AsyncIOBlock::MemPtr()
{
	return m_memBlock;
}

size_t AsyncIOBlock::MemSize()
{
	return m_memSize;
}

bool   AsyncIOBlock::Seeking()
{
	return m_seekSet;
}

size_t AsyncIOBlock::SeekOffset()
{
	return m_seekPos;
}


////////////////////////////////////////////////////////////////////
// AsyncWriteThread
////////////////////////////////////////////////////////////////////


class AsyncWriteThread :  public wxThread
{
public:
   AsyncWriteThread(AsyncWrite* caller, const wxString& fname);
   virtual ~AsyncWriteThread();

	//! thread interface
	ExitCode Entry();


private:
	AsyncWriteThread(); // we don't expose this one

   AsyncWrite* m_pCaller;
	int     	   m_exitCode;
	wxString    m_fileName;

};


// invalid creator
AsyncWriteThread::AsyncWriteThread()
: wxThread(wxTHREAD_JOINABLE)
, m_pCaller(NULL)
, m_exitCode(5)
{
}


AsyncWriteThread::AsyncWriteThread(AsyncWrite* caller, const wxString& fname)
: wxThread(wxTHREAD_JOINABLE)
, m_pCaller(caller)
, m_exitCode(0)
, m_fileName(fname)
{
}

AsyncWriteThread::~AsyncWriteThread()
{
}

//! Task execution comes here
wxThread::ExitCode AsyncWriteThread::Entry()
{
const long WAIT_INTERVALL = 250;  		// milliseconds
const long FILE_BUFFER_SIZE = 1024000;	// Bytes

   // Sanity check
	if ( !m_pCaller ) {
		m_exitCode = 99;  // no caller
		return &m_exitCode; // immediately - this is messy - we cannot even reach our creator
	}

	if ( m_fileName.IsEmpty() ) {
		m_pCaller->m_closed  = true;
		m_exitCode = 2;  // no filename
		return &m_exitCode;
	}

	// local vars while the thread runs
	wxFileOutputStream*        fileOutStream=NULL;
	wxBufferedOutputStream*    bufOutStream=NULL;

    fileOutStream = new wxFileOutputStream(m_fileName);
    if ( ! fileOutStream->IsOk()) {
		m_pCaller->m_closed  = true;
		m_exitCode = 4;  // error opening file
		return &m_exitCode;
    }
    bufOutStream = new wxBufferedOutputStream(*fileOutStream);
    bufOutStream->GetOutputStreamBuffer()->SetBufferIO(FILE_BUFFER_SIZE); // set buffer B


   // Thread actions
	AsyncIOBlock*	thisBlock = NULL;

	// INIT
	m_exitCode = 0;         // task result
	bool done = false;      // task status
	m_pCaller->m_writing = false;
	m_pCaller->m_closed  = false;

   /* ----------------------------------------------------------- */
   while(!done)
   {
      // wxThread obligation
      if ( this->TestDestroy() ) {
         // must terminate prematurely
         m_pCaller->m_aborted = true;
      }
      if (m_pCaller->m_aborted) {
         // task was aborted by some
         m_exitCode = 1;
         done = true;  // thread will die now
      }
      else {
         // process queue
		thisBlock = m_pCaller->GetNextBlock();
		if (thisBlock) {
			if ( thisBlock->Seeking() ) {
				// seek first then write
				bufOutStream->SeekO(thisBlock->SeekOffset(), wxFromStart);
			}
			void*  buf = thisBlock->MemPtr();
			size_t length = thisBlock->MemSize();
			if (buf && length) bufOutStream->Write(buf, length);

			delete thisBlock; thisBlock = NULL; // drop block completely

			m_pCaller->m_writing = true; // continue processing immediately
		}
		else {
			m_pCaller->m_writing = false; // wait a while before next queue visit
			if (m_pCaller->m_close) {
				m_exitCode = 0; // regular exit
				done = true;  // thread will die now
			}
		}

        if (!m_pCaller->m_writing) AsyncWrite::Sleep_ms(WAIT_INTERVALL);  // now wait a while if not writing

      }//not aborted
   }// thread loop

    bufOutStream->Sync();
    delete bufOutStream; bufOutStream=NULL;
    fileOutStream->Close();
    delete fileOutStream; fileOutStream=NULL;

	m_pCaller->m_closed  = true; // finally

	return &m_exitCode;  // thread completed successfully
}



//////////////////////////////////////////////////////////////////////
/// AsyncWrite
//////////////////////////////////////////////////////////////////////

AsyncWrite::AsyncWrite()
: m_writeThread(NULL)
, m_lastWriteSize(0)
, m_currentOffset(0)
, m_currentLastPos(0)
, m_close(true)
, m_writing(false)
, m_closed(false)
, m_exitCode(0)
, m_aborted(true)
, m_abortWriting(true)
, m_writeErrorDetected(false)
{
}

AsyncWrite::AsyncWrite(const wxString& fname)
: m_writeThread(NULL)
, m_lastWriteSize(0)
, m_currentOffset(0)
, m_currentLastPos(0)
, m_close(false)
, m_writing(false)
, m_closed(false)
, m_exitCode(0)
, m_aborted(false)
, m_abortWriting(false)
, m_writeErrorDetected(false)
{
	// immediately create the thread and run it
	m_writeThread = new AsyncWriteThread(this, fname);
	m_writeThread->Create();
	m_writeThread->Run();   // let's run it
}

AsyncWrite::~AsyncWrite()
{
	m_aborted = true;
	m_writeThread->Wait(); // will respond if finished
	delete m_writeThread;  m_writeThread = NULL;

	// empty the current queue (if there is anything left...)
	AsyncIOBlock* ioBlock;
	while ( !m_queue.empty() ) {
		ioBlock = m_queue.front();
		m_queue.pop_front();
		if (ioBlock) delete ioBlock; ioBlock=NULL;
	}
}


//////////////////////////////////////////////////////////////////////
// for the thread only
//////////////////////////////////////////////////////////////////////

AsyncIOBlock* AsyncWrite::GetNextBlock()
{
AsyncIOBlock* ioBlock = NULL;

	// before using the list we must acquire the mutex
	wxMutexLocker lock(m_mutexQueue);

   if ( ! m_queue.empty()) {
      ioBlock = m_queue.front();
      m_queue.pop_front();
   }

	m_mutexQueue.Unlock();
	return ioBlock;
}
//////////////////////////////////////////////////////////////////////
// for the owner
//////////////////////////////////////////////////////////////////////

//!@brief Queues an IO block for writing to the file
size_t AsyncWrite::Write(const void* buffer, size_t size)
{
size_t retVal = 0;

	if (size>0) {
		AsyncIOBlock* ioBlock = new AsyncIOBlock(size);
		if (ioBlock && (ioBlock->MemSize()==size) ) {
			::memcpy(ioBlock->MemPtr(), buffer, size);
			retVal = Write(ioBlock);
		}
	}
	return retVal;
}

//!@brief defines a new seek position
size_t AsyncWrite::SeekO(size_t pos)
{
size_t retVal = 0;

   AsyncIOBlock* ioBlock = new AsyncIOBlock(0);

   if (ioBlock) {
      ioBlock->SeekPos(pos);
      retVal = Write(ioBlock);
   }
	return m_currentOffset;
}



//!@brief Queues an IO block for writing to the file
size_t AsyncWrite::Write(AsyncIOBlock* ioBlock)
{
	// before using the list we must acquire the mutex
	wxMutexLocker lock(m_mutexQueue);

	if (ioBlock) {
		// we have:
	//		m_currentOffset  - the actual file pos of the last write (0=no Write ..)
	//		m_currentLastPos - the farthest file pos of any write (0=no Write ..)

		m_lastWriteSize = ioBlock->MemSize();
		if ( m_lastWriteSize>0 ) {
			m_lastWriteSizeStatus = m_lastWriteSize; // only if something was written
		}

		if ( ioBlock->Seeking() ) {
			// recalculate things based on seeking
			// first seek, then write
			m_currentOffset = ioBlock->SeekOffset(); // new seekpos
		}
		wxFileOffset n = m_currentOffset + m_lastWriteSize; // seek + write bytes
		m_currentLastPos = (n>m_currentLastPos) ? n : m_currentLastPos; // after write
		// last update the currentOffset to show things after writing
		m_currentOffset += m_lastWriteSize;
	}

	// just add it to the list
	m_queue.push_back(ioBlock);

	// lock is managed by wxMutexLocker
	return m_lastWriteSizeStatus;
}

size_t AsyncWrite::LastWrite()
{
	return m_lastWriteSizeStatus;
}

//!@brief returns the current write position (independent of actual written bytes)
size_t AsyncWrite::TellO()
{
	return m_currentOffset;
}


//!@brief Closes the file - returns when all pending was written
void AsyncWrite::Close()
{
	m_close = true;  // command to close - no mutex needed here...
	while (!m_closed) {
		AsyncWrite::Sleep_ms(500);
	}
}

//!@brief Sync file write - returns when all pending was written
void AsyncWrite::Sync()
{
	// wait while the thread writes things
	while (m_writing) {
		AsyncWrite::Sleep_ms(500);
	}
}


//! Prematurely abort writing and close the file
void AsyncWrite::AbortClose()
{
AsyncIOBlock* ioBlock = NULL;

	m_aborted=true; // command to abort

	// before using the list we must acquire the mutex
	wxMutexLocker lock(m_mutexQueue);

	// empty the current queue (if there is anything left...)
	while ( !m_queue.empty() ) {
		ioBlock = m_queue.front();
		m_queue.pop_front();
		if (ioBlock) delete ioBlock; ioBlock=NULL;
	}

	m_mutexQueue.Unlock();

	while (!m_closed) {
		AsyncWrite::Sleep_ms(500);
	}

}


#ifdef _WXMSW_
///////////////////////////////////////////////////////////////////////////////
/// @brief Sleep for an amount of milliseconds
void AsyncWrite::Sleep_ms(unsigned short  mseconds )
{
   // Windows Sleep uses miliseconds
   // linux usleep uses microsecond
   // the argument is   coming in millisecond.
	::Sleep(mseconds);
}
#else
///////////////////////////////////////////////////////////////////////////////
/// @brief portable sleep command; take milliseconds as argument
//static
void AsyncWrite::Sleep_ms(unsigned short  mseconds )
{
   // Windows Sleep uses miliseconds
   // linux usleep uses microsecond
   // the argument is   coming in millisecond.
   long useconds = long(mseconds) * 1000;
   // usleep can only handle <1000000 usec
   if (useconds>=1000000) {
      long seconds = useconds / 1000000;
      sleep(seconds);
      useconds %=1000000;
   }
   if (useconds>20) usleep(useconds);
}
#endif //_WXMWS_
