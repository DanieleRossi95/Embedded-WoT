#ifndef __FISHINORA8875SPI_INTERFACE_H
#define __FISHINORA8875SPI_INTERFACE_H

#include <Arduino.h>

class FishinoRA8875SPI_Interface
{
	private:
		
		uint8_t _cs;

	protected:

	public:

		// constructor
		FishinoRA8875SPI_Interface();

		// destructor
		~FishinoRA8875SPI_Interface();
		
		// initialize the interface
		void begin(uint8_t cs);

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
};

#endif
