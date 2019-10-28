//
// SqratThread: Sqrat threading module
//

//
// Copyright (c) 2009 Brandon Jones
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must not
//  claim that you wrote the original software. If you use this software
//  in a product, an acknowledgment in the product documentation would be
//  appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and must not be
//  misrepresented as being the original software.
//
//  3. This notice may not be removed or altered from any source
//  distribution.
//
#include <FishinoSquirrel.h>
#include "ThreadLib.h"
#include <time.h>
#include <string.h>

#define DEBUG_LEVEL_ERROR
#include <FishinoDebug.h>

//
// Thread lib utility functions (not visible externally)
//

static SQInteger sqrat_strlen(const SQChar* str)
{
#if defined(_UNICODE)
	return static_cast<SQInteger>(wcslen(str) * sizeof(SQChar));
#else
	return static_cast<SQInteger>(strlen(str) * sizeof(SQChar));
#endif
}

static void sqrat_pushtaskarray(HSQUIRRELVM v)
{
	HSQOBJECT taskarray;

	sq_pushroottable(v);
	sq_pushstring(v, _SC("__sqrat_taskarray__"), -1, SQTrue);
	if (SQ_FAILED(sq_get(v, -2)))
	{
		// Not found, create a new namespace
		sq_pushstring(v, _SC("__sqrat_taskarray__"), -1, SQTrue);
		sq_newarray(v, 0);
		sq_getstackobj(v, -1, &taskarray); // Save namespace for later use
		sq_newslot(v, -3, 0);
		sq_pop(v, 1); // pop root table

		sq_pushobject(v, taskarray); // push the newly bound array
	}
	else
	{
		sq_remove(v, -2); // pop sqrat table
	}
}

static SQInteger sqrat_schedule_argcall(HSQUIRRELVM v)
{
	SQInteger nparams = sq_gettop(v) - 2; // Get the number of parameters provided

	// The task table is the last argument (free variable), so we can operate on immediately
	sq_pushstring(v, _SC("args"), -1, SQTrue);

	sq_newarray(v, 0); // Create the array for the arguments

	// Loop through all arguments and push them into the arg array
	for (SQInteger i = 0; i < nparams; ++i)
	{
		sq_push(v, i + 2);
		sq_arrayappend(v, -2);
	}

	sq_newslot(v, -3, 0); // Push the arg array into the task table
	return 0;
}

// This is a horrid way to get this functionality in there, but I can't find any alternatives right now.
static SQRESULT sqrat_pushsleep(HSQUIRRELVM v)
{
	const SQChar* sleep_script = _SC(
		"__sqratsleep__ <- function(timeout)"
		"{"
		"	local begin = millis();"
		"	local now;"
		"	do {"
		"		::suspend();"
		"		now = millis();"
		"	}"
		"	while( (now - begin) < timeout );"
		"}"
	);

	if (SQ_FAILED(sq_compilebuffer(v, sleep_script, sqrat_strlen(sleep_script), _SC(""), true)))
	{
		return SQ_ERROR;
	}

	sq_pushroottable(v);
	if (SQ_FAILED(sq_call(v, 1, 0, 1)))
	{
		sq_remove(v, -1); // remove compiled closure
		return SQ_ERROR;
	}
	sq_remove(v, -1); // remove compiled closure

	sq_pushroottable(v);
	sq_pushstring(v, _SC("__sqratsleep__"), -1, SQTrue);
	SQRESULT res = sq_get(v, -2);

	sq_remove(v, -2); // remove root table

	return res;
}

//
// Thread lib main functions
//
static void sqrat_schedule(HSQUIRRELVM v, SQInteger idx)
{
	HSQOBJECT thread;
	HSQOBJECT func;
	HSQOBJECT task;

	sq_getstackobj(v, idx, &func);
	SQInteger stksize = 256; // TODO: Allow initial stack size to be configurable

	sqrat_pushtaskarray(v); // Push the task array

	// Create the task
	sq_newtable(v);
	sq_getstackobj(v, -1, &task);

	// Create the thread and add it to the task table
	sq_pushstring(v, _SC("thread"), -1, SQTrue);
	sq_newthread(v, stksize);
	sq_getstackobj(v, -1, &thread);
	sq_newslot(v, -3, 0);

	// Push the function to be called onto the thread stack
	sq_pushobject(v, func);
	sq_move(thread._unVal.pThread, v, -1);
	sq_pop(v, 1);

	// Args will be pushed later, in the closure

	sq_arrayappend(v, -2); // Add the task to the task array

	sq_pushobject(v, task); // Push the task object as a free variable for the temporary closure
	sq_newclosure(v, sqrat_schedule_argcall, 1); // push a temporary closure used to retrieve call args
}

