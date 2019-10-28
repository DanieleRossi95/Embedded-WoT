#include "cit.h"

#include <Fishino.h>

// constructor
Cit::Cit()
{
	_host = NULL;
	_path = NULL;
	_lastId = -1;
}

// destructor
Cit::~Cit()
{
}

// set citation server host and path
Cit &Cit::setHost(const __FlashStringHelper *host)
{
	_host = host;
	return *this;
}

Cit &Cit::setPath(const __FlashStringHelper *path)
{
	_path = path;
	return *this;
}

// debug helper
int Cit::readChar(void)
{
	int c = read();
/*
	if(c == -1)
		return c;
	Serial << (char)c;
*/
	return c;
}
		

// skip http headers
bool Cit::skipHeaders(void)
{
	uint16_t pos = 0;
	while(available())
	{
		if(readChar() == '\r')
		{
			// skip \n
			readChar();
			
			if(pos == 0)
				return true;
			pos = 0;
		}
		else
			pos++;
	}
	return false;
}

// read a number from client
bool Cit::readNum(uint16_t &num)
{
	num = 0;
	uint8_t digits = 0;
	int c;
	while(true)
	{
		c = readChar();
		if(!isdigit(c))
			break;
		num = 10 * num + c - '0';
		digits++;
	}
	if(!digits || c != '&')
	{
		stop();
		return false;
	}
	return true;
}

// query for a new citation, storing message id and length
// if id is given, use it to fetch the citation, otherwise get a random one
bool Cit::query(uint16_t id)
{
	if(!_host || !_path)
		return false;
	
	// connect to host
	String host = _host;
	if(!connect(host.c_str(), 80))
		return false;
	
	// send request
	print(F("GET "));
	print(_path);
	if(id != (uint16_t)-1)
	{
		print(F("?id="));
		print(id);
	}
	println(F(" HTTP/1.0"));
	print(F("Host: "));
	println(_host);
	println(F("User-Agent: cit"));
	println();

	// wait till some data is available
	uint32_t tim = millis() + 1000;
	while(!available() && millis() < tim)
		;
	if(!available())
	{
		stop();
		return false;
	}
	
	// skip http headers
	if(!skipHeaders())
		return false;
	
	// read citation id
	if(!readNum(_lastId))
		return false;
	
	// read message length
	if(!readNum(_msgLen))
		return false;
	
	return true;
}
