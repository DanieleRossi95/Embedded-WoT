//////////////////////////////////////////////////////////////////////////////////////
//																					//
//								FishinoTouch.h										//
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
#ifndef __FISHINOTOUCH_H
#define __FISHINOTOUCH_H

#include <Arduino.h>

typedef void(*FishinoTouchHandler)(void);

class FishinoTouch
{
	friend void __FishinoTouch_ISR(void);
	
	public:
		
		enum KIND {
			RESISTIVE = 0,
			CAPACITIVE = 1,
			OTHER = 2
		};
		
	private:
		
	protected:

		// panel size
		uint16_t _width, _height;
		
		// display rotation (counterclockwse increasing)
		uint8_t _rotation;
		
		// calibration values
		int16_t _ax, _bx;
		int16_t _ay, _by;
		
		// multitouch available
		bool _multiTouch;
		
		// type of touch
		KIND _kind;
		
		// last raw cursor positions
		// (uncalibrated values from chip)
		uint16_t _x, _y;
		
		// last raw pressure
		uint16_t _p;
		
		// interrupt I/O, if used (0xff if not using interrupts)
		uint8_t _isrPin;
		
		// handler for interrupt, for devices supporting it
		static FishinoTouchHandler _handler;
		
		// constructor
		FishinoTouch();
		
		// enable/disable interrupts from touch
		void enableInterrupts(void);
		void disableInterrupts(void);

		// update values from touch
		// must be defined in derived classes
		// must update the touch raw coordinates
		// and return true if touched, false otherwise
		virtual bool update(void) = 0;
		
	public:

		// destructor
		virtual ~FishinoTouch();

		// set calibration values
		void calibrate(uint16_t w, uint16_t h, int16_t ax, int16_t bx, int16_t ay, int16_t by);
		
		// set default calibration values depending on used hardware
		virtual void calibrate() = 0;
		
		// set display rotation (counterclockwise)
		void setRotation(uint8_t rot);
		
		// check if being touched
		bool touching(void);
		
		// read raw values (used only for calibration)
		void readRaw(uint16_t &x, uint16_t &y, int16_t &pressure);
		void readRaw(uint16_t &x, uint16_t &y);

		// read calibrated coordinates (and pressure value)
		void read(uint16_t &x, uint16_t &y, uint16_t &pressure);
		void read(uint16_t &x, uint16_t &y);
		
		// handler for touch, if using interrupts
		// beware, it runs into an interrupt handler, so BE FAST
		// and don't use delay() or long running loops inside!
		void setHandler(FishinoTouchHandler h) { _handler = h; }
		void clearHandler(void) { _handler = NULL; }
};

#endif
