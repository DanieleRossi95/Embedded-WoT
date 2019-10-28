#include "FishinoRA8875SPI.h"

// Set graphic cursor beween 8 different ones.
// Graphic cursors has to be inserted before use!
// Parameters:
// cur: 0...7
void FishinoRA8875Class::setGraphicCursor(uint8_t cur)
{
	if (cur > 7)
		cur = 7;
	uint8_t temp = readReg(RA8875_MWCR1);
	temp &= ~(0x70);//clear bit 6,5,4
	temp |= cur << 4;
	temp |= cur;
	if (_useMultiLayers)
	{
		_currentLayer == 1 ? temp |= (1 << 0) : temp &= ~(1 << 0);
	}
	else
	{
		temp &= ~(1 << 0);//
	}
	writeData(temp);
}

// Show the graphic cursor
// Graphic cursors has to be inserted before use!
// Parameters:
// cur: true,false
void FishinoRA8875Class::showGraphicCursor(bool cur)
{
	uint8_t temp = readReg(RA8875_MWCR1);
	cur == true ? temp |= (1 << 7) : temp &= ~(1 << 7);
	if (_useMultiLayers)
	{
		_currentLayer == 1 ? temp |= (1 << 0) : temp &= ~(1 << 0);
	}
	else
	{
		temp &= ~(1 << 0);//
	}
	writeData(temp);
}

// Set the position for Graphic Write
// Parameters:
// x:horizontal position
// y:vertical position
void FishinoRA8875Class::setCursor0(int16_t x, int16_t y)
{
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;

	setCursorX0(x);
	setCursorY0(y);
}

void FishinoRA8875Class::setCursorX0(int16_t x)
{
	if (x >= (int16_t)_width)
		x = _width-1;
	_cursorX = x;

	if(_swapxy)
	{
		writeReg(RA8875_CURV0, x);
		writeReg(RA8875_CURV1, x >> 8);
	
		writeReg(RA8875_F_CURYL,(x & 0xFF));
		writeReg(RA8875_F_CURYH,(x >> 8));
	}
	else
	{
		writeReg(RA8875_CURH0, x);
		writeReg(RA8875_CURH1, x >> 8);
	
		writeReg(RA8875_F_CURXL,(x & 0xFF));
		writeReg(RA8875_F_CURXH,(x >> 8));
	}
}


void FishinoRA8875Class::setCursorY0(int16_t y)
{
	if (y >= (int16_t)_height)
		y = _height-1;
	_cursorY = y;
	
	if(_swapxy)
	{
		writeReg(RA8875_CURH0, y);
		writeReg(RA8875_CURH1, y >> 8);
	
		writeReg(RA8875_F_CURXL,(y & 0xFF));
		writeReg(RA8875_F_CURXH,(y >> 8));
	}
	else
	{
		writeReg(RA8875_CURV0, y);
		writeReg(RA8875_CURV1, y >> 8);
	
		writeReg(RA8875_F_CURYL,(y & 0xFF));
		writeReg(RA8875_F_CURYH,(y >> 8));
	}
}

// get current graphic write position
void FishinoRA8875Class::getCursor0(int16_t &x, int16_t &y)
{
	x = readReg(RA8875_CURH1) & 0x03;
	x = (x << 8) | readReg(RA8875_CURH0);

	y = readReg(RA8875_CURV1) & 0x01;
	y = (y << 8) | readReg(RA8875_CURV0);
}


void FishinoRA8875Class::setReadCursorX0(int16_t x)
{
	if (x >= (int16_t)_width)
		x = _width-1;

	writeReg(RA8875_RCURH0, x & 0xFF);
	writeReg(RA8875_RCURH1, x >> 8);
}

void FishinoRA8875Class::setReadCursorY0(int16_t y)
{
	if (y >= (int16_t)_height)
		y = _height-1;

	writeReg(RA8875_RCURV0, y & 0xFF);
	writeReg(RA8875_RCURV1, y >> 8);
}

void FishinoRA8875Class::setReadCursor0(int16_t x, int16_t y)
{
	if(x < 0)
		x = 0;
	if(y < 0)
		y = 0;
	setReadCursorX0(x);
	setReadCursorY0(y);
}


