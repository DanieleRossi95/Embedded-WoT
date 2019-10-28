//////////////////////////////////////////////////////////////////////////////////////
//						 		FishinoDebug.h										//
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
#ifndef __FISHINO_DEBUG_H
#define __FISHINO_DEBUG_H

#include <FishinoPrintf.h>

#ifdef _FISHINO_PIC32_
#include <cp0defs.h>
#include <setjmp.h>
#endif

#if defined(DEBUG_LEVEL_FATAL) || defined(DEBUG_LEVEL_ERROR) || defined(DEBUG_LEVEL_WARNING) || defined(DEBUG_LEVEL_INFO)
	#ifndef DEBUG
		#define DEBUG
	#endif
#endif

#ifdef DEBUG_LEVEL_INFO
	#ifndef DEBUG_LEVEL_WARNING
		#define DEBUG_LEVEL_WARNING
	#endif
#endif
#ifdef DEBUG_LEVEL_WARNING
	#ifndef DEBUG_LEVEL_ERROR
		#define DEBUG_LEVEL_ERROR
	#endif
#endif
#ifdef DEBUG_LEVEL_ERROR
	#ifndef DEBUG_LEVEL_FATAL
		#define DEBUG_LEVEL_FATAL
	#endif
#endif


#ifdef __cplusplus
extern "C"
{
#endif

	void _general_exception_handler(unsigned cause, unsigned status, uint32_t *stack, uint32_t *stackEnd);

	// convert a binary to its string representation
	const char *__debug__binString(uint32_t val, uint8_t bytes );
	
	// hex dump
	void __debug__hexDump(void const *ptr, uint32_t len);
	
	// binary dump
	void __debug__binDump(void const *ptr, uint32_t len);
	
	// stack dump
#ifdef _FISHINO_PIC32_
	void __debug__dump(void);
#endif
	
	// a couple of malloc hooks
	void *__debug__malloc(size_t size);
	void *__debug__calloc(size_t num, size_t size);
	void *__debug__realloc(void *ptr, size_t size);
	void __debug__free(void *ptr);

	extern void __debug__display__mallinfo__(void);
	extern uint32_t __debug__freeram__(void);

#ifdef __cplusplus
}
#endif
	
