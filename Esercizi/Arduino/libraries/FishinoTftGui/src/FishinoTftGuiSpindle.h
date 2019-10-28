#ifndef __FISHINOTFTGUI_FISHINOTFTGUISPINDLE_H
#define __FISHINOTFTGUI_FISHINOTFTGUISPINDLE_H

#include <Arduino.h>

class FishinoTftGuiSpindle : public FishinoTftGuiElementGroup
{
	friend class FishinoTftGuiClass;
	friend class FishinoTftGuiPage;
	friend class FishinoTftGuiElementGroup;

	private:
		
	protected:
		
		// constructor
		FishinoTftGuiSpindle(FishinoTftGuiElementGroup *group, int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t hLabel);

		// event handler
		virtual void whenEvent(EventType eventType);
		
	public:

		// destructor
		~FishinoTftGuiSpindle();

		// set/clear the handler
		FishinoTftGuiSpindle &setHandler(FishinoTftGuiHandler handler) { return (FishinoTftGuiSpindle &)FishinoTftGuiInteractiveElement::setHandler(handler); }
		FishinoTftGuiSpindle &clearHandler(void) { return (FishinoTftGuiSpindle &)FishinoTftGuiInteractiveElement::clearHandler(); }
		
		// get current element
		FishinoTftGuiSpindle &getElement(FishinoTftGuiSpindle *&me) { me = this; return *this; }

		// long press reaction
		FishinoTftGuiSpindle &setLongPressReaction(uint32_t startDelay = 0, uint32_t stepDelay = 0);
};

class FishinoTftGuiIntegerSpindle : public FishinoTftGuiSpindle
{
	friend class FishinoTftGuiClass;
	friend class FishinoTftGuiPage;
	friend class FishinoTftGuiElementGroup;

	private:
		
	protected:
		
		// constructor
		FishinoTftGuiIntegerSpindle(FishinoTftGuiElementGroup *group, int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t hLabel);

		// event handler
		virtual void whenEvent(EventType eventType);
		
	public:

		// destructor
		~FishinoTftGuiIntegerSpindle();

		// set/clear the handler
		FishinoTftGuiIntegerSpindle &setHandler(FishinoTftGuiHandler handler) { return (FishinoTftGuiIntegerSpindle &)FishinoTftGuiInteractiveElement::setHandler(handler); }
		FishinoTftGuiIntegerSpindle &clearHandler(void) { return (FishinoTftGuiIntegerSpindle &)FishinoTftGuiInteractiveElement::clearHandler(); }
		
		// set current value
		FishinoTftGuiIntegerSpindle &set(int32_t val);
		FishinoTftGuiIntegerSpindle &set(int32_t min, int32_t max, int32_t val);
		FishinoTftGuiIntegerSpindle &setLimits(int32_t min, int32_t max);
		FishinoTftGuiIntegerSpindle &setStep(int32_t step);
		
		// get current element
		FishinoTftGuiIntegerSpindle &getElement(FishinoTftGuiIntegerSpindle *&me) { me = this; return *this; }
};

#endif
