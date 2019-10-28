//////////////////////////////////////////////////////////////////////////////////////
//																					//
//								FishinoRecorder.cpp									//
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
#include "FishinoRecorder.h"

#ifndef ARDUINO_STREAMING
#define ARDUINO_STREAMING
template<class T> inline Print &operator <<(Print &stream, T arg)
{
	stream.print(arg);
	return stream;
}
#endif

// I2S interrupt handler -- handles data requests from controller
void i2sHandler(FishinoRecorder &r)
{
	r.handleI2SRequest();
}

// handle i2s requests
void FishinoRecorder::handleI2SRequest(void)
{
	_writer->nextSample(_i2s->receive24());
}

// set the audio format for recording
bool FishinoRecorder::setAudioFormat(uint32_t sampleRate, uint8_t bits, ChannelModes channelMode)
{
	// store locally audio parameters
	// if writer is still not available, set them later
	_bits = bits;
	_sampleRate = sampleRate;
	_channelMode = channelMode;
	
	// if writer is available (file already opened)
	// set params to it
	if(_writer)
		_writer->setAudioParams(_sampleRate, _bits, channelMode);

	return true;
}

// constructor
FishinoRecorder::FishinoRecorder(I2SClass &i2s, AudioCodec &codec)
{
	_i2s = &i2s;
	_codec = &codec;
	_writer = NULL;
	_recording = false;

	_bits = 8;
	_sampleRate = 44100;
	_channelMode = CHANMODE_STEREO;

	// count of skipped samples
	_curSkippedSamples = 0;
	_totalSkippedSamples = 0;
	_maxSkippedSamples = 0;
	
	// total recorded samples
	_numSamples = 0;
	
	// no debug stream
	_debugStream = NULL;
}
	
// destructor
FishinoRecorder::~FishinoRecorder()
{
	if(_writer)
		delete(_writer);
	_writer = NULL;
}

// open the record file and prepare for recording
// but don't start it
bool FishinoRecorder::open(const char *fileName)
{
	if(_debugStream)
		*_debugStream << "Opening recorder\n";
	// if already opened, close it before
	if(_writer)
		close();
	
	// needs a fileName
	if(!fileName)
		return false;
	

	// check which reader to use
	const char *ext = fileName + strlen(fileName) - 4;
	if(!stricmp(ext, ".WAV"))
		_writer = new WavWriter(fileName);
	else if(!stricmp(ext, ".MP3"))
		_writer = new Mp3Writer(fileName);
	else
	{
		if(_debugStream)
			*_debugStream << "Invalid file type\n";
		return false;
	}

	if(_debugStream)
		*_debugStream << "Got writer\n";

	if(!_writer->isValid())
	{
		if(_debugStream)
			*_debugStream << "Writer error\n";
		delete _writer;
		_writer = NULL;
		return false;
	}
	
	// setup audio params into writer
	_writer->setAudioParams(_sampleRate, _bits, _channelMode);
	
	if(_debugStream)
		*_debugStream << "Params set\n";
	
	// power on codec for record from MIC inputs
	// (we may add AUX inputs later)
	_codec->power(AudioCodec::STEREOMICS);
	
	// setup PATH to record from MIC inputs
	_codec->path(AudioCodec::STEREOMICS2ADC);
	
	// setup I2S handler
	_i2s->setReceiveHandler(i2sHandler, *this);
	
	// set I2S audio parameters
	if(_debugStream)
	{
		_debugStream->print("Audio parameters : sample rate = ");
		_debugStream->print(_sampleRate);
		_debugStream->print(", bits = ");
		_debugStream->print(_bits);
		_debugStream->print(" channel mode = ");
		_debugStream->println(_channelMode);
		_debugStream->flush();
	}
	_i2s->setAudioParams(_sampleRate, _bits, _channelMode);
	
	return true;
}

// close and finish recording
bool FishinoRecorder::close(void)
{
	// do nothing if already closed
	if(!_writer)
		return true;
	
	// grab some info from writer before deleting it
	getNumSamples();
	getMaxSkippedSamples();
	getTotalSkippedSamples();

	// stop recording
	_i2s->stopReception();
	_recording = false;

	// free the writer
	delete(_writer);
	_writer = NULL;
	
	// power off the codec
	_codec->power(AudioCodec::POWEROFF);
	
	return true;
}
	
// play a file from SD card
bool FishinoRecorder::record(const char *fileName)
{
	// if a filename is given, start a new record
	if(fileName)
		close();
	
	// if not opened, we need to do it first
	if(!_writer && !open(fileName))
		return false;
	
	if(_debugStream)
		*_debugStream << "RECORD STARTED OK!\n";
	// from now, it's opened

	// start recording!
	_recording = true;
	_i2s->startReception();

	return true;
}

// pause the recording
bool FishinoRecorder::pause(void)
{
	// stop recording
	_i2s->stopReception();
	_recording = false;
	return true;
}

// stop recording and close file on SD card
bool FishinoRecorder::stop(void)
{
	// close and terminate
	// (same as close() )
	close();
	return true;
}

// change the input gain
bool FishinoRecorder::setMicGain(float vol, float bal)
{
	return _codec->volume(AudioCodec::MICSINPUTGAIN, vol, bal);
}

// change the input volume
bool FishinoRecorder::setInputVolume(float vol, float bal)
{
	return _codec->volume(AudioCodec::ADCDIGITALVOL, vol, bal);
}

// get some info from writer
uint32_t FishinoRecorder::getNumSamples(void)
{
	if(_writer)
		_numSamples = _writer->getNumSamples();
	return _numSamples;
}

uint32_t FishinoRecorder::getMaxSkippedSamples(void)
{
	if(_writer)
		_maxSkippedSamples = _writer->getMaxSkippedSamples();
	return _maxSkippedSamples;
}

uint32_t FishinoRecorder::getTotalSkippedSamples(void)
{
	if(_writer)
		_totalSkippedSamples = _writer->getTotalSkippedSamples();
	return _totalSkippedSamples;
}

// enable/disable debug logs
void FishinoRecorder::debug(Stream &s)
{
	_debugStream = &s;
}

void FishinoRecorder::noDebug(void)
{
	_debugStream = NULL;
}
