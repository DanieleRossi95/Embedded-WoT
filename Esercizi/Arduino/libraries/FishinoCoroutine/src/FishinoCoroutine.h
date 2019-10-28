//////////////////////////////////////////////////////////////////////////////////////
//																					//
//							FishinoCoroutine.h										//
//			Coroutine (Cooperative threads) routines for Fishino boards				//
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
//	VERSION 7.1.0 - 20/10/2017 - INITIAL VERSION									//
//  Version 7.3.0 - 12/12/2017 - Fixed library category								//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#ifndef __FISHINO_COROUTINE_H
#define __FISHINO_COROUTINE_H

#define DEFAULT_STACK_SIZE 15000
#define Sequencing(S) { FishinoCoroutine::InitSequencing(DEFAULT_STACK_SIZE); S; }
#include <stddef.h>
#include <setjmp.h>

class Task;

class FishinoCoroutine
{
	protected:
		FishinoCoroutine(size_t StackSize = DEFAULT_STACK_SIZE);
		virtual void Routine() = 0;
	private:
		void Enter();
		void Eat();
		Task *MyTask;
		size_t StackSize;
		int Ready, Terminated;
		FishinoCoroutine *Caller, *Callee;
		static FishinoCoroutine *ToBeResumed;
		
		static void Resume(FishinoCoroutine *);
		static void Call(FishinoCoroutine *);

		static FishinoCoroutine *CurrentCoroutine();
		static FishinoCoroutine *MainCoroutine();

	public:
	
		// detach current coroutine
		static void Detach();

		// start / resume me
		void Resume() { Resume(this); }
		
		// start me
		void Call() { Call(this); }

		static void InitSequencing(size_t main_StackSize);
};


struct Task
{
	FishinoCoroutine *MyCoroutine;
	jmp_buf jmpb;
	int used;
	size_t size;
	struct Task *pred, *suc;
	struct Task *prev, *next;
};

#endif

