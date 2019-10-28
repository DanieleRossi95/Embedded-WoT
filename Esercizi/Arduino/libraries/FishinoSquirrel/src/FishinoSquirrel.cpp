//////////////////////////////////////////////////////////////////////////////////////
//						 		FishinoSquirrel.cpp									//
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
#include "FishinoSquirrel.h"

//#include "include/sqstdblob.h"
//#include "include/sqstdsystem.h"
//#include "include/sqstdio.h"
//#include "include/sqstdstring.h"

#include "libs/sqstdmath.h"
#include "libs/sqstdaux.h"

#include "libs/ThreadLib.h"
#include "libs/ArduinoLib.h"
#include "libs/StreamLib.h"
#include "libs/HardwareSerialLib.h"
#include "libs/HardwareSerialLib.h"
#include "libs/SdLib.h"
#include "libs/FishinoLib.h"
#include "libs/SPILib.h"
#include "libs/TestLib.h"

#define DEBUG_LEVEL_INFO
#include <FishinoDebug.h>

#include "squirrel/sqvm.h"
#include "squirrel/sqstring.h"

extern size_t __malloc__total;

HSQUIRRELVM GLOBALVM;

// constructor
FishinoSquirrel::FishinoSquirrel()
{
	_squirrelVM = NULL;
	_terminate = false;
	_doStart = false;
	_runError = false;
	_state = STATE_DESTROYED;

	_terminalOutput = false;
	_outputStream = NULL;
	_errorStream = NULL;
	_terminalStream = NULL;
	
	// 200 VM loops before checking for detach time expired
	_detachLoops = 200;

	// 1 second detach time default
	_detachTime = 1000;
}

// destructor
FishinoSquirrel::~FishinoSquirrel()
{
	if(_squirrelVM)
		destroyVM();
}

// fmt is used just to output "%s" in squirrel
// so we spare the printf() overhead
void FishinoSquirrel::__printFunc(HSQUIRRELVM v, const char *fmt, ...)
{
	FishinoSquirrel *THIS = (FishinoSquirrel *)sq_getsharedforeignptr(v);

	va_list args;
	va_start(args, fmt);
	THIS->printFunc(fmt, args);
	va_end(args);
}

void FishinoSquirrel::__errorFunc(HSQUIRRELVM v, const char *fmt, ...)
{
	FishinoSquirrel *THIS = (FishinoSquirrel *)sq_getsharedforeignptr(v);

	va_list args;
	va_start(args, fmt);
	THIS->errorFunc(fmt, args);
	va_end(args);
}

// internal print and error functions -- entered by external ones
void FishinoSquirrel::printFunc(const char *fmt, va_list args)
{
	Stream *s = _terminalOutput ? _terminalStream : _outputStream;
	if(s)
	{
		streamVPrintf(*s, fmt, args);
		Detach();
	}
}

void FishinoSquirrel::errorFunc(const char *fmt, va_list args)
{
	Stream *s = _terminalOutput ? _terminalStream : _errorStream;
	if(s)
	{
		streamVPrintf(*s, fmt, args);
		Detach();
	}
}

// static break hook function -- needed to glue to squirrel machine
void FishinoSquirrel::__breakHook(HSQUIRRELVM v)
{
	FishinoSquirrel *THIS = (FishinoSquirrel *)sq_getsharedforeignptr(v);
	THIS->breakHook();
}

// internal break hook -- entered by external one
void FishinoSquirrel::breakHook(void)
{
	// if timer is not expired, just return
	if(millis() < _breakTime)
		return;
	
	// set state to sleep mode
	_state = STATE_SLEEP;
	
	// return to caller
	Detach();

	// re-set state to running mode
	_state = STATE_RUNNING;
	
	// reset timer on re-enter
	_breakTime = millis() + _detachTime;

}