void FishinoRA8875Class::getReadCursor0(int16_t &x, int16_t &y)
{
	x = (readReg(RA8875_RCURH1) & 0x01) << 8 | readReg(RA8875_RCURH0);
	y = (readReg(RA8875_RCURV1) & 0x01) << 8 | readReg(RA8875_RCURV0);
}

// Set the Active Window
// Parameters:
// x1: Horizontal Left
// x1: Vertical TOP
// x2: Horizontal Right
// x2: Vertical Bottom
void FishinoRA8875Class::setActiveWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	if(_swapxy)
	{
		swap(x1, y1);
		swap(x2, y2);
	}
	if ((int16_t)x2 >= WIDTH)
		x2 = WIDTH - 1;
	if ((int16_t)y2 >= HEIGHT)
		y2 = HEIGHT - 1;
	
	// y
	writeReg(RA8875_HSAW0, x1);
	writeReg(RA8875_HSAW1, x1 >> 8);
	writeReg(RA8875_HEAW0, x2);
	writeReg(RA8875_HEAW1, x2 >> 8);
	
	// y
	writeReg(RA8875_VSAW0, y1);
	writeReg(RA8875_VSAW1, y1 >> 8);
	writeReg(RA8875_VEAW0, y2);
	writeReg(RA8875_VEAW1, y2 >> 8);
}

// Set the Active Window as FULL SCREEN
void FishinoRA8875Class::setActiveWindow(void)
{
	setActiveWindow(0, 0, _width - 1, _height - 1);
}

// get the active window
void FishinoRA8875Class::getActiveWindow(uint16_t &x1, uint16_t &y1, uint16_t &x2, uint16_t &y2)
{
	x1 = readReg(RA8875_HSAW1) & 0x03;
	x1 = (x1 << 8) | readReg(RA8875_HSAW0);

	y1 = readReg(RA8875_VSAW1) & 0x01;
	y1 = (y1 << 8) | readReg(RA8875_VSAW0);

	x2 = readReg(RA8875_HEAW1) & 0x03;
	x2 = (x2 << 8) | readReg(RA8875_HEAW0);

	y2 = readReg(RA8875_VEAW1) & 0x01;
	y2 = (y2 << 8) | readReg(RA8875_VEAW0);
}

// Change the beam scan direction on display
// Parameters:
// invertH:true(inverted),false(normal) horizontal
// invertV:true(inverted),false(normal) vertical
// used on screen rotation
void FishinoRA8875Class::setScanDirection(bool invertH, bool invertV)
{
	invertH == true ? _DPCRReg |= (1 << 3) : _DPCRReg &= ~(1 << 3);
	invertV == true ? _DPCRReg |= (1 << 2) : _DPCRReg &= ~(1 << 2);
	writeReg(RA8875_DPCR,_DPCRReg);
}

// Change memory write direction
// Parameters:
// dir the direction, 0..3
// 0 : L->R, then T->D
// 1 : R->L, then T->D
// 2 : T->D, then L->R
// 3 : D->T, then L->R
void FishinoRA8875Class::setWriteDirection(uint8_t dir)
{
	// clear direction and non-auto increment bits
	_MWCR0Reg &= ~(0x0f);
	
	switch(dir)
	{
		case 0:
		default:
			break;
		case 1:
			_MWCR0Reg |= 0b0100;
			break;
		case 2:
			_MWCR0Reg |= 0b1000;
			break;
		case 3:
			_MWCR0Reg |= 0b1100;
			break;
	}
	writeReg(RA8875_MWCR0,_MWCR0Reg);
}


