#ifndef __FISHINOTFTGUI_FISHINOTFTGUILABEL_H
#define __FISHINOTFTGUI_FISHINOTFTGUILABEL_H

#include "FishinoTftGuiElement.h"

class FishinoTftGuiLabel : public FishinoTftGuiElement
{
	friend class FishinoTftGuiClass;
	friend class FishinoTftGuiPage;
	friend class FishinoTftGuiElementGroup;

	private:

	protected:
		
		char *_mText;
		
		LabelAlign _align;
		
		// label bounding box
		int16_t _xBBox, _yBBox;
		uint16_t _wBBox, _hBBox;
		
		// previous bounding box, to erase
		int16_t _prevXBBox, _prevYBBox;
		uint16_t _prevWBBox, _prevHBBox;
		
		uint16_t _txtColor;
		uint16_t _txtBackground;
		bool _paintBackground;
		
		// calculate elements positions
		// insertion point and bounding box
		void calcPositions(int16_t &txtX, int16_t &txtY);

		// paint the icon
		virtual void paint(bool pressed);
		
		// initialization
		void init(LabelAlign align, const char *text, bool fText);
		
		// constructors
		FishinoTftGuiLabel(FishinoTftGuiElementGroup *group, int16_t x, int16_t y, LabelAlign align, const __FlashStringHelper *text);
		FishinoTftGuiLabel(FishinoTftGuiElementGroup *group, int16_t x, int16_t y, uint16_t w, uint16_t h, LabelAlign align, const __FlashStringHelper *text);

		FishinoTftGuiLabel(FishinoTftGuiElementGroup *group, int16_t x, int16_t y, LabelAlign align, const char *text);
		FishinoTftGuiLabel(FishinoTftGuiElementGroup *group, int16_t x, int16_t y, uint16_t w, uint16_t h, LabelAlign align, const char *text);
	public:

		// destructor
		~FishinoTftGuiLabel();

		// get position and size
		virtual int16_t x(void) const { return _xBBox; }
		virtual int16_t y(void) const { return _yBBox; }
		virtual uint16_t w(void) const { return _wBBox; }
		virtual uint16_t h(void) const { return _hBBox; }
		
		// get previous position and size
		// useful to erase background of shrinked elements
		virtual int16_t prevX(void) const { return _prevXBBox; }
		virtual int16_t prevY(void) const { return _prevYBBox; }
		virtual uint16_t prevW(void) const { return _prevWBBox; }
		virtual uint16_t prevH(void) const { return _prevHBBox; }

		// replace the label
		FishinoTftGuiLabel &setText(const char *text);
		FishinoTftGuiLabel &setText(const __FlashStringHelper *text);
		
		// get the text
		const char *getText(void) const { return _mText; }
		
		// change text color
		FishinoTftGuiLabel &setTextColor(uint16_t color);
		
		// get text color
		uint16_t getTextColor(void) const { return _txtColor; }
		
		// set text background
		FishinoTftGuiLabel &setTextBackground(uint16_t background);
		FishinoTftGuiLabel &noTextBackground(void);
		
		// get text background
		uint16_t getTextBackground(void) const { return _txtBackground; }
		bool hasTextBackground(void) const { return _paintBackground; }

		// get current element
		FishinoTftGuiLabel &getElement(FishinoTftGuiLabel *&me) { me = this; return *this; }
};

#endif
