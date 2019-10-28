//////////////////////////////////////////////////////////////////////////////////////
//																					//
//							FishinoFileStream.cpp										//
//				A stream class able to handle files on SD card						//
//					Copyright (c) 2017 Massimo Del Fedele							//
//																					//
//	Redistribution and use in source and binary forms, with or without				//
//	modification, are permitted provided that the following conditions are met:		//
//																					//
//	- Redistributions of source code must retain the above copyright notice,		//
//	  this list of conditions and the following disclaimer.							//
//	- Redistributions in binary form must reproduce the above copyright notice,		//
//	  this list of conditions and the following disclaimer in the documentation		//
//	  and/or other materials provided with the distribution.						//
//																					//	
//	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"		//
//	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE		//
//	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE		//
//	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE		//
//	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR				//
//	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF			//
//	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS		//
//	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN			//
//	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)			//
//	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE		//
//	POSSIBILITY OF SUCH DAMAGE.														//
//																					//
//	VERSION 1.0.0	2017		INITIAL VERSION										//
//  Version 6.0.2	June 2017	Some small fixes									//
//																					//
//////////////////////////////////////////////////////////////////////////////////////

#include "FishinoFileStream.h"

#define DEBUG_LEVEL_INFO
#include <FishinoDebug.h>

// read data
// return number of actually ridden bytes
uint32_t FishinoFileStream::read(uint8_t *buf, uint32_t len)
{
	return _file.read(buf, len);
}

// write data
// return number of actually written bytes
uint32_t FishinoFileStream::write(uint8_t const *buf, uint32_t len)
{
	return _file.write(buf, len);
}

// seeks stream
// return true on success, false on failure or if stream is non seekable
bool FishinoFileStream::seek(int32_t pos, uint8_t whence)
{
	switch(whence)
	{
		default:
		case SEEK_SET:
			return _file.seekSet(pos);
			
		case SEEK_CUR:
			return _file.seekCur(pos);
			
		case SEEK_END:
			return _file.seekEnd(pos);
	}
}

// tells stream position
// return current file position
uint32_t FishinoFileStream::tell(void) const
{
	uint32_t curPos = _file.curPosition();
	return curPos;
}

// skip bytes
// return true on success, false otherwise
bool FishinoFileStream::skip(uint32_t nBytes)
{
	if(_file.seekCur(nBytes))
		return nBytes;
	return 0;
}

// check if stream has some data to read
// return true if data is available, false otherwise
bool FishinoFileStream::available(void) const
{
	return _file.curPosition() < _file.fileSize();
}		

// check if read only
bool FishinoFileStream::isReadOnly(void) const
{
	return _file.isReadOnly();
}

// check for eof
bool FishinoFileStream::isEof(void) const
{
	return _file.curPosition() >= _file.fileSize();
}

// check for error
bool FishinoFileStream::isError(void)
{
	return _file.getError();
}

// open the file
bool FishinoFileStream::open(const char *path, uint8_t oFlag)
{
	return _file.open(path, oFlag);
}

// close the file
void FishinoFileStream::close(void)
{
	_file.close();
}

// flush the file on writing
void FishinoFileStream::flush(void)
{
	_file.sync();
}
		

// check if file is opened
bool FishinoFileStream::isOpened(void) const
{
	return _file.isOpen();
}

// peeks some bytes from input stream
// copy at most reqSize bytes on dest buffer
// returns size of available data (< reqSize) or 0 if can't peek
uint32_t FishinoFileStream::peekBuffer(uint8_t *buf, uint32_t reqSize)
{
	if(!isOpened())
		return 0;
	
	// store current file position
	uint32_t pos = tell();
	
	// read some data into buffer
	uint32_t res = read(buf, reqSize);
	
	// reset file position
	seek(pos, SEEK_SET);
	
	return res;
}


// constructor
FishinoFileStream::FishinoFileStream()
{
}

// destructor
FishinoFileStream::~FishinoFileStream()
{
	close();
}
