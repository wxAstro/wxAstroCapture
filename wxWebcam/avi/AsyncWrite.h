
/**
 * @file           AsyncWrite.h
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


#ifndef _ASYNCWRITE_H_
#define _ASYNCWRITE_H_


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <deque>

class AsyncIOBlock
{
public:
	// allocate an IO block with a memory size (can be 0 - then no Mem is allocated)
	// no mem can be used to seek only
	AsyncIOBlock(size_t memSize);
	virtual ~AsyncIOBlock();

	// defines a seek position that is used BEFORE writing takes place
	void SeekPos(size_t seekPosition);

	// returns a writeable mem ptr (take care not to trash mem - no check is done!!)
	void*  MemPtr();
	// returns the allocated memory space
	size_t MemSize();

	// returns true if a seek is requested
	bool   Seeking();
	// returns a seekoffset (0-based) - valid only if Seeking() returned true
	size_t SeekOffset();

private:
	void*  m_memBlock;
	size_t m_memSize;

	bool   m_seekSet;
	size_t m_seekPos;
};


class AsyncWriteThread;
class AsyncWrite
{
friend class AsyncWriteThread;

public:
   AsyncWrite(const wxString& fname);
   virtual ~AsyncWrite();

	//! external control interface

	//!@brief Queues an IO block for writing to the file
	size_t Write(AsyncIOBlock* ioBlock);
	//!@brief Queues an IO block for writing to the file
	size_t Write(const void* buffer, size_t size);
	//!@brief Returns the last Write mem size (compatibility only)
	size_t LastWrite();
	//!@brief returns the current write position (independent of actual written bytes)
	size_t TellO();
	//!@brief defines a new seek position
	size_t SeekO(size_t pos);

	//!@brief Closes the file - returns when all pending was written
	void Close();

	//!@brief Sync file write - returns when all pending was written
	void Sync();


	//! Prematurely abort writing and close the file
	void AbortClose();

	//! external status interface
	// - not mutexed as we assume this is an atomar function
	bool IsAborted() {return m_aborted;};

private:
	AsyncWrite();
	AsyncWriteThread* m_writeThread;

	AsyncIOBlock* GetNextBlock();

	wxString		m_fName;	// save the filename
	wxMutex         m_mutexQueue;

	size_t			m_lastWriteSizeStatus;
	size_t			m_lastWriteSize;
	wxFileOffset    m_currentOffset;
	wxFileOffset	m_currentLastPos; // 0-based - 1byte written -> LastPos=0

	// some status vars
	bool 			m_close;  // true when a close is requested
	bool			m_writing;  // true while the thread is writing
	bool			m_closed;   // true when the tread has finished
	int     		m_exitCode;
	bool   			m_aborted;

	bool      		m_abortWriting;     // true if we have to kill the take
	bool     		m_writeErrorDetected;

	typedef std::deque<AsyncIOBlock*>  WRITEQUEUE;
	WRITEQUEUE	m_queue;

private:
	static void Sleep_ms(unsigned short  mseconds );

};


#endif  // _ASYNCWRITE_H_
