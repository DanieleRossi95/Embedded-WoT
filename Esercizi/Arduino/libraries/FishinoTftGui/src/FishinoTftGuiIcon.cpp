#include "FishinoTftGui.h"

// constructor
FishinoTftGuiIcon::FishinoTftGuiIcon(FishinoTftGuiElementGroup *group, int16_t x, int16_t y, ICON const *icon)
#ifdef __AVR__
	: FishinoTftGuiElement(group, x, y, pgm_read_word(&icon->width), pgm_read_word(&icon->height))
#else
	: FishinoTftGuiElement(group, x, y, icon->width, icon->height)
#endif
{
	_icon = icon;
}

// destructor
FishinoTftGuiIcon::~FishinoTftGuiIcon()
{
}

// paint the icon
void FishinoTftGuiIcon::paint(bool pressed)
{
	tft().drawBitmap(*_icon, _x, _y);
}

// replace the icon
FishinoTftGuiIcon &FishinoTftGuiIcon::setIcon(ICON const &i)
{
	_icon = &i;
	_changed = true;
	_repaintBackground = true;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////////////////

// constructor
FishinoTftGuiInteractiveIcon::FishinoTftGuiInteractiveIcon(FishinoTftGuiElementGroup *group, int16_t x, int16_t y, ICON const *icon, ICON const *pressedIcon)
#ifdef __AVR__
	: FishinoTftGuiInteractiveElement(group, x, y, pgm_read_word(&icon->width), pgm_read_word(&icon->height))
#else
	: FishinoTftGuiInteractiveElement(group, x, y, icon->width, icon->height)
#endif
{
	_icon = icon;
	_pressedIcon = pressedIcon;
}

// destructor
FishinoTftGuiInteractiveIcon::~FishinoTftGuiInteractiveIcon()
{
}

// paint the icon
void FishinoTftGuiInteractiveIcon::paint(bool pressed)
{
	if(!pressed)
		tft().drawBitmap(*_icon, _x, _y);
	else
	{
		if(_pressedIcon)
			tft().drawBitmap(*_pressedIcon, _x, _y);
		else
			tft().drawBitmapInverted(*_icon, _x, _y);
	}
}

// event handler
void FishinoTftGuiInteractiveIcon::whenEvent(EventType eventType)
{
	if(_handler && interestingEvent(eventType))
		_handler(*this, eventType);
}

// replace the icon
FishinoTftGuiInteractiveIcon &FishinoTftGuiInteractiveIcon::setIcon(ICON const &i)
{
	_icon = &i;
	_changed = true;
	_repaintBackground = true;
	return *this;
}
