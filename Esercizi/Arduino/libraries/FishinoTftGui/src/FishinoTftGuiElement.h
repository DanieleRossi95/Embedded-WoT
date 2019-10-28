#ifndef __FISHINOTFTGUI_FISHINOTFTGUIELEMENT_H
#define __FISHINOTFTGUI_FISHINOTFTGUIELEMENT_H

#include <Arduino.h>

class FishinoTftGuiClass;
class FishinoTftGuiPage;
class FishinoTftGuiElementGroup;

enum class ShapeDir
{
	UP, DOWN, LEFT, RIGHT
};

enum class GradientDir
{
	VERTICAL, HORIZONTAL
};

enum class EventType
{
	DUMMY,		// dummy event - means no event
	PRESS,		// element has been touched
	KEEP,		// touch on same element since last touch
	DRAG,		// touch position changed on same element
	ABANDONED,	// touch moved outside element without releasing
	RELEASE		// touch released on this element
};
	

template<typename T>auto constref_or_evaluate(T const& t) -> T const& {
	return t;
}

template<typename T>auto constref_or_evaluate(T&& t) -> decltype(static_cast<T&&>(t)()) {
	return static_cast<T&&>(t)();
}

class FishinoTftGuiInteractiveElement;
typedef void (*FishinoTftGuiHandler)(FishinoTftGuiInteractiveElement &elem, EventType eventType);

class FishinoTftGuiElement
{
	friend class FishinoTftGuiClass;
	friend class FishinoTftGuiPage;
	friend class FishinoTftGuiElementGroup;

	private:
		
	protected:
		
		// the group where the object is located
		FishinoTftGuiElementGroup *_group;
		
		// position and size
		int16_t _x, _y;
		uint16_t _w, _h;
		
		// event handler
		virtual void whenEvent(EventType eventType) {}

		// check if event shall be propagated to handler
		inline bool interestingEvent(EventType event)
		{
			return
				event == EventType::PRESS	||
				event == EventType::KEEP	||
				event == EventType::RELEASE
			;
		}
		
		// changed flag -- used to force control repaint
		// if changed externally
		bool _changed;
		
		// repaint background needed
		// useful to replace an item
		bool _repaintBackground;
		
		// constructor - protected - can be instantiated only by page objects
		FishinoTftGuiElement(FishinoTftGuiElementGroup *group, int16_t x, int16_t y, uint16_t w, uint16_t h);
		
		// draw the element onto display
		virtual void paint(bool pressed) = 0;
		
		// same function with given press point and tolerance
		// useful for group elements
		virtual void paint(bool pressed, int16_t x, int16_t y, uint16_t tol)
			{ paint(pressed); }
		
	public:
		
		// destructor
		virtual ~FishinoTftGuiElement();
		
		// get drawing and touch interfaces
		FishinoGFX &tft();
		FishinoTouch &touch();
		
		// check if it's an interactive element
		virtual bool isInteractive(void) const { return false; }

		// get position and size
		virtual int16_t x(void) const { return _x; }
		virtual int16_t y(void) const { return _y; }
		virtual uint16_t w(void) const { return _w; }
		virtual uint16_t h(void) const { return _h; }
		
		// get previous position and size
		// useful to erase background of shrinked elements
		virtual int16_t prevX(void) const { return _x; }
		virtual int16_t prevY(void) const { return _y; }
		virtual uint16_t prevW(void) const { return _w; }
		virtual uint16_t prevH(void) const { return _h; }
		
		// check if point is on element
		virtual bool isInside(int16_t x, int16_t y) const { return x >= _x && x < _x + (int16_t)_w && y >= _y && y <= _y + (int16_t)_h; }
		virtual bool isInside(int16_t x, int16_t y, int16_t tol) const { return x >= _x - tol && x < _x + (int16_t)_w + tol && y >= _y - tol && y <= _y + (int16_t)_h + tol; }
		
		// get contaning group object (can be a page object)
		FishinoTftGuiElementGroup &operator--(int) { return *_group; }
		FishinoTftGuiElementGroup &Group(void) { return *_group; }
		
		// get gui object
		FishinoTftGuiClass &Gui(void);
		
		// get page object
		FishinoTftGuiPage &Page(void);
		
		// get/create a named page
		FishinoTftGuiPage &Page(const __FlashStringHelper *name);
};

class FishinoTftGuiInteractiveElement : public FishinoTftGuiElement
{
	friend class FishinoTftGuiClass;
	friend class FishinoTftGuiPage;
	friend class FishinoTftGuiElementGroup;

	private:
		
	protected:
		
		// constructor - protected - can be instantiated only by page objects
		FishinoTftGuiInteractiveElement(FishinoTftGuiElementGroup *group, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
		
		// the associated handler
		FishinoTftGuiHandler _handler;

		// set/clear the handler
		FishinoTftGuiInteractiveElement &setHandler(FishinoTftGuiHandler handler) { _handler = handler; return *this; }
		FishinoTftGuiInteractiveElement &clearHandler(void) { _handler = NULL; return *this; }
		
	public:
		
		// destructor
		virtual ~FishinoTftGuiInteractiveElement();
		
		// check if it's an interactive element
		virtual bool isInteractive(void) const { return true; }

};

#endif
