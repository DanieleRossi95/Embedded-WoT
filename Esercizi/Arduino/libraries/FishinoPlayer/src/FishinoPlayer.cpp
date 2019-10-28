//////////////////////////////////////////////////////////////////////////////////////
//																					//
//							FishinoPlayer.cpp										//
//						An audio player class										//
//		Can play audio streams from SD files or HTTP/memory streams					//
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
//  Version 6.0.2	June 2017	Some small fixes									//
//  Version 6.0.4	June 2017	Skip folders and system files						//
//																					//
//////////////////////////////////////////////////////////////////////////////////////

#include "FishinoPlayer.h"

#include <FishinoFileStream.h>

//#define DEBUG_LEVEL_INFO
#include <FishinoDebug.h>


// I2S interrupt handler -- handles data requests from controller
void __fishinoPlayer__i2sHandler__(FishinoPlayerClass &p)
{
	p.handleI2SRequest();
}

// handle i2s requests
void FishinoPlayerClass::handleI2SRequest(void)
{
	if(_reader->available())
		_i2s->send24(_reader->nextSample());
	else
	{
		DEBUG_INFO("PLAY STOPPED BY READER\n");
		_i2s->stopTransmission();
		
		// stops and free resources
		freeResources();

	}
}

// free player resources when done
void FishinoPlayerClass::freeResources(void)
{
	// signal play end
	_playing = false;
	
	// not paused
	_paused = false;
	
	// stop i2s transmission if still active
	if(_i2s)
		_i2s->stopTransmission();
		
	// frees reader
	if(_reader)
	{
		_reader->end();
		delete _reader;
		_reader = NULL;
	}
	
	// reset audio codec
	// @@ we can't reset codec here... we're in an high priority
	// interrupt task, higher than I2C one.
	// using I2C here would hang the controller
/*
	if(_codec)
		_codec->reset();
*/
	
	// close the stream
	if(_stream && _ownStream)
		delete _stream;
	_stream = NULL;
	_ownStream = false;
}

// constructor
FishinoPlayerClass::FishinoPlayerClass()
{
	// defaults to I2S device and ALC5631 codec
	_i2s = &I2S;
	_codec = &ALC5631;
	
	_stream = NULL;
	_ownStream = false;
	
	_reader = NULL;
	_playing = false;
	_paused = false;
	
	_vol = 0.2;
	_bal = 0;
}
	
// destructor
FishinoPlayerClass::~FishinoPlayerClass()
{
	freeResources();
}
	
// change default devices
void FishinoPlayerClass::setDevices(I2SClass &i2s, AudioCodec &codec)
{
	_i2s = &i2s;
	_codec = &codec;
}

// play from a FishinoStream object
bool FishinoPlayerClass::play(FishinoStream &stream)
{
	// if still playing, stop it
	if(_playing)
		stop();
		
	// store the stream pointer
	_stream = &stream;
	
	// identify audio media if it's possible and create corresponding reader
	uint8_t *buf = (uint8_t *)malloc(2000);
	if(!buf)
	{
		DEBUG_ERROR("Can't create identify buffer\n");
		freeResources();
		return false;
	}
	
	uint32_t siz = stream.peekBuffer(buf, 2000);
	if(!siz)
	{
		DEBUG_ERROR("Stream is not buffered, can't identify media\n");
		free(buf);
		freeResources();
		return false;
	}
	
	AudioMedia media = AudioReader::identify(buf, siz);
	free(buf);
	_reader = NULL;
	switch(media)
	{
		case MEDIA_UNKNOWN:
		default:
			DEBUG_ERROR("Unknown audio stream\n");
			return false;
			
		case MEDIA_WAV:
			DEBUG_INFO("Uncompressed WAV media\n");
			_reader = new WavReader(stream);
			break;
			
		case MEDIA_MP3:
			DEBUG_INFO("MP3 media\n");
			_reader = new Mp3Reader(stream);
			break;

		case MEDIA_VORBIS:
			DEBUG_INFO("Ogg/Vorbis media\n");
			_reader = new OggReader(stream);
			break;

		case MEDIA_OPUS:
			DEBUG_INFO("Ogg/Opus media\n");
			_reader = new OpusReader(stream);
			break;
	}
	if(!_reader)
	{
		DEBUG_ERROR("Couldn't create audio reader\n");
		freeResources();
		return false;
	}
	if(!_reader->begin())
	{
		DEBUG_ERROR("Error starting reader\n");
		freeResources();
		return false;
	}
	
	// initialize audio codec
	_codec->reset();
	
	// power on codec for play DAC data
	_codec->power(AudioCodec::STEREOSPEAKERS);
	
	// setup PATH
	_codec->path(AudioCodec::DAC2STEREOSPEAKERS);
	
	// lower volume
	_codec->volume(AudioCodec::SPEAKERSVOL, _vol, _bal);
	
	// setup I2S handler
	_i2s->setTransmitHandler(__fishinoPlayer__i2sHandler__, *this);
	
	// set I2S audio parameters
	uint32_t sampleRate = _reader->getSampleRate();
	uint8_t bits = _reader->getBits();
	bool stereo = _reader->isStereo();

	DEBUG_INFO("Audio parameters : sample rate = %lu, bits = %d, %s\n", sampleRate, bits, stereo ? "STEREO" : "MONO");

	_i2s->setAudioParams(sampleRate, bits, stereo ? CHANMODE_STEREO : CHANMODE_MONO);
	
	// start playing!
	_playing = true;
	_i2s->startTransmission();
	
	return true;
}

// play a file from SD card
bool FishinoPlayerClass::play(const char *fileName)
{
	// if still playing, stop it
	if(_playing)
		stop();
		
	_stream = NULL;

	// create and open a file stream
	FishinoFileStream *stream = new FishinoFileStream;
	if(!stream)
	{
		DEBUG_ERROR("Couldn't create file stream\n");
		freeResources();
		return false;
	}
	
	// open the stream
	if(!stream->open(fileName, O_READ))
	{
		DEBUG_ERROR("Couldn't open file\n");
		delete stream;
		freeResources();
		return false;
	}
	
	// from now stream is owned by us
	_ownStream = true;
	
	// continue processing with the opened stream
	return play(*stream);
}

// pause play
bool FishinoPlayerClass::pause(void)
{
	if(!_playing || _paused)
		return false;
	_i2s->stopTransmission();
	_paused = true;
	return true;
}

// resume play
bool FishinoPlayerClass::resume(void)
{
	if(!_playing || !_paused)
		return false;
	if(_reader && _reader->available())
	{
		_paused = false;
		_i2s->startTransmission();
		return true;
	}
	// some error occurred, stop and free
	stop();
	return false;
}

// stop playing current file and free resources
bool FishinoPlayerClass::stop(void)
{
	if(_playing)
		_i2s->stopTransmission();
	
	// stops and free resources
	freeResources();

	return true;
}

// change the volume
bool FishinoPlayerClass::volume(double vol, double bal)
{
	_vol = vol;
	_bal = bal;
	return true;
}

FishinoPlayerClass &__GetFishinoPlayer(void)
{
	static FishinoPlayerClass player;
	return player;
}
