//////////////////////////////////////////////////////////////////////////////////////
//																					//
//							FishinoSquirrelServer.h									//
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
#ifndef __FISHINOSQUIRREL_SERVER_H
#define __FISHINOSQUIRREL_SERVER_H

#include <FishinoWebServer32.h>
#include <FishinoSquirrel.h>

#include "StringStream.h"

//////////////////////////////////////////////////////////////////////////////////////////
// some detaisl on filesystem on SD card
#define SQUIRREL_PORT		8080
#define SQUIRREL_ROOT		"/SQUIRREL"
#define PROJECTS_ROOT		SQUIRREL_ROOT "/PROJECTS"
#define LIBS_ROOT			SQUIRREL_ROOT "/LIBRARIES"
#define SQUIRREL_INDEX		"/index.html"
//////////////////////////////////////////////////////////////////////////////////////////

#define SQSERVER FishinoSquirrelServerClass::__getFishinoSquirrelServer()

class FishinoSquirrelServerClass
{
	public:
		static FishinoSquirrelServerClass &__getFishinoSquirrelServer() ;
	private:

		// static functions and callbacks
		static bool __fileHandler(FishinoWebServer &) ;
		static bool __indexHandler(FishinoWebServer &) ;
		static bool __sqfishHandler(FishinoWebServer &) ;
		static bool __fileUploaderHandler(FishinoWebServer &, uint8_t action, char* buffer, int size) ;
		static bool __redirectHandler(FishinoWebServer &server) ;
	
		// the web server object
		FishinoWebServer _server;
		
		// the squirrel object
		FishinoSquirrel _sq;

		// constructor -- don't allow construction
		FishinoSquirrelServerClass() ;
		
		// post data parsed by parsePostData function
		uint16_t _postCount;
		char *_postBuf;
		char **_postNames;
		char **_postValues;
		
		// terminal messages queue
		StringStream _terminalStream;
		
		// file object - used for uploads
		SdFile *_uploadFile;

	protected:
	
		// clear the post data
		void clearPostData(void) ;
	
		// parse post data from client in form of url-encoded data
		// PATH=PROJECTS%2F&COMMAND=LIST&FOLDERS=true
		// builds 2 arrays, one holding names and the other the values
		bool parsePostData(void) ;
		
		// get post value from name
		const char *getPostValue(const char *name) const ;
		
		// build full path from PATH post value
		String getFullPath(void) ;

		// sends a file to client
		void sendFile(const char* filename) ;
		
		// handle file requests
		bool fileHandler(void) ;

		// handle index.htm requests
		bool indexHandler(void) ;

		// handle sqfish.php requests
		bool sqfishHandler(void) ;

		// handle file uploads
		bool fileUploaderHandler(uint8_t action, char* buffer, int size) ;
		
		// load execution manager and run it
		bool runApplication(void) ;

		// run a project
		bool runProject(const char *name) ;

	public:

		// destructor
		~FishinoSquirrelServerClass() ;
		
		void begin(void) ;
		void end() ;
		
		// run the last session's project name
		bool runLastProject(void);
		
		// main loop 
		void loop(void) ;
		
		// output function
		void puts(const char *s) ;
};

#endif
