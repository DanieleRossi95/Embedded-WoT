#include "FishinoTftGui.h"

// constructor
FishinoTftGuiPage::FishinoTftGuiPage(const __FlashStringHelper *name)
	: FishinoTftGuiElementGroup(this, 0, 0, 0, 0)
{
	_name = name;
	_gui = NULL;
}

// destructor
FishinoTftGuiPage::~FishinoTftGuiPage()
{
}

// get drawing and touch interfaces
FishinoGFX &FishinoTftGuiPage::tft()
{
	return _gui->tft();
}

FishinoTouch &FishinoTftGuiPage::touch()
{
	return _gui->touch();
}
		
// set/get background color
FishinoTftGuiPage &FishinoTftGuiPage::setBkColor(uint16_t col)
{
	_bkColor = col;
	return *this;
}

uint16_t FishinoTftGuiPage::getBkColor(void)
{
	return _bkColor;
}

// paints the whole page
void FishinoTftGuiPage::paint(void)
{
	if(!_gui)
		return;
	tft().fillScreen(_bkColor);

	for(size_t i = 0; i < _elements.size(); i++)
		_elements[i]->paint(false);
}
