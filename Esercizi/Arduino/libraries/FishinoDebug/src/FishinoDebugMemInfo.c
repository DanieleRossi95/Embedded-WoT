//////////////////////////////////////////////////////////////////////////////////////
//					 		FishinoDebugMemInfo.cpp									//
//																					//
//						Some debug helpers for Fishino32							//
//					Give informations about allocated memory						//
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

#ifdef _FISHINO_PIC32_
#include <malloc.h>

void __debug__display__mallinfo__(void)
{
	struct mallinfo mi;

	mi = mallinfo();

	__DEBUG__PRINT__("Total non-mmapped bytes (arena):       %d\n", mi.arena);
	__DEBUG__PRINT__("# of free chunks (ordblks):            %d\n", mi.ordblks);
	__DEBUG__PRINT__("# of free fastbin blocks (smblks):     %d\n", mi.smblks);
	__DEBUG__PRINT__("# of mapped regions (hblks):           %d\n", mi.hblks);
	__DEBUG__PRINT__("Bytes in mapped regions (hblkhd):      %d\n", mi.hblkhd);
	__DEBUG__PRINT__("Max. total allocated space (usmblks):  %d\n", mi.usmblks);
	__DEBUG__PRINT__("Free bytes held in fastbins (fsmblks): %d\n", mi.fsmblks);
	__DEBUG__PRINT__("Total allocated space (uordblks):      %d\n", mi.uordblks);
	__DEBUG__PRINT__("Total free space (fordblks):           %d\n", mi.fordblks);
	__DEBUG__PRINT__("Topmost releasable block (keepcost):   %d\n", mi.keepcost);
}

// get available free ram
extern char* sbrk(int incr);
uint32_t __debug__freeram__(void)
{
	char top;
	malloc_trim(0);
	return &top - sbrk(0);
}
#else
uint32_t __debug__freeram__(void)
{
	extern int __heap_start, *__brkval; 
	int v;
	uint8_t *vp = (uint8_t *)&v;
	return vp - (__brkval == 0 ? (uint8_t *)&__heap_start : (uint8_t *)__brkval); 
}
#endif
