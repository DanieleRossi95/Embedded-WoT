//////////////////////////////////////////////////////////////////////////////////////
//																					//
//							FishinoMp3Reader.cpp									//
//						Mp3 Audio Reader Interface									//
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
//  Version 6.0.3	June 2017	Fix a crash when trying to read on a closed stream	//
//  Version 6.0.4	June 2017	Some small fixes									//
//  Version 6.0.5	June 2017	Fixed MP3 for weird files							//
//  Version 7.3.1	24/12/2017	Fixed mips16 compilation issues						//
//																					//
//////////////////////////////////////////////////////////////////////////////////////

//#define DEBUG_LEVEL_INFO
//#define DEBUG_LEVEL_WARNING
#include "FishinoDebug.h"

#include "FishinoAudioReader.h"

#ifndef ARDUINO_STREAMING
#define ARDUINO_STREAMING
template<class T> inline Print &operator <<(Print &stream, T arg)
{
	stream.print(arg);
	return stream;
}
#endif

// fill buffer -- called by timer interrupt handler
uint32_t Mp3Reader::getSamples(uint32_t *buffer, uint32_t len)
{
	DEBUG_INFO("getSamples()\n");
	
	// this outer loop is to recover from frame errors
	while(1)
	{
		// find next sync word
		int offset = MP3FindSyncWord(_mp3BufPtr, _mp3BytesLeft);
		// offset == -1 means end of stream
		if(offset < 0)
		{
			DEBUG_INFO("Offset < 0, EOF\n");
			setEof();
			return 0;
		}
		DEBUG_INFO("MP3 Offset: %d\n", offset);
		DEBUG_INFO("MP3 bytes left : %d\n", _mp3BytesLeft);
	
		_mp3BytesLeft -= offset;
		_mp3BufPtr += offset;
	 	uint32_t res = 0;
	
		// decode current frame
#ifdef DEBUG_LEVEL_INFO
		uint32_t tim = millis();
#endif
	
		int err = MP3Decode(_mp3Decoder, &_mp3BufPtr, (int *)&_mp3BytesLeft, (int16_t *)buffer, 0);
	
		if (err)
		{
			// error occurred */
			switch (err)
			{
				case ERR_MP3_INDATA_UNDERFLOW:
					// it should not happen, but it does
					// so we disregard this one, and just return 0 buffer size
					// (it happens at end of file)
					// we don't reset remaining samples, as the other buffer may still have some
					DEBUG_WARNING("In Data underflow\n");
					return 0;
					break;
				case ERR_MP3_INVALID_FRAMEHEADER:
					DEBUG_WARNING("Invalid frame header, try more\n");
					_mp3BufPtr++;
					_mp3BytesLeft--;
					continue;
				case ERR_MP3_MAINDATA_UNDERFLOW:
					DEBUG_WARNING("Main Data underflow\n");
					/* do nothing - next call to decode will provide more mainData */
					break;
				case ERR_MP3_FREE_BITRATE_SYNC:
				default:
					// terminate processing
					DEBUG_INFO("Some MP3 error (%d), closing\n", err);
					setEof();
					return 0;
			}
		}
		else
		{
			// no error
			MP3GetLastFrameInfo(_mp3Decoder, &_mp3FrameInfo);
			
			// store (again) audio data
			// we could do just once, with a flag
			_bits = _mp3FrameInfo.bitsPerSample;
			_sampleRate = _mp3FrameInfo.samprate;
			_stereo = (_mp3FrameInfo.nChans > 1);
		
			// I suppose that 'outputsamps' is the total numbero of samples
			// inside the frame; we use it here to set the _numSamples
			// even if it's not correct; we re-set it on each buffer translation
			uint32_t _numSamples = _mp3FrameInfo.outputSamps;
			
			// calculate number of bytes read
			res = _numSamples * (_bits / 8);
			
#ifdef DEBUG_LEVEL_INFO
		tim = millis() - tim;
		tim = res / tim;
//	DEBUG_INFO("BYTES: %d, SPEED:%d kbps\n", (int)res, (int)tim);
#endif
	
		}
		
		// if not terminated, just check if we need to refill main mp3 buffer
		if((_mp3BytesLeft < MP3_BUFFER_SIZE / 2) /*&& inStreamAvailable()*/)
		{
			DEBUG_INFO("Getting more data from stream\n");
			// ok, we've processed at least half of the mp3 buffer
			// just move remaining data on head and read more of it
			memcpy(_mp3Buffer, _mp3BufPtr, _mp3BytesLeft);
			
			// read new mp3 data
			uint32_t siz = inStreamRead(_mp3Buffer + _mp3BytesLeft, MP3_BUFFER_SIZE - _mp3BytesLeft);
			
			// move ptr to beginning and setup new bytes left
			_mp3BufPtr = _mp3Buffer;
			_mp3BytesLeft += siz;
		}
	
		DEBUG_INFO("getSamples(%" PRIu32 ") returned %" PRIu32 " bytes\n", len, res);
		return res;
	}
}

// empty constructor, must be followed by open()
Mp3Reader::Mp3Reader(FishinoStream &s) : AudioReader(s)
{
	_mp3Decoder = NULL;
}

// destructor
Mp3Reader::~Mp3Reader()
{
	// free resources
	end();
}

// starts processing data and prefill buffers
bool Mp3Reader::initialize(void)
{
	// initialize MP3 stuffs
	_mp3Decoder = MP3InitDecoder();
	if(!_mp3Decoder)
		return false;

	// read starting mp3 buffer
	uint32_t siz = inStreamRead(_mp3Buffer, MP3_BUFFER_SIZE);
	_mp3BufPtr = _mp3Buffer;
	_mp3BytesLeft = siz;
	
	return true;
}

// terminate processing and free resources
// (should be called in final class destructor)
void Mp3Reader::finalize(void)
{
	DEBUG_INFO("Finalize\n");
	// frees decoder resources
	if(_mp3Decoder)
		MP3FreeDecoder(_mp3Decoder);
	_mp3Decoder = NULL;
}


