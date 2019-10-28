#include "FishinoGFX.h"
#include "FishinoGFX_defs.h"

//////////////////////////////////////////////////////////////////////////////////////////////
//										DRAWING HELPERS										//
//////////////////////////////////////////////////////////////////////////////////////////////

// helper to shade a triangle
// note : if vert is false, the vertices must be given as swapped x/y coordinates!
void FishinoGFX::gradientTriangleHelper(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool vert, uint16_t color1, uint16_t color2)
{
	int16_t a, b, y, last;
	
	// we use this one also for solid fill without gradient!
	bool grad = (color1 != color2);

	// Sort coordinates by Y order (y2 >= y1 >= y0)
	if (y0 > y1)
	{
		_swap_int16_t(y0, y1);
		_swap_int16_t(x0, x1);
	}
	if (y1 > y2)
	{
		_swap_int16_t(y2, y1);
		_swap_int16_t(x2, x1);
	}
	if (y0 > y1)
	{
		_swap_int16_t(y0, y1);
		_swap_int16_t(x0, x1);
	}

	if (y0 == y2)  // Handle awkward all-on-same-line case as its own thing
	{
		a = b = x0;
		if (x1 < a)
			a = x1;
		else if (x1 > b)
			b = x1;
		if (x2 < a)
			a = x2;
		else if (x2 > b)
			b = x2;
		if(vert)
			drawFastHLine0(a, y0, b - a + 1, color1);
		else
			drawFastVLine0(y0, a, b - a + 1, color1);
		return;
	}
	
	// create the gradient object
	FishinoGradient gradient(y0, y2, color1, color2);

	int16_t	dx01 = x1 - x0;
	int16_t dy01 = y1 - y0;
	int16_t dx02 = x2 - x0;
	int16_t dy02 = y2 - y0;
	int16_t dx12 = x2 - x1;
	int16_t dy12 = y2 - y1;
	int32_t	sa   = 0;
	int16_t sb   = 0;

	// For upper part of triangle, find scanline crossings for segments
	// 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
	// is included here (and second loop will be skipped, avoiding a /0
	// error there), otherwise scanline y1 is skipped here and handled
	// in the second loop...which also avoids a /0 error here if y0=y1
	// (flat-topped triangle).
	if (y1 == y2)
		last = y1;  // Include y1 scanline
	else
		last = y1-1; // Skip it

	for (y = y0; y <= last; y++)
	{
		a   = x0 + sa / dy01;
		b   = x0 + sb / dy02;
		sa += dx01;
		sb += dx02;
		/* longhand:
		a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
		b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
		*/
		if (a > b)
			_swap_int16_t(a, b);
		if(vert)
			drawFastHLine0(a, y, b - a + 1, grad ? gradient.getColor(y) : color1);
		else
			drawFastVLine0(y, a, b - a + 1, grad ? gradient.getColor(y) : color1);
	}

	// For lower part of triangle, find scanline crossings for segments
	// 0-2 and 1-2.  This loop is skipped if y1=y2.
	sa = dx12 * (y - y1);
	sb = dx02 * (y - y0);
	for (; y <= y2; y++)
	{
		a   = x1 + sa / dy12;
		b   = x0 + sb / dy02;
		sa += dx12;
		sb += dx02;
		/* longhand:
		a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
		b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
		*/
		if (a > b)
			_swap_int16_t(a, b);
		if(vert)
			drawFastHLine0(a, y, b - a + 1, grad ? gradient.getColor(y) : color1);
		else
			drawFastVLine0(y, a, b - a + 1, grad ? gradient.getColor(y) : color1);
	}
}

