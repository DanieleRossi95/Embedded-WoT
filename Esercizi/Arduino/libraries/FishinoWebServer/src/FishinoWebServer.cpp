//////////////////////////////////////////////////////////////////////////////////////
//																					//
//							FishinoWebServer.cpp									//
//					A small Web Server for Fishino boards							//
//					Created by Massimo Del Fedele, 2016								//
//																					//
//  Copyright (c) 2016 and 2017 Massimo Del Fedele.  All rights reserved.			//
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
//	VERSION 1.0.0 - INITIAL VERSION - BASED ON TinyWebServer						//
//	VERSION 2.0.0 - 18/12/2016 - COMPLETE REWRITE FOR RAM OPTIMIZATION				//
//  Version 6.0.0 -- June 2017 - USE NEW DEBUG LIBRARY								//
//	VERSION 7.0.0 - 18/12/2016 - DISCARD 'GET' PART OF URL							//
//	VERSION 7.0.1 - 09/10/2017 - FIXED A BUG ON HANDLERS TERMINATION				//
//	VERSION 7.1.0 - 20/10/2017 - FIXED A BUG ON HANDLERS TERMINATION				//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#include "FishinoWebServer.h"

//#define DEBUG_MEMORY_ALLOC
#define DEBUG_LEVEL_INFO
#include <FishinoDebug.h>

// socket read timeout
#define SOCKET_READ_TIMEOUT	10

// buffer size for file sending
#if ((RAMEND - RAMSTART) < 4096)
#define FILEBUF_SIZE	32
#else
#define FILEBUF_SIZE	256
#endif

PROGMEM_STRING(mimeTypes,
	"HTM*text/html|"
	"TXT*text/plain|"
	"CSS*text/css|"
	"XML*text/xml|"
	"JS*text/javascript|"
	
	"GIF*image/gif|"
	"JPG*image/jpeg|"
	"PNG*image/png|"
	"ICO*image/vnd.microsoft.icon|"
	
	"MP3*audio/mpeg|"
);

// Offset for text/html in `mime_types' above.
static const FishinoWebServer::MimeType TextHtmlContentType = 4;

PROGMEM_STRING(contentTypeMsg, "Content-Type: ");

// constructor
FishinoWebServer::FishinoWebServer(uint16_t port) : _server(port)
{
	// no full path mode by default
	_fullPathMode = false;
	
	// the headers
	_numHeaders = 0;
	_headers = NULL;
	
	// the header values
	_headerValues = NULL;
	
	// the handlers
	_numHandlers = 0;
	_handlers = NULL;
	
	_putHandler = NULL;
	
	// request and path
	_request = UNKNOWN_REQUEST;
	_requestPath = NULL;
}

// destructor
FishinoWebServer::~FishinoWebServer()
{
	if(_headers)
		DEBUG_FREE(_headers);
	if(_handlers)
		DEBUG_FREE(_handlers);
}

// set full path mode
// (doesn't strip paths from requests - keep cases
FishinoWebServer &FishinoWebServer::setFullPathMode(bool mode)
{
	_fullPathMode = mode;
	return *this;
}

// add an accepted header
FishinoWebServer &FishinoWebServer::addHeader(const __FlashStringHelper *header)
{
	if(!_numHeaders)
		_headers = (const __FlashStringHelper **)DEBUG_MALLOC(sizeof(const __FlashStringHelper *));
	else
		_headers = (const __FlashStringHelper **)DEBUG_REALLOC(_headers, (_numHeaders + 1) * sizeof(const __FlashStringHelper *));
	_headers[_numHeaders] = header;
	_numHeaders++;
	return *this;
}

// add an handler
FishinoWebServer &FishinoWebServer::addHandler(const __FlashStringHelper *path, HttpRequestType type, FishinoWebServerHandler handler)
{
	if(!_numHandlers)
		_handlers = (PathHandler *)DEBUG_MALLOC(sizeof(PathHandler));
	else
		_handlers = (PathHandler *)DEBUG_REALLOC(_handlers, (_numHandlers + 1) * sizeof(PathHandler));
	_handlers[_numHandlers].path = path;
	_handlers[_numHandlers].type = type;
	_handlers[_numHandlers].handler = handler;
	_numHandlers++;
	return *this;
}

