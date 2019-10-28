#ifndef __FISHINOTFTGUI_FISHINOTFTGUIELEMENTGROUP_H
#define __FISHINOTFTGUI_FISHINOTFTGUIELEMENTGROUP_H

// we need to forward those :-(
class FishinoTftGuiButton;
class FishinoTftGuiIcon;
class FishinoTftGuiInteractiveIcon;
class FishinoTftGuiSlider;
class FishinoTftGuiLabel;
class FishinoTftGuiToggle;
class FishinoTftGuiLevelBar;
class FishinoTftGuiTriangleButton;
class FishinoTftGuiSpindle;

class FishinoTftGuiElementGroup : public FishinoTftGuiInteractiveElement
{
	friend class FishinoTftGuiClass;
	friend class FishinoTftGuiPage;

	private:
		
	protected:
		
		// the elements
		std::vector<FishinoTftGuiElement *>_elements;
		
		// event handler
		virtual void whenEvent(EventType eventType);
		
		// constructor - protected - can be instantiated only by page objects
		FishinoTftGuiElementGroup(FishinoTftGuiElementGroup *group, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
		
		// the associated handler
		FishinoTftGuiHandler _handler;

		// set/clear the handler
		FishinoTftGuiElementGroup &setHandler(FishinoTftGuiHandler handler) { _handler = handler; return *this; }
		FishinoTftGuiElementGroup &clearHandler(void) { _handler = NULL; return *this; }
		
		// locate sub-element
		FishinoTftGuiElement *findSubItem(int16_t x, int16_t y, uint16_t tol);
		
		// draw the element onto display
		virtual void paint(bool pressed)
			{ paint(pressed, 0, 0, 0); }
		virtual void paint(bool pressed, int16_t x, int16_t y, uint16_t tol);
		
		// add a button
		FishinoTftGuiButton &ButtonImpl(int16_t x, int16_t y, uint16_t w, uint16_t h);

		// add an icon
		FishinoTftGuiIcon &IconImpl(int16_t x, int16_t y, ICON const &icon);
		FishinoTftGuiInteractiveIcon &InteractiveIconImpl(int16_t x, int16_t y, ICON const &icon);

		// add a slider
		FishinoTftGuiSlider &SliderImpl(int16_t x, int16_t y, uint16_t len, bool vert, float min, float max);
		FishinoTftGuiSlider &SliderImpl(int16_t x, int16_t y, uint16_t len, bool vert, ICON const &cursor, float min, float max);

		// add a label
		FishinoTftGuiLabel &LabelImpl(int16_t const &x, int16_t const &y, uint16_t w, uint16_t h, LabelAlign align, const __FlashStringHelper *text);
		FishinoTftGuiLabel &LabelImpl(int16_t const &x, int16_t const &y, LabelAlign align, const __FlashStringHelper *text);
		FishinoTftGuiLabel &LabelImpl(int16_t const &x, int16_t const &y, uint16_t w, uint16_t h, LabelAlign align, const char *text);
		FishinoTftGuiLabel &LabelImpl(int16_t const &x, int16_t const &y, LabelAlign align, const char *text);
		
		// add a toggle button
		FishinoTftGuiToggle &ToggleImpl(int16_t x, int16_t y, ICON const &offIcon, ICON const &onIcon);

		// add a level bar
		FishinoTftGuiLevelBar &LevelBarImpl(int16_t x, int16_t y, uint16_t thick, uint16_t len, bool vert, float min, float max);

		// add a triangle button
		FishinoTftGuiTriangleButton &TriangleButtonImpl(int16_t x, int16_t y, uint16_t w, uint16_t h, ShapeDir dir, GradientDir gDir);

		// add a spindle
		FishinoTftGuiSpindle &SpindleImpl(int16_t x, int16_t y, uint16_t w = 0, uint16_t h = 0, uint16_t hLabel = 0);

		// set the active page
		FishinoTftGuiClass &setPageImpl(const __FlashStringHelper *name);
		FishinoTftGuiClass &setPageImpl(uint8_t idx);
		FishinoTftGuiClass &setPageImpl(FishinoTftGuiPage *page);
		
	public:

		// destructor
		virtual ~FishinoTftGuiElementGroup();
		
		// check if it's an interactive element
		virtual bool isInteractive(void) const;
		
		// check if this group is a page
		virtual bool isPage(void) const { return false; }

		// check if point is on element
		virtual bool isInside(int16_t x, int16_t y) const;
		virtual bool isInside(int16_t x, int16_t y, int16_t tol) const;
		
		// update bounding box -- used when adding elements
		virtual void updateBoundingBox(void);
		
		// set the active page
		template<typename... T>auto setPage(T&&... t) -> FishinoTftGuiClass&
			{ return setPageImpl( constref_or_evaluate(static_cast<T&&>(t))... ); }

		// add a button
		template<typename... T>auto Button(T&&... t) -> FishinoTftGuiButton&
			{ return ButtonImpl( constref_or_evaluate(static_cast<T&&>(t))... ); }

		// add an icon
		template<typename... T>auto Icon(T&&... t) -> FishinoTftGuiIcon&
			{ return IconImpl( constref_or_evaluate(static_cast<T&&>(t))... ); }

		template<typename... T>auto InteractiveIcon(T&&... t) -> FishinoTftGuiInteractiveIcon&
			{ return InteractiveIconImpl( constref_or_evaluate(static_cast<T&&>(t))... ); }

		// add a slider
		template<typename... T>auto Slider(T&&... t) -> FishinoTftGuiSlider&
			{ return SliderImpl( constref_or_evaluate(static_cast<T&&>(t))... ); }

		// add a label
		template<typename... T>auto Label(T&&... t) -> FishinoTftGuiLabel&
			{ return LabelImpl( constref_or_evaluate(static_cast<T&&>(t))... ); }

		// add a toggle button
		template<typename... T>auto Toggle(T&&... t) -> FishinoTftGuiToggle&
			{ return ToggleImpl( constref_or_evaluate(static_cast<T&&>(t))... ); }

		// add a level bar
		template<typename... T>auto LevelBar(T&&... t) -> FishinoTftGuiLevelBar&
			{ return LevelBarImpl( constref_or_evaluate(static_cast<T&&>(t))... ); }

		// add a triangle button
		template<typename... T>auto TriangleButton(T&&... t) -> FishinoTftGuiTriangleButton&
			{ return TriangleButtonImpl( constref_or_evaluate(static_cast<T&&>(t))... ); }

		// add a spindle
		template<typename... T>auto Spindle(T&&... t) -> FishinoTftGuiSpindle&
			{ return SpindleImpl( constref_or_evaluate(static_cast<T&&>(t))... ); }

		// get current element
		FishinoTftGuiElementGroup  &getElement(FishinoTftGuiElementGroup *&me) { me = this; return *this; }
};

#endif