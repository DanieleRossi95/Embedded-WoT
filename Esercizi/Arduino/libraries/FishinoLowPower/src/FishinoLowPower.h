//////////////////////////////////////////////////////////////////////////////////////
//						 		FishinoLowPower.h									//
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
#ifndef __FISHINOLOWPOWER_FISHINOLOWPOWER_H
#define __FISHINOLOWPOWER_FISHINOLOWPOWER_H

#include <Arduino.h>

// catch all interrupt, if we don't define any ISR to handle
// wake interrupt -- declared WEAK so it can be redefined
#if defined(_FISHINO_UNO_) || defined(_FISHINO_GUPPY_) || defined(_FISHINO_MEGA_)

	#ifdef __cplusplus
		extern "C" {
	#endif
		void badInterrupt(void) __attribute__((weak));
	#ifdef __cplusplus
		}
	#endif

#endif

#if defined(_FISHINO_MEGA_)

	// ESP to Atmega wake up pin for Fishino MEGA
	// you can choose between those I/O; you MUST wire this I/O with I/O 7 on MEGA board
	//
	// 2, 3 (external interrupts, INT4, INT5))
	// 10..13 (pin change interrupts, PB4..PB7, PCINT4..PCINT7)
	// 14, 15 (pin change interrupt, PJ1, PJ0, PCINT10, PCINT9)
	// 18..21 (external interrupts, 20 and 21 are also used for I2C, not recommended, PD3..PD0, INT3..INT0)
	// A8..A15 (PK0..PK7, pin change interrupt, PCINT16..PCINT23)
	#define ESP_INT_PIN		12
	//
	// ESP_CHPD_PIN
	// Pin connected to ESP CH_PD pin, used to power down the WiFi chip
	// you can choose any free pin, besides the one chosen above
	// you MUST connect this I/O with CH_PD pin on ESPCONN connector
	#define ESP_CHPD_PIN	11
	
#elif defined(_FISHINO_UNO_) || defined(_FISHINO_GUPPY_)

	// ESP to Atmega wake up pin for Fishino UNO and GUPPY
	// it is fixed at I/O 7, as it can be triggered by an interrupt and it's already
	// connected inside the board to GPIO16 on ESP module
	#define ESP_INT_PIN		7
	//
	// ESP_CHPD_PIN
	// Pin connected to ESP CH_PD pin, used to power down the WiFi chip
	// you can choose any free pin; we default to I/O 3 which is usually free
	// you MUST connect this I/O with CH_PD pin on ESPCONN connector
	#define ESP_CHPD_PIN	3
	
#elif defined(FISHINO32)

	#define ESP_CHPD_PIN	31

#elif defined(FISHINO_PIRANHA)

	#define ESP_CHPD_PIN	32

#elif defined(FISHINO_PIRANHA_PROTO)

	#define ESP_CHPD_PIN	32

#else
	#error "Unsupported board"
#endif


// on 8 bit boards it's not defined by core
#ifndef WIFICS
	#define WIFICS 10
#endif


class FishinoLowPowerClass
{
	private:
		
		friend FishinoLowPowerClass &__getFishinoLowPowerClass();
		
		// wifi disabled flag
		bool _wifiDisabled;
		
#ifdef _FISHINO_PIC32_
		// full power off enabled flag
		bool _fullPowerOff;
		
		// pins used as external wakeup interrupt
		// (can't be put in analog low power mode)
		// ports b..g, 6 32 bit ports total)
		uint32_t _intPins[6];
		uint32_t _intPullups[6];
		uint32_t _intPulldowns[6];
		
		// used I/O which shall not be disabled when
		// going low power.
		// ports b..g, 6 32 bit ports total)
		uint32_t _usedIOs[6];
		
#endif
		
		// enabled periferal flags
		uint16_t _peripheralsEnabled;

		// private constructor -- we don't want more objects
		// besides the one created by macro
		FishinoLowPowerClass();
		
	protected:

	public:
		
		// put board on sleep for requested time
		// REMEMBER - on 8 bit boards this needs wiring!
		#ifdef _FISHINO_PIC32_
			void deepSleep(uint32_t ms) __attribute__((nomips16));
		#else
			void deepSleep(uint32_t ms);
		#endif
		
		// switch wifi module power
		void wifiOff(void);
		void wifiOn(void);
		
#if defined(_FISHINO_PIC32_)
		// on Fishino 32 bit boards there is an hardware power off available
		// please READ THE DOCUMENTS BEFORE USING IT!
		// it needs a battery connected to the board!
		void enableFullPowerOff(bool enable = true)
			{ _fullPowerOff = enable; }
		void disableFullPowerOff(void) { enableFullPowerOff(false); }
#endif
		
		// enable wakeup from external pin state change
		bool enableInterruptPin(uint16_t pin, bool enable = true);
		bool disableInterruptPin(uint16_t pin) { return enableInterruptPin(pin, false); }
		
#if defined(_FISHINO_PIC32_)
		bool enableInterruptPinPullup(uint16_t pin, bool enable = true);
		bool disableInterruptPinPullup(uint16_t pin) { return enableInterruptPinPullup(pin, false); }

		bool enableInterruptPinPulldown(uint16_t pin, bool enable = true);
		bool disableInterruptPinPulldown(uint16_t pin) { return enableInterruptPinPulldown(pin, false); }

		bool portUsed(uint16_t pin, bool used = true);
		bool portNotUsed(uint16_t pin) { return portUsed(pin, false); }
#endif

		
		// enable wakeup by serial ports
		// note : enabled peripherals must be ON, so they use power
#if defined(_FISHINO_PIC32_)
		// NOTE : USB wakeup works ONLY if board is not in full power off mode
		void enableUSBWakeup(bool enable = true);
		void disableUSBWakeup(void) { enableUSBWakeup(false); }
#endif
		
#if defined(_FISHINO_UNO_) || defined(_FISHINO_MEGA_) || defined(_FISHINO_GUPPY_)
		void enableSerialWakeup(bool enable = true);
		void disableSerialWakeup(void) { enableSerialWakeup(false); }
#endif
		
#if defined(_FISHINO_PIC32_) || defined(_FISHINO_MEGA_)
		void enableSerial0Wakeup(bool enable = true);
		void disableSerial0Wakeup(void) { enableSerial0Wakeup(false); }
#endif
		
#if defined(_FISHINO_MEGA_)
		void enableSerial1Wakeup(bool enable = true);
		void disableSeria1lWakeup(void) { enableSerial1Wakeup(false); }
		
		void enableSerial2Wakeup(bool enable = true);
		void disableSerial2Wakeup(void) { enableSerial2Wakeup(false); }
		
		void enableSerial3Wakeup(bool enable = true);
		void disableSerial3Wakeup(void) { enableSerial3Wakeup(false); }
#endif
		
		// enable I2C/Wire wakeup
		void enableI2CWakeup(bool enable = true);
		void disableI2CWakeup(void) { enableI2CWakeup(false); }
		
#ifdef _FISHINO_PIC32_
		// enable SPI wakeup
		void enableSPIWakeup(bool enable = true);
		void disableSPIWakeup(void) { enableSPIWakeup(false); }
#endif
};

extern FishinoLowPowerClass &__getFishinoLowPowerClass();

#define FishinoLowPower __getFishinoLowPowerClass()

#endif
