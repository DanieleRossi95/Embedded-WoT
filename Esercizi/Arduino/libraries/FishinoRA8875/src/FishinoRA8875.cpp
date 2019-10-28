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
#include <SPI.h>

#include <FishinoFlash.h>

// map RA8875 screen sizes with GFX ones
struct RA8875_SCREEN_SIZES
{
	uint16_t width;
	uint16_t height;
};

RA8875_SCREEN_SIZES ra8875_screen_sizes[] = {
	{ 320, 240 },		// RA8875_320x240
	{ 480, 272 },		// RA8875_480x272
	{ 800, 480 },		// RA8875_800x480
	{ 480, 272 },		// Adafruit_480x272
	{ 800, 480 },		// Adafruit_800x480
	{ 640, 480 },		// RA8875_640x480
};

// constructor
FishinoRA8875Class::FishinoRA8875Class(RA8875sizes size)
	: FishinoGFX(ra8875_screen_sizes[size].width, ra8875_screen_sizes[size].height)
{
	_size = size;
}