// set the put handler
FishinoWebServer &FishinoWebServer::setPutHandler(FishinoWebServerPutHandler handler)
{
	_putHandler = handler;
	
	return *this;
}

// put handler -- to be used inside addHandler function
bool FishinoWebServer::putHandler(FishinoWebServer &s)
{
	FishinoClient &client = s.getClient();

	// if no put handler defined, just drop the connection
	if(!s._putHandler)
	{
		client.stop();
		return false;
	}
	
	uint32_t bufSize = Fishino.freeRam() / 2;
	if(bufSize > 1024)
		bufSize = 1024;
	char buf[bufSize];
		
	s.sendErrorCode(200);
	s.endHeaders();

	const char* lengthStr = s.getHeaderValue("Content-Length");
	unsigned long length = atol(lengthStr);
	uint32_t startTime = 0;
	boolean watchdogStart = false;

	s._putHandler(s, START, NULL, length);

	uint32_t i;
	for (i = 0; i < length && client.connected();)
	{
		uint16_t size = s.readChars((uint8_t*)buf, bufSize);
		if (!size)
		{
			if (watchdogStart)
			{
				if (millis() - startTime > 30000)
				{
					// Exit if there has been zero data from connected client
					// for more than 30 seconds.
#if FISHINO_WEBSERVER_DEBUG
					Serial << F("TWS:There has been no data for >30 Sec.\n");
#endif
					break;
				}
			}
			else
			{
				// We have hit an empty buffer, start the watchdog.
				startTime = millis();
				watchdogStart = true;
			}
			continue;
		}
		i += size;
		// Ensure we re-start the watchdog if we get ANY data input.
		watchdogStart = false;

		s._putHandler(s, WRITE, buf, size);
	}
	s._putHandler(s, END, NULL, 0);

	return true;
}

// helper -- read a char from client
int FishinoWebServer::readChar(void)
{
	return _client.read();
}

// helper -- read chars from client
size_t FishinoWebServer::readChars(uint8_t *buf, size_t maxSize)
{
	return _client.read(buf, maxSize);
}

// skip till start of next line
bool FishinoWebServer::skipLine(void)
{
	while(_client.available())
		if(readChar() == '\n')
			return true;
	return false;
}

// free local data
void FishinoWebServer::freeHeaders(void)
{
	// free header values
	if(_headerValues)
	{
		for(uint8_t i = 0; i < _numHeaders; i++)
			if(_headerValues[i])
				DEBUG_FREE(_headerValues[i]);
		DEBUG_FREE(_headerValues);
		_headerValues = NULL;
	}
}

void FishinoWebServer::freeRequest(void)
{
	// free stored request path
	if(_requestPath)
	{
		DEBUG_FREE(_requestPath);
		_requestPath = NULL;
	}
	_request = UNKNOWN_REQUEST;
}

void FishinoWebServer::freeLocalData(void)
{
	freeRequest();
	freeHeaders();
}

// parse request line
bool FishinoWebServer::parseRequest(void)
{
	freeRequest();
	
	// request can be GET, POST, PUT or DELETE
	char c = readChar();
	if(c == 'G')
	{
		if(readChar() != 'E' || readChar() != 'T')
			return false;
		_request = GET;
	}
	else if(c == 'P')
	{
		c = readChar();
		if(c == 'U')
		{
			if(readChar() != 'T')
				return false;
			_request = PUT;
		}
		else if(c == 'O')
		{
			if(readChar() != 'S' || readChar() != 'T')
				return false;
			_request = POST;
		}
		else
			return false;
	}
	else if(c == 'D')
	{
		if(readChar() != 'E' || readChar() != 'L' || readChar() != 'E' || readChar() != 'T' || readChar() != 'E')
			return false;
		_request = DELETE;
	}
	else
		return false;
	
	// skip space before path
	readChar();
	
	// read the path
	String path;
	while(_client.available())
	{
		c = readChar();
		if(isspace(c) || c == '?')
			break;
		path += c;
	}
	_requestPath = strdup(path.c_str());
	
	// by now just skip any 'GET' URL part
	// in future we could parse it!
	
	// skip to next line (header start)
	skipLine();
	
	return _client.available();
}
		
