//////////////////////////////////////////////////////////////////////////////////////
//																					//
//								FishinoXPT2046.h									//
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
#ifndef _FishinoXPT2046_h_
#define _FishinoXPT2046_h_

#include <SPI.h>
#include <FishinoTouch.h>

// de-comment this one if you want to use interrupt feature
// #define XPT2046_USE_INTERRUPT

// uncomment and adapt these if you have a non-fishino board
// with different connections
// #define TOUCH_CS	6
// #define TOUCH_IRQ	3

////////////////////////////////////////////////////////////////////////////////
// touch minimum raw value for pressed event to happen
// do not lower too much or you can get spurious presses
#define XPT2046_Z_THRESHOLD		50
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// calibration values -- given for a 2.8' 240x320 TFT display here
// can be calculated using FishinoTouchCalibrate example sketch
// for other screen sizes or to have a better precision on yours
// formula from raw value to scren value is
// SCREEN_VAL = (RAW_VAL + B) * 8 / A
// you can replace the values here or use FishinoXPT2046.calibrate() function
// inside your sketch to adapt to your display
const uint16_t XPT2046_WIDTH	=   240;
const uint16_t XPT2046_HEIGHT	=   320;
const int16_t XPT2046_AX		=  -117;
const int16_t XPT2046_BX		= -3786;
const int16_t XPT2046_AY		=    88;
const int16_t XPT2046_BY		=  -233;
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// connection pins for Fishino and Arduino on Fishino TFT shield
// if this library has to be used with another shield/breakout
// adapt connection pins using connect() display and touch functions
#if !defined(TOUCH_CS)
	#if defined(_FISHINO_PIRANHA_) || defined(_FISHINO_PIRANHA_96M_)
		#define TOUCH_CS 	6
	#elif defined(_FISHINO32_) || defined(_FISHINO32_96M_)
		#define TOUCH_CS	6
	#elif defined(_FISHINO_UNO_)
		#define TOUCH_CS	6
	#elif defined(_FISHINO_MEGA_)
		#define TOUCH_CS	6
	#elif defined(_FISHINO_GUPPY_)
		#define TOUCH_CS	6
	#else
		#error "You must define TOUCH_CS"
	#endif
#endif

#if !defined(TOUCH_IRQ)
	#ifdef XPT2046_USE_INTERRUPT
		#if defined(_FISHINO_PIRANHA_) || defined(_FISHINO_PIRANHA_96M_)
			#define TOUCH_IRQ	5
		#elif defined(_FISHINO32_) || defined(_FISHINO32_96M_)
			#define TOUCH_IRQ	3
		#elif defined(_FISHINO_UNO_)
			#define TOUCH_IRQ	3
		#elif defined(_FISHINO_MEGA_)
			#define TOUCH_IRQ	3
		#elif defined(_FISHINO_GUPPY_)
			#define TOUCH_IRQ	3
		#else
			#error "You must define TOUCH_IRQ"
		#endif
	#else
		#define TOUCH_IRQ	0xff
	#endif
#endif

class FishinoXPT2046Class : public FishinoTouch
{
	friend FishinoXPT2046Class &__GetFishinoXPT2046(void);
	friend void FishinoXPT2046_ISR(void);
	
	private:
	
		// connected flag
		bool _connected;
		
		// connection pins
		uint8_t _cs;
		
		// read some points, discards the extremes
		// and take the mean of internal ones
		int16_t readVal(uint8_t what);
		
		// constructor
		FishinoXPT2046Class();
		
		// update values from touch
		virtual bool update(void);
		
	protected:
	
	public:
	
		// destructor
		~FishinoXPT2046Class();
		
		// connect to hardware ports
		void connect(uint8_t cs = TOUCH_CS, uint8_t irq = TOUCH_IRQ);
		
		// disconnect from hardware ports
		void disconnect(void);
		
		// set default calibration values (see comments on top)
		virtual void calibrate();
		using FishinoTouch::calibrate;
};

extern FishinoXPT2046Class &__GetFishinoXPT2046(void);

#define FishinoXPT2046 	__GetFishinoXPT2046()

#endif
