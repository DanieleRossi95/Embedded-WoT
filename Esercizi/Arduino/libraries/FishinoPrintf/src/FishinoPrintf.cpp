//////////////////////////////////////////////////////////////////////////////////////
//						 		FishinoPrintf.cpp									//
//																					//
//					Some useful printf functions for Arduino streams				//
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
//  Version 1.0.0 - 29/10/2017		Initial version									//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#include <FishinoPrintf.h>

#ifndef _FISHINO_PIC32_
#include "avr/pgmspace.h"
#endif

	static bool __fishinoPrintfInitialized = false;
	static Stream *__fishinoPrintfStream = &Serial;

#ifdef _FISHINO_PIC32_
extern "C"
{
	struct _reent;
	long _write_r(struct _reent *ptr, int fd, const void *buf, size_t cnt)
	{
		size_t l = cnt;
		const char *p = (const char *)buf;
		while(l--)
			__fishinoPrintfStream->write(*p++);
	    return cnt;
	}
}
#else

	static FILE __fishinoPrintfFile = {0};
	
	int __fishinoPrintfPutchar (char c, FILE *stream)
	{
	    __fishinoPrintfStream->write(c);
	    return 0 ;
	}
	
#endif

// printf on a stream, using normal char *
void streamVPrintf(Stream &stream, const char *fmt, va_list args)
{
	if(!__fishinoPrintfInitialized)
	{
#ifdef _FISHINO_PIC32_

		// disable buffer for standard streams
		setbuf(stdin, NULL);
		setbuf(stdout, NULL);
		
#else

		fdev_setup_stream (&__fishinoPrintfFile, __fishinoPrintfPutchar, NULL, _FDEV_SETUP_WRITE);
		stdout = &__fishinoPrintfFile;
		
#endif

		__fishinoPrintfInitialized = true;
	}
	
	// move temporarily the output stream
	Stream *prevStream = __fishinoPrintfStream;
	__fishinoPrintfStream = &stream;

	vprintf(fmt, args);

	// restore default stream
	__fishinoPrintfStream = prevStream;
}

void streamPrintf(Stream &stream, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	streamVPrintf(stream, fmt, args);
	va_end(args);
}

// same using PROGMEM strings as format
void streamVPrintfP(Stream &stream, const __FlashStringHelper *fmt, va_list args)
{
#ifdef _FISHINO_PIC32_

	streamVPrintf(stream, (const char *)fmt, args);
	
#else

	if(!__fishinoPrintfInitialized)
	{
		fdev_setup_stream (&__fishinoPrintfFile, __fishinoPrintfPutchar, NULL, _FDEV_SETUP_WRITE);
		stdout = &__fishinoPrintfFile;
		__fishinoPrintfInitialized = true;
	}
	
	// move temporarily the output stream
	Stream *prevStream = __fishinoPrintfStream;
	__fishinoPrintfStream = &stream;

	vfprintf_P(stdout, (const char *)fmt, args);

	// restore default stream
	__fishinoPrintfStream = prevStream;
#endif
}

void streamPrintfP(Stream &stream, const __FlashStringHelper *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	streamVPrintfP(stream, fmt, args);
	va_end(args);
}

// some default streams - can be redirected at runtime
Stream *__outputStream	= &Serial;
Stream *__errorStream	= &Serial;
Stream *__debugStream	= &Serial;

// variants with fixed default streams
void outputVPrintf(const char *fmt, va_list args)
{
	streamVPrintf(*__outputStream, fmt, args);
}

void outputPrintf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	streamVPrintf(*__outputStream, fmt, args);
	va_end(args);
}

void outputVPrintfP(const __FlashStringHelper *fmt, va_list args)
{
	streamVPrintfP(*__outputStream, fmt, args);
}

void outputPrintfP(const __FlashStringHelper *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	streamVPrintfP(*__outputStream, fmt, args);
	va_end(args);
}

void errorVPrintf(const char *fmt, va_list args)
{
	streamVPrintf(*__errorStream, fmt, args);
}

void errorPrintf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	streamVPrintf(*__errorStream, fmt, args);
	va_end(args);
}

void errorVPrintfP(const __FlashStringHelper *fmt, va_list args)
{
	streamVPrintfP(*__errorStream, fmt, args);
}

void errorPrintfP(const __FlashStringHelper *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	streamVPrintfP(*__errorStream, fmt, args);
	va_end(args);
}

void debugVPrintf(const char *fmt, va_list args)
{
	streamVPrintf(*__debugStream, fmt, args);
}

void debugPrintf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	streamVPrintf(*__debugStream, fmt, args);
	va_end(args);
}

void debugVPrintfP(const __FlashStringHelper *fmt, va_list args)
{
	streamVPrintfP(*__debugStream, fmt, args);
}

void debugPrintfP(const __FlashStringHelper *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	streamVPrintfP(*__debugStream, fmt, args);
	va_end(args);
}

// set default output streams
void setOutputStream(Stream &stream)
{
	__outputStream = &stream;
}

void setErrorStream(Stream &stream)
{
	__errorStream = &stream;
}

void setDebugStream(Stream &stream)
{
	__debugStream = &stream;
}
