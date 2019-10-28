//////////////////////////////////////////////////////////////////////////////////////
//																					//
//							FishinoRA8875SPI.cpp									//
//		Hardware driver for TFT displays with RA8875 display controller				//
//								SPI interface										//
//					Created by Massimo Del Fedele, 2018								//
//																					//
//  Copyright (c) 2018 Massimo Del Fedele.  All rights reserved				.		//
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
//	Version 7.5.0 - 2018/02/02 - INITIAL VERSION									//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#include <FishinoRA8875SPI.h>

#include <FishinoFlash.h>

// constructor
FishinoRA8875SPIClass::FishinoRA8875SPIClass()
	: FishinoRA8875Class(FISHINO_RA8875SPI_SIZE)
{
	_cs = 0xff;
}

// destructor
FishinoRA8875SPIClass::~FishinoRA8875SPIClass()
{
	if(_cs != 0xff)
		pinMode(_cs, INPUT);
}

// initialize the interface
void FishinoRA8875SPIClass::begin(uint8_t cs)
{
	beginInterface(cs);
	FishinoRA8875Class::begin();
}


FishinoRA8875SPIClass &__GetFishinoRA8875SPI(void)
{
	static FishinoRA8875SPIClass tft;
	return tft;
}

