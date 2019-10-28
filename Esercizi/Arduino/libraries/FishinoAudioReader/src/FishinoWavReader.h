//////////////////////////////////////////////////////////////////////////////////////
//																					//
//							FishinoWavReader.h										//
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
#ifndef _FISHINOWAVREADER_H
#define _FISHINOWAVREADER_H

// a .Wav file reader -- supports ONLY uncompressed PCM formats
// the bit width can be any of supported ones
class WavReader : public AudioReader
{
	private:
	
	protected:
	
		// starts processing data and prefill buffers
		virtual bool initialize(void);

		// terminate processing and free resources
		// (should be called in final class destructor)
		virtual void finalize(void);

		// fill buffer -- called by timer interrupt handler
		virtual uint32_t getSamples(uint32_t *buf, uint32_t len);
	
	public:
	
		// constructor -- takes a stream
		WavReader(FishinoStream &s);
		
		// destructor
		virtual ~WavReader();
};

#endif
