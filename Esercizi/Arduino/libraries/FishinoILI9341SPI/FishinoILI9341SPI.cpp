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
//	Version 7.0.0 - 2017/01/01 - Rewrite for Fishino boards and TFT shield			//
//	Version 7.5.0 - 2018/02/02 - INITIAL VERSION									//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#include "FishinoILI9341SPI.h"
#ifdef __AVR
	#include <avr/pgmspace.h>
#endif

#include "pins_arduino.h"
#include "wiring_private.h"
#
#include <limits.h>
#include <SPI.h>

#include <FishinoFlash.h>


// If the SPI library has transaction support, these functions
// establish settings and protect from interference from other
// libraries.  Otherwise, they simply do nothing.
static inline void spi_begin(void) __attribute__((always_inline));
static inline void spi_begin(void)
{
#if defined(_FISHINO32_) || defined(_FISHINO32_96M_)
	SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
#elif defined(_FISHINO_PIRANHA_) || defined(_FISHINO_PIRANHA_96M_)
	SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));
#else
	SPI.beginTransaction(SPISettings(24000000, MSBFIRST, SPI_MODE0));
#endif
}

static inline void spi_end(void) __attribute__((always_inline));
static inline void spi_end(void)
{
	SPI.endTransaction();
}

// Constructor when using hardware SPI.  Faster, but must use SPI pins
// specific to each board type (e.g. 11,13 for Uno, 51,52 for Mega, etc.)
FishinoILI9341SPIClass::FishinoILI9341SPIClass() : FishinoGFX(ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT)
{
}

void FishinoILI9341SPIClass::spiwrite(uint8_t c)
{
#if defined (__AVR__)
	SPDR = c;
	while(!(SPSR & _BV(SPIF)))
		;
#else
	SPI.transfer(c);
#endif
}

void FishinoILI9341SPIClass::writecommand(uint8_t c)
{
	cmdMode();
	spiSelect();
	spiwrite(c);
	spiDeselect();
}

void FishinoILI9341SPIClass::writedata(uint8_t c)
{
	dataMode();
	spiSelect();
	spiwrite(c);
	spiDeselect();
}

// Rather than a bazillion writecommand() and writedata() calls, screen
// initialization commands and arguments are organized in these tables
// stored in PROGMEM.  The table may look bulky, but that's mostly the
// formatting -- storage-wise this is hundreds of bytes more compact
// than the equivalent code.  Companion function follows.
#define DELAY 0x80

// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void FishinoILI9341SPIClass::commandList(uint8_t *addr)
{

	uint8_t  numCommands, numArgs;
	uint16_t ms;

	numCommands = pgm_read_byte(addr++);   // Number of commands to follow
	while (numCommands--)                  // For each command...
	{
		writecommand(pgm_read_byte(addr++)); //   Read, issue command
		numArgs  = pgm_read_byte(addr++);    //   Number of args to follow
		ms       = numArgs & DELAY;          //   If hibit set, delay follows args
		numArgs &= ~DELAY;                   //   Mask out delay bit
		while (numArgs--)                    //   For each argument...
		{
			writedata(pgm_read_byte(addr++));  //     Read, issue argument
		}

		if (ms)
		{
			ms = pgm_read_byte(addr++); // Read post-command delay time (ms)
			if (ms == 255) ms = 500;    // If 255, delay for 500 ms
			delay(ms);
		}
	}
}

