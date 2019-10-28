#include "FishinoRA8875SPI.h"

//////////////////////////////////////////////////////////////////////
//						Helpers functions							//
//////////////////////////////////////////////////////////////////////

// common helper for check value limiter
// limits are checked on HARDWARE orientation; if screen is rotated
// the coordinates must be swapped BEFORE checking limits
void FishinoRA8875Class::checkLimitsHelper(int16_t &x,int16_t &y)
{
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;
	if (x >= (int16_t)WIDTH)
		x = WIDTH - 1;
	if (y >= (int16_t)HEIGHT)
		y = HEIGHT -1;
}

// Graphic line addressing helper
void FishinoRA8875Class::lineAddressing(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
	//X0
	writeReg(RA8875_DLHSR0,x0);
	writeReg(RA8875_DLHSR1,x0 >> 8);
	//Y0
	writeReg(RA8875_DLVSR0,y0);
	writeReg(RA8875_DLVSR1,y0 >> 8);
	//X1
	writeReg(RA8875_DLHER0,x1);
	writeReg(RA8875_DLHER1,(x1) >> 8);
	//Y1
	writeReg(RA8875_DLVER0,y1);
	writeReg(RA8875_DLVER1,(y1) >> 8);
}

// curve addressing
void FishinoRA8875Class::curveAddressing(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
	//center
	writeReg(RA8875_DEHR0,x0);
	writeReg(RA8875_DEHR1,x0 >> 8);
	writeReg(RA8875_DEVR0,y0);
	writeReg(RA8875_DEVR1,y0 >> 8);
	//long,short ax
	writeReg(RA8875_ELL_A0,x1);
	writeReg(RA8875_ELL_A1,x1 >> 8);
	writeReg(RA8875_ELL_B0,y1);
	writeReg(RA8875_ELL_B1,y1 >> 8);
}

// helper function for circles
void FishinoRA8875Class::circleHelper(int16_t x0, int16_t y0, uint16_t r, uint16_t color, bool filled)
{
	if(_swapxy)
	{
		swap(x0, y0);
	}
	
	checkLimitsHelper(x0,y0);
	if (r < 1)
		r = 1;

	writeReg(RA8875_DCHR0,x0);
	writeReg(RA8875_DCHR1,x0 >> 8);

	writeReg(RA8875_DCVR0,y0);
	writeReg(RA8875_DCVR1,y0 >> 8);

	writeReg(RA8875_DCRR,r);

	setForegroundColor(color);

	writeCommand(RA8875_DCR);
	filled == true ? writeData(RA8875_DCR_CIRCLE_START | RA8875_DCR_FILL) : writeData(RA8875_DCR_CIRCLE_START | RA8875_DCR_NOFILL);
	waitPoll(RA8875_DCR, RA8875_DCR_CIRCLE_STATUS);
}

// helper function for rects
void FishinoRA8875Class::rectHelper(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color, bool filled)
{
	if(_swapxy)
	{
		swap(x, y);
		swap(w, h);
	}
	
	checkLimitsHelper(x, y);
	if(x + w >= WIDTH)
		w = WIDTH - x;
	if(y + h >= HEIGHT)
		h = HEIGHT - y;
	if(w == 0 || h == 0)
		return;

	lineAddressing(x, y, x + w - 1, y + h - 1);

	setForegroundColor(color);

	writeCommand(RA8875_DCR);
	filled == true ? writeData(0xB0) : writeData(0x90);
	waitPoll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}

// helper function for triangles
void FishinoRA8875Class::triangleHelper(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color, bool filled)
{
	if(_swapxy)
	{
		swap(x0, y0);
		swap(x1, y1);
		swap(x2, y2);
	}
	checkLimitsHelper(x0,y0);
	checkLimitsHelper(x1,y1);
	checkLimitsHelper(x2,y2);

	lineAddressing(x0,y0,x1,y1);
	//p2
	writeReg(RA8875_DTPH0,x2);
	writeReg(RA8875_DTPH1,x2 >> 8);
	writeReg(RA8875_DTPV0,y2);
	writeReg(RA8875_DTPV1,y2 >> 8);

	setForegroundColor(color);

	writeCommand(RA8875_DCR);
	filled == true ? writeData(0xA1) : writeData(0x81);
	waitPoll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}

// helper function for ellipse
void FishinoRA8875Class::ellipseHelper(int16_t xCenter, int16_t yCenter, uint16_t longAxis, uint16_t shortAxis, uint16_t color, bool filled)
{
	if(_swapxy)
	{
		swap(xCenter, yCenter);
		swap(longAxis, shortAxis);
	}
	//TODO:limits!
	curveAddressing(xCenter, yCenter, longAxis, shortAxis);

	setForegroundColor(color);

	writeCommand(RA8875_ELLIPSE);
	filled == true ? writeData(0xC0) : writeData(0x80);
	waitPoll(RA8875_ELLIPSE, RA8875_ELLIPSE_STATUS);
}

