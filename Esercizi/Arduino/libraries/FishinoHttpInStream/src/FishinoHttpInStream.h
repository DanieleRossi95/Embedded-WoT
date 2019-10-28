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
#ifndef __FISHINOHTTPSTREAM_FISHINOHTTPSTREAM_H
#define __FISHINOHTTPSTREAM_FISHINOHTTPSTREAM_H

#include <FishinoStream.h>
#include <Fishino.h>

enum HttpResultCodes
{
	HttpOk							= 200,	// OK
	PartialContent					= 206,	// partial content returned (on range request...)
	BadRequest						= 400,	// Bad Request
	Unauthorized					= 401,	// Unauthorized
	PaymentRequired					= 402,	// Payment Required
	Forbidden						= 403,	// Forbidden
	NotFound						= 404,	// Not Found
	MethodNotAllowed				= 405,	// Method Not Allowed
	NotAcceptable					= 406,	// Not Acceptable
	ProxyAuthenticationRequired		= 407,	// Proxy Authentication Required
	RequestTimeout					= 408,	// Request Timeout
	Conflict						= 409,	// Conflict
	Gone							= 410,	// Gone
	LengthRequired					= 411,	// Length Required
	PreconditionFailed				= 412,	// Precondition Failed
	RequestEntityTooLarge			= 413,	// Request Entity Too Large
	RequestURITooLong				= 414,	// Request-URI Too Long
	UnsupportedMediaType			= 415,	// Unsupported Media Type
	RequestedRangeNotSatisfiable	= 416,	// Requested Range Not Satisfiable
	ExpectationFailed				= 417,	// Expectation Failed
	Teapot							= 418,	// I'm a teapot
	EnhanceYourCalm					= 420,	// Enhance your calm
	UnprocessableEntity				= 422,	// Unprocessable Entity
	UpgradeRequired					= 426,	// Upgrade Required (RFC 2817)
	RetryWith						= 449,	// Retry With
	UnavailableForLegalReasons		= 451,	// Unavailable For Legal Reasons (Approvato da Internet Engineering Steering Group IESG)
	InternalServerError				= 500,	// Internal Server Error
	NotImplemented					= 501,	// Not Implemented
	BadGateway						= 502,	// Bad Gateway
	ServiceUnavailable				= 503,	// Service Unavailable
	GatewayTimeout					= 504,	// Gateway Timeout
	HTTPVersionNotSupported			= 505,	// HTTP Version Not Supported
	BandwidthLimitExceeded			= 509, 	// Bandwidth Limit Exceeded
	
	// these ones are added to signal other errors
	HttpUnknownCode					= 900,	// the code is not among previously listed
	HttpConnectionFailed			= 901,	// not connected - connection failed
	BadHeaders						= 902,	// first line of http response is bad
	MissingContentLength			= 903,	// for range read we need content length
	HttpOutOfMemory					= 904,	// out of memory on malloc
	HttpReadError					= 905,	// error reading from client
	HttpNotConnected				= 906,	// not connected or connection lost
};

// http timeout reading headers - 1 second here
const int HttpHeaderTimeout			= 2000;

const int HttpDataTimeout			= 2000;

// maximum accepted length for header line
// longer lines will be truncated
const int HttpMaxHeaderLine	= 80;

// maximum chunk size if we use range read
// (shall be less than 7 KBytes because of ESP8266 8K buffer on SSL connections)
const int HttpMaxChunkSize	= 6144; // assume less than 2K headers

class FishinoHttpInStream : public FishinoStream
{
	private:
	
		// the http client
		FishinoClient *_client;
		
		// https flag
		bool _isHttps;
		
		// the http error code received inside headers
		// on success (code 200) this is set to 0
		uint16_t _httpError;
		
		// the server
		char *_host;
		
		// the port
		uint32_t _port;
		
		// the path to retrieve
		char *_path;
		
		// content length, as got from headers
		uint32_t _contentLength;
	
		// remaining stream size, for range read
		uint32_t _remainingSize;
		
		// buffered size
		uint16_t _bufferSize;
		
		// buffer total content
		uint16_t _bufferContent;
		
		// buffer current position
		uint8_t const *_bufPos;
		
		// the read buffer, if enabled
		uint8_t *_readBuffer;
		
		// position (needed for tell() function
		uint32_t _position;
	
	protected:
	
		// sets the error code
		// (debug hook)
		void setError(uint16_t httpError);
	
		// closes and deletes client
		bool cleanup(void);
		
		// parse url to fill https, host, port and path
		bool parseURL(const char *url);
		
		// get an HTTP header line from stream
		// return NULL ptr if timeout
		// on header's end it returns an empty line (NOT null result!)
		// result buffer MUST be freed by caller
		char *getHttpHeaderLine(void);
		
		// parse headers received from server
		// returns true if all ok, false if http error
		// _httpError variable is set with error number
		// if 'full' is false, just get error code and skip remaining
		// if 'full' is true, get relevant parts (file size, range accepted, etc)
		bool parseHeaders(void);
		
		// connect to remote host and retrieve some info
		// from headers
		bool connect(void);
		
		// send request to server
		bool sendRequest(void);
		
		// refill buffer on buffered read
		bool refillBuffer(void);
		
		// base read, common to both buffered and unbuffered modes
		uint32_t readUnbuffered(uint8_t *buf, uint32_t size);
		
		// read data in buffered mode, if enabled
		uint32_t readBuffered(uint8_t *buf, uint32_t size);
	
	public:
	
		// read data
		// return number of actually ridden bytes
		virtual uint32_t read(uint8_t *buf, uint32_t len);
		
		// write data
		// return number of actually written bytes
		virtual uint32_t write(uint8_t const *buf, uint32_t len);
		
		// seeks stream
		// return true on success, false on failure or if stream is non seekable
		virtual bool seek(int32_t pos, uint8_t whence);
		
		// tells stream position
		// return current file position
		virtual uint32_t tell(void) const;
		
		// check if stream has some data to read
		// return true if data is available, false otherwise
		virtual bool available(void) const;
		
		// check if read only
		virtual bool isReadOnly(void) const;
		
		// check for eof
		virtual bool isEof(void) const;
		
		// check for error
		virtual bool isError(void);

		// connect to remote host for reading
		virtual bool open(const char *url);
 		virtual bool open(bool https, const char *host, uint32_t port, const char *path);
 		virtual bool open(bool https, const __FlashStringHelper *host, uint32_t port, const __FlashStringHelper *path);
		
		// close the client
		virtual void close(void);
		
		// check if file is opened
		virtual bool isOpened(void) const;
		
		// set buffered mode
		// if bufSize is set to 0, buffer mode gets disabled
		virtual bool setBuffered(uint16_t bufSize);

		// peeks some bytes from input stream
		// copy at most reqSize bytes on dest buffer
		// returns size of available data (< reqSize) or 0 if can't peek
		virtual uint32_t peekBuffer(uint8_t *buf, uint32_t reqSize);

		// constructor
		FishinoHttpInStream();

		// destructor
		~FishinoHttpInStream();
};

#endif
