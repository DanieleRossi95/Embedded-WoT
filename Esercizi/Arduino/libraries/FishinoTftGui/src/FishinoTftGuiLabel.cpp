#include "FishinoTftGui.h"

// initialization
void FishinoTftGuiLabel::init(LabelAlign align, const char *text, bool fText)
{
	_align = align;
	_txtColor = 0;
	_txtBackground = 0;
	_paintBackground = false;
	
	if(fText)
		_mText = strdup((const __FlashStringHelper *)text);
	else
		_mText = strdup(text);

	int16_t txtX, txtY;
	calcPositions(txtX, txtY);

	_prevXBBox = _xBBox;
	_prevYBBox = _yBBox;
	_prevWBBox = _wBBox;
	_prevHBBox = _hBBox;
}

// constructors
FishinoTftGuiLabel::FishinoTftGuiLabel(FishinoTftGuiElementGroup *group, int16_t x, int16_t y, LabelAlign align, const __FlashStringHelper *text)
	: FishinoTftGuiElement(group, x, y, 0, 0)
{
	init(align, (const char *)text, true);
}

FishinoTftGuiLabel::FishinoTftGuiLabel(FishinoTftGuiElementGroup *group, int16_t x, int16_t y, uint16_t w, uint16_t h, LabelAlign align, const __FlashStringHelper *text)
	: FishinoTftGuiElement(group, x, y, w, h)
{
	init(align, (const char *)text, true);
}

FishinoTftGuiLabel::FishinoTftGuiLabel(FishinoTftGuiElementGroup *group, int16_t x, int16_t y, LabelAlign align, const char *text)
	: FishinoTftGuiElement(group, x, y, 0, 0)
{
	init(align, text, false);
}

FishinoTftGuiLabel::FishinoTftGuiLabel(FishinoTftGuiElementGroup *group, int16_t x, int16_t y, uint16_t w, uint16_t h, LabelAlign align, const char *text)
	: FishinoTftGuiElement(group, x, y, w, h)
{
	init(align, text, false);
}

// destructor
FishinoTftGuiLabel::~FishinoTftGuiLabel()
{
	free(_mText);
}

// calculate elements positions
void FishinoTftGuiLabel::calcPositions(int16_t &txtX, int16_t &txtY)
{
	uint16_t w, h, udummy;
	int16_t sdummy;
	
	// we center vertically on a fixed string, which has the highest values
	// for ascent and descent, so consequent labels looks aligned
//	tft().getTextBounds(_mText, 0, 0, &txtX, &txtY, &w, &h);
	tft().getTextBounds(_mText, 0, 0, &txtX, &sdummy, &w, &udummy);
	tft().getTextBounds("Ay", 0, 0, &sdummy, &txtY, &udummy, &h);
	
	// apply align
	if(_h)
//		txtY = _y - txtY + (_h - h) / 2;
		// center upper text part (from baseline to top)
		// as centering the whole stuff looks worse
		txtY = _y + (_h - txtY) / 2;
	else
		txtY = _y - txtY;
	switch(_align)
	{
		case LabelAlign::Left:
		default:
			txtX = _x;
			break;
			
		case LabelAlign::Right:
			txtX = _x - w;
			if(_w)
				txtX += _w;
			break;
			
		case LabelAlign::Center:
			txtX = _x - w / 2;
			if(_w)
				txtX +=  _w / 2;
			break;
	}
	
	// store current bb
	_xBBox = _x;
	if(!_w)
	{
		if(_align == LabelAlign::Center)
			_xBBox -= w / 2;
		else if(_align == LabelAlign::Right)
			_xBBox -= w;
	}
	_yBBox = _y;
	_wBBox = _w ? _w : w;
	_hBBox = _h ? _h : h;
}

// paint the icon
void FishinoTftGuiLabel::paint(bool pressed)
{
	// if background clear is requested, do it
	// before recalculating box size
/*
	if(_repaintBackground)
	{
		Serial << "Repainting " << _prevXBBox << ", " << _prevYBBox << ", " << _prevWBBox << ", " << _prevHBBox << "\n";
		tft().fillRect(_prevXBBox, _prevYBBox, _prevWBBox, _prevHBBox, Page().getBkColor());
		_repaintBackground = false;
	}
*/
	
	if(!_mText)
		return;
	
	// get text bounding box and insertion pos
	int16_t txtX, txtY;
	calcPositions(txtX, txtY);
	
	_prevXBBox = _xBBox;
	_prevYBBox = _yBBox;
	_prevWBBox = _wBBox;
	_prevHBBox = _hBBox;

	// paint background if requested
	if(_paintBackground)
		tft().fillRect(_xBBox, _yBBox, _wBBox, _hBBox, _txtBackground);
/*
	else
		tft().fillRect(_xBBox, _yBBox, _wBBox, _hBBox, Page().getBkColor());
*/
	
	tft().setCursor(txtX, txtY);
	tft().setTextColor(_txtColor);
	tft().setTextWrap(false);
	tft().print(_mText);
}

// replace the label
FishinoTftGuiLabel &FishinoTftGuiLabel::setText(const char *text)
{
	if(_mText)
		free(_mText);
	_mText = strdup(text);
	_changed = true;
	_repaintBackground = true;
	return *this;
}

FishinoTftGuiLabel &FishinoTftGuiLabel::setText(const __FlashStringHelper *text)
{
	if(_mText)
		free(_mText);
	_mText = strdup(text);
	_changed = true;
	_repaintBackground = true;
	return *this;
}

// change text color
FishinoTftGuiLabel &FishinoTftGuiLabel::setTextColor(uint16_t color)
{
	_txtColor = color;
	_changed = true;
	_repaintBackground = true;
	return *this;
}

// set text background
FishinoTftGuiLabel &FishinoTftGuiLabel::setTextBackground(uint16_t background)
{
	_txtBackground = background;
	_paintBackground = true;
	_changed = true;
	_repaintBackground = true;
	return *this;
}

FishinoTftGuiLabel &FishinoTftGuiLabel::noTextBackground(void)
{
	_txtBackground = 0;
	_paintBackground = false;
	_changed = true;
	_repaintBackground = true;
	return *this;
}

