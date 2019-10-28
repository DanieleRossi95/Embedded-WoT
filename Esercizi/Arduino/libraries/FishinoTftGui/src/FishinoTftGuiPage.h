// Library : 'FishinoTftGui' -- File : 'FishinoTftGuiPage.h'
// Created by FishIDE application 
#ifndef __FISHINOTFTGUI_FISHINOTFTGUIPAGE_H
#define __FISHINOTFTGUI_FISHINOTFTGUIPAGE_H

#include <Arduino.h>

class FishinoTftGuiClass;

class FishinoTftGuiPage : public FishinoTftGuiElementGroup
{
	friend class FishinoTftGuiClass;
	
	private:
		
		// the gui object
		FishinoTftGuiClass *_gui;
		
		// the page name (used to display it)
		const __FlashStringHelper *_name;
		
		// background color
		uint16_t _bkColor;
		
		// last clicked element in page, if any
		FishinoTftGuiInteractiveElement *_lastClickedElement;
		
		// constructor
		FishinoTftGuiPage(const __FlashStringHelper *name);
		
	protected:
		
	public:

		// destructor
		virtual ~FishinoTftGuiPage();
		
		// check if this group is a page
		virtual bool isPage(void) const { return true; }

		// get contaning page
		FishinoTftGuiClass &operator--(int) { return *_gui; }
		
		// get drawing and touch interfaces
		FishinoGFX &tft();
		FishinoTouch &touch();
		
		// set/get background color
		FishinoTftGuiPage &setBkColor(uint16_t col);
		uint16_t getBkColor(void);
		
		// paints the page
		void paint();
		
		// return Gui object
		FishinoTftGuiClass &Gui(void) { return *_gui; }

		// get current element
		FishinoTftGuiPage  &getElement(FishinoTftGuiPage *&me) { me = this; return *this; }
};

#endif
