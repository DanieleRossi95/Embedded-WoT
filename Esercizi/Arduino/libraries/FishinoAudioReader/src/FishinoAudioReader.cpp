//////////////////////////////////////////////////////////////////////////////////////
//																					//
//							FishinoAudioReader.cpp									//
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

//#define DEBUG_LEVEL_INFO
#define DEBUG_LEVEL_WARNING
#include "FishinoDebug.h"

#include "FishinoAudioReader.h"

#include <Timer.h>

#ifndef ARDUINO_STREAMING
#define ARDUINO_STREAMING
template<class T> inline Print &operator <<(Print &stream, T arg)
{
	stream.print(arg);
	return stream;
}
#endif

// using Timer4 to fill buffers
#define AUDIOREADER_TIMER Timer4

// timer frequency
#define AUDIOREADER_TIMER_FREQUENCY		5000

// timer interrupt handler used to fill buffers
void _audioreader_timer_handler(AudioReader &reader)
{
//	DEBUG_INFO("Timer called\n");

	// avoid re-entering
	if(reader._inHandler)
	{
		DEBUG_WARNING("Try to re-enter timer callback\n");
		return;
	}

	reader._inHandler = true;

	// stop the timer
	AUDIOREADER_TIMER.stop();
	
	// if out of data, just stop
	if(!reader.available())
	{
/*
		DEBUG_WARNING("No data available\n");
		reader._inHandler = false;
		return;
*/

		DEBUG_INFO("EOF\n");
		AUDIOREADER_TIMER.stop();
		reader._inHandler = false;
		reader._eof = true;
		return;
	}
	
	// if both buffers are empty, just refill them both
	// should not happen, but can, on slow http streams
	if(reader._bothEmpty)
	{
		DEBUG_WARNING("Both buffers empty\n");
		reader.preFillBuffers();
		reader._inHandler = false;
		return;
	}
	
	// get the buffer to fill
	uint8_t bufToFill = 1 - reader._curBuf;
	uint32_t *buf = reader._buf[bufToFill];

	// fill the buffer if needed
	if(reader._bufEmpty[bufToFill])
	{
		uint32_t siz = reader.getSamples(buf, AUDIOREADER_BUFSIZE);
		reader._bufLen[bufToFill] = siz;
		if(!siz)
		{
			DEBUG_INFO("EOF\n");
			AUDIOREADER_TIMER.stop();
			reader._eof = true;
			
			// terminate the reader here
			reader._inHandler = false;
			return;
		}
		
		reader._bufEmpty[bufToFill] = false;
//		DEBUG_INFO("Got %d bytes of samples\n", siz);
	}
	
	// reset the timer
	AUDIOREADER_TIMER.reset();

	reader._inHandler = false;
}

// allocate data buffers
bool AudioReader::allocBuffers(void)
{
	_bufEmpty[0] = _bufEmpty[1] = true;
	_bufLen[0] = _bufLen[1] = 0;
	_bufPtr = (uint8_t *)_buf[0];
	_bufEnd = _bufPtr;
	_curBuf = 0;

	_buf[0] = (uint32_t *)malloc(AUDIOREADER_BUFSIZE);
	if(!_buf[0])
		return false;

	_buf[1] = (uint32_t *)malloc(AUDIOREADER_BUFSIZE);
	if(!_buf[1])
	{
		free(_buf[0]);
		_buf[0] = NULL;
		return false;
	}
	return true;
}

// free data buffers
void AudioReader::freeBuffers(void)
{
	for(int i = 0; i < 2; i++)
	{
		if(_buf[i])
		{
			free(_buf[i]);
			_buf[i] = NULL;
		}
	}
}
		
