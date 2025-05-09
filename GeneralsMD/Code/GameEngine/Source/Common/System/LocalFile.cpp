/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright(C) 2001 - All Rights Reserved                  
//                                                                          
//----------------------------------------------------------------------------
//
// Project:   WSYS Library
//
// Module:    IO_
//
// File name: IO_LocalFile.cpp
//
// Created:   4/23/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//         Includes                                                      
//----------------------------------------------------------------------------

#include "PreRTS.h"

#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <ctype.h>

#include "Common/LocalFile.h"
#include "Common/RAMFile.h"
#include "Lib/BaseType.h"
#include "Common/PerfTimer.h"
			

#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

//----------------------------------------------------------------------------
//         Externals                                                     
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Defines                                                         
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Types                                                     
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Data                                                     
//----------------------------------------------------------------------------

static Int s_totalOpen = 0;

//----------------------------------------------------------------------------
//         Public Data                                                      
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Prototypes                                               
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Functions                                               
//----------------------------------------------------------------------------

//=================================================================
// LocalFile::LocalFile
//=================================================================

LocalFile::LocalFile()
#ifdef USE_BUFFERED_IO
	: m_file(NULL)
#else
	: m_handle(-1)
#endif
{
}


//----------------------------------------------------------------------------
//         Public Functions                                                
//----------------------------------------------------------------------------


//=================================================================
// LocalFile::~LocalFile	
//=================================================================

LocalFile::~LocalFile()
{
#ifdef USE_BUFFERED_IO
	if (m_file)
	{
		fclose(m_file);
		m_file = NULL;
		--s_totalOpen;
	}
#else
	if( m_handle != -1 )
	{
		_close( m_handle );
		m_handle = -1;
		--s_totalOpen;
	}
#endif

	File::close();

}

//=================================================================
// LocalFile::open	
//=================================================================
/**
  *	This function opens a file using the standard C open() call. Access flags
	* are mapped to the appropriate open flags. Returns true if file was opened
	* successfully.
	*/
//=================================================================

//DECLARE_PERF_TIMER(LocalFile)
Bool LocalFile::open( const Char *filename, Int access )
{
	//USE_PERF_TIMER(LocalFile)
	if( !File::open( filename, access) )
	{
		return FALSE;
	}

	/* here we translate WSYS file access to the std C equivalent */
#ifdef USE_BUFFERED_IO
	char mode[32];
	char* m = mode;

	if (m_access & APPEND)
	{
		DEBUG_CRASH(("not yet supported by buffered mode"));
	}

	if (m_access & TRUNCATE)
	{
		DEBUG_CRASH(("not yet supported by buffered mode"));
	}

	if((m_access & READWRITE ) == READWRITE )
	{
		if (m_access & CREATE)
		{
			*m++ = 'w';
			*m++ = '+';
		}
		else
		{
			*m++ = 'r';
			*m++ = '+';
		}
	}
	else if(m_access & WRITE)
	{
		*m++ = 'w';
	}
	else
	{
		*m++ = 'r';
		DEBUG_ASSERTCRASH(!(m_access & TRUNCATE), ("cannot truncate with read-only"));
	}

	if (m_access & TEXT)
	{
		*m++ = 't';
	}
	if (m_access & BINARY)
	{
		*m++ = 'b';
	}

	*m++ = 0;

	m_file = fopen(filename, mode);
	if (m_file == NULL)
	{
		goto error;
	}
	
	//setvbuf(m_file, m_vbuf, _IOFBF, sizeof(BUF_SIZE));

#else

	int flags = 0;

	if (m_access & CREATE)
	{
		flags |= _O_CREAT;
	}
	if (m_access & TRUNCATE)
	{
		flags |= _O_TRUNC;
	}
	if (m_access & APPEND)
	{
		flags |= _O_APPEND;
	}
	if (m_access & TEXT)
	{
		flags |= _O_TEXT;
	}
	if (m_access & BINARY)
	{
		flags |= _O_BINARY;
	}

	if((m_access & READWRITE )== READWRITE )
	{
		flags |= _O_RDWR;
	}
	else if(m_access & WRITE)
	{
		flags |= _O_WRONLY;
		flags |= _O_CREAT;
	}
	else
	{
		flags |= _O_RDONLY;
	}

	m_handle = _open( filename, flags , _S_IREAD | _S_IWRITE);

	if( m_handle == -1 )
	{
		goto error;
	}

#endif

	++s_totalOpen;
///	DEBUG_LOG(("LocalFile::open %s (total %d)\n",filename,s_totalOpen));
	if ( m_access & APPEND )
	{
		if ( seek ( 0, END ) < 0 )
		{
			goto error;
		}
	}

	return TRUE;

error:

	close();

	return FALSE;
}

