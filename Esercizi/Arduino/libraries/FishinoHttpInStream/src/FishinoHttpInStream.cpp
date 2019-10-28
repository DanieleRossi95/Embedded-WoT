//////////////////////////////////////////////////////////////////////////////////////
//																					//
//							FishinoHttpInStream.h									//
//				Library to read remote files with HTTP protocol						//
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
//	VERSION 6.0.0	June 2017	INITIAL VERSION										//
//  Version 6.0.2	June 2017	Added peek support to identify audio streams		//
//																					//
//////////////////////////////////////////////////////////////////////////////////////

//#define DEBUG_LEVEL_INFO
#include <FishinoDebug.h>

#include "FishinoHttpInStream.h"

// sets the error code
// (debug hook)
void FishinoHttpInStream::setError(uint16_t httpError)
{
	if(httpError != HttpOk && httpError != PartialContent && httpError != 0)
	{
		DEBUG_ERROR_FUNCTION("Http error : %u\n", httpError);
		_httpError = httpError;
	}
	else
		_httpError = 0;
}
	
// closes and deletes client
bool FishinoHttpInStream::cleanup(void)
{
	if(_client)
	{
		_client->stop();
		delete _client;
		_client = NULL;
	}
	return false;
}

// parse url to fill https, host, port and path
bool FishinoHttpInStream::parseURL(const char *url)
{
	DEBUG_INFO_FUNCTION("ParseURL: '%s'\n", url);

	// read protocol part, if available
	_isHttps = false;
	if(!strncmp(url, "https://", 8))
	{
		_isHttps = true;
		url += 8;
	}
	else if(!strncmp(url, "http://", 7))
		url += 7;
	
	if(_isHttps)
		_port = 443;
	else
		_port = 80;
	
	DEBUG_INFO_FUNCTION("HTTPS: '%s'\n", _isHttps ? "TRUE" : "FALSE");

	// read host name, as the string up to first "/" terminator or end of string
	const char *urlP = url;
	while(*urlP && *urlP != '/' && *urlP != ':')
		urlP++;
	
	// get host
	if(_host)
		free(_host);
	size_t hostLen = urlP - url;
	_host = (char *)malloc(hostLen + 1);
	strncpy(_host, url, hostLen);
	_host[hostLen] = 0;
	url = urlP;
	DEBUG_INFO("Host: '%s'\n", _host);
	
	// if port is given, read it
	if(*url == ':')
	{
		url++;
		_port = 0;
		while(isdigit(*url))
			_port = 10 * _port + *url++ - '0';
	}
	DEBUG_INFO("PORT: %" PRIu32 "\n", _port);
	
	// if string is terminated, we want the root path
	if(!*url)
	{
		_path = (char *)malloc(2);
		_path[0] = '/';
		_path[1] = 0;
		return true;
	}
	
	// copy remaining url part to path
	if(_path)
		free(_path);
	size_t pathLen = strlen(url);
	_path = (char *)malloc(pathLen + 1);
	strncpy(_path, url, pathLen);
	_path[pathLen] = 0;
	
	DEBUG_INFO("Path: '%s'\n", _path);
	return true;
}

// get an HTTP header line from stream
// return NULL ptr if timeout
// on header's end it returns an empty line (NOT null result!)
char *FishinoHttpInStream::getHttpHeaderLine(void)
{
	// check if client is connected
	if(!_client || !_client->connected())
	{
		DEBUG_ERROR_FUNCTION("Client not connected\n");
		setError(HttpNotConnected);
		return NULL;
	}
	
	// wait for data - return NULL if timeout
	uint32_t tim = millis() + HttpHeaderTimeout;
	while(!_client->available() && millis() < tim)
		;
	if(!_client->available())
	{
		DEBUG_ERROR_FUNCTION("HTTP timeout\n");
		setError(RequestTimeout);
		return NULL;
	}
	
	char *buf = (char *)malloc(HttpMaxHeaderLine + 1);
	if(!buf)
	{
		DEBUG_ERROR_FUNCTION("Out of memory\n");
		setError(HttpOutOfMemory);
		return NULL;
	}
	char *bufP = buf;
	
	char c;
	// read loop -- leave on error
	while( (c = _client->read()) != -1)
	{
		// check for EOL
		if(c == '\r')
		{
			// get nect char, leave if error
			c = _client->read();
			if(c == -1)
			{
				DEBUG_ERROR_FUNCTION("Client returned -1\n");
				break;
			}
			
			// end of line ?
			if(c == '\n')
			{
				// yep, return the line
				*bufP = 0;
				return buf;
			}
			
			// add '\r' as a part of the line, if not
			// followed by a '\n'
			if(bufP - buf <= HttpMaxHeaderLine)
				*bufP++ = '\r';
		}
		
		// add character to buffer, if not too many chars
		if(bufP - buf <= HttpMaxHeaderLine)
			*bufP++ = c;
	}
	
	// if here, a read error occurred
	free(buf);
	setError(HttpReadError);
	return NULL;
}
		
