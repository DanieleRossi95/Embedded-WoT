//////////////////////////////////////////////////////////////////////////////////////
//																					//
//										WavWriter.h									//
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

#ifndef __WAVWRITER_H
#define __WAVWRITER_H

class WavWriter : public FileAudioWriter
{
	private:
	
		// writes wav header and fill audio info
		virtual bool writeHeader(void);
		
		// fill buffer -- called by timer interrupt handler
		virtual bool flushBuffer(uint8_t bufToFlush);
	
	protected:
	
	public:
	
		// constructor -- takes a file path
		WavWriter(const char *path);
		
		// empty constructor, must be followed by open()
		WavWriter();
		
		// destructor
		virtual ~WavWriter();
		
		// opens a file
		virtual bool open(const char *path);
};

#endif