//=================================================================
// LocalFile::close 	
//=================================================================
/**
	* Closes the current file if it is open.
  * Must call LocalFile::close() for each successful LocalFile::open() call.
	*/
//=================================================================

void LocalFile::close( void )
{
	File::close();
}

//=================================================================
// LocalFile::read 
//=================================================================

Int LocalFile::read( void *buffer, Int bytes )
{
	//USE_PERF_TIMER(LocalFile)
	if( !m_open )
	{
		return -1;
	}

	if (buffer == NULL) 
	{
#ifdef USE_BUFFERED_IO
		fseek(m_file, bytes, SEEK_CUR);
#else
		_lseek(m_handle, bytes, SEEK_CUR);
#endif
		return bytes;
	}

#ifdef USE_BUFFERED_IO
	Int ret = fread(buffer, 1, bytes, m_file);
#else
	Int ret = _read( m_handle, buffer, bytes );
#endif

	return ret;
}

//=================================================================
// LocalFile::write 
//=================================================================

Int LocalFile::write( const void *buffer, Int bytes )
{

	if( !m_open || !buffer )
	{
		return -1;
	}

#ifdef USE_BUFFERED_IO
	Int ret = fwrite(buffer, 1, bytes, m_file);
#else
	Int ret = _write( m_handle, buffer, bytes );
#endif
	return ret;
}

//=================================================================
// LocalFile::seek 
//=================================================================

Int LocalFile::seek( Int pos, seekMode mode)
{
	int lmode;

	switch( mode )
	{
		case START:
			lmode = SEEK_SET;
			break;
		case CURRENT:
			lmode = SEEK_CUR;
			break;
		case END:
			DEBUG_ASSERTCRASH(pos <= 0, ("LocalFile::seek - pos should be <= 0 for a seek starting at the end of the file"));
			lmode = SEEK_END;
			break;
		default:
			// bad seek mode
			return -1;
	}

#ifdef USE_BUFFERED_IO
	Int ret = fseek(m_file, pos, lmode);
	if (ret == 0)
		return ftell(m_file);
	else
		return -1;
#else
	Int ret = _lseek( m_handle, pos, lmode );
#endif
	return ret;
}

//=================================================================
// LocalFile::scanInt
//=================================================================
// skips preceding whitespace and stops at the first non-number
// or at EOF
Bool LocalFile::scanInt(Int &newInt)
{
	newInt = 0;
	AsciiString tempstr;
	Char c;
	Int val;

	// skip preceding non-numeric characters
	do {
#ifdef USE_BUFFERED_IO
		val = fread(&c, 1, 1, m_file);
#else
		val = _read( m_handle, &c, 1);
#endif
	} while ((val != 0) && (((c < '0') || (c > '9')) && (c != '-')));

	if (val == 0) {
		return FALSE;
	}

	do {
		tempstr.concat(c);
#ifdef USE_BUFFERED_IO
		val = fread(&c, 1, 1, m_file);
#else
		val = _read( m_handle, &c, 1);
#endif
	} while ((val != 0) && ((c >= '0') && (c <= '9')));

	// put the last read char back, since we didn't use it.
	if (val != 0) {
#ifdef USE_BUFFERED_IO
		fseek(m_file, -1, SEEK_CUR);
#else
		_lseek(m_handle, -1, SEEK_CUR);
#endif
	}

	newInt = atoi(tempstr.str());
	return TRUE;
}

