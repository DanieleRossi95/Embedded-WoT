//////////////////////////////////////////////////////////////////////////////////////
//						 		StackDumpTest.ino									//
//																					//
//				Test stack dump features on Fishino 32 bit boards					//
//			For better results, compile with optimizations disabled					//
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
//  Version 7.2.0 - 29/10/2017		Use FishinoPrintf library						//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FISHINO_PIC32_
#error "This demo requires a Fishino 32 bit board!!!!"
#endif

#define DEBUG_LEVEL_INFO
#include <FishinoDebug.h>

// this function contains an error which triggers an exception
// (divide by zero)
void crash(int x)
{
	DEBUG_INFO("CRASHING\n");
	int b = 1 / x;
	DEBUG_INFO("RESULT IS %d\n", b);
}

// this function calls the function with error
void a(int x)
{
	int k = 2 * x;
	crash(k);
}

// this function calls 'a' function which in turn calls the error 'crash' function
void b(double x)
{
	int n = (int)x;
	a(n);
}

// this function calls 'b' function which calls 'a' function which in turn calls the error 'crash' function
void c(int m)
{
	double d = m * sin(0);
	b(d);
}

// we need an output on alternate serial to see the crash better
// you could try with USB serial too commenting following line
//#define USE_SERIAL_0

void setup()
{
#ifdef USE_SERIAL_0
	DEBUG_SET_STREAM(Serial0);
	Serial0.begin(115200);
#else
	DEBUG_SET_STREAM(Serial);
	Serial.begin(115200);
	while(!Serial)
		;
#endif
	
	// trigger the crash call infunction chain c-b-a-crash()
	c(2);
}

void loop()
{
}
