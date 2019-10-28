///////////////////////////////////////////////////////////////////////////////////
//                             FishinoXPT2046Test.ino                            //
//                                                                               //
// Prints coordinates of touched point on a touch screen with XPT2046 controller //
//                                                                               //
// Copyright(C) 2017 by Massimo Del Fedele                                       //
//                                                                               //
// This program is released in public domain                                     //
///////////////////////////////////////////////////////////////////////////////////
#include <FishinoXPT2046.h>
#include <FishinoFlash.h>
#include <SPI.h>

void setup()
{
	// initialize serial port
	Serial.begin(115200);
	
	// set scree rotation
	// (you can change with 0..3 to rotate screen counterclockwise)
	FishinoXPT2046.setRotation(0);
}

void loop()
{
	if(FishinoXPT2046.touching())
	{
		uint16_t x, y, z;
		FishinoXPT2046.read(x, y, z);
		Serial << "X : " << x << "\n";
		Serial << "Y : " << y << "\n";
		Serial << "Z : " << z << "\n";
		Serial << "------------------\n";
	}
	delay(200);
}
