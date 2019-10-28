//////////////////////////////////////////////////////////////////////////////////////
//																					//
//							FishinoPlayer.h											//
//						An audio player class										//
//		Can play audio streams from SD files or HTTP/memory streams					//
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
//  Version 6.0.4	June 2017	Skip folders and system files						//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FISHINOPLAYER_H
#define _FISHINOPLAYER_H

// for sending audio to codec
#include <I2S.h>

// for codec interface
#include <FishinoAudioCodec.h>

// for audio reader classes
#include <FishinoAudioReader.h>

// file stream class
#include <FishinoStream.h>

class FishinoPlayerClass
{
	friend void __fishinoPlayer__i2sHandler__(FishinoPlayerClass &p);
	friend FishinoPlayerClass &__GetFishinoPlayer(void);

	private:
	
		// the input stream
		FishinoStream *_stream;
		
		// flag signaling that we created the stream
		bool _ownStream;
	
		// the audio reader for this file
		AudioReader *_reader;
		
		// the I2S interface
		I2SClass *_i2s;
		
		// the used codec
		AudioCodec *_codec;
		
		// handle i2s requests
		void handleI2SRequest(void);
		
		// play in progress flag
		volatile bool _playing;
		
		// playing paused flag
		volatile bool _paused;
		
		// free player resources when done
		void freeResources(void);
		
		// volume and balance
		double _vol, _bal;

	protected:
	
	public:
	
		// constructor
		FishinoPlayerClass();
	
		// destructor
		~FishinoPlayerClass();
		
		// change default devices
		void setDevices(I2SClass &i2s = I2S, AudioCodec &codec = ALC5631);
	
		// play a file from SD card
		bool play(const char *fileName);
		
		// play from a FishinoStream object
		bool play(FishinoStream &stream);
		
		// pause play
		bool pause(void);
		
		// resume play
		bool resume(void);
		
		// stop playing current file and free resources
		bool stop(void);
		
		// check if it's still playing
		bool isPlaying(void) { return _playing; }
		
		// change the volume
		bool volume(double vol, double bal = 0);
};

FishinoPlayerClass &__GetFishinoPlayer(void);

#define FishinoPlayer __GetFishinoPlayer()



#endif
