#include "FishinoAS3935.h"

#include <limits.h>

#define AS3935_AFE_GB		0x00, 0x3E
#define AS3935_PWD			0x00, 0x01
#define AS3935_NF_LEV		0x01, 0x70
#define AS3935_WDTH			0x01, 0x0F
#define AS3935_CL_STAT		0x02, 0x40
#define AS3935_MIN_NUM_LIGH	0x02, 0x30
#define AS3935_SREJ			0x02, 0x0F
#define AS3935_LCO_FDIV		0x03, 0xC0
#define AS3935_MASK_DIST	0x03, 0x20
#define AS3935_INT			0x03, 0x0F
#define AS3935_DISTANCE		0x07, 0x3F
#define AS3935_DISP_LCO		0x08, 0x80
#define AS3935_DISP_SRCO	0x08, 0x40
#define AS3935_DISP_TRCO	0x08, 0x20
#define AS3935_TUN_CAP		0x08, 0x0F

// other constants
#define AS3935_AFE_INDOOR	0x12
#define AS3935_AFE_OUTDOOR	0x0E

//#define DEBUG_LEVEL_INFO
#include <FishinoDebug.h>

// we use C++11 here
constexpr uint8_t _ffsz(uint8_t mask, uint8_t i = 0)
{
	return ~mask & 1 ? _ffsz(mask >> 1, i + 1) : i;
}


// read/write functions for I2C mode
uint8_t FishinoAS3935::_i2cRead(uint8_t addr)
{
	DEBUG_INFO("I2CRead(%02x)\n", addr);
	// send register number
	Wire.beginTransmission(_devAddr);
	Wire.write(addr);
	Wire.endTransmission(false);

	// request register data
	Wire.requestFrom(_devAddr, (uint8_t)1);
	
	// read data
	uint8_t res = 0;
	if(Wire.available())
		res = Wire.read();
	else
		DEBUG_INFO("I2CRead FAILED\n");

	DEBUG_INFO("I2CRead(%02x) = %02x DONE\n", addr, res);
	return res;
}

void FishinoAS3935::_i2cWrite(uint8_t addr, uint8_t data)
{
	DEBUG_INFO("I2CWrite(%02x, %02x)\n", addr, data);
	Wire.beginTransmission(_devAddr);
	Wire.write(addr);
	Wire.write(data);
	Wire.endTransmission();
	DEBUG_INFO("I2CWrite(%02x, %02x) DONE\n", addr, data);
}

// read/write functions for SPI mode
uint8_t FishinoAS3935::_spiRead(uint8_t addr)
{
	DEBUG_INFO("SpiRead(%02x)\n", addr);
	digitalWrite(_cs, LOW);
	SPI.beginTransaction(_spiSettings);
	SPI.transfer((addr & 0x3f) | 0x40);
	uint8_t res = SPI.transfer(0x00);
	SPI.endTransaction();
	digitalWrite(_cs, HIGH);
	DEBUG_INFO("SpiRead(%02x) = %02x DONE\n", addr, res);
	return res;
}

void FishinoAS3935::_spiWrite(uint8_t addr, uint8_t data)
{
	DEBUG_INFO("SpiWrite(%02x, %02x)\n", addr, data);
	digitalWrite(_cs, LOW);
	SPI.beginTransaction(_spiSettings);
	SPI.transfer(addr & 0x3f);
	SPI.transfer(data);
	SPI.endTransaction();
	digitalWrite(_cs, HIGH);
	DEBUG_INFO("SpiWrite(%02x, %02x) DONE\n", addr, data);
}

// read an AS3935 register
uint8_t FishinoAS3935::registerRead(uint8_t reg, uint8_t mask)
{
	DEBUG_INFO("Read(%02x, %02x)", reg, mask);
	uint8_t data = rawRegisterRead(reg);
	
	// apply mask, if provided
	if(mask)
		data >>= _ffsz(mask);

	DEBUG_INFO("Read(%02x, %02x) = %02x DONE\n", reg, mask, data);
	return data;
}

// write an AS3935 register
void FishinoAS3935::registerWrite(uint8_t reg, uint8_t mask, uint8_t data)
{
	DEBUG_INFO("Write(%02x, %02x, %02x)\n", reg, mask, data);
	uint8_t regval = rawRegisterRead(reg);
	regval &= ~(mask);
	if (mask)
		regval |= (data << _ffsz(mask));
	else
		regval |= data;
	rawRegisterWrite(reg, regval);
	DEBUG_INFO("Write DONE\n");
}

// constructor for I2C mode
FishinoAS3935::FishinoAS3935(uint8_t addr, uint8_t cs, uint8_t irq)
{
	_devAddr = addr;
	_cs  = cs;
	_irq = irq;

	// initalize the chip select and interrupt pin
	pinMode(_cs, OUTPUT);
	pinMode(_irq, INPUT);

	// set cs pin to high for I2C mode
	digitalWrite(_cs, HIGH);

	// setup function pointers for I2C mode
	_rawRegisterReadFunc = &FishinoAS3935::_i2cRead;
	_rawRegisterWriteFunc = &FishinoAS3935::_i2cWrite;
}