// helper function for curve (circle-ellipse-roundrects)
// Parameters:
// xc:			x location of the ellipse center
// yc:			y location of the ellipse center
// xr:			Size in pixels of horizontal radius
// yr:			Size in pixels of vertical radius
// curvePart:	bit flags marking corners to draw (see enum CORNER_FLAGS)
// color:		RGB565 color
// filled:		if true, the path is filled, otherwise just the outline get drawn
void FishinoGFX::curveHelper(int16_t xc, int16_t yc, uint16_t xr, uint16_t yr, uint8_t curvePart, uint16_t color, bool filled)
{
	int32_t x = xr;
	int32_t y = 0;
	int32_t xChange = yr * yr * (1 - 2 * (int32_t)xr);
	int32_t yChange = xr * xr;
	int32_t twoASquare = 2 * xr * xr;
	int32_t twoBSquare = 2 * yr * yr;
	int32_t ellipseError = 0;
	int32_t stoppingX = twoBSquare * xr;
	int32_t stoppingY = 0;
	
	while(stoppingX >= stoppingY)
	{
		if(filled)
		{
			if(curvePart & CORNER_TOPLEFT_FLAG)
				drawFastHLine0(xc - x, yc - y, x, color);

			if(curvePart & CORNER_TOPRIGHT_FLAG)
				drawFastHLine0(xc, yc - y, x, color);

			if(curvePart & CORNER_BOTTOMRIGHT_FLAG)
				drawFastHLine0(xc, yc + y, x, color);

			if(curvePart & CORNER_BOTTOMLEFT_FLAG)
				drawFastHLine0(xc - x, yc + y, x, color);
		}
		else
		{
			if(curvePart & CORNER_TOPLEFT_FLAG)
				drawPixel0(xc - x, yc - y, color);
			if(curvePart & CORNER_TOPRIGHT_FLAG)
				drawPixel0(xc + x, yc - y, color);
			if(curvePart & CORNER_BOTTOMRIGHT_FLAG)
				drawPixel0(xc + x, yc + y, color);
			if(curvePart & CORNER_BOTTOMLEFT_FLAG)
				drawPixel0(xc - x, yc + y, color);
		}

		y++;
		stoppingY += twoASquare;
		ellipseError += yChange;
		yChange += twoASquare;
		if(2 * ellipseError + xChange > 0)
		{
			x--;
			stoppingX -= twoBSquare;
			ellipseError += xChange;
			xChange += twoBSquare;
		}
	}
	
	x = 0;
	y = yr;
	xChange = yr * yr;
	yChange = xr * xr * (1 - 2 * (int32_t)yr);
	ellipseError = 0;
	stoppingX = 0;
	stoppingY = twoASquare * yr;
	while(stoppingX <= stoppingY)
	{
		if(!filled)
		{
			if(curvePart & CORNER_TOPLEFT_FLAG)
				drawPixel0(xc - x, yc - y, color);
			if(curvePart & CORNER_TOPRIGHT_FLAG)
				drawPixel0(xc + x, yc - y, color);
			if(curvePart & CORNER_BOTTOMRIGHT_FLAG)
				drawPixel0(xc + x, yc + y, color);
			if(curvePart & CORNER_BOTTOMLEFT_FLAG)
				drawPixel0(xc - x, yc + y, color);
		}

		stoppingX += twoBSquare;
		ellipseError += xChange;
		xChange += twoBSquare;
		if(2 * ellipseError + yChange > 0)
		{
			if(filled)
			{
				if(curvePart & CORNER_TOPLEFT_FLAG)
					drawFastHLine0(xc - x, yc - y, x, color);
	
				if(curvePart & CORNER_TOPRIGHT_FLAG)
					drawFastHLine0(xc, yc - y, x, color);
	
				if(curvePart & CORNER_BOTTOMRIGHT_FLAG)
					drawFastHLine0(xc, yc + y, x, color);
	
				if(curvePart & CORNER_BOTTOMLEFT_FLAG)
					drawFastHLine0(xc - x, yc + y, x, color);
			}
			y--;
			stoppingY -= twoASquare;
			ellipseError += yChange;
			yChange += twoASquare;
		}
		x++;
	}
}

