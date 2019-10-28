#ifndef __FISHINOTFTGUI_FISHINOTFTGUISLIDER_H
#define __FISHINOTFTGUI_FISHINOTFTGUISLIDER_H

#include "FishinoTftGuiElement.h"

class FishinoTftGuiSlider : public FishinoTftGuiInteractiveElement
{
	friend class FishinoTftGui;
	friend class FishinoTftGuiPage;
	friend class FishinoTftGuiElementGroup;

	private:

	protected:
		
		// slider position
		float _pos;
		
		// previous position, used when dragging
		float _prevPos;
		
		// slider minimum and maximum values
		float _min, _max;
		
		// vertical/horizontal flag
		bool _vert;
		
		ICON const *_cursorIcon;
		
		// slider total length
		uint16_t _len;
		
		// colors
		uint16_t _innerColor, _borderColor, _cursorColor, _cursorBorderColor;


		// constructor - protected - can be instantiated only by page objects
		FishinoTftGuiSlider(FishinoTftGuiElementGroup *group, int16_t x, int16_t y, uint16_t len, bool vert, ICON const *cursor, float min, float max);
		FishinoTftGuiSlider(FishinoTftGuiElementGroup *group, int16_t x, int16_t y, uint16_t len, bool vert, float min, float max);
		
		// calculate cursor position
		void cursorPos(float pos, int16_t &x, int16_t &y);
		
		// paint the button
		virtual void paint(bool pressed);
		
		// event handler
		virtual void whenEvent(EventType eventType);
		
	public:

		// constructor
		FishinoTftGuiSlider();

		// destructor
		~FishinoTftGuiSlider();

		// set/clear the handler
		FishinoTftGuiSlider &setHandler(FishinoTftGuiHandler handler) { return (FishinoTftGuiSlider &)FishinoTftGuiInteractiveElement::setHandler(handler); }
		FishinoTftGuiSlider &clearHandler(void) { return (FishinoTftGuiSlider &)FishinoTftGuiInteractiveElement::clearHandler(); }
		
		// get current element
		FishinoTftGuiSlider &getElement(FishinoTftGuiSlider *&me) { me = this; return *this; }
		
		// set cursor's icon
		FishinoTftGuiSlider &setCursorIcon(ICON const &i);
		FishinoTftGuiSlider &clearCursorIcon(ICON const &i);
		
		// get element position, minimum and maximum
		float getPos(void) const { return _pos; }
		float getMin(void) const { return _min; }
		float getMax(void) const { return _max; }
		
		// set element position
		FishinoTftGuiSlider &setPos(float pos);
};

#endif
