//////////////////////////////////////////////////////////////////////////////////////
//																					//
//								Octopus.cpp											//
//			Library to handle Fishino Octopus I/O expander boards					//
//					Created by Massimo Del Fedele, 2016								//
//																					//
//  Copyright (c) 2015, 2016 and 2017 Massimo Del Fedele.  All rights reserved.		//
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
//	VERSION 5.0.0 -            - Converted to new library format					//
//	VERSION 7.5.0 - 30/04/2018 - FIXED PULLUPS HANDLING								//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#include "Octopus.h"

#include <Wire.h>

// Register defines from data sheet - we set IOCON.BANK to 0
// as it is easier to manage the registers sequentially.
#define MCP23017_IODIR			0x00
#define MCP23017_IPOL 			0x02
#define MCP23017_INTEN 			0x04
#define MCP23017_DEFVAL			0x06
#define MCP23017_INTCON			0x08
#define MCP23017_GPPU 			0x0C
#define MCP23017_INTF 			0x0E
#define MCP23017_GPIO 			0x12

#define MCP23017_IOCON			0x0a
#define MCP23017_IOCON_BANKED_1	0x05
#define MCP23017_IOCON_BANKED_2	0x15

// flags for IOCON register
#define MCP23017_IOCON_BANK		0x80
#define MCP23017_IOCON_MIRROR	0x40
#define MCP23017_IOCON_SEQOP	0x20
#define MCP23017_IOCON_DISSLW	0x10
#define MCP23017_IOCON_HAEN		0x08
#define MCP23017_IOCON_ODR		0x04
#define MCP23017_IOCON_INTPOL	0x02

#define MCP23017_I2C_BASE_ADDRESS	0x20

#define PCA9685_I2C_BASE_ADDRESS	0x40

// PCA9685 registers
#define PCA9685_MODE1			0x00
#define PCA9685_MODE2			0x01
#define PCA9685_SUBADR1			0x02
#define PCA9685_SUBADR2			0x03
#define PCA9685_SUBADR3			0x04
#define PCA9685_ALLCALLADDR		0x05
#define PCA9685_LED_ON_L(i)		(4 * (i) + 6)
#define PCA9685_LED_ON_H(i)		(4 * (i) + 7)
#define PCA9685_LED_OFF_L(i)	(4 * (i) + 8)
#define PCA9685_LED_OFF_H(i)	(4 * (i) + 9)
#define PCA9685_ALL_LED_ON_L	0xfa
#define PCA9685_ALL_LED_ON_H	0xfb
#define PCA9685_ALL_LED_OFF_L	0xfc
#define PCA9685_ALL_LED_OFF_H	0xfd
#define PCA9685_PRE_SCALE		0xfe
#define PCA9685_TESTMODE		0xff

// PCA9685 MODE1 FLAGS
#define PCA9685_MODE1_RESTART	0x80
#define PCA9685_MODE1_EXTCLK	0x40
#define PCA9685_MODE1_AI		0x20
#define PCA9685_MODE1_SLEEP		0x10
#define PCA9685_MODE1_SUB1		0x08
#define PCA9685_MODE1_SUB2		0x04
#define PCA9685_MODE1_SUB3		0x02
#define PCA9685_MODE1_ALLCALL	0x01

// PCA9685 MODE2 FLAGS
#define PCA9685_MODE2_INVRT		0x08
#define PCA9685_MODE2_OCH		0x04
#define PCA9685_MODE2_OUTDRV	0x02
#define PCA9685_MODE2_OUTNE1	0x01
#define PCA9685_MODE2_OUTNE0	0x00
	

// low-level MCP register access
void OctopusClass::writeMCPRegister8(uint8_t mcpAddr, uint8_t regAddr, uint8_t val)
{
	Wire.beginTransmission(MCP23017_I2C_BASE_ADDRESS + mcpAddr);
	Wire.write(regAddr);
	Wire.write(val);
	Wire.endTransmission();
}

