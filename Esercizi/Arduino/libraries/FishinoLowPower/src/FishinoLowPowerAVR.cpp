//////////////////////////////////////////////////////////////////////////////////////
//						 		FishinoLowPowerAVR.h								//
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

#ifdef __AVR__

#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>

#include <Fishino.h>

#if defined(__AVR_ATmega328P__)
	#ifndef sleep_bod_disable
		#define sleep_bod_disable()								\
			do { 												\
				unsigned char tempreg; 							\
				__asm__ __volatile__(							\
					"in %[tempreg], %[mcucr]" "\n\t" 			\
					"ori %[tempreg], %[bods_bodse]" "\n\t" 		\
					"out %[mcucr], %[tempreg]" "\n\t" 			\
					"andi %[tempreg], %[not_bodse]" "\n\t" 		\
					"out %[mcucr], %[tempreg]" 					\
					: [tempreg] "=&d" (tempreg) 				\
					: [mcucr] "I" _SFR_IO_ADDR(MCUCR), 			\
					[bods_bodse] "i" (_BV(BODS) | _BV(BODSE)),	\
					[not_bodse] "i" (~_BV(BODSE))); 			\
			} while (0)
	#endif
#else
	#define sleep_bod_disable()
#endif

#define	lowPowerBodOn(mode)		\
	do { 						\
		set_sleep_mode(mode);	\
		cli();					\
		sleep_enable();			\
		sei();					\
		sleep_cpu();			\
		sleep_disable();		\
		sei();					\
	} while (0)

#if defined(__AVR_ATmega328P__) || defined(_FISHINO_MEGA_)
	#define	lowPowerBodOff(mode)	\
		do { 							\
			set_sleep_mode(mode);		\
			cli();						\
			sleep_enable();				\
			sleep_bod_disable();		\
			sei();						\
			sleep_cpu();				\
			sleep_disable();			\
			sei();						\
		} while (0)
#endif

#if defined(__AVR_HAVE_PRR1_PRUSART3)
	#define power_usart3_enable()           (PRR1 &= (uint8_t)~(1 << PRUSART3))
	#define power_usart3_disable()          (PRR1 |= (uint8_t)(1 << PRUSART3))
#endif

// enabled peripherals in off mode
#define		PERIPH_SERIAL		0x0001
#define		PERIPH_SERIAL0		0x0002
#define		PERIPH_SERIAL1		0x0004
#define		PERIPH_SERIAL2		0x0008
#define		PERIPH_SERIAL3		0x0010
#define		PERIPH_SPI			0x0020
#define		PERIPH_I2C			0x0040
#define		PERIPH_IDLE			(PERIPH_SERIAL | PERIPH_SERIAL0 | PERIPH_SERIAL1 | PERIPH_SERIAL2 | PERIPH_SERIAL3 | PERIPH_SPI)
			
// catch all interrupt routine -- points to a weak, redefinable function
void badInterrupt(void)	{}
ISR(BADISR_vect)		{ badInterrupt(); }

