//////////////////////////////////////////////////////////////////////////////////////
//						 		FishinoDebug.cpp									//
//																					//
//						Some debug helpers for Fishino32							//
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
//  Version 5.1.0 -- April 2017		Initial version									//
//  Version 5.1.1 -- May 2017		Removed \n on DEBUG_PRINT macro					//
//  Version 5.2.0 -- May 2017		Renamed in FishinoDebug.h						//
//									Uniformed 8 and 32 bit boards code				//
//  Version 6.0.0 -- June 2017		Added stack dump code for 32 bit boards			//
//  Version 6.0.4 -- June 2017		Added memory info functions						//
//  Version 7.1.0 - 20/10/2017		Better free memory display						//
//  Version 7.2.0 - 29/10/2017		Use FishinoPrintf library						//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#include "FishinoDebug.h"

#ifndef ARDUINO_STREAMING
#define ARDUINO_STREAMING
template<class T> inline Print &operator <<(Print &stream, T arg)
{
	stream.print(arg);
	return stream;
}
#endif

#ifndef _FISHINO_PIC32_
#include "avr/pgmspace.h"
#endif

Print *__debug_serial = &Serial;

extern "C"
{
	// hex dump
	#define HEXDUMP_ROW_SIZE 16
	#define BINDUMP_ROW_SIZE 16
	
	static char __nibble2char(uint8_t nibble)
	{
		nibble &= 0x0f;
		if(nibble < 0x0a)
			return nibble + '0';
		else
			return nibble + 'a' - 10;
	}
	
	void __debug__hexDump(const void *ptr, uint32_t len)
	{
		const uint8_t *bPtr = (const uint8_t *)ptr;
		uint32_t rows = (len + HEXDUMP_ROW_SIZE - 1) / HEXDUMP_ROW_SIZE;
		for(uint32_t row = 0; row < rows && len; row++)
		{
			for(uint32_t col = 0; col < HEXDUMP_ROW_SIZE && len; col++, len--)
			{
				uint8_t b = *bPtr++;
				__debug_serial->print(__nibble2char(b >> 4));
				__debug_serial->print(__nibble2char(b));
				__debug_serial->print(' ');
			}
			__debug_serial->print("\r\n");
		}
	}
	
	// binary dump
	void __debug__binDump(void const *ptr, uint32_t len)
	{
		const uint8_t *bPtr = (const uint8_t *)ptr;
		uint32_t rows = (len + BINDUMP_ROW_SIZE - 1) / BINDUMP_ROW_SIZE;
		for(uint32_t row = 0; row < rows && len; row++)
		{
			for(uint32_t col = 0; col < BINDUMP_ROW_SIZE && len; col++, len--)
			{
				uint8_t b = *bPtr++;
				uint8_t mask = 0x80;
				for(uint8_t i = 0; i < 8; i++)
				{
					__debug_serial->print( (b & mask) ? "1" : "0");
					mask >>= 1;
				}
				__debug_serial->print(' ');
			}
			__debug_serial->print("\r\n");
		}
	}

	
	void __debug__set__stream(void *s)
	{
		__debug_serial = (Print *)s;
	}
	
	void *__debug__get__stream(void)
	{
		return __debug_serial;
	}
}
