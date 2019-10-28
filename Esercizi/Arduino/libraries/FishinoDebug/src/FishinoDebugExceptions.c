//////////////////////////////////////////////////////////////////////////////////////
//						 	FishinoDebugExcpeptions.cpp								//
//																					//
//						Some debug helpers for Fishino32							//
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
//  Version 5.1.0 -- April 2017		Initial version									//
//  Version 5.1.1 -- May 2017		Removed \n on DEBUG_PRINT macro					//
//  Version 5.2.0 -- May 2017		Renamed in FishinoDebug.h						//
//									Uniformed 8 and 32 bit boards code				//
//  Version 6.0.0 -- June 2017		Added stack dump code for 32 bit boards			//
//  Version 6.0.4 -- June 2017		Added memory info functions						//
//  Version 7.1.0 - 20/10/2017		Better free memory display						//
//  Version 7.2.0 - 29/10/2017		Use FishinoPrintf library						//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#define DEBUG
#include "FishinoDebug.h"
#include <stdbool.h>

#ifdef _FISHINO_PIC32_

	#define EXCEPTION_NUM_EXCEPTIONS 14
	
	// declared static in case exception condition would prevent
	// auto variable being created
	enum
	{
		EXCEP_IRQ = 0,		// interrupt
		EXCEP_AdEL = 4,		// address error exception (load or ifetch)
		EXCEP_AdES,			// address error exception (store)
		EXCEP_IBE,			// bus error (ifetch)
		EXCEP_DBE,			// bus error (load/store)
		EXCEP_Sys,			// syscall
		EXCEP_Bp,			// breakpoint
		EXCEP_RI,			// reserved instruction
		EXCEP_CpU,			// coprocessor unusable
		EXCEP_Overflow,		// arithmetic overflow
		EXCEP_Trap,			// trap (possible divide by zero)
		EXCEP_IS1 = 16,		// implementation specfic 1
		EXCEP_CEU,			// CorExtend Unuseable
		EXCEP_C2E			// coprocessor 2
	} /*_excep_codes */;
	
	// print exception code in readable format
	static const char *__debug__exception__message(uint8_t exc)
	{
		switch(exc)
		{
			case EXCEP_IRQ :
				return "interrupt";
			case EXCEP_AdEL :
				return "address error exception (load or ifetch)";
				break;
			case EXCEP_AdES :
				return "address error exception (store)";
			case EXCEP_IBE :
				return "bus error (ifetch)";
			case EXCEP_DBE :
				return "bus error (load/store)";
			case EXCEP_Sys :
				return "syscall";
			case EXCEP_Bp :
				return "breakpoint";
			case EXCEP_RI :
				return "reserved instruction";
			case EXCEP_CpU :
				return "coprocessor unusable";
			case EXCEP_Overflow :
				return "arithmetic overflow";
			case EXCEP_Trap :
				return "trap (possible divide by zero)";
			case EXCEP_IS1 :
				return "implementation specfic 1";
			case EXCEP_CEU :
				return "CorExtend Unuseable";
			case EXCEP_C2E :
				return "coprocessor 2";
			default :
				return "UNKNOWN EXCEPTION";
		}
	}
	
	unsigned int _excep_code; // exception code corresponds to _excep_codes
	unsigned int _excep_addr; // exception address
	unsigned int _excep_stat; // status register
	uint32_t *_excep_stack;   // stack pointer on exception
	uint32_t *_excep_framePtr;// frame pointer on exception
	
	extern void *_stack;
	
	#define STACK_MAX_SCAN			10000
	#define STACK_MAX_FRAMES		256
	int __stack_num_frames;
	bool __stack_has_frames;
	void *__stack_frames[STACK_MAX_FRAMES];

	// code scan back to get sp and ra offsets
	// return false if couldn't find function start
	bool __debug_bt_scanback(void *sp, void *ra, int32_t *spOffset, int32_t *raOffset, bool *hasFramePtr)
	{
		if(hasFramePtr)
			*hasFramePtr = false;
		
		// locate function start
		uint32_t* wra = (uint32_t *)ra;
	
		// scan towards the beginning of the function addui sp,sp,spofft should be the first command
		uint32_t i = 0;
		while((*wra >> 16) != 0x27bd && i < STACK_MAX_SCAN)
		{
			// test for "scanned too much" elided
			wra--;
			i++;
		}
		// if not found just return
		if((*wra >> 16) != 0x27bd) //  addiu p,sp,spOffset
			return false;
		
		 // get stack pointer offset and sign-extend it
		*spOffset = ((int32_t)*wra << 16) >> 16;
		
		// to be sure, check if next assembly op is ra saving
		wra++;
		if((*wra >> 16) != 0xafbf) // sw ra,{spOffset+4}(sp)
			return false;
		
		// get saved ra offset in stack, it should be -spOffset - 4
		// (it's relative to shifted stack!)
		*raOffset = ((int32_t)*wra << 16) >> 16;
		if(*raOffset != -*spOffset - 4)
			return false;
		
		if(!hasFramePtr)
			return true;
		
		// and finally, check if we've got frames
		wra++;
		if((*wra >> 16) == 0xafbe) // sw s8,{spOffset+8}(sp)
		{
			wra++;
			if(*wra == 0x03a0f021) // move	s8,sp
				*hasFramePtr = true;
		}
		return true;
	}
	
	// non-framed walkback
	uint32_t __debug_bt_noframes(void *sp, void *ra)
	{
		int32_t spOffset, raOffset;
		__stack_num_frames = 0;
		__stack_frames[__stack_num_frames++] = ra;
	
		while(__debug_bt_scanback(sp, ra, &spOffset, &raOffset, NULL) && __stack_num_frames < STACK_MAX_FRAMES)
		{
			// walk back
	
			// with no frames, we must dig into code to find
			// stack offsets; less reliable but works even
			// in optimized modes
			
			// get next return address from stack
	/*
			DEBUG_INFO("Raoffset = %d\n", raOffset);
			DEBUG_INFO("sp       = %p\n", sp);
			DEBUG_INFO("*sp      = %x\n", *(uint32_t *)sp);
			DEBUG_INFO("*sp+1    = %x\n", *((uint32_t *)sp + 1));
	*/
			ra = *(void**)(void *)(((char*)sp) + raOffset);
	/*
			DEBUG_INFO("ra       = %p\n", ra);
	*/
		
			// get next sp
			sp = (char*)sp - spOffset;
			
			// store current return address
			__stack_frames[__stack_num_frames++] = ra;
			
		}
		return __stack_num_frames;
	}
	
	// framed walkback
	uint32_t __debug_bt_frames(void *sp, void *ra)
	{
		__stack_num_frames = 0;
		
		// get frame pointer from exception handler
		return 0;
	}
	
	// from exception address walks back trying to find function start
	// and check if we've frames or not
	uint32_t __debug_bt(void)
	{
		// get current stack pointer and exception address
		void *sp = _excep_stack;
		void *ra = (void *)_excep_addr;
		
		// this one is just to check if we've got frames or not
		int32_t spOffset, raOffset;
		bool hasFramePtr;
		__debug_bt_scanback(sp, ra, &spOffset, &raOffset, &hasFramePtr);
		
		hasFramePtr = false;
		
		if(hasFramePtr)
			return __debug_bt_frames(sp, ra);
		else
			return __debug_bt_noframes(sp, ra);
	}

	extern void __debug__runserial__(void);
	bool __in__exception__handler = false;
	void _general_exception_handler(unsigned cause, unsigned status, uint32_t *stack, uint32_t *framePtr)
	{
		// avoid re-entering
		// I don't know why, but the whole stuff doesn't work witout
		// even if there's no apparent reason for exception re-entering
		if(__in__exception__handler)
			while(1)
				;
		__in__exception__handler = true;
		
		// get exception data
		_excep_code = (cause & 0x0000007C) >> 2;
		_excep_stat = status;
		_excep_addr = __builtin_mfc0(_CP0_EPC, _CP0_EPC_SELECT);
		_excep_stack = stack;
		_excep_framePtr = framePtr;

		if ((cause & 0x80000000) != 0)
			_excep_addr += 4;
		
		// print an header
		debugPrintfP(F("\nException : '%s'\n"), __debug__exception__message(_excep_code));
		debugPrintfP(F("Address   : %08x\n"), _excep_addr);
		debugPrintfP(F("STACK PTR : %08x\n"), (unsigned int)stack);
		debugPrintfP(F("\n--- STACK DUMP ---\n"));
		
		// do a stack walkback
		__debug_bt();
		debugPrintfP(F("Number of frames : %0d\n"), (int)__stack_num_frames);
		int i;
		for(i = 0; i < __stack_num_frames; i++)
			debugPrintfP(F("  %03d : %p\n"), i, __stack_frames[i]);
		
		// closing message and halt
		debugPrintfP(F("\nCONTROLLER HALTED!"));
		
		// hang
		while(1)
			;
		
		__in__exception__handler = false;
	}
#endif
