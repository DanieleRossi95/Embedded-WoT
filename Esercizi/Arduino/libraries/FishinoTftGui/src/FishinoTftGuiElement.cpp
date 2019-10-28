#include "FishinoTftGui.h"

// constructor
FishinoTftGuiElement::FishinoTftGuiElement(FishinoTftGuiElementGroup *group, int16_t x, int16_t y, uint16_t w, uint16_t h)
{
	_group = group;
	_x = x;
	_y = y;
	_w = w;
	_h = h;
	_changed = false;
	_repaintBackground = false;
}

// destructor
FishinoTftGuiElement::~FishinoTftGuiElement()
{
}

// get drawing and touch interfaces
FishinoGFX &FishinoTftGuiElement::tft()
{
	return Page().tft();
}

FishinoTouch &FishinoTftGuiElement::touch()
{
	return Page().touch();
}

// get gui object
FishinoTftGuiClass &FishinoTftGuiElement::Gui(void)
{
	return Page().Gui();
}

// get page object
FishinoTftGuiPage &FishinoTftGuiElement::Page(void)
{
	if(_group->isPage())
		return *(FishinoTftGuiPage *)_group;
	return _group->Page();
}

// get/create a named page
FishinoTftGuiPage &FishinoTftGuiElement::Page(const __FlashStringHelper *name)
{
	return Gui().Page(name);
}

//////////////////////////////////////////////////////////////////////////////////////////////

FishinoTftGuiInteractiveElement::FishinoTftGuiInteractiveElement(FishinoTftGuiElementGroup *group, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
	: FishinoTftGuiElement(group, x, y, w, h)
{
	_handler = NULL;
}

// destructor
FishinoTftGuiInteractiveElement::~FishinoTftGuiInteractiveElement()
{
}
