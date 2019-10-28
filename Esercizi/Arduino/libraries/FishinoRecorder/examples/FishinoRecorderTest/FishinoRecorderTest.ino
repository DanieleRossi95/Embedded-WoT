/////////////////////////////////////////////////////////////////////////////////
//
//  Fishino Recorder demo
//
//  Copyright(C) 2016 by Massimo Del Fedele. All right reserved.
//
//  This demo records one minute of stereo audio sample in .WAV format
//  and re-plays it forever
//  Before using it you should prepare an SD card formatted with FAT16 or FAT32 systems
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
/////////////////////////////////////////////////////////////////////////////////

#ifndef ARDUINO_STREAMING
#define ARDUINO_STREAMING
template<class T> inline Print &operator <<(Print &stream, T arg)
{
	stream.print(arg);
	return stream;
}
#endif

#include <Wire.h>
#include <Timer.h>
#include <SPI.h>
#include <I2S.h>

#include <FishinoSdFat.h>
SdFat SD;

#include <FishinoAudioCodec.h>
#include <FishinoAudioWriter.h>
#include <FishinoRecorder.h>
#include <FishinoPlayer.h>

#define DEBUG_LEVEL_ERROR
#include <FishinoDebug.h>

FishinoRecorder recorder;

void setup(void)
{
	// Open serial communications and wait for port to open:
	Serial.begin(115200);
	while (!Serial)
		;

	// enable debug features of Fishino32
	Serial0.begin(115200);
	DEBUG_PRINT("DEBUG INITIALIZED");

	// initialize I2C
	Wire.begin();
	Wire.setClock(100000);

	Serial.print("Initializing SD card...");
	if (!SD.begin(SDCS, SPI_QUARTER_SPEED))
//  if (!SD.begin(SDCS))
	{
		Serial.println("Card failed, or not present");
		return;
	}
	Serial.println("card initialized.");

	// change the input gain
	recorder.setMicGain(1);

	// change the input volume
//  recorder.setInputVolume(0.5);


	// record one minute audio sample
	Serial.println("Recording...");
	recorder.setAudioFormat(44100, 16, CHANMODE_STEREO);
	recorder.record("test.wav");
	uint32_t tim = millis() + 60000L;
	while (millis() < tim)
		;
	recorder.stop();

	Serial << "Finished recording\n";
	Serial << "Total samples                  : " << recorder.getNumSamples() << "\n";
	Serial << "Total skipped samples          : " << recorder.getTotalSkippedSamples() << "\n";
	Serial << "Max sequential skipped samples : " << recorder.getMaxSkippedSamples() << "\n";

	// replay recorded sample forever
	while (1)
	{
		// re-play the sample
		Serial.println("Playing...");
		FishinoPlayer.play("test.wav");
		while (FishinoPlayer.isPlaying())
			;
		delay(1000);
	}

}

void loop(void)
{
	// no code here
}
