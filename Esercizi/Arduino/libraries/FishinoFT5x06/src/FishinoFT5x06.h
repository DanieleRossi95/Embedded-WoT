//////////////////////////////////////////////////////////////////////////////////////
//																					//
//								FishinoFT5x06.h										//
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
#ifndef __FISHINO_FT5X06_H
#define __FISHINO_FT5X06_H

#include <FishinoTouch.h>

// de-comment this one if you want to use interrupt feature
// #define FT5X06_USE_INTERRUPT

// uncomment and adapt these if you have a non-fishino board
// with different connections
// #define TOUCH_IRQ	3

////////////////////////////////////////////////////////////////////////////////
// calibration values -- given for a 7' 800x480 TFT display here
// This device doesn't need to be calibrated, just needs the panel size
// in order to calculate the coordinates when rotated
// so coefficients AX..BY are just for 1:1 correspondence
// for other screen sizes or to have a better precision on yours
// formula from raw value to scren value is
// SCREEN_VAL = (RAW_VAL + B) * 8 / A
// you can replace the values here or use FishinoFT5x06.calibrate() function
// inside your sketch to adapt to your display
const uint16_t FT5x06_WIDTH		=  800;
const uint16_t FT5x06_HEIGHT	=  480;
const int16_t FT5x06_AX			=    8;
const int16_t FT5x06_BX			=    0;
const int16_t FT5x06_AY			=    8;
const int16_t FT5x06_BY			=    0;
////////////////////////////////////////////////////////////////////////////////

#if !defined(TOUCH_IRQ)
	#ifdef XPT2046_USE_INTERRUPT
		#error "You must define TOUCH_IRQ to use interrupt features"
	#else
		#define TOUCH_IRQ	0xff
	#endif
#endif

// FT5206 definitions
#define FT5206_I2C_ADDRESS 0x38

// there are more registers, but this is enought to get all 5 touch coordinates.
#define FT5206_NUMBER_OF_REGISTERS 31     

#define FT5206_NUMBER_OF_TOTAL_REGISTERS 0xFE

#define FT5206_DEVICE_MODE 0x00

#define FT5206_GEST_ID 0x01
#define FT5206_GEST_ID_MOVE_UP     0x10
#define FT5206_GEST_ID_MOVE_LEFT   0x14
#define FT5206_GEST_ID_MOVE_DOWN   0x18
#define FT5206_GEST_ID_MOVE_RIGHT  0x1c
#define FT5206_GEST_ID_ZOOM_IN     0x48
#define FT5206_GEST_ID_ZOOM_OUT    0x49
#define FT5206_GEST_ID_NO_GESTURE  0x00

#define FT5206_TD_STATUS 0x02

#define FT5206_TOUCH1_XH 0x03
#define FT5206_TOUCH1_XL 0x04
#define FT5206_TOUCH1_YH 0x05
#define FT5206_TOUCH1_YL 0x06

#define FT5206_TOUCH2_XH 0x09
#define FT5206_TOUCH2_XL 0x0a
#define FT5206_TOUCH2_YH 0x0b
#define FT5206_TOUCH2_YL 0x0c

#define FT5206_TOUCH3_XH 0x0f
#define FT5206_TOUCH3_XL 0x10
#define FT5206_TOUCH3_YH 0x11
#define FT5206_TOUCH3_YL 0x12

#define FT5206_TOUCH4_XH 0x15
#define FT5206_TOUCH4_XL 0x16
#define FT5206_TOUCH4_YH 0x17
#define FT5206_TOUCH4_YL 0x18

#define FT5206_TOUCH5_XH 0x1b
#define FT5206_TOUCH5_XL 0x1c
#define FT5206_TOUCH5_YH 0x1d
#define FT5206_TOUCH5_YL 0x1e

class FishinoFT5x06Class : public FishinoTouch
{
	private:
		bool _connected;
		
		// update values from touch
		virtual bool update(void);
		
	public:
		// constructor
		// parameter is arduino's interrupt pin
		FishinoFT5x06Class(void);
		
		// connect to hardware ports
		void connect(uint8_t irq = TOUCH_IRQ);
		
		// disconnect from hardware ports
		void disconnect(void);
		
		// get last touch position
		void getTouchPosition(uint16_t &x, uint16_t &y);

		// set default calibration values (see comments on top)
		virtual void calibrate();
		using FishinoTouch::calibrate;
};

extern FishinoFT5x06Class &__GetFishinoFT5x06(void);

#define FishinoFT5x06 	__GetFishinoFT5x06()

#endif
