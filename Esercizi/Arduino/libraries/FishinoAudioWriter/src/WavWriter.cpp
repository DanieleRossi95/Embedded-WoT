//////////////////////////////////////////////////////////////////////////////////////
//																					//
//										WavWriter.cpp								//
//							Wav File Audio Writer Interface							//
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

enum WavChunks
{
	ChunkRiffHeader			= 0x46464952,
	ChunkWavRiff			= 0x45564157,
	ChunkFormat				= 0x20746d66,
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

// the WAV header
typedef struct
{
	// riff master record
	uint32_t riffId;
	uint32_t riffSize;
	
	// wave subrecord
	uint32_t waveId;
	
	// format subrecord
	uint32_t formatId;
	uint32_t formatSize;
	uint16_t formatTag;
	uint16_t nChannels;
	uint32_t sampleRate;
	uint32_t byteRate;
	uint16_t blockAlign;
	uint16_t bitsPerSample;
	
	// data subrecord
	uint32_t dataId;
	uint32_t dataSize;
	
	// sample data follow
	
} WavHeader;

WavHeader wavWriterHeader =
{
	ChunkRiffHeader,
	0x00000000,					// 36 + sample data size, to be filled!
	ChunkWavRiff,
	ChunkFormat,
	16,							// should be fixed...
	FormatPulseCodeModulation,
	2,							// channels, to be filled!!!
	44100,						// sample rate, to be filled!!!
	88200,						// byte rate, sampleRate x channels x bits / 8, to be filled!!!
	2,							// block align, channels x bits / 8, to be filled!!!
	8,							// bits per sample, to be filled!
	ChunkData,
	0x00000000					// data size, to be filled!!!
};

const size_t WavHeaderSize = sizeof(WavHeader);

// parse wav header and fill audio info
bool WavWriter::writeHeader(void)
{
	// update some header fields
	wavWriterHeader.nChannels		= (_channelMode == CHANMODE_STEREO ? 2 : 1);
	wavWriterHeader.sampleRate		= _sampleRate;
	wavWriterHeader.byteRate		= _sampleRate * (_channelMode == CHANMODE_STEREO ? 2 : 1) * _bits / 8;
	wavWriterHeader.blockAlign		= (_channelMode == CHANMODE_STEREO ? 2 : 1) * _bits / 8;
	wavWriterHeader.bitsPerSample	= _bits;
	wavWriterHeader.dataSize		= _numSamples /* * (_stereo ? 2 : 1) */ * _bits / 8;
	wavWriterHeader.riffSize		= wavWriterHeader.dataSize + 36;

	if(_debugStream)
		*_debugStream << "Writing header : samplerate=" << _sampleRate << " bits=" << _bits << " channel mode:" << _channelMode << "\n";

	// get actual writing position
	uint32_t pos = _file.position();
	
	// go to file beginning
	_file.seek(0);
	
	// write the header
	_file.write((uint8_t *)&wavWriterHeader, WavHeaderSize);
	
	// if first write, seek at end of header
	// otherwise seek at previous position
	if(!_numSamples)
		_file.seek(WavHeaderSize);
	else
		_file.seek(pos);
	
	return true;
}

// fill buffer -- called by timer interrupt handler
bool WavWriter::flushBuffer(uint8_t bufToFlush)
{
	uint8_t *buf = _buf[bufToFlush];
	uint32_t siz = _bufLen[bufToFlush];

	uint32_t tim = 0;
	if(_debugStream)
	{
		*_debugStream << "Flushing buffer " << bufToFlush << " size is " << siz << "\n";
		tim = millis();
	}

	bool res = (_file.write(buf, siz) == siz);
	
	if(_debugStream)
	{
		tim = millis() - tim;
		*_debugStream << "Flushed in " << tim << "mSec\n";
	}
	_bufLen[bufToFlush] = 0;
	_bufFull[bufToFlush] = false;
	return res;
}

// constructor -- takes a file path
WavWriter::WavWriter(const char *path) : FileAudioWriter()
{
	open(path);
}

// empty constructor, must be followed by open()
WavWriter::WavWriter() : FileAudioWriter()
{
}

// destructor
WavWriter::~WavWriter()
{
	close();
}

// opens a file
bool WavWriter::open(const char *path)
{
	close();
	
	if(path)
		_path = path;
	if(!_path)
		return false;
	
	_file = SD.open(_path, O_WRITE | O_CREAT | O_TRUNC);
	if(_file)
	{
		// write initial header
		if(!writeHeader())
		{
			close();
			return false;
		}
		
		// mark stream as valid
		_valid = true;
		
		return true;
	}
	reset();
	return false;
}