void OctopusClass::writeMCPRegister16(uint8_t mcpAddr, uint8_t regAddr, uint16_t val)
{
	Wire.beginTransmission(MCP23017_I2C_BASE_ADDRESS + mcpAddr);
	Wire.write(regAddr);
	Wire.write(val);
	Wire.write(val >> 8);
	Wire.endTransmission();
}

uint16_t OctopusClass::readMCPRegister16(uint8_t mcpAddr, uint8_t regAddr)
{
	uint16_t res = 0x00;
	Wire.beginTransmission(MCP23017_I2C_BASE_ADDRESS + mcpAddr);
	Wire.write(regAddr);
	Wire.endTransmission();
	Wire.requestFrom(MCP23017_I2C_BASE_ADDRESS + mcpAddr, 2);
    
    if(Wire.available())
        res = Wire.read();
    if(Wire.available())
        res |= ((uint16_t)Wire.read()) << 8;
    
    return res;
}

// low-level PCA register access
void OctopusClass::writePCARegister8(uint8_t pcaAddr, uint8_t regAddr, uint8_t val)
{
	Wire.beginTransmission(PCA9685_I2C_BASE_ADDRESS + pcaAddr);
	Wire.write(regAddr);
	Wire.write(val);
	Wire.endTransmission();
}

void OctopusClass::writePCARegister16(uint8_t pcaAddr, uint8_t regAddr, uint16_t val)
{
	Wire.beginTransmission(PCA9685_I2C_BASE_ADDRESS + pcaAddr);
	Wire.write(regAddr);
	Wire.write(val);
	Wire.write(val >> 8);
	Wire.endTransmission();
}

uint8_t OctopusClass::readPCARegister8(uint8_t pcaAddr, uint8_t regAddr)
{
	uint16_t res = 0x00;

	Wire.beginTransmission(PCA9685_I2C_BASE_ADDRESS + pcaAddr);
	Wire.write(regAddr);
	Wire.endTransmission();
	Wire.requestFrom(PCA9685_I2C_BASE_ADDRESS + pcaAddr, 1);
    
 	if(Wire.available())
		res = Wire.read();
	return res;
}

uint16_t OctopusClass::readPCARegister16(uint8_t pcaAddr, uint8_t regAddr)
{
	uint16_t res = 0x00;
	Wire.beginTransmission(PCA9685_I2C_BASE_ADDRESS + pcaAddr);
	Wire.write(regAddr);
	Wire.endTransmission();
	Wire.requestFrom(PCA9685_I2C_BASE_ADDRESS + pcaAddr, 2);
    
    if(Wire.available())
        res = Wire.read();
    if(Wire.available())
         res |= ((uint16_t)Wire.read()) << 8;
    
    return res;
}

// check if the PWM part is at given address
// return true on success, false otherwise
bool OctopusClass::detectPWM(uint8_t addr)
{
	// detection is done by writing and re-reading the 3 SUBADR registers
	// which have their 7 MSB valids and the LSB at 0
	writePCARegister8(addr, PCA9685_SUBADR1, 0xab);
	if(readPCARegister8(addr, PCA9685_SUBADR1) != 0xaa)
		return false;
	writePCARegister8(addr, PCA9685_SUBADR2, 0x55);
	if(readPCARegister8(addr, PCA9685_SUBADR2) != 0x54)
		return false;
	writePCARegister8(addr, PCA9685_SUBADR3, 0xc3);
	if(readPCARegister8(addr, PCA9685_SUBADR3) != 0xc2)
		return false;
	
	// reset registers to their defaults
	writePCARegister8(addr, PCA9685_SUBADR1, 0xe2);
	writePCARegister8(addr, PCA9685_SUBADR2, 0xe4);
	writePCARegister8(addr, PCA9685_SUBADR3, 0xe8);

	return true;
}