// prefill audio buffers
bool AudioReader::preFillBuffers()
{
#ifdef DEBUG_LEVEL_INFO
	uint32_t tim = 0;
	
		tim = millis();
#endif
	_bufLen[0] = getSamples(_buf[0], AUDIOREADER_BUFSIZE);
	_bufLen[1] = getSamples(_buf[1], AUDIOREADER_BUFSIZE);
	if(!_bufLen[0] && !_bufLen[1])
		return false;
	
	_bufEmpty[0] = (_bufLen[0] == 0);
	_bufEmpty[1] = (_bufLen[1] == 0);

	_bufPtr = (uint8_t *)_buf[0];
	_bufEnd = _bufPtr + _bufLen[0];
	_curBuf = 0;
	
#ifdef DEBUG_LEVEL_INFO
	tim = millis() - tim;
	DEBUG_INFO("Buffers read in %ld ms, speed is %ld\n", tim, (uint32_t)2 * AUDIOREADER_BUFSIZE * 1000 / tim);
	DEBUG_INFO("Buffer 0 has %d bytes\n", (int)_bufLen[0]);
	DEBUG_INFO("Buffer 1 has %d bytes\n", (int)_bufLen[1]);
#endif

	_bothEmpty = false;
	
	return true;
}

uint8_t AudioReader::inStreamRead8(void)
{
	uint8_t b;
	inStreamRead(&b, 1);
	return b;
}

uint16_t AudioReader::inStreamRead16(void)
{
	uint16_t w;
	inStreamRead((uint8_t *)&w, 2);
	return w;
}

uint32_t AudioReader::inStreamRead24(void)
{
	uint32_t dw = 0;
	inStreamRead((uint8_t *)&dw, 3);
	return dw;
}

uint32_t AudioReader::inStreamRead32(void)
{
	uint32_t dw;
	inStreamRead((uint8_t *)&dw, 4);
	return dw;
}

// empty constructor, must be followed by open()
AudioReader::AudioReader(FishinoStream &s) : _stream(s)
{
	_buf[0] = NULL;
	_buf[1] = NULL;
	
	_bothEmpty = true;
}

// destructor
AudioReader::~AudioReader()
{
	// do NOT call end() here, it must be called
	// on final derived class destructor
	// or it will call bad finalize() function
}

// starts processing data
bool AudioReader::begin(void)
{
	_valid = false;
	_eof = false;
	
	_bits = 0;
	_sampleRate = 0;
	_stereo = false;
	
	// restarts the input stream
	inStreamRestart();
	
	// check if we've got data
	if(!inStreamAvailable())
	{
		DEBUG_ERROR("No audio data available\n");
		return false;
	}
	
	// allocate buffers
	if(!allocBuffers())
	{
		DEBUG_ERROR("Failed to allocate samples buffers\n");
		return false;
	}

	// initialize internal structures
	if(!initialize())
	{
		DEBUG_ERROR("Failed to initialize codec internal structures\n");
		freeBuffers();
		return false;
	}
	
	// prefill both data buffers
	if(!preFillBuffers())
	{
		// if both buffers are empty, there's some problems
		finalize();
		freeBuffers();
		return false;
	}
	
	// all ok, mark reader as valid now
	_valid = true;
	_eof = false;
	
	// not inside timer handler
	_inHandler = false;
	
	// setup timer that will be used
	// to fill audio buffers
	
	// reset the timer, just in case
	AUDIOREADER_TIMER.reset();
	
	// we want a call every millisecond (1 KHz frequency)
	AUDIOREADER_TIMER.setFrequency(AUDIOREADER_TIMER_FREQUENCY);
	
	// low priority -- I2S must be able to interrupt it
	AUDIOREADER_TIMER.setInterruptPriority(2);

	// Link in out ISR routine.
	AUDIOREADER_TIMER.attachInterrupt(_audioreader_timer_handler, *this);

	return true;
}

// terminate processing and free resources
// it gets automatically called on eof
// or it can be called externally
void AudioReader::end(void)
{
	// stop the timer
	AUDIOREADER_TIMER.stop();
	AUDIOREADER_TIMER.detachInterrupt();
	
	// finalize internal structures
	finalize();

	// free buffers
	freeBuffers();
	
	// stream is not valid now
	_eof = true;
	_valid = false;
}

// return number of still available samples (NOT bytes!) on stream
// (stereo samples are counted double!)
bool AudioReader::available(void)
{
	return _valid && !_eof;
}