void FishinoILI9341SPIClass::begin(uint8_t cs, uint8_t dc)
{
	_cs   = cs;
	_dc   = dc;
	pinMode(_dc, OUTPUT);
	pinMode(_cs, OUTPUT);

#if defined (USE_FAST_PINIO)
	csport    = portOutputRegister(digitalPinToPort(_cs));
	cspinmask = digitalPinToBitMask(_cs);
	dcport    = portOutputRegister(digitalPinToPort(_dc));
	dcpinmask = digitalPinToBitMask(_dc);
#endif

	SPI.begin();

	spi_begin();
	
	writecommand(0x01);

	writecommand(0xEF);
	writedata(0x03);
	writedata(0x80);
	writedata(0x02);

	writecommand(0xCF);
	writedata(0x00);
	writedata(0XC1);
	writedata(0X30);

	writecommand(0xED);
	writedata(0x64);
	writedata(0x03);
	writedata(0X12);
	writedata(0X81);

	writecommand(0xE8);
	writedata(0x85);
	writedata(0x00);
	writedata(0x78);

	writecommand(0xCB);
	writedata(0x39);
	writedata(0x2C);
	writedata(0x00);
	writedata(0x34);
	writedata(0x02);

	writecommand(0xF7);
	writedata(0x20);

	writecommand(0xEA);
	writedata(0x00);
	writedata(0x00);

	writecommand(ILI9341_PWCTR1);    //Power control
	writedata(0x23);   //VRH[5:0]

	writecommand(ILI9341_PWCTR2);    //Power control
	writedata(0x10);   //SAP[2:0];BT[3:0]

	writecommand(ILI9341_VMCTR1);    //VCM control
	writedata(0x3e); //ﾶￔﾱ￈ﾶ￈ﾵ￷ﾽￚ
	writedata(0x28);

	writecommand(ILI9341_VMCTR2);    //VCM control2
	writedata(0x86);  //--

	writecommand(ILI9341_MADCTL);    // Memory Access Control
	writedata(0x48);

	writecommand(ILI9341_PIXFMT);
	writedata(0x55);

	writecommand(ILI9341_FRMCTR1);
	writedata(0x00);
	writedata(0x18);

	writecommand(ILI9341_DFUNCTR);    // Display Function Control
	writedata(0x08);
	writedata(0x82);
	writedata(0x27);

	writecommand(0xF2);    // 3Gamma Function Disable
	writedata(0x00);

	writecommand(ILI9341_GAMMASET);    //Gamma curve selected
	writedata(0x01);

	writecommand(ILI9341_GMCTRP1);    //Set Gamma
	writedata(0x0F);
	writedata(0x31);
	writedata(0x2B);
	writedata(0x0C);
	writedata(0x0E);
	writedata(0x08);
	writedata(0x4E);
	writedata(0xF1);
	writedata(0x37);
	writedata(0x07);
	writedata(0x10);
	writedata(0x03);
	writedata(0x0E);
	writedata(0x09);
	writedata(0x00);

	writecommand(ILI9341_GMCTRN1);    //Set Gamma
	writedata(0x00);
	writedata(0x0E);
	writedata(0x14);
	writedata(0x03);
	writedata(0x11);
	writedata(0x07);
	writedata(0x31);
	writedata(0xC1);
	writedata(0x48);
	writedata(0x08);
	writedata(0x0F);
	writedata(0x0C);
	writedata(0x31);
	writedata(0x36);
	writedata(0x0F);

	writecommand(ILI9341_SLPOUT);    //Exit Sleep
	spi_end();
	delay(120);
	spi_begin();
	writecommand(ILI9341_DISPON);    //Display on
	spi_end();
}

void FishinoILI9341SPIClass::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	writecommand(ILI9341_CASET); // Column addr set
	writedata(x0 >> 8);
	writedata(x0 & 0xFF);     // XSTART
	writedata(x1 >> 8);
	writedata(x1 & 0xFF);     // XEND

	writecommand(ILI9341_PASET); // Row addr set
	writedata(y0>>8);
	writedata(y0);     // YSTART
	writedata(y1>>8);
	writedata(y1);     // YEND

	writecommand(ILI9341_RAMWR); // write to RAM
}

void FishinoILI9341SPIClass::pushColor(uint16_t color)
{
	spi_begin();

#if defined(USE_FAST_PINIO)
	*dcport |=  dcpinmask;
	*csport &= ~cspinmask;
#else
	digitalWrite(_dc, HIGH);
	digitalWrite(_cs, LOW);
#endif

	spiwrite(color >> 8);
	spiwrite(color);

#if defined(USE_FAST_PINIO)
	*csport |= cspinmask;
#else
	digitalWrite(_cs, HIGH);
#endif

	spi_end();
}

void FishinoILI9341SPIClass::pushColors(uint16_t nColors, uint16_t const*colors)
{
	spi_begin();

#if defined(USE_FAST_PINIO)
	*dcport |=  dcpinmask;
	*csport &= ~cspinmask;
#else
	digitalWrite(_dc, HIGH);
	digitalWrite(_cs, LOW);
#endif

	for(uint16_t i = 0; i < nColors; i++)
	{
		spiwrite(*colors >> 8);
		spiwrite(*colors++);
	}


#if defined(USE_FAST_PINIO)
	*csport |= cspinmask;
#else
	digitalWrite(_cs, HIGH);
#endif

	spi_end();
}


