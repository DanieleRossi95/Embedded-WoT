#include "FishinoTftGui.h"

// constructor
FishinoTftGuiSlider::FishinoTftGuiSlider(FishinoTftGuiElementGroup *group, int16_t x, int16_t y, uint16_t len, bool vert, ICON const *cursor, float min, float max)
#ifdef __AVR__
	: FishinoTftGuiInteractiveElement(group, x, y, vert ? pgm_read_word(&cursor->height) : len, vert ? len : pgm_read_word(&cursor->width))
#else
	: FishinoTftGuiInteractiveElement(group, x, y, vert ? cursor->height : len, vert ? len : cursor->width)
#endif
{
	_min = min;
	_max = max;
	_len = len;
	_vert = vert;
	_pos = (_min + _max) / 2;
	_prevPos = _pos;

	// colors
//	_innerColor			= 0xffdf;
	_innerColor			= 0x4a49;

//	_borderColor		= 0xb5b6;
	_borderColor		= 0x738e;
	
	_cursorColor		= 0xffdf;
	_cursorBorderColor	= 0xb5b6;
	
	_cursorIcon = cursor;
}

FishinoTftGuiSlider::FishinoTftGuiSlider(FishinoTftGuiElementGroup *group, int16_t x, int16_t y, uint16_t len, bool vert, float min, float max)
	: FishinoTftGuiInteractiveElement(group, x, y, vert ? 24 : len, vert ? len : 24)
{
	_min = min;
	_max = max;
	_len = len;
	_vert = vert;
	_pos = (_min + _max) / 2;
	_prevPos = _pos;

	// colors
//	_innerColor			= 0xffdf;
	_innerColor			= 0x4a49;

//	_borderColor		= 0xb5b6;
	_borderColor		= 0x738e;
	
	_cursorColor		= 0xffdf;
	_cursorBorderColor	= 0xb5b6;
	
	_cursorIcon = NULL;
}

// destructor
FishinoTftGuiSlider::~FishinoTftGuiSlider()
{
}

// calculate cursor position
void FishinoTftGuiSlider::cursorPos(float pos, int16_t &x, int16_t &y)
{
	int16_t rPos;
	int16_t cSize;
	if(_cursorIcon)
#ifdef __AVR__
		cSize = (_vert ? pgm_read_word(&_cursorIcon->height) : pgm_read_word(&_cursorIcon->width));
#else
		cSize = (_vert ? _cursorIcon->height : _cursorIcon->width);
#endif
	else
		cSize = 24;
	rPos = (int16_t)((_len - cSize) * (pos - _min) / (_max - _min)) + cSize / 2;

	if(_vert)
	{
		y = _y + (int16_t)_h - rPos;
		x = _x + (int16_t)_w / 2;
	}
	else
	{
		x = _x + rPos;
		y = _y + (int16_t)_h / 2;
	}
}

// paint the button
void FishinoTftGuiSlider::paint(bool pressed)
{
	int16_t cx, cy;

	uint16_t cSize;
	if(_cursorIcon)
#ifdef __AVR__
		cSize = (_vert ? pgm_read_word(&_cursorIcon->height) : pgm_read_word(&_cursorIcon->width));
#else
		cSize = (_vert ? _cursorIcon->height : _cursorIcon->width);
#endif
	else
		cSize = 24;
	
	// erase old cursor, if different position
	if(_prevPos != _pos)
	{
		cursorPos(_prevPos, cx, cy);
		if(_cursorIcon)
#ifdef __AVR__
			tft().fillRect(cx - (int16_t)pgm_read_word(&_cursorIcon->width) / 2, cy - (int16_t)pgm_read_word(&_cursorIcon->height) / 2, pgm_read_word(&_cursorIcon->width), pgm_read_word(&_cursorIcon->height), Page().getBkColor());
#else
			tft().fillRect(cx - (int16_t)_cursorIcon->width / 2, cy - (int16_t)_cursorIcon->height / 2, _cursorIcon->width, _cursorIcon->height, Page().getBkColor());
#endif
		else
			tft().fillCircle(cx, cy, cSize / 2, Page().getBkColor());
	}
	
	// draw the slider line and border
	if(_vert)
	{
		tft().fillRect(_x + 12 - 3, _y, 6, _len, _innerColor);
		tft().drawRect(_x + 12 - 3, _y, 6, _len, _borderColor);
	}
	else
	{
		tft().fillRect(_x, _y + 12 - 3, _len, 6, _innerColor);
		tft().drawRect(_x, _y + 12 - 3, _len, 6, _borderColor);
	}
	
	// draw the cursor
	cursorPos(_pos, cx, cy);
	if(_cursorIcon)
#ifdef __AVR__
		tft().drawBitmap(*_cursorIcon, cx - (int16_t)pgm_read_word(&_cursorIcon->width) / 2, cy - (int16_t)pgm_read_word(&_cursorIcon->height) / 2);
#else
		tft().drawBitmap(*_cursorIcon, cx - (int16_t)_cursorIcon->width / 2, cy - (int16_t)_cursorIcon->height / 2);
#endif
	else
	{
		tft().fillCircle(cx, cy, 12, _cursorColor);
		tft().drawCircle(cx, cy, 12, _cursorBorderColor);
	}
	
	// set previous position
	_prevPos = _pos;
}

// event handler
void FishinoTftGuiSlider::whenEvent(EventType eventType)
{
	switch(eventType)
	{
		case EventType::PRESS:
		{
			uint16_t x, y;
			touch().read(x, y);
			int16_t pos;
			if(_vert)
				pos = _y + _h - y;
			else
				pos = x - _x;
			pos -= 12;
			_prevPos = _pos;
			_pos = ((float)pos) / (_len - 24) * (_max - _min) + _min;
			if(_pos > _max)
				_pos = _max;
			if(_pos < _min)
				_pos = _min;
			_changed = true;
			if(_handler)
				_handler(*this, eventType);
			break;
		}
		
		case EventType::DRAG:
		{
			uint16_t x, y;
			touch().read(x, y);
			int16_t pos;
			if(_vert)
				pos = _y + _h - y;
			else
				pos = x - _x;
			pos -= 12;
			_prevPos = _pos;
			float newPos = ((float)pos) / (_len - 24) * (_max - _min) + _min;
			if(newPos > _max)
				newPos = _max;
			if(newPos < _min)
				newPos = _min;
			
			if(newPos != _pos)
			{
				_pos = newPos;
				_changed = true;
				if(_handler)
					_handler(*this, eventType);
			}
			break;
		}
		
		default:
			break;
	}
}

// set cursor's icon
FishinoTftGuiSlider &FishinoTftGuiSlider::setCursorIcon(ICON const &i)
{
	_cursorIcon = &i;
	if(_vert)
		_w = i.width;
	else
		_h = i.height;
	_changed = true;
	return *this;
}

FishinoTftGuiSlider &FishinoTftGuiSlider::clearCursorIcon(ICON const &i)
{
	_cursorIcon = NULL;
	if(_vert)
		_w = 24;
	else
		_h = 24;
	_changed = true;
	return *this;
}

// set element position
FishinoTftGuiSlider &FishinoTftGuiSlider::setPos(float pos)
{
	_pos = pos;
	if(_pos < _min)
		_pos = _min;
	if(_pos > _max)
		_pos = _max;
	_changed = true;
	return *this;
}
