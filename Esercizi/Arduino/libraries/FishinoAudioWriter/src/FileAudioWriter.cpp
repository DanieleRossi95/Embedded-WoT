//////////////////////////////////////////////////////////////////////////////////////
//																					//
//									FileAudioWriter.cpp								//
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

#ifndef ARDUINO_STREAMING
#define ARDUINO_STREAMING
template<class T> inline Print &operator <<(Print &stream, T arg)
{
	stream.print(arg);
	return stream;
}
#endif

// seeks output file at a given position
bool FileAudioWriter::outStreamSeek(uint32_t pos)
{
	return _file.seek(pos);
}

// gets current output stream position
uint32_t FileAudioWriter::outStreamPosition(void)
{
	return _file.position();
}

// get size of output stream
uint32_t FileAudioWriter::outStreamSize(void)
{
	return _file.size();
}

// write a number of bytes from a buffer
// return number of bytes actually written
// to be redefined on derived classes
uint32_t FileAudioWriter::outStreamWrite(uint8_t *buf, uint32_t count)
{
	return _file.write(buf, count);
}

// empty constructor, must be followed by open()
FileAudioWriter::FileAudioWriter()
{
	_path = NULL;
}

// destructor
FileAudioWriter::~FileAudioWriter()
{
	close();
}

// close attached file
bool FileAudioWriter::close(void)
{
	if(_file)
	{
		finalizeBuffers();
		_file.close();
	}

	// reset internal state
	reset();
	
	return true;
}
