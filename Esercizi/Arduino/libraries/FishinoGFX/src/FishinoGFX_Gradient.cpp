//////////////////////////////////////////////////////////////////////////////////////
//																					//
//								FishinoGFX_Gradient.cpp								//
//				Color shading module for FishinoGFX graphic library					//
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
#include "FishinoGFX_Gradient.h"

static void splitColor(uint16_t c, uint8_t &r, uint8_t &g, uint8_t &b)
{
	r = c >> 11;
	g = (c >> 5) & 0x3f;
	b = c & 0x1f;
}

static inline uint16_t combineColor(uint8_t r, uint8_t g, uint8_t b)
{
	return ( ((uint16_t)r) << 11) | ( ((uint16_t)g) << 5) | b;
}

// constructor
FishinoGradient::FishinoGradient(int16_t min, int16_t max, uint16_t color1, uint16_t color2)
{
	_min = min;
	
	uint8_t r1, g1, b1;
	splitColor(color1, r1, g1, b1);

	uint8_t r2, g2, b2;
	splitColor(color2, r2, g2, b2);
	
	_dr2 = (r2 - r1) * 2;
	_dg2 = (g2 - g1) * 2;
	_db2 = (b2 - b1) * 2;
	
	_dp = max - min;
	_dp2 = 2 * _dp;
	
	_r12 = (2 * r1 + 1) * _dp;
	_g12 = (2 * g1 + 1) * _dp;
	_b12 = (2 * b1 + 1) * _dp;
	
	_r = r1;
	_g = g1;
	_b = b1;
	_p = 0;

}

// destructor
FishinoGradient::~FishinoGradient()
{
}

// get next color
uint16_t FishinoGradient::nextColor(void)
{
	uint16_t col = combineColor(_r, _g, _b);
	
	_r = (_dr2 * _p + _r12 ) / _dp2;
	_g = (_dg2 * _p + _g12) / _dp2;
	_b = (_db2 * _p + _b12) / _dp2;
	_p++;
	
	return col;
}

// get color at a given point
uint16_t FishinoGradient::getColor(int16_t pos)
{
	pos -= _min;
	uint16_t r = (_dr2 * pos + _r12 ) / _dp2;
	uint16_t g = (_dg2 * pos + _g12) / _dp2;
	uint16_t b = (_db2 * pos + _b12) / _dp2;
	return combineColor(r, g, b);
}

// restart from a given point
void FishinoGradient::restart(int16_t pos)
{
	_p = pos - _min;
	_r = (_dr2 * _p + _r12 ) / _dp2;
	_g = (_dg2 * _p + _g12) / _dp2;
	_b = (_db2 * _p + _b12) / _dp2;
}
