//////////////////////////////////////////////////////////////////////////////////////
//																					//
//							FishinoFlashExample.cpp									//
//					Example code for FishinoFlash library							//
//					Created by Massimo Del Fedele, 2017								//
//																					//
//  Copyright (c) 2016, 2017 Massimo Del Fedele.  All rights reserved.				//
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
//	VERSION 1.0.0 - INITIAL VERSION													//
//	VERSION 2.0.0 - 06/01/2016 - REWROTE SPI INTERFACE AND ERROR HANDLING			//
//	VERSION 4.0.0 - 01/01/2017 - REWROTE SPI INTERFACE AND ERROR HANDLING			//
//	VERSION 5.1.0 - 04/05/2017 - USE NEW DEBUG LIBRARY								//
//	VERSION 5.2.0 - 20/05/2017 - USE NEW DEBUG LIBRARY								//
//  Version 6.0.0 - June 2017  - USE NEW DEBUG LIBRARY								//
//  Version 7.0.0 - July 2017  - ADDED NEW, EASY TO USE CLASSES						//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#include <FishinoFlash.h>

#define DEBUG_LEVEL_INFO
#include <FishinoDebug.h>

// a flash string example
FlashString flashString = FString("This is a string in flash memory");

// a flash string array example
FlashStringArray flashStringArray = FStringArray(
	"This is a simple string array",
	"located in flash memory",
	"you can use it almost as a normal C character array",
	"just use 'FStringArray(...)' macro to define it"
);

// an array of bytes in flash
FlashUint8Array flashUint8Array = FUint8Array(
	12, 34, 56, 78, 22, 44, 66, 88, 99,
	12, 34, 56, 78, 22, 44, 66, 88, 99,
	12, 34, 56, 78, 22, 44, 66, 88, 99
);

// an array of words (uint16_t) in flash
FlashUint16Array flashUint16Array = FUint16Array(
	1234, 5678, 9012, 3456,
	1234, 5678, 9012, 3456,
	1234, 5678, 9012, 3456
);

// an array of dwords (uint32_t) in flash
FlashUint32Array flashUint32Array = FUint32Array(
	12345678, 67891021, 66676358, 24409876,
	12345678, 67891021, 66676358, 24409876,
	12345678, 67891021, 66676358, 24409876
);

void setup(void)
{
/*
#ifdef _FISHINO_PIC32_
	DEBUG_SET_STREAM(Serial0);
	Serial0.begin(115200);
#else
*/
	DEBUG_SET_STREAM(Serial);
//#endif

	// setup serial port and wait for serial monitor
	Serial.begin(115200);
	while (!Serial)
		;
	Serial << "\n\n\n";

	// print a sample flash string
	Serial << FString("A simple flash string:\n");
	Serial << flashString << "\n\n";

	// print a sample flash string array
	Serial << FString("A simple flash string array with ") << flashStringArray.getCount() << FString(" lines\n");
	for (size_t i = 0; i < flashStringArray.getCount(); i++)
		Serial << flashStringArray[i] << "\n";
	Serial << "\n";
	
	// print a sample byte array
	Serial << FString("A simple byte (uint8_t) array with ") << flashUint8Array.getCount() << FString(" elements\n");
	for(size_t i = 0; i < flashUint8Array.getCount(); i++)
		Serial << flashUint8Array[i] << ' ';
	Serial << "\n\n";

	// print a sample word array
	Serial << FString("A simple word (uint16_t) array with ") << flashUint16Array.getCount() << FString(" elements\n");
	for(size_t i = 0; i < flashUint16Array.getCount(); i++)
		Serial << flashUint16Array[i] << ' ';
	Serial << "\n\n";

	// print a sample dword array
	Serial << FString("A simple dword (uint32_t) array with ") << flashUint32Array.getCount() << FString(" elements\n");
	for(size_t i = 0; i < flashUint32Array.getCount(); i++)
		Serial << flashUint32Array[i] << ' ';
	Serial << "\n\n";
}

void loop(void)
{
	// nothing to do here
}
