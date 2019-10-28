//////////////////////////////////////////////////////////////////////////////////////
//																					//
//										Mp3Writer.h									//
//							MP3 File Audio Writer Interface							//
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

#ifndef __MP3WRITER_H
#define __MP3WRITER_H

#include "shine/layer3.h"

class Mp3Writer : public FileAudioWriter
{
	private:
	
		// writes wav header and fill audio info
		// no use here
		virtual bool writeHeader(void) { return true; }
		
		// write trailer data
		// needed by some encoders, for example Mp3
		// redefine only if needed
		virtual bool writeTrailer(void);

		// fill buffer -- called by timer interrupt handler
		virtual bool flushBuffer(uint8_t bufToFlush);
	
		// for some encoders, we need a particular number of samples
		// stored inside the buffer(s), so we shall ensure that
		// this integral number fits inside. We provide a function
		// that rounds, if needed, the AUDIOWRITER_BUFSIZE
		// this function can be redefined on derived classes
		virtual uint32_t audioWriterRoundSize(void);

		// signal update on audio parametera
		// needed when changing them, on some codecs
		// redefine if needed
		virtual void audioParamsUpdated(void);
		
		// mp3 bit rate
		uint32_t _mp3BitRate;
		
		// samples per pass of Mp3 encoder
		uint32_t _samplesPerPass;
		
		// number of integral passes inside buffer
		uint32_t _passesPerBuffer;
	
		// shine encoder
		shine_config_t _shineConfig;
		shine_t _shine;
		
		// shine engine initialized flag
		bool _shineInitialized;
		
		// shine engine initialization and termination
		// (encloses it inside _shineInitialized flag)
		bool openShine(void);
		bool closeShine(void);
		
	protected:
	
	public:
	
		// constructor -- takes a file path
		Mp3Writer(const char *path);
		
		// empty constructor, must be followed by open()
		Mp3Writer();
		
		// destructor
		virtual ~Mp3Writer();
		
		// opens a file
		virtual bool open(const char *path);
};

#endif