// parse headers received from server
// returns true if all ok, false if http error
// _httpError variable is set with error number
// if 'full' is false, just get error code and skip remaining
// if 'full' is true, get relevant parts (file size, range accepted, etc)
bool FishinoHttpInStream::parseHeaders()
{
	// first line is status line
	// HTTP/1.1 200 OK
	// HTTP/1.1 404 Not Found
	char *line = getHttpHeaderLine();
	if(!line)
	{
		DEBUG_ERROR_FUNCTION("Error getting header line\n");
		return false;
	}
	DEBUG_INFO_FUNCTION("HEADER LINE : '%s'\n", line);
	if(strncmp(line, F("HTTP/"), 5))
	{
		DEBUG_ERROR_FUNCTION("Bad header line '%s'\n", line);
		free(line);
		setError(BadHeaders);
		return false;
	}
	// skip the version number
	const char *lineP = line + 5;
	while(*lineP && !isspace(*lineP))
		lineP++;
	while(*lineP && isspace(*lineP))
		lineP++;
	if(!isdigit(*lineP))
	{
		DEBUG_ERROR_FUNCTION("MISSING HTTP STATUS\n");
		free(line);
		return false;
	}
	uint16_t httpError = 0;
	while(isdigit(*lineP))
		httpError = 10 * httpError + *lineP++ - '0';
	free(line);
	setError(httpError);
	DEBUG_INFO("GOT HTTP STATUS %d\n", httpError);
	if(httpError != HttpOk && httpError != PartialContent)
		return false;
	
	// reset content length, we wanna read from headers
	_contentLength = 0;

	// read header's lines and, if requested
	// collect interesting values
	while(true)
	{
		line = getHttpHeaderLine();
		if(!line)
		{
			DEBUG_ERROR_FUNCTION("Error getting header line\n");
			return false;
		}
		DEBUG_INFO_FUNCTION("HEADER LINE : '%s'\n", line);
		
		// empty line == headers end
		if(!*line)
		{
			DEBUG_INFO_FUNCTION("End of headers\n");
			free(line);
			return true;
		}
		
		// analyze line
		if(!strncmp(line, F("Content-Length: "), 16))
		{
			const char *lineP = line + 16;
			while(isdigit(*lineP))
				_contentLength = 10 * _contentLength + *lineP++ - '0';
			DEBUG_INFO_FUNCTION("GOT CONTENT LENGTH : %" PRIu32 "\n", _contentLength);
		}
		
		// free line and prepare to next one
		free(line);
	}
	
}
		
// connect to remote host and retrieve some info
// from headers
bool FishinoHttpInStream::connect(void)
{
	if(_client)
		cleanup();
	if(_isHttps)
		_client = new FishinoSecureClient;
	else
		_client = new FishinoClient;
	if(!_client)
	{
		setError(HttpOutOfMemory);
		return false;
	}

	// connect to client
	if(!_client->connect(_host, _port))
	{
		DEBUG_ERROR_FUNCTION("Can't connect to host '%s', port '%u'\n", _host, (unsigned)_port);
		return cleanup();
	}
	
	// try to spare as much ram as possible on HTTPS sockets
	if(_isHttps)
		_client->setBufferedMode(false);

	// execute an HEAD request, in order to get file size
	// and range capabilities
	if(!sendRequest())
	{
		DEBUG_ERROR_FUNCTION("Error sending request\n");
		return cleanup();
	}
	
 	if(!parseHeaders() || !_contentLength)
 	{
		DEBUG_ERROR_FUNCTION("Error parsing headers\n");
 		return cleanup();
 	}
 		
	// ok, we're connected and we got all relevant data
	// just initialize some vars...
	_remainingSize = _contentLength;
	
	_position = 0;

	// done connecting, all ok
	// if we're in buffered mode, just pre-fill the read buffer
	if(_readBuffer)
		refillBuffer();

	return true;
}