// create the virtual machine
bool FishinoSquirrel::createVM(void)
{
	if(_squirrelVM)
		destroyVM();

/*
DEBUG_PRINT("Size(SQObject)    = %u\n", sizeof(SQObject));
DEBUG_PRINT("Size(SQObjectPtr) = %u\n", sizeof(SQObjectPtr));
DEBUG_PRINT("Size(SQString)    = %u\n", sizeof(SQString));
*/

#if defined(SQUIRREL_MEMORY_TEST) || defined(SQUIRREL_MEMORY_SHOW_ALLOC)
	DEBUG_PRINT("CREATING VM\n");
	size_t prevAlloc = __malloc__total;
#endif

	// initialize squirrel engine
	_squirrelVM = sq_open(512);
	GLOBALVM = _squirrelVM;
	
	sq_setsharedforeignptr(_squirrelVM, this);
	sq_setprintfunc(_squirrelVM, __printFunc, __errorFunc);
	
	//aux library
	//sets error handlers
	sqstd_seterrorhandlers(_squirrelVM);

//	sqstd_register_bloblib(_squirrelVM);
//	sqstd_register_iolib(_squirrelVM);
//	sqstd_register_systemlib(_squirrelVM);
//	sqstd_register_stringlib(_squirrelVM);
	
//	registerArduinoStreamLib(_squirrelVM);
//	registerFileLib(_squirrelVM);

	registerArduinoLib(_squirrelVM);
	registerStreamLib(_squirrelVM);
	registerSerialLib(_squirrelVM);
	registerSdLib(_squirrelVM);
	registerFishinoLib(_squirrelVM);
	registerSPILib(_squirrelVM);
	registerMathLib(_squirrelVM);
	registerTestLib(_squirrelVM);
	registerThreadLib(_squirrelVM);
	
	// set break hook to be able to pause from long runs
	sq_setbreakhook(_squirrelVM, __breakHook, _detachLoops);
	_breakTime = millis() + _detachTime;
	
	// reset machine's stack pointer
	// don't know why, but top is at 2 upon startup...
	sq_settop(_squirrelVM, 0);

#if defined(SQUIRREL_MEMORY_TEST) || defined(SQUIRREL_MEMORY_SHOW_ALLOC)
	DEBUG_PRINT("DONE CREATING VM, ALLOCATED %u bytes\n", __malloc__total - prevAlloc);
#endif
	return true;
}

// destroy the virtual machine
bool FishinoSquirrel::destroyVM(void)
{
#if defined(SQUIRREL_MEMORY_TEST) || defined(SQUIRREL_MEMORY_SHOW_ALLOC)
	DEBUG_PRINT("DESTROYING VM\n");
	size_t prevAlloc = __malloc__total;
#endif

	if(_squirrelVM)
		sq_close(_squirrelVM);
	_squirrelVM = NULL;

#if defined(SQUIRREL_MEMORY_TEST) || defined(SQUIRREL_MEMORY_SHOW_ALLOC)
	DEBUG_PRINT("DONE DESTROYING VM, FREED %u bytes\n", prevAlloc - __malloc__total);
#endif
	return true;
}
		
void FishinoSquirrel::Routine(void)
{
	// keep working till not terminated
	while(!_terminate)
	{
		// if asked to run closure on TOS, do it
		if(_doStart)
		{
			_doStart = false;
			
			// store TOS, in order to clean it after run
			int oldTop = sq_gettop(_squirrelVM);
			
			sq_pushroottable(_squirrelVM);
			
			// run the machine -- it can be paused by timed YIELD or just terminate
			_state = STATE_RUNNING;
			_runError = !SQ_SUCCEEDED(sq_call(_squirrelVM, 1, SQFalse, SQTrue));

			// restore stack pointer
			sq_settop(_squirrelVM, oldTop);

			// remove closure from stack
			sq_poptop(_squirrelVM);
			
			_state = STATE_IDLE;
		}

		// otherwise keep running the machine
		Detach();
	}
}

// get squirrel machine state
FishinoSquirrel::STATES FishinoSquirrel::getState(void)
{
	if(!_squirrelVM)
		return STATE_DESTROYED;
	else
		return _state;
}

const char *FishinoSquirrel::getStateStr(void)
{
	if(!_squirrelVM || _state == STATE_DESTROYED)
		return "STATE_DESTROYED";
	else if(_state == STATE_IDLE)
		return "STATE_IDLE";
	else if(_state == STATE_RUNNING)
		return "STATE_RUNNING";
	else if(_state == STATE_SUSPENDED)
		return "STATE_SUSPENDED";
	else if(_state == STATE_SLEEP)
		return "STATE_SLEEP";
	else
		return "UNKNOWN STATE";
}

// set machine detach time
bool FishinoSquirrel::setDetachTime(uint32_t time, uint32_t loops)
{
	// machine must NOT be running to change state
	if(_state == STATE_RUNNING)
	{
		DEBUG_ERROR("Can't change detach time on a running machine\n");
		return false;
	}

	_detachTime = time;
	_detachLoops = loops;
	if(_squirrelVM)
		sq_setbreakhook(_squirrelVM, __breakHook, loops);

	_breakTime = millis() + _detachTime;
	return true;
}

