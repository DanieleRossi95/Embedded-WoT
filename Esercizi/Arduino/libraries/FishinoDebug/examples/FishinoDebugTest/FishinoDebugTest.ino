//////////////////////////////////////////////////////////////////////////////////////
//						 	FishinoDebugTest.ino									//
//																					//
//				Test print features of FishinoDebug library							//
//			For better results, compile with optimizations disabled					//
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
//  Version 6.0.0 -- June 2017		Initial version									//
//  Version 7.2.0 - 29/10/2017		Use FishinoPrintf library						//
//																					//
//////////////////////////////////////////////////////////////////////////////////////


// to test you can comment some of following #defines
// INFO level		: it is the broadest one, it print INFO, WARNING, ERROR and FATAL messages
// WARNING level	: print WARNING, ERROR and FATAL messages
// ERROR level		: print ERROR and FATAL messages
// FATAL level		: print FATAL messages
// Prints are done by following macros : DEBUG_INFO(), DEBUG_WARNING(), DEBUG_ERROR(), DEBUG_FATAL()
// DEBUG_PRINT prints always, disregarding DEBUG_LEVEL macros, when DEBUG macro is defined
// parameters are the same as C printf() function
#define DEBUG_LEVEL_INFO
#define DEBUG_LEVEL_WARNING
#define DEBUG_LEVEL_ERROR
#define DEBUG_LEVEL_FATAL
#define DEBUG

#include <FishinoPrintf.h>
#include <FishinoDebug.h>

void setup()
{
	// set debug output stream
	// you can choose any available serial port
	DEBUG_SET_STREAM(Serial);
	
	// initialize serial port and wait for it to open
	Serial.begin(115200);
	while(!Serial)
		;
	Serial.println("INITIALIZED!");
	
	// do some print test
	DEBUG_PRINT("This text is printed always when DEBUG macro is defined\n");
	DEBUG_INFO("This text is printed when DEBUG_INFO macro is defined\n");
	DEBUG_WARNING("This text is printed when DEBUG_INFO or DEBUG_WARNING macro are defined\n");
	DEBUG_ERROR("This text is printed when DEBUG_INFO, DEBUG_WARNING or DEBUG_ERROR macro are defined\n");
	DEBUG_FATAL("This text is printed when DEBUG_INFO, DEBUG_WARNING, DEBUG_ERROR or DEBUG_FATAL macro are defined\n");
	DEBUG_PRINT("\n");
	
	// a print with some data
	DEBUG_PRINT("Here you see how to print a number : %d\n", 123);
	DEBUG_PRINT("\n");
	DEBUG_PRINT("And here you see a string : '%s'\n", "A TEST STRING");
	DEBUG_PRINT("\n");
	
	// the debug macros with _N does not print INFO, WARNING, ERROR and FATAL messages
	// and can be used for multiple prints:
	DEBUG_INFO("Some numbers :");
	for(int i = 0; i < 10; i++)
		DEBUG_INFO_N("%d ", i);
	DEBUG_PRINT("\n\n");
	
	// the macros with _FUNCTION prints the name of current function before text
	DEBUG_PRINT_FUNCTION("Here the message with function name!!!\n");
	DEBUG_PRINT("\n");
	
	// these show how to print intxxx_t and uintxxx_t data
	DEBUG_PRINT("Following lines show how to print uintXXX_t and intXXX_t numbers without warnings:\n");
	DEBUG_PRINT("\n");
	uint16_t x16 = 12345;
	DEBUG_PRINT("Printing an uint32_t        : %" PRIu16 "\n", x16);
	DEBUG_PRINT("Printing an uint32_t as hex : %" PRIx16 "\n", x16);
	DEBUG_PRINT("Same with capital letters   : %" PRIX16 "\n", x16);
	DEBUG_PRINT("\n");
	
	int16_t y16 = -12345;
	DEBUG_PRINT("Printing an int32_t         : %" PRIi16 "\n", y16);
	DEBUG_PRINT("Printing an  int32_t as hex : %" PRIx16 "\n", y16);
	DEBUG_PRINT("Same with capital letters   : %" PRIX16 "\n", y16);
	DEBUG_PRINT("\n");

	uint32_t x32 = 123456789;
	DEBUG_PRINT("Printing an uint32_t        : %" PRIu32 "\n", x32);
	DEBUG_PRINT("Printing an uint32_t as hex : %" PRIx32 "\n", x32);
	DEBUG_PRINT("Same with capital letters   : %" PRIX32 "\n", x32);
	DEBUG_PRINT("\n");
	
	int32_t y32 = -123456789;
	DEBUG_PRINT("Printing an int32_t         : %" PRIi32 "\n", y32);
	DEBUG_PRINT("Printing an  int32_t as hex : %" PRIx32 "\n", y32);
	DEBUG_PRINT("Same with capital letters   : %" PRIX32 "\n", y32);
	DEBUG_PRINT("\n");
	
	DEBUG_PRINT("That's all folks...\n");
	
}

void loop()
{
	// put your loop code here
}