//=================================================================
// LocalFile::scanReal
//=================================================================
// skips preceding whitespace and stops at the first non-number
// or at EOF
Bool LocalFile::scanReal(Real &newReal) 
{
	newReal = 0.0;
	AsciiString tempstr;
	Char c;
	Int val;
	Bool sawDec = FALSE;

	// skip the preceding white space
	do {
#ifdef USE_BUFFERED_IO
		val = fread(&c, 1, 1, m_file);
#else
		val = _read( m_handle, &c, 1);
#endif
	} while ((val != 0) && (((c < '0') || (c > '9')) && (c != '-') && (c != '.')));

	if (val == 0) {
		return FALSE;
	}

	do {
		tempstr.concat(c);
		if (c == '.') {
			sawDec = TRUE;
		}
#ifdef USE_BUFFERED_IO
		val = fread(&c, 1, 1, m_file);
#else
		val = _read(m_handle, &c, 1);
#endif
	} while ((val != 0) && (((c >= '0') && (c <= '9')) || ((c == '.') && !sawDec)));

	if (val != 0) {
#ifdef USE_BUFFERED_IO
		fseek(m_file, -1, SEEK_CUR);
#else
		_lseek(m_handle, -1, SEEK_CUR);
#endif
	}

	newReal = atof(tempstr.str());
	return TRUE;
}

//=================================================================
// LocalFile::scanString
//=================================================================
// skips preceding whitespace and stops at the first whitespace
// or at EOF
Bool LocalFile::scanString(AsciiString &newString) 
{
	Char c;
	Int val;

	newString.clear();

	// skip the preceding whitespace
	do {
#ifdef USE_BUFFERED_IO
		val = fread(&c, 1, 1, m_file);
#else
		val = _read(m_handle, &c, 1);
#endif
	} while ((val != 0) && (isspace(c)));

	if (val == 0) {
		return FALSE;
	}

	do {
		newString.concat(c);
#ifdef USE_BUFFERED_IO
		val = fread(&c, 1, 1, m_file);
#else
		val = _read(m_handle, &c, 1);
#endif
	} while ((val != 0) && (!isspace(c)));

	if (val != 0) {
#ifdef USE_BUFFERED_IO
		fseek(m_file, -1, SEEK_CUR);
#else
		_lseek(m_handle, -1, SEEK_CUR);
#endif
	}

	return TRUE;
}

//=================================================================
// LocalFile::nextLine
//=================================================================
// scans to the first character after a new-line or at EOF
void LocalFile::nextLine(Char *buf, Int bufSize) 
{
	Char c = 0;
	Int val;
	Int i = 0;

	// seek to the next new-line.
	do {
		if ((buf == NULL) || (i >= (bufSize-1))) {
#ifdef USE_BUFFERED_IO
			val = fread(&c, 1, 1, m_file);
#else
			val = _read(m_handle, &c, 1);
#endif
		} else {
#ifdef USE_BUFFERED_IO
			val = fread(buf + i, 1, 1, m_file);
#else
			val = _read(m_handle, buf + i, 1);
#endif
			c = buf[i];
		}
		++i;
	} while ((val != 0) && (c != '\n'));

	if (buf != NULL) {
		if (i < bufSize) {
			buf[i] = 0;
		} else {
			buf[bufSize] = 0;
		}
	}
}

//=================================================================
//=================================================================
File* LocalFile::convertToRAMFile()
{
	RAMFile *ramFile = newInstance( RAMFile );
	if (ramFile->open(this)) 
	{
		if (this->m_deleteOnClose)
		{
			ramFile->deleteOnClose();
			this->close(); // is deleteonclose, so should delete.
		}
		else
		{
			this->close();
			this->deleteInstance();
		}
		return ramFile;
	}	
	else 
	{
		ramFile->close();
		ramFile->deleteInstance();
		return this;
	}
}

//=================================================================
// LocalFile::readEntireAndClose
//=================================================================
/**
	Allocate a buffer large enough to hold entire file, read 
	the entire file into the buffer, then close the file.
	the buffer is owned by the caller, who is responsible
	for freeing is (via delete[]). This is a Good Thing to
	use because it minimizes memory copies for BIG files.
*/
char* LocalFile::readEntireAndClose()
{
	UnsignedInt fileSize = size();
	char* buffer = NEW char[fileSize];

	read(buffer, fileSize);

	close();

	return buffer;
}
