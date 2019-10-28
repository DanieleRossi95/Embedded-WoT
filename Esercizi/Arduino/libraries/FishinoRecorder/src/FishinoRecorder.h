//////////////////////////////////////////////////////////////////////////////////////
//																					//
//								FishinoRecorder.h									//
//				A class to record audio samples on an SD card						//
//  Available formata :																//
//		.wav (PCM, uncompressed), full support										//
//		.mp3, limited support, usable only on low rate audio streams				//
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
//  Version 7.3.1	24/12/2017	Fixed mips16 compilation issues						//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FISHINORECORDER_H
#define _FISHINORECORDER_H

// for sending audio to codec
#include <I2S.h>

// for codec interface
#include <FishinoAudioCodec.h>

// for audio writer classes
#include <FishinoAudioWriter.h>

class FishinoRecorder
{
	friend void i2sHandler(FishinoRecorder &r);

	private:
	
		// the audio writer for this file
		AudioWriter *_writer;
		
		// the I2S interface
		I2SClass *_i2s;
		
		// the used codec
		AudioCodec *_codec;
		
		// handle i2s requests
		void handleI2SRequest(void);
		
		// play in progress flag
		volatile bool _recording;
		
		// we need to store audioformat here
		// because it will be set on writer creation
		uint8_t _bits;
		uint32_t _sampleRate;
		ChannelModes _channelMode;
		
		// some info grabbed from writer before closing

		// count of skipped samples
		uint32_t _curSkippedSamples;
		uint32_t _totalSkippedSamples;
		uint32_t _maxSkippedSamples;
		
		// total recorded samples
		uint32_t _numSamples;
		
		// debugger channel
		Stream *_debugStream;

	protected:
	
	public:
	
		// constructor
		FishinoRecorder(I2SClass &i2s = I2S, AudioCodec &codec = ALC5631);
	
		// destructor
		~FishinoRecorder();
		
		// set the audio format for recording
		bool setAudioFormat(uint32_t sampleRate, uint8_t bits, ChannelModes channelMode);
		
		// set compression
		bool setCompression(uint8_t mode);
		
		// open the record file and prepare for recording
		// but don't start it
		bool open(const char *fileName);
		
		// close and finish recording
		bool close(void);
	
		// start/resume recording a file to SD card
		// if fileName is not given, just use previous one
		// or restart record if paused
		bool record(const char *fileName = NULL);
		
		// pause the recording
		bool pause(void);
		
		// stop recording and close file on SD card
		bool stop(void);
		
		// check if it's still recording
		bool isRecording(void) { return _recording; }
		
		// change the input gain
		bool setMicGain(float vol, float bal = 0);
		
		// change the input volume
		bool setInputVolume(float vol, float bal = 0);
		
		// get some info from writer
		uint32_t getNumSamples(void);
		uint32_t getMaxSkippedSamples(void);
		uint32_t getTotalSkippedSamples(void);

		// enable/disable debug logs
		void debug(Stream &s = Serial);
		void noDebug(void);
};

#endif
