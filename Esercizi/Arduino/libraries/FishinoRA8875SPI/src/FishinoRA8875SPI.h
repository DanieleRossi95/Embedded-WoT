//////////////////////////////////////////////////////////////////////////////////////
//																					//
//							FishinoRA8875SPI.h										//
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
#ifndef _FISHINO_RA8875SPI_H_
#define _FISHINO_RA8875SPI_H_

#include <FishinoGFX.h>
#include <SPI.h>

#include <FishinoFlash.h>

#ifdef __AVR__
#include <math.h>
#endif

/* ---------------------------- USER SETTINGS ---------------------*/

// chip select pin (default, may be changed in begin()
#define FISHINO_RA8875SPI_CS	2

// display size -- must be set here
#define FISHINO_RA8875SPI_SIZE	RA8875_800x480

/* EXTERNAL TOUCH CONTROLLER ++++++++++++++++++++++++++++++++++++++++++
Some TFT come with capacitive touch screen or you may decide to use a better
controller for that, decomment the following line to save resources */
#define USE_EXTERNALTOUCH

/* INTERNAL KEY MATRIX ++++++++++++++++++++++++++++++++++++++++++
RA8875 has a 5x6 Key Matrix controller onboard, if you are not plan to use it
better leave commented the following define since it will share some registers
with several functions, otherwise de-comment it! */
//#define USE_RA8875_KEYMATRIX

/* DEFAULT CURSOR BLINK RATE ++++++++++++++++++++++++++++++++++++++++++++
Nothing special here, you can set the default blink rate */
#define DEFAULTCURSORBLINKRATE		10

/* DEFAULT INTERNAL FONT ENCODING ++++++++++++++++++++++++++++++++++++++++++++
RA8875 has 4 different font set, same shape but suitable for most languages
please look at RA8875 datasheet and choose the correct one for your language!
The default one it's the most common one and should work in most situations */
#define DEFAULTINTENCODING			ISO_IEC_8859_1//ISO_IEC_8859_2,ISO_IEC_8859_3,ISO_IEC_8859_4

#include "FishinoRA8875.h"

class FishinoRA8875SPIClass : public FishinoRA8875Class
{
	private:
		
		//////////////////////////////////////////////////////////
		//					INTERFACE....INTERFACE!				//
		//////////////////////////////////////////////////////////

		// initialize the interface
		void beginInterface(uint8_t cs);

		// low level access  commands
		void writeReg(uint8_t reg, uint8_t val);
		uint8_t readReg(uint8_t reg);
		
		// mid level access commands
		void writeCommand(uint8_t d);
		void writeData(uint8_t data);
		void writeData16(uint16_t data);
		uint8_t readData(void);
		uint8_t readStatus(void);

		// write a block of data, RAM version
		// bufLen is the number of pixels (2 byte) data
		void writeBuffer(uint16_t const *buf, size_t bufLen);
		
		// write a block of data, FLASH version
		// buf MUST be a 16 bit image buffer in flash area
		// bufLen is the number of pixels (2 byte) data
		void writeBuffer(const __FlashStringHelper *buf, size_t bufLen);

		// read-transparent-write a block of data, RAM version
		void writeBufferT(uint16_t const *buf, size_t bufLen, uint16_t transpColor);
		
		// read-transparent-write a block of data, Flash version
		// buffer MUST be a 16 bit image chunk
		void writeBufferT(const __FlashStringHelper *buf, size_t bufLen, uint16_t transpColor);

		// from adafruit
		boolean waitPoll(uint8_t r, uint8_t f);
		
		// 0x80, 0x40(BTE busy), 0x01(DMA busy)
		void waitBusy(uint8_t res=0x80);
		
		// the cs pin
		uint8_t _cs;
		
	public:
		
		// constructor
		FishinoRA8875SPIClass();
		
		// destructor
		~FishinoRA8875SPIClass();

		// initialize the interface
		void begin(uint8_t cs);
};


FishinoRA8875SPIClass &__GetFishinoRA8875SPI(void);

#define FishinoRA8875SPI __GetFishinoRA8875SPI()

#endif
