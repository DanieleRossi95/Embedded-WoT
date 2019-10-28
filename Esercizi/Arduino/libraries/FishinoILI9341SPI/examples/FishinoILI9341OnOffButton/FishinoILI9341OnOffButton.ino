//This example implements a simple sliding On/Off button. The example
// demonstrates drawing and touch operations.
//
//Thanks to Adafruit forums member Asteroid for the original sketch!
//
#include <SPI.h>
#include <FishinoGFX.h>
#include <FishinoILI9341SPI.h>
#include <FishinoXPT2046.h>

#define tft FishinoILI9341SPI
#define touch FishinoXPT2046

boolean RecordOn = false;

#define FRAME_X 210
#define FRAME_Y 180
#define FRAME_W 100
#define FRAME_H 50

#define REDBUTTON_X FRAME_X
#define REDBUTTON_Y FRAME_Y
#define REDBUTTON_W (FRAME_W/2)
#define REDBUTTON_H FRAME_H

#define GREENBUTTON_X (REDBUTTON_X + REDBUTTON_W)
#define GREENBUTTON_Y FRAME_Y
#define GREENBUTTON_W (FRAME_W/2)
#define GREENBUTTON_H FRAME_H

void drawFrame()
{
	tft.drawRect(FRAME_X, FRAME_Y, FRAME_W, FRAME_H, GFX_BLACK);
}

void redBtn()
{
	tft.fillRect(REDBUTTON_X, REDBUTTON_Y, REDBUTTON_W, REDBUTTON_H, GFX_RED);
	tft.fillRect(GREENBUTTON_X, GREENBUTTON_Y, GREENBUTTON_W, GREENBUTTON_H, GFX_BLUE);
	drawFrame();
	tft.setCursor(GREENBUTTON_X + 6 , GREENBUTTON_Y + (GREENBUTTON_H/2));
	tft.setTextColor(GFX_WHITE);
	tft.setFontScale(2);
	tft.println("ON");
	RecordOn = false;
}

void greenBtn()
{
	tft.fillRect(GREENBUTTON_X, GREENBUTTON_Y, GREENBUTTON_W, GREENBUTTON_H, GFX_GREEN);
	tft.fillRect(REDBUTTON_X, REDBUTTON_Y, REDBUTTON_W, REDBUTTON_H, GFX_BLUE);
	drawFrame();
	tft.setCursor(REDBUTTON_X + 6 , REDBUTTON_Y + (REDBUTTON_H/2));
	tft.setTextColor(GFX_WHITE);
	tft.setFontScale(2);
	tft.println("OFF");
	RecordOn = true;
}

void setup(void)
{
	Serial.begin(115200);

	tft.begin();
	tft.fillScreen(GFX_BLUE);
	tft.setRotation(1);
	
	touch.setRotation(1);
	
	redBtn();
}

void loop()
{
	// See if there's any  touch data for us
	if (touch.touching())
	{
		// Retrieve a point
		uint16_t x, y;
		touch.read(x, y);

		if (RecordOn)
		{
			if ((x > REDBUTTON_X) && (x < (REDBUTTON_X + REDBUTTON_W)))
			{
				if ((y > REDBUTTON_Y) && (y <= (REDBUTTON_Y + REDBUTTON_H)))
				{
					Serial.println("Red btn hit");
					redBtn();
				}
			}
		}
		else //Record is off (RecordOn == false)
		{
			if ((x > GREENBUTTON_X) && (x < (GREENBUTTON_X + GREENBUTTON_W)))
			{
				if ((y > GREENBUTTON_Y) && (y <= (GREENBUTTON_Y + GREENBUTTON_H)))
				{
					Serial.println("Green btn hit");
					greenBtn();
				}
			}
		}

		Serial.println(RecordOn);
		delay(100);
	}
}