// helper function for curve
void FishinoRA8875Class::curveHelper(int16_t xCenter, int16_t yCenter, uint16_t longAxis, uint16_t shortAxis, uint8_t curvePart, uint16_t color, bool filled)
{
	if(_swapxy)
	{
		swap(xCenter, yCenter);
		swap(longAxis, shortAxis);
		if(curvePart == 2)
			curvePart = 0;
		else if(curvePart == 0)
			curvePart = 2;
	}
	//TODO:limits!
	curveAddressing(xCenter, yCenter, longAxis, shortAxis);

	setForegroundColor(color);

	writeCommand(RA8875_ELLIPSE);
	filled == true ? writeData(0xD0 | (curvePart & 0x03)) : writeData(0x90 | (curvePart & 0x03));
	waitPoll(RA8875_ELLIPSE, RA8875_ELLIPSE_STATUS);
}

// helper function for rounded Rects
void FishinoRA8875Class::roundRectHelper(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r, uint16_t color, bool filled)
{
	// radius must be smaller than shortest side
	if(r >= h / 2)
		r = h / 2 - 1;
	if(r >= w / 2)
		r = w / 2 - 1;

	// if radius is null, just use rect
	if (r < 1 || h < 4 || w < 4)
	{
		rectHelper(x, y, w, h, color, filled);
		return;
	}

	if(_swapxy)
	{
		swap(x, y);
		swap(w, h);
	}

	checkLimitsHelper(x, y);
	if(x + w >= WIDTH)
		w = WIDTH - x;
	if(y + h >= HEIGHT)
		h = HEIGHT - y;
	if(w == 0 || h == 0)
		return;

	lineAddressing(x, y, x + w - 1, y + h - 1);
	//P2
	writeReg(RA8875_ELL_A0, r);
	writeReg(RA8875_ELL_A1, r >> 8);
	writeReg(RA8875_ELL_B0, r);
	writeReg(RA8875_ELL_B1, r >> 8);

	setForegroundColor(color);

	writeCommand(RA8875_ELLIPSE);
	filled == true ? writeData(0xE0) : writeData(0xA0);
	waitPoll(RA8875_ELLIPSE, RA8875_DCR_LINESQUTRI_STATUS);
}

//////////////////////////////////////////////////////////////////////
//						Drawing functions							//
//////////////////////////////////////////////////////////////////////


// Basic pixel write
// Parameters:
// x:horizontal pos
// y:vertical pos
// color: RGB565 color
void FishinoRA8875Class::drawPixel0(int16_t x, int16_t y, uint16_t color)
{
	//checkLimitsHelper(x, y);
	setCursor0(x,y);

	writeCommand(RA8875_MRWC);
	writeData16(color);
}

// Basic line draw
// Parameters:
// x0:horizontal start pos
//  y0:vertical start
// x1:horizontal end pos
// y1:vertical end pos
// color: RGB565 color
void FishinoRA8875Class::drawLine0(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
	Serial << "X0:" << x0 << "  Y0:" << y0 << "  X1:" << x1 << "  Y1:" << y1 << "\n";
	if(x0 == x1 && y0 == y1)
	{
		drawPixel0(x0, y0, color);
		return;
	}

	if(_swapxy)
	{
		swap(x0, y0);
		swap(x1, y1);
	}

	checkLimitsHelper(x0, y0);
	checkLimitsHelper(x1, y1);

	lineAddressing(x0, y0, x1, y1);

	setForegroundColor(color);

	writeReg(RA8875_DCR, 0x80);
	waitPoll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}

// for compatibility with popular Adafruit_GFX
// draws a single vertical line
// Parameters:
// x:horizontal start
// y:vertical start
// h:height
// color: RGB565 color
void FishinoRA8875Class::drawFastVLine0(int16_t x, int16_t y, int16_t h, uint16_t color)
{
	if (h < 1)
		h = 1;
	drawLine0(x, y, x, y + h, color);
}

// for compatibility with popular Adafruit_GFX
// draws a single orizontal line
// Parameters:
// x:horizontal start
// y:vertical start
// w:width
// color: RGB565 color
void FishinoRA8875Class::drawFastHLine0(int16_t x, int16_t y, int16_t w, uint16_t color)
{
	if (w < 1)
		w = 1;
	drawLine(x, y, x + w, y, color);

}

// draws a rectangle
// Parameters:
// x:horizontal start
// y:vertical start
// w: width
// h:height
// color: RGB565 color
void FishinoRA8875Class::drawRect0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color)
{
	rectHelper(x, y, w, h, color, false);
}

// draws a FILLED rectangle
// Parameters:
// x:horizontal start
// y:vertical start
// w: width
// h:height
// color: RGB565 color
void FishinoRA8875Class::fillRect0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color)
{
	rectHelper(x, y, w, h, color, true);
}

// Fill the screen by using a specified RGB565 color
// Parameters:
// color: RGB565 color
void FishinoRA8875Class::fillScreen(uint16_t color)
{
	lineAddressing(0, 0, WIDTH - 1, HEIGHT - 1);
	setForegroundColor(color);
	writeCommand(RA8875_DCR);
	writeData(0xB0);
	waitPoll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}