#ifdef DEBUG

	// helper for debug printouts
	#define __DEBUG__PRINT__FUNCTION__(MESSAGE, ...) debugPrintfP(F("%s:" MESSAGE), __FUNCTION__, ##__VA_ARGS__)
	#define __DEBUG__PRINT__(MESSAGE, ...) debugPrintfP(F(MESSAGE), ##__VA_ARGS__)
	
	#define DEBUG_SET_STREAM(s) setDebugStream(s)
	
	#define DEBUG_PRINT(MESSAGE, ...) __DEBUG__PRINT__(MESSAGE, ##__VA_ARGS__)
	#define DEBUG_PRINT_FUNCTION(MESSAGE, ...) __DEBUG__PRINT__FUNCTION__(MESSAGE, ##__VA_ARGS__)
	
	// return a binary string from number
	#define DEBUG_BIN(__val, ...) [](uint32_t val, uint8_t bytes = 4) \
		{											\
			static char bitStr[33];					\
			uint8_t bits = 4 * bytes;				\
			uint32_t bit = 1 << bits;				\
			char *bitP = bitStr;					\
			while(bits--)							\
			{										\
				*bitP++ = (val & bit) ? '1' : '0';	\
				bit >>= 1;							\
			}										\
			*bitP = 0;								\
			return bitStr;							\
		}											\
		(__val,  ##__VA_ARGS__)
	
	#ifdef DEBUG_LEVEL_INFO
		#define DEBUG_INFO(MESSAGE, ...) __DEBUG__PRINT__("INFO:" MESSAGE, ##__VA_ARGS__)
		#define DEBUG_INFO_N(MESSAGE, ...) __DEBUG__PRINT__(MESSAGE, ##__VA_ARGS__)
		#define DEBUG_INFO_FUNCTION(MESSAGE, ...) __DEBUG__PRINT__FUNCTION__("INFO:" MESSAGE, ##__VA_ARGS__)
		#define DEBUG_INFO_FUNCTION_N(MESSAGE, ...) __DEBUG__PRINT__FUNCTION__(MESSAGE, ##__VA_ARGS__)
	#else
		#define DEBUG_INFO(MESSAGE, ...)
		#define DEBUG_INFO_N(MESSAGE, ...)
		#define DEBUG_INFO_FUNCTION(MESSAGE, ...)
		#define DEBUG_INFO_FUNCTION_N(MESSAGE, ...)
	#endif
	
	#ifdef DEBUG_LEVEL_WARNING
		#define DEBUG_WARNING(MESSAGE, ...) __DEBUG__PRINT__("WARNING:" MESSAGE, ##__VA_ARGS__)
		#define DEBUG_WARNING_N(MESSAGE, ...) __DEBUG__PRINT__(MESSAGE, ##__VA_ARGS__)
		#define DEBUG_WARNING_FUNCTION(MESSAGE, ...) __DEBUG__PRINT__FUNCTION__("WARNING:" MESSAGE, ##__VA_ARGS__)
		#define DEBUG_WARNING_FUNCTION_N(MESSAGE, ...) __DEBUG__PRINT__FUNCTION__(MESSAGE, ##__VA_ARGS__)
	#else
		#define DEBUG_WARNING(MESSAGE, ...)
		#define DEBUG_WARNING_N(MESSAGE, ...)
		#define DEBUG_WARNING_FUNCTION(MESSAGE, ...)
		#define DEBUG_WARNING_FUNCTION_N(MESSAGE, ...)
	#endif
	
	#ifdef DEBUG_LEVEL_ERROR
		#define DEBUG_ERROR(MESSAGE, ...) __DEBUG__PRINT__("ERROR:" MESSAGE, ##__VA_ARGS__)
		#define DEBUG_ERROR_N(MESSAGE, ...) __DEBUG__PRINT__(MESSAGE, ##__VA_ARGS__)
		#define DEBUG_ERROR_FUNCTION(MESSAGE, ...) __DEBUG__PRINT__FUNCTION__("ERROR:" MESSAGE, ##__VA_ARGS__)
		#define DEBUG_ERROR_FUNCTION_N(MESSAGE, ...) __DEBUG__PRINT__FUNCTION__(MESSAGE, ##__VA_ARGS__)
	#else
		#define DEBUG_ERROR(MESSAGE, ...)
		#define DEBUG_ERROR_N(MESSAGE, ...)
		#define DEBUG_ERROR_FUNCTION(MESSAGE, ...)
		#define DEBUG_ERROR_FUNCTION_N(MESSAGE, ...)
	#endif
	
	#ifdef DEBUG_LEVEL_FATAL
		#define DEBUG_FATAL(MESSAGE, ...) __DEBUG__PRINT__("FATAL:" MESSAGE, ##__VA_ARGS__)
		#define DEBUG_FATAL_N(MESSAGE, ...) __DEBUG__PRINT__(MESSAGE, ##__VA_ARGS__)
		#define DEBUG_FATAL_FUNCTION(MESSAGE, ...) __DEBUG__PRINT__FUNCTION__("FATAL:" MESSAGE, ##__VA_ARGS__)
		#define DEBUG_FATAL_FUNCTION_N(MESSAGE, ...) __DEBUG__PRINT__FUNCTION__(MESSAGE, ##__VA_ARGS__)
	#else
		#define DEBUG_FATAL(MESSAGE, ...)
		#define DEBUG_FATAL_N(MESSAGE, ...)
		#define DEBUG_FATAL_FUNCTION(MESSAGE, ...)
		#define DEBUG_FATAL_FUNCTION_N(MESSAGE, ...)
	#endif
	
	#define DEBUG_HEXDUMP(ptr, len) __debug__hexDump(ptr, len)
	#define DEBUG_BINDUMP(ptr, len) __debug__binDump(ptr, len)
	
	#ifdef DEBUG_MEMORY_ALLOC
		#define DEBUG_MALLOC(size)			__debug__malloc(size)
		#define DEBUG_CALLOC(num, size)		__debug__calloc(num, size)
		#define DEBUG_REALLOC(ptr, size)	__debug__realloc(ptr, size)
		#define DEBUG_FREE(ptr)				__debug__free(ptr)
	#else
		#define DEBUG_MALLOC(size)			malloc(size)
		#define DEBUG_CALLOC(num, size)		calloc(num, size)
		#define DEBUG_REALLOC(ptr, size)	realloc(ptr, size)
		#define DEBUG_FREE(ptr)				free(ptr)
	#endif
	
	#define DEBUG_FREE_MEMORY()				__DEBUG__PRINT__("FREE MEMORY : %" PRIu32 " bytes\n", __debug__freeram__())
	#ifdef _FISHINO_PIC32_ 
		#define DEBUG_MEMORY_INFO()				__debug__display__mallinfo__()
	#else
		#define DEBUG_MEMORY_INFO()
	#endif

#else

	#define DEBUG_SET_STREAM(s)
	
	#define DEBUG_PRINT(MESSAGE, ...)
	#define DEBUG_PRINT_FUNCTION(MESSAGE, ...)
	
	#define DEBUG_BIN(val, ...) ""
	
	#define DEBUG_INFO(MESSAGE, ...)
	#define DEBUG_INFO_N(MESSAGE, ...)
	#define DEBUG_INFO_FUNCTION(MESSAGE, ...)
	#define DEBUG_INFO_FUNCTION_N(MESSAGE, ...)
	
	#define DEBUG_WARNING(MESSAGE, ...)
	#define DEBUG_WARNING_N(MESSAGE, ...)
	#define DEBUG_WARNING_FUNCTION(MESSAGE, ...)
	#define DEBUG_WARNING_FUNCTION_N(MESSAGE, ...)
	
	#define DEBUG_ERROR(MESSAGE, ...)
	#define DEBUG_ERROR_N(MESSAGE, ...)
	#define DEBUG_ERROR_FUNCTION(MESSAGE, ...)
	#define DEBUG_ERROR_FUNCTION_N(MESSAGE, ...)
	
	#define DEBUG_FATAL(MESSAGE, ...)
	#define DEBUG_FATAL_N(MESSAGE, ...)
	#define DEBUG_FATAL_FUNCTION(MESSAGE, ...)
	#define DEBUG_FATAL_FUNCTION_N(MESSAGE, ...)

	#define DEBUG_HEXDUMP(ptr, len)
	#define DEBUG_BINDUMP(ptr, len)
	
	#define DEBUG_MALLOC(size)			malloc(size)
	#define DEBUG_CALLOC(num, size)		calloc(num, size)
	#define DEBUG_REALLOC(ptr, size)	realloc(ptr, size)
	#define DEBUG_FREE(ptr)				free(ptr)

	#define DEBUG_FREE_MEMORY()
	#define DEBUG_MEMORY_INFO()

#endif

#endif