// constructor for SPI mode
FishinoAS3935::FishinoAS3935(uint8_t cs, uint8_t irq)
{
	_devAddr = 0;
	_cs  = cs;
	_irq = irq;
	
	// max speed is 2 MHz, SPI MODE1 mode
	_spiSettings = SPISettings(2000000, MSBFIRST, SPI_MODE1);

	// initalize the chip select and interrupt pin
	pinMode(_cs, OUTPUT);
	pinMode(_irq, INPUT);

	// set cs pin to high to disable module at startup
	digitalWrite(_cs, HIGH);

	// setup function pointers for SPI mode
	_rawRegisterReadFunc = &FishinoAS3935::_spiRead;
	_rawRegisterWriteFunc = &FishinoAS3935::_spiWrite;
}

// destructor
FishinoAS3935::~FishinoAS3935()
{
}

void FishinoAS3935::reset()
{
	rawRegisterWrite(0x3c, 0x96);
	delay(2);
}

bool FishinoAS3935::calibrate()
{
	int target = 3125;
	int currentcount = 0;
	int bestdiff = INT_MAX;
	int currdiff = 0;
	uint8_t bestTune = 0;
	uint8_t currTune = 0;
	
	unsigned long setUpTime;
	int currIrq, prevIrq;
	
	// set lco_fdiv divider to 0, which translates to 16
	// so we are looking for 31250Hz on irq pin
	// and since we are counting for 100ms that translates to number 3125
	// each capacitor changes second least significant digit
	// using this timing so this is probably the best way to go
	registerWrite(AS3935_LCO_FDIV, 0);
	registerWrite(AS3935_DISP_LCO, 1);
	
	// tuning is not linear, can't do any shortcuts here
	// going over all built-in cap values and finding the best
	for (currTune = 0; currTune <= 0x0F; currTune++) 
	{
		registerWrite(AS3935_TUN_CAP, currTune);

		// let it settle
		delay(2);
		currentcount = 0;
		prevIrq = digitalRead(_irq);
		setUpTime = millis() + 100;
		while(millis() < setUpTime)
		{
			currIrq = digitalRead(_irq);
			if (currIrq > prevIrq)
				currentcount++;	
			prevIrq = currIrq;
		}
		currdiff = target - currentcount;
		
		// don't look at me, abs() misbehaves
		if(currdiff < 0)
			currdiff = -currdiff;
		if(bestdiff > currdiff)
		{
			bestdiff = currdiff;
			bestTune = currTune;
		}
	}
	registerWrite(AS3935_TUN_CAP, bestTune);
	delay(2);
	registerWrite(AS3935_DISP_LCO, 0);

	// and now do RCO calibration
	powerUp();
	
	// if error is over 109, we are outside allowed tuning range of +/-3.5%
	return bestdiff <= 109;
}

void FishinoAS3935::powerDown()
{
	registerWrite(AS3935_PWD, 1);
}

void FishinoAS3935::powerUp()
{
	registerWrite(AS3935_PWD, 0);
	rawRegisterWrite(0x3c, 0x96);
	delay(3);
}

int FishinoAS3935::getInterruptSource()
{
	return registerRead(AS3935_INT);
}

void FishinoAS3935::setInterruptSource(uint8_t src)
{
	registerWrite(AS3935_INT, src);
}

void FishinoAS3935::disableDisturbers()
{
	registerWrite(AS3935_MASK_DIST, 1);
}

void FishinoAS3935::enableDisturbers()
{
	registerWrite(AS3935_MASK_DIST, 0);
}

int FishinoAS3935::getMinimumLightnings()
{
	return registerRead(AS3935_MIN_NUM_LIGH);
}

int FishinoAS3935::setMinimumLightnings(int minlightning)
{
	registerWrite(AS3935_MIN_NUM_LIGH, minlightning);
	return getMinimumLightnings();
}

int FishinoAS3935::getLightningDistanceKm()
{
	return registerRead(AS3935_DISTANCE);
}

void FishinoAS3935::setIndoors()
{
	registerWrite(AS3935_AFE_GB, AS3935_AFE_INDOOR);
}

void FishinoAS3935::setOutdoors()
{
	registerWrite(AS3935_AFE_GB, AS3935_AFE_OUTDOOR);
}

int FishinoAS3935::getNoiseFloor()
{
	return registerRead(AS3935_NF_LEV);
}

int FishinoAS3935::setNoiseFloor(int noisefloor)
{
	registerWrite(AS3935_NF_LEV, noisefloor);
	return getNoiseFloor();
}

int FishinoAS3935::getSpikeRejection()
{
	return registerRead(AS3935_SREJ);
}

int FishinoAS3935::setSpikeRejection(int srej)
{
	registerWrite(AS3935_SREJ, srej);
	return getSpikeRejection();
}

int FishinoAS3935::getWatchdogThreshold()
{
	return registerRead(AS3935_WDTH);
}

int FishinoAS3935::setWatchdogThreshold(int wdth)
{
	registerWrite(AS3935_WDTH, wdth);
	return getWatchdogThreshold();
}

void FishinoAS3935::clearStats()
{
	registerWrite(AS3935_CL_STAT, 1);
	registerWrite(AS3935_CL_STAT, 0);
	registerWrite(AS3935_CL_STAT, 1);
}

