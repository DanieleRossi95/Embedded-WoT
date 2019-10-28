#include "FishinoTftGui.h"

// press and release handlers
void FishinoTftGuiElementGroup::whenEvent(EventType eventType)
{
}

// constructor - protected - can be instantiated only by page objects
FishinoTftGuiElementGroup::FishinoTftGuiElementGroup(FishinoTftGuiElementGroup *group, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
	: FishinoTftGuiInteractiveElement(group, x, y, w, h)
{
}

// locate sub-element
FishinoTftGuiElement *FishinoTftGuiElementGroup::findSubItem(int16_t x, int16_t y, uint16_t tol)
{
	// go to local coordinates
	x -= _x;
	y -= _y;
	
	// look first without tolerance
	for(size_t i = 0; i < _elements.size(); i++)
	{
		FishinoTftGuiElement *e = _elements[i];
		if(e->isInside(x, y))
			return e;
	}
		
	// if not found, look with tolerance
	for(size_t i = 0; i < _elements.size(); i++)
	{
		FishinoTftGuiElement *e = _elements[i];
		if(e->isInside(x, y, tol))
			return e;
	}

	// still not found -- return NULL
	return NULL;
}

// draw the element onto display
void FishinoTftGuiElementGroup::paint(bool pressed, int16_t x, int16_t y, uint16_t tol)
{
	// translate origin to local coordinates
	tft().translate(_x, _y);
	
	// fast path - if not pressed, just repaint all elements
	if(!pressed)
	{
		for(size_t i = 0; i < _elements.size(); i++)
			_elements[i]->paint(false);
	}
	// long path - we shall find the pressed element
	else
	{
		// locate pressed element
		FishinoTftGuiElement *active = findSubItem(x, y, tol);

		for(size_t i = 0; i < _elements.size(); i++)
		{
			FishinoTftGuiElement *e = _elements[i];
			if(e == active)
				e->paint(true, x, y, tol);
			else
				e->paint(false);
		}
	}

	// translate origin back to global coordinates
	tft().translate(-_x, -_y);
}

// destructor
FishinoTftGuiElementGroup::~FishinoTftGuiElementGroup()
{
	for(size_t i = 0; i < _elements.size(); i++)
		delete _elements[i];
}

// check if it's an interactive element
bool FishinoTftGuiElementGroup::isInteractive(void) const
{
	for(size_t i = 0; i < _elements.size(); i++)
		if(_elements[i]->isInteractive())
			return true;
	return false;
}

// update bounding box -- used when adding elements
void FishinoTftGuiElementGroup::updateBoundingBox(void)
{
	uint16_t w = 0;
	uint16_t h = 0;
	for(size_t i = 0; i < _elements.size(); i++)
	{
		FishinoTftGuiElement *e = _elements[i];
		if(e->x() + e->w() > w)
			w = e->x() + e->w();
		if(e->y() + e->h() > h)
			h = e->y() + e->h();
	}
	_w = w;
	_h = h;
	
	// update parent bounding box
	if(_group != this)
		_group->updateBoundingBox();
}

// check if point is on element
bool FishinoTftGuiElementGroup::isInside(int16_t x, int16_t y) const
{
	// check bounding box, before
	if(x < _x || x >= _x + (int16_t)_w || y < _y || y > _y + (int16_t)_h)
		return false;

	for(size_t i = 0; i < _elements.size(); i++)
		if(_elements[i]->isInside(x - _x, y - _y))
			return true;
	return false;
}

bool FishinoTftGuiElementGroup::isInside(int16_t x, int16_t y, int16_t tol) const
{
	// check bounding box, before
	if(x < _x - tol || x >= _x + (int16_t)_w + tol || y < _y - tol || y > _y + (int16_t)_h + tol)
		return false;
	
	for(size_t i = 0; i < _elements.size(); i++)
		if(_elements[i]->isInside(x, y, tol))
			return true;
	return false;
}

// set the active page
FishinoTftGuiClass &FishinoTftGuiElementGroup::setPageImpl(const __FlashStringHelper *name)
{
	Gui().setPage(name);
	return Gui();
}

FishinoTftGuiClass &FishinoTftGuiElementGroup::setPageImpl(uint8_t idx)
{
	Gui().setPage(idx);
	return Gui();
}

FishinoTftGuiClass &FishinoTftGuiElementGroup::setPageImpl(FishinoTftGuiPage *page)
{
	Gui().setPage(page);
	return Gui();
}

// add a button
FishinoTftGuiButton &FishinoTftGuiElementGroup::ButtonImpl(int16_t x, int16_t y, uint16_t w, uint16_t h)
{
	FishinoTftGuiButton *button = new FishinoTftGuiButton(this, x, y, w, h);
	_elements.push_back(button);
	updateBoundingBox();
	return *button;
}

// add an icon
FishinoTftGuiIcon &FishinoTftGuiElementGroup::IconImpl(int16_t x, int16_t y, ICON const &icon)
{
	FishinoTftGuiIcon *icn = new FishinoTftGuiIcon(this, x, y, &icon);
	_elements.push_back(icn);
	updateBoundingBox();
	return *icn;
}

FishinoTftGuiInteractiveIcon &FishinoTftGuiElementGroup::InteractiveIconImpl(int16_t x, int16_t y, ICON const &icon)
{
	FishinoTftGuiInteractiveIcon *icn = new FishinoTftGuiInteractiveIcon(this, x, y, &icon);
	_elements.push_back(icn);
	updateBoundingBox();
	return *icn;
}

// add a slider
FishinoTftGuiSlider &FishinoTftGuiElementGroup::SliderImpl(int16_t x, int16_t y, uint16_t len, bool vert, float min, float max)
{
	FishinoTftGuiSlider *sld = new FishinoTftGuiSlider(this, x, y, len, vert, min, max);
	_elements.push_back(sld);
	updateBoundingBox();
	return *sld;
}

FishinoTftGuiSlider &FishinoTftGuiElementGroup::SliderImpl(int16_t x, int16_t y, uint16_t len, bool vert, ICON const &cursor, float min, float max)
{
	FishinoTftGuiSlider *sld = new FishinoTftGuiSlider(this, x, y, len, vert, &cursor, min, max);
	_elements.push_back(sld);
	updateBoundingBox();
	return *sld;
}

// add a label
FishinoTftGuiLabel &FishinoTftGuiElementGroup::LabelImpl(int16_t const &x, int16_t const &y, uint16_t w, uint16_t h, LabelAlign align, const __FlashStringHelper *text)
{
	FishinoTftGuiLabel *lbl = new FishinoTftGuiLabel(this, x, y, w, h, align, text);
	_elements.push_back(lbl);
	updateBoundingBox();
	return *lbl;
}

FishinoTftGuiLabel &FishinoTftGuiElementGroup::LabelImpl(int16_t const &x, int16_t const &y, LabelAlign align, const __FlashStringHelper *text)
{
	FishinoTftGuiLabel *lbl = new FishinoTftGuiLabel(this, x, y, align, text);
	_elements.push_back(lbl);
	updateBoundingBox();
	return *lbl;
}

FishinoTftGuiLabel &FishinoTftGuiElementGroup::LabelImpl(int16_t const &x, int16_t const &y, uint16_t w, uint16_t h, LabelAlign align, const char *text)
{
	FishinoTftGuiLabel *lbl = new FishinoTftGuiLabel(this, x, y, w, h, align, text);
	_elements.push_back(lbl);
	updateBoundingBox();
	return *lbl;
}

FishinoTftGuiLabel &FishinoTftGuiElementGroup::LabelImpl(int16_t const &x, int16_t const &y, LabelAlign align, const char *text)
{
	FishinoTftGuiLabel *lbl = new FishinoTftGuiLabel(this, x, y, align, text);
	_elements.push_back(lbl);
	updateBoundingBox();
	return *lbl;
}

// add a toggle button
FishinoTftGuiToggle &FishinoTftGuiElementGroup::ToggleImpl(int16_t x, int16_t y, ICON const &offIcon, ICON const &onIcon)
{
	FishinoTftGuiToggle *t = new FishinoTftGuiToggle(this, x, y, &offIcon, &onIcon);
	_elements.push_back(t);
	updateBoundingBox();
	return *t;
}

// add a level bar
FishinoTftGuiLevelBar &FishinoTftGuiElementGroup::LevelBarImpl(int16_t x, int16_t y, uint16_t thick, uint16_t len, bool vert, float min, float max)
{
	FishinoTftGuiLevelBar *l = new FishinoTftGuiLevelBar(this, x, y, thick, len, vert, min, max);
	_elements.push_back(l);
	updateBoundingBox();
	return *l;
}

// add a triangle button
FishinoTftGuiTriangleButton &FishinoTftGuiElementGroup::TriangleButtonImpl(int16_t x, int16_t y, uint16_t w, uint16_t h, ShapeDir dir, GradientDir gDir)
{
	FishinoTftGuiTriangleButton *t = new FishinoTftGuiTriangleButton(this, x, y, w, h, dir, gDir);
	_elements.push_back(t);
	updateBoundingBox();
	return *t;
}

// add a spindle
FishinoTftGuiSpindle &FishinoTftGuiElementGroup::SpindleImpl(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t hLabel)
{
	FishinoTftGuiSpindle *s = new FishinoTftGuiSpindle(this, x, y, w, h, hLabel);
	_elements.push_back(s);
	updateBoundingBox();
	return *s;
}
