//////////////////////////////////////////////////////////////////////////////////////
//																					//
//								FishinoFlash.cpp									//
//					Library for an easy usage of flash data							//
//					Created by Massimo Del Fedele, 2017								//
//																					//
//  Copyright (c) 2016, 2017 Massimo Del Fedele.  All rights reserved.				//
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
//	VERSION 2.0.0 - 06/01/2016 - REWROTE SPI INTERFACE AND ERROR HANDLING			//
//	VERSION 4.0.0 - 01/01/2017 - REWROTE SPI INTERFACE AND ERROR HANDLING			//
//	VERSION 5.1.0 - 04/05/2017 - USE NEW DEBUG LIBRARY								//
//	VERSION 5.2.0 - 20/05/2017 - USE NEW DEBUG LIBRARY								//
//  Version 6.0.0 - June 2017  - USE NEW DEBUG LIBRARY								//
//  Version 7.0.0 - July 2017  - ADDED NEW, EASY TO USE CLASSES						//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#include "FishinoFlash.h"

// get string length
size_t FlashString::strlen(void) const
{
#ifdef _FISHINO_PIC32_
	size_t res = 0;
	while(at(res++))
		;
	return res - 1;
#else
	return strlen_P(_data);
#endif
}

// create a copy of the string in ram
// (must be freed by caller)
char *FlashString::strdup(void) const
{
#ifdef _FISHINO_PIC32_
	return ::strdup(_data);
#else
	size_t len = strlen_P(_data);
	char *buf = (char *)malloc(len + 1);
	::strcpy_P(buf, _data);
	return buf;
#endif
}

// copy the string inside a buffer in ram
void FlashString::strcpy(char *buf) const
{
#ifdef _FISHINO_PIC32_
	::strcpy(buf, _data);
#else
	::strcpy_P(buf, _data);
#endif
}

// copy the string inside a buffer in ram, limited at n chars
void FlashString::strncpy(char *buf, size_t n) const
{
#ifdef _FISHINO_PIC32_
	::strncpy(buf, _data, n);
#else
	::strncpy_P(buf, _data, n);
#endif
}

// append the string to a ram one
void FlashString::strcat(char *buf)
{
#ifdef _FISHINO_PIC32_
	::strcpy(buf + ::strlen(buf), _data);
#else
	::strcpy_P(buf + ::strlen(buf), (const char *)_data);
#endif
}

// append at most n chars to a ram string
void FlashString::strncat(char *buf, size_t n)
{
#ifdef _FISHINO_PIC32_
	::strncpy(buf + ::strlen(buf), _data, n);
#else
	::strncpy_P(buf + ::strlen(buf), (const char *)_data, n);
#endif
}

// compare with another string in ram
int FlashString::strcmp(const char *s) const
{
#ifdef _FISHINO_PIC32_
	return ::strcmp(s, _data);
#else
	return ::strcmp_P(s, _data);
#endif
}

// compare first n chars of a string in ram
int FlashString::strncmp(const char *s, size_t n) const
{
#ifdef _FISHINO_PIC32_
	return ::strncmp(s, _data, n);
#else
	return ::strncmp_P(s, _data, n);
#endif
}

// access single characters
char FlashString::at(size_t i) const
{
#ifdef _FISHINO_PIC32_
	return _data[i];
#else
	return pgm_read_byte(_data + i);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////

FlashStringArray::FlashStringArray(char const * const *arr)
{
	_count = 0;
	_arr = arr;
	if(!arr)
		return;

	const char * const *p = arr;
#ifdef _FISHINO_PIC32_
	while(*p++)
		_count++;
#else
	while(pgm_read_ptr(p++))
		_count++;
#endif
}

// access single string
FlashString FlashStringArray::at(size_t i) const
{
	if(i >= _count)
		return FlashString(NULL);
#ifdef _FISHINO_PIC32_
	return FlashString(_arr[i]);
#else
	return FlashString((const char *)pgm_read_ptr(_arr + i));
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////

// get byte at position
const uint8_t FlashUint8Array::at(size_t i) const
{
#ifdef _FISHINO_PIC32_
	return _buf[i];
#else
	return pgm_read_byte(_buf + i);
#endif
}

// copy block of data to buffer
size_t  FlashUint8Array::copy(size_t start, uint8_t *buf, size_t len) const
{
	if(start >= _len)
		return 0;
	if(start + len >= _len)
		len = _len - start;
	
#ifdef _FISHINO_PIC32_
	memcpy(buf, _buf + start, len);
#else
	for(size_t i = start; i < start + len; i++)
		*buf++ = at(i);
#endif
	return len;
}

///////////////////////////////////////////////////////////////////////////////////////////

// get dword at position
const uint16_t FlashUint16Array::at(size_t i) const
{
#ifdef _FISHINO_PIC32_
	return _buf[i];
#else
	return pgm_read_word(_buf + i);
#endif
}

// copy block of data to buffer
size_t  FlashUint16Array::copy(size_t start, uint16_t *buf, size_t len) const
{
	if(start >= _len)
		return 0;
	if(start + len >= _len)
		len = _len - start;
	
#ifdef _FISHINO_PIC32_
	memcpy(buf, _buf + start, len * sizeof(uint16_t));
#else
	for(size_t i = start; i < start + len; i++)
		*buf++ = at(i);
#endif
	return len;
}

///////////////////////////////////////////////////////////////////////////////////////////

// get dword at position
const uint32_t FlashUint32Array::at(size_t i) const
{
#ifdef _FISHINO_PIC32_
	return _buf[i];
#else
	return pgm_read_dword(_buf + i);
#endif
}

// copy block of data to buffer
size_t  FlashUint32Array::copy(size_t start, uint32_t *buf, size_t len) const
{
	if(start >= _len)
		return 0;
	if(start + len >= _len)
		len = _len - start;
	
#ifdef _FISHINO_PIC32_
	memcpy(buf, _buf + start, len * sizeof(uint32_t));
#else
	for(size_t i = start; i < start + len; i++)
		*buf++ = at(i);
#endif
	return len;
}
