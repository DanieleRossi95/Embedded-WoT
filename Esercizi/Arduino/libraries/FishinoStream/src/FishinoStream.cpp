//////////////////////////////////////////////////////////////////////////////////////
//						 		FishinoStream.cpp									//
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

#include "FishinoStream.h"

//#define DEBUG_LEVEL_INFO
#include <FishinoDebug.h>

// constructor
FishinoStream::FishinoStream()
{
}

// destructor
FishinoStream::~FishinoStream()
{
}

// skip bytes
// return true on success, false otherwise
bool FishinoStream::skip(uint32_t nBytes)
{
	uint8_t buf[16];
	while(nBytes >= 16 && !isEof())
	{
		uint32_t n = read(buf, 16);
		nBytes -= n;
		if(n != 16)
			return false;
	}
	if(nBytes && !isEof())
		return read(buf, nBytes) == nBytes;
	return false;
}

// peeks some bytes from input stream
// copy at most reqSize bytes on dest buffer
// returns size of available data (< reqSize) or 0 if can't peek
uint32_t FishinoStream::peekBuffer(uint8_t *buf, uint32_t reqSize)
{
	// do not allow peek by default
	return 0;
}