// enable interrupt on wakeup input
static void enableWakeupInterrupt(bool enable)
{
#if defined(_FISHINO_MEGA_)

	// 2, 3 (external interrupts, INT4, INT5))
	// 10..13 (pin change interrupts, PB4..PB7, PCINT4..PCINT7)
	// 14, 15 (pin change interrupt, PJ1, PJ0, PCINT10, PCINT9)
	// 18..21 (external interrupts, 20 and 21 are also used for I2C, not recommended, PD3..PD0, INT3..INT0)
	// A8..A15 (PK0..PK7, pin change interrupt, PCINT16..PCINT23)
	pinMode(ESP_INT_PIN, INPUT);

	#if ESP_INT_PIN == 2 || ESP_INT_PIN == 3
		EICRB &= ~(0x03 << ((ESP_INT_PIN - 2) * 2));
		EICRB |= (0x02 << ((ESP_INT_PIN - 2) * 2));
		EIMSK &= ~(0x01 << (ESP_INT_PIN + 2));
		if(enable)
		{
			EIFR |= (0x01 << (ESP_INT_PIN + 2));
			EIMSK |= (0x01 << (ESP_INT_PIN + 2));
		}
			
	#elif ESP_INT_PIN == 10 || ESP_INT_PIN == 11 || ESP_INT_PIN == 12 || ESP_INT_PIN == 13
		if(enable)
		{
			PCIFR |= 0x01;
			PCICR |= 0x01;
			PCMSK0 |= (0x01 << (ESP_INT_PIN - 6));
		}
		else
			PCMSK0 &= ~(0x01 << (ESP_INT_PIN - 6));

	#elif ESP_INT_PIN == 14 || ESP_INT_PIN == 15
		if(enable)
		{
			PCIFR |= 0x02;
			PCICR |= 0x02;
			PCMSK1 |= (0x01 << (16 - ESP_INT_PIN));
		}
		else
			PCMSK1 &= ~(0x01 << (16 - ESP_INT_PIN));

	#elif ESP_INT_PIN == 18 || ESP_INT_PIN == 19 || ESP_INT_PIN == 20 || ESP_INT_PIN == 21
		EICRA &= ~(0x03 << ((21 - ESP_INT_PIN) * 2));
		EICRA |= (0x02 << ((21 - ESP_INT_PIN) * 2));
		EIMSK &= ~(0x01 << (21 - ESP_INT_PIN));
		if(enable)
		{
			EIFR |= (0x01 << (21 - ESP_INT_PIN));
			EIMSK |= (0x01 << (21 - ESP_INT_PIN));
		}

	#elif \
		ESP_INT_PIN == A8  || ESP_INT_PIN == A9  || ESP_INT_PIN == A10 || ESP_INT_PIN == A11 || \
		ESP_INT_PIN == A12 || ESP_INT_PIN == A13 || ESP_INT_PIN == A14 || ESP_INT_PIN == A15
		if(enable)
		{
			PCIFR |= 0x04;
			PCICR |= 0x04;
			PCMSK2 |= (0x01 << (ESP_INT_PIN - A8));
		}
		else
			PCMSK2 &= ~(0x01 << (ESP_INT_PIN - A8));
	#else
		#error "Invalid ESP_INT_PIN selected"
	#endif


#elif defined(_FISHINO_UNO_) || defined(_FISHINO_GUPPY_)

	// D7 == PD7 == PCINT23
	pinMode(ESP_INT_PIN, INPUT_PULLUP);

	#if ESP_INT_PIN == 7
		if(enable)
		{
			PCIFR |= 0x04;
			PCICR |= 0x04;
			PCMSK2 |= 0x80;
		}
		else
			PCMSK2 &= ~0x80;
	#else
		#error "Invalid ESP_INT_PIN selected"
	#endif
#else
	#error "Unsupported board"
#endif
}

// private constructor -- we don't want more objects
// besides the one created by macro
FishinoLowPowerClass::FishinoLowPowerClass()
{
	// wifi module ON by default
	_wifiDisabled = false;
	
	// no peripheral enabled in sleep mode by default
	_peripheralsEnabled = 0;

	// ensure that WIFI module is on
	digitalWrite(ESP_CHPD_PIN, HIGH);
	pinMode(ESP_CHPD_PIN, OUTPUT);
}

