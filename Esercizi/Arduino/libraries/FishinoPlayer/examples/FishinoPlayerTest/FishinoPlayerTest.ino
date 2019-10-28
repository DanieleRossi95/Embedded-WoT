//////////////////////////////////////////////////////////////////////////////////////
//																					//
//							FishinoPlayerTest.ino									//
//							Fishino Player demo										//
//				Plays all audio files on an sd card in sequence						//
//					Copyright (c) 2017 Massimo Del Fedele							//
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
//	VERSION 1.0.0	2016		INITIAL VERSION										//
//  Version 6.0.2	June 2017	Complete rewrite using new FishinoPlayer class		//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#include <FishinoPlayer.h>

#include <FishinoSdFat.h>

#define DEBUG_LEVEL_INFO
#include <FishinoDebug.h>

SdFat SD;

SdBaseFile root;
uint16_t numFiles;

// enable this define to have debug output on alternative Serial0
//#define DEBUG_ON_SERIAL0

// enable this define if you are printing on USB Serial port
// and want to wite that it's opened before starting
// (the sketch will stop until you open serial monitor)
#define WAIT_FOR_SERIAL

#ifdef DEBUG_LEVEL_INFO
uint32_t tim;
#endif

void setup()
{
	// this if you want some debug on secondary serial port
	// (you need an USB-Serial adapter for this!!!)
#ifdef DEBUG_ON_SERIAL0
	Serial0.begin(115200);
	DEBUG_SET_STREAM(Serial0);
#else
	Serial.begin(115200);
	DEBUG_SET_STREAM(Serial);
#ifdef WAIT_FOR_SERIAL
	while (!Serial)
		;
#endif
#endif
	DEBUG_INFO("DEBUG INITIALIZED\n");

	DEBUG_INFO("Initializing SD card...");

//	if (!SD.begin(SDCS /*, 3 */))
	if (!SD.begin(SDCS, SD_SCK_MHZ(16)))
	{
		DEBUG_INFO_N("Card failed, or not present\n");
		return;
	}
	DEBUG_INFO_N("OK\n");

	// open SD root
	root.openRoot(SD.vol());
	numFiles = 0;

#ifdef DEBUG_LEVEL_INFO
	tim = millis();
#endif
}

void loop()
{
	// open next file
	SdFile entry;
	char *name;

	// if terminated, start again
	if (!entry.openNext(&root, O_READ))
	{
		// if we've read all the directory without finding any audio file
		// just tell it and stop
		if (!numFiles)
		{
			DEBUG_WARNING("No audio files on SD card\n");
			while (1)
				;
		}

		// close and re-open root folder to start again
		root.close();
		root.open("/", O_READ);

		// no files on SD, just halt
		if (!entry.openNext(&root, O_READ))
		{
			DEBUG_WARNING("No files on SD card\n");
			while (1)
				;
		}
	}

	// check if entry is a file
	if(entry.isFile())
	{
		// get current file name
		name = (char *)malloc(100);
		entry.getName(name, 99);
	
		// close file entry
		entry.close();
	
		// ok, if we're here we've got an audio file name to play!
		numFiles++;
	
		FishinoPlayer.volume(0.5, 0);
		if(FishinoPlayer.play(name))
			DEBUG_PRINT("Playing : '%s'\n", name);
		else
			DEBUG_INFO("Skipping non audio media '%s'\n", name);
		while (FishinoPlayer.isPlaying())
			delay(200);
		
		// free stored file name
		free(name);
	}
	else
		entry.close();
	
#ifdef DEBUG_LEVEL_INFO
	if(millis() > tim)
	{
		DEBUG_FREE_MEMORY();
		tim = millis() + 500;
	}
#endif
	
}
