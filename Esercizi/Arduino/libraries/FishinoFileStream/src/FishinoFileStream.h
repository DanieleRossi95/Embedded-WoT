//////////////////////////////////////////////////////////////////////////////////////
//																					//
//							FishinoFileStream.h										//
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
#ifndef __FISHINOFILESTREAM_H
#define __FISHINOFILESTREAM_H

#include <FishinoStream.h>
#include <FishinoSdFat.h>

class FishinoFileStream : public FishinoStream
{
	private:
	
		FatFile _file;
	
	protected:
	
	public:
	
		// read data
		// return number of actually ridden bytes
		virtual uint32_t read(uint8_t *buf, uint32_t len);
		
		// write data
		// return number of actually written bytes
		virtual uint32_t write(uint8_t const *buf, uint32_t len);
		
		// seeks stream
		// return true on success, false on failure or if stream is non seekable
		virtual bool seek(int32_t pos, uint8_t whence);
		
		// tells stream position
		// return current file position
		virtual uint32_t tell(void) const;
		
		// skip bytes
		// return true on success, false otherwise
		virtual bool skip(uint32_t nBytes);
		
		// check if stream has some data to read
		// return true if data is available, false otherwise
		virtual bool available(void) const;
		
		// check if read only
		virtual bool isReadOnly(void) const;
		
		// check for eof
		virtual bool isEof(void) const;
		
		// check for error
		virtual bool isError(void);

		// open the file
		// oFlag is an OR of following values:
		// O_READ		Open for reading.
		// O_RDONLY		Same as O_READ.
		// O_WRITE		Open for writing.
		// O_WRONLY		Same as O_WRITE.
		// O_RDWR		Open for reading and writing.
		// O_APPEND		If set, the file offset shall be set to the end of the
		// O_AT_END		Set the initial position at the end of the file.
		//				under O_EXCL below. Otherwise, the file shall be created
		// O_EXCL		If O_CREAT and O_EXCL are set, open() shall fail if the file exists.
		// O_SYNC		Call sync() after each write.  This flag should not be used with
		//				write(uint8_t) or any functions do character at a time writes since sync()
		//				will be called after each byte.
		// O_TRUNC		If the file exists and is a regular file, and the file is
		//				successfully opened and is not read only, its length shall be truncated to 0.
 		virtual bool open(const char *path, uint8_t oFlag = O_READ);
		
		// close the file
		virtual void close(void);
		
		// flush the file on writing
		virtual void flush(void);
		
		// check if file is opened
		virtual bool isOpened(void) const;

		// peeks some bytes from input stream
		// copy at most reqSize bytes on dest buffer
		// returns size of available data (< reqSize) or 0 if can't peek
		virtual uint32_t peekBuffer(uint8_t *buf, uint32_t reqSize);

		// constructor
		FishinoFileStream();

		// destructor
		~FishinoFileStream();
};

#endif
