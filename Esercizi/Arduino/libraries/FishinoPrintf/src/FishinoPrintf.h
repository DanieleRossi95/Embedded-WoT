//////////////////////////////////////////////////////////////////////////////////////
//						 		FishinoPrintf.h										//
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
#ifndef __FISHINO_PRINTF__H
#define __FISHINO_PRINTF__H

#include <Arduino.h>
#include <stdarg.h>

#ifdef __cplusplus
	class __FlashStringHelper;
#else
	// this is to be able to use debugging functions in pure C code
	typedef struct {} __FlashStringHelper;
	#define F(x) ((const __FlashStringHelper *)PSTR(x))
#endif

// little trick to make usage possible from pure c
// (as long as a stream pointer is available)
#ifndef __cplusplus

	// printf on a stream, using normal char *
	void streamVPrintf(void *stream, const char *fmt, va_list args);
	void streamPrintf(void *stream, const char *fmt, ...);
	
	// same using PROGMEM strings as format
	void streamVPrintfP(void *stream, const __FlashStringHelper *fmt, va_list args);
	void streamPrintfP(void *stream, const __FlashStringHelper *fmt, ...);
	
	// set default output streams
	void setOutputStream(void *stream);
	void setErrorStream(void *stream);
	void setDebugStream(void *stream);

#else
extern "C"
{
	// printf on a stream, using normal char *
	void streamVPrintf(Stream &stream, const char *fmt, va_list args);
	void streamPrintf(Stream &stream, const char *fmt, ...) __attribute__ (( format( printf, 2, 3 ) ));
	
	// same using PROGMEM strings as format
	void streamVPrintfP(Stream &stream, const __FlashStringHelper *fmt, va_list args);
	void streamPrintfP(Stream &stream, const __FlashStringHelper *fmt, ...);

	// set default output streams
	void setOutputStream(Stream &stream);
	void setErrorStream(Stream &stream);
	void setDebugStream(Stream &stream);
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif
	// same functions on default streams

	void outputVPrintf(const char *fmt, va_list args);
	void outputPrintf(const char *fmt, ...) __attribute__ (( format( printf, 1, 2 ) ));
	void outputVPrintfP(const __FlashStringHelper *fmt, va_list args);
	void outputPrintfP(const __FlashStringHelper *fmt, ...);
	
	void errorVPrintf(const char *fmt, va_list args);
	void errorPrintf(const char *fmt, ...) __attribute__ (( format( printf, 1, 2 ) ));
	void errorVPrintfP(const __FlashStringHelper *fmt, va_list args);
	void errorPrintfP(const __FlashStringHelper *fmt, ...);
	
	void debugVPrintf(const char *fmt, va_list args);
	void debugPrintf(const char *fmt, ...) __attribute__ (( format( printf, 1, 2 ) ));
	void debugVPrintfP(const __FlashStringHelper *fmt, va_list args);
	void debugPrintfP(const __FlashStringHelper *fmt, ...);
	
#ifdef __cplusplus
}
#endif

#endif
