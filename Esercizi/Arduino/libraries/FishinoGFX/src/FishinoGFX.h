//////////////////////////////////////////////////////////////////////////////////////
//																					//
//								FishinoGFX.h										//
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
#ifndef _FISHINO_GFX_H
#define _FISHINO_GFX_H

#include "Arduino.h"
#include "Print.h"

#include <FishinoFlash.h>

#include "gfxfont.h"
#include "FishinoGFX_Gradient.h"

// Color definitions
#define GFX_BLACK       0x0000      /*   0,   0,   0 */
#define GFX_NAVY        0x000F      /*   0,   0, 128 */
#define GFX_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define GFX_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define GFX_MAROON      0x7800      /* 128,   0,   0 */
#define GFX_PURPLE      0x780F      /* 128,   0, 128 */
#define GFX_OLIVE       0x7BE0      /* 128, 128,   0 */
#define GFX_LIGHTGREY   0xC618      /* 192, 192, 192 */
#define GFX_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define GFX_BLUE        0x001F      /*   0,   0, 255 */
#define GFX_GREEN       0x07E0      /*   0, 255,   0 */
#define GFX_CYAN        0x07FF      /*   0, 255, 255 */
#define GFX_RED         0xF800      /* 255,   0,   0 */
#define GFX_MAGENTA     0xF81F      /* 255,   0, 255 */
#define GFX_YELLOW      0xFFE0      /* 255, 255,   0 */
#define GFX_WHITE       0xFFFF      /* 255, 255, 255 */
#define GFX_ORANGE      0xFD20      /* 255, 165,   0 */
#define GFX_GREENYELLOW 0xAFE5      /* 173, 255,  47 */
#define GFX_PINK        0xF81F

#define GFX_DARKGRAY1	0x634e
#define GFX_DARKGRAY2	0x2945

typedef struct {
	bool hasTransparency;
	uint16_t width;
	uint16_t height;
	uint16_t const *pixels;
} ICON;

class FishinoGFX : public Print
{
	public:
		// corner values for circles and ellipses parts
		enum CORNER_NAMES
		{
			CORNER_180_270			= 0,
			CORNER_TOPLEFT			= 0,
			CORNER_270_0			= 1,
			CORNER_TOPRIGHT			= 1,
			CORNER_0_90				= 2,
			CORNER_BOTTOMRIGHT		= 2,
			CORNER_90_180			= 3,
			CORNER_BOTTOMLEFT		= 3,
		};
		
		// corner flags for circles and ellipses parts
		enum CORNER_FLAGS
		{
			CORNER_TOPLEFT_FLAG		= 0x01,
			CORNER_TOPRIGHT_FLAG	= 0x02,
			CORNER_BOTTOMRIGHT_FLAG	= 0x04,
			CORNER_BOTTOMLEFT_FLAG	= 0x08,
			CORNER_ALL_FLAG			= 0x0f
		};
		
	protected:
		// This is the 'raw' display w/h - never changes
		const int16_t WIDTH, HEIGHT;
		
		// Display w/h as modified by current rotation
		uint16_t _width, _height;
		
		// origin translation
		int16_t _xOrg, _yOrg;
		
		// screen rotation
		uint8_t _rotation;
		
		// text cursor position
		int16_t	_cursorX, _cursorY;
		
		// text parameters
		uint16_t _textColor, _textBgColor;
		uint8_t _fontScale;
		
		// If set, 'wrap' text at right edge of display
		bool _textWrap;
		
		// If set, use correct CP437 charset (default is off)
		bool _cp437;
		
		// font - if NULL use internal fixed font
		GFXfont *_gfxFont;

		// helper to shade a triangle
		// note : if vert is false, the vertices must be given as swapped x/y coordinates!
		void gradientTriangleHelper(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool vert, uint16_t color1, uint16_t color2);

