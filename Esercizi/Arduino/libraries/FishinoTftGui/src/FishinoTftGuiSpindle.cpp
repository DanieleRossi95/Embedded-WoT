#include "FishinoTftGui.h"

// constructor
FishinoTftGuiSpindle::FishinoTftGuiSpindle(FishinoTftGuiElementGroup *group, int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t hLabel)
	: FishinoTftGuiElementGroup(group, x, y, 0, 0)
{
	// apply default values
	if(hLabel < 24)
		hLabel = 24;
	if(h < 98)
		h = 98;
	
	// calculate button sizes (leave 5px between label and triangles
	uint16_t trSize = (h - hLabel - 10) / 2;
	if(w < trSize)
		w = trSize;
	
	int16_t xc = w / 2;
	int16_t yc = h / 2;
	Label(xc - w / 2, yc - hLabel / 2, w, hLabel, LabelAlign::Center, "xxxxx");
	TriangleButton(xc - trSize / 2, yc - hLabel / 2 - 5 - trSize, trSize, trSize, ShapeDir::UP, GradientDir::VERTICAL);
	TriangleButton(xc - trSize / 2, yc + hLabel / 2 + 5, trSize, trSize, ShapeDir::DOWN, GradientDir::VERTICAL);
}

// destructor
FishinoTftGuiSpindle::~FishinoTftGuiSpindle()
{
}

// @@ FIXME event handler
void FishinoTftGuiSpindle::whenEvent(EventType eventType)
{
	if(_handler && interestingEvent(eventType))
		_handler(*this, eventType);
}