// parse headers
bool FishinoWebServer::parseHeaders(void)
{
	freeHeaders();
	if(_numHeaders)
	{
		_headerValues = (char **)DEBUG_MALLOC(_numHeaders * sizeof(char *));
		memset(_headerValues, 0, _numHeaders * sizeof(char *));
	}

	int c;
	while(true)
	{
		String name;
		c = readChar();
		
		// unexpexted end of file
		if(c == -1)
			return false;
		
		// check for end of headere
		if(c == '\r')
		{
			skipLine();
			return true;
		}
		// read name
		while(isalnum(c) || c == '-')
		{
			name += (char)c;
			c = readChar();
			if(c == -1)
				return false;
		}
		// check if name match one of requested headers
		bool match = false;
		uint8_t iHeader = 0;
		while(iHeader < _numHeaders)
		{
			if(!strcmp(name.c_str(), _headers[iHeader]))
			{
				match = true;
				break;
			}
			iHeader++;
		}
		name = "";
		if(match)
		{
			// skip (expected) ':' character
			if(c != ':')
				return false;

			// skip spaces
			do
			{
				c = readChar();
				if(c == -1)
					return false;
			}
			while(isspace(c));
			
			// read the value
			String value;
			while(c != '\r')
			{
				value += (char)c;
				c = readChar();
				if(c == -1)
					return false;
			}
			
			// store the value
			if(value.length())
				_headerValues[iHeader] = strdup(value.c_str());
			value = "";

			// go to start of next line
			skipLine();
		}
		else
			skipLine();
	}
}

// These methods write directly in the response stream of the connected client
size_t FishinoWebServer::write(uint8_t c)
{
	return _client.write(c);
}

size_t FishinoWebServer::write(const char *str)
{
	return _client.write(str);
}

size_t FishinoWebServer::write(const uint8_t *buffer, size_t size)
{
	return _client.write(buffer, size);
}

// send error code to client
bool FishinoWebServer::sendErrorCode(uint16_t errCode)
{
	_client << F("HTTP/1.1 ") << errCode << F(" OK\r\n");
	if (errCode != 200)
		endHeaders();
	return true;
}

// send content-type to client
bool FishinoWebServer::sendContentType(MimeType mimeType)
{
	_client << contentTypeMsg;

	char c;
	int i = mimeType;
	while ((c = charAt(mimeTypes, i++)) != '|')
		_client.print(c);
	_client.println();
	return true;
}

bool FishinoWebServer::sendContentType(const __FlashStringHelper *mimeType)
{
	_client << contentTypeMsg << mimeType << "\r\n";
	return true;
}

// send end-of-headers to client
bool FishinoWebServer::endHeaders(void)
{
	_client.println();
	return true;
}

// send a file to client
bool FishinoWebServer::sendFile(SdFile& file)
{
	// use buffer size based on free RAM
	// (use half of it as a size)
	uint32_t bufSize = Fishino.freeRam() / 2;
	if(bufSize > 1024)
		bufSize = 1024;
	uint8_t buf[bufSize];
	
	size_t size;
	while ((size = file.read(buf, bufSize)) > 0)
	{
		if (!_client.connected())
			break;
		write(buf, size);
	}
	return true;
}

// get the mime type from file extension
FishinoWebServer::MimeType FishinoWebServer::getMimeTypeFromFilename(const char *name)
{
	MimeType r = TextHtmlContentType;
	if (!name)
		return r;

	char* ext = strrchr(name, '.');
	if (ext)
	{
		// We found an extension. Skip past the '.'
		ext++;

		char c;
		unsigned i = 0;
		while (i < strlen(mimeTypes))
		{
			// Compare the extension.
			char* p = ext;
			c = charAt(mimeTypes, i);
			while (*p && c != '*' && toupper(*p) == c)
			{
				p++;
				i++;
				c = charAt(mimeTypes, i);
			}
			if (!*p && c == '*')
			{
				// We reached the end of the extension while checking
				// equality with a MIME type: we have a match. Increment i
				// to reach past the '*' char, and assign it to `mime_type'.
				r = ++i;
				break;
			}
			else
			{
				// Skip past the the '|' character indicating the end of a
				// MIME type.
				while(charAt(mimeTypes, i++) != '|')
					;
			}
		}
	}
	return r;
}

