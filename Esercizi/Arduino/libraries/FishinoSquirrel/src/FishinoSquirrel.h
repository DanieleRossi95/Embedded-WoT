//////////////////////////////////////////////////////////////////////////////////////
//						 		FishinoSquirrel.h									//
//																					//
//						Squirrel language for Fishino32 boards						//
//																					//
//		Copyright (c) 2017 Massimo Del Fedele.  All rights reserved.				//
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
#ifndef FISHINOSQUIRREL__H
#define FISHINOSQUIRREL__H

#include <Arduino.h>
#include <FishinoCoroutine.h>
#include <FishinoSdFat.h>

#include "include/squirrel.h"

class FishinoSquirrel : public FishinoCoroutine
{
	public:

		// machine states
		enum STATES {
			STATE_IDLE,		// not running code
			STATE_RUNNING,	// running code
			STATE_SUSPENDED,	// suspended (with sq_suspend())
			STATE_SLEEP,		// sleep (on Detach() call inside main loop)
			STATE_DESTROYED,	// machine is still not created or is destroyed
		};

	private:
	
		// the squirrel virtual machine
		SQVM *_squirrelVM;
		
		// terminate flag -- will terminate the coroutine
		bool _terminate;
		
		// start flag -- set by run command
		bool _doStart;
		
		// error state -- after run commands
		bool _runError;
		
		// machine state
		STATES _state;
		
		// flag set if we want output on terminal stream
		bool _terminalOutput;
		
		// number of VM loops before check the detach time
		// that's used to avoid too many calls inside main VM loop
		// which would slow down the vm
		uint32_t _detachLoops;
		
		// detach time, in milliseconds
		// that's the maximum time to wait before force VM pause
		// and return to coroutine caller
		uint32_t _detachTime;
		
		// current break time
		uint32_t _breakTime;

		// output stream
		// used for generic outputs
		Stream *_outputStream;
		
		// error stream
		// used for error output
		Stream *_errorStream;
		
		// terminal stream
		// use to display responses to terminal commands
		// and to load operations
		Stream *_terminalStream;
		
		// create the virtual machine
		bool createVM(void);
		
		// destroy the virtual machine
		bool destroyVM(void);
		
		// static print and error functions -- needed to glue them to squirrel machine
		static void __printFunc(HSQUIRRELVM vm, const char *fmt, ...) ;
		static void __errorFunc(HSQUIRRELVM vm, const char *fmt, ...) ;
		
		// static break hook function -- needed to glue to squirrel machine
		static void __breakHook(HSQUIRRELVM vm) ;
		
		// internal print and error functions -- entered by external ones
		void printFunc(const char *fmt, va_list args) ;
		void errorFunc(const char *fmt, va_list args) ;
		
		// internal break hook -- entered by external one
		void breakHook(void) ;
		
	protected:
	
		virtual void Routine(void) ;
	
	public:
	
		// constructor
		FishinoSquirrel() ;
		
		// destructor
		~FishinoSquirrel() ;
		
		// get squirrel machine state
		STATES getState(void) ;
		const char *getStateStr(void) ;
		
		// set machine detach time
		bool setDetachTime(uint32_t time, uint32_t loops = 200) ;
		
		// compile code on machine
		// (do NOT use coroutine mechanics here)
		bool compile(const char *buf, const char *bufDesc = NULL) ;
		bool compile(Stream &s, const char *streamDesc = NULL) ;
		bool compile(SdFile &f, const char *fileDesc = NULL) ;
		
		// run closure on machine TOS
		// (do use coroutine mechanics)
		bool run(void) ;
		
		// abort current run
		// (do use coroutine mechanics)
		bool abort(void) ;
		
		// restart the machine completely
		// (do use coroutine mechanics)
		bool restart(void) ;
		
		// set various streams
		void setOutputStream(Stream &s) ;
		void setErrorStream(Stream &s) ;
		void setTerminalStream(Stream &s) ;

		// clear various streams
		void clearOutputStream(Stream &s) ;
		void clearErrorStream(Stream &s) ;
		void clearTerminalStream(Stream &s) ;
};

#endif