		// helper function for curve (circle-ellipse-roundrects)
		// Parameters:
		// xc:			x location of the ellipse center
		// yc:			y location of the ellipse center
		// xr:			Size in pixels of horizontal radius
		// yr:			Size in pixels of vertical radius
		// curvePart:	bit flags marking corners to draw (see enum CORNER_FLAGS)
		// color:		RGB565 color
		// filled:		if true, the path is filled, otherwise just the outline get drawn
		void curveHelper(int16_t xc, int16_t yc, uint16_t xr, uint16_t yr, uint8_t curvePart, uint16_t color, bool filled);

		// helper function for gradient-filled curve (circle-ellipse-roundrects)
		// Parameters:
		// xc:			x location of the ellipse center
		// yc:			y location of the ellipse center
		// xr:			Size in pixels of horizontal radius
		// yr:			Size in pixels of vertical radius
		// curvePart:	bit flags marking corners to draw (see enum CORNER_FLAGS)
		// vert:		if true, the gradient is vertical, otherwise horizontal
		// gradient:	a FishinoGradient object, initialized for the required gradient
		void gradientCurveHelper(int16_t xc, int16_t yc, uint16_t xr, uint16_t yr, uint8_t curvePart, bool vert, FishinoGradient &gradient);

		// text write implementation
		size_t textWrite(uint8_t c);
		size_t textWrite(const char *buf, size_t len);
		
		// these functions works on absolute display coordinates, and can be redefined by child classes
		// the true drawing functions are on public part
		// This MUST be defined by the subclass:
		virtual void drawPixel0(int16_t x, int16_t y, uint16_t color) = 0;
		
