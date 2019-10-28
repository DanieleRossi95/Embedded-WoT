//////////////////////////////////////////////////////////////////////////////////////
//																					//
//								FishinoGFX.cpp										//
//					Base library for graphic displays								//
//					Created by Massimo Del Fedele, 2017								//
//	Loosely based on Adafruit's GFX library - copyright notice follows				//
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


//////////////////////////////////////////////////////////////////////////////////////
//	This is the core graphics library for all our displays, providing a common		//
//	set of graphics primitives (points, lines, circles, etc.).  It needs to be		//
//	paired with a hardware-specific library for each display device we carry		//
//	(to handle the lower-level functions).											//
//																					//
//	Adafruit invests time and resources providing this open source code, please		//
//	support Adafruit & open-source hardware by purchasing products from Adafruit!	//
//																					//
//	Copyright (c) 2013 Adafruit Industries.  All rights reserved.					//
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
//////////////////////////////////////////////////////////////////////////////////////
#include "FishinoGFX.h"

#include <FishinoFlash.h>

FishinoGFX::FishinoGFX(int16_t w, int16_t h) : WIDTH(w), HEIGHT(h)
{
	_width			= WIDTH;
	_height			= HEIGHT;
	_rotation		= 0;
	_xOrg			= 0;
	_yOrg			= 0;
	_cursorX    	= 0;
	_cursorY		= 0;
	_fontScale		= 1;
	_textColor		= 0xFFFF;
	_textBgColor	= 0xFFFF;
	_textWrap		= true;
	_cp437			= false;
	_gfxFont		= NULL;
}

void FishinoGFX::fillScreen(uint16_t color)
{
	fillRect(0, 0, _width, _height, color);
}

void FishinoGFX::invertDisplay(boolean i)
{
	// Do nothing, must be subclassed if supported by hardware
}

// Return the size of the display (per current rotation)
int16_t FishinoGFX::width(void) const
{
	return _width;
}

int16_t FishinoGFX::height(void) const
{
	return _height;
}
