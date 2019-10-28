//////////////////////////////////////////////////////////////////////////////////////
//								FishinoFT5x06Test.ino								//
//																					//
//	Prints coordinates of touched point on a touch screen with XPT2046 controller	//
//																					//
// Copyright(C) 2017 by Massimo Del Fedele											//
//																					//
// This program is released in public domain										//
//////////////////////////////////////////////////////////////////////////////////////
#include <FishinoFT5x06.h>
#include <FishinoFlash.h>

void setup()
{
	// initialize serial port
	Serial.begin(115200);
	while(!Serial)
		;
	Serial << F("FishinoFT5x06 touch screen test\n");
	
	// set scree rotation
	// (you can change with 0..3 to rotate screen counterclockwise)
	FishinoFT5x06.setRotation(0);
}

void loop()
{
	if(FishinoFT5x06.touching())
	{
		uint16_t x, y;
		FishinoFT5x06.read(x, y);
		Serial << "X : " << x << "\n";
		Serial << "Y : " << y << "\n";
		Serial << "------------------\n";
	}
	delay(200);
}
