//							FishinoSquirrelServer.cpp								//
//			Squirrel language server and web IDE for Fishino boards					//
//					Created by Massimo Del Fedele, 2017								//
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
//	VERSION 7.1.0 - 2017/10/20 - INITIAL VERSION									//
//  Version 7.3.0 - 2017/12/12 - Fixed library category								//
//  Version 7.3.2 - 2017/01/05 - Small fixes on includes and web files				//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#include "FishinoSquirrelServer.h"

#include "utils.h"
#include "Session.h"

#define DEBUG_LEVEL_ERROR
#include <FishinoDebug.h>

// handle file requests
bool FishinoSquirrelServerClass::__fileHandler(FishinoWebServer &)
{
	return FishinoSquirrelServerClass::__getFishinoSquirrelServer().fileHandler();
}

// handle index.htm requests
bool FishinoSquirrelServerClass::__indexHandler(FishinoWebServer &)
{
	return __getFishinoSquirrelServer().indexHandler();
}

// redirects to index
bool FishinoSquirrelServerClass::__redirectHandler(FishinoWebServer &server)
{
	return server.sendRedirect(SQUIRREL_ROOT "/");
}

// handle file uploads
bool FishinoSquirrelServerClass::__fileUploaderHandler(FishinoWebServer &, uint8_t action, char* buffer, int size)
{
	return __getFishinoSquirrelServer().fileUploaderHandler(action, buffer, size);
}

// handle sqfish.php requests
bool FishinoSquirrelServerClass::__sqfishHandler(FishinoWebServer &)
{
	return __getFishinoSquirrelServer().sqfishHandler();
}

// constructor
FishinoSquirrelServerClass::FishinoSquirrelServerClass() : _server(SQUIRREL_PORT)
{
	// clear post data
	_postCount = 0;
	_postNames = NULL;
	_postValues = NULL;
	_postBuf = NULL;

	// setup accepted headers and handlers
	_server.addHeader(F("Content-Length"));
	_server
		.addHandler(F(SQUIRREL_ROOT "/")			, FishinoWebServer::GET	, &__indexHandler)
		.addHandler(F(SQUIRREL_ROOT)				, FishinoWebServer::GET	, &__redirectHandler)
		.addHandler(F("/upload/" "*")				, FishinoWebServer::PUT	, &FishinoWebServer::putHandler)
		.addHandler(F(SQUIRREL_ROOT "/sqfish.php")	, FishinoWebServer::POST, &__sqfishHandler)
//		.addHandler(F("/status")					, FishinoWebServer::GET	, &statusHandler)
		.addHandler(F("/" "*")						, FishinoWebServer::GET	, &__fileHandler)
	;
	
	_server.setPutHandler(__fileUploaderHandler);
	
	_uploadFile = NULL;
}

// destructor
FishinoSquirrelServerClass::~FishinoSquirrelServerClass()
{
}

// clear the post data
void FishinoSquirrelServerClass::clearPostData(void)
{
	if(!_postCount)
		return;
	free(_postNames);
	free(_postValues);
	free(_postBuf);
	
	_postNames = NULL;
	_postValues = NULL;
	_postBuf = NULL;
	
	_postCount = 0;
}

static int  parseHexChar(char c)
{
	if (isdigit(c))
		return c - '0';

	c = tolower(c);
	if (c >= 'a' &&  c <= 'f')
		return c - 'a' + 10;

	return 0;
}