// get request's path
const char *FishinoWebServer::getPath(void)
{
	return _requestPath;
}

// parse an exadecilam character
static int parseHexChar(char c)
{
	if (isdigit(c))
		return c - '0';

	c = tolower(c);
	if (c >= 'a' &&  c <= 'e')
		return c - 'a' + 10;

	return 0;
}

// decode an url-encoded string
String FishinoWebServer::decodeUrl(const char *url)
{
	if (!url)
		return "";

	char* r = (char*)DEBUG_MALLOC(strlen(url) + 1);
	if (!r)
		return "";

	char* r2 = r;
	const char* p = url;
	while (*url && (p = strchr(url, '%')))
	{
		if (p - url)
		{
			memcpy(r2, url, p - url);
			r2 += (p - url);
		}
		// If the remaining number of characters is less than 3, we cannot
		// have a complete escape sequence. Break early.
		if (strlen(p) < 3)
		{
			// Move the new beginning to the value of p.
			url = p;
			break;
		}
		uint8_t r = parseHexChar(*(p + 1)) << 4 | parseHexChar(*(p + 2));
		*r2++ = r;
		p += 3;

		// Move the new beginning to the value of p.
		url = p;
	}
	// Copy whatever is left of the string in the result.
	int len = strlen(url);
	if (len > 0)
		strncpy(r2, url, len);

	// Append the 0 terminator.
	*(r2 + len) = 0;

	String res = r;
	DEBUG_FREE(r);
	return res;
}


// get the file path from request path
String FishinoWebServer::getFileFromPath(const char *path)
{
	String decoded;
	
	// strip paths and convert to uppercase
	// if not in full path mode
	if(!_fullPathMode)
	{
		// Obtain the last path component.
		const char* encodedName = strrchr(path, '/');
		if (!encodedName)
			return "";
		else
			// Skip past the '/'.
			encodedName++;

		decoded = decodeUrl(encodedName);
		decoded.toUpperCase();
	}
	else
		decoded = decodeUrl(path);

	return decoded;
}

// get header value
const char *FishinoWebServer::getHeaderValue(const char *name)
{
	for(uint16_t i = 0; i < _numHeaders; i++)
		if(!strcmp(name, _headers[i]))
			return _headerValues[i];
	return "";
}

// starts the server
bool FishinoWebServer::begin(void)
{
	_server.begin();
	return true;
}

// process client connections
bool FishinoWebServer::process(void)
{
	// check if server has requests
	_client = _server.available();
	if (!_client)
		return false;
		
	if(!_client.available())
	{
//		_client.stop();
		return false;
	}
	
	// parse the request line
	if(!parseRequest())
	{
		sendErrorCode(414);
		_client.stop();
		freeLocalData();
		DEBUG_ERROR("Error parsing request\n");
		return false;
	}

	// parse the headers
	if(!parseHeaders())
	{
		sendErrorCode(417);
		_client.stop();
		freeLocalData();
		DEBUG_ERROR("Error parsing headers\n");
		return false;
	}

//Serial << "Request:" << _request << "  Url:" << _requestPath << "\n";
	
	bool shouldClose = true;
	bool found = false;
	for (int i = 0; i < _numHandlers; i++)
	{
		int len = strlen(_handlers[i].path);
		bool exactMatch = !strcmp(_requestPath, _handlers[i].path);
		bool regexMatch = false;
		if(charAt(_handlers[i].path, len - 1) == '*')
			regexMatch = !strncmp(_requestPath, _handlers[i].path, len - 1);

		if ((exactMatch || regexMatch) && (_handlers[i].type == ANY || _handlers[i].type == _request))
		{
			found = true;
			shouldClose = (_handlers[i].handler)(*this);
			break;
		}
	}

	if (!found)
		sendErrorCode(404);
	if (shouldClose)
		_client.stop();

	freeLocalData();
	return true;
}


