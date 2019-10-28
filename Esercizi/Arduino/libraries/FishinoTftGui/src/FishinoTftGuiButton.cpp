#include "FishinoTftGui.h"

// constructor
FishinoTftGuiButton::FishinoTftGuiButton(FishinoTftGuiElementGroup *group, int16_t x, int16_t y, uint16_t w, uint16_t h)
	: FishinoTftGuiInteractiveElement(group, x, y, w, h)
{
	_borderColor = 0x4a49;
	_innerColor1 = 0x634e;
	_innerColor2 = 0x2945;
	_txtColor = 0xffff;

	_pressedInnerColor1 = 0x2945;
	_pressedInnerColor2 = 0x634e;
	_pressedBorderColor = 0x8c71;
	_pressedTxtColor = 0xffff;
	
	_text = NULL;
	_icon = NULL;
}

// destructor
FishinoTftGuiButton::~FishinoTftGuiButton()
{
}

// paint the button
void FishinoTftGuiButton::paint(bool pressed)
{
	if(pressed)
	{
		tft().gradientRoundRectHorizontal(_x, _y, _w, _h, 5, _pressedInnerColor1, _pressedInnerColor2);
	
		// draw border
		tft().drawRoundRect(_x, _y, _w, _h, 5, _pressedBorderColor);
		tft().drawRoundRect(_x + 1, _y + 1, _w - 2, _h - 2, 4, _pressedBorderColor);
	}
	else
	{
		tft().gradientRoundRectHorizontal(_x, _y, _w, _h, 5, _innerColor1, _innerColor2);
	
		// draw border
		tft().drawRoundRect(_x, _y, _w, _h, 5, _borderColor);
		tft().drawRoundRect(_x + 1, _y + 1, _w - 2, _h - 2, 4, _borderColor);
	}


	// point for text/icon start
	int16_t x = _x + 3;
	uint16_t w = _w - 6;

	// paint the icon, if any
	if(_icon)
	{
#ifdef __AVR__
		int16_t y = _y + (_h - pgm_read_word(&_icon->height)) / 2;
#else
		int16_t y = _y + (_h - _icon->height) / 2;
#endif
		tft().drawBitmap(*_icon, x, y);
#ifdef __AVR__
		x += pgm_read_word(&_icon->width) + 1;
		w -= pgm_read_word(&_icon->width) + 1;
#else
		x += _icon->width + 1;
		w -= _icon->width + 1;
#endif
	}

	// paint the text
	if(!_text)
		return;
	
	// trim the text to fit the button
	char *s = strdup(_text);
	size_t sLen = strlen(s);
	if(!sLen)
	{
		free(s);
		return;
	}
	uint16_t txtW, txtH;
	while(sLen)
	{
		int16_t x1 = 0, y1 = 0;
		tft().getTextBounds(s, 0, 0, &x1, &y1, &txtW, &txtH);
		if(txtW < w)
			break;
		sLen--;
		s[sLen] = 0;
	}
	if(!sLen)
	{
		free(s);
		return;
	}

	int16_t sdummy;
	uint16_t udummy;
	int16_t y;
	tft().getTextBounds("Ay", 0, 0, &sdummy, &y, &udummy, &txtH);
	
	
	x = x + (w - txtW) / 2;
//	uint16_t y = _y + (_h + txtH) / 2;
	y = _y + (_h - y) / 2;

	tft().setCursor(x, y);
	tft().setTextColor(pressed ? _pressedTxtColor : _txtColor);
	tft().print(s);
	free(s);
}

// release handler
void FishinoTftGuiButton::whenEvent(EventType eventType)
{
	if(_handler && (eventType == EventType::RELEASE))
		_handler(*this, eventType);
}
		
// set text on button
FishinoTftGuiButton &FishinoTftGuiButton::Text(const __FlashStringHelper *txt)
{
	_text = txt;
	return *this;
}

// set icon on button
FishinoTftGuiButton &FishinoTftGuiButton::Icon(ICON const &icon)
{
	_icon = &icon;
	return *this;
}