// check if the digital part is at given address
// return true on success, false otherwise
bool OctopusClass::detectIO(uint8_t addr)
{
	// first, initialize IOCON register
	// we do at first a bank enable and then a bank disable
	// as we don't know where IOCON register is
	writeMCPRegister8(addr, MCP23017_IOCON_BANKED_1, 0);
	writeMCPRegister8(addr, MCP23017_IOCON_BANKED_2, 0);
	writeMCPRegister8(addr, MCP23017_IOCON, 0);
	writeMCPRegister8(addr, MCP23017_IOCON + 1, 0);
	
	// we shall be in non-banked mode now
	// so we test the IPOL register which should be harmless
	writeMCPRegister16(addr, MCP23017_IPOL, 0xaa55);
	if(readMCPRegister16(addr, MCP23017_IPOL) != 0xaa55)
		return false;
	writeMCPRegister16(addr, MCP23017_IPOL, 0x55aa);
	if(readMCPRegister16(addr, MCP23017_IPOL) != 0x55aa)
		return false;
	writeMCPRegister16(addr, MCP23017_IPOL, 0x0);

	// all ok, IPOL register is there
	return true;
}
	
// initialize and count the connected boards
uint8_t OctopusClass::initialize(void)
{
	// temporary allocate space for 8 expanders
	// (maximum allowed number)
	uint8_t tmpAddr[8];
	
	// count responding boards
	uint8_t *addrPnt = tmpAddr;
	for(uint8_t i = 0; i < 8; i++)
	{
		if(detectPWM(i) && detectIO(i))
			*addrPnt++ = i;
	}
	nBoards = addrPnt - tmpAddr;
	if(!nBoards)
		return 0;
	
	// now allocate data for all found boards
	addresses = (uint8_t *)malloc(nBoards * sizeof(uint8_t));
	mcpCaches = (MCPCache *)malloc(nBoards * sizeof(MCPCache));
	
	// fill data and initialize boards
	for(uint8_t i = 0; i < nBoards; i++)
	{
		// store address for current board
		uint8_t addr = tmpAddr[i];
		addresses[i] = addr;
		
		// initialize MCP part
		// (IOCON bank bit is initialized in detect routine)
		MCPCache &mcpCache = mcpCaches[i];
		
		// direction as input by default
		mcpCache.IODIR	= 0x0000;
		writeMCPRegister16(addr, MCP23017_IODIR, 0xffff);
		
		// input polarity same as pin value
		writeMCPRegister16(addr, MCP23017_IPOL, 0x0000);
		
		// interrupts disabled on startup
		writeMCPRegister16(addr, MCP23017_INTEN, 0x0000);
		
		// init default value for comparing+interrupt to 0
		writeMCPRegister16(addr, MCP23017_DEFVAL, 0x0000);
		
		// bits are compared with their previous value
		writeMCPRegister16(addr, MCP23017_INTCON, 0x0000);
		
		// configure IOCON register
		// all default values : non-banked access, no mirror int, etc
		writeMCPRegister8(addr, MCP23017_IOCON, 0x00);
		
		// disable pullups
		mcpCache.GPPU	= 0x0000;
		writeMCPRegister16(addr, MCP23017_GPPU, 0x0000);

		// outputs to 0 value by default
		mcpCache.GPIO	= 0x0000;
		writeMCPRegister16(addr, MCP23017_GPIO, 0x0000);
		
		// initialize cached interrupt states
		mcpCache.IOINT	= 0x0000;
		
		// initialize PWM part
		uint8_t mod1;

		// reset the board to known MODE1 values
		writePCARegister8(addr, PCA9685_MODE1, 0x00);
	
		// as PCA board is not tied with hardware reset
		// we issue a board restart upon detect
		// doing that we activate the autoincrement too
		mod1 = readPCARegister8(addr, PCA9685_MODE1) | PCA9685_MODE1_AI;
		if(mod1 & PCA9685_MODE1_RESTART)
		{
			mod1 &= ~PCA9685_MODE1_SLEEP;
			writePCARegister8(addr, PCA9685_MODE1, mod1);
			delay(10);
	
			mod1 = readPCARegister8(addr, PCA9685_MODE1) | PCA9685_MODE1_RESTART;
			writePCARegister8(addr, PCA9685_MODE1, mod1);
		}
		
		writePCARegister8(addr, PCA9685_MODE1, mod1 | 0xa1);
	
		// reset all output to 0
		Wire.beginTransmission(PCA9685_I2C_BASE_ADDRESS + addr);
		Wire.write(PCA9685_LED_ON_L(0));
		for(uint8_t out = 0; out < 16; out++)
		{
			Wire.write(0x00);
			Wire.write(0x00);
			Wire.write(0x00);
			Wire.write(0xff);
		}
		Wire.endTransmission();

		// set outputs to totem-pole mode
		writePCARegister8(addr, PCA9685_MODE2, PCA9685_MODE2_OUTDRV | PCA9685_MODE2_OCH);

	}
	
	return nBoards;
}

