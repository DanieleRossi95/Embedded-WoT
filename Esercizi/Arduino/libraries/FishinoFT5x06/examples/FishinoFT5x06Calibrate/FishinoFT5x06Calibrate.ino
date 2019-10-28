/////////////////////////////////////////////////////////////
// Project : 'FishinoXPT2046Calibrate'
// Created by FishIDE application 
//
// this sketch gives on serial port the calibration values
// for your TFT screen and a resistive touch display
// based on XPT2046 chip
/////////////////////////////////////////////////////////////
#include <SPI.h>
#include <FishinoGFX.h>
#include <FishinoILI9341.h>
#include <FishinoXPT2046.h>

/////////////////////////////////////////////////////////////
// change following values with your TFT screen resolution
const uint16_t TFT_X_SIZE = 240;
const uint16_t TFT_Y_SIZE = 320;
/////////////////////////////////////////////////////////////

void calibratePoint(uint16_t x, uint16_t y, uint16_t &xRaw, uint16_t &yRaw)
{
	// Draw cross
	FishinoILI9341.drawFastHLine(x - 8, y, 16, ILI9341_WHITE);
	FishinoILI9341.drawFastVLine(x, y - 8, 16, ILI9341_WHITE);

	// wait till we touch the display
	while (!FishinoXPT2046.touching())
		delay(10);

	// get raw values -- discard z
	int16_t z;
	FishinoXPT2046.readRaw(xRaw, yRaw, z);
	
	// wait till we stop touching the display
	while (FishinoXPT2046.touching())
		delay(10);

	// Erase by overwriting with black
	FishinoILI9341.drawFastHLine(x - 8, y, 16, ILI9341_BLACK);
	FishinoILI9341.drawFastVLine(x, y - 8, 16, ILI9341_BLACK);
}

void setup()
{
	Serial.begin(115200);
	while (!Serial)
		;
	Serial.println("Touch Calibrate!");
	Serial.println();

	// starts TFT display and fill it with black color
	FishinoILI9341.begin();
	FishinoILI9341.fillScreen(ILI9341_BLACK);

	// connect touch screen
	FishinoXPT2046.connect(TOUCH_CS, TOUCH_IRQ);

	// get first point calibration values
	uint16_t x1Raw, y1Raw;
	Serial.print("Please touch cross on upper left corner:");
	calibratePoint(20, 20, x1Raw, y1Raw);
	Serial.println(" DONE");
	delay(200);

	// get second point calibration values
	uint16_t x2Raw, y2Raw;
	Serial.print("Please touch cross on lower right corner:");
	calibratePoint(TFT_X_SIZE - 20, TFT_Y_SIZE - 20, x2Raw, y2Raw);
	Serial.println(" DONE");
	delay(200);
	
	// calculate calibration values
	int32_t ax = (((int32_t)(x2Raw - x1Raw)) * 8) / (TFT_X_SIZE - 40);
	int32_t bx = (20L * ax - 8L * x1Raw) / 8;
	int32_t ay = (((int32_t)(y2Raw - y1Raw)) * 8) / (TFT_Y_SIZE - 40);
	int32_t by = (20L * ay - 8L * y1Raw) / 8;

	Serial.println();
	Serial.println("Calibration constants:");
	Serial.print("AX = ");
	Serial.println(ax);
	Serial.print("BX = ");
	Serial.println(bx);
	Serial.print("AY = ");
	Serial.println(ay);
	Serial.print("BY = ");
	Serial.println(by);
	Serial.println();
	Serial.println("Please use FishinoXPT2046.calibrate(ax, bx, ay, by) function for best results");
	
	// stop
	while(1)
		;
}

void loop()
{
}
