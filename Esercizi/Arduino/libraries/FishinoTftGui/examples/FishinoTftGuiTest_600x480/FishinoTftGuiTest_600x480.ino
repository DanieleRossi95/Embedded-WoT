#include <FishinoTftGui.h>
#include <Fishino.h>

//#define FISHINO_DISPLAY_ILI9341_SPI
#define FISHINO_DISPLAY_RA8875_SPI

//#define FISHINO_TOUCH_XPT2046
#define FISHINO_TOUCH_FT5X06

#if defined FISHINO_DISPLAY_RA8875_SPI
	#include <FishinoRA8875SPI.h>
	#define TFT FishinoRA8875SPI
#elif defined FISHINO_DISPLAY_ILI9341_SPI
	#include <FishinoILI9341SPI.h>
	#define TFT FishinoILI9341SPI
#else
	#error "Must define display type"
#endif

#if defined FISHINO_TOUCH_XPT2046
	#include <FishinoXPT2046.h>
	#define TOUCH FishinoXPT2046
#elif defined FISHINO_TOUCH_FT5X06
	#include <FishinoFT5x06.h>
	#define TOUCH FishinoFT5x06
#endif

// icons file
#include "icons.h"

// font file
#include <Fonts/FreeSansBold9pt7b.h>


FishinoTftGuiPage *p1 = (FishinoTftGuiPage *)1, *p2 = (FishinoTftGuiPage *)2;
FishinoTftGuiLabel *l1 = NULL, *l2 = NULL;

FishinoTftGuiSlider *slider, *slider2;
FishinoTftGuiLevelBar *levelBar, *levelBar2;

void buttonHandler(FishinoTftGuiInteractiveElement &e, EventType event)
{
	if(e.Page().getBkColor() == GFX_DARKGREEN)
		e.Page().setBkColor(GFX_BLUE);
	else
		e.Page().setBkColor(GFX_DARKGREEN);
	e.Page().paint();
}

void iconHandler(FishinoTftGuiInteractiveElement &e, EventType event)
{
	// react only on PRESS events
	if(event != EventType::PRESS)
		return;
	
	if(e.Gui().getPage() == p1)
		e.Gui().setPage(p2);
	else
		e.Gui().setPage(p1);
	e.Gui().paint();
}

void sliderHandler(FishinoTftGuiInteractiveElement &e, EventType event = EventType::DUMMY)
{
	FishinoTftGuiSlider &s = (FishinoTftGuiSlider &)e;
	float pos = s.getPos();
	char buf[50];
#ifdef __AVR__
// avr doesn't support sprintf with float
	sprintf(buf, "%d.%03d", (int)pos, (int)((pos - int(pos))*1000));
#else
	sprintf(buf, "%2.3f", (double)pos);
#endif
	l2->setText(buf);
	levelBar->setValue(10 * pos);
	levelBar2->setValue(10 * pos);
	if(&e == slider)
		slider2->setPos(s.getPos());
	else
		slider->setPos(s.getPos());
}

uint32_t tim;

