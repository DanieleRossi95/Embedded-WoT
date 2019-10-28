#include "StringStream.h"

#define DEBUG_LEVEL_ERROR
#include <FishinoDebug.h>

// constructor
StringStream::StringStream()
{
}

// destructor
StringStream::~StringStream()
{
}

// get current string
String StringStream::get()
{
	String s2 = _s;
	_s = "";
	return s2;
}
		
size_t StringStream::write(uint8_t *buf, size_t len)
{
	char *s2 = (char *)malloc(len + 1);
	memcpy(s2, buf, len);
	s2[len] = 0;
	_s += s2;
	return len;
}

size_t StringStream::write(uint8_t b)
{
	_s += (char)b;
	return 1;
}