// send request to server
bool FishinoHttpInStream::sendRequest(void)
{
	if(!_client || !_client->connected())
	{
		DEBUG_ERROR_FUNCTION("Client not connected\n");
		return false;
	}
	
	*_client << F("GET ");
	*_client << _path;
		
	*_client
		<< F(" HTTP/1.1\r\n")
		<< F("User-Agent: Fishino 5.2.0\r\n")
		<< F("Host: ")
		<< _host << "\r\n\r\n"
	;
	return true;
}
	
// unbuffered, common to both buffered and unbuffered modes
uint32_t FishinoHttpInStream::readUnbuffered(uint8_t *buf, uint32_t size)
{
	if(!_client || !_client->connected())
	{
		DEBUG_ERROR_FUNCTION("Client not connected\n");
		return 0;
	}

	uint32_t got = 0;
	uint8_t *bufP = buf;
	
	// setup a timeout of HttpDataTimeout mSecs
	uint32_t tim = millis() + HttpDataTimeout;
	
	while(got != size && millis() < tim)
	{
		// read data into buffer
		uint32_t curGot = _client->read(bufP, size - got);

		_remainingSize -= curGot;
		bufP += curGot;
		got += curGot;
	}
	
	DEBUG_INFO_FUNCTION("Requested %" PRIu32 " bytes, got %" PRIu32 "\n", size, got);
	return got;
}

// refill buffer on buffered read
bool FishinoHttpInStream::refillBuffer(void)
{
	if(!_client || !_client->connected())
		return false;
	
	_bufferContent = 0;
	_bufPos = _readBuffer;

	uint32_t requested = min(_bufferSize, _remainingSize);
	uint8_t *bufP = _readBuffer;

	// setup a timeout of HttpDataTimeout mSecs
	uint32_t tim = millis() + HttpDataTimeout;
	
	while(requested && millis() < tim)
	{
		uint32_t got = _client->read(bufP, requested);
		requested -= got;
		_bufferContent += got;
		bufP += got;
	}
	DEBUG_INFO_FUNCTION("Refilling buffer, %" PRIu32 " bytes requested, %" PRIu16 " bytes got\n", min(_bufferSize, _remainingSize), _bufferContent);
	return _bufferContent != 0;
}
		
// read data in buffered mode, if enabled
uint32_t FishinoHttpInStream::readBuffered(uint8_t *buf, uint32_t size)
{
	uint32_t got = 0;
	while(true)
	{
		uint32_t used = _bufPos - _readBuffer;
		uint32_t avail = _bufferContent - used;
		if(avail)
		{
			uint32_t req = (avail >= size - got ? size - got : avail);
			memcpy(buf, _bufPos, req);
			got += req;
			buf += req;
			_bufPos += req;
			_remainingSize -= req;
		}
		if(got == size)
			break;
		
		// still not finished, we shall re-fill the buffer
		if(!refillBuffer())
		{
			DEBUG_ERROR_FUNCTION("Failed refilling buffer\n");
			return got;
		}
	}
	return got;
}

// read data
// return number of actually ridden bytes
uint32_t FishinoHttpInStream::read(uint8_t *buf, uint32_t size)
{
	uint32_t got = 0;
	
	if(!_remainingSize)
	{
		DEBUG_ERROR_FUNCTION("End of data\n");
		if(_client)
			_client->stop();
		return 0;
	}
	if(size > _remainingSize)
		size = _remainingSize;

	// if enabled, use buffered mode
	if(_bufferSize)
		got = readBuffered(buf, size);
	else
		got = readUnbuffered(buf, size);
	
	// update position, for tell() function
	_position += got;

	DEBUG_INFO_FUNCTION("Requested %" PRIu32 " bytes, returned %" PRIu32 "\n", size, got);
	return got;
}
	
// write data
// return number of actually written bytes
uint32_t FishinoHttpInStream::write(uint8_t const *buf, uint32_t len)
{
	// do not support writes by now
	return 0;
}

