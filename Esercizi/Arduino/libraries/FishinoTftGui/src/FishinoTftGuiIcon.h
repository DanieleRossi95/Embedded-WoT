#ifndef __FISHINOTFTGUI_FISHINOTFTGUIICON_H
#define __FISHINOTFTGUI_FISHINOTFTGUIICON_H

#include "FishinoTftGuiElement.h"

class FishinoTftGuiIcon : public FishinoTftGuiElement
{
	friend class FishinoTftGuiClass;
	friend class FishinoTftGuiPage;
	friend class FishinoTftGuiElementGroup;

	private:

	protected:
		
		// the icon
		ICON const *_icon;

		// constructor
		FishinoTftGuiIcon(FishinoTftGuiElementGroup *group, int16_t x, int16_t y, ICON const *icon);

		// paint the icon
		virtual void paint(bool pressed);
		
	public:

		// destructor
		~FishinoTftGuiIcon();

		// replace the icon
		FishinoTftGuiIcon &setIcon(ICON const &i);
		
		// get the icon
		ICON const *getIcon(void) const { return _icon; }

		// get current element
		FishinoTftGuiIcon &getElement(FishinoTftGuiIcon *&me) { me = this; return *this; }
};

class FishinoTftGuiInteractiveIcon : public FishinoTftGuiInteractiveElement
{
	friend class FishinoTftGuiClass;
	friend class FishinoTftGuiPage;
	friend class FishinoTftGuiElementGroup;

	private:

	protected:

		// the icons
		ICON const *_icon;
		ICON const *_pressedIcon;

		// constructor
		FishinoTftGuiInteractiveIcon(FishinoTftGuiElementGroup *group, int16_t x, int16_t y, ICON const *icon, ICON const *pressedIcon = NULL);

		// paint the icon
		virtual void paint(bool pressed);
		
		// event handler
		virtual void whenEvent(EventType eventType);
		
	public:

		// destructor
		~FishinoTftGuiInteractiveIcon();

		// set/clear the handler
		FishinoTftGuiInteractiveIcon &setHandler(FishinoTftGuiHandler handler) { return (FishinoTftGuiInteractiveIcon &)FishinoTftGuiInteractiveElement::setHandler(handler); }
		FishinoTftGuiInteractiveIcon &clearHandler(void) { return (FishinoTftGuiInteractiveIcon &)FishinoTftGuiInteractiveElement::clearHandler(); }
		
		// replace the icon
		FishinoTftGuiInteractiveIcon &setIcon(ICON const &i);
		
		// get the icon
		ICON const *getIcon(void) const { return _icon; }

		// get current element
		FishinoTftGuiInteractiveIcon &getElement(FishinoTftGuiInteractiveIcon *&me) { me = this; return *this; }
};

#endif