// get next sample data from stream
// this is a quick function that CAN be called from inside an interrupt
// handler; internally we're doing double buffering of data
uint32_t AudioReader::nextSample(void)
{
	// don't try to read an invalid file
	// or to go past end of it
	if (_bothEmpty || !_valid || _eof)
		return 0;
	
	// build the return sample -- always as a 24 bit signed one
	// other formats get converted here
	uint32_t res;
	switch(_bits)
	{
		case 8:
			res = *_bufPtr++;
			res -= 128;
			res <<= 16;
			break;
			
		case 16:
			res = *_bufPtr++;
			res |= ((uint32_t)*_bufPtr++) << 8;
			res <<= 8;
			break;
			
		case 24:
			res = *_bufPtr++;
			res |= ((uint32_t)*_bufPtr++) << 8;
			res |= ((uint32_t)*_bufPtr++) << 16;
			break;
			
		default:
			// discard LSB
			_bufPtr++;
			res = *_bufPtr++;
			res |= ((uint32_t)*_bufPtr++) << 8;
			res |= ((uint32_t)*_bufPtr++) << 16;
			break;
	}
	
	// check if we got at end of running buffer
	if(_bufPtr >= _bufEnd)
	{
		// one buffer finished, we shall switch to second one
		_bufEmpty[_curBuf] = true;
		_curBuf = 1 - _curBuf;
		_bufPtr = (uint8_t *)_buf[_curBuf];
		_bufEnd = _bufPtr + _bufLen[_curBuf];
		
		// if second buffer is also empty we've got a problem
		// just set error state and return last sample
		if(_bufEmpty[_curBuf])
		{
			DEBUG_WARNING("Both buffers empty\n");
			_bothEmpty = true;
		}
		
		// all ok, we shall fill the empty buffer
		// we can't do it directly here, as it would take too much time
		// and we'll loose audio samples
		// so we run a timed interrupt routine to do the job
		// it's important that routine's priority is LOWER than the
		// one used by I2S handler, which is _I2S_IPL
//		DEBUG_INFO("Timer start triggered\n");
		AUDIOREADER_TIMER.start();
	}
	return res;
}

// check if file has a valid format
bool AudioReader::isValid(void)
{
	return _valid;
}

bool AudioReader::isEof(void)
{
	return _eof;
}

// check MP3 header
// AAAAAAAA AAABBCCD EEEEFFGH IIJJKLMM
// A	sync bytes		all 1
// B	version ID		00=V2.5 01=reserved 10=v2 11=v1
// C	layer desc		00=reserved 01=Layer3 10=layer2 11=layer1
// D	protection bit	0=CRC 1=none
// E	bit rate		0x0=free and 0xf=bad
// F	sampling rate	0b11=reserved
// G	padding bit		0=not 1=padded
// H	private bit
// I	channel mode
// J	mode extension	(for joint stereo)
//
// FF F3 90 44
// AAAAAAAA AAABBCCD EEEEEFGH IIJJKLMN
// 11111111 11110011 10010000 01000100
static bool _checkMp3Hdr(uint8_t const *buf)
{
	uint8_t b;
	
	// sync data must have all bits set
	b = *buf++;
	if(b != 0xff)
		return false;

	b = *buf++;
	if(!(b & 0b11100000))
		return false;

	// discard reserved version
	b &= 0b00011110;
	uint8_t ver = b >> 3;
	if(!ver)
		return false;

	// check for layer 3 and accept layer 2 too
	b &= 0b111;
	b >>= 1;
	if(b != 2 && b != 1)
		return false;
	
	// just in case check for bad bitrate
	// accept free bitrate too (0x0)
	b = *buf++;
	b &= 0x0f;
	if(b == 0x0f)
		return false;
	
	// we stop here... it should be enough.
	// if not, the decoder will take care of it
	return true;
}

// ID3 header
typedef struct __attribute((packed))
{
	uint8_t id[3];
	uint16_t version;
	uint8_t flags;
	
	// size uses only 28 bits
	// (skips bit 0x80 of each byte)
	// so any bute MUST have MSB cleared
	uint8_t size[4];
} ID3_HDR;

// ogg page header
typedef struct __attribute((packed))
{
	uint32_t capturePattern;
	uint8_t version;
	uint8_t type;
	uint64_t granule;
	uint32_t serial;
	uint32_t pageSequence;
	uint32_t chksum;
	uint8_t numSegments;
	uint8_t const segmentTable[0];
} OGG_HDR;

