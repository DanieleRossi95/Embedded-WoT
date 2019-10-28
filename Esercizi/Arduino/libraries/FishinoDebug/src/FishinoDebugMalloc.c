//////////////////////////////////////////////////////////////////////////////////////
//					 		FishinoDebugMalloc.cpp									//
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
#define DEBUG
#include "FishinoDebug.h"

static size_t __malloc__allocated = 0;

// a couple of malloc hooks
void *__debug__malloc(size_t size)
{
	debugPrintfP(F("MALLOC(%d) - Previous allocated is %d\n"), (int)size, (int)__malloc__allocated);
	uint32_t *ptr = (uint32_t *)malloc(size + sizeof(uint32_t));
	if(!ptr)
		debugPrintfP(F("MALLOC(%d) ERROR\n"), (int)size);
	else
		*ptr++ = size;

	// keep track of allocations
	__malloc__allocated += size;
	
	return (void *)ptr;
}

void *__debug__calloc(size_t num, size_t size)
{
	return __debug__malloc(num * size);
}

void *__debug__realloc(void *ptr, size_t size)
{
	if(!ptr)
		return __debug__malloc(size);
	uint32_t *oldPtr = (uint32_t *)ptr;
	oldPtr--;
	size_t oldSize = *oldPtr;
	debugPrintfP(F("REALLOC(%p, %d) - old size was %d - Previous allocated is %d\n"), ptr, (int)size, (int)oldSize, (int)__malloc__allocated);
	uint32_t *newPtr = (uint32_t *)realloc(oldPtr, size + sizeof(uint32_t));
	if(!newPtr)
		debugPrintfP(F("REALLOC(%p, %d) ERROR\n"), ptr, (int)size);
	else
		*newPtr++ = size;
	
	// keep track of allocations
	__malloc__allocated -= oldSize;
	__malloc__allocated += size;
	
	return (void *)newPtr;
}

void __debug__free(void *ptr)
{
	if(!ptr)
		debugPrintfP(F("FREEING NULL POINTER\n"));
	else
	{
		uint32_t *p32 = (uint32_t *)ptr;
		p32--;
		__malloc__allocated -= *p32;
		debugPrintfP(F("FREEING %d bytes, still %d allocated\n"), (int)*p32, (int)__malloc__allocated);
		free(p32);
	}
}

