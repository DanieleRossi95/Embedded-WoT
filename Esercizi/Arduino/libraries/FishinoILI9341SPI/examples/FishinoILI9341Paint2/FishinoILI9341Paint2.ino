///////////////////////////////////////////////////////////////////////////////////
//	This is our touchscreen painting example for the Adafruit ILI9341 Shield	//
//	----> http://www.adafruit.com/products/1651									//
//																				//
//	Check out the links above for our tutorials and wiring diagrams				//
//	These displays use SPI to communicate, 4 or 5 pins are required to			//
//	interface (RST is optional)													//
//	Adafruit invests time and resources providing this open source code,		//
//	please support Adafruit and open-source hardware by purchasing				//
//	products from Adafruit!														//
//																				//
//	Written by Limor Fried/Ladyada for Adafruit Industries.						//
//	MIT license, all text above must be included in any redistribution			//
//																				//
//	Simple paint sketch using Fishino TFT shield								//
//	Adapted to Fishino TFT shield by Massimo Del Fedele - April 2017			//
//																				//
//	Just run the sketch, open serial port and start								//
//	painting on TFT screen with your fingers									//
//																				//
//////////////////////////////////////////////////////////////////////////////////
#include <SPI.h>
#include <FishinoGFX.h>
#include <FishinoILI9341SPI.h>
#include <FishinoXPT2046.h>

#define tft		FishinoILI9341SPI
#define touch	FishinoXPT2046

// Size of the color selection boxes and the paintbrush size
#define BOXSIZE 40
#define PENRADIUS 3
int oldcolor, currentcolor;

void setup(void)
{
	Serial.begin(115200);
	while(!Serial)
		;
	Serial.println(F("Touch Paint!"));

	tft.begin();
	tft.fillScreen(GFX_BLACK);

	// make the color selection boxes
	tft.fillRect(0, 0, BOXSIZE, BOXSIZE, GFX_RED);
	tft.fillRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, GFX_YELLOW);
	tft.fillRect(BOXSIZE*2, 0, BOXSIZE, BOXSIZE, GFX_GREEN);
	tft.fillRect(BOXSIZE*3, 0, BOXSIZE, BOXSIZE, GFX_CYAN);
	tft.fillRect(BOXSIZE*4, 0, BOXSIZE, BOXSIZE, GFX_BLUE);
	tft.fillRect(BOXSIZE*5, 0, BOXSIZE, BOXSIZE, GFX_MAGENTA);

	// select the current color 'red'
	tft.drawRect(0, 0, BOXSIZE, BOXSIZE, GFX_WHITE);
	currentcolor = GFX_RED;
}


void loop()
{
	// See if there's any  touch data for us
	if(!touch.touching())
		return;

	// Retrieve a point
	uint16_t x, y;
	touch.read(x, y);

	if(y < BOXSIZE)
	{
		oldcolor = currentcolor;

		if (x < BOXSIZE)
		{
			currentcolor = GFX_RED;
			tft.drawRect(0, 0, BOXSIZE, BOXSIZE, GFX_WHITE);
		}
		else if (x < BOXSIZE*2)
		{
			currentcolor = GFX_YELLOW;
			tft.drawRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, GFX_WHITE);
		}
		else if (x < BOXSIZE*3)
		{
			currentcolor = GFX_GREEN;
			tft.drawRect(BOXSIZE*2, 0, BOXSIZE, BOXSIZE, GFX_WHITE);
		}
		else if (x < BOXSIZE*4)
		{
			currentcolor = GFX_CYAN;
			tft.drawRect(BOXSIZE*3, 0, BOXSIZE, BOXSIZE, GFX_WHITE);
		}
		else if (x < BOXSIZE*5)
		{
			currentcolor = GFX_BLUE;
			tft.drawRect(BOXSIZE*4, 0, BOXSIZE, BOXSIZE, GFX_WHITE);
		}
		else if (x < BOXSIZE*6)
		{
			currentcolor = GFX_MAGENTA;
			tft.drawRect(BOXSIZE*5, 0, BOXSIZE, BOXSIZE, GFX_WHITE);
		}

		if (oldcolor != currentcolor)
		{
			if (oldcolor == GFX_RED)
				tft.fillRect(0, 0, BOXSIZE, BOXSIZE, GFX_RED);
			if (oldcolor == GFX_YELLOW)
				tft.fillRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, GFX_YELLOW);
			if (oldcolor == GFX_GREEN)
				tft.fillRect(BOXSIZE*2, 0, BOXSIZE, BOXSIZE, GFX_GREEN);
			if (oldcolor == GFX_CYAN)
				tft.fillRect(BOXSIZE*3, 0, BOXSIZE, BOXSIZE, GFX_CYAN);
			if (oldcolor == GFX_BLUE)
				tft.fillRect(BOXSIZE*4, 0, BOXSIZE, BOXSIZE, GFX_BLUE);
			if (oldcolor == GFX_MAGENTA)
				tft.fillRect(BOXSIZE*5, 0, BOXSIZE, BOXSIZE, GFX_MAGENTA);
		}
	}
	if (((y - PENRADIUS) > BOXSIZE) && ((y + PENRADIUS) < tft.height()))
	{
		tft.fillCircle(x, y, PENRADIUS, currentcolor);
	}
}