// parse post data from client in form of url-encoded data
// PATH=PROJECTS%2F&COMMAND=LIST&FOLDERS=true
// builds 2 arrays, one holding names and the other the values
bool FishinoSquirrelServerClass::parsePostData(void)
{
	clearPostData();
	
	// get post size and read the whole post data into buffer
	// we don't try to optimize here... too complicated by now
	// just drop stuffs if data is too large. It shouldn't happen
	uint32_t postSize = atoi(_server.getHeaderValue("Content-Length"));
	if(!postSize)
		return true;
	_postBuf = (char *)malloc(postSize + 1);
	if(!_postBuf)
	{
		DEBUG_ERROR("Not enough memory to allocate %" PRIu32 " bytes for post buffer\n", postSize);
		return false;
	}

	// get the client
	FishinoClient &client = _server.getClient();
	
	// read post data
	uint32_t remSize = postSize;
	char *bufP = _postBuf;
	uint32_t tim = millis() + 1000;
	do
	{
		uint32_t size = client.read((uint8_t *)bufP, remSize);
		remSize -= size;
		bufP += size;
		if(size)
			tim = millis() + 1000;
	}
	while(remSize && millis() < tim);
	if(remSize)
	{
		DEBUG_ERROR("Timeout reading post data - bailing out\n");
		free(_postBuf);
		_postBuf = NULL;
		return false;
	}
	_postBuf[postSize] = 0;
	
	// now count the values and allocate pointers
	bufP = _postBuf;
	_postCount = 0;
	while(*bufP)
	{
		if(*bufP++ == '&')
			_postCount++;
	}
	_postCount++;
	_postNames = (char **)malloc(sizeof(char **) * _postCount);
	_postValues = (char **)malloc(sizeof(char **) * _postCount);
	if(!_postNames || !_postValues)
	{
		DEBUG_ERROR("Out of memory allocating post names/values arrays\n");
		if(_postNames)
			free(_postNames);
		if(_postValues)
			free(_postValues);
		free(_postBuf);
		_postNames = _postValues = NULL;
		_postBuf = NULL;
		_postCount = 0;
		return false;
	}
	
	// ok, we got all allocated and data loaded, time to parse!
	bufP = _postBuf;
	char *bufP2 = _postBuf;
	uint16_t valIdx = 0;
	do
	{
		// parse name
		_postNames[valIdx] = bufP;
		while(*bufP2 && *bufP2 != '=')
		{
			if(*bufP2 == '%')
			{
				bufP2++;
				if(!*bufP2)
					break;
				char c = parseHexChar(*bufP2++);
				if(*bufP2)
					c = c << 4 | parseHexChar(*bufP2++);
				*bufP++ = c;
			}
			else if(*bufP2 == '+')
			{
				*bufP++ = ' ';
				bufP2++;
			}
			else
				*bufP++ = *bufP2++;
		}

		// parse value
		if(!*bufP2)
		{
			*bufP = 0;
			_postValues[valIdx] = bufP2;
			break;
		}
		*bufP = 0;
		bufP2++;
		bufP = bufP2;
		_postValues[valIdx] = bufP2;
		while(*bufP2 && *bufP2 != '&')
		{
			if(*bufP2 == '%')
			{
				bufP2++;
				if(!*bufP2)
					break;
				char c = parseHexChar(*bufP2++);
				if(*bufP2)
					c = c << 4 | parseHexChar(*bufP2++);
				*bufP++ = c;
			}
			else if(*bufP2 == '+')
			{
				*bufP++ = ' ';
				bufP2++;
			}
			else
				*bufP++ = *bufP2++;
		}
		valIdx++;
		if(!*bufP2)
		{
			*bufP = 0;
			break;
		}
		*bufP = 0;
		bufP2++;
		bufP = bufP2;
	}
	while(1);

	if(valIdx != _postCount)
	{
		DEBUG_ERROR("Counted values (%d) different from read ones (%d), bailing out\n", _postCount, valIdx);
		clearPostData();
		return false;
	}
	return true;
}

// get post value from name
const char *FishinoSquirrelServerClass::getPostValue(const char *name) const
{
	for(uint16_t i = 0; i < _postCount; i++)
		if(!strcmp(_postNames[i], name))
			return _postValues[i];
	return "";
}

// sends a file to client
void FishinoSquirrelServerClass::sendFile(const char* filename)
{
	SdFile file;
	
	if (!filename)
	{
		_server.sendErrorCode(404);
		_server << F("Could not parse URL");
	}
	else if (!file.open(filename, O_READ))
	{
		_server.sendErrorCode(404);
		_server << F("File '") << filename << F("' not found");
	}
	else
	{
		DEBUG_INFO("Sending file %s ", filename);

		FishinoWebServer::MimeType mimeType = _server.getMimeTypeFromFilename(filename);
		_server.sendErrorCode(200);
		_server.sendContentType(mimeType);
		
		// ask to cache image types
		if(
			mimeType == FishinoWebServer::MIMETYPE_GIF ||
			mimeType == FishinoWebServer::MIMETYPE_JPG ||
			mimeType == FishinoWebServer::MIMETYPE_PNG ||
			mimeType == FishinoWebServer::MIMETYPE_ICO
		)
			_server << F("Cache-Control:public, max-age=900\r\n");
		_server.endHeaders();

		_server.sendFile(file);
		file.close();
		DEBUG_INFO_N("DONE\n");
	}
}

// handle file requests
bool FishinoSquirrelServerClass::fileHandler(void)
{
	String filename = _server.getFileFromPath(_server.getPath());
	sendFile(filename.c_str());
	return true;
}

