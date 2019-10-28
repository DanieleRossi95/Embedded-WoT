//////////////////////////////////////////////////////////////////////////////////////
//																					//
//							FishinoAudioCodec.h										//
//						Generic Audio Codec Interface								//
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
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _AUDIOCODEC_H
#define _AUDIOCODEC_H

#include "Arduino.h"

// class to manage audio codecs
class AudioCodec
{
	private:
	
	protected:
	
		// current active path
		uint32_t _path;
		
		// current active power mode
		uint32_t _power;
		
		// clear all mixer paths
		// used internally before setting new one(s)
		virtual void clearPaths(void) = 0;
	
		// debugger channel
		Stream *_debugStream;

	public:
	
		typedef enum
		{
			POWEROFF				= 0x0000,
			LEFTSPEAKER				= 0x0001,
			RIGHTSPEAKER			= 0x0002,
			STEREOSPEAKERS			= 0x0003,
			MONOSPEAKER				= 0x0004,
			LEFTHEADPHONE			= 0x0008,
			RIGHTHEADPHONE			= 0x0010,
			STEREOHEADPHONES		= 0x0018,
			LEFTAUXOUT				= 0x0020,
			RIGHTAUXOUT				= 0x0040,
			STEREOAUXOUT			= 0x0060,
			MONOAUXOUT				= 0x0080,
			LEFTMIC					= 0x0100,
			RIGHTMIC				= 0x0200,
			STEREOMICS				= 0x0300,
			LEFTAUXIN				= 0x0400,
			RIGHTAUXIN				= 0x0800,
			STEREOAUXIN				= 0x0c00,
			MONOAUXIN				= 0x1000
		} CodecPower;
		
		typedef enum
		{
			// audio play paths
			DAC2STEREOSPEAKERS		= 0x0001,
			DAC2MONOSPEAKER			= 0x0002,
			DAC2STEREOHEADPHONES	= 0x0004,
			DAC2STEREOAUXOUT		= 0x0008,
			DAC2MONOAUXOUT			= 0x0010,
			
			// audio recording paths
			LEFTMIC2LEFTADC			= 0x0020,
			RIGHTMIC2RIGHTADC		= 0x0040,
			STEREOMICS2ADC			= LEFTMIC2LEFTADC | RIGHTMIC2RIGHTADC,

			STEREOAUXIN2ADC			= 0x0080,
			MONOAUXIN2ADC			= 0x0100
		} CodecPaths;
		
		typedef enum
		{
			SPEAKERSVOL				= 0x0001,
			HEADPHONESVOL			= 0x0002,
			OUTPUTVOL				= 0x0004,
			OTPUTTOAXO1VOL			= 0x0008,
			OUTPUTTOAXO2VOL			= 0x0010,

			MICSINPUTGAIN			= 0x0020,
			AUXINPUTVOL				= 0x0040,

			DACDIGITALPREBOOST		= 0x0080,
			DACDIGITALVOL			= 0x0100,

			ADCDIGITALPREBOOST		= 0x0200,
			ADCDIGITALVOL			= 0x0400,
		} Volumes;
		
		// constructor
		AudioCodec();
		
		// destructor
		virtual ~AudioCodec();
		
		// reset the controller
		// can be redefined if needed
		virtual bool reset(void) { return true; }
		
		// codec power management
		virtual bool power(uint32_t control) = 0;
		
		// codec path management
		virtual bool path(uint32_t pth) = 0;
		
		// volume management
		virtual bool volume(uint32_t what, double vol, double bal = 0) = 0;
		
		// enable/disable debug logs
		void debug(Stream &s = Serial);
		void noDebug(void);
		
		// debug helper -- prints codec registers
		virtual void printRegs(Stream &s) = 0;
};

#include "ALC5631.h"

#endif