void FishinoILI9341SPIClass::drawPixel0(int16_t x, int16_t y, uint16_t color)
{
	if ((x < 0) ||(x >= (int16_t)_width) || (y < 0) || (y >= (int16_t)_height))
		return;

	spi_begin();
	setAddrWindow(x,y,x+1,y+1);

#if defined(USE_FAST_PINIO)
	*dcport |=  dcpinmask;
	*csport &= ~cspinmask;
#else
	digitalWrite(_dc, HIGH);
	digitalWrite(_cs, LOW);
#endif

	spiwrite(color >> 8);
	spiwrite(color);

#if defined(USE_FAST_PINIO)
	*csport |= cspinmask;
#else
	digitalWrite(_cs, HIGH);
#endif

	spi_end();
}

void FishinoILI9341SPIClass::drawFastVLine0(int16_t x, int16_t y, uint16_t h, uint16_t color)
{
	// Rudimentary clipping
	if ((x >= (int16_t)_width) || (y >= (int16_t)_height))
		return;

	if ((y + h - 1) >= _height)
		h = _height - y;

	spi_begin();
	setAddrWindow(x, y, x, y+h-1);

	uint8_t hi = color >> 8, lo = color;

#if defined(USE_FAST_PINIO)
	*dcport |=  dcpinmask;
	*csport &= ~cspinmask;
#else
	digitalWrite(_dc, HIGH);
	digitalWrite(_cs, LOW);
#endif

	while (h--)
	{
		spiwrite(hi);
		spiwrite(lo);
	}

#if defined(USE_FAST_PINIO)
	*csport |= cspinmask;
#else
	digitalWrite(_cs, HIGH);
#endif

	spi_end();
}

void FishinoILI9341SPIClass::drawFastHLine0(int16_t x, int16_t y, uint16_t w, uint16_t color)
{
	// Rudimentary clipping
	if ((x >= (int16_t)_width) || (y >= (int16_t)_height))
		return;
	if ((x + w - 1) >= _width)
		w = _width - x;
	spi_begin();
	setAddrWindow(x, y, x+w-1, y);

	uint8_t hi = color >> 8, lo = color;
#if defined(USE_FAST_PINIO)
	*dcport |=  dcpinmask;
	*csport &= ~cspinmask;
#else
	digitalWrite(_dc, HIGH);
	digitalWrite(_cs, LOW);
#endif
	while (w--)
	{
		spiwrite(hi);
		spiwrite(lo);
	}
#if defined(USE_FAST_PINIO)
	*csport |= cspinmask;
#else
	digitalWrite(_cs, HIGH);
#endif
	spi_end();
}

void FishinoILI9341SPIClass::fillScreen(uint16_t color)
{
	fillRect(0, 0,  _width, _height, color);
}

// fill a rectangle
void FishinoILI9341SPIClass::fillRect0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color)
{
	// rudimentary clipping (drawChar w/big text requires this)
	if ((x >= (int16_t)_width) || (y >= (int16_t)_height))
		return;
	if ((x + w - 1) >= _width)
		w = _width  - x;
	if ((y + h - 1) >= _height)
		h = _height - y;

	spi_begin();
	setAddrWindow(x, y, x + w - 1, y + h - 1);

	uint8_t hi = color >> 8, lo = color;

#if defined(USE_FAST_PINIO)
	*dcport |=  dcpinmask;
	*csport &= ~cspinmask;
#else
	digitalWrite(_dc, HIGH);
	digitalWrite(_cs, LOW);
#endif

	for (y = h; y > 0; y--)
	{
		for (x = w; x > 0; x--)
		{
			spiwrite(hi);
			spiwrite(lo);
		}
	}
#if defined(USE_FAST_PINIO)
	*csport |= cspinmask;
#else
	digitalWrite(_cs, HIGH);
#endif

	spi_end();
}

// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t FishinoILI9341SPIClass::color565(uint8_t r, uint8_t g, uint8_t b)
{
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}


#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

