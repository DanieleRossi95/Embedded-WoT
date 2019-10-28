#ifndef __FISHINOTFTGUI_FISHINOTFTGUIBUTTON_H
#define __FISHINOTFTGUI_FISHINOTFTGUIBUTTON_H

#include "FishinoTftGuiElement.h"

class FishinoTftGuiButton : public FishinoTftGuiInteractiveElement
{
	friend class FishinoTftGui;
	friend class FishinoTftGuiPage;
	friend class FishinoTftGuiElementGroup;

	private:
		
	protected:
		
		const __FlashStringHelper *_text;
		
		uint16_t _borderColor;
		uint16_t _innerColor1, _innerColor2;
		uint16_t _txtColor;
		
		uint16_t _pressedBorderColor;
		uint16_t _pressedInnerColor1, _pressedInnerColor2;
		uint16_t _pressedTxtColor;
		
		ICON const *_icon;
		
		// constructor - protected - can be instantiated only by page objects
		FishinoTftGuiButton(FishinoTftGuiElementGroup *group, int16_t x, int16_t y, uint16_t w, uint16_t h);
		
		// paint the button
		virtual void paint(bool pressed);
		
		// release handler
		virtual void whenEvent(EventType eventType);
		
	public:
		
		// destructor
		virtual ~FishinoTftGuiButton();
		
		// set text on button
		FishinoTftGuiButton &Text(const __FlashStringHelper *txt);

		// set icon on button
		FishinoTftGuiButton &Icon(ICON const &icon);

		// set/clear the handler
		FishinoTftGuiButton &setHandler(FishinoTftGuiHandler handler) { return (FishinoTftGuiButton &)FishinoTftGuiInteractiveElement::setHandler(handler); }
		FishinoTftGuiButton &clearHandler(void) { return (FishinoTftGuiButton &)FishinoTftGuiInteractiveElement::clearHandler(); }
		
		// get current element
		FishinoTftGuiButton &getElement(FishinoTftGuiButton *&me) { me = this; return *this; }
};

#endif