		// This MUST be defined by subclass
		virtual void bitBlt0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t const *buf) = 0;
		virtual void bitBltT0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t const *buf) = 0;
		virtual void bitBltInv0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t const *buf) = 0;
		virtual void bitBltInvT0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t const *buf) = 0;

		// These MAY be overridden by the subclass to provide device-specific
		// optimized code.  Otherwise 'generic' versions are used.
		virtual void drawLine0(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
		virtual void drawFastVLine0(int16_t x, int16_t y, uint16_t h, uint16_t color);
		virtual void drawFastHLine0(int16_t x, int16_t y, uint16_t w, uint16_t color);

		virtual void drawRect0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color);
		virtual void fillRect0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color);

		virtual void gradientRectVertical0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color1, uint16_t color2);
		virtual void gradientRectHorizontal0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color1, uint16_t color2);

		virtual void gradientTriangleVertical0(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color1, uint16_t color2);
		virtual void gradientTriangleHorizontal0(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color1, uint16_t color2);

		// these MAY be overridden
		virtual void drawCircle0(int16_t x0, int16_t y0, uint16_t r, uint16_t color);
		virtual void fillCircle0(int16_t x0, int16_t y0, uint16_t r, uint16_t color);
		virtual void gradientCircleVertical0(int16_t x, int16_t y, uint16_t r, uint16_t color1, uint16_t color2);
		virtual void gradientCircleHorizontal0(int16_t x, int16_t y, uint16_t r, uint16_t color1, uint16_t color2);

		virtual void drawEllipse0(int16_t xc, int16_t yc, uint16_t xr, uint16_t yr, uint16_t color);
		virtual void fillEllipse0(int16_t xc, int16_t yc, uint16_t xr, uint16_t yr, uint16_t color);
		virtual void gradientEllipseVertical0(int16_t x0, int16_t y0, uint16_t rx, uint16_t ry, uint16_t color1, uint16_t color2);
		virtual void gradientEllipseHorizontal0(int16_t x0, int16_t y0, uint16_t rx, uint16_t ry, uint16_t color1, uint16_t color2);

		virtual void drawCurve0(int16_t xCenter, int16_t yCenter, uint16_t longAxis, uint16_t shortAxis, uint8_t curvePart, uint16_t color);
		virtual void fillCurve0(int16_t xCenter, int16_t yCenter, uint16_t longAxis, uint16_t shortAxis, uint8_t curvePart, uint16_t color);
		virtual void gradientCurveVertical0(int16_t x, int16_t y, uint16_t rx, uint16_t ry, uint8_t curvePart, uint16_t color1, uint16_t color2);
		virtual void gradientCurveHorizontal0(int16_t x, int16_t y, uint16_t rx, uint16_t ry, uint8_t curvePart, uint16_t color1, uint16_t color2);

		virtual void drawTriangle0(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
		virtual void fillTriangle0(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);

		virtual void drawRoundRect0(int16_t x0, int16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t color);
		virtual void fillRoundRect0(int16_t x0, int16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t color);

		virtual void gradientRoundRectVertical0(int16_t x0, int16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t color1, uint16_t color2);
		virtual void gradientRoundRectHorizontal0(int16_t x0, int16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t color1, uint16_t color2);

		virtual void drawBitmap0(int16_t x, int16_t y, const uint8_t *bitmap, uint16_t w, uint16_t h, uint16_t color);
		virtual void drawBitmap0(int16_t x, int16_t y, const uint8_t *bitmap, uint16_t w, uint16_t h, uint16_t color, uint16_t bg);
		virtual void drawBitmap0(int16_t x, int16_t y, uint8_t *bitmap, uint16_t w, uint16_t h, uint16_t color);
		virtual void drawBitmap0(int16_t x, int16_t y, uint8_t *bitmap, uint16_t w, uint16_t h, uint16_t color, uint16_t bg);
		virtual void drawXBitmap0(int16_t x, int16_t y, const uint8_t *bitmap, uint16_t w, uint16_t h, uint16_t color);

		virtual void drawChar0(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size);
		
		// draws a color bitmap on displays that supports 16 bit (565) color format
		virtual void drawBitmap0(ICON const &icon, int16_t x, int16_t y);
		virtual void drawBitmapInverted0(ICON const &icon, int16_t x, int16_t y);

		virtual void setCursor0(int16_t x, int16_t y);

		// get current cursor position (get rotation safe maximum values, using: width() for x, height() for y)
		virtual int16_t getCursorX0(void) const;
		virtual int16_t getCursorY0(void) const;
		virtual void getCursor0(int16_t &x, int16_t &y) const;
		
	public:

		// Constructor
		FishinoGFX(int16_t w, int16_t h);

		int16_t height(void) const;
		int16_t width(void) const;

		uint8_t getRotation(void) const;

		// translate origin
		void translate(int16_t dx, int16_t dy);
		
		// reset the origin
		void resetOrigin(void);
		
		// get current origin
		void getOrigin(int16_t &x, int16_t &y) const { x = _xOrg; y = _yOrg; }

		virtual void fillScreen(uint16_t color);
		virtual void invertDisplay(boolean i);
		
		void drawPixel(int16_t x, int16_t y, uint16_t color)
			{ drawPixel0(x + _xOrg, y + _xOrg, color); }
		
		void bitBlt(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t const *buf)
			{ bitBlt0(x + _xOrg, y + _yOrg, w, h, buf); }
		void bitBltT(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t const *buf)
			{ bitBltT0(x + _xOrg, y + _yOrg, w, h, buf); }
		void bitBltInv(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t const *buf)
			{ bitBltInv0(x + _xOrg, y + _yOrg, w, h, buf); }
		void bitBltInvT(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t const *buf)
			{ bitBltInvT0(x + _xOrg, y + _yOrg, w, h, buf); }

		void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
			{ drawLine0(x0 + _xOrg, y0 + _yOrg, x1 + _xOrg, y1 + _yOrg, color); }
		void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
			{ drawFastVLine0(x + _xOrg, y + _yOrg, h, color); }
		void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
			{ drawFastHLine0(x + _xOrg, y + _yOrg, w, color); }

		void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
			{ drawRect0(x + _xOrg, y + _yOrg, w, h, color); }
		void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
			{ fillRect0(x + _xOrg, y + _yOrg, w, h, color); }
		void gradientRectVertical(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color1, uint16_t color2)
			{ gradientRectVertical0(x + _xOrg, y + _yOrg, w, h, color1, color2); }
		void gradientRectHorizontal(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color1, uint16_t color2)
			{ gradientRectHorizontal0(x + _xOrg, y + _yOrg, w, h, color1, color2); }

		void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
			{ drawTriangle0(x0 + _xOrg, y0 + _yOrg, x1 + _xOrg, y1 + _yOrg, x2 + _xOrg, y2 + _yOrg, color); }
		void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
			{ fillTriangle0(x0 + _xOrg, y0 + _yOrg, x1 + _xOrg, y1 + _yOrg, x2 + _xOrg, y2 + _yOrg, color); }
		void gradientTriangleVertical(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color1, uint16_t color2)
			{ gradientTriangleVertical0(x0 + _xOrg, y0 + _yOrg, x1 + _xOrg, y1 + _yOrg, x2 + _xOrg, y2 + _yOrg, color1, color2); }
		void gradientTriangleHorizontal(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color1, uint16_t color2)
			{ gradientTriangleHorizontal0(x0 + _xOrg, y0 + _yOrg, x1 + _xOrg, y1 + _yOrg, x2 + _xOrg, y2 + _yOrg, color1, color2); }

		// ellipse arc function
		// Parameters:
		// xCenter:]   x location of the ellipse center
		// yCenter:   y location of the ellipse center
		// longAxis:  Size in pixels of the long axis
		// shortAxis: Size in pixels of the short axis
		// curvePart: Curve to draw in clock-wise dir: 0[180-270ﾰ],1[270-0ﾰ],2[0-90ﾰ],3[90-180ﾰ]
		// color: RGB565 color
		void drawCurve(int16_t x, int16_t y, uint16_t rx, uint16_t ry, uint8_t curvePart, uint16_t color)
			{ drawCurve0(x, y, rx, ry, curvePart, color); }
		void fillCurve(int16_t x, int16_t y, uint16_t rx, uint16_t ry, uint8_t curvePart, uint16_t color)
			{ fillCurve0(x, y, rx, ry, curvePart, color); }
		void gradientCurveVertical(int16_t x, int16_t y, uint16_t rx, uint16_t ry, uint8_t curvePart, uint16_t color1, uint16_t color2)
			{ gradientCurveVertical0(x, y, rx, ry, curvePart, color1, color2); }
		void gradientCurveHorizontal(int16_t x, int16_t y, uint16_t rx, uint16_t ry, uint8_t curvePart, uint16_t color1, uint16_t color2)
			{ gradientCurveHorizontal0(x, y, rx, ry, curvePart, color1, color2); }

		void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
			{ drawCircle0(x0 + _xOrg, y0 + _yOrg, r, color); }
		void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
			{ fillCircle0(x0 + _xOrg, y0 + _yOrg, r, color); }
		void gradientCircleVertical(int16_t x0, int16_t y0, uint16_t r, uint16_t color1, uint16_t color2)
			{ gradientCircleVertical0(x0 + _xOrg, y0 + _yOrg, r, color1, color2); }
		void gradientCircleHorizontal(int16_t x0, int16_t y0, uint16_t r, uint16_t color1, uint16_t color2)
			{ gradientCircleHorizontal0(x0 + _xOrg, y0 + _yOrg, r, color1, color2); }

		void drawEllipse(int16_t xc, int16_t yc, uint16_t xr, uint16_t yr, uint16_t color)
			{ drawEllipse0(xc + _xOrg, yc + _yOrg, xr, yr, color); }
		void fillEllipse(int16_t xc, int16_t yc, uint16_t xr, uint16_t yr, uint16_t color)
			{ fillEllipse0(xc + _xOrg, yc + _yOrg, xr, yr, color); }
		void gradientEllipseVertical(int16_t x0, int16_t y0, uint16_t rx, uint16_t ry, uint16_t color1, uint16_t color2)
			{ gradientEllipseVertical0(x0 + _xOrg, y0 + _yOrg, rx, ry, color1, color2); }
		void gradientEllipseHorizontal(int16_t x0, int16_t y0, uint16_t rx, uint16_t ry, uint16_t color1, uint16_t color2)
			{ gradientEllipseHorizontal0(x0 + _xOrg, y0 + _yOrg, rx, ry, color1, color2); }

		void drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color)
			{ drawRoundRect0(x0 + _xOrg, y0 + _yOrg, w, h, radius, color); }
		void fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color)
			{ fillRoundRect0(x0 + _xOrg, y0 + _yOrg, w, h, radius, color); }
		void gradientRoundRectVertical(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color1, uint16_t color2)
			{ gradientRoundRectVertical0(x0 + _xOrg, y0 + _yOrg, w, h, radius, color1, color2); }
		void gradientRoundRectHorizontal(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color1, uint16_t color2)
			{ gradientRoundRectHorizontal0(x0 + _xOrg, y0 + _yOrg, w, h, radius, color1, color2); }

		void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color)
			{ drawBitmap0(x + _xOrg, y + _yOrg, bitmap, w, h, color); }
		void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg)
			{ drawBitmap0(x + _xOrg, y + _yOrg, bitmap, w, h, color, bg); }
		void drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color)
			{ drawBitmap0(x + _xOrg, y + _yOrg, bitmap, w, h, color); }
		void drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg)
			{ drawBitmap0(x + _xOrg, y + _yOrg, bitmap, w, h, color, bg); }
		void drawXBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color)
			{ drawXBitmap0(x + _xOrg, y + _yOrg, bitmap, w, h, color); }
		void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size)
			{ drawChar0(x + _xOrg, y + _yOrg, c, color, bg, size); }

		// get current cursor position (get rotation safe maximum values, using: width() for x, height() for y)
		int16_t getCursorX(void) const
			{ return getCursorX0() - _xOrg; }
		int16_t getCursorY(void) const
			{ return getCursorY0() - _yOrg; }
		void getCursor(int16_t &x, int16_t &y) const
			{ getCursor0(x, y); x -= _xOrg; y -= _yOrg; }
		
		void setCursor(int16_t x, int16_t y)
			{ setCursor0(x + _xOrg, y + _yOrg); }

		virtual void setTextColor(uint16_t c);
		virtual void setTextColor(uint16_t c, uint16_t bg);
		
		// replaced with setFontScale(), which is more meaningful
		virtual void setFontScale(uint8_t s);
		
		
		virtual void setTextWrap(boolean w);
		virtual void setRotation(uint8_t r);
		virtual void cp437(boolean x=true);
		
		// sets an embedded font from Fonts folder
		// beware, memory hungry!
		virtual void setFont(const GFXfont *f = NULL);
		
		// this one is foreseen for drivers with multiple internal fonts
		// on normal drivers just resets to internal font
		virtual void setFontIndex(uint8_t fontIdx = 0);
		
		virtual void getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
		virtual void getTextBounds(const __FlashStringHelper *s, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);

		// draws a color bitmap on displays that supports 16 bit (565) color format
		virtual void drawBitmap(ICON const &icon, int16_t x, int16_t y)
			{ drawBitmap0(icon, x + _xOrg, y + _yOrg); }
		virtual void drawBitmapInverted(ICON const &icon, int16_t x, int16_t y)
			{ drawBitmapInverted0(icon, x + _xOrg, y + _yOrg); }

		virtual size_t write(uint8_t c) { return textWrite(c); }

		static inline uint16_t Color565(uint8_t r, uint8_t g, uint8_t b)
		{
			return ( ((uint16_t)r) << 11) | ( ((uint16_t)g) << 5) | b;
		}

};

#endif