// handle index.htm requests
bool FishinoSquirrelServerClass::indexHandler(void)
{
Serial << "indexHandler\n";
	sendFile(SQUIRREL_ROOT SQUIRREL_INDEX);
	return true;
}

// build full path from PATH post value
String FishinoSquirrelServerClass::getFullPath(void)
{
	const char *path = getPostValue("PATH");
	if(!path || !*path)
	{
		DEBUG_ERROR("sqfish LOAD -- missing path\n");
		_server.sendChunk("{\"status\":\"missing path\"}\n");
		return "";
	}
	return path;
}

// handle sqfish.php requests
bool FishinoSquirrelServerClass::sqfishHandler(void)
{
	// parse post data
	bool err = false;
	if(!parsePostData())
	{
		DEBUG_ERROR("Error parsing post data\n");
		err = true;
	}

	const char *cmd = getPostValue("COMMAND");
	if(!cmd || !*cmd)
	{
		DEBUG_ERROR("sqfish handler -- missing command\n");
		err = true;
	}
	
	// send OK code and headers
	_server.sendErrorCode(200);
	_server.sendContentType(FishinoWebServer::MIMETYPE_TEXT);
	_server.sendTransferEncodingChunked();
	_server.endHeaders();
	
	if(err)
	{
		_server.sendChunk("{\"status\":\"failed\"}\n");
		_server.endChunkedTransfer();
		return true;
	}

	// load command ?
	if(!strcmp(cmd, "LOAD"))
	{
		DEBUG_INFO("LOAD\n");
		String path = getFullPath();
		if(path == "")
			return true;

		SdFile f;
		if(!f.open(path.c_str(), O_READ))
		{
			// file not found
			_server.sendChunk("{\"status\":\"file not found\"}\n");
			DEBUG_ERROR("sqfish : path '%s' not found\n", path.c_str());
		}
		else
		{
			_server.sendChunk("{\"status\":\"ok\"}\n");
			_server.sendChunkedFile(f);
			f.close();
			DEBUG_INFO("sqfish : file '%s' sent\n", path.c_str());
		}
		_server.endChunkedTransfer();
		return true;
	}
	// store command ?
	else if(!strcmp(cmd, "STORE"))
	{
		DEBUG_INFO("STORE\n");
		String path = getFullPath();
		if(path == "")
			return true;
		const char *data = getPostValue("CONTENT");
		if(!data || !*data)
		{
			_server.sendChunk("{\"status\":\"missing data\"}\n");
			_server.endChunkedTransfer();
			return true;
		}
		
		// create folder if missing
		createFileFolder(path);

		SdFile f;
		if(!f.open(path.c_str(), O_WRITE | O_CREAT | O_TRUNC))
		{
			// file not found
			_server.sendChunk("{\"status\":\"error creating file\"}\n");
			DEBUG_ERROR("sqfish : error creating file '%s'\n", path.c_str());
		}
		else
		{
			f.write(data, strlen(data));
			f.close();
			_server.sendChunk("{\"status\":\"ok\"}\n");
		}
		_server.endChunkedTransfer();
		return true;

	}
	// rmfile command ?
	else if(!strcmp(cmd, "RMFILE"))
	{
		DEBUG_INFO("RMFILE\n");
		String path = getFullPath();
		if(path == "")
			return true;
		SdFile f;
		if(!f.open(path.c_str(), O_WRITE))
		{
			_server.sendChunk("{\"status\":\"file not found\"}\n");
			DEBUG_ERROR("sqfish : file '%s' not found\n", path.c_str());
		}
		else
		{
			if(f.remove())
			{
				_server.sendChunk("{\"status\":\"ok\"}\n");
				DEBUG_INFO("sqfish : file '%s' removed\n", path.c_str());
			}
			else
			{
				_server.sendChunk("{\"status\":\"error removing file\"}\n");
				DEBUG_INFO("sqfish : error removing file '%s'\n", path.c_str());
			}
			f.close();
		}
		_server.endChunkedTransfer();
		return true;
	}
	// rmfolder command ?
	else if(!strcmp(cmd, "RMFOLDER"))
	{
		DEBUG_INFO("RMFOLDER\n");
		String path = getFullPath();
		if(path == "")
			return true;
		SdFile f;
		if(!f.open(path.c_str()))
		{
			_server.sendChunk("{\"status\":\"path not found\"}\n");
			DEBUG_ERROR("sqfish : path '%s' not found\n", path.c_str());
		}
		else
		{
			if(!f.isDir())
			{
				_server.sendChunk("{\"status\":\"not a directory\"}\n");
				DEBUG_INFO("sqfish : path '%s' is not a directory\n", path.c_str());
			}
			else if(f.rmRfStar())
			{
				_server.sendChunk("{\"status\":\"ok\"}\n");
				DEBUG_INFO("sqfish : path '%s' removed\n", path.c_str());
			}
			else
			{
				_server.sendChunk("{\"status\":\"error removing path\"}\n");
				DEBUG_INFO("sqfish : error removing path '%s'\n", path.c_str());
			}
			f.close();
		}
		_server.endChunkedTransfer();
		return true;

	}
	// list command ?
	else if(!strcmp(cmd, "LIST"))
	{
		DEBUG_INFO("LIST\n");
		String path = getFullPath();
		if(path == "")
			return true;
		bool folders = !strcmp(getPostValue("FOLDERS"), "true");
		SdFile dirFile;
		if(!dirFile.open(path.c_str()) || !dirFile.isDir())
		{
			_server.sendChunk("{\"status\":\"not a directory\"}\n");
			DEBUG_ERROR("sqfish : path '%s' is not a directory\n", path.c_str());
		}
		else
		{
			char buf[256];
			_server.sendChunk("{\"status\":\"ok\"}\n");
			_server.sendChunk("{\"elements\":[");
			SdFile f;
			bool cont = false;
			while(f.openNext(&dirFile))
			{
				if(!(folders ^ f.isDir()))
				{
					f.getName(buf, 256);
					_server.sendChunk("\"");
					_server.sendChunk(buf);
					_server.sendChunk("\"");
					cont = true;
					f.close();
					break;
				}
				f.close();
			}
			if(cont)
			{
				while(f.openNext(&dirFile))
				{
					if(!(folders ^ f.isDir()))
					{
						f.getName(buf, 256);
						_server.sendChunk(",\"");
						_server.sendChunk(buf);
						_server.sendChunk("\"");
					}
					f.close();
				}
			}
			_server.sendChunk("]}");
		}
		dirFile.close();
		_server.endChunkedTransfer();
		return true;
	}
	else if(!strcmp(cmd, "TERMSEND"))
	{
		DEBUG_INFO("TERMSEND\n");
		// get terminal command
		const char *content = getPostValue("CONTENT");
		if(content && *content)
		{
			DEBUG_INFO("Aborting current sketch\n");
			_sq.abort();
			DEBUG_INFO("Compiling '%s'\n", content);
			if(_sq.compile(content))
			{
				DEBUG_INFO("Running '%s'\n", content);
				_sq.run();
				DEBUG_INFO("Done running\n");
			}
			else
				DEBUG_ERROR("Compile error\n");
		}
		_server.sendChunk("{\"status\":\"ok\"}\n");
		_server.endChunkedTransfer();
		return true;
	}
	else if(!strcmp(cmd, "STATUS"))
	{
		DEBUG_INFO("STATUS\n");
		_server.sendChunk("{\"status\":\"ok\"}\n");
		_server.sendChunk("{");
		if(_terminalStream.getCount())
		{
			_server.sendChunk("\"term\":\"");
			String s =  _terminalStream.get();
			_server.sendChunk(escapeString(s.c_str()).c_str());
			_server.sendChunk("\",");
		}
		char ramBuf[25];
		sprintf(ramBuf, "\"ram\":\"%" PRIu32 "\"", __debug__freeram__());
		_server.sendChunk(ramBuf);
		_server.sendChunk("}");
		_server.endChunkedTransfer();
		return true;
	}
	else if(!strcmp(cmd, "RUN"))
	{
		DEBUG_INFO("RUN\n");
		// get project to run
		const char *project = getPostValue("PROJECT");
		if(!project || !*project)
		{
			_server.sendChunk("{\"status\":\"missing project\"}\n");
			DEBUG_ERROR("sqfish : missing project name\n");
			_server.endChunkedTransfer();
			return true;
		}
		
		// just check if project exists
		String fullPath = appendPath(PROJECTS_ROOT, project);
		SdFile f;
		if(!f.open(fullPath.c_str(), O_READ) || !f.isDir())
		{
			_server.sendChunk("{\"status\":\"project not found\"}\n");
			DEBUG_ERROR("sqfish : project '%s' not found\n", project);
			_server.endChunkedTransfer();
			return true;
		}
		f.close();
		_server.sendChunk("{\"status\":\"ok\"}\n");
		_server.endChunkedTransfer();
		runProject(project);
		return true;
	}
	else
	{
		DEBUG_ERROR("sqfish handler -- unknown command '%s'\n", cmd);
		_server.sendChunk("{\"status\":\"unknown command\"}\n");
	}
	return true;
}

