//////////////////////////////////////////////////////
//	Simple paint sketch using Fishino TFT shield	//
//													//
//	Just run the sketch, open serial port and start	//
//	painting on TFT screen with your fingers		//
//													//
//	CopyRight(C) 2017 By Massimo Del Fedele			//
//													//
//	This program is released in the public domain	//
//////////////////////////////////////////////////////
#include <Arduino.h>
#include <SPI.h>

#include "FishinoGFX.h"
#include "FishinoILI9341SPI.h"
#include "FishinoXPT2046.h"

#define tft FishinoILI9341SPI
#define touch FishinoXPT2046

void setup()
{
	Serial.begin(115200);
	while (!Serial)
		;
	Serial.println("ILI9341 Paint");

	tft.begin();
	tft.setRotation(1);
	tft.fillScreen(GFX_BLACK);

	// Replace these for your screen module
	touch.setRotation(1);

/*
	tft.drawLine(0, 0, 319, 239, ILI9341_WHITE);
	tft.drawLine(0, 239, 319, 0, ILI9341_RED);
*/

	Serial.println("Setup complete");
}

static uint16_t prev_x = 0xffff, prev_y = 0xffff;

void loop()
{
	if (touch.touching())
	{
		uint16_t x, y;
		touch.read(x, y);
		Serial.print("X:");
		Serial.print(x);
		Serial.print("  Y:");
		Serial.println(y);
		if (prev_x == 0xffff)
		{
			tft.drawPixel(x, y, GFX_WHITE);
		}
		else
		{
			tft.drawLine(prev_x, prev_y, x, y, GFX_WHITE);
		}
		prev_x = x;
		prev_y = y;
	}
	else
	{
		prev_x = prev_y = 0xffff;
	}
	delay(20);
}