// compile code on machine
// (do NOT use coroutine mechanics here)
bool FishinoSquirrel::compile(const char *buf, const char *bufDesc)
{
	// create machine if still not done
	if(!_squirrelVM && !createVM())
		return false;
	
	// load code from buffer
	if(!buf)
		return false;
	size_t len = strlen(buf);
	if(!len)
		return false;
	
	bool res = SQ_SUCCEEDED(sq_compilebuffer(_squirrelVM, buf, len, bufDesc ? bufDesc : _SC("interactive console"), SQTrue));

	return res;
}

static SQInteger _streamReadFunc(SQUserPointer userdata)
{
	Stream &s = *(Stream *)userdata;
	if(!s.available())
		return 0;
	return s.read();
}

bool FishinoSquirrel::compile(Stream &s, const char *streamDesc)
{
	// create machine if still not done
	if(!_squirrelVM && !createVM())
		return false;

	// compile the file
	bool res = SQ_SUCCEEDED(sq_compile(_squirrelVM, _streamReadFunc, &s, streamDesc ? streamDesc : "", SQTrue));

	return res;
}

static SQInteger _fileReadFunc(SQUserPointer userdata)
{
	SdFile &f = *(SdFile *)userdata;
	int i = f.read();
	if(i < 0)
		return 0;
	else
		return i;
}

bool FishinoSquirrel::compile(SdFile &f, const char *fileDesc)
{
	// create machine if still not done
	if(!_squirrelVM && !createVM())
		return false;

	// compile the file
	return SQ_SUCCEEDED(sq_compile(_squirrelVM, _fileReadFunc, &f, fileDesc ? fileDesc : "", SQTrue));
}


// run closure on machine TOS
// (do use coroutine mechanics)
bool FishinoSquirrel::run(void)
{
	if(!_squirrelVM)
		return false;
	
	_doStart = true;
	Resume();
	return _runError;
}

// abort current run
// (do use coroutine mechanics)
bool FishinoSquirrel::abort(void)
{
	if(!_squirrelVM || _state == STATE_IDLE)
		return true;
	
	if(_state != STATE_SLEEP)
	{
		DEBUG_ERROR("Machine must be in STATE_SLEEP, STATE_IDLE or STATE_DESTROYED state to abort\n");
		return false;
	}
	
	// set the shutdown flag
	sq_setshutdown(_squirrelVM);
	
	// restart the machine, it will stop on shutdown completed
	Resume();
	
	// reset shutdown flag
	sq_clearshutdown(_squirrelVM);

	return true;
}

// restart the machine completely
// (do use coroutine mechanics)
bool FishinoSquirrel::restart(void)
{
	// shut down machine if needed
	if(!abort())
		return false;
	
	// if state is STATE_DESTROYED, nothing to do
	if(_state == STATE_DESTROYED)
		return true;
	
	// state must be STATE_IDLE to shutdown
	if(_state != STATE_IDLE)
	{
		DEBUG_ERROR("state must be STATE_SLEEP, STATE_IDLE or STATE_DESTROYED to restart\n");
		return false;
	}
	
	destroyVM();
	return true;
}

// set various streams
void FishinoSquirrel::setOutputStream(Stream &s)
{
	_outputStream = &s;
}

void FishinoSquirrel::setErrorStream(Stream &s)
{
	_errorStream = &s;
}

void FishinoSquirrel::setTerminalStream(Stream &s)
{
	_terminalStream = &s;
}

// clear various streams
void FishinoSquirrel::clearOutputStream(Stream &s)
{
	_outputStream = NULL;
}

void FishinoSquirrel::clearErrorStream(Stream &s)
{
	_errorStream = NULL;
}

void FishinoSquirrel::clearTerminalStream(Stream &s)
{
	_terminalStream = NULL;
}

void printAlloc(size_t prev, size_t next)
{
	if(prev && next)
		DEBUG_PRINT("Realloc %8u to %8u, diff %8d, total %8u\n", prev, next, (int)next - (int)prev, __malloc__total);
	else if(next)
		DEBUG_PRINT("Alloc   %8u bytes,                      total %8u\n", next, __malloc__total);
	else
		DEBUG_PRINT("Free    %8u bytes,                      total %8u\n", prev, __malloc__total);
}