// handle file uploads
bool FishinoSquirrelServerClass::fileUploaderHandler(uint8_t action, char* buffer, int size)
{
	static uint32_t startTime;
	static uint32_t totalSize;
	
	switch (action)
	{
		case FishinoWebServer::START:
		{
			// get file path
			String filePath = _server.getFileFromPath(_server.getPath());
			
			// strip "/upload" from path
			filePath = filePath.substring(7);
			DEBUG_INFO("Uploading file %s\n", filePath.c_str());
			
			// create containing folder tree if needed
			createFileFolder(filePath);
			
			startTime = millis();
			totalSize = 0;

			// create the file
			if(_uploadFile)
			{
				_uploadFile->close();
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
				delete _uploadFile;
#pragma GCC diagnostic pop
			}
			_uploadFile = new SdFile;
			_uploadFile->open(filePath.c_str(), O_CREAT | O_WRITE | O_TRUNC);
			break;
		}

		case FishinoWebServer::WRITE:
			if (_uploadFile && _uploadFile->isOpen())
			{
				_uploadFile->write(buffer, size);
				totalSize += size;
			}
			break;

		case FishinoWebServer::END:
			if(_uploadFile)
			{
				_uploadFile->sync();
				DEBUG_INFO("Wrote %" PRIu32 " bytes in %lu millis (received %" PRIu32 " bytes\n", _uploadFile->fileSize(), millis() - startTime, totalSize);
				_uploadFile->close();
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
				delete _uploadFile;
#pragma GCC diagnostic pop
				_uploadFile = NULL;
			}
	}
	
	return true;
}

