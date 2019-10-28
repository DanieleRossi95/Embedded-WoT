//////////////////////////////////////////////////////////////////////////////////////
//																					//
//								ALC5631.h											//
//						ALC5631 Audio Codec driver									//
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
#ifndef _ALC5631_H
#define _ALC5631_H

#include "Arduino.h"

// ALC I2C interface
#define ALC_I2C Wire

#define ALC5631 __ALC5631()

class ALC5631Class : public AudioCodec
{
	private:

		// write an ALC register
		bool writeReg(uint8_t reg, uint16_t mask, uint16_t val);
		
		// read an ALC register
		uint16_t readReg(uint8_t reg);
		
		// write 2nd level register
		bool write2Reg(uint8_t reg, uint16_t mask, uint16_t val);
		
		// read 2nd level regiser
		uint16_t read2Reg(uint8_t reg);
	
		// helper -- prints register content
		void printReg(Stream &s, uint8_t reg);

	protected:
	
		// clear all mixer paths
		// used internally before setting new one(s)
		void clearPaths(void);
	
	public:
	
		// constructor
		ALC5631Class();
		
		// destructor
		~ALC5631Class();
		
		// reset the controller
		// can be redefined if needed
		virtual bool reset(void);
		
		// codec power management
		virtual bool power(uint32_t control);
		
		// codec path management
		virtual bool path(uint32_t pth);
		
		// volume management
		virtual bool volume(uint32_t what, double vol, double bal = 0);

		// debug helper -- prints codec registers
		virtual void printRegs(Stream &s);
};

ALC5631Class &__ALC5631();



#endif