//////////////////////////////////////////////////////////////////////////////////////
//																					//
//								FishinoFT5x06.cpp									//
//					Library for FT5x06 touch screen devices							//
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
#include <Wire.h>
#include <FishinoFlash.h>
#include <FishinoFT5x06.h>

#define MKUINT16(a, b) ( ((uint16_t)a) << 8 | (b) )

// constructor
// parameter is arduino's interrupt pin
FishinoFT5x06Class::FishinoFT5x06Class()
{
	_connected = false;
	
	// initialize wire
	Wire.begin();
	Wire.setClock(400000);
	
	// setup default calibration values
	calibrate();

	// connect to default pins
	// use connect() function to change them
	connect(TOUCH_IRQ);
}

bool FishinoFT5x06Class::update()
{
	disableInterrupts();
	
	// read first 6 registers.... enough to get first touch point and other data
	Wire.requestFrom(FT5206_I2C_ADDRESS, 7);

	// discard DEVIDE_MODE register
	Wire.read();

	// discard GEST_ID register
	Wire.read();

	// read needed registers

	// TD_STATUS
	uint8_t touchStatus = Wire.read();

	// TOUCH1_XH
	uint8_t xh = Wire.read();

	// TOUCH1_XL
	uint8_t xl = Wire.read();

	// TOUCH1_YH
	uint8_t yh = Wire.read();

	// TOUCH1_YL
	uint8_t yl = Wire.read();
	
/*
	uint8_t evt = xh >> 6;
	Serial << "stat:" << touchStatus << " evt:" << evt << " xh:" << xh << " xl:" << xl << " yh:" << yh << " yl:" << yl << "\n";
*/
	
	enableInterrupts();

	// do nothing if no touch points
	if (!(touchStatus & 0x0f))
		return false;

//	Serial << F("Touch!!!\n");

	// get type of event
	uint8_t event = xh >> 6;

/*
	// discard any non-put event
	if (event != 0)
		return false;
*/
	// as we don't use interrupts (usually) we accept both
	// put down and contact events (0x00 and 0x02
	if (event != 0 && event != 2)
		return false;

	// signal event and store its coordinates
	_x = MKUINT16(xh & 0x0f, xl);
	_y = MKUINT16(yh & 0x0f, yl);

	// we've got no pressure sensing on this device
	// so set it to maximum value
	_p = 0xffff;

	return true;
}

// disconnect from hardware ports
void FishinoFT5x06Class::disconnect(void)
{
	if(!_connected)
		return;
	
	// disable interrupts on disconnection
	disableInterrupts();
}
		
// connect to hardware ports
void FishinoFT5x06Class::connect(uint8_t irq)
{
	// if already connected, disconnect first
	disconnect();
		
	// store connection pins
	_isrPin = irq;
	
	Wire.beginTransmission(FT5206_I2C_ADDRESS);
	Wire.write(FT5206_DEVICE_MODE);
	Wire.write(0);
	Wire.endTransmission(FT5206_I2C_ADDRESS);

	// enable touch interrupts, if available
	enableInterrupts();
	
	_connected = true;
}

// set default calibration values
// (see comments on include file)
void FishinoFT5x06Class::calibrate()
{
	calibrate(FT5x06_WIDTH, FT5x06_HEIGHT, FT5x06_AX, FT5x06_BX, FT5x06_AY, FT5x06_BY);
}

FishinoFT5x06Class &__GetFishinoFT5x06(void)
{
	static FishinoFT5x06Class x;
	return x;
}
