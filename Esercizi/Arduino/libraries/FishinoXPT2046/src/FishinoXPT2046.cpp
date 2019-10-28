//////////////////////////////////////////////////////////////////////////////////////
//																					//
//								FishinoXPT2046.cpp									//
//					Library for XPT2046 touch screen devices						//
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
#include <Arduino.h>

#include "FishinoXPT2046.h"

static SPISettings spiSettings;

// S A2 A1 A0 MODE SER/DFR PD1 PD0
#define XPT2046_S				0b10000000

#define XPT2046_SER_TEMP0		0b00000000
#define XPT2046_SER_Y			0b00010000
#define XPT2046_SER_VBAT		0b00100000
#define XPT2046_SER_Z1			0b00110000
#define XPT2046_SER_Z2			0b01000000
#define XPT2046_SER_X			0b01010000
#define XPT2046_SER_AUX			0b01100000
#define XPT2046_SER_X_TEMP		0b01110000

#define XPT2046_DFR_Y			0b00010000
#define XPT2046_DFR_Z1			0b00110000
#define XPT2046_DFR_Z2			0b01000000
#define XPT2046_DFR_X			0b01010000

#define XPT2046_SER				0b00000100
#define XPT2046_DFR				0b00000000

#define XPT2046_PWR_OFF			0b00000000
#define XPT2046_PWR_ADC			0b00000001
#define XPT2046_PWR_REF			0b00000010
#define XPT2046_PWR_ON			0b00000011

#define XPT2046_NUMVALS			12
#define XPT2046_DISCARD			4

int16_t _vals[XPT2046_NUMVALS];

#ifdef XPT2046_USE_INTERRUPT
#endif

// constructor
FishinoXPT2046Class::FishinoXPT2046Class()
{
	_connected = false;
	_cs = 0;
	
	// setup default calibration values
	calibrate();

	// setup SPI settings
	// (can't be done as a static value on 32 bit boards)
	spiSettings = SPISettings(2000000, MSBFIRST, SPI_MODE0);
	
	// connect to default pins
	// use connect() function to change them
#ifdef XPT2046_USE_INTERRUPT
	connect(TOUCH_CS, TOUCH_IRQ);
#else
	connect(TOUCH_CS);
#endif
}

// destructor
FishinoXPT2046Class::~FishinoXPT2046Class()
{
}

// read some points, discards the extremes
// and take the mean of internal ones
int16_t FishinoXPT2046Class::readVal(uint8_t what)
{
	// prepare to read first value
	SPI.transfer(XPT2046_S | what | XPT2046_PWR_ADC);
	delayMicroseconds(10);
	for(int i = 0; i < XPT2046_NUMVALS; i++)
	{
		_vals[i] = SPI.transfer16(XPT2046_S | what | XPT2046_PWR_ADC) >> 3;
		delayMicroseconds(10);
	}
	
	// re-power off
	SPI.transfer16(XPT2046_S);

	// sort read values
	for(int i = 0; i < XPT2046_NUMVALS - 1; i++)
		for(int j = i + 1; j < XPT2046_NUMVALS; j++)
			if(_vals[i] < _vals[j])
			{
				int16_t tmp = _vals[i];
				_vals[i] = _vals[j];
				_vals[j] = tmp;
			}
			
	// take the mean of inner values
	int16_t retVal = 0;
	for(int i = XPT2046_DISCARD ; i < XPT2046_NUMVALS - XPT2046_DISCARD; i++)
		retVal += _vals[i];
	return retVal / (XPT2046_NUMVALS - 2 * XPT2046_DISCARD);
}
		
// update values from touch
bool FishinoXPT2046Class::update(void)
{
	if(!_connected)
		return false;
	bool _touched = true;
	
	// start a transaction
	SPI.beginTransaction(spiSettings);
	digitalWrite(_cs, LOW);
	
	// read all needed values, taking means and discarding bads
	int32_t p1 = readVal(XPT2046_DFR_Z1);
	int32_t p2 = readVal(XPT2046_DFR_Z2);
	int32_t p = p1 + 4095 - p2;
	if(p < 0)
		p = 0;
	
	if(p < XPT2046_Z_THRESHOLD)
	{
		_x = -1;
		_y = -1;
		_p = 0;
		_touched = false;
	}
	else
	{
		_x = readVal(XPT2046_DFR_X);
		_y = readVal(XPT2046_DFR_Y);
		p = (((4096L * p1) / _x) * 4096L) / (p2 - p1);
		if(p < 0)
			p = 0;
		_p = (uint16_t)p;
	}

	// end the transaction
	digitalWrite(_cs, HIGH);
	SPI.endTransaction();
	
	if(_p < XPT2046_Z_THRESHOLD)
		_touched = false;

	return _touched;
}

// disconnect from hardware ports
void FishinoXPT2046Class::disconnect(void)
{
	if(!_connected)
		return;
	
	// disable interrupts on disconnection
	disableInterrupts();

	// set cs pin to input
	pinMode(_cs, INPUT);
}
		
// connect to hardware ports
void FishinoXPT2046Class::connect(uint8_t cs, uint8_t irq)
{
	// if already connected, disconnect first
	disconnect();
		
	// store connection pins
	_cs = cs;
	_isrPin = irq;
	
	pinMode(_cs, OUTPUT);
	
	// enable touch interrupts, if available
	enableInterrupts();
	
	// start SPI interface
	SPI.begin();
	
	// initialize the controller
	SPI.beginTransaction(spiSettings);
	digitalWrite(_cs, LOW);
	
	// power down
	SPI.transfer(XPT2046_S);
	SPI.transfer(0);

	digitalWrite(_cs, HIGH);
	SPI.endTransaction();

	_connected = true;
}

// set default calibration values
// (see comments on include file)
void FishinoXPT2046Class::calibrate()
{
	calibrate(XPT2046_WIDTH, XPT2046_HEIGHT, XPT2046_AX, XPT2046_BX, XPT2046_AY, XPT2046_BY);
}

FishinoXPT2046Class &__GetFishinoXPT2046(void)
{
	static FishinoXPT2046Class x;
	return x;
}