void setup()
{
	Serial.begin(115200);
	
/*
	while(!Serial)
		;
*/

	Serial.println("FishinoTFTGui Test!");

#if defined(FISHINO_DISPLAY_ILI9341_SPI)
	TFT.begin();
	TFT.setRotation(3);
	TOUCH.setRotation(3);
	TFT.setFontScale(1);
	TFT.setFont(&FreeSansBold9pt7b);
#elif defined(FISHINO_DISPLAY_RA8875_SPI)
	TFT.begin(2);
	TOUCH.setRotation(0);
	TFT.setFont(&FreeSansBold9pt7b);
	TFT.setFontScale(1);
#else
	#error "Unsupported display"
#endif

	
/*
	TFT.setFont(&FreeSansBold9pt7b);
	TFT.fillScreen(GFX_RED);
	
	TFT.setFontScale(2);
	TFT.setCursor(100, 200);
	TFT.setTextColor(GFX_CYAN);
	TFT.print("CIAO PEPPPPPPPP");
	
	while(1)
		;
*/

/*
	FishinoILI9341.shadeRectVertical(120, 50, 100, 20, GFX_RED, GFX_CYAN);
	FishinoILI9341.shadeRectVertical(120, 100, 100, 110, GFX_RED, GFX_CYAN);

	FishinoILI9341.shadeRectVertical(120, 50, 100, 20, 0x634e, 0x2945);
	FishinoILI9341.shadeRectVertical(120, 100, 100, 110, 0x634e, 0x2945);
*/
	uint16_t tSize = 64;
	FishinoTftGui
		.begin(TFT, TOUCH)
//		.beepOn()
		.Page(F("Page1"))
			.getElement(p1)
			.setBkColor(GFX_DARKGREEN)
			.Label(0, 0, 320, 24, LabelAlign::Center, F("Fishino Tft Gui Library demo"))
//			.Label(160, 0, 0, 32, LabelAlign::Center, F("Fishino Tft Gui Library demo"))
				.setTextColor(GFX_CYAN)
				.setTextBackground(GFX_RED)
				--
			.Label(50, 140, 0, 24, LabelAlign::Left, F("Slider value:"))
				.setTextColor(GFX_WHITE)
				.noTextBackground()
				.getElement(l1)
				--
			.Label([&](){ return l1->x() + l1->w() + 10; }, 140, 0, 24, LabelAlign::Left, F("xxxxx"))
//			.Label(180, 140, 0, 24, LabelAlign::Left, F("pippo"))
				.setTextColor(GFX_WHITE)
//				.setTextBackground(GFX_BLUE)
				.getElement(l2)
				--
			.Button(140, 200, 120, 32)
				.Text(F("Cliccami !!"))
				.Icon(iconApply24)
				.setHandler(buttonHandler)
				--
			.InteractiveIcon(320-26, 240-26, iconRight24)
				.setHandler(iconHandler)
				--
			.Slider(20, 30, 205, true, iconround24, 0, 1)
				.getElement(slider)
				.setHandler(sliderHandler)
				--
			.Toggle(50, 200, iconoff32, iconon32)
				--
			.LevelBar(50, 70, 32, 250, false, 0, 10)
				.getElement(levelBar)
				--
			.TriangleButton(400, 150, tSize, tSize, ShapeDir::UP, GradientDir::VERTICAL)
				--
			.TriangleButton(400, 150 + tSize * 3, tSize, tSize, ShapeDir::DOWN, GradientDir::VERTICAL)
				--
			.TriangleButton(400 - tSize * 1.5, 150 + tSize * 1.5, tSize, tSize, ShapeDir::LEFT, GradientDir::VERTICAL)
				--
			.TriangleButton(400 + tSize * 1.5, 150 + tSize * 1.5, tSize, tSize, ShapeDir::RIGHT, GradientDir::VERTICAL)
				--
/*
			.Spindle(200, 100, 24, 24)
				--
*/
			--
		.Page(F("Page2"))
			.getElement(p2)
			.setBkColor(GFX_BLUE)
/*
			.InteractiveIcon(150, 100, iconDelete24)
				--
*/
			.InteractiveIcon(320-26, 240-26, iconLeft24)
				.setHandler(iconHandler)
				--
			.Spindle(140, 100)
				--
			.Slider(20, 20, 300, false, iconround24, 0, 1)
				.getElement(slider2)
				.setHandler(sliderHandler)
				--
			.LevelBar(20, 50, 32, 180, true, 0, 10)
				.getElement(levelBar2)
				--
			--
		.setPage([&]() { return p2; })
		.paint()
	;
	
	// run slider handler to display initial value
	sliderHandler(*slider2);

	tim = millis() + 2000;
	Serial << F("Setup end\n");
}

void loop()
{
	FishinoTftGui.loop();
	if(millis() > tim)
	{
		Serial << "Free RAM : " << Fishino.freeRam() << "\n";
		tim = millis() + 2000;
	}
}
