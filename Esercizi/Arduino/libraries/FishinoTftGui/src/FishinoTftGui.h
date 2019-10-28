#ifndef __FISHINOTFTGUI_FISHINOTFTGUI_H
#define __FISHINOTFTGUI_FISHINOTFTGUI_H

#include <Arduino.h>

#include <FishinoStl.h>
#include <FishinoFlash.h>
#include <FishinoGFX.h>
#include <FishinoTouch.h>

enum class LabelAlign
{
	Left = 0,
	Center = 1,
	Right = 2
};

#include "FishinoTftGuiElement.h"
#include "FishinoTftGuiElementGroup.h"
#include "FishinoTftGuiIcon.h"
#include "FishinoTftGuiLabel.h"
#include "FishinoTftGuiButton.h"
#include "FishinoTftGuiSlider.h"
#include "FishinoTftGuiToggle.h"
#include "FishinoTftGuiLevelBar.h"
#include "FishinoTftGuiTriangleButton.h"
#include "FishinoTftGuiSpindle.h"
#include "FishinoTftGuiPage.h"

class FishinoTftGuiClass
{
	friend FishinoTftGuiClass &__getFishinoTftGui(void);
	private:
		
		// touch and tft interfaces
		// to be changed later to make them more general
		FishinoTouch *_touch;
		FishinoGFX *_tft;

		// the pages
		std::vector<FishinoTftGuiPage *> _pages;
		
		// currently active page
		uint8_t _activePage;
		
		// currently touched element
		FishinoTftGuiInteractiveElement *_lastTouched;
		
		// last touch check time -- antibump!
		uint32_t _lastTouchCheckTime;
		
		// beep active flag
		bool _beepOn;
		
		// beeps the lcd embedded buzzer when display is touched
		// (if buzzer is available!)
		void _beep(void);
		
		// set the active page
		FishinoTftGuiClass &setPageImpl(const __FlashStringHelper *name);
		FishinoTftGuiClass &setPageImpl(uint8_t idx);
		FishinoTftGuiClass &setPageImpl(FishinoTftGuiPage *page);
		
	protected:

	public:

		// constructor
		FishinoTftGuiClass();

		// destructor
		~FishinoTftGuiClass();
		
		// get drawing and touch interfaces
		inline FishinoGFX &tft() { return *_tft; }
		inline FishinoTouch &touch() { return *_touch; }
		
		// enable/disable audio feedback
		FishinoTftGuiClass &beepOn(void) { _beepOn = true; return *this; }
		FishinoTftGuiClass &beepOff(void) { _beepOn = false; return *this; }
		
		// add a page
		FishinoTftGuiPage &Page(const __FlashStringHelper *name);
		
		// set the active page
		template<typename... T>auto setPage(T&&... t) -> FishinoTftGuiClass&
			{ return setPageImpl( constref_or_evaluate(static_cast<T&&>(t))... ); }

		// get active page index
		uint8_t getPageIdx(void) const { return _activePage; }
		
		// get active page
		FishinoTftGuiPage *getPage(void) { if(_activePage == 0xff) return NULL; return _pages[_activePage]; }
		
		// repaint changed elements
		void paintChanged(void);
		
		// repaint screen
		void paint(void);
		
		// start the gui interface
		FishinoTftGuiClass &begin(FishinoGFX &tft, FishinoTouch &touch);
		
		// internal loop step
		void loop(void);

};

#define FishinoTftGui __getFishinoTftGui()

extern FishinoTftGuiClass &__getFishinoTftGui(void);

#endif
