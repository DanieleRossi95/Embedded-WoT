//////////////////////////////////////////////////////////////////////////////////////
//																					//
//						FishinoRA8875SPI_Interface.cpp								//
//		Hardware driver for TFT displays with RA8875 display controller				//
//							SPI interface module									//
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

// If the SPI library has transaction support, these functions
// establish settings and protect from interference from other
static inline void spiBeginSlow(void) __attribute__((always_inline));
static inline void spiBeginSlow(void)
{
#if defined(_FISHINO32_) || defined(_FISHINO32_96M_)
	SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE3));
#elif defined(_FISHINO_PIRANHA_) || defined(_FISHINO_PIRANHA_96M_)
	SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE3));
#else
	SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE3));
#endif
}

static inline void spiBeginMedium(void) __attribute__((always_inline));
static inline void spiBeginMedium(void)
{
#if defined(_FISHINO32_) || defined(_FISHINO32_96M_)
	SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE3));
#elif defined(_FISHINO_PIRANHA_) || defined(_FISHINO_PIRANHA_96M_)
	SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE3));
#else
	SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE3));
#endif
}

static inline void spiBeginFast(void) __attribute__((always_inline));
static inline void spiBeginFast(void)
{
#if defined(_FISHINO32_) || defined(_FISHINO32_96M_)
	SPI.beginTransaction(SPISettings(12000000, MSBFIRST, SPI_MODE3));
#elif defined(_FISHINO_PIRANHA_) || defined(_FISHINO_PIRANHA_96M_)
	SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE3));
#else
	SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE3));
#endif
}

static inline void spi_end(void) __attribute__((always_inline));
static inline void spi_end(void)
{
	SPI.endTransaction();
}

// initialize the interface
void FishinoRA8875SPIClass::beginInterface(uint8_t cs)
{
	if(_cs != 0xff)
		pinMode(_cs, INPUT);
	_cs = cs;
	pinMode(_cs, OUTPUT);
	digitalWrite(_cs, HIGH);

	SPI.begin();
}

// write data to RA8875
void  FishinoRA8875SPIClass::writeData(uint8_t data)
{
	spiBeginSlow();
	digitalWrite(_cs, LOW);
	SPI.transfer(RA8875_DATAWRITE);
	SPI.transfer(data);
	digitalWrite(_cs, HIGH);
	spi_end();
}

// write 16 bit data to RA8875
void  FishinoRA8875SPIClass::writeData16(uint16_t data)
{
	spiBeginSlow();
	digitalWrite(_cs, LOW);
	SPI.transfer(RA8875_DATAWRITE);
	SPI.transfer(data >> 8);
	SPI.transfer(data);
	digitalWrite(_cs, HIGH);
	spi_end();
}

// write a command
void FishinoRA8875SPIClass::writeCommand(uint8_t d)
{
	spiBeginSlow();
	digitalWrite(_cs, LOW);
	SPI.transfer(RA8875_CMDWRITE);
	SPI.transfer(d);
	digitalWrite(_cs, HIGH);
	spi_end();
}

// read data or status
uint8_t  FishinoRA8875SPIClass::readData(void)
{
	spiBeginSlow();
	digitalWrite(_cs, LOW);
	SPI.transfer(RA8875_DATAREAD);
	uint8_t x = SPI.transfer(0x0);
	digitalWrite(_cs, HIGH);
	spi_end();
	return x;
}

// write a register
void  FishinoRA8875SPIClass::writeReg(uint8_t reg, uint8_t val)
{
	writeCommand(reg);
	writeData(val);
}

// read a register
uint8_t  FishinoRA8875SPIClass::readReg(uint8_t reg)
{
	writeCommand(reg);
	return readData();
}

// read the status
uint8_t  FishinoRA8875SPIClass::readStatus(void)
{
	spiBeginSlow();
	digitalWrite(_cs, LOW);
	SPI.transfer(RA8875_CMDREAD);
	uint8_t x = SPI.transfer(0x0);
	digitalWrite(_cs, HIGH);
	spi_end();
	return x;
}


//////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// MEMORY IMAGE WRITE /////////////////////////////////////
///////////////////// JUST SEND A BLOCK OF DATA WORDS TO LCD IMAGE RAM ///////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

// write a block of data, RAM version
// bufLen is the number of pixels (2 byte) data
void FishinoRA8875SPIClass::writeBuffer(uint16_t const *buf, size_t bufLen)
{
	spiBeginFast();
	digitalWrite(_cs, LOW);
	SPI.transfer(RA8875_DATAWRITE);
	for(size_t i = 0; i < bufLen; i++)
	{
		uint16_t data = *buf++;
		SPI.transfer(data >> 8);
		SPI.transfer(data);
	}
	digitalWrite(_cs, HIGH);
	spi_end();
}

