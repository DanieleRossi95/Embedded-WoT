//////////////////////////////////////////////////////////////////////////////////////
//																					//
//							FishinoILI9341SPI.cpp									//
//		Hardware driver for TFT displays with ILI9341 display controller			//
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
#ifndef _FISHINO_ILI9341SPIH_
#define _FISHINO_ILI9341SPIH_

#include "Arduino.h"
#include "Print.h"

#include <FishinoGFX.h>

#ifdef __AVR
	#include <avr/pgmspace.h>
#endif

#if defined (__AVR__)
	#define USE_FAST_PINIO
#endif

/////////////////////////////////////////////////////////////////////////
// connection pins for Fishino and Arduino on Fishino TFT shield
// if this library has to be used with another shield/breakout
// adapt connection pins using begin() display function
#if defined(_FISHINO_PIRANHA_) || defined(_FISHINO_PIRANHA_96M_)
	#define TFT_DC		7
	#define TFT_CS		2
#elif defined(_FISHINO32_) || defined(_FISHINO32_96M_)
	#define TFT_DC		5
	#define TFT_CS		2
#elif defined(_FISHINO_UNO_)
	#define TFT_DC		5
	#define TFT_CS		2
#elif defined(_FISHINO_MEGA_)
	#define TFT_DC		5
	#define TFT_CS		2
#elif defined(_FISHINO_GUPPY_)
	#define TFT_DC		5
	#define TFT_CS		2
#else
	#define TFT_DC		5
	#define TFT_CS		2
#endif

#define ILI9341_TFTWIDTH  240
#define ILI9341_TFTHEIGHT 320

#define ILI9341_NOP			0x00
#define ILI9341_SWRESET		0x01
#define ILI9341_RDDID		0x04
#define ILI9341_RDDST		0x09

#define ILI9341_SLPIN		0x10
#define ILI9341_SLPOUT		0x11
#define ILI9341_PTLON		0x12
#define ILI9341_NORON		0x13

#define ILI9341_RDMODE		0x0A
#define ILI9341_RDMADCTL	0x0B
#define ILI9341_RDPIXFMT	0x0C
#define ILI9341_RDIMGFMT	0x0D
#define ILI9341_RDSELFDIAG	0x0F

#define ILI9341_INVOFF		0x20
#define ILI9341_INVON		0x21
#define ILI9341_GAMMASET	0x26
#define ILI9341_DISPOFF		0x28
#define ILI9341_DISPON		0x29

#define ILI9341_CASET		0x2A
#define ILI9341_PASET		0x2B
#define ILI9341_RAMWR		0x2C
#define ILI9341_RAMRD		0x2E

#define ILI9341_PTLAR		0x30
#define ILI9341_MADCTL		0x36
#define ILI9341_PIXFMT		0x3A

#define ILI9341_RAMWRCNT	0x3C
#define ILI9341_RAMRDCNT	0x3E

#define ILI9341_FRMCTR1		0xB1
#define ILI9341_FRMCTR2		0xB2
#define ILI9341_FRMCTR3		0xB3
#define ILI9341_INVCTR		0xB4
#define ILI9341_DFUNCTR		0xB6

#define ILI9341_PWCTR1		0xC0
#define ILI9341_PWCTR2		0xC1
#define ILI9341_PWCTR3		0xC2
#define ILI9341_PWCTR4		0xC3
#define ILI9341_PWCTR5		0xC4
#define ILI9341_VMCTR1		0xC5
#define ILI9341_VMCTR2		0xC7

#define ILI9341_RDID1		0xDA
#define ILI9341_RDID2		0xDB
#define ILI9341_RDID3		0xDC
#define ILI9341_RDID4		0xDD

#define ILI9341_GMCTRP1		0xE0
#define ILI9341_GMCTRN1		0xE1
/*
#define ILI9341_PWCTR6		0xFC

*/

class FishinoILI9341SPIClass : public FishinoGFX
{
	friend FishinoILI9341SPIClass &__GetFishinoILI9341SPI(void);
	private:
		uint8_t tabcolor;

#if defined (__AVR__)
		volatile uint8_t *dcport, *csport;
		int8_t  _cs, _dc;
		uint8_t  cspinmask, dcpinmask;
#else
		int8_t  _cs, _dc;
#endif

		inline void cmdMode(void) {
#if defined (USE_FAST_PINIO)
			*dcport &= ~dcpinmask;
#else
			digitalWrite(_dc, LOW);
#endif
		}
		
		inline void dataMode(void)
		{
#if defined (USE_FAST_PINIO)
			*dcport |=  dcpinmask;
#else
			digitalWrite(_dc, HIGH);
#endif
		}
		
		inline void spiSelect(void) {
#if defined (USE_FAST_PINIO)
			*csport &= ~cspinmask;
#else
			digitalWrite(_cs, LOW);
#endif
		}
		
		inline void spiDeselect(void) {
#if defined (USE_FAST_PINIO)
			*csport |= cspinmask;
#else
			digitalWrite(_cs, HIGH);
#endif
		}

		void spiwrite(uint8_t);
		void writecommand(uint8_t c);
		void writedata(uint8_t d);
		void commandList(uint8_t *addr);

		//////////////////////////////////////////////////////////
		//					BITBLT ROUTINES						//
		//////////////////////////////////////////////////////////

		// transfer image blocks to display
		// (data MUST be in progmem)
		void bitBlt0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t const *buf);
		
		// transfer image blocks to display -- color 0x0000 is taken as trasparent
		// (data MUST be in progmem)
		void bitBltT0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t const *buf);

		// transfer image blocks to display inverting it
		// (data MUST be in progmem)
		void bitBltInv0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t const *buf);

		// transfer image blocks to display inverting it -- color 0x0000 is taken as trasparent
		// (data MUST be in progmem)
		void bitBltInvT0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t const *buf);

		//////////////////////////////////////////////////////////
		//					DRAWING ROUTINES					//
		//////////////////////////////////////////////////////////

		void drawPixel0(int16_t x, int16_t y, uint16_t color);

		void drawFastVLine0(int16_t x, int16_t y, uint16_t h, uint16_t color);
		void drawFastHLine0(int16_t x, int16_t y, uint16_t w, uint16_t color);

		void fillRect0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color);

	public:
		FishinoILI9341SPIClass();

		void begin(uint8_t cs = TFT_CS, uint8_t dc = TFT_DC);
		void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

		void pushColor(uint16_t color);
		void pushColors(uint16_t nColors, uint16_t const*colors);

		void fillScreen(uint16_t color);
		void setRotation(uint8_t r);
		void invertDisplay(boolean i);

		uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
		
};

FishinoILI9341SPIClass &__GetFishinoILI9341SPI(void);

#define FishinoILI9341SPI __GetFishinoILI9341SPI()

#endif
