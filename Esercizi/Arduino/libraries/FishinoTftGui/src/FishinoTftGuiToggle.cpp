#include "FishinoTftGui.h"

// constructor
FishinoTftGuiToggle::FishinoTftGuiToggle(FishinoTftGuiElementGroup *group, int16_t x, int16_t y, ICON const *onIcon, ICON const *offIcon)
#ifdef __AVR__
	: FishinoTftGuiInteractiveElement(group, x, y, pgm_read_word(&onIcon->width), pgm_read_word(&onIcon->height))
#else
	: FishinoTftGuiInteractiveElement(group, x, y, onIcon->width, onIcon->height)
#endif
{
	_onIcon = onIcon;
	_offIcon = offIcon;
	_on = false;
}

// destructor
FishinoTftGuiToggle::~FishinoTftGuiToggle()
{
}

// paint the icon
void FishinoTftGuiToggle::paint(bool pressed)
{
	tft().drawBitmap(_on ? *_onIcon : *_offIcon, _x, _y);
}

// release handler
void FishinoTftGuiToggle::whenEvent(EventType eventType)
{
	if(eventType == EventType::PRESS)
	{
		_on = !_on;
		_changed = true;
		if(_handler)
			_handler(*this, eventType);
	}
}

// replace the icon
FishinoTftGuiToggle &FishinoTftGuiToggle::setOnIcon(ICON const &i)
{
	_onIcon = &i;
	_changed = true;
	_repaintBackground = true;
	return *this;
}

// replace the icon
FishinoTftGuiToggle &FishinoTftGuiToggle::setOffIcon(ICON const &i)
{
	_offIcon = &i;
	_changed = true;
	_repaintBackground = true;
	return *this;
}

// set, reset and toggle
FishinoTftGuiToggle &FishinoTftGuiToggle::setOn(void)
{
	if(_on)
		return *this;
	_on = true;
	_changed = true;
	_repaintBackground = true;
	return *this;
}

FishinoTftGuiToggle &FishinoTftGuiToggle::setOff(void)
{
	if(!_on)
		return *this;
	_on = false;
	_changed = true;
	_repaintBackground = true;
	return *this;
}

FishinoTftGuiToggle &FishinoTftGuiToggle::toggle(void)
{
	_on = !_on;
	_changed = true;
	_repaintBackground = true;
	return *this;
}
