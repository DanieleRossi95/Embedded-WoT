//////////////////////////////////////////////////////////////////////////////////////
//																					//
//								FishinoFlash.h										//
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
#ifndef __FISHINO_FLASH_H
#define __FISHINO_FLASH_H

#include <Arduino.h>
#include "MapMacro.h"

// some templates for streaming with operator<<
#ifndef ARDUINO_STREAMING
#define ARDUINO_STREAMING
template<class T> inline Print &operator <<(Print &stream, T arg)
{
	stream.print(arg);
	return stream;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////
// this class is for backware compatibility -- use new FlashString and others for Flash data !!!!	//
//////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _FISHINO_PIC32_

	#define PROGMEM_STRING(name, value) \
		static const char name##_flash[] PROGMEM __attribute__((aligned(4))) = value; \
		const __FlashStringHelper *name = (const __FlashStringHelper *)name##_flash

	inline int strcmp(const char *s1, const __FlashStringHelper *s2)
	{
		return strcmp(s1, (const char *)s2);
	}
	
	inline int strcmp(const __FlashStringHelper *s1, const __FlashStringHelper *s2)
	{
		return strcmp((const char *)s1, (const char *)s2);
	}
	
	inline int strncmp(const char *s1, const __FlashStringHelper *s2, size_t n)
	{
		return strncmp(s1, (const char *)s2, n);
	}

	inline char *strdup(const __FlashStringHelper *s)
	{
		return strdup((const char *)s);
	}
	
	inline char *strcat(char *s, const __FlashStringHelper *sc)
	{
		return strcat(s, (const char *)sc);
	}
	
	inline char *strcpy(char *s, const __FlashStringHelper *sc)
	{
		return strcpy(s, (const char *)sc);
	}

	inline char charAt(const __FlashStringHelper *s, int idx) { return ((const char *)s)[idx]; }

	inline size_t strlen(const __FlashStringHelper *s) { return strlen((const char *)s); }

#else

	#define PROGMEM_STRING(name, value) \
		static const char name##_flash[] PROGMEM = value; \
		const __FlashStringHelper *name = (const __FlashStringHelper *)name##_flash
		
	inline char *strdup(const __FlashStringHelper *s)
	{
		size_t len = strlen_P((const char *)s) + 1;
		char *newS = (char *)malloc(len);
		memcpy_P(newS, s, len);
		return newS;
	}

	inline int strcmp(const char *s1, const __FlashStringHelper *s2)
	{
		return strcmp_P(s1, (const char *)s2);
	}
	
	inline int strcmp(const __FlashStringHelper *s1, const __FlashStringHelper *s2)
	{
		char *s = strdup(s1);
		int res = strcmp_P(s, (const char *)s2);
		free(s);
		return res;
	}
	
	inline int strncmp(const char *s1, const __FlashStringHelper *s2, size_t n)
	{
		return strncmp_P(s1, (const char *)s2, n);
	}

	inline char *strcat(char *s, const __FlashStringHelper *sc)
	{
		return strcpy_P(s + strlen(s), (const char *)sc);
	}
	
	inline char *strcpy(char *s, const __FlashStringHelper *sc)
	{
		return strcpy_P(s, (const char *)sc);
	}

	inline char charAt(const __FlashStringHelper *s, int idx) { return pgm_read_byte((const char *)s + idx); }
	
	inline size_t strlen(const __FlashStringHelper *s) { return strlen_P((const char *)s); }

#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////
// New, easy to use, flash data classes -- use them in replacement to F() and PROGMEM_STRING		//
//////////////////////////////////////////////////////////////////////////////////////////////////////

// empty string to be returned for NULL pointers
static const char __fishinoflash_empty_string[] PROGMEM __attribute__((aligned(4))) = "";

class FlashString
{
	private:
	
		const char *_data;
	
	protected:
	
	public:
	
		FlashString(const char *data) { if(data) _data = data; else _data = __fishinoflash_empty_string; }
	
		// get string length
		size_t strlen(void) const;

		// create a copy of the string in ram
		// (must be freed by caller)
		char *strdup(void) const;
		
		// copy the string inside a buffer in ram
		void strcpy(char *buf) const;
		
		// copy the string inside a buffer in ram, limited at n chars
		void strncpy(char *buf, size_t n) const;
		
		// append the string to a ram one
		void strcat(char *buf);
		
		// append at most n chars to a ram string
		void strncat(char *buf, size_t n);
		
		// compare with another string in ram
		int strcmp(const char *s) const;
		
		// compare first n chars of a string in ram
		int strncmp(const char *s, size_t n) const;
		
		// access single characters
		char at(size_t i) const;
		char operator[](size_t i) const  { return at(i); }
		
		// conversion to old __FlashStringHelper
		operator __FlashStringHelper const *() const { return (const __FlashStringHelper *)_data; }
};

#define FString(s) [] () \
{ \
	static const char __c[] PROGMEM __attribute__((aligned(4))) = (s); \
	return FlashString(__c); \
}()

class FlashStringArray
{
	private:
	
		const char * const *_arr;
		size_t _count;

	protected:
	
	public:
	
		// constructor
		FlashStringArray(char const * const *arr);
		
		// get number of strings
		size_t getCount(void) const { return _count; }

		// access single string
		FlashString at(size_t i) const;
		FlashString operator[](size_t i) const  { return at(i); }
};

#define __DEFSTRING(x, y) static const char CONCAT(_, y)[] PROGMEM __attribute__((aligned(4))) = x;
#define __DEFARRELEM(x, y) CONCAT(_, y),

#define FStringArray(...) [] () \
{ \
	MAP(__DEFSTRING, __VA_ARGS__)	\
	static const char * const __a[] PROGMEM =	\
	{	\
		MAP(__DEFARRELEM, __VA_ARGS__) \
		NULL \
	}; \
	return FlashStringArray(__a); \
}()

class FlashUint8Array
{
	private:
	
		size_t _len;
		const uint8_t *_buf;
	
	protected:
	
	public:
	
		FlashUint8Array(size_t len, const uint8_t *buf) : _len(len), _buf(buf) {}
		
		// get byte at position
		const uint8_t at(size_t i) const;
		const uint8_t operator[](size_t i) const { return at(i); }
		
		// copy block of data to buffer
		size_t copy(size_t start, uint8_t *buf, size_t len) const;
		
		size_t getLen(void) const { return _len; }
		size_t getCount(void) const { return _len; }
	
};

#define FUint8Array(...) [] () \
{ \
	static const uint8_t __buf[] PROGMEM __attribute__((aligned(4))) = { __VA_ARGS__ }; \
	return FlashUint8Array(sizeof(__buf), __buf); \
}()

class FlashUint16Array
{
	private:
	
		size_t _len;
		const uint16_t *_buf;
	
	protected:
	
	public:
	
		FlashUint16Array(size_t len, const uint16_t *buf) : _len(len), _buf(buf) {}
		
		// get byte at position
		const uint16_t at(size_t i) const;
		const uint16_t operator[](size_t i) const { return at(i); }
		
		// copy block of data to buffer
		size_t copy(size_t start, uint16_t *buf, size_t len) const;
		
		size_t getLen(void) const { return _len; }
		size_t getCount(void) const { return _len; }
	
};

#define FUint16Array(...) [] () \
{ \
	static const uint16_t __buf[] PROGMEM __attribute__((aligned(4))) = { __VA_ARGS__ }; \
	return FlashUint16Array(sizeof(__buf) / sizeof(uint16_t), __buf); \
}()

class FlashUint32Array
{
	private:
	
		size_t _len;
		const uint32_t *_buf;
	
	protected:
	
	public:
	
		FlashUint32Array(size_t len, const uint32_t *buf) : _len(len), _buf(buf) {}
		
		// get byte at position
		const uint32_t at(size_t i) const;
		const uint32_t operator[](size_t i) const { return at(i); }
		
		// copy block of data to buffer
		size_t copy(size_t start, uint32_t *buf, size_t len) const;
		
		size_t getLen(void) const { return _len; }
		size_t getCount(void) const { return _len; }
	
};

#define FUint32Array(...) [] () \
{ \
	static const uint32_t __buf[] PROGMEM __attribute__((aligned(4))) = { __VA_ARGS__ }; \
	return FlashUint32Array(sizeof(__buf) / sizeof(uint32_t), __buf); \
}()

#endif

