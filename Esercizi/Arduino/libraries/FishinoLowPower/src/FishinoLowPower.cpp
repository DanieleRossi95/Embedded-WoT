//////////////////////////////////////////////////////////////////////////////////////
//						 		FishinoLowPower.cpp									//
//																					//
//						Energy savings features for Fishino boards					//
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
//  Version 7.3.0 - December 2017	Initial version									//
//  Version 7.3.2 - 2018/01/05		Added support for Piranha boards				//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#include "FishinoLowPower.h"

// switch wifi module power
void FishinoLowPowerClass::wifiOff(void)
{
	// do nothing if already disabled
	if(_wifiDisabled)
		return;
	
	// mark as disabled
	_wifiDisabled = true;
	
	// shut it down
	digitalWrite(ESP_CHPD_PIN, LOW);
	pinMode(ESP_CHPD_PIN, OUTPUT);
}

void FishinoLowPowerClass::wifiOn(void)
{
	if(!_wifiDisabled)
		return;
	
	// put ESP in OFF mode, just to be sure
	digitalWrite(ESP_CHPD_PIN, LOW);
	pinMode(ESP_CHPD_PIN, OUTPUT);
	
	// CS must be low to correctly re-boot ESP module
	// otherwise it'll boot in sd mode
	digitalWrite(WIFICS, LOW);
	pinMode(WIFICS, OUTPUT);

	// give ESP some time to get it
	delay(50);

	// wake up esp
	digitalWrite(ESP_CHPD_PIN, HIGH);
	
	// give it some time to start boot
	// (mandatory too!!!)
	delay(50);
	
	// deselect it again
	digitalWrite(WIFICS, HIGH);
	
	// mark as enabled
	_wifiDisabled = false;
}

FishinoLowPowerClass &__getFishinoLowPowerClass()
{
	static FishinoLowPowerClass lp;
	return lp;
}


