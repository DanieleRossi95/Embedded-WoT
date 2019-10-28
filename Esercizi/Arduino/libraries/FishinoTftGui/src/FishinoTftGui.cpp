#include "FishinoTftGui.h"

#include <FishinoFlash.h>

// constructor
FishinoTftGuiClass::FishinoTftGuiClass()
{
	_touch = NULL;
	_tft = NULL;
	_lastTouched = NULL;
	_beepOn = false;
	_lastTouchCheckTime = millis();
	_activePage = 0xff;
}

// destructor
FishinoTftGuiClass::~FishinoTftGuiClass()
{
	for(size_t i = 0; i < _pages.size(); i++)
		delete _pages[i];
	_pages.clear();
}

// beeps the lcd embedded buzzer when display is touched
// (if buzzer is available!)
void FishinoTftGuiClass::_beep(void)
{
	// buzzer is on I/O 9
	pinMode(9, OUTPUT);
	uint32_t tim = millis() + 100;
	while(millis() < tim)
	{
		digitalWrite(9, HIGH);
		delayMicroseconds(150);
		digitalWrite(9, LOW);
		delayMicroseconds(150);
	}
	pinMode(9, INPUT);
}

// add a page
FishinoTftGuiPage &FishinoTftGuiClass::Page(const __FlashStringHelper *name)
{
	FishinoTftGuiPage *page = new FishinoTftGuiPage(name);
	page->_gui = this;
	_pages.push_back(page);
	_activePage = _pages.size() - 1;

	return *page;
}

// set the active page
FishinoTftGuiClass &FishinoTftGuiClass::setPageImpl(const __FlashStringHelper *name)
{
//	Serial << "SetPage " << name << "\n";
	for(size_t i = 0; i < _pages.size(); i++)
		if(!strcmp(name, _pages[i]->_name))
		{
			if(_activePage != i)
				_lastTouched = NULL;
			_activePage = i;
			return *this;
		}
		return *this;
}

FishinoTftGuiClass &FishinoTftGuiClass::setPageImpl(uint8_t idx)
{
//	Serial << "SetPage " << idx << "\n";
	if(idx == _activePage)
		return *this;
	if(idx >= _pages.size())
		return *this;
	_activePage = idx;
	return *this;
}

FishinoTftGuiClass &FishinoTftGuiClass::setPageImpl(FishinoTftGuiPage *page)
{
//	Serial << "SetPagePtr " << (int)&page << ":" << (int)page << "\n";
	for(size_t i = 0; i < _pages.size(); i++)
		if(page == _pages[i])
		{
			_activePage = i;
			return *this;
		}
	return *this;
}

// repaint screen
void FishinoTftGuiClass::paint(void)
{
	if(_activePage != 0xff)
		_pages[_activePage]->paint();
}

// repaint changed elements
void FishinoTftGuiClass::paintChanged(void)
{
	if(_activePage == 0xff)
		return;
	FishinoTftGuiPage *page = _pages[_activePage];
	for(size_t i = 0; i < page->_elements.size(); i++)
	{
		FishinoTftGuiElement *e = page->_elements[i];
		if(e->_changed)
		{
			e->_changed = false;
			if(e->_repaintBackground)
				tft().fillRect(e->prevX(), e->prevY(), e->prevW() + 1, e->prevH(), page->getBkColor());
			e->_repaintBackground = false;
			e->paint(false, 0, 0, 0);
		}
	}
}

// start the gui interface
FishinoTftGuiClass &FishinoTftGuiClass::begin(FishinoGFX &tft, FishinoTouch &touch)
{
	_touch = &touch;
	_tft = &tft;
//	_tft->setFont(&FreeSansBold9pt7b);

	return *this;
}

// internal loop step
void FishinoTftGuiClass::loop(void)
{
	// if no active page, do nothing
	if(_activePage == 0xff)
		return;
	
	// if too early, do nothing
	// we wait 50 mSec between tests
	if(millis() < _lastTouchCheckTime + 50)
		return;
	_lastTouchCheckTime = millis();
	
	// check if touch is touched
	bool touched = touch().touching();
	if(!touched && !_lastTouched)
	{
		paintChanged();
		return;
	}
	
	// touched, get current touched element
	uint16_t x, y;
	touch().read(x, y);
	
//	Serial << "Touch: " << x << ", " << y << "\n";
	
	FishinoTftGuiPage *page = _pages[_activePage];

	// check first if last touched element is into current page
	bool lastInPage = false;
	if(_lastTouched)
	{
		for(size_t i = 0; i < page->_elements.size(); i++)
		{
			FishinoTftGuiElement *e = page->_elements[i];
			if(e == _lastTouched)
			{
				lastInPage = true;
				break;
			}
		}
		
		// if previous touched element is not in current page
		// just wait till it gets released
		if(touched && !lastInPage)
			return;
	}
	
	// now check if an element is touched, no tolerance first
	FishinoTftGuiInteractiveElement *curElem = NULL;
	if(touched)
	{
		for(size_t i = 0; i < page->_elements.size(); i++)
		{
			FishinoTftGuiElement *e = page->_elements[i];
			if(!e->isInteractive())
				continue;
			FishinoTftGuiInteractiveElement *ie = (FishinoTftGuiInteractiveElement *)e;
			if(touched && ie->isInside(x, y))
			{
				curElem = ie;
				break;
			}
		}
		// if no element found, try again with 10 pixels tolerance on all sides
		if(!curElem)
		{
			for(size_t i = 0; i < page->_elements.size(); i++)
			{
				FishinoTftGuiElement *e = page->_elements[i];
				if(!e->isInteractive())
					continue;
				FishinoTftGuiInteractiveElement *ie = (FishinoTftGuiInteractiveElement *)e;
				if(touched && ie->isInside(x, y, 10))
				{
					curElem = ie;
					break;
				}
			}
		}
	
		// if previous clicked element is same as actual, just send dragging and keep events
		if(curElem && (curElem == _lastTouched))
		{
			curElem->whenEvent(EventType::KEEP);
			curElem->whenEvent(EventType::DRAG);
			paintChanged();
			return;
		}
	}

	// beep once if touched an element
	if(curElem && _beepOn)
		_beep();
	
	// if there was a previously touched element, deselect it
	if(lastInPage && _lastTouched)
	{
		if(_lastTouched->_repaintBackground)
		{
			tft().fillRect(_lastTouched->prevX(), _lastTouched->prevY(), _lastTouched->prevW() + 1, _lastTouched->prevH(), page->getBkColor());
			_lastTouched->_repaintBackground = false;
		}
		_lastTouched->paint(false, 0, 0, 0);
		_lastTouched->_changed = false;
		_lastTouched->whenEvent(EventType::RELEASE);
	}

	// select new element, if any
	_lastTouched = curElem;
	if(_lastTouched)
	{
		if(_lastTouched->_repaintBackground)
		{
			tft().fillRect(_lastTouched->prevX(), _lastTouched->prevY(), _lastTouched->prevW() + 1, _lastTouched->prevH(), page->getBkColor());
			_lastTouched->_repaintBackground = false;
		}
		_lastTouched->paint(true, x, y, 10);
		_lastTouched->_changed = false;
		_lastTouched->whenEvent(EventType::PRESS);
	}
	paintChanged();
}

FishinoTftGuiClass &__getFishinoTftGui(void)
{
	static FishinoTftGuiClass gui;
	return gui;
}
