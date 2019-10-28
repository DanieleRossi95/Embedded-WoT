#ifndef __FISHINOTFTGUI_FISHINOTFTGUITRIANGLEBUTTON_H
#define __FISHINOTFTGUI_FISHINOTFTGUITRIANGLEBUTTON_H

#include "FishinoTftGuiElement.h"

class FishinoTftGuiTriangleButton : public FishinoTftGuiInteractiveElement
{
	friend class FishinoTftGui;
	friend class FishinoTftGuiPage;
	friend class FishinoTftGuiElementGroup;

	private:

	protected:
		
		ShapeDir _dir;
		GradientDir _gDir;

		uint16_t _borderColor;
		uint16_t _innerColor1, _innerColor2;
		
		uint16_t _pressedBorderColor;
		uint16_t _pressedInnerColor1, _pressedInnerColor2;

		// constructor
		FishinoTftGuiTriangleButton(FishinoTftGuiElementGroup *group, int16_t x, int16_t y, uint16_t w, uint16_t h, ShapeDir dir, GradientDir gDir);

		// paint the button
		virtual void paint(bool pressed);
		
		// event handler
		virtual void whenEvent(EventType eventType);
		
	public:

		// destructor
		~FishinoTftGuiTriangleButton();

		// set/clear the handler
		FishinoTftGuiTriangleButton &setHandler(FishinoTftGuiHandler handler) { return (FishinoTftGuiTriangleButton &)FishinoTftGuiInteractiveElement::setHandler(handler); }
		FishinoTftGuiTriangleButton &clearHandler(void) { return (FishinoTftGuiTriangleButton &)FishinoTftGuiInteractiveElement::clearHandler(); }
		
		// get current element
		FishinoTftGuiTriangleButton &getElement(FishinoTftGuiTriangleButton *&me) { me = this; return *this; }
};

#endif