// write a block of data, FLASH version
// buf MUST be a 16 bit image buffer in flash area
// bufLen is the number of pixels (2 byte) data
void FishinoRA8875SPIClass::writeBuffer(const __FlashStringHelper *buf, size_t bufLen)
{
	uint16_t const *pBuf = (uint16_t const *)buf;
	
	spiBeginFast();
	digitalWrite(_cs, LOW);
	SPI.transfer(RA8875_DATAWRITE);
	for(size_t i = 0; i < bufLen; i++)
	{
#ifdef __AVR__
		uint16_t data = pgm_read_word(pBuf++);
#else
		uint16_t data = *pBuf++;
#endif
		SPI.transfer(data >> 8);
		SPI.transfer(data);
	}
	digitalWrite(_cs, HIGH);
	spi_end();
}


//////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// NON-BTE MEMORY TrANSPARENT WRITE //////////////////////////////
//////////////////// NEEDED MOSTLY BECAUSE BTE DOESN'T SUPPORT ROTATION //////////////////////
//////////////////////////// QUITE SLOW, SO USE ONLY WHEN NEEDED /////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

// read-transparent-write a block of data, RAM version
void FishinoRA8875SPIClass::writeBufferT(uint16_t const *buf, size_t bufLen, uint16_t transpColor)
{
	for(size_t i = 0; i < bufLen; i++)
	{
		// read background pixel
		spiBeginMedium();
		digitalWrite(_cs, LOW);
		SPI.transfer(RA8875_DATAREAD);
		
		// flash read needs to discard first byte
		SPI.transfer(0x00);
		
		uint16_t bck = SPI.transfer(0x00);
		bck |= (uint16_t)SPI.transfer(0x00) << 8;
		
		// flash read needs an extra byte, otherwise it will
		// mess up write part (sigh)
		SPI.transfer(0x00);
		
		digitalWrite(_cs, HIGH);
		spi_end();

		// get pixel to write
		uint16_t pix = *buf++;

		// if it's transparent, replace with background color
		if(pix == transpColor)
			pix = bck;


		// write the pixel
		spiBeginFast();
		digitalWrite(_cs, LOW);
		SPI.transfer(RA8875_DATAWRITE);
		SPI.transfer(pix >> 8);
		SPI.transfer(pix);
		digitalWrite(_cs, HIGH);
		spi_end();
	}
}

// read-transparent-write a block of data, Flash version
// buffer MUST be a 16 bit image chunk
void FishinoRA8875SPIClass::writeBufferT(const __FlashStringHelper *buf, size_t bufLen, uint16_t transpColor)
{
	const uint16_t *pBuf = (const uint16_t *)buf;
	for(size_t i = 0; i < bufLen; i++)
	{
		// read background pixel
		spiBeginMedium();
		digitalWrite(_cs, LOW);
		SPI.transfer(RA8875_DATAREAD);
		
		// flash read needs to discard first byte
		SPI.transfer(0x00);
		
		uint16_t bck = SPI.transfer(0x00);
		bck |= (uint16_t)SPI.transfer(0x00) << 8;
		
		// flash read needs an extra byte, otherwise it will
		// mess up write part (sigh)
		SPI.transfer(0x00);
		
		digitalWrite(_cs, HIGH);
		spi_end();

		// get pixel to write
#ifdef __AVR__
		uint16_t pix = pgm_read_word(pBuf++);
#else
		uint16_t pix = *pBuf++;
#endif

		// if it's transparent, replace with background color
		if(pix == transpColor)
			pix = bck;

		// write the pixel
		spiBeginFast();
		digitalWrite(_cs, LOW);
		SPI.transfer(RA8875_DATAWRITE);
		SPI.transfer(pix >> 8);
		SPI.transfer(pix);
		digitalWrite(_cs, HIGH);
		spi_end();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

// From Adafruit_RA8875, need to be fixed!!!!!!!!!
boolean FishinoRA8875SPIClass::waitPoll(uint8_t regname, uint8_t waitflag)
{
	while (1)
	{
		uint8_t temp = readReg(regname);
		if (!(temp & waitflag))
			return true;
	}
	// MEMEFIX: yeah i know, unreached! - add timeout?
	return false; 
}

// Just a wait routine until job it's done
// Parameters:
// res:
//		0x80(for most operations)
//		0x40(BTE wait)
//		0x01(DMA wait)
void FishinoRA8875SPIClass::waitBusy(uint8_t res)
{
	uint8_t w;
	do
	{
		// dma
		if (res == 0x01)
			writeCommand(RA8875_DMACR);
		w = readStatus();
	}
	while ((w & res) == res);
}
