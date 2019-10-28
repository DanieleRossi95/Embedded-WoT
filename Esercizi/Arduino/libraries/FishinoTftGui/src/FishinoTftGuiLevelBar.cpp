#include "FishinoTftGui.h"

// constructor
FishinoTftGuiLevelBar::FishinoTftGuiLevelBar(FishinoTftGuiElementGroup *group, uint16_t x, uint16_t y, uint16_t thick, uint16_t len, bool vert, float min, float max)
	: FishinoTftGuiElement(group, x, y, vert ? thick : len, vert ? len : thick)
{
	_vert = vert;
	_min = min;
	_max = max;
	_val = (min + max) / 2;
	
	_innerColor1		= 0x634e;
	_innerColor2		= 0x2945;
	_borderColor		= 0x4a49;
	_levelColor1		= 0xbed6;
	_levelColor2		= 0x36a9;
	_levelBorderColor	= 0x36a9;
}

// destructor
FishinoTftGuiLevelBar::~FishinoTftGuiLevelBar()
{
}

// paint the button
void FishinoTftGuiLevelBar::paint(bool pressed)
{
	if(_vert)
	{
		// paint inner side
		tft().gradientRectHorizontal(_x, _y, _w, _h, _innerColor1, _innerColor2);
		
		// calculate bar length
		uint16_t len = _h - 8;
		len = (_val - _min) / (_max - _min) * len;
		
		// paint the bar
		tft().gradientRectHorizontal(_x + 4, _y + _h - 4 - len, _w - 8, len, _levelColor1, _levelColor2);
	}
	else
	{
		// paint inner side
		tft().gradientRectVertical(_x, _y, _w, _h, _innerColor1, _innerColor2);

		// calculate bar length
		uint16_t len = _w - 8;
		len = (_val - _min) / (_max - _min) * len;
		
		// paint the bar
		tft().gradientRectVertical(_x + 4, _y + 4, len, _h - 8, _levelColor1, _levelColor2);
	}
	// paint border
	tft().drawRect(_x, _y, _w, _h, _borderColor);
	tft().drawRect(_x + 1, _y + 1, _w - 2, _h - 2, _borderColor);
}

// set bar value
FishinoTftGuiLevelBar &FishinoTftGuiLevelBar::setValue(float val)
{
	_val = val;
	if(_val < _min)
		_val = _min;
	if(_val > _max)
		_val = _max;
	_changed = true;
	return *this;
}
