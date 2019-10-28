//////////////////////////////////////////////////////////////////////////////////////
//																					//
//									FileAudioWriter.h								//
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

#ifndef __FILEAUDIOWRITER_H
#define __FILEAUDIOWRITER_H

#include <FishinoSdFat.h>
extern SdFat SD;

class FileAudioWriter : public AudioWriter
{
	private:
	
	protected:
	
		// attached file path
		const char *_path;
	
		// the attached file
		File _file;
		
		// seeks output file at a given position
		virtual bool outStreamSeek(uint32_t pos);
		
		// gets current output stream position
		virtual uint32_t outStreamPosition(void);
		
		// get size of output stream
		virtual uint32_t outStreamSize(void);
		
		// write a number of bytes from a buffer
		// return number of bytes actually written
		// to be redefined on derived classes
		virtual uint32_t outStreamWrite(uint8_t *buf, uint32_t count);
		
	public:
	
		// empty constructor, must be followed by open()
		FileAudioWriter();
		
		// destructor
		virtual ~FileAudioWriter();

		// open attached file
		virtual bool open(const char *path = NULL) = 0;
		
		// close attached file
		virtual bool close(void);
};

#endif
