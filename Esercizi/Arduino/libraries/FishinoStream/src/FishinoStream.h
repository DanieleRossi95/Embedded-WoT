//////////////////////////////////////////////////////////////////////////////////////
//						 		FishinoStream.h										//
//																					//
//					Stream abstract class for Fishino Boards						//
//				Provides an uniform interface to local and remote files				//
//																					//
//		Copyright (c) 2017 Massimo Del Fedele.  All rights reserved.				//
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
//  Version 6.0.0 -- June 2017		Initial version									//
//  Version 6.0.2 -- June 2017		Added support to peek data on read buffer		//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#ifndef __FISHINOSTREAM_FISHINOSTREAM_H
#define __FISHINOSTREAM_FISHINOSTREAM_H

#include <Arduino.h>

#define FISHINO_STREAM_ERROR -1

class FishinoStream
{
	private:

	protected:
	
	public:
	
		// read data
		// return number of actually ridden bytes
		virtual uint32_t read(uint8_t *buf, uint32_t len) = 0;
		
		// write data
		// return number of actually written bytes
		virtual uint32_t write(uint8_t const *buf, uint32_t len) = 0;
		
		// seeks stream
		// return true on success, false on failure or if stream is non seekable
		virtual bool seek(int32_t pos, uint8_t whence) { return FISHINO_STREAM_ERROR; }
		
		// tells stream position
		// return current file position
		virtual uint32_t tell(void) const { return (uint32_t)FISHINO_STREAM_ERROR; }
		
		// skip bytes
		// return true on success, false otherwise
		virtual bool skip(uint32_t nBytes);
		
		// check if stream has some data to read
		// return true if data is available, false otherwise
		virtual bool available(void) { return !isEof(); }
		
		// check if read only
		virtual bool isReadOnly(void) const { return true; }
		
		// check for eof
		virtual bool isEof(void) const { return false; }
		
		// check for error
		virtual bool isError(void) { return false; }
		
		// peeks some bytes from input stream
		// copy at most reqSize bytes on dest buffer
		// returns size of available data (< reqSize) or 0 if can't peek
		virtual uint32_t peekBuffer(uint8_t *buf, uint32_t reqSize);

		// constructor
		FishinoStream();

		// destructor
		virtual ~FishinoStream();

};

#endif