void FishinoILI9341SPIClass::setRotation(uint8_t m)
{

	spi_begin();
	writecommand(ILI9341_MADCTL);
	_rotation = m % 4; // can't be higher than 3
	switch (_rotation)
	{
		case 0:
			writedata(MADCTL_MX | MADCTL_BGR);
			_width  = ILI9341_TFTWIDTH;
			_height = ILI9341_TFTHEIGHT;
			break;
		case 1:
			writedata(MADCTL_MV | MADCTL_BGR);
			_width  = ILI9341_TFTHEIGHT;
			_height = ILI9341_TFTWIDTH;
			break;
		case 2:
			writedata(MADCTL_MY | MADCTL_BGR);
			_width  = ILI9341_TFTWIDTH;
			_height = ILI9341_TFTHEIGHT;
			break;
		case 3:
			writedata(MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
			_width  = ILI9341_TFTHEIGHT;
			_height = ILI9341_TFTWIDTH;
			break;
	}
	spi_end();
}


void FishinoILI9341SPIClass::invertDisplay(boolean i)
{
	spi_begin();
	writecommand(i ? ILI9341_INVON : ILI9341_INVOFF);
	spi_end();
}

// transfer image blocks to display
// (data MUST be in progmem)
void FishinoILI9341SPIClass::bitBlt0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t const *buf)
{
	uint16_t dxl, dxr;
	uint16_t wb = w;
	
	// full out of screen image skip
	if(x + w < 0 || x >= width() || y + h < 0 || y >= height())
		return;

	// left/top crop
	if(x >= 0)
		dxl = 0;
	else
	{
		dxl = -x;
		x = 0;
		if(w < dxl)
			return;
		w -= dxl;
	}
	if(y < 0)
	{
		uint16_t dyl = -y * w;
		y = 0;
		if(h < dyl)
			return;
		h -= dyl;
		buf += dyl * wb;
	}
	
	// right/bottom crop
	if(x + (int16_t)w > width())
	{
		dxr = width() - x - w;
		w -= dxr;
	}
	else
		dxr = 0;
	if(y + (int16_t)h > height())
		h = height() - y;

	// start spi transaction
	spi_begin();
	
	// set display RAM window
	// and start writing data
	setAddrWindow(x, y, x + w - 1, y + h - 1);

	dataMode();
	spiSelect();
	for(uint16_t iy = 0; iy < h; iy++)
	{
		buf += dxl;
		for(uint16_t ix = 0; ix < w; ix++)
		{
#if defined (__AVR__)
			uint16_t w = pgm_read_word(buf++);
			spiwrite(w >> 8);
			spiwrite(w);
#else
			spiwrite(*buf >> 8);
			spiwrite(*buf++);
#endif
		}
		buf += dxr;
	}
	spiDeselect();
}

// transfer image blocks to display -- color 0x0000 is taken as trasparent
// (data MUST be in progmem)
void FishinoILI9341SPIClass::bitBltT0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t const *buf)
{
// Serial << "bitBltT(" << x << ", " << y << ", " << w << ", " << h << "\n";
	uint16_t dxl, dxr;
	uint16_t wb = w;
	
	// full out of screen image skip
	if(x + (int16_t)w < 0 || x >= width() || y + (int16_t)h < 0 || y >= height())
		return;

	// left/top crop
	if(x >= 0)
		dxl = 0;
	else
	{
		dxl = -x;
		x = 0;
		if(w < dxl)
			return;
		w -= dxl;
	}
	if(y < 0)
	{
		uint16_t dyl = -y * (int16_t)w;
		y = 0;
		if(h < dyl)
			return;
		h -= dyl;
		buf += dyl * wb;
	}
	
	// right/bottom crop
	if(x + (int16_t)w > width())
	{
		dxr = width() - x - (int16_t)w;
		w -= dxr;
	}
	else
		dxr = 0;
	if(y + (int16_t)h > height())
		h = height() - y;

	// start spi transaction
	spi_begin();
	
	// set display RAM window
	// and start writing data
	setAddrWindow(x, y, x + w - 1, y + h - 1);

	dataMode();
	spiSelect();
	bool writing = true;
	for(uint16_t iy = 0; iy < h; iy++)
	{
		buf += dxl;
		for(uint16_t ix = 0; ix < w; ix++)
		{
#if defined (__AVR__)
			uint16_t w = pgm_read_word(buf++);
#else
			uint16_t w = *buf++;
#endif
			if(!w)
			{
				if(writing)
				{
					writing = false;
					cmdMode();
					spiwrite(ILI9341_RAMRDCNT);
					dataMode();
					spiwrite(0);
				}
				spiwrite(0);
				spiwrite(0);
				spiwrite(0);
			}
			else
			{
				if(!writing)
				{
					writing = true;
					spiDeselect();
					cmdMode();
					spiSelect();
					spiwrite(ILI9341_RAMWRCNT);
//					spiDeselect();
					dataMode();
//					spiSelect();
				}
				spiwrite(w >> 8);
				spiwrite(w);
			}
		}
		buf += dxr;
	}
	spiDeselect();
// Serial << "DONE\n";
}