// select destination of memory writes
// Parameters:
// d : destination
void FishinoRA8875Class::writeTo(enum RA8875writes d)
{
	uint8_t temp = readReg(RA8875_MWCR1);

	switch (d)
	{
		case L1:
			// Clear bits 3 and 2
			temp &= ~((1<<3) | (1<<2));

			//clear bit 0
			temp &= ~(1 << 0);

			_currentLayer = 0;
			break;
		case L2:
			// Clear bits 3 and 2
			temp &= ~((1<<3) | (1<<2));

			//bit set 0
			temp |= (1 << 0);

			_currentLayer = 1;
			break;
		case CGRAM:
			//clear bit 3
			temp &= ~(1 << 3);
			//bit set 2
			temp |= (1 << 2);
			if (bitRead(_FNCR0Reg,7))
			{
				//REG[0x21] bit7 must be 0
				_FNCR0Reg &= ~(1 << 7); //clear bit 7
				writeReg(RA8875_FNCR0,_FNCR0Reg);
			}
			break;
		case PATTERN:
			//bit set 3
			temp |= (1 << 3);
			//bit set 2
			temp |= (1 << 2);
			break;
		case CURSOR:
			//bit set 3
			temp |= (1 << 3);
			//clear bit 2
			temp &= ~(1 << 2);
			break;
		default:
			break;
	}
	writeReg(RA8875_MWCR1,temp);
}

// Change the rotation of the screen (CCW)
// Parameters:
// rotation:
// 0 = default
// 1 = 90
// 2 = 180
// 3 = 270
void FishinoRA8875Class::setRotation(uint8_t rotation)
{
	//limit to the range 0-3
	_rotation = rotation % 4;
	switch (_rotation)
	{
		case 0:
			//default, connector to bottom
			_swapxy = false;
			setScanDirection(0, 0);
			setWriteDirection(0);
			#if defined(USE_RA8875_TOUCH)
				if (!_calibrated)
				{
					_tsAdcMinX = 0;  _tsAdcMinY = 0; _tsAdcMaxX = 1023;  _tsAdcMaxY = 1023;
				}
				else
				{
					_tsAdcMinX = TOUCSRCAL_XLOW; _tsAdcMinY = TOUCSRCAL_YLOW; _tsAdcMaxX = TOUCSRCAL_XHIGH; _tsAdcMaxY = TOUCSRCAL_YHIGH;
				}
			#endif
	    break;
		case 1:
			//90
			_swapxy = true;
			setScanDirection(1,0);
			setWriteDirection(2);
			#if defined(USE_RA8875_TOUCH)
				if (!_calibrated)
				{
					_tsAdcMinX = 1023; _tsAdcMinY = 0; _tsAdcMaxX = 0; _tsAdcMaxY = 1023;
				}
				else
				{
					_tsAdcMinX = TOUCSRCAL_XHIGH; _tsAdcMinY = TOUCSRCAL_YLOW; _tsAdcMaxX = TOUCSRCAL_XLOW; _tsAdcMaxY = TOUCSRCAL_YHIGH;
				}
			#endif
	    break;
		case 2:
			//180
			_swapxy = false;
			setScanDirection(1,1);
			setWriteDirection(0);
			#if defined(USE_RA8875_TOUCH)
				if (!_calibrated)
				{
					_tsAdcMinX = 1023; _tsAdcMinY = 1023; _tsAdcMaxX = 0; _tsAdcMaxY = 0;
				}
				else
				{
					_tsAdcMinX = TOUCSRCAL_XHIGH; _tsAdcMinY = TOUCSRCAL_YHIGH; _tsAdcMaxX = TOUCSRCAL_XLOW; _tsAdcMaxY = TOUCSRCAL_YLOW;
				}
			#endif
	    break;
		case 3:
			//270
			_swapxy = true;
			setScanDirection(0,1);
			setWriteDirection(2);
			#if defined(USE_RA8875_TOUCH)
				if (!_calibrated)
				{
					_tsAdcMinX = 0; _tsAdcMinY = 1023; _tsAdcMaxX = 1023; _tsAdcMaxY = 0;
				}
				else
				{
					_tsAdcMinX = TOUCSRCAL_XLOW; _tsAdcMinY = TOUCSRCAL_YHIGH; _tsAdcMaxX = TOUCSRCAL_XHIGH; _tsAdcMaxY = TOUCSRCAL_YLOW;
				}
			#endif
	    break;
	}

	if (_swapxy)
	{
		_width = HEIGHT;
		_height = WIDTH;
		_FNCR1Reg |= (1 << 4);
	}
	else
	{
		_width = WIDTH;
		_height = HEIGHT;
		_FNCR1Reg &= ~(1 << 4);
	}
	writeReg(RA8875_FNCR1,_FNCR1Reg);//0.69b21
	setActiveWindow();
}