// put board on sleep for requested time
// REMEMBER - on 8 bit boards this needs wiring!
// if you put 0 as time, the board will NOT wake up on timer event
// but only on external interrupts (you must enable them)
void FishinoLowPowerClass::deepSleep(uint32_t ms)
{
	// for sleep time other than 0 (sleep forever)
	// we need the wifi module active as we use its RTC
	bool wifiOff;
	
	if(ms)
	{
		wifiOff = _wifiDisabled;
		if(_wifiDisabled)
			wifiOn();
	
		delay(10);

		// put wifi module on sleep
		Fishino.deepSleep(ms);
		
		// CS must be low to correctly re-boot ESP module
		// otherwise it'll boot in sd mode
		digitalWrite(WIFICS, LOW);
		pinMode(WIFICS, OUTPUT);
	
		// give ESP some time to go to sleep
		// otherwise he can send some spurious interrupt
		// (mandatory, otherwise we'll be waken immediately)
		delay(200);
	}

	// turn off all peripherals
	
	// for timer2 suppress the clock too
	uint8_t clkSource = 0;
	if (TCCR2B & CS22)
		clkSource |= (1 << CS22);
	if (TCCR2B & CS21)
		clkSource |= (1 << CS21);
	if (TCCR2B & CS20)
		clkSource |= (1 << CS20);
	TCCR2B &= ~(1 << CS22);
	TCCR2B &= ~(1 << CS21);
	TCCR2B &= ~(1 << CS20);
	power_timer2_disable();

	power_adc_disable();
	ADCSRA &= ~(1 << ADEN);

	ACSR &= ~(1 << ACD);

#ifdef _FISHINO_MEGA_
	power_timer5_disable();	
	power_timer4_disable();	
	power_timer3_disable();	
#endif
	power_timer1_disable();	
	power_timer0_disable();	
	power_spi_disable();
#ifdef _FISHINO_MEGA_
	if(!(_peripheralsEnabled & PERIPH_SERIAL2))
		power_usart3_disable();
	if(!(_peripheralsEnabled & PERIPH_SERIAL1))
		power_usart2_disable();
	if(!(_peripheralsEnabled & PERIPH_SERIAL0))
		power_usart1_disable();
#endif
	if(!(_peripheralsEnabled & PERIPH_SERIAL))
		power_usart0_disable();
	if(!(_peripheralsEnabled & PERIPH_I2C))
		power_twi_disable();
	
	// put some used I/O as input
#ifdef _FISHINO_MEGA_
	pinMode(1, INPUT);		
	pinMode(52, INPUT);
	pinMode(51, INPUT);
#elif defined(_FISHINO_UNO_) || defined(_FISHINO_GUPPY_)
	pinMode(1, INPUT);		
	pinMode(13, INPUT);
	pinMode(11, INPUT);
#endif

	// enable interrupts on selected input
	if(ms)
		enableWakeupInterrupt(true);
	
	// put the controller in halt state
	// if some periferals needs it, use idle mode
	// otherwise use power down mode
	if(_peripheralsEnabled & PERIPH_IDLE)
		lowPowerBodOff(SLEEP_MODE_IDLE);
	else
		lowPowerBodOff(SLEEP_MODE_PWR_DOWN);

	// disable interrupts on selected input
	if(ms)
		enableWakeupInterrupt(false);

#ifdef _FISHINO_MEGA_
	pinMode(1, OUTPUT);		
	pinMode(52, OUTPUT);
	pinMode(51, OUTPUT);
#elif defined(_FISHINO_UNO_) || defined(_FISHINO_GUPPY_)
	pinMode(1, OUTPUT);		
	pinMode(13, OUTPUT);
	pinMode(11, OUTPUT);
#endif

	// turn on again all peripherals

	if (clkSource & CS22)
		TCCR2B |= (1 << CS22);
	if (clkSource & CS21)
		TCCR2B |= (1 << CS21);
	if (clkSource & CS20)
		TCCR2B |= (1 << CS20);
	power_timer2_enable();

	ADCSRA |= (1 << ADEN);
	power_adc_enable();

#ifdef _FISHINO_MEGA_
	power_timer5_enable();
	power_timer4_enable();
	power_timer3_enable();
#endif
	power_timer1_enable();
	power_timer0_enable();
	power_spi_enable();
#ifdef _FISHINO_MEGA_
	if(!(_peripheralsEnabled & PERIPH_SERIAL2))
		power_usart3_enable();
	if(!(_peripheralsEnabled & PERIPH_SERIAL1))
		power_usart2_enable();
	if(!(_peripheralsEnabled & PERIPH_SERIAL0))
		power_usart1_enable();
#endif
	if(!(_peripheralsEnabled & PERIPH_SERIAL))
		power_usart0_enable();
	if(!(_peripheralsEnabled & PERIPH_I2C))
		power_twi_enable();
	
	// give some time to settle
	// (this is mandatory!!!)
	delay(5);
	
	if(ms)
	{
		// put ESP in OFF mode
		digitalWrite(ESP_CHPD_PIN, LOW);
		
		// if wifi module was enabled before sleep
		// re-enable it
		if(!wifiOff)
		{
			// prepare to wake up esp correctly
			digitalWrite(WIFICS, LOW);
			
			// give time to ESP to feel CS Low
			// (it needs to boot in correct mode...)
			delay(1);
			
			// wake up esp
			digitalWrite(ESP_CHPD_PIN, HIGH);
			
			// give it some time to start boot
			// (mandatory too!!!)
			delay(50);
			
			// deselect it again
			digitalWrite(WIFICS, HIGH);
		}
		else
			// otherwise re-set it as disabled
			_wifiDisabled = true;
	}

}

