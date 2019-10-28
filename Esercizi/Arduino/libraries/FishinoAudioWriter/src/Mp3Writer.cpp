//////////////////////////////////////////////////////////////////////////////////////
//																					//
//										Mp3Writer.cpp								//
//							MP3 File Audio Writer Interface							//
//																					//
//	Copyright(C) 2016 by Massimo Del Fedele. All right reserved.					//
//																					//
//	This library is free software; you can redistribute it and/or					//
//	modify it under the terms of the GNU Lesser General Public						//
//	License as published by the Free Software Foundation; either					//
//	version 2.1 of the License, or (at your option) any later version.				//
//																					//
//	This library is distributed in the hope that it will be useful,					//
//	but WITHOUT ANY WARRANTY; without even the implied warranty of					//
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU				//
//	Lesser General Public License for more details.									//
//																					//
//	You should have received a copy of the GNU Lesser General Public				//
//	License along with this library; if not, write to the Free Software				//
//	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA		//
//																					//
//	Version 7.3.1	24/12/2017	Fixed mips16 compilation issues						//
//////////////////////////////////////////////////////////////////////////////////////

#include "FishinoAudioWriter.h"

#ifndef ARDUINO_STREAMING
#define ARDUINO_STREAMING
template<class T> inline Print &operator <<(Print &stream, T arg)
{
	stream.print(arg);
	return stream;
}
#endif

// shine engine initialization and termination
// (encloses it inside _shineInitialized flag)
bool Mp3Writer::openShine(void)
{
	if(_debugStream)
		*_debugStream << "Openshine\n";
	if(!_shineInitialized)
	{
		_shine = shine_initialise(&_shineConfig);
		if(_shine)
			_shineInitialized = true;
		else if(_debugStream)
			*_debugStream << "Ooops... error opening shine\n";
	}
	return true;
}

bool Mp3Writer::closeShine(void)
{
	if(_debugStream)
		*_debugStream  << "CloseShine\n";
	if(_shineInitialized)
	{
		shine_close(_shine);
		_shineInitialized = false;
	}
	return true;
}

// write trailer data
// needed by some encoders, for example Mp3
// redefine only if needed
bool Mp3Writer::writeTrailer(void)
{
	int written;

	// Flush and write remaining data.
	uint8_t *data = shine_flush(_shine, &written);
	outStreamWrite(data, written);

	// Close encoder
	return closeShine();
}

// fill buffer -- called by timer interrupt handler
bool Mp3Writer::flushBuffer(uint8_t bufToFlush)
{
	uint8_t *buf = _buf[bufToFlush];
	uint32_t siz = _bufLen[bufToFlush];
	uint8_t *bufEnd = buf + siz;
	bool res = true;
	
	uint32_t tim = 0;
	if(_debugStream)
	{
		Serial << "Flushing buffer " << bufToFlush << " size is " << siz << "\n";
		Serial << "Samples per pass = " << _samplesPerPass << "\n";
		Serial << "Bytes per pass   = " << _samplesPerPass * (_bits / 8) * (_channelMode == CHANMODE_STEREO ? 2 : 1) << "\n";
		tim = millis();
	}

	int written;
	uint8_t *data;
	while(buf < bufEnd)
	{
		data = shine_encode_buffer_interleaved(_shine, (int16_t *)buf, &written);
		if(_debugStream)
			*_debugStream << "Encoded " << written << " bytes\n";
		if(outStreamWrite(data, written) != (uint32_t)written)
		{
			res = false;
			if(_debugStream)
				*_debugStream << "write error...aborting\n";
			break;
		}
		buf += _samplesPerPass * (_bits / 8) * (_channelMode == CHANMODE_STEREO ? 2 : 1);
	}

	if(_debugStream)
	{
		tim = millis() - tim;
		Serial << "Flushed in " << tim << "mSec\n";
	}
	_bufLen[bufToFlush] = 0;
	_bufFull[bufToFlush] = false;
	return res;
}

// for some encoders, we need a particular number of samples
// stored inside the buffer(s), so we shall ensure that
// this integral number fits inside. We provide a function
// that rounds, if needed, the AUDIOWRITER_BUFSIZE
// this function can be redefined on derived classes
uint32_t Mp3Writer::audioWriterRoundSize(void)
{
	// round buffer size to be a multiple of shine samples per pass
	_samplesPerPass = shine_samples_per_pass(_shine);
	uint32_t samplesSize = _samplesPerPass * (_bits / 8) * (_channelMode == CHANMODE_STEREO ? 2 : 1);
	_passesPerBuffer = AUDIOWRITER_BUFSIZE / samplesSize;
	Serial << "Rounded buffer to " << _passesPerBuffer << " blocks of " << _samplesPerPass << " mp3 samples, total " << _passesPerBuffer * samplesSize << "\n";
	return _passesPerBuffer * samplesSize;
}

// signal update on audio parametera
// needed when changing them, on some codecs
// redefine if needed
void Mp3Writer::audioParamsUpdated(void)
{
	// we accept ONLY 16 bit samples, so adjust it
	// forcing it to 16 bits
	// we could try to improve it later
	_bits = 16;

	// setup mpeg parameters
	_shineConfig.mpeg.bitr = _mp3BitRate;
	_shineConfig.mpeg.mode = (_channelMode == CHANMODE_STEREO ? JOINT_STEREO : MONO);

	// setup wav parameters
	_shineConfig.wave.samplerate = _sampleRate;
	_shineConfig.wave.channels = (_channelMode == CHANMODE_STEREO ? PCM_STEREO : PCM_MONO);
	
	// close and re-open shine in order to re-fetch the updated params
	closeShine();
	openShine();
}

// constructor -- takes a file path
Mp3Writer::Mp3Writer(const char *path)
{
	// defaults to 128kbps bit rate
	_mp3BitRate = 128;
	_shineConfig.mpeg.bitr = _mp3BitRate;
	
	// sets default shine configuration parameters
	shine_set_config_mpeg_defaults(&_shineConfig.mpeg);
		
	// we need to open shine early, because AudioWriter class
	// needs to have some data from it early
	// we'll close and re-open it if we need to change params
	_shineInitialized = false;
	openShine();

	open(path);
}

// empty constructor, must be followed by open()
Mp3Writer::Mp3Writer()
{
	// defaults to 128kbps bit rate
	_mp3BitRate = 128;
	_shineConfig.mpeg.bitr = _mp3BitRate;

	// sets default shine configuration parameters
	shine_set_config_mpeg_defaults(&_shineConfig.mpeg);
		
	// we need to open shine early, because AudioWriter class
	// needs to have some data from it early
	// we'll close and re-open it if we need to change params
	_shineInitialized = false;
	openShine();
}

// destructor
Mp3Writer::~Mp3Writer()
{
	close();

	// just in case, if something went wrong
	closeShine();
}

// opens a file
bool Mp3Writer::open(const char *path)
{
	close();

	if (path)
		_path = path;
	if (!_path)
		return false;

	_file = SD.open(_path, O_WRITE | O_CREAT | O_TRUNC);
	if (_file)
	{
		// mark stream as valid
		_valid = true;
		return true;
	}
	if(_debugStream)
		*_debugStream << "woops... error opening writer\n";
	reset();
	return false;
}