// Wow... this has to be one of the ugliest functions I've ever writter. Ever.
// Building complex logic with the squirrel stack really sucks.
static void sqrat_run(HSQUIRRELVM v)
{

	HSQOBJECT taskArray;
	HSQOBJECT thread;
	HSQUIRRELVM threadVm;
	SQInteger nparams;  // Number of parameters to pass to a function
	SQInteger arrayidx; //Cached index of the task array

	// Push the tasklist
	sqrat_pushtaskarray(v); // Push the task array to the stack

	sq_getstackobj(v, -1, &taskArray);
	arrayidx = sq_gettop(v); // Cache the stack location of the task array

	SQInteger tasklistSize = sq_getsize(v, arrayidx); // Query the initial size of the task array
	do
	{
		// check shutdown flag to avoid infinite loop
		// on machine shutdown
		if(sq_getshutdown(v))
		{
			// remove all tasks before shutting down
			// so if we run() again the machine won't hang
			sq_arrayresize(v, arrayidx, 0);
			break;
		}
		
		SQInteger i = 0;

		// This outer while is to allow us to pick up any new tasks that are added during the loop,
		// but still give us an opportunity to sleep after running through the initial tasks
		while (i < tasklistSize)
		{

			for (; i < tasklistSize; ++i)
			{
				sq_pushinteger(v, i);
				if (SQ_FAILED(sq_get(v, -2)))
				{
					// Get the task
					sq_arrayremove(v, -2, i);
					sq_pop(v, 1);
					--tasklistSize;
					--i;
					continue;
				}

				// Now that we have the task, get the thread
				sq_pushstring(v, _SC("thread"), -1, SQTrue);
				if (SQ_FAILED(sq_get(v, -2)))
				{
					sq_arrayremove(v, -3, i);
					sq_pop(v, 1);
					--tasklistSize;
					--i;
					continue;
				}
				sq_getstackobj(v, -1, &thread);
				sq_pop(v, 1);

				threadVm = thread._unVal.pThread;

				if (sq_getvmstate(threadVm) == SQ_VMSTATE_IDLE)
				{
					// New thread? If so we need to call it

					// Function to be called is already pushed to the thread (happens in schedule)
					sq_pushroottable(threadVm); // Pus the threads root table

					sq_pushstring(v, _SC("args"), -1, SQTrue);
					if (SQ_FAILED(sq_get(v, -2)))
					{
						// Check to see if we have arguments for this thread
						nparams = 0; // No arguments
					}
					else
					{
						nparams = sq_getsize(v, -1); // Get the number of args in the arg array

						// Push the arguments onto the thread stack
						for (SQInteger a = 0; a < nparams; ++a)
						{
							sq_pushinteger(v, a);
							if (SQ_FAILED(sq_get(v, -2)))
							{
								sq_pushnull(threadVm); // Is this the best way to handle this?
							}
							else
							{
								sq_move(threadVm, v, -1);
								sq_pop(v, 1);
							}
						}

						sq_pop(v, 1); // Pop the arg array
					}

					sq_call(threadVm, nparams + 1, 0, 1); // Call the thread

				}
				else
				{
					// If the thread is suspended, wake it up.
					// This function changed in Squirrel 2.2.3,
					// removing the last parameter makes it compatible with 2.2.2 and earlier
					sq_wakeupvm(threadVm, 0, 0, 1, 0);
				}

				if (sq_getvmstate(threadVm) == SQ_VMSTATE_IDLE)
				{
					// Check to see if the thread is finished (idle again)
					sq_arrayremove(v, -2, i); // Remove the task from the task array

					--tasklistSize; // Adjust the for variables to account for the removal
					--i;
				}

				sq_pop(v, 1); // Pop off the task
			}

			// Yield to system if needed

			tasklistSize = sq_getsize(v, arrayidx); // Get the task

		}

	}
	while (tasklistSize > 0); // Loop until we have no more pending tasks
}

//
// Script interface functions
//

static SQInteger sqratbase_schedule(HSQUIRRELVM v)
{
	sqrat_schedule(v, -1);
	return 1;
}

static SQInteger sqratbase_run(HSQUIRRELVM v)
{
	sqrat_run(v);
	return 0;
}

// This is a squirrel only function, since there's really no need to
// expose a native api for it. Just use the VM that you would have passed
// in anyway!
static SQInteger sqratbase_getthread(HSQUIRRELVM v)
{
	// For the record, this way of doing things really sucks.
	// I would love a better way of retrieving this object!
	HSQOBJECT threadObj;
	threadObj._type = OT_THREAD;
	threadObj._unVal.pThread = v;

	sq_pushobject(v, threadObj);
	sq_weakref(v, -1);
	sq_remove(v, -2);
	return 1;
}

//
// Module registration
//

SQInteger registerThreadLib(HSQUIRRELVM v)
{
	// push the name of the Threads table
	// and create it
	sq_pushstring(v, _SC("Threads"), -1, SQTrue);
	sq_newtable(v);
	
	sq_pushstring(v, _SC("schedule"), -1, SQTrue);
	sq_newclosure(v, &sqratbase_schedule, 0);
	sq_newslot(v, -3, 0);

	sq_pushstring(v, _SC("run"), -1, SQTrue);
	sq_newclosure(v, &sqratbase_run, 0);
	sq_newslot(v, -3, 0);

	sq_pushstring(v, _SC("getthread"), -1, SQTrue);
	sq_newclosure(v, &sqratbase_getthread, 0);
	sq_newslot(v, -3, 0);

	// Than this...
	sq_pushstring(v, _SC("sleep"), -1, SQTrue);
	if (SQ_FAILED(sqrat_pushsleep(v)))
		sq_pop(v, 1);
	else
		sq_newslot(v, -3, 0);
	
	// set the slot in roottable
	sq_newslot(v, -3, 0);

	return SQ_OK;
}
