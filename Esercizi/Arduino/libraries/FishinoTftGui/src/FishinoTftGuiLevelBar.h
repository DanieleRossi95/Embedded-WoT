#ifndef __FISHINOTFTGUI_FISHINOTFTGUILEVELBAR_H
#define __FISHINOTFTGUI_FISHINOTFTGUILEVELBAR_H

#include "FishinoTftGuiElement.h"

class FishinoTftGuiLevelBar : public FishinoTftGuiElement
{
	friend class FishinoTftGui;
	friend class FishinoTftGuiPage;
	friend class FishinoTftGuiElementGroup;

	private:

	protected:

		// level value
		float _val;
		
		// slider minimum and maximum values
		float _min, _max;
		
		// vertical/horizontal flag
		bool _vert;
		
		// colors
		uint16_t _innerColor1, _innerColor2, _borderColor, _levelColor1, _levelColor2, _levelBorderColor;
		
		// paint the button
		virtual void paint(bool pressed);
		
		// constructor
		FishinoTftGuiLevelBar(FishinoTftGuiElementGroup *group, uint16_t x, uint16_t y, uint16_t thick, uint16_t len, bool vert, float min, float max);

	public:

		// destructor
		~FishinoTftGuiLevelBar();
		
		// set bar value
		FishinoTftGuiLevelBar &setValue(float val);
		
		// get the value
		float getValue(void) { return _val; }

		// get current element
		FishinoTftGuiLevelBar &getElement(FishinoTftGuiLevelBar *&me) { me = this; return *this; }
};

#endif
