//////////////////////////////////////////////////////////////////////////////////////
//																					//
//								FishinoAudioWriter.cpp								//
//							Generic File Audio Writer Interface						//
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

#include <Timer.h>

#ifndef ARDUINO_STREAMING
#define ARDUINO_STREAMING
template<class T> inline Print &operator <<(Print &stream, T arg)
{
	stream.print(arg);
	return stream;
}
#endif

// using Timer4 to flush buffers
#define AUDIOWRITER_TIMER Timer4

// timer interrupt handler used to flush buffers
void _audiowriter_timer_handler(AudioWriter &writer)
{
	static bool reentered = false;
	
	// stop the timer
	AUDIOWRITER_TIMER.stop();
	
	if(reentered && writer._debugStream)
		*writer._debugStream << "Ops... audiowriter re-entered\n";
	reentered = true;
	
	// get the buffer to flush
	uint8_t flushBuf = 1 - writer._curBuf;

	// fill the buffer
	/*bool res = */writer.flushBuffer(flushBuf);
	writer._bufLen[flushBuf] = 0;
	
	// reset the timer
	AUDIOWRITER_TIMER.reset();
	
	reentered = false;
}

// resets state
void AudioWriter::reset(void)
{
	_bufFull[0] = _bufFull[1] = false;
	_bufLen[0] = _bufLen[1] = 0;
	
	_numSamples = 0;
	
	_bufPtr = _buf[0];
	_bufEnd = _bufPtr + _audiowriter_roundsize;
	_curBuf = 0;
	
	_skipSamples = false;
	_channel = false;
	
	_valid = true;
	
	_totalSkippedSamples = 0;
	_maxSkippedSamples = 0;
}

// write some integer types
bool AudioWriter::outStreamWrite8(uint8_t data)
{
	return outStreamWrite(&data, 1) == 1;
}

bool AudioWriter::outStreamWrite16(uint16_t data)
{
	return outStreamWrite((uint8_t *)&data, 2) == 2;
}

bool AudioWriter::outStreamWrite24(uint32_t data)
{
	return outStreamWrite((uint8_t *)&data, 3) == 3;
}

bool AudioWriter::outStreamWrite32(uint32_t data)
{
	return outStreamWrite((uint8_t *)&data, 4) == 4;
}

// finalize flushing audio buffers on stream
void AudioWriter::finalizeBuffers()
{
	// stop the flush timer, in case it's in the middle of a flush
	AUDIOWRITER_TIMER.stop();
	
	// don't do on buffer overrun states
	if(!_valid)
		return;

	uint32_t len = 0, tim = 0;
	if(_debugStream)
	{
		len = _bufLen[0] + _bufLen[1];
		tim = millis();
	}
	
	// flush both buffers
	flushBuffer(1 - _curBuf);
	flushBuffer(_curBuf);
	
	// write out updated header
	writeHeader();
	
	if(_debugStream)
	{
		tim = millis() - tim + 1;
		*_debugStream << "Buffers flushed in " << tim << "ms, speed is " << len * 1000 / tim << "\n";
	}

	// write trailer, if needed
	writeTrailer();
}

// empty constructor
AudioWriter::AudioWriter()
{
	_buf[0] = (uint8_t *)malloc(AUDIOWRITER_BUFSIZE);
	_buf[1] = (uint8_t *)malloc(AUDIOWRITER_BUFSIZE);

	// setup default audio params
	_bits = 16;
	_sampleRate = 44100;
	_channelMode = CHANMODE_STEREO;

	// defaults with non-rounded buffer size
	_audiowriter_roundsize	= AUDIOWRITER_BUFSIZE;
	
	// reset buffers and samples
	reset();
	
	// setup timer that will be used
	// to flush audio buffers
	
	// reset the timer, just in case
	AUDIOWRITER_TIMER.reset();
	
	// we want a call every millisecond (1 KHz frequency)
//	AUDIOWRITER_TIMER.setFrequency(1000);
	AUDIOWRITER_TIMER.setFrequency(5000);
	
	// low priority -- I2S must be able to interrupt it
	AUDIOWRITER_TIMER.setInterruptPriority(2);

	// Link in out ISR routine.
	AUDIOWRITER_TIMER.attachInterrupt(_audiowriter_timer_handler, *this);

	// no debug stream
	_debugStream = NULL;
}

