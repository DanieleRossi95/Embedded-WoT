#include "FishinoGFX.h"

// translate origin
void FishinoGFX::translate(int16_t dx, int16_t dy)
{
	_xOrg += dx;
	_yOrg += dy;
}

// reset the origin
void FishinoGFX::resetOrigin(void)
{
	_xOrg = _yOrg = 0;
}

uint8_t FishinoGFX::getRotation(void) const
{
	return _rotation;
}

void FishinoGFX::setRotation(uint8_t x)
{
	_rotation = (x & 3);
	switch (_rotation)
	{
		case 0:
		case 2:
			_width  = WIDTH;
			_height = HEIGHT;
			break;
		case 1:
		case 3:
			_width  = HEIGHT;
			_height = WIDTH;
			break;
	}
}

void FishinoGFX::setCursor0(int16_t x, int16_t y)
{
	_cursorX = x;
	_cursorY = y;
}

void FishinoGFX::getCursor0(int16_t &x, int16_t &y) const
{
	x = _cursorX;
	y = _cursorY;
}

int16_t FishinoGFX::getCursorX0(void) const
{
	return _cursorX;
}

int16_t FishinoGFX::getCursorY0(void) const
{
	return _cursorY;
}