// seeks stream
// return true on success, false on failure or if stream is non seekable
bool FishinoHttpInStream::seek(int32_t pos, uint8_t whence)
{
	return false;
}

// tells stream position - return -1 if not supported
uint32_t FishinoHttpInStream::tell(void) const
{
	return _position;
}

// check if stream has some data to read
bool FishinoHttpInStream::available(void) const
{
	if(!_client || !_client->connected())
		return 0;
	return _remainingSize != 0;
}

// check if read only
bool FishinoHttpInStream::isReadOnly(void) const
{
	// by now we just support http read
	return true;
}

// check for eof
bool FishinoHttpInStream::isEof(void) const
{
	return !available();
}

// check for error
bool FishinoHttpInStream::isError(void)
{
	return _client && !_client->connected();
}

// connect to remote host for reading
bool FishinoHttpInStream::open(const char *url)
{
	if(!parseURL(url))
		return false;
	return connect();
}

bool FishinoHttpInStream::open(bool https, const char *host, uint32_t port, const char *path)
{
	_isHttps = https;
	_host = strdup(host);
	_port = port;
	_path = strdup(path);
	return connect();
}

bool FishinoHttpInStream::open(bool https, const __FlashStringHelper *host, uint32_t port, const __FlashStringHelper *path)
{
	_isHttps = https;
	_host = strdup(host);
	_port = port;
	_path = strdup(path);
	return connect();
}

// close the client
void FishinoHttpInStream::close(void)
{
	if(_client)
	{
		_client->stop();
		free(_client);
		_client = NULL;
	}
	_position = 0;
}

// check if file is opened
bool FishinoHttpInStream::isOpened(void) const
{
	return _client && _client->connected();
}

// set buffered mode
// if bufSize is set to 0, buffer mode gets disabled
bool FishinoHttpInStream::setBuffered(uint16_t bufSize)
{
	if(_readBuffer)
	{
		free(_readBuffer);
		_readBuffer = NULL;
	}
	
	// if going to unbuffered mode, just return
	if(!bufSize)
		return true;
	
	// allocate the buffer
	_readBuffer = (uint8_t *)malloc(bufSize);
	if(!_readBuffer)
	{
		_bufferSize = 0;
		return false;
	}
	_bufferSize = bufSize;
	_bufferContent = 0;
	_bufPos = _readBuffer;
	
	// prefill buffer if stream is opened
	if(isOpened())
		refillBuffer();
	
	return true;
}

// peeks some bytes from input stream
// copy at most reqSize bytes on dest buffer
// returns size of available data (< reqSize) or 0 if can't peek
uint32_t FishinoHttpInStream::peekBuffer(uint8_t *buf, uint32_t reqSize)
{
	if(!isOpened())
		return 0;

	// if not buffered, we shall activate buffering
	if(!_readBuffer)
	{
		if(!setBuffered(reqSize))
		{
			DEBUG_ERROR("Error activating buffered mode - can't peek\n");
			return 0;
		}
	}
	uint32_t avail = _bufferContent - (_bufPos - _readBuffer);
	if(!avail)
		return 0;
	if(avail > reqSize)
		avail = reqSize;
	memcpy(buf, _bufPos, avail);
	return avail;
}

// constructor
FishinoHttpInStream::FishinoHttpInStream()
{
	// the http client
	_client = NULL;
	
	// https flag
	_isHttps = false;
	
	// the http error code received inside headers
	// on success (code 200) this is set to 0
	_httpError = 0;
	
	// the server
	_host = NULL;
	
	// the port
	_port = 0;
	
	// the path to retrieve
	_path = NULL;
	
	// content length, as got from headers
	_contentLength = 0;

	// remaining stream size, for range read
	_remainingSize = 0;
	
	// buffered size
	_bufferSize = 0;
	
	// buffer total content
	_bufferContent = 0;
	
	// buffer current position
	_bufPos = NULL;
	
	// the read buffer, if enabled
	_readBuffer = NULL;
	
	// position at 0
	_position = 0;
}

// destructor
FishinoHttpInStream::~FishinoHttpInStream()
{
	// free allocated resources
	if(_client)
	{
		_client->stop();
		delete _client;
	}
	if(_host)
		free(_host);
	if(_path)
		free(_path);
	
	if(_readBuffer)
		free(_readBuffer);
}
