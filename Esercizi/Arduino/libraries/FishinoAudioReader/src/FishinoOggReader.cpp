//////////////////////////////////////////////////////////////////////////////////////
//																					//
//							FishinoOggReader.cpp									//
//					Ogg Vorbis Audio Reader Interface								//
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
//	VERSION 1.0.0	2017		INITIAL VERSION										//
//  Version 6.0.2	June 2017	Some small fixes									//
//  Version 6.0.3	June 2017	Fix a crash when trying to read on a closed stream	//
//  Version 6.0.4	June 2017	Some small fixes									//
//  Version 6.0.5	June 2017	Fixed MP3 for weird files							//
//  Version 7.3.1	24/12/2017	Fixed mips16 compilation issues						//
//																					//
//////////////////////////////////////////////////////////////////////////////////////

//#define DEBUG_LEVEL_INFO
#include <FishinoDebug.h>

#include "FishinoAudioReader.h"

#ifndef ARDUINO_STREAMING
#define ARDUINO_STREAMING
template<class T> inline Print &operator <<(Print &stream, T arg)
{
	stream.print(arg);
	return stream;
}
#endif

// external callbacks needed to re-enter the object
inline size_t __OggReadFunc(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	size_t res = ((OggReader *)datasource)->inStreamRead((uint8_t *)ptr, size * nmemb);
//	DEBUG_INFO("__OggReadFunc(%p, %u) RETURNED %u\n", ptr, size * nmemb, res);
/*
	if(res != (uint32_t)-1)
		DEBUG_HEXDUMP(ptr, size * nmemb);
	DEBUG_INFO("File position : %u\n", ((OggReader *)datasource)->_stream.tell());
*/
	return res;
}

inline int __OggSeekFunc(void *datasource, ogg_int64_t offset, int whence)
{
//	return -1;
	if(!((OggReader *)datasource)->inStreamSeek(offset, whence))
		return -1;
	return 0;
}

inline int __OggCloseFunc(void *datasource)
{
//	((OggReader *)datasource)->end();
//	DEBUG_INFO("__OggCloseFunc()\n");
	return 1;
}

inline long __OggTellFunc(void *datasource)
{
	long res = ((OggReader *)datasource)->inStreamTell();
//	DEBUG_INFO("__OggTellFunc() RETURNED %ld\n", res);
	return res;
}

ov_callbacks __OggCallbacks =
{
	__OggReadFunc,
	__OggSeekFunc,
	__OggCloseFunc,
	__OggTellFunc
};

// parse ogg header and fill audio info
bool OggReader::parseHeader(void)
{
	// no remaining samples until parsed buffers
//	_remainingSamples = 0;

	return true;
}

// fill buffer -- called by timer interrupt handler
uint32_t OggReader::getSamples(uint32_t *buf, uint32_t len)
{
	// just to avoid a crash...
	if(isEof())
		return 0;
	
	uint8_t *bufP = (uint8_t *)buf;
	uint32_t totalRead = 0;
	
/*
	int32_t bytesRead = ov_read(&_vorbisFile, bufP, len, &_oggCurrentSection);
	if(bytesRead < 0)
	{
		// should not happen
		DEBUG_INFO("Stream error\n");
	}
	return bytesRead;
*/
	
	
	// fill next buffer
	long bytesRead = -1;
	uint32_t missing = len;
	do
	{
		bytesRead = ov_read(&_vorbisFile, bufP, missing, &_oggCurrentSection);
//		DEBUG_INFO("ov_read returned %ld\n", bytesRead);	

		if(bytesRead > 0)
		{
			bufP += bytesRead;
			totalRead += bytesRead;
			missing -= bytesRead;
		}
	}
	while(bytesRead > 0 && missing > 1000 /*&& totalRead < _bufLen[1 - bufToFill]*/);

	if(bytesRead == 0)
	{
		DEBUG_INFO("No bytes read on loop\n");
		if(!totalRead)
		{
			DEBUG_INFO("No total bytes read\n");
			setEof();
		}
		return totalRead;
	}
	else if(bytesRead < 0)
	{
		// should not happen
		DEBUG_INFO("Stream error\n");
		setEof();
		return totalRead;
	}
	// return current buffer size
	return totalRead;
}

// constructor -- takes a stream
OggReader::OggReader(FishinoStream &s) : AudioReader(s)
{
	_vorbisOpened = false;
	memset(&_vorbisFile, 0, sizeof(_vorbisFile));
}

// destructor
OggReader::~OggReader()
{
	end();
	DEBUG_INFO("DESTROYING READER\n");
}

// opens a file
bool OggReader::initialize(void)
{
	// restarts the input stream
	inStreamRestart();
	
	// check if we've got data
	if(!inStreamAvailable())
		return false;
	
	// initialize ogg subsystem
	int res = ov_open_callbacks(this, &_vorbisFile, NULL, 0, __OggCallbacks);
	if(res)
	{
		DEBUG_ERROR("ov_open_callbacks FAILED : ");
		switch(res)
		{
			case OV_EREAD:
				DEBUG_ERROR_N("A read from media returned an error.\n");
				break;

			case OV_ENOTVORBIS:
				DEBUG_ERROR_N("Bitstream is not Vorbis data.\n");
				break;

			case OV_EVERSION:
				DEBUG_ERROR_N("Vorbis version mismatch.\n");
				break;

			case OV_EBADHEADER:
				DEBUG_ERROR_N("Invalid Vorbis bitstream header.\n");
				break;

			case OV_EFAULT:
				DEBUG_ERROR_N("Internal logic fault; indicates a bug or heap/stack corruption.\n");
				break;

			default :
				DEBUG_ERROR_N("UNKNOWN ERROR\n");
				break;
		}

		return false;
	}
	
	_vorbisOpened = true;
	vorbis_info *vi = ov_info(&_vorbisFile, -1);

	// show some debug output
#ifdef DEBUG
	DEBUG_INFO("SOME DEBUG OUTPUT\n");
	char **ptr = ov_comment(&_vorbisFile, -1)->user_comments;
	while(*ptr)
	{
		DEBUG_INFO("%s\n", *ptr);
		ptr++;
	}
	DEBUG_INFO("Bitstream is %d channel, %d\n", vi->channels, (int)vi->rate);
	DEBUG_INFO("Decoded length: %d samples\n", (int)ov_pcm_total(&_vorbisFile, -1));
	DEBUG_INFO("Encoded by: %s\n\n", ov_comment(&_vorbisFile, -1)->vendor);
#endif

	// store audio data
	// data is always 16 bits signed
	_bits = 16;
	_sampleRate = vi->rate;
	_stereo = (vi->channels > 1);

	return true;
}

// terminate processing and free resources
// (should be called in final class destructor)
void OggReader::finalize(void)
{
	DEBUG_INFO("OggReader::finalize()\n");
	if(_vorbisOpened)
		ov_clear(&_vorbisFile);
	_vorbisOpened = false;
}
