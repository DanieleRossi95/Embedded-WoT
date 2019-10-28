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

#ifndef __AUDIOWRITER_H
#define __AUDIOWRITER_H

#include <Arduino.h>

#ifndef __AUDIO_DEFS
#define __AUDIO_DEFS
enum ChannelModes
{
	CHANMODE_MONO		= 0,
	CHANMODE_MONO_BOTH	= 0,
	CHANMODE_MONO_LEFT	= 1,
	CHANMODE_MONO_RIGHT	= 2,
	CHANMODE_STEREO		= 3
};
#endif

// Audio buffers sizing:
// we use a size that can fit an integral number of audio records
// so multiple of 4x3x2 = 24 bytes. As we shall keep some 0.1 seconds
// of sound buffered, this is around 4400 samples, so max 17600 bytes
// rounding to 24, that gives 2 buffers of 8832 bytes each
#define AUDIOWRITER_BUFSIZE	8832

class AudioWriter
{
	friend void _audiowriter_timer_handler(AudioWriter &writer);
	
	public:
	
	private:
	
	protected:
	
		// for some encoders, we need a particular number of samples
		// stored inside the buffer(s), so we shall ensure that
		// this integral number fits inside. We provide a function
		// that rounds, if needed, the AUDIOWRITER_BUFSIZE
		// this function can be redefined on derived classes
		uint32_t _audiowriter_roundsize;
		virtual uint32_t audioWriterRoundSize(void) { return AUDIOWRITER_BUFSIZE; }
		
		// signal update on audio parametera
		// needed when changing them, on some codecs
		// redefine if needed
		virtual void audioParamsUpdated(void) {}
	
		// valid flag -- gets reset on buffers full
		// or other kind of errors
		bool _valid;
		
		// buffer full flag
		// used to skip samples till the buffer(s) are free again
		volatile bool _skipSamples;
		
		// count of skipped samples
		uint32_t _curSkippedSamples;
		uint32_t _totalSkippedSamples;
		uint32_t _maxSkippedSamples;
		
		// current channel (toggles between left-false and right, true)
		volatile bool _channel;
	
		// bits per sample
		uint8_t _bits;
		
		// sample rate
		uint32_t _sampleRate;
		
		// channel mode
		ChannelModes _channelMode;
		
		// write header on output stream -- this may imply a seek
		// after writing must reposition on end of data stream
		// to be redefined on derived classes
		virtual bool writeHeader(void) = 0;
		
		// write trailer data
		// needed by some encoders, for example Mp3
		// redefine only if needed
		virtual bool writeTrailer(void) { return true; }
		
		// seeks output file at a given position
		virtual bool outStreamSeek(uint32_t pos) = 0;
		
		// gets current output stream position
		virtual uint32_t outStreamPosition(void) = 0;
		
		// get size of output stream
		virtual uint32_t outStreamSize(void) = 0;
		
		// write a number of bytes from a buffer
		// return number of bytes actually written
		// to be redefined on derived classes
		virtual uint32_t outStreamWrite(uint8_t *buf, uint32_t count) = 0;
		
		// reset the writer
		void reset(void);
		
		// write some integer types
		bool outStreamWrite8(uint8_t data);
		bool outStreamWrite16(uint16_t data);
		bool outStreamWrite24(uint32_t data);
		bool outStreamWrite32(uint32_t data);
	
		// double buffer for sampled data
		uint8_t *_buf[2];
		
		// actual buffer data lengths
		uint32_t _bufLen[2];

		// currently recording buffer
		uint8_t _curBuf;
		
		// byte position inside it
		uint8_t *_bufPtr;
		uint8_t *_bufEnd;
		
		// flags signaling full buffer(s)
		// if both get full, we're late with data flushing
		// so we shall stop recording and report error
		bool _bufFull[2];

		// total number of samples at the moment
		uint32_t _numSamples;
		
		// flush buffer -- called by timer interrupt handler
		// to be redefined in derived classes
		virtual bool flushBuffer(uint8_t bufToFlush) = 0;
		
		// finalize flushing audio buffers on stream
		void finalizeBuffers();

		// debugger channel
		Stream *_debugStream;

	public:
	
		// empty constructor
		AudioWriter();
		
		// destructor
		virtual ~AudioWriter();
		
		// check if writer is valid
		bool isValid(void) { return _valid; }

		// get bits per sample
		uint8_t getBits(void) { return _bits; }
		
		// get sample rate
		uint32_t getSampleRate(void) { return _sampleRate; }
		
		// get stereo mode
		bool isStereo(void) { return _channelMode == CHANMODE_STEREO; }
		
		// set audio parameters
		// may be ONLY called on idle recorder
		bool setAudioParams(uint32_t sampleRate, uint8_t bits, ChannelModes mode);
		
		// return number of recorded samples (NOT bytes!)
		uint32_t getNumSamples(void) { return _numSamples; }
		
		// put next sample on buffers
		// this is a quick function that CAN be called from inside an interrupt handler
		bool nextSample(uint32_t sample);
		
		// query for skipped samples count
		uint32_t getTotalSkippedSamples(void) { return _totalSkippedSamples; }
		uint32_t getMaxSkippedSamples(void) { return _maxSkippedSamples; }

		// enable/disable debug logs
		void debug(Stream &s = Serial);
		void noDebug(void);
};

#include "FileAudioWriter.h"
#include "WavWriter.h"
#include "Mp3Writer.h"

#endif