// sets the 2 PWM values for an output
// values can be 0..4095
// special values of 0xffff are reserved to turn fully ON or OFF
// the output (OFF has precedence over ON)
void OctopusClass::setPWMValues(uint8_t port, uint16_t on, uint16_t off)
{
	// get actual board number
	uint8_t board = port / 16;
	
	// do nothing if port is out of range
	if(board > nBoards)
		return;
	
	uint8_t addr = addresses[board];

	// get the actual port on board
	port %= 16;
	
	if(off == 0xffff)
	{
		// full OFF
		on = 0;
		off = 4096;
	}
	else if(on == 0xffff)
	{
		// full ON
		on = 4096;
		off = 0;
	}
	else
	{
		// limit values to 0..4095
		on &= 4095;
		off &= 4095;
	}

	Wire.beginTransmission(PCA9685_I2C_BASE_ADDRESS + addr);
	Wire.write(PCA9685_LED_ON_L(port));
	Wire.write(on);
	Wire.write(on >> 8);
	Wire.write(off);
	Wire.write(off >> 8);
	Wire.endTransmission();
}

OctopusClass::OctopusClass()
{
	// initialize the board on construction
	initialize();
}

OctopusClass::~OctopusClass()
{
	free(addresses);
	free(mcpCaches);
}

// set pwm frequency for a single connected board
// valid values 24 Hz...1526 Hz
void OctopusClass::setPWMFreq(uint8_t board, uint16_t freq)
{
	// if wrong board, just leave
	if(board >= nBoards)
		return;
	
	// limit frequency value
	uint8_t addr = addresses[board];
	if(freq < 24)
		freq = 24;
	else if(freq > 1526)
		freq = 1526;
	
	// calculate prescaler value
	double dPres = 25e6 / 4096 / freq - 0.5;
	if(dPres < 3)
		dPres = 3;
	else if(dPres > 255)
		dPres = 255;
	uint8_t iPres = (uint8_t)dPres;
	
	// enter sleep mode
	uint8_t mod1 = readPCARegister8(addr, PCA9685_MODE1) | PCA9685_MODE1_SLEEP;
	writePCARegister8(addr, PCA9685_MODE1, mod1);
	
	// wait sleep to settle
	delay(1);
	
	// setup prescaler value
	writePCARegister8(addr, PCA9685_PRE_SCALE, iPres);
	
	// restart PWM
	mod1 &= ~PCA9685_MODE1_SLEEP;
	writePCARegister8(addr, PCA9685_MODE1, mod1);
	delay(1);	
	mod1 |= PCA9685_MODE1_RESTART;
	writePCARegister8(addr, PCA9685_MODE1, mod1);
}

// set pwm frequency for ALL connected boards
void OctopusClass::setPWMFreq(uint16_t freq)
{
	for(uint8_t iBoard = 0; iBoard < nBoards; iBoard++)
		setPWMFreq(iBoard, freq);
}
		
// pwm output
void OctopusClass::analogWrite(uint8_t port, uint16_t val, bool invert)
{
	// val should be 4096 max (for full ON leds)
	if(val > 4096)
		val = 4096;
	if(val == 0)
	{
		if(invert)
			setPWMValues(port, 0xffff, 0x0000);
		else
			setPWMValues(port, 0x0000, 0xffff);
	}
	else if(val == 4096)
	{
		if(invert)
			setPWMValues(port, 0x0000, 0xffff);
		else
			setPWMValues(port, 0xffff, 0x0000);
	}
	else if(invert)
		setPWMValues(port, 0, 4096 - val);
	else
		setPWMValues(port, 0, val);
}

