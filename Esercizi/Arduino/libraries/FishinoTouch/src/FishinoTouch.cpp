//////////////////////////////////////////////////////////////////////////////////////
//																					//
//								FishinoTouch.cpp									//
//					Base library for touch screen devices							//
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
#include "FishinoTouch.h"

// handler for interrupt, for devices supporting it
FishinoTouchHandler FishinoTouch::_handler = NULL;

// interrupt service routine, for devices supporting it
void __FishinoTouch_ISR(void)
{
	if(FishinoTouch::_handler)
		FishinoTouch::_handler();
}

// constructor
FishinoTouch::FishinoTouch()
{
	// panel size (some defaults...)
	_width = 240;
	_height = 320;
	
	// multitouch available
	_multiTouch = false;
	
	// type of touch
	_kind = RESISTIVE;
	
	// some default values for calibrations
	_ax = 1;
	_bx = 0;
	_ay = 1;
	_by = 0;
	
	_x = _y = 0;
	_p = 0;
	
	_isrPin = 0xff;
	_handler = NULL;
}

// destructor
FishinoTouch::~FishinoTouch()
{
	// disable interrupt pin on destroy
	disableInterrupts();
}

// enable/disable interrupts from touch
void FishinoTouch::enableInterrupts(void)
{
	if(_isrPin == 0xff)
		return;
	pinMode(_isrPin, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(_isrPin), __FishinoTouch_ISR, FALLING);
}

void FishinoTouch::disableInterrupts(void)
{
	if(_isrPin == 0xff)
		return;
	detachInterrupt(digitalPinToInterrupt(_isrPin));
}


// check if being touched
bool FishinoTouch::touching(void)
{
	disableInterrupts();
	bool res = update();
	enableInterrupts();
	return res;
}

// set calibration values
void FishinoTouch::calibrate(uint16_t w, uint16_t h, int16_t ax, int16_t bx, int16_t ay, int16_t by)
{
	_width = w;
	_height = h;
	_ax = ax;
	_bx = bx;
	_ay = ay;
	_by = by;
}

// set display rotation (counterclockwise)
void FishinoTouch::setRotation(uint8_t rot)
{
	// rot must be between 0 and 3
	_rotation = rot & 0x03;
}

// read raw coordinates (as returned by XPT2046 chip)
void FishinoTouch::readRaw(uint16_t &x, uint16_t &y, int16_t &p)
{
	x = _x;
	y = _y;
	p = _p;
}

void FishinoTouch::readRaw(uint16_t &x, uint16_t &y)
{
	int16_t p;
	readRaw(x, y, p);
}

// read calibrated coordinates (and pressure value)
void FishinoTouch::read(uint16_t &x, uint16_t &y, uint16_t &pressure)
{
	// convert raw coordinates to screen coordinates
	// on unrotated screen
	int32_t xx = (((int32_t)_x + _bx) << 3) / _ax;
	if(xx < 0)
		xx = 0;
	else if(xx >= _width)
		xx = _width - 1;
	int32_t yy = (((int32_t)_y + _by) << 3) / _ay;
	if(yy < 0)
		yy = 0;
	else if(yy >= _height)
		yy = _height - 1;
	
	// apply rotation
	switch(_rotation)
	{
		case 0:
		default:
			x = (uint16_t)xx;
			y = (uint16_t)yy;
			break;
		case 1:
			x = (uint16_t)yy;
			y = (uint16_t)(_width - xx - 1);
			break;
		case 2:
			x = (uint16_t)(_width - xx - 1);
			y = (uint16_t)(_height - yy - 1);
			break;
		case 3:
			x = (uint16_t)(_height - yy - 1);
			y = (uint16_t)xx;
			break;
	}
	
	pressure = _p;
}

void FishinoTouch::read(uint16_t &x, uint16_t &y)
{
	uint16_t p;
	return read(x, y, p);
}