// destructor
AudioWriter::~AudioWriter()
{
	// free buffers
	if (_buf[0])
		free(_buf[0]);
	if (_buf[1])
		free(_buf[1]);
	_buf[0] = _buf[1] = NULL;
	
	reset();
}

// set audio parameters
// may be ONLY called on idle recorder
bool AudioWriter::setAudioParams(uint32_t sampleRate, uint8_t bits, ChannelModes mode)
{
	// don't change sample rate if we've already collected some
	if(_numSamples)
		return false;
	
	// setup parameters
	_sampleRate = sampleRate;
	_bits = bits;
	_channelMode = mode;
	
	// signal that we updated audio parameters
	audioParamsUpdated();
	
	// round the audio buffer length to encoder required size
	// (needed for mp3 encoder, for example)
	_audiowriter_roundsize = audioWriterRoundSize();
	reset();

	return true;
}

// put next sample on buffers
// this is a quick function that CAN be called from inside an interrupt handler
bool AudioWriter::nextSample(uint32_t sample)
{
	// check if we're skipping samples because of full buffer
	// beware, always start with left channel sample, to not mix them!
	if(_skipSamples)
	{
		if(!_bufFull[_curBuf] && !_channel)
		{
			if(_maxSkippedSamples < _curSkippedSamples)
				_maxSkippedSamples = _curSkippedSamples;
			_totalSkippedSamples += _curSkippedSamples;
			_skipSamples = false;
		}
		else
		{
			_channel = !_channel;
			_curSkippedSamples++;
			return false;
		}
	}
	_channel = !_channel;
		
	uint8_t s8;
	
	// build the sample to store
	switch(_bits)
	{
		case 8:
			// o bits are stored as unsigned data
			s8 = (uint8_t)(sample >> 16);
			s8 += 128;
			*_bufPtr++ = s8;
			_bufLen[_curBuf]++;
			break;
			
		case 16:
			*_bufPtr++ = (uint8_t)(sample >> 8);
			*_bufPtr++ = (uint8_t)(sample >> 16);
			_bufLen[_curBuf] += 2;
			break;
			
		case 24:
			*_bufPtr++ = (uint8_t)sample;
			*_bufPtr++ = (uint8_t)(sample >> 8);
			*_bufPtr++ = (uint8_t)(sample >> 16);
			_bufLen[_curBuf] += 3;
			break;
			
		default:
			*_bufPtr++ = (uint8_t)sample;
			*_bufPtr++ = (uint8_t)(sample >> 8);
			*_bufPtr++ = (uint8_t)(sample >> 16);
			*_bufPtr++ = 0;
			_bufLen[_curBuf] += 4;
			break;
	}
	
	// increment number of samples;
	_numSamples++;
	
	// check if we got at end of running buffer
	if(_bufPtr >= _bufEnd)
	{
		// one buffer finished, we shall switch to second one
		_bufFull[_curBuf] = true;
		_curBuf = 1 - _curBuf;
		_bufPtr = _buf[_curBuf];
		_bufEnd = _bufPtr + _audiowriter_roundsize;
		
		// if second buffer is also full we've got a problem
		// instead of abort writing data, we simply skip samples
		// up to the buffer is freed
		// just set error state and return
		if(_bufFull[_curBuf])
		{
			_skipSamples = true;
			_curSkippedSamples = 0;
		}

		// all ok, we shall flush the full buffer
		// we can't do it directly here, as it would take too much time
		// and we'll loose audio samples
		// so we run a timed interrupt routine to do the job
		// it's important that routine's priority is LOWER than the
		// one used by I2S handler, which is _I2S_IPL
		AUDIOWRITER_TIMER.start();
	}
	return true;
}

// enable/disable debug logs
void AudioWriter::debug(Stream &s)
{
	_debugStream = &s;
}

void AudioWriter::noDebug(void)
{
	_debugStream = NULL;
}
