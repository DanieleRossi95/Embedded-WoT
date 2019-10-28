//////////////////////////////////////////////////////////////////////////////////////
//						    FishinoJPEGTest.ino										//
//																					//
//			Small sample sketch for FishinoJPEGDecoder library						//
//																					//
//		Copyright (c) 2017 Massimo Del Fedele.  All rights reserved.				//
//																					//
//	Redistribution and use in source and binary forms, with or without				//
//	modification, are permitted provided that the following conditions are met:		//
//																					//
//	- Redistributions of source code must retain the above copyright notice,		//
//	  this list of conditions and the following disclaimer.							//
//	- Redistributions in binary form must reproduce the above copyright notice,		//
//	  this list of conditions and the following disclaimer in the documentation		//
//	  and/or other materials provided with the distribution.						//
//																					//	
//	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"		//
//	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE		//
//	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE		//
//	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE		//
//	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR				//
//	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF			//
//	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS		//
//	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN			//
//	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)			//
//	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE		//
//	POSSIBILITY OF SUCH DAMAGE.														//
//																					//
//  Version 5.0.0 -- May 2017   -- Initial version									//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#define DEBUG_LEVEL_INFO
#include "FishinoDebug.h"

#include<FishinoGFX.h>
#include<FishinoILI9341.h>
#include<FishinoFileStream.h>
#include<FishinoJPEGDecoder.h>

#ifdef SDCS
	#define SD_CS SDCS
#else
	#define SD_CS 4
#endif

SdFat SD;

#define tft		FishinoILI9341

bool consumeLine(JDR_RECT const &rec, void const *buf)
{
	if(rec.left + rec.width > 240 || rec.top + rec.height > 320)
		return true;
	FishinoILI9341Class &t = FishinoILI9341;
	t.setAddrWindow(rec.left, rec.top, rec.left + rec.width - 1, rec.top + rec.height - 1);
	uint16_t const *buf16 = (uint16_t const *)buf;
	t.pushColors(rec.width * rec.height, buf16);
	return true;
}

void setup()
{
	// Open serial communications and wait for port to open:
	Serial.begin(115200);

	// remove comment on following 2 lines if you want to wait
	// for serial monito to be opened before playing
/*
	while (!Serial)
		;
*/

	// this if you want some debug on secondary serial port
	// (you need an USB-Serial adapter for this!!!)
	// if you want debug on usual Serial port just comment these lines
#ifdef _FISHINO_PIC32_
	Serial0.begin(115200);
	DEBUG_SET_STREAM(Serial0);
#endif
	DEBUG_INFO("DEBUG INITIALIZED\n");

	// initializa tft display
	tft.begin();
	tft.fillScreen(ILI9341_BLACK);

	// initialize SD card
	DEBUG_INFO("Initializing SD card...");
#ifdef _FISHINO_PIC32_
	if (!SD.begin(SD_CS, SD_SCK_MHZ(15)))
#else
	if (!SD.begin(SD_CS, SD_SCK_MHZ(6)))
#endif
		DEBUG_ERROR("failed!\n");
	DEBUG_INFO("OK!\n");
	
	FishinoFileStream s;
	if(!s.open("chri8s.jpg"))
		DEBUG_ERROR("Error opening file\n");
	else
	{
		DEBUG_INFO("Open ok\n");
		FishinoJPEGDecoder decoder;
		decoder.setConsumer(consumeLine, 240, 320, JDR_COLOR565);
		decoder.setAutoCenter(JDR_CENTER_BOTH);

		for(uint8_t autos = 0; autos < 2; autos++)
		{
			for(JDR_ROTATIONS rot = JDR_ROT_0; rot <= JDR_ROT_AUTOCCW; rot = (JDR_ROTATIONS)(rot + 1))
			{
				int res = decoder.setProvider(s);
				DEBUG_INFO("setProvider returned %d\n", res);
				decoder
					.setAutoScale(autos)
					.setRotation(rot)
				;
				uint32_t tim = millis();
				decoder.decode();
				tim = millis() - tim;
				DEBUG_INFO("IMAGE TIME : %ld\n", tim);
			}
		}
	}
}

void loop()
{
}
