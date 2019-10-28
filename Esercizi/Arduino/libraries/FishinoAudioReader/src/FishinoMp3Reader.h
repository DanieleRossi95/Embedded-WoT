//////////////////////////////////////////////////////////////////////////////////////
//																					//
//							FishinoMp3Reader.h										//
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
#ifndef _FISHINOMP3READER_H
#define _FISHINOMP3READER_H

#include "helix/pub/mp3dec.h"

#define MP3_BUFFER_SIZE 8192
class Mp3Reader : public AudioReader
{
	private:
	
		MP3FrameInfo _mp3FrameInfo;
		HMP3Decoder _mp3Decoder;
		
		uint8_t _mp3Buffer[MP3_BUFFER_SIZE];
		volatile int _mp3BytesLeft;
		uint8_t *_mp3BufPtr;
		
	protected:
	
		// initialize internal reader data
		// to be redefined in derived classes
		virtual bool initialize(void);
		
		// finalize reader data
		// to be redefined in derived classes
		virtual void finalize(void);

		// fill buffer -- called by timer interrupt handler
		virtual uint32_t getSamples(uint32_t *buffer, uint32_t len);
	
	public:

		// constructor -- takes a stream
		Mp3Reader(FishinoStream &s);
		
		// destructor
		~Mp3Reader();
};

#endif