// output function
void FishinoSquirrelServerClass::puts(const char *s)
{
	DEBUG_FREE_MEMORY();
	
	_terminalStream.add(s);
}

void FishinoSquirrelServerClass::begin(void)
{
	// initialize machine
	_server.begin();
	
	// setup streams on squirrel machine
/*
	_sq.setOutputStream(Serial);
	_sq.setErrorStream(Serial);
*/
	_sq.setOutputStream(_terminalStream);
	_sq.setErrorStream(_terminalStream);

	// start VM coroutine
	Sequencing(_sq.Resume());
	DEBUG_INFO("BEGIN() DONE\n");
}

void FishinoSquirrelServerClass::end()
{
}

// get the last session's project name
bool FishinoSquirrelServerClass::runLastProject(void)
{
	Session session;
	if(!session)
	{
		DEBUG_ERROR("Failed reading session file\n");
		return false;
	}
	DEBUG_INFO("Running project '%s'\n", session.getActiveProject().c_str());
	bool res = runProject(session.getActiveProject().c_str());
	if(!res)
		DEBUG_ERROR("Error running project '%s'\n", session.getActiveProject().c_str());
	return res;
}
		

// load execution manager and run it
bool FishinoSquirrelServerClass::runApplication(void)
{
	DEBUG_ERROR("NOT IMPLEMENTED\n");
	return true;
}

// run a project
bool FishinoSquirrelServerClass::runProject(const char *name)
{
	// if machine is still running, reset it
	_sq.restart();
	
/*
	Project project(name);
	project.Dump();
	if(!project)
	{
		DEBUG_ERROR("Error loading project\n");
		return false;
	}
*/

	// by now, just run main project file
	String root = appendPath(PROJECTS_ROOT, name);
	
	String filePath = appendPathExt(root, name, ".nut");
	
	SdFile f;
	if(!f.open(filePath.c_str(), O_READ))
	{
		DEBUG_ERROR("Error opening project %s\n", filePath.c_str());
		return false;
	}
	
	if(!_sq.compile(f))
	{
		DEBUG_ERROR("Compiler error\n");
		return false;
	}
	_sq.run();
	
	return true;
}

void FishinoSquirrelServerClass::loop(void)
{
	// process server requests
	_server.process();
	
	// run squirrel vm
	if(_terminalStream.getCount() < 2000)
		_sq.Resume();
	else
		DEBUG_ERROR("QUEUE FULL\n");
}

FishinoSquirrelServerClass &FishinoSquirrelServerClass::__getFishinoSquirrelServer()
{
	static FishinoSquirrelServerClass sq;
	return sq;
}

