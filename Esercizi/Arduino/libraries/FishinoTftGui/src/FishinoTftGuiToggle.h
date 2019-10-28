#ifndef __FISHINOTFTGUI_FISHINOTFTGUITOGGLE_H
#define __FISHINOTFTGUI_FISHINOTFTGUITOGGLE_H

#include "FishinoTftGuiElement.h"

class FishinoTftGuiToggle : public FishinoTftGuiInteractiveElement
{
	friend class FishinoTftGuiClass;
	friend class FishinoTftGuiPage;
	friend class FishinoTftGuiElementGroup;

	private:

		// the icons
		ICON const *_onIcon;
		ICON const *_offIcon;
		
		// on flag
		bool _on;

		// constructor
		FishinoTftGuiToggle(FishinoTftGuiElementGroup *group, int16_t x, int16_t y, ICON const *offIcon, ICON const *onIcon);

		// paint the icon
		virtual void paint(bool pressed);
		
		// event handler
		virtual void whenEvent(EventType eventType);
		
	public:

		// destructor
		~FishinoTftGuiToggle();

		// set/clear the handler
		FishinoTftGuiToggle &setHandler(FishinoTftGuiHandler handler) { return (FishinoTftGuiToggle &)FishinoTftGuiInteractiveElement::setHandler(handler); }
		FishinoTftGuiToggle &clearHandler(void) { return (FishinoTftGuiToggle &)FishinoTftGuiInteractiveElement::clearHandler(); }
		
		// replace the icons
		FishinoTftGuiToggle &setOnIcon(ICON const &i);
		FishinoTftGuiToggle &setOffIcon(ICON const &i);
		
		// get the icon
		ICON const *getOnIcon(void) const { return _onIcon; }
		ICON const *getOffIcon(void) const { return _offIcon; }
		
		// set, reset and toggle
		FishinoTftGuiToggle &setOn(void);
		FishinoTftGuiToggle &setOff(void);
		FishinoTftGuiToggle &toggle(void);
		
		// request state
		bool isOn(void) const { return _on; }

		// get current element
		FishinoTftGuiToggle &getElement(FishinoTftGuiToggle *&me) { me = this; return *this; }

};

#endif
