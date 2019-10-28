//////////////////////////////////////////////////////////////////////////////////////
//																					//
//							FishinoGFX_Gradient.h									//
//			Color gradient module for FishinoGFX graphic library					//
//					Created by Massimo Del Fedele, 2018								//
//																					//
//  Copyright (c) 217, 2018 Massimo Del Fedele.  All rights reserved		.		//
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
//	Version 7.0.0 - 2017/01/01 - Rewrite for Fishino boards and TFT shield			//
//	Version 7.5.0 - 2018/02/02 - INITIAL VERSION									//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#ifndef __FISHINOGFX_FISHINOSHADER_H
#define __FISHINOGFX_FISHINOSHADER_H

#include <Arduino.h>

// bresenham color interpolation class
class FishinoGradient
{
	private:
		
		int16_t _min;
		int16_t _p;
		uint16_t _r, _g, _b;
		int16_t _dr2, _dg2, _db2;
		int32_t _r12, _g12, _b12;
		int16_t _dp, _dp2;
		
	protected:

	public:

		// constructor
		FishinoGradient(int16_t min, int16_t max, uint16_t color1, uint16_t color2);

		// destructor
		~FishinoGradient();
		
		// get next color
		uint16_t nextColor(void);
		
		// get color at a given point
		uint16_t getColor(int16_t pos);
		
		// restart from beginning
		void restart(void) { restart(_min); }
		
		// restart from a given point
		void restart(int16_t pos);
};

#endif
