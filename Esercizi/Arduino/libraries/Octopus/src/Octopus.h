//////////////////////////////////////////////////////////////////////////////////////
//																					//
//								Octopus.h											//
//			Library to handle Fishino Octopus I/O expander boards					//
//					Created by Massimo Del Fedele, 2016								//
//																					//
//  Copyright (c) 2015, 2016 and 2017 Massimo Del Fedele.  All rights reserved.		//
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
//	VERSION 1.0.0 - INITIAL VERSION													//
//	VERSION 5.0.0 -            - Converted to new library format					//
//	VERSION 7.5.0 - 30/04/2018 - FIXED PULLUPS HANDLING								//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#ifndef __OCTOPUS_H
#define __OCTOPUS_H

#include <Arduino.h>

#define Octopus __octopus()

// struct to cache some MCP registers
typedef struct
{
	uint16_t GPIO;
	uint16_t IODIR;
	uint16_t GPPU;
	uint16_t IOINT;
} MCPCache;

class OctopusClass
{
	private:
	
		// connected boards and their addresses
		uint8_t nBoards;
		uint8_t *addresses;
		
		// cached MCP registers
		MCPCache *mcpCaches;
		
		// low-level MCP register access
		void writeMCPRegister8(uint8_t mcpAddr, uint8_t regAddr, uint8_t val);
		void writeMCPRegister16(uint8_t mcpAddr, uint8_t regAddr, uint16_t val);
		uint16_t readMCPRegister16(uint8_t mcpAddr, uint8_t regAddr);

		// low-level PCA register access
		void writePCARegister8(uint8_t pcaAddr, uint8_t regAddr, uint8_t val);
		void writePCARegister16(uint8_t pcaAddr, uint8_t regAddr, uint16_t val);
		uint8_t readPCARegister8(uint8_t pcaAddr, uint8_t regAddr);
		uint16_t readPCARegister16(uint8_t pcaAddr, uint8_t regAddr);

		// check if the PWM part is at given address
		// return true on success, false otherwise
		bool detectPWM(uint8_t addr);
		
		// sets the 2 PWM values for an output
		// values can be 0..4095
		// special values of 0xffff are reserved to turn fully ON or OFF
		// the output (OFF has precedence over ON)
		void setPWMValues(uint8_t port, uint16_t on, uint16_t off);
		
		// check if the digital part is at given address
		// return true on success, false otherwise
		bool detectIO(uint8_t addr);
	
		// initialize and count the connected boards
		uint8_t initialize(void);

	protected:

	public:

		// constructor
		OctopusClass();

		// destructor
		~OctopusClass();
		
		// return number of boards found
		uint8_t getNumBoards(void) const { return nBoards; }
		
		// return number of available I/O
		uint8_t getNumIO(void) const { return nBoards * 16; }
		
		// set pwm frequency for a single connected board
		// valid values 24 Hz...1526 Hz
		void setPWMFreq(uint8_t board, uint16_t freq);
		
		// set pwm frequency for ALL connected boards
		void setPWMFreq(uint16_t freq);
		
		// pwm output
		void analogWrite(uint8_t port, uint16_t val, bool invert = false);
		
		// digital I/O
		void pinMode(uint8_t port, uint8_t mode);
		bool digitalRead(uint8_t port);
		void digitalWrite(uint8_t port, bool value);
		
		// read/write all digital pins of given board at once
		uint16_t digitalReadAll(uint8_t board);
		void digitalWriteAll(uint8_t board, uint16_t val);

};

OctopusClass &__octopus();

#endif