// helper function for gradient-filled curve (circle-ellipse-roundrects)
// Parameters:
// xc:			x location of the ellipse center
// yc:			y location of the ellipse center
// xr:			Size in pixels of horizontal radius
// yr:			Size in pixels of vertical radius
// curvePart:	bit flags marking corners to draw (see enum CORNER_FLAGS)
// vert:		if true, the gradient is vertical, otherwise horizontal
// gradient:	a FishinoGradient object, initialized for the required gradient
void FishinoGFX::gradientCurveHelper(int16_t xc, int16_t yc, uint16_t xr, uint16_t yr, uint8_t curvePart, bool vert, FishinoGradient &gradient)
{
	int32_t x = xr;
	int32_t y = 0;
	int32_t xChange = yr * yr * (1 - 2 * (int32_t)xr);
	int32_t yChange = xr * xr;
	int32_t twoASquare = 2 * xr * xr;
	int32_t twoBSquare = 2 * yr * yr;
	int32_t ellipseError = 0;
	int32_t stoppingX = twoBSquare * xr;
	int32_t stoppingY = 0;
	
	while(stoppingX >= stoppingY)
	{
		if(vert)
		{
			uint16_t cTop = 0, cBottom = 0;
			if(curvePart & (CORNER_TOPLEFT_FLAG | CORNER_TOPRIGHT_FLAG))
				cTop = gradient.getColor(yc - y);
			if(curvePart & (CORNER_BOTTOMLEFT_FLAG | CORNER_BOTTOMRIGHT_FLAG))
				cBottom = gradient.getColor(yc + y);

			if(curvePart & CORNER_TOPLEFT_FLAG)
				drawFastHLine0(xc - x, yc - y, x, cTop);

			if(curvePart & CORNER_TOPRIGHT_FLAG)
				drawFastHLine0(xc, yc - y, x, cTop);

			if(curvePart & CORNER_BOTTOMRIGHT_FLAG)
				drawFastHLine0(xc, yc + y, x, cBottom);

			if(curvePart & CORNER_BOTTOMLEFT_FLAG)
				drawFastHLine0(xc - x, yc + y, x, cBottom);
		}

		stoppingY += twoASquare;
		ellipseError += yChange;
		yChange += twoASquare;
		if(2 * ellipseError + xChange > 0)
		{
			if(!vert)
			{
				uint16_t cLeft = 0, cRight = 0;
				if(curvePart & (CORNER_BOTTOMLEFT_FLAG | CORNER_TOPLEFT_FLAG))
					cLeft = gradient.getColor(xc - x);
				if(curvePart & (CORNER_BOTTOMRIGHT_FLAG | CORNER_TOPRIGHT_FLAG))
					cRight = gradient.getColor(xc + x);
	
				if(curvePart & CORNER_TOPLEFT_FLAG)
					drawFastVLine0(xc - x, yc - y, y, cLeft);
	
				if(curvePart & CORNER_TOPRIGHT_FLAG)
					drawFastVLine0(xc + x, yc - y, y, cRight);
	
				if(curvePart & CORNER_BOTTOMRIGHT_FLAG)
					drawFastVLine0(xc + x, yc, y, cRight);
	
				if(curvePart & CORNER_BOTTOMLEFT_FLAG)
					drawFastVLine0(xc - x, yc, y, cLeft);
			}
			x--;
			stoppingX -= twoBSquare;
			ellipseError += xChange;
			xChange += twoBSquare;
		}
		y++;
	}
	
	x = 0;
	y = yr;
	xChange = yr * yr;
	yChange = xr * xr * (1 - 2 * (int32_t)yr);
	ellipseError = 0;
	stoppingX = 0;
	stoppingY = twoASquare * yr;
	while(stoppingX <= stoppingY)
	{
		if(!vert)
		{
			uint16_t cLeft = 0, cRight = 0;
			if(curvePart & (CORNER_BOTTOMLEFT_FLAG | CORNER_TOPLEFT_FLAG))
				cLeft = gradient.getColor(xc - x);
			if(curvePart & (CORNER_BOTTOMRIGHT_FLAG | CORNER_TOPRIGHT_FLAG))
				cRight = gradient.getColor(xc + x);

			if(curvePart & CORNER_TOPLEFT_FLAG)
				drawFastVLine0(xc - x, yc - y, y, cLeft);

			if(curvePart & CORNER_TOPRIGHT_FLAG)
				drawFastVLine0(xc + x, yc - y, y, cRight);

			if(curvePart & CORNER_BOTTOMRIGHT_FLAG)
				drawFastVLine0(xc + x, yc, y, cRight);

			if(curvePart & CORNER_BOTTOMLEFT_FLAG)
				drawFastVLine0(xc - x, yc, y, cLeft);
		}
		stoppingX += twoBSquare;
		ellipseError += xChange;
		xChange += twoBSquare;
		if(2 * ellipseError + yChange > 0)
		{
			if(vert)
			{
				uint16_t cTop = 0, cBottom = 0;
				if(curvePart & (CORNER_TOPLEFT_FLAG | CORNER_TOPRIGHT_FLAG))
					cTop = gradient.getColor(yc - y);
				if(curvePart & (CORNER_BOTTOMLEFT_FLAG | CORNER_BOTTOMRIGHT_FLAG))
					cBottom = gradient.getColor(yc + y);
	
				if(curvePart & CORNER_TOPLEFT_FLAG)
					drawFastHLine0(xc - x, yc - y, x, cTop);
	
				if(curvePart & CORNER_TOPRIGHT_FLAG)
					drawFastHLine0(xc, yc - y, x, cTop);
	
				if(curvePart & CORNER_BOTTOMRIGHT_FLAG)
					drawFastHLine0(xc, yc + y, x, cBottom);
	
				if(curvePart & CORNER_BOTTOMLEFT_FLAG)
					drawFastHLine0(xc - x, yc + y, x, cBottom);
			}
			y--;
			stoppingY -= twoASquare;
			ellipseError += yChange;
			yChange += twoASquare;
		}
		x++;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//										DRAWING ROUTINES									//
//////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////
//											LINES											//
//////////////////////////////////////////////////////////////////////////////////////////////
void FishinoGFX::drawFastVLine0(int16_t x, int16_t y, uint16_t h, uint16_t color)
{
	// Update in subclasses if desired!
	drawLine0(x, y, x, y+h-1, color);
}

void FishinoGFX::drawFastHLine0(int16_t x, int16_t y, uint16_t w, uint16_t color)
{
	// Update in subclasses if desired!
	drawLine0(x, y, x+w-1, y, color);
}

// Bresenham's algorithm - thx wikpedia
void FishinoGFX::drawLine0(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
	int16_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep)
	{
		_swap_int16_t(x0, y0);
		_swap_int16_t(x1, y1);
	}

	if (x0 > x1)
	{
		_swap_int16_t(x0, x1);
		_swap_int16_t(y0, y1);
	}

	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1)
	{
		ystep = 1;
	}
	else
	{
		ystep = -1;
	}

	for (; x0<=x1; x0++)
	{
		if (steep)
		{
			drawPixel0(y0, x0, color);
		}
		else
		{
			drawPixel0(x0, y0, color);
		}
		err -= dy;
		if (err < 0)
		{
			y0 += ystep;
			err += dx;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//											POLYGONS										//
//////////////////////////////////////////////////////////////////////////////////////////////

// @@@ TO DO !!!!

//////////////////////////////////////////////////////////////////////////////////////////////
//											TRIANGLES										//
//////////////////////////////////////////////////////////////////////////////////////////////

void FishinoGFX::drawTriangle0(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
	drawLine0(x0, y0, x1, y1, color);
	drawLine0(x1, y1, x2, y2, color);
	drawLine0(x2, y2, x0, y0, color);
}

// Fill a triangle
void FishinoGFX::fillTriangle0(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
	gradientTriangleHelper(x0, y0, x1, y1, x2, y2, true, color, color);
}

void FishinoGFX::gradientTriangleVertical0(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color1, uint16_t color2)
{
	gradientTriangleHelper(x0, y0, x1, y1, x2, y2, true, color1, color2);
}

void FishinoGFX::gradientTriangleHorizontal0(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color1, uint16_t color2)
{
	gradientTriangleHelper(y0, x0, y1, x1, y2, x2, false, color1, color2);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//											RECTANGLES										//
//////////////////////////////////////////////////////////////////////////////////////////////

void FishinoGFX::drawRect0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color)
{
	drawFastHLine0(x, y, w, color);
	drawFastHLine0(x, y+h-1, w, color);
	drawFastVLine0(x, y, h, color);
	drawFastVLine0(x+w-1, y, h, color);
}

void FishinoGFX::fillRect0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color)
{
	// Update in subclasses if desired!
	for (int16_t i = x; i < x + (int16_t)w; i++)
	{
		drawFastVLine0(i, y, h, color);
	}
}

void FishinoGFX::gradientRectVertical0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color1, uint16_t color2)
{
	int16_t y2 = y + h - 1;
	FishinoGradient gradient(y, y2, color1, color2);
	while(y <= y2)
	{
		drawFastHLine0(x, y, w, gradient.nextColor());
		y++;
	}
}

void FishinoGFX::gradientRectHorizontal0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color1, uint16_t color2)
{
	int16_t x2 = x + w - 1;
	FishinoGradient gradient(x, x2, color1, color2);
	while(x <= x2)
	{
		drawFastVLine0(x, y, h, gradient.nextColor());
		x++;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//								CURVES (ELLIPSE QUARTERS)									//
//////////////////////////////////////////////////////////////////////////////////////////////

// ellipse arc function
// Parameters:
// xCenter:]   x location of the ellipse center
// yCenter:   y location of the ellipse center
// longAxis:  Size in pixels of the long axis
// shortAxis: Size in pixels of the short axis
// curvePart: Curve to draw in clock-wise dir: 0[180-270ﾰ],1[270-0ﾰ],2[0-90ﾰ],3[90-180ﾰ]
// color: RGB565 color
void FishinoGFX::drawCurve0(int16_t xCenter, int16_t yCenter, uint16_t longAxis, uint16_t shortAxis, uint8_t curvePart, uint16_t color)
{
	switch(curvePart)
	{
		case CORNER_TOPLEFT :
		default:
			curvePart = CORNER_TOPLEFT_FLAG;
			break;
		case CORNER_TOPRIGHT :
			curvePart = CORNER_TOPRIGHT_FLAG;
			break;
		case CORNER_BOTTOMRIGHT :
			curvePart = CORNER_BOTTOMRIGHT_FLAG;
			break;
		case CORNER_BOTTOMLEFT :
			curvePart = CORNER_BOTTOMLEFT_FLAG;
			break;
	}
	curveHelper(xCenter, yCenter, longAxis, shortAxis, curvePart, color, false);
}

void FishinoGFX::fillCurve0(int16_t xCenter, int16_t yCenter, uint16_t longAxis, uint16_t shortAxis, uint8_t curvePart, uint16_t color)
{
	switch(curvePart)
	{
		case CORNER_TOPLEFT :
			curvePart = CORNER_TOPLEFT_FLAG;
			break;
		case CORNER_TOPRIGHT :
			curvePart = CORNER_TOPRIGHT_FLAG;
			break;
		case CORNER_BOTTOMRIGHT :
			curvePart = CORNER_BOTTOMRIGHT_FLAG;
			break;
		case CORNER_BOTTOMLEFT :
			curvePart = CORNER_BOTTOMLEFT_FLAG;
			break;
	}
	curveHelper(xCenter, yCenter, longAxis, shortAxis, curvePart, color, true);
}

void FishinoGFX::gradientCurveVertical0(int16_t x, int16_t y, uint16_t rx, uint16_t ry, uint8_t curvePart, uint16_t color1, uint16_t color2)
{
	int16_t yTop = y;
	int16_t yBottom = y;
	switch(curvePart)
	{
		case CORNER_TOPLEFT :
			yTop -= ry;
			curvePart = CORNER_TOPLEFT_FLAG;
			break;
		case CORNER_TOPRIGHT :
			yTop -= ry;
			curvePart = CORNER_TOPRIGHT_FLAG;
			break;
		case CORNER_BOTTOMRIGHT :
			yBottom += ry;
			curvePart = CORNER_BOTTOMRIGHT_FLAG;
			break;
		case CORNER_BOTTOMLEFT :
			yBottom += ry;
			curvePart = CORNER_BOTTOMLEFT_FLAG;
			break;
	}
	FishinoGradient gradient(yTop, yBottom, color1, color2);
	gradientCurveHelper(x, y, rx, ry, curvePart, true, gradient);
}

void FishinoGFX::gradientCurveHorizontal0(int16_t x, int16_t y, uint16_t rx, uint16_t ry, uint8_t curvePart, uint16_t color1, uint16_t color2)
{
	int16_t xLeft = x;
	int16_t xRight = x;
	switch(curvePart)
	{
		case CORNER_TOPLEFT :
			xLeft -= rx;
			curvePart = CORNER_TOPLEFT_FLAG;
			break;
		case CORNER_TOPRIGHT :
			xRight += rx;
			curvePart = CORNER_TOPRIGHT_FLAG;
			break;
		case CORNER_BOTTOMRIGHT :
			xRight += rx;
			curvePart = CORNER_BOTTOMRIGHT_FLAG;
			break;
		case CORNER_BOTTOMLEFT :
			xLeft -= rx;
			curvePart = CORNER_BOTTOMLEFT_FLAG;
			break;
	}
	FishinoGradient gradient(xLeft, xRight, color1, color2);
	gradientCurveHelper(x, y, rx, ry, curvePart, false, gradient);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//											CIRCLES											//
//////////////////////////////////////////////////////////////////////////////////////////////

void FishinoGFX::drawCircle0(int16_t x0, int16_t y0, uint16_t r, uint16_t color)
{
	curveHelper(x0, y0, r, r, CORNER_ALL_FLAG, color, false);
}

void FishinoGFX::fillCircle0(int16_t x0, int16_t y0, uint16_t r, uint16_t color)
{
	curveHelper(x0, y0, r, r, CORNER_ALL_FLAG, color, true);
}

void FishinoGFX::gradientCircleVertical0(int16_t x, int16_t y, uint16_t r, uint16_t color1, uint16_t color2)
{
	FishinoGradient gradient(y - r, y + r, color1, color2);

	gradientCurveHelper(x, y, r, r, CORNER_ALL_FLAG, true, gradient);
}

void FishinoGFX::gradientCircleHorizontal0(int16_t x, int16_t y, uint16_t r, uint16_t color1, uint16_t color2)
{
	FishinoGradient gradient(x - r, x + r, color1, color2);

	gradientCurveHelper(x, y, r, r, CORNER_ALL_FLAG, false, gradient);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//											ELLIPSES										//
//////////////////////////////////////////////////////////////////////////////////////////////

void FishinoGFX::drawEllipse0(int16_t xc, int16_t yc, uint16_t xr, uint16_t yr, uint16_t color)
{
	curveHelper(xc, yc, xr, yr, CORNER_ALL_FLAG, color, false);
}

void FishinoGFX::fillEllipse0(int16_t xc, int16_t yc, uint16_t xr, uint16_t yr, uint16_t color)
{
	curveHelper(xc, yc, xr, yr, CORNER_ALL_FLAG, color, true);
}

void FishinoGFX::gradientEllipseVertical0(int16_t x, int16_t y, uint16_t rx, uint16_t ry, uint16_t color1, uint16_t color2)
{
	FishinoGradient gradient(y - ry, y + ry, color1, color2);
	gradientCurveHelper(x, y, rx, ry, CORNER_ALL_FLAG, true, gradient);
}

void FishinoGFX::gradientEllipseHorizontal0(int16_t x, int16_t y, uint16_t rx, uint16_t ry, uint16_t color1, uint16_t color2)
{
	FishinoGradient gradient(x - rx, x + rx, color1, color2);
	gradientCurveHelper(x, y, rx, ry, CORNER_ALL_FLAG, false, gradient);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//									ROUNDED-CORNERS RECTANGLES								//
//////////////////////////////////////////////////////////////////////////////////////////////

void FishinoGFX::drawRoundRect0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r, uint16_t color)
{
	// smarter version
	drawFastHLine0(x + r, y    , w -2 * r, color);	// Top
	drawFastHLine0(x + r, y + h, w -2 * r, color);	// Bottom
	drawFastVLine0(x    , y + r, h -2 * r, color);	// Left
	drawFastVLine0(x + w, y + r, h -2 * r, color);	// Right

	// draw four corners
	curveHelper(x + r    , y + r    , r, r, 1, color, false);
	curveHelper(x + w - r, y + r    , r, r, 2, color, false);
	curveHelper(x + w - r, y + h - r, r, r, 4, color, false);
	curveHelper(x + r    , y + h - r, r, r, 8, color, false);
}

// Fill a rounded rectangle
void FishinoGFX::fillRoundRect0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r, uint16_t color)
{
	// smarter version
	fillRect0(x + r, y, w - 2 * r, h, color);
	fillRect0(x, y + r, w, h - 2 * r, color);

	// draw four corners
	curveHelper(x + r        , y + r        , r, r, 1, color, true);
	curveHelper(x + w - r - 1, y + r        , r, r, 2, color, true);
	curveHelper(x + w - r - 1, y + h - r - 1, r, r, 4, color, true);
	curveHelper(x + r        , y + h - r - 1, r, r, 8, color, true);
}

void FishinoGFX::gradientRoundRectVertical0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r, uint16_t color1, uint16_t color2)
{
	FishinoGradient gradient(y, y + h, color1, color2);

	// smarter version
	gradientRectVertical0(x + r, y, w - 2 * r, h, gradient.getColor(y), gradient.getColor(y + h));
	gradientRectVertical0(x, y + r, w, h - 2 * r, gradient.getColor(y + r), gradient.getColor(y + h - r));

	// draw four corners
	gradientCurveHelper(x + r        , y + r        , r, r, 1, true, gradient);
	gradientCurveHelper(x + w - r - 1, y + r        , r, r, 2, true, gradient);
	gradientCurveHelper(x + w - r - 1, y + h - r - 1, r, r, 4, true, gradient);
	gradientCurveHelper(x + r        , y + h - r - 1, r, r, 8, true, gradient);
}

void FishinoGFX::gradientRoundRectHorizontal0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r, uint16_t color1, uint16_t color2)
{
	FishinoGradient gradient(x, x + w, color1, color2);

	// smarter version
	gradientRectHorizontal0(x + r, y, w - 2 * r, h, gradient.getColor(x + r), gradient.getColor(x + w - r));
	gradientRectHorizontal0(x, y + r, w, h - 2 * r, gradient.getColor(x), gradient.getColor(x + w));

	// draw four corners
	gradientCurveHelper(x + r        , y + r        , r, r, 1, false, gradient);
	gradientCurveHelper(x + w - r - 1, y + r        , r, r, 2, false, gradient);
	gradientCurveHelper(x + w - r - 1, y + h - r - 1, r, r, 4, false, gradient);
	gradientCurveHelper(x + r        , y + h - r - 1, r, r, 8, false, gradient);
}
