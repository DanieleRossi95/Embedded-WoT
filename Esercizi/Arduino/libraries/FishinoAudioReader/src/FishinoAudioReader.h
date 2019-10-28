//////////////////////////////////////////////////////////////////////////////////////
//																					//
//							FishinoAudioReader.h									//
//						Generic Audio Reader Interface								//
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
//  Version 6.0.3	June 2017	Relax Mp3 identification requirements				//
//  Version 6.0.4	June 2017	Some small fixes									//
//  Version 6.0.5	June 2017	Fixed MP3 for weird files							//
//  Version 7.3.1	24/12/2017	Fixed mips16 compilation issues						//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FISHINOAUDIOREADER_H
#define _FISHINOAUDIOREADER_H

#include <FishinoStream.h>

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

// supported audio media
enum AudioMedia
{
	MEDIA_UNKNOWN		= 0,
	MEDIA_WAV			= 1,
	MEDIA_MP3			= 2,
	MEDIA_VORBIS		= 3,
	MEDIA_OPUS			= 4
};

// Audio buffers sizing:
// we use a size that can fit an integral number of audio records
// so multiple of 4x3x2 = 24 bytes. As we shall keep some 0.1 seconds
// of sound buffered, this is around 4400 samples, so max 17600 bytes
// rounding to 24, that gives 2 buffers of 8832 bytes each
#define AUDIOREADER_BUFSIZE	8832

// generic audio stream reader class
class AudioReader
{
	friend void _audioreader_timer_handler(AudioReader &reader);
	
	private:
	
		// flags signaling empty buffer(s)
		// if both get empty, we're late with data reading
		// so we shall stop playing
		bool _bufEmpty[2];

		// a valid (accepted) wav
		bool _valid;
		
		// end of file state
		bool _eof;
		
		// both buffers empty, shall refill
		bool _bothEmpty;
		
		// in timer handler flag
		// avoid re-entering
		bool _inHandler;
		
		// allocate data buffers
		bool allocBuffers(void);
		
		// free data buffers
		void freeBuffers(void);
		
		// prefill audio buffers
		bool preFillBuffers();

		// double buffer for sample data
		// ensude buffers are 4 byte aligned
		uint32_t *_buf[2];
		
		// actual buffer data lengths
		uint32_t _bufLen[2];

		// currently playing buffer
		uint8_t _curBuf;
		
		// byte position inside it
		uint8_t *_bufPtr;
		uint8_t *_bufEnd;
		
	protected:
	
		// the attached stream
		FishinoStream &_stream;
	
		// get the number of bytes still available on input stream
		inline bool inStreamAvailable(void) { return _stream.available(); }
		
		// skip a number of bytes on stream
		inline bool inStreamSkip(uint32_t nBytes) { return _stream.skip(nBytes); }
		
		// read a number of bytes into a buffer
		inline uint32_t inStreamRead(uint8_t *buf, uint32_t count) { return _stream.read(buf, count); }
		
		// restarts input stream
		inline void inStreamRestart(void) { _stream.seek(0, SEEK_SET); }
		
		// seeks the input stream
		// returns false on error or if seek is not supported
		inline bool inStreamSeek(int32_t offset, int whence) { return _stream.seek(offset, whence); }
		
		// tell current stream position
		inline uint32_t inStreamTell(void) { return _stream.tell(); }
		
		// read some integer types
		// (those should belong to stream class too...)
		uint8_t inStreamRead8(void);
		uint16_t inStreamRead16(void);
		uint32_t inStreamRead24(void);
		uint32_t inStreamRead32(void);
	
		// bits per sample
		uint8_t _bits;
		
		// sample rate
		uint32_t _sampleRate;
		
		// stereo mode
		bool _stereo;
		
		// initialize internal reader data
		// to be redefined in derived classes
		virtual bool initialize(void) = 0;
		
		// finalize reader data
		// to be redefined in derived classes
		virtual void finalize(void) = 0;

		// get decoded samples -- called by timer interrupt handler
		// to be redefined in derived classes
		virtual uint32_t getSamples(uint32_t *buf, uint32_t len) = 0;
		
		// sets EOF flag -- must be done inside codec
		// because he knows when data is over
		void setEof(void) { _eof = true; }
		
	public:
	
		// constructor - takes an input stream as parameter
		AudioReader(FishinoStream &s);
		
		// destructor
		virtual ~AudioReader();
		
		// starts processing data
		bool begin(void);
		
		// terminate processing and free resources
		// it gets automatically called on eof
		// or it can be called externally
		void end(void);

		// check if file has a valid format
		bool isValid(void);
		bool isEof(void);
		operator bool() { return _valid && !_eof; }
		bool operator!() { return !_valid || _eof; }

		// bits per sample
		uint8_t getBits(void) { return _bits; }
		
		// sample rate
		uint32_t getSampleRate(void) { return _sampleRate; }
		
		// stereo mode
		bool isStereo(void) { return _stereo; }
		
		// check if data is available
		bool available(void);
		
		// get next sample data from stream
		// this is a quick function that CAN be called from inside an interrupt handler
		uint32_t nextSample(void);
		
		// try to identify media by some initial data
		// you must supply a buffer with enough bytes to identify the file
		static AudioMedia identify(uint8_t const *buf, uint32_t len);
};

#include "FishinoWavReader.h"
#include "FishinoMp3Reader.h"
#include "FishinoOggReader.h"
#include "FishinoOpusReader.h"

#endif