// enable wakeup from external pin state change
bool FishinoLowPowerClass::enableInterruptPin(uint16_t pin, bool enable)
{
#ifdef _FISHINO_MEGA_
	if(
		(pin >= 10 && pin <= 15)		||
		(pin >= A8 && pin <= A15)
	)
	{
		pinMode(pin, INPUT);

		// enable pin
		if(enable)
			*digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));
		else
			*digitalPinToPCMSK(pin) &= ~bit (digitalPinToPCMSKbit(pin));

		// clear any outstanding interrupt
		PCIFR  |= bit (digitalPinToPCICRbit(pin));

		// enable interrupt for the group
		if(enable)
			PCICR  |= bit (digitalPinToPCICRbit(pin));

		return true;
	}
	else if(pin == 2 || pin == 3 || (pin >= 18 && pin <= 21))
	{
		int intr = digitalPinToInterrupt(pin);
		if(intr == NOT_AN_INTERRUPT)
			return false;
		if(pin < 4)
		{
			EICRA &= ~(0x03 << (2 * intr));
			EICRA |= (0x01 << (2 * intr));
		}
		else
		{
			intr -= 4;
			EICRB &= ~(0x03 << (2 * intr));
			EICRB |= (0x01 << (2 * intr));
			intr += 4;
		}
		EIFR |= 1 << intr;
		if(enable)
			EIMSK |= 1 << intr;
		else
			EIMSK &= ~(1 << intr);
		return true;
	}
	else
		return false;
#elif defined(_FISHINO_UNO_) || defined(_FISHINO_GUPPY_)

	// do NOT use I/O 7, is already used
	// do NOT use I/O 10..13, they're also in use for SPI
	if(pin == 7 || (pin >= 10 && pin <= 13) || pin > A7)
		return false;
	// on pin 2 and 3 use the normal external vector
	if(pin == 2 || pin == 3)
	{
		int intr = pin - 2;
		EICRA &= ~(0x03 << (2 * intr));
		EICRA |= (0x01 << (2 * intr));
		if(enable)
			EIMSK |= 1 << intr;
		else
			EIMSK &= ~(1 << intr);
	}
	else
	{
		// enable pin
		if(enable)
			*digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));
		else
			*digitalPinToPCMSK(pin) &= ~bit (digitalPinToPCMSKbit(pin));

		// clear any outstanding interrupt
		PCIFR  |= bit (digitalPinToPCICRbit(pin));

		// enable interrupt for the group
		if(enable)
			PCICR  |= bit (digitalPinToPCICRbit(pin));
	}
	return true;

#else
#error "Unsupported board"
#endif
}

// enable wakeup by serial ports
// note : enabled peripherals must be ON, so they use power
void FishinoLowPowerClass::enableSerialWakeup(bool enable)
{
	if(enable)
		_peripheralsEnabled |= PERIPH_SERIAL;
	else
		_peripheralsEnabled &= ~PERIPH_SERIAL;
}

#ifdef _FISHINO_MEGA_
void FishinoLowPowerClass::enableSerial0Wakeup(bool enable)
{
	if(enable)
		_peripheralsEnabled |= PERIPH_SERIAL0;
	else
		_peripheralsEnabled &= ~PERIPH_SERIAL0;
}

void FishinoLowPowerClass::enableSerial1Wakeup(bool enable)
{
	if(enable)
		_peripheralsEnabled |= PERIPH_SERIAL1;
	else
		_peripheralsEnabled &= ~PERIPH_SERIAL1;
}

void FishinoLowPowerClass::enableSerial2Wakeup(bool enable)
{
	if(enable)
		_peripheralsEnabled |= PERIPH_SERIAL2;
	else
		_peripheralsEnabled &= ~PERIPH_SERIAL2;
}

void FishinoLowPowerClass::enableSerial3Wakeup(bool enable)
{
	if(enable)
		_peripheralsEnabled |= PERIPH_SERIAL3;
	else
		_peripheralsEnabled &= ~PERIPH_SERIAL3;
}
#endif

// enable I2C/Wire wakeup
void FishinoLowPowerClass::enableI2CWakeup(bool enable)
{
	if(enable)
		_peripheralsEnabled |= PERIPH_I2C;
	else
		_peripheralsEnabled &= ~PERIPH_I2C;
}

#endif
