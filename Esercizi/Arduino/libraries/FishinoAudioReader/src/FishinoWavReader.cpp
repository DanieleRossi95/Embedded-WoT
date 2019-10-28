//////////////////////////////////////////////////////////////////////////////////////
//																					//
//							FishinoWavReader.cpp									//
//						Wav Audio Reader Interface									//
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

enum WavChunks
{
	ChunkRiffHeader			= 0x46464952,
	ChunkWavRiff			= 0x45564157,
//	ChunkWavRiff			= 0x54651475,
	ChunkFormat				= 0x020746d66,
	ChunkLabeledText		= 0x478747C6,
	ChunkInstrumentation	= 0x478747C6,
	ChunkSample				= 0x6C706D73,
	ChunkFact				= 0x47361666,
	ChunkData				= 0x61746164,
	ChunkJunk				= 0x4b4e554a,
};

enum WavFormat
{
	FormatPulseCodeModulation	= 0x01,
	FormatIEEEFloatingPoint		= 0x03,
	FormatALaw					= 0x06,
	FormatMuLaw					= 0x07,
	FormatIMAADPCM				= 0x11,
	FormatYamahaITUG723ADPCM	= 0x16,
	FormatGSM610				= 0x31,
	FormatITUG721ADPCM			= 0x40,
	FormatMPEG					= 0x50,
	FormatExtensible			= 0xFFFE
};

// fill buffer -- called by timer interrupt handler
uint32_t WavReader::getSamples(uint32_t *buf, uint32_t len)
{
	// fill it. It may get less than AUDIOREADER_BUFSIZE bytes
	// if we're close to end of stream
	uint32_t res = inStreamRead((uint8_t *)buf, len);
	if(!res)
		return 0;
	
	return res;
}

// constructor -- takes a stream
WavReader::WavReader(FishinoStream &s) : AudioReader(s)
{
}

// destructor
WavReader::~WavReader()
{
	// free resources
	end();
}

// starts processing data and prefill buffers
bool WavReader::initialize(void)
{
	// parse wav header and fill audio info
	uint32_t chunkId = 0;
	bool dataChunk = false;
	uint16_t format = 0;
	uint16_t channels = 0;
	uint32_t sampleRate = 0;
#ifdef DEBUG_LEVEL_INFO
	uint32_t dataSize;
	uint32_t bitsPerSecond;
	uint16_t formatBlockAlign;
#endif
	uint16_t bitDepth = 0;
	
	while (!dataChunk && inStreamAvailable())
	{
		chunkId = inStreamRead32();
		switch (chunkId)
		{
			case ChunkFormat:
			{
				DEBUG_INFO("Got Format record\n");
				uint32_t formatSize = inStreamRead32();
				format = (WavFormat)inStreamRead16();
				channels = inStreamRead16();
				sampleRate = inStreamRead32();
#ifdef DEBUG_LEVEL_INFO
				bitsPerSecond = inStreamRead32();
				formatBlockAlign = inStreamRead16();
#else
				/* bitsPerSecond = */ inStreamRead32();
				/* formatBlockAlign = */ inStreamRead16();
#endif
				bitDepth = inStreamRead16();
				if (formatSize == 18)
				{
					uint32_t extraData = inStreamRead16();
					inStreamSkip(extraData);
				}
				DEBUG_INFO("format           : %d\n", format);
				DEBUG_INFO("channels         : %d\n", channels);
				DEBUG_INFO("sampleRate       : %d\n", sampleRate);
				DEBUG_INFO("bitsPerSecond    : %d\n", bitsPerSecond);
				DEBUG_INFO("formatBlockAlign : %d\n", formatBlockAlign);
				DEBUG_INFO("bitDepth         : %d\n", bitDepth);
			}
				break;
			case ChunkRiffHeader:
				DEBUG_INFO("Got RIFF header record\n");
//				headerid = chunkid;
				/* uint32_t memSize = */ inStreamRead32();
				/* uint32_t riffStyle = */ inStreamRead32();
				break;
			case ChunkData:
				dataChunk = true;
#ifdef DEBUG_LEVEL_INFO
				dataSize = inStreamRead32();
#else
				inStreamRead32();
#endif
				DEBUG_INFO("Got Data record\n");
				DEBUG_INFO("DataSize         : %d\n", dataSize);
				break;
			default:
			{
				DEBUG_INFO("Got Unknown record\n");
				uint32_t skipSize = inStreamRead32();
				inStreamSkip(skipSize);
			}
			break;
		}
	}
	
	// we accept only mono or stereo PCM files
	if(!dataChunk || format != FormatPulseCodeModulation || channels > 2)
		return false;
	
	// store format data
	_sampleRate = sampleRate;
	_bits = bitDepth;
	_stereo = (channels == 2);
	
	return true;
}

// terminate processing and free resources
// (should be called in final class destructor)
void WavReader::finalize(void)
{
}