// transfer image blocks to display inverting it
// (data MUST be in progmem)
void FishinoILI9341SPIClass::bitBltInv0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t const *buf)
{
	uint16_t dxl, dxr;
	uint16_t wb = w;
	
	// full out of screen image skip
	if(x + w < 0 || x >= width() || y + h < 0 || y >= height())
		return;

	// left/top crop
	if(x >= 0)
		dxl = 0;
	else
	{
		dxl = -x;
		x = 0;
		if(w < dxl)
			return;
		w -= dxl;
	}
	if(y < 0)
	{
		uint16_t dyl = -y * w;
		y = 0;
		if(h < dyl)
			return;
		h -= dyl;
		buf += dyl * wb;
	}
	
	// right/bottom crop
	if(x + (int16_t)w > width())
	{
		dxr = width() - x - w;
		w -= dxr;
	}
	else
		dxr = 0;
	if(y + (int16_t)h > height())
		h = height() - y;

	// start spi transaction
	spi_begin();
	
	// set display RAM window
	// and start writing data
	setAddrWindow(x, y, x + w - 1, y + h - 1);

	dataMode();
	spiSelect();
	for(uint16_t iy = 0; iy < h; iy++)
	{
		buf += dxl;
		for(uint16_t ix = 0; ix < w; ix++)
		{
#if defined (__AVR__)
			uint16_t w = pgm_read_word(buf++) ^ 0xffff;
			spiwrite(w >> 8);
			spiwrite(w);
#else
			uint16_t w = *buf++ ^ 0xffff;
			spiwrite(w >> 8);
			spiwrite(w);
#endif
		}
		buf += dxr;
	}
	spiDeselect();
}

// transfer image blocks to display inverting it -- color 0x0000 is taken as trasparent
// (data MUST be in progmem)
void FishinoILI9341SPIClass::bitBltInvT0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t const *buf)
{
	uint16_t dxl, dxr;
	uint16_t wb = w;
	
	// full out of screen image skip
	if(x + w < 0 || x >= width() || y + h < 0 || y >= height())
		return;

	// left/top crop
	if(x >= 0)
		dxl = 0;
	else
	{
		dxl = -x;
		x = 0;
		if(w < dxl)
			return;
		w -= dxl;
	}
	if(y < 0)
	{
		uint16_t dyl = -y * w;
		y = 0;
		if(h < dyl)
			return;
		h -= dyl;
		buf += dyl * wb;
	}
	
	// right/bottom crop
	if(x + (int16_t)w > width())
	{
		dxr = width() - x - w;
		w -= dxr;
	}
	else
		dxr = 0;
	if(y + (int16_t)h > height())
		h = height() - y;

	// start spi transaction
	spi_begin();
	
	// set display RAM window
	// and start writing data
	setAddrWindow(x, y, x + w - 1, y + h - 1);

	dataMode();
	spiSelect();
	bool writing = true;
	for(uint16_t iy = 0; iy < h; iy++)
	{
		buf += dxl;
		for(uint16_t ix = 0; ix < w; ix++)
		{
#if defined (__AVR__)
			uint16_t w = pgm_read_word(buf++);
#else
			uint16_t w = *buf++;
#endif
			if(!w)
			{
				if(writing)
				{
					writing = false;
					cmdMode();
					spiwrite(ILI9341_RAMRDCNT);
					dataMode();
					spiwrite(0);
				}
				spiwrite(0);
				spiwrite(0);
				spiwrite(0);
			}
			else
			{
				if(!writing)
				{
					writing = true;
					spiDeselect();
					cmdMode();
					spiSelect();
					spiwrite(ILI9341_RAMWRCNT);
//					spiDeselect();
					dataMode();
//					spiSelect();
				}
				w ^= 0xffff;
				spiwrite(w >> 8);
				spiwrite(w);
			}
		}
		buf += dxr;
	}
	spiDeselect();
}

FishinoILI9341SPIClass &__GetFishinoILI9341SPI(void)
{
	static FishinoILI9341SPIClass ili;
	return ili;
}