// Draw circle
// Parameters:
// x0:The 0-based x location of the center of the circle
// y0:The 0-based y location of the center of the circle
// r:radius
// color: RGB565 color
void FishinoRA8875Class::drawCircle0(int16_t x0, int16_t y0, uint16_t r, uint16_t color)
{
	if (r <= 0)
		return;
	circleHelper(x0, y0, r, color, false);
}

// Draw filled circle
// Parameters:
// x0:The 0-based x location of the center of the circle
// y0:The 0-based y location of the center of the circle
// r:radius
// color: RGB565 color
void FishinoRA8875Class::fillCircle0(int16_t x0, int16_t y0, uint16_t r, uint16_t color)
{
	if (r <= 0)
		return;
	circleHelper(x0, y0, r, color, true);
}

// Draw Triangle
// Parameters:
// x0:The 0-based x location of the point 0 of the triangle LEFT
// y0:The 0-based y location of the point 0 of the triangle LEFT
// x1:The 0-based x location of the point 1 of the triangle TOP
// y1:The 0-based y location of the point 1 of the triangle TOP
// x2:The 0-based x location of the point 2 of the triangle RIGHT
// y2:The 0-based y location of the point 2 of the triangle RIGHT
// color: RGB565 color
void FishinoRA8875Class::drawTriangle0(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
	triangleHelper(x0, y0, x1, y1, x2, y2, color, false);
}

// Draw filled Triangle
// Parameters:
// x0:The 0-based x location of the point 0 of the triangle
// y0:The 0-based y location of the point 0 of the triangle
// x1:The 0-based x location of the point 1 of the triangle
// y1:The 0-based y location of the point 1 of the triangle
// x2:The 0-based x location of the point 2 of the triangle
// y2:The 0-based y location of the point 2 of the triangle
// color: RGB565 color
void FishinoRA8875Class::fillTriangle0(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
	triangleHelper(x0, y0, x1, y1, x2, y2, color, true);
}

// Draw an ellipse
// Parameters:
// xCenter:   x location of the center of the ellipse
// yCenter:   y location of the center of the ellipse
// longAxis:  Size in pixels of the long axis
// shortAxis: Size in pixels of the short axis
// color: RGB565 color
void FishinoRA8875Class::drawEllipse0(int16_t xCenter, int16_t yCenter, uint16_t longAxis, uint16_t shortAxis, uint16_t color)
{
	ellipseHelper(xCenter, yCenter, longAxis, shortAxis, color, false);
}

// Draw a filled ellipse
// Parameters:
// xCenter:   x location of the center of the ellipse
// yCenter:   y location of the center of the ellipse
// longAxis:  Size in pixels of the long axis
// shortAxis: Size in pixels of the short axis
// color: RGB565 color
void FishinoRA8875Class::fillEllipse0(int16_t xCenter, int16_t yCenter, uint16_t longAxis, uint16_t shortAxis, uint16_t color)
{
	ellipseHelper(xCenter, yCenter, longAxis, shortAxis, color, true);
}

// Draw a curve
// Parameters:
// xCenter:]   x location of the ellipse center
// yCenter:   y location of the ellipse center
// longAxis:  Size in pixels of the long axis
// shortAxis: Size in pixels of the short axis
// curvePart: Curve to draw in clock-wise dir: 0[180-270ﾰ],1[270-0ﾰ],2[0-90ﾰ],3[90-180ﾰ]
// color: RGB565 color
void FishinoRA8875Class::drawCurve0(int16_t xCenter, int16_t yCenter, uint16_t longAxis, uint16_t shortAxis, uint8_t curvePart, uint16_t color)
{
	curveHelper(xCenter, yCenter, longAxis, shortAxis, curvePart, color, false);
}

// Draw a filled curve
// Parameters:
// xCenter:]   x location of the ellipse center
// yCenter:   y location of the ellipse center
// longAxis:  Size in pixels of the long axis
// shortAxis: Size in pixels of the short axis
// curvePart: Curve to draw in clock-wise dir: 0[180-270ﾰ],1[270-0ﾰ],2[0-90ﾰ],3[90-180ﾰ]
// color: RGB565 color
void FishinoRA8875Class::fillCurve0(int16_t xCenter, int16_t yCenter, uint16_t longAxis, uint16_t shortAxis, uint8_t curvePart, uint16_t color)
{
	curveHelper(xCenter, yCenter, longAxis, shortAxis, curvePart, color, true);
}

// Draw a rounded rectangle
// Parameters:
// x:   x location of the rectangle
// y:   y location of the rectangle
// w:  the width in pix
// h:  the height in pix
// r:  the radius of the rounded corner
// color: RGB565 color
void FishinoRA8875Class::drawRoundRect0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r, uint16_t color)
{
	roundRectHelper(x, y, w, h, r, color, false);
}

// Draw a filled rounded rectangle
// Parameters:
// x:   x location of the rectangle
// y:   y location of the rectangle
// w:  the width in pix
// h:  the height in pix
// r:  the radius of the rounded corner
// color: RGB565 color
void FishinoRA8875Class::fillRoundRect0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r, uint16_t color)
{
	roundRectHelper(x, y, w, h, r, color, true);
}