// try to identify media by some initial data
// you must supply a buffer with enough bytes to identify the file
AudioMedia AudioReader::identify(uint8_t const *buf, uint32_t len)
{
	DEBUG_INFO("IDENTIFY\n");
	if(len < 50)
	{
		DEBUG_ERROR("Sample too short to identify\n");
		return MEDIA_UNKNOWN;
	}

	// check for OGG container
	if(!strncmp((const char *)buf, "OggS", 4))
	{
		DEBUG_INFO("Ogg container\n");
		if(len < sizeof(OGG_HDR) + 10)
		{
			DEBUG_ERROR("Sample too short for an Ogg container\n");
			return MEDIA_UNKNOWN;
		}
		
		// we must check if it's an ogg/vorbis or ogg/opus
		// (or something else!)
		
		// we take the header
		OGG_HDR const &hdr = *(OGG_HDR const *)buf;
		
		// we shall reach first segment data, whic contains ID
		uint8_t nSegs = hdr.numSegments;
		DEBUG_INFO("OGG page segments : %" PRIu8 "\n", nSegs);
		uint8_t const *idPos = &hdr.segmentTable[0] + nSegs;
		DEBUG_INFO("OGG : buf = %p, idPos = %p, offset = %ld\n", buf, idPos, idPos - buf);
		
		if(idPos - buf + 8 > (int)len)
		{
			DEBUG_ERROR("Something wrong in OGG header\n");
			return MEDIA_UNKNOWN;
		}
		
		// opus start with 'OpusHead' string
		if(!strncmp((const char *)idPos, "OpusHead", 8))
			return MEDIA_OPUS;
		// vorbis start with 'vorbis' string (preceded by an unknown byte...)
		else if(!strncmp((const char *)idPos + 1, "vorbis", 6))
			return MEDIA_VORBIS;
		else
		{
			DEBUG_INFO("Unknown format string '%4s' in ogg file\n", (const char *)idPos);
			return MEDIA_UNKNOWN;
		}
	}
	// otherwise can be an WAV file
	else if(!strncmp((const char *)buf, "RIFF", 4))
	{
		DEBUG_INFO("RIFF\n");
		// we shall just that we've got 'WAVE' from byte 8
		buf += 8;
		if(!strncmp((const char *)buf, "WAVE", 4))
			return MEDIA_WAV;
		return MEDIA_UNKNOWN;
	}
	// otherwise look for MP3 file, which is more difficult to identify...
	else
	{
		// MP3 can start with an ID3 tag, but it's not always
		// I D 3 VV VV FF SS SS SS SS
		// SS is size, all bytes < 80
		if(!strncmp((const char *)buf, "ID3", 3))
		{
			DEBUG_INFO("MP3 ID3\n");
			ID3_HDR const &hdr = *(ID3_HDR const *)buf;
			
			// we check size bytes, they should be < 0x80
			for(int i = 0; i < 4; i++)
				if(hdr.size[i] > 0x80)
				{
					DEBUG_INFO("Bad size byte in ID3 tag\n");
					DEBUG_HEXDUMP(buf, 32);
					return MEDIA_UNKNOWN;
				}

			DEBUG_INFO("MP3 ID3 size is OK\n");
				
			// now get the real header size
			uint32_t hdrSize = 0;
			for(int i = 0; i < 4; i++)
				hdrSize |= ((uint32_t)hdr.size[i]) << (7 * (3 - i));

			DEBUG_INFO("ID3 HDR size is %" PRIu32 " bytes\n", hdrSize);
			
			// if we gave too few data, accept it as an mp3 anyways...
			if(hdrSize + 14 > len)
			{
				DEBUG_INFO("ID3 header too big (%" PRIx32 " bytes), acceppting anyways\n", hdrSize);
				return MEDIA_MP3;
			}
			
			// skip the header and go on with MP3 frame header
			buf += hdrSize + 10;
			
			DEBUG_INFO("ID3 Skipped, data follows:\n");
			DEBUG_HEXDUMP(buf, 64);
		}
		DEBUG_INFO("Check MP3 header\n");
		// check for an MP3 frame header (32 bit)
		// (if we read in little endian mode the sequence is reversed)
		// 12 bits of frame sync (all 1s)
		// 4 bits of version + layer + error protection
		// 4 bits of bitrate
		// 4 mode bits (mono/stereo/joint stereo, etc
		// 4 bits of copyright, emphasis, etc
		return _checkMp3Hdr(buf) ? MEDIA_MP3 : MEDIA_UNKNOWN;
	}
	
	return MEDIA_MP3;
}
