//////////////////////////////////////////////////////////////////////////////////////
//						 		FishinoRTC_DS1307.cpp								//
//																					//
//					Manage Real Time Controllers on Fishino Boards					//
//																					//
//		Copyright (c) 2017 Massimo Del Fedele.  All rights reserved.				//
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
//  Version 7.3.0 - December 2017	Initial version									//
//  Version 7.3.2 - 2018/01/05		Small fixes										//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#include "FishinoRTC.h"

#include <Wire.h>

#ifdef _HAS_DS1307_

#define DS1307_ADDRESS  0x68
#define DS1307_CONTROL  0x07
#define DS1307_NVRAM    0x08

static uint8_t bcd2bin(uint8_t val)
{
	return val - 6 * (val >> 4);
}
static uint8_t bin2bcd(uint8_t val)
{
	return val + 6 * (val / 10);
}

// constructor
FishinoRTCClass::FishinoRTCClass()
{
	Wire.begin();
}

// destructor
FishinoRTCClass::~FishinoRTCClass()
{
}

bool FishinoRTCClass::isrunning(void) const
{
	Wire.beginTransmission(DS1307_ADDRESS);
	Wire.write((byte)0);
	Wire.endTransmission();

	Wire.requestFrom(DS1307_ADDRESS, 1);
	uint8_t ss = Wire.read();
	return !(ss >> 7);
}

void FishinoRTCClass::adjust(const DateTime& dt)
{
	Wire.beginTransmission(DS1307_ADDRESS);
	Wire.write((byte)0); // start at location 0
	Wire.write(bin2bcd(dt.second()));
	Wire.write(bin2bcd(dt.minute()));
	Wire.write(bin2bcd(dt.hour()));
	Wire.write(bin2bcd(0));
	Wire.write(bin2bcd(dt.day()));
	Wire.write(bin2bcd(dt.month()));
	Wire.write(bin2bcd(dt.year() - 2000));
	Wire.endTransmission();
}

DateTime FishinoRTCClass::now() const
{
	Wire.beginTransmission(DS1307_ADDRESS);
	Wire.write((byte)0);
	Wire.endTransmission();

	Wire.requestFrom(DS1307_ADDRESS, 7);
	uint8_t ss = bcd2bin(Wire.read() & 0x7F);
	uint8_t mm = bcd2bin(Wire.read());
	uint8_t hh = bcd2bin(Wire.read());
	Wire.read();
	uint8_t d = bcd2bin(Wire.read());
	uint8_t m = bcd2bin(Wire.read());
	uint16_t y = bcd2bin(Wire.read()) + 2000;

	return DateTime(y, m, d, hh, mm, ss);
}

void FishinoRTCClass::readnvram(uint8_t* buf, uint8_t size, uint8_t address) const
{
	int addrByte = DS1307_NVRAM + address;
	Wire.beginTransmission(DS1307_ADDRESS);
	Wire.write(addrByte);
	Wire.endTransmission();

	Wire.requestFrom((uint8_t) DS1307_ADDRESS, size);
	for (uint8_t pos = 0; pos < size; ++pos)
	{
		buf[pos] = Wire.read();
	}
}

uint8_t FishinoRTCClass::readnvram(uint8_t address) const
{
	uint8_t data;
	readnvram(&data, 1, address);
	return data;
}

void FishinoRTCClass::writenvram(uint8_t address, uint8_t* buf, uint8_t size)
{
	int addrByte = DS1307_NVRAM + address;
	Wire.beginTransmission(DS1307_ADDRESS);
	Wire.write(addrByte);
	for (uint8_t pos = 0; pos < size; ++pos)
	{
		Wire.write(buf[pos]);
	}
	Wire.endTransmission();
}

void FishinoRTCClass::writenvram(uint8_t address, uint8_t data)
{
	writenvram(address, &data, 1);
}

#endif