// digital I/O
void OctopusClass::pinMode(uint8_t port, uint8_t mode)
{

	// get actual board number
	uint8_t board = port / 16;
	
	// do nothing if port is out of range
	if(board >= nBoards)
		return;

	// get the actual port on board
	port %= 16;
	
	uint16_t bit = (uint16_t)1 << port;

	// get board data
	MCPCache &cache = mcpCaches[board];
	uint8_t addr = addresses[board];
	
	switch(mode)
	{
		case OUTPUT:
		
			// disable pullup if active
			if(cache.GPPU & bit)
			{
				// store new pullup state
				cache.GPPU &= ~bit;
				
				// setup register
				writeMCPRegister16(addr, MCP23017_GPPU, cache.GPPU);
			}
		
			// if direction is already output, do nothing
			if(cache.IODIR & bit)
				return;

			// store new direction into cache
			cache.IODIR |= bit;
		
			// first write currently stored GPIO word to board
			// in order to have the output set to correct value when changing mode
			writeMCPRegister16(addr, MCP23017_GPIO, cache.GPIO);
			
			// then switch mode
			writeMCPRegister16(addr, MCP23017_IODIR, ~cache.IODIR);
			
			break;
		
		case INPUT:
		
			// if pullup already inactive, don't change it
			if(cache.GPPU & (1 << port))
			{
				// store new pullup state
				cache.GPPU &= ~(1 << port);
				
				// setup register
				writeMCPRegister16(addr, MCP23017_GPPU, cache.GPPU);
			}
		
			// if direction is already input, do nothing
			if(!(cache.IODIR & (1 << port)))
				return;

			// store new direction into cache
			cache.IODIR &= ~(1 << port);
		
			// then switch mode
			writeMCPRegister16(addr, MCP23017_IODIR, ~cache.IODIR);
			
			break;
		
		case INPUT_PULLUP:
		
			// if pullup already active, don't change it
			if(!(cache.GPPU & (1 << port)))
			{
				// store new pullup state
				cache.GPPU |= (1 << port);
				
				// setup register
				writeMCPRegister16(addr, MCP23017_GPPU, cache.GPPU);
			}
		
			// if direction is already input, do not switch mode
			if(!(cache.IODIR & (1 << port)))
				break;

			// store new direction into cache
			cache.IODIR &= ~(1 << port);
		
			// then switch mode
			writeMCPRegister16(addr, MCP23017_IODIR, ~cache.IODIR);

			break;
		
		default:
		
			// do nothing on param error
			break;
	}
	
}

bool OctopusClass::digitalRead(uint8_t port)
{
	// get actual board number
	uint8_t board = port / 16;
	
	// do nothing if port is out of range
	if(board > nBoards)
		return false;
	
	// get the actual port on board
	port %= 16;
	
	return readMCPRegister16(addresses[board], MCP23017_GPIO) & (1 << port);
}

void OctopusClass::digitalWrite(uint8_t port, bool value)
{
	// get actual board number
	uint8_t board = port / 16;
	
	// do nothing if port is out of range
	if(board >= nBoards)
		return;
	
	// get the actual port on board
	port %= 16;
	
	MCPCache &cache = mcpCaches[board];
	if(value)
		cache.GPIO |= (uint16_t)1 << port;
	else
		cache.GPIO &= ~((uint16_t)1 << port);
	writeMCPRegister16(addresses[board], MCP23017_GPIO, cache.GPIO);
}

// read/write all digital pins at once
uint16_t OctopusClass::digitalReadAll(uint8_t board)
{
	// if board out of range, just return 0
	if(board >= nBoards)
		return 0;
	
	return readMCPRegister16(addresses[board], MCP23017_GPIO);
}

void OctopusClass::digitalWriteAll(uint8_t board, uint16_t val)
{
	// if board out of range, just return
	if(board >= nBoards)
		return;
	
	mcpCaches[board].GPIO = val;
	writeMCPRegister16(addresses[board], MCP23017_GPIO, val);
}

OctopusClass &__octopus()
{
	static OctopusClass octo;
	return octo;
}
