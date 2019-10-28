#ifndef __FISHINOAS3935_FISHINOAS3935_H
#define __FISHINOAS3935_FISHINOAS3935_H

#include <Arduino.h>

#include <SPI.h>
#include <Wire.h>

class FishinoAS3935
{
		typedef uint8_t (FishinoAS3935::*READFUNC)(uint8_t);
		typedef void (FishinoAS3935::*WRITEFUNC)(uint8_t, uint8_t);

	private:

		// write/read function pointers
		// setup by init depending on the interface
		// we want to use (Wire/I2C or SPI)
		READFUNC _rawRegisterReadFunc;
		WRITEFUNC _rawRegisterWriteFunc;

		// read/write functions for I2C mode
		uint8_t _i2cRead(uint8_t addr);
		void _i2cWrite(uint8_t addr, uint8_t data);

		// read/write functions for SPI mode
		uint8_t _spiRead(uint8_t addr);
		void _spiWrite(uint8_t addr, uint8_t data);
		
		// raw read/write function
		inline uint8_t rawRegisterRead(uint8_t addr) { return (this->*_rawRegisterReadFunc)(addr); }
		inline void rawRegisterWrite(uint8_t addr, uint8_t data) { (this->*_rawRegisterWriteFunc)(addr, data); }

		// cs pin for SPI mode
		// (must be held high in I2C mode)
		uint8_t _cs;

		// device addres for I2C mode
		uint8_t _devAddr;

		// irq pin
		uint8_t _irq;

		// spi settings
		SPISettings _spiSettings;

		// read an AS3935 register
		uint8_t registerRead(uint8_t reg, uint8_t mask);

		// write an AS3935 register
		void registerWrite(uint8_t reg, uint8_t mask, uint8_t data);

	protected:

	public:

		// constructor for I2C mode
		FishinoAS3935(uint8_t addr, uint8_t cs, uint8_t irq);

		// constructor for SPI mode
		FishinoAS3935(uint8_t cs, uint8_t irq);

		// destructor
		~FishinoAS3935();

		void reset();
		bool calibrate();

		void powerDown();
		void powerUp();

		int getInterruptSource();
		void setInterruptSource(uint8_t src);

		void disableDisturbers();
		void enableDisturbers();

		int getMinimumLightnings();
		int setMinimumLightnings(int minlightning);

		int getLightningDistanceKm();

		void setIndoors();
		void setOutdoors();

		int getNoiseFloor();
		int setNoiseFloor(int noisefloor);

		int getSpikeRejection();
		int setSpikeRejection(int srej);

		int getWatchdogThreshold();
		int setWatchdogThreshold(int wdth);

		void clearStats();
};

#endif
