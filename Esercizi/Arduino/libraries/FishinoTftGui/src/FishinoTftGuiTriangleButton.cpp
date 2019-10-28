#include "FishinoTftGui.h"

// constructor
FishinoTftGuiTriangleButton::FishinoTftGuiTriangleButton(FishinoTftGuiElementGroup *group, int16_t x, int16_t y, uint16_t w, uint16_t h, ShapeDir dir, GradientDir gDir)
	: FishinoTftGuiInteractiveElement(group, x, y, w, h)
{
	_dir = dir;
	_gDir = gDir;
	
	_borderColor = 0x4a49;
	_innerColor1 = 0x634e;
	_innerColor2 = 0x2945;

	_pressedInnerColor1 = 0x2945;
	_pressedInnerColor2 = 0x634e;
	_pressedBorderColor = 0x8c71;
}

// destructor
FishinoTftGuiTriangleButton::~FishinoTftGuiTriangleButton()
{
}

// paint the button
void FishinoTftGuiTriangleButton::paint(bool pressed)
{
	uint16_t c1, c2, bc;
	uint16_t wm = _w / 2;
	uint16_t hm = _h / 2;
	if(pressed)
	{
		c1 = _innerColor1;
		c2 = _innerColor2;
		bc = _borderColor;
	}
	else
	{
		c1 = _pressedInnerColor1;
		c2 = _pressedInnerColor2;
		bc = _pressedBorderColor;
	}
	int16_t x1, x2, x3, y1, y2, y3;
	switch(_dir)
	{
		case ShapeDir::UP:
		default:
			x1 = _x + wm;
			y1 = _y;
			x2 = _x;
			y2 = _y + _h;
			x3 = _x + _w;
			y3 = y2;
			break;
		case ShapeDir::DOWN:
			x1 = _x + wm;
			y1 = _y + _h;
			x2 = _x + _w;
			y2 = _y;
			x3 = _x;
			y3 = y2;
			break;
		case ShapeDir::LEFT:
			x1 = _x;
			y1 = _y + hm;
			x2 = _x + _w;
			y2 = _y + _h;
			x3 = x2;
			y3 = _y;
			break;
		case ShapeDir::RIGHT:
			x1 = _x + _w;
			y1 = _y + hm;
			x2 = _x;
			y2 = _y;
			x3 = x2;
			y3 = _y + _h;
			break;
	}
	
	if(_gDir == GradientDir::VERTICAL)
		tft().gradientTriangleVertical(x1, y1, x2, y2, x3, y3, c1, c2);
	else
		tft().gradientTriangleHorizontal(x1, y1, x2, y2, x3, y3, c1, c2);
	tft().drawTriangle(x1, y1, x2, y2, x3, y3, bc);
}

// event handler
void FishinoTftGuiTriangleButton::whenEvent(EventType eventType)
{
	if(_handler && interestingEvent(eventType))
		_handler(*this, eventType);
}
