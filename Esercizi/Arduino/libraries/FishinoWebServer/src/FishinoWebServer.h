//////////////////////////////////////////////////////////////////////////////////////
//																					//
//							FishinoWebServer.h										//
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
#ifndef __FISHINO_WEBSERVER_H
#define __FISHINO_WEBSERVER_H

#include "Fishino.h"
#include <SD.h>

class FishinoWebServer;
class SdFile;

// web server handler type
typedef bool (*FishinoWebServerHandler)(FishinoWebServer &s);

// web server put handler type
typedef bool (*FishinoWebServerPutHandler)(FishinoWebServer &s, uint8_t action, char* buffer, int size);

class FishinoWebServer : public Print
{
	public:
	
		// types of http requests
		enum HttpRequestType
		{
			UNKNOWN_REQUEST,
			GET,
			HEAD,
			POST,
			PUT,
			DELETE,
			ANY,
		};
		
		// actions for PUT upload command
		enum PutAction
		{
			START,
			WRITE,
			END
		};

		// An identifier for a MIME type. The number is opaque to a human,
		// but it's really an offset in the `mime_types' array.
		typedef uint16_t MimeType;

		enum MimeTypes
		{
			MIMETYPE_HTML	= 4,
			MIMETYPE_TEXT	= 18,
			MIMETYPE_CSS	= 33,
			MIMETYPE_XML	= 46,
			MIMETYPE_JS		= 58,
			MIMETYPE_GIF	= 78,
			MIMETYPE_JPG	= 92,
			MIMETYPE_PNG	= 107,
			MIMETYPE_ICO	= 121,
			MIMETYPE_MPEG	= 150
		};


	private:
	
		// path handler definition
		struct PathHandler
		{
			const __FlashStringHelper *path;
			HttpRequestType type;
			FishinoWebServerHandler handler;
		};
		
		// full path mode
		bool _fullPathMode;
		
		// the headers
		uint8_t _numHeaders;
		const __FlashStringHelper **_headers;
		
		// the header values, got from server
		char **_headerValues;
		
		// the handlers
		uint8_t _numHandlers;
		PathHandler *_handlers;
		
		// the put handler
		FishinoWebServerPutHandler _putHandler;
		
		// the connected server
		FishinoServer _server;
		
		// the connected client
		FishinoClient _client;
		
		// currently parsed request
		HttpRequestType _request;
		
		// currently parsed request path
		char *_requestPath;
		
		// parse request line
		bool parseRequest(void);
		
		// parse headers
		bool parseHeaders(void);
		
		// helper -- read a char from client
		int readChar(void);
		
		// helper -- read chars from client
		size_t readChars(uint8_t *buf, size_t maxSize);
		
		// skip till start of next line
		bool skipLine(void);
		
		// free local data
		void freeHeaders(void);
		void freeRequest(void);
		void freeLocalData(void);
		
		// decode an url-encoded string
		String decodeUrl(const char *url);
		
	protected:
	
	public:
	
		// constructor
		FishinoWebServer(uint16_t port);
		
		// destructor
		~FishinoWebServer();
		
		// set full path mode
		// (doesn't strip paths from requests - keep cases
		FishinoWebServer &setFullPathMode(bool mode = true);
		
		// add an accepted header
		FishinoWebServer &addHeader(const __FlashStringHelper *header);
		
		// add an handler
		FishinoWebServer &addHandler(const __FlashStringHelper *path, HttpRequestType type, FishinoWebServerHandler handler);
		
		// set the put handler
		FishinoWebServer &setPutHandler(FishinoWebServerPutHandler handler);

		// put handler -- to be used inside addHandler function
		static bool putHandler(FishinoWebServer &s);

		// These methods write directly in the response stream of the connected client
		virtual size_t write(uint8_t c);
		virtual size_t write(const char *str);
		virtual size_t write(const uint8_t *buffer, size_t size);
		
		// send error code to client
		bool sendErrorCode(uint16_t errCode);
		
		// send content-type to client
		bool sendContentType(MimeType mimeType);
		bool sendContentType(const __FlashStringHelper *mimeType);
		
		// send end-of-headers to client
		bool endHeaders(void);
		
		// send a file to client
		bool sendFile(SdFile& file);
		
		// get the mime type from file extension
		MimeType getMimeTypeFromFilename(const char *name);
		
		// get request's type
		HttpRequestType getType(void) { return _request; }
		
		// get request's path
		const char *getPath(void);
		
		// get the file path from request path
		String getFileFromPath(const char *path);
		
		// get the client object
		FishinoClient &getClient(void) { return _client; }
		
		// get header value
		const char *getHeaderValue(const char *name);
		
		// starts the server
		bool begin(void);
		
		// process client connections
		bool process(void);
};

#endif
