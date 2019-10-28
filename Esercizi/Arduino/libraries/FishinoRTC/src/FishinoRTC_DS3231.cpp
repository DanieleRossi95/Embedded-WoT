//////////////////////////////////////////////////////////////////////////////////////
//						 		FishinoRTC_DS3231.cpp								//
//																					//
//					Manage Real Time Controllers on Fishino Boards					//
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
//  Version 7.3.2 - 2018/01/05		Small fixes										//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#include "FishinoRTC.h"

#include <Wire.h>

#include <FishinoFlash.h>

#ifdef _HAS_DS3231_

#if defined(_FISHINO_UNO_R2_)
	#define RTC_ALARM_IO	6 // @@ FIX IN PRODUCTION : FISHINO_ALARM
#elif defined(_FISHINO_MEGA_R2_)
	#define RTC_ALARM_IO	FISHINO_ALARM
#else
	#error "Unsupported board"
#endif

#define RTC_ADDRESS		0x68

// DS3231 Register Addresses
#define RTC_SECONDS		0x00
#define RTC_MINUTES		0x01
#define RTC_HOURS		0x02
#define RTC_DAY			0x03
#define RTC_DATE		0x04
#define RTC_MONTH		0x05
#define RTC_YEAR		0x06
#define ALM1_SECONDS	0x07
#define ALM1_MINUTES	0x08
#define ALM1_HOURS		0x09
#define ALM1_DAYDATE	0x0A
#define ALM2_MINUTES	0x0B
#define ALM2_HOURS		0x0C
#define ALM2_DAYDATE	0x0D
#define RTC_CONTROL		0x0E
#define RTC_STATUS		0x0F
#define RTC_AGING		0x10
#define RTC_TEMP_MSB	0x11
#define RTC_TEMP_LSB	0x12

//Alarm mask bits
#define A1M1				7
#define A1M2				7
#define A1M3				7
#define A1M4				7
#define A2M2				7
#define A2M3				7
#define A2M4				7

//Control register bits
#define EOSC				7
#define BBSQW				6
#define CONV				5
#define RS2					4
#define RS1					3
#define INTCN				2
#define A2IE				1
#define A1IE				0

//Status register bits
#define OSF					7
#define BB32KHZ				6
#define CRATE1				5
#define CRATE0				4
#define EN32KHZ				3
#define BSY					2
#define A2F					1
#define A1F					0

//Square-wave output frequency (TS2, RS1 bits)
enum DS3231_SQWAVE_FREQS_t {SQWAVE_1_HZ, SQWAVE_1024_HZ, SQWAVE_4096_HZ, SQWAVE_8192_HZ, SQWAVE_NONE};

//Alarm masks
enum DS3231_ALARM_TYPES_t {
    ALM1_EVERY_SECOND	= 0x0F,
    ALM1_MATCH_SECONDS	= 0x0E,
    ALM1_MATCH_MINUTES	= 0x0C,		// match minutes *and* seconds
    ALM1_MATCH_HOURS	= 0x08,		// match hours *and* minutes, seconds
    ALM1_MATCH_DATE		= 0x00,		// match date *and* hours, minutes, seconds
    ALM1_MATCH_DAY		= 0x10,		// match day *and* hours, minutes, seconds
    ALM2_EVERY_MINUTE	= 0x8E,
    ALM2_MATCH_MINUTES	= 0x8C,		// match minutes
    ALM2_MATCH_HOURS	= 0x88,		// match hours *and* minutes
    ALM2_MATCH_DATE		= 0x80,		// match date *and* hours, minutes
    ALM2_MATCH_DAY		= 0x90,		// match day *and* hours, minutes
};

//constants for calling functions
#define ALARM_1			1
#define ALARM_2			2

//Other
#define DS1307_CH		7	// for DS1307 compatibility, Clock Halt bit in Seconds register
#define HR1224			6	// Hours register 12 or 24 hour mode (24 hour mode==0)
#define CENTURY			7	// Century bit in Month register
#define DYDT			6	// Day/Date flag bit in alarm Day/Date registers


static uint8_t bcd2bin(uint8_t val)
{
	return val - 6 * (val >> 4);
}
static uint8_t bin2bcd(uint8_t val)
{
	return val + 6 * (val / 10);
}

// read an RTC register
static uint8_t readReg(uint8_t reg)
{
	Wire.beginTransmission(RTC_ADDRESS);
	Wire.write(reg);
	Wire.endTransmission();
	Wire.requestFrom(RTC_ADDRESS, 1);
	return Wire.read();
}

// write an RTC register
static void writeReg(uint8_t reg, uint8_t val)
{
	Wire.beginTransmission(RTC_ADDRESS);
	Wire.write(reg);
	Wire.write(val);
	Wire.endTransmission();
}

// constructor
FishinoRTCClass::FishinoRTCClass()
{
	Wire.begin();
	
	_alarmSeconds = 0;
	_alarmMode = AlarmModes::ALARM_ONCE;
	_alarmCount = 0;
	_alarmHandler = NULL;
	
	_alarm2Mode = AlarmModes::ALARM_ONCE;
	_alarm2Count = 0;
	_alarm2Handler = NULL;
	
	// set alarm output to trigger on alarm
	// (disable square wave output)
	// and disable both alarms
	uint8_t ctrlMask = readReg(RTC_CONTROL);
	ctrlMask |= _BV(INTCN);
	ctrlMask &= ~(_BV(A2IE) | _BV(A1IE));
	writeReg(RTC_CONTROL, ctrlMask);
	
	// clear alarm interrupt flags
	uint8_t statusMask = readReg(RTC_STATUS);
	statusMask &= ~(_BV(A2F) | _BV(A1F));
	writeReg(RTC_STATUS, statusMask);
}

// destructor
FishinoRTCClass::~FishinoRTCClass()
{
}

// the global ISR
#if defined(_FISHINO_UNO_R2_)
ISR(PCINT2_vect)
#elif defined(_FISHINO_MEGA_R2_)
void _ALARM_ISR(void)
#else
#error "Unsupported board"
#endif
{
	RTC._isr();
}

// enable interrupt on alarm input
static void enableAlarmInterrupt(bool enable)
{
	// set Pin as Input (default) with pullup resistor
	pinMode(RTC_ALARM_IO, INPUT_PULLUP);
	
	// enable pullup resistor
//	digitalWrite(RTC_ALARM_IO, HIGH);

#if defined(_FISHINO_UNO_R2_)

	// enable pin
	*digitalPinToPCMSK(RTC_ALARM_IO) |= bit(digitalPinToPCMSKbit(RTC_ALARM_IO));
	
	// clear any pending interrupts on it
	PCIFR  |= bit(digitalPinToPCICRbit(RTC_ALARM_IO));
	
	// enable interrupt for pin group
	PCICR  |= bit(digitalPinToPCICRbit(RTC_ALARM_IO));

#elif defined(_FISHINO_MEGA_R2_)

	attachInterrupt(digitalPinToInterrupt(RTC_ALARM_IO), _ALARM_ISR, FALLING);

#else
	#error "Unsupported board"
#endif
}


// the internal ISR
void FishinoRTCClass::_isr(void)
{
	static bool reenter = false;
	
	if(reenter)
		return;
	reenter = true;
	
	interrupts();
	
	// read alarm source
	uint8_t s = readReg(RTC_STATUS);

	bool alarm1 = (s & _BV(A1F));
	bool alarm2 = (s & _BV(A2F));

	// clear alarm state
	s &= ~(A1F | A2F);
	writeReg(RTC_STATUS, s);
	
	if(alarm1)
	{
	
		// if we've set an handler, just call it
		if(_alarmHandler)
			_alarmHandler();
	
		// decrement alarm counter if needed
		if(_alarmMode == AlarmModes::ALARM_REPEAT_N)
			_alarmCount--;

		// if alarm count has expired, disable futher alarma
		if( (_alarmMode == AlarmModes::ALARM_REPEAT_N && !_alarmCount) || (_alarmMode == AlarmModes::ALARM_ONCE))
			enableAlarmHelper(false, false);
		// otherwise, if we've set a seconds alarm, seset it
		else if(_alarmSeconds)
			setAlarm(_alarmSeconds);
	}

	if(alarm2)
	{
		// if we've set an handler, just call it
		if(_alarm2Handler)
			_alarm2Handler();
	
		// decrement alarm counter if needed
		if(_alarm2Mode == AlarmModes::ALARM_REPEAT_N)
			_alarm2Count--;

		// if alarm count has expired, disable futher alarma
		if( (_alarm2Mode == AlarmModes::ALARM_REPEAT_N && !_alarm2Count) || (_alarm2Mode == AlarmModes::ALARM_ONCE))
			enableAlarmHelper(true, false);
	}
	
	noInterrupts();
	
	reenter = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// ALARM1 HELPERS //////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

// enable/disable/request helpers
void FishinoRTCClass::enableAlarmHelper(bool alarm2, bool enable)
{
	uint8_t ctrlMask = readReg(RTC_CONTROL);
	uint8_t alarmMask;
	if(alarm2)
		alarmMask = _BV(A2IE);
	else
		alarmMask = _BV(A1IE);
	if(enable)
		ctrlMask |= alarmMask;
	else
		ctrlMask &= ~alarmMask;
	writeReg(RTC_CONTROL, ctrlMask);
	
	// enable/disable interrupt handling
	enableAlarmInterrupt(ctrlMask & (_BV(A1IE) | _BV(A2IE)));
}

bool FishinoRTCClass::isAlarmEnabledHelper(bool alarm2) const
{
	uint8_t ctrlMask = readReg(RTC_CONTROL);
	if(alarm2)
		return ctrlMask & _BV(A2IE);
	else
		return ctrlMask & _BV(A1IE);
}

void FishinoRTCClass::clearAlarmStatusHelper(bool alarm2)
{
	uint8_t statusMask = readReg(RTC_STATUS);
	if(alarm2)
		statusMask &= ~_BV(A2F);
	else
		statusMask &= ~_BV(A1F);
	writeReg(RTC_STATUS, statusMask);
}

// set alarm - helper function
void FishinoRTCClass::setAlarmHelper(bool alarm2, DateTime const &dt, uint8_t flags, bool enable)
{
	// extract fields and apply alarm mask
	uint8_t second = bin2bcd(dt.second());
	uint8_t minute = bin2bcd(dt.minute());
	uint8_t hour   = bin2bcd(dt.hour());
	uint8_t day;
	if(flags & ALM1_MATCH_DAY)
		day = bin2bcd(dt.dayOfTheWeek());
	else
		day = bin2bcd(dt.day());
	
	if(flags & 0x01)
		second |= _BV(A1M1);
	if(flags & 0x02)
		minute |= _BV(A1M2);
	if(flags & 0x04)
		hour |= _BV(A1M3);
	if(flags & 0x08)
		day |= _BV(A1M4);
	if(flags & 0x10)
		day |= _BV(DYDT);
	
	// set the alarm
	uint8_t addr;
	if(!alarm2)
	{
		addr = ALM1_SECONDS;
		writeReg(addr++, bin2bcd(dt.second()));
	}
	else
		addr = ALM2_MINUTES;
	writeReg(addr++, bin2bcd(dt.minute()));
	writeReg(addr++, bin2bcd(dt.hour()));
	writeReg(addr++, bin2bcd(dt.day()));
	
	// enable/disable the alarm as requested
	enableAlarmHelper(alarm2, enable);
	
	// clear alarm flag
	clearAlarmStatusHelper(alarm2);
}

// get alarm - helper function
DateTime  FishinoRTCClass::getAlarmHelper(bool alarm2) const
{
	uint8_t second, minute, hour, day;

	// get the alarm
	uint8_t addr;
	if(!alarm2)
	{
		addr = ALM1_SECONDS;
		second = readReg(addr++);
	}
	else
		second = 0;
	minute = readReg(addr++);
	hour = readReg(addr++);
	day = readReg(addr++);
	
	// get flags
	uint8_t flags = 0;
	if(second & _BV(A1M1))
	{
		flags |= 0x01;
		second &= ~_BV(A1M1);
	}
	if(minute & _BV(A1M2))
	{
		flags |= 0x02;
		minute &= ~_BV(A1M2);
	}
	if(hour & _BV(A1M3))
	{
		flags |= 0x04;
		hour &= ~_BV(A1M3);
	}
	if(day & _BV(A1M4))
	{
		flags |= 0x08;
		day &= ~_BV(A1M4);
	}
	if(day & _BV(DYDT))
	{
		flags |= 0x10;
		day &= ~_BV(DYDT);
	}
	
	// convert to binary
	second	= bcd2bin(second);
	minute	= bcd2bin(minute);
	hour	= bcd2bin(hour);
	day		= bcd2bin(day);
	
	// now we must find next alarm full date, from now()
	DateTime dt = now();
	if(flags == ALM1_EVERY_SECOND)
		return dt + TimeSpan(1);
	else if(flags == ALM1_MATCH_SECONDS)
	{
		if(dt.second() > second)
			return dt;
		else
			return dt + TimeSpan(60 - second + dt.second());
	}
	else if(flags == ALM1_MATCH_MINUTES)
	{
		if(dt.minute() > minute)
			return dt;
		else if(dt.minute() == minute)
		{
			if(dt.second() > second)
				return dt;
			else
				return dt + TimeSpan(60 - second + dt.second());
		}
		else
			return dt + (TimeSpan(0, 1, minute, second) - TimeSpan(0, 0, dt.minute(), dt.second()));
	}
	else if(flags == ALM1_MATCH_HOURS)
	{
		if(dt.hour() > hour)
			return dt;
		else if(dt.hour() == hour)
		{
			if(dt.minute() > minute)
				return dt;
			else if(dt.minute() == minute)
			{
				if(dt.second() > second)
					return dt;
				else
					return dt + TimeSpan(60 - second + dt.second());
			}
			else
				return dt + (TimeSpan(0, 1, minute, second) - TimeSpan(0, 0, dt.minute(), dt.second()));
		}
		else
			return dt + (TimeSpan(1, hour, minute, second) - TimeSpan(0, dt.hour(), dt.minute(), dt.second()));
	}

	else if(flags == ALM1_MATCH_DATE)
	{
		if(dt.day() > day)
			return dt;
		else if(dt.day() == day)
		{
			if(dt.hour() > hour)
				return dt;
			else if(dt.hour() == hour)
			{
				if(dt.minute() > minute)
					return dt;
				else if(dt.minute() == minute)
				{
					if(dt.second() > second)
						return dt;
					else
						return dt + TimeSpan(60 - second + dt.second());
				}
				else
					return dt + (TimeSpan(0, 1, minute, second) - TimeSpan(0, 0, dt.minute(), dt.second()));
			}
			else
				return dt + (TimeSpan(1, hour, minute, second) - TimeSpan(0, dt.hour(), dt.minute(), dt.second()));
		}
		else
			return dt + (TimeSpan(day + 365, hour, minute, second) - TimeSpan(dt.day(), dt.hour(), dt.minute(), dt.second()));
	}
	
	else /* if(flags == ALM1_MATCH_DAY) */
	{
		if(dt.dayOfTheWeek() > day)
			return dt;
		else if(dt.dayOfTheWeek() == day)
		{
			if(dt.hour() > hour)
				return dt;
			else if(dt.hour() == hour)
			{
				if(dt.minute() > minute)
					return dt;
				else if(dt.minute() == minute)
				{
					if(dt.second() > second)
						return dt;
					else
						return dt + TimeSpan(60 - second + dt.second());
				}
				else
					return dt + (TimeSpan(0, 1, minute, second) - TimeSpan(0, 0, dt.minute(), dt.second()));
			}
			else
				return dt + (TimeSpan(1, hour, minute, second) - TimeSpan(0, dt.hour(), dt.minute(), dt.second()));
		}
		else
			return dt + (TimeSpan(day + 7, hour, minute, second) - TimeSpan(dt.dayOfTheWeek(), dt.hour(), dt.minute(), dt.second()));
	}
}

// save/restore alarm state helpers
void FishinoRTCClass::saveAlarmStateHelper(bool alarm2, RTCAlarmState &state) const
{
	state.enabled = isAlarmEnabledHelper(alarm2);
	if(alarm2)
	{
		state.mode			= _alarm2Mode;
		state.repeatCount	= _alarm2Count;
		state.handler		= _alarm2Handler;
		state.alarmSeconds	= 0;
	}
	else
	{
		state.mode			= _alarmMode;
		state.repeatCount	= _alarmCount;
		state.handler		= _alarmHandler;
		state.alarmSeconds	= _alarmSeconds;
	}
	
	uint8_t addr;
	if(!alarm2)
	{
		addr = ALM1_SECONDS;
		state.second = readReg(addr++);
	}
	else
		state.second = 0;
	state.minute = readReg(addr++);
	state.hour = readReg(addr++);
	state.day = readReg(addr++);
}

void FishinoRTCClass::restoreAlarmStateHelper(bool alarm2, RTCAlarmState const &state)
{
	uint8_t addr;
	if(!alarm2)
	{
		addr = ALM1_SECONDS;
		writeReg(addr++, state.second);
	}
	else
		addr = ALM2_MINUTES;
	writeReg(addr++, state.minute);
	writeReg(addr++, state.hour);
	writeReg(addr++, state.minute);

	if(alarm2)
	{
		_alarm2Mode			= state.mode;
		_alarm2Count		= state.repeatCount;
		_alarm2Handler		= state.handler;
	}
	else
	{
		_alarmMode			= state.mode;
		_alarmCount		= state.repeatCount;
		_alarmHandler		= state.handler;
		_alarmSeconds		= state.alarmSeconds;
	}
	clearAlarmStatusHelper(alarm2);
	enableAlarmHelper(alarm2, state.enabled);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// RTC STUFFS /////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

bool FishinoRTCClass::isrunning(void) const
{
	return !(readReg(RTC_STATUS) & _BV(OSF));
}

void FishinoRTCClass::adjust(const DateTime& dt)
{
	Wire.beginTransmission(RTC_ADDRESS);
	Wire.write((uint8_t)RTC_SECONDS);
	Wire.write(bin2bcd(dt.second()));
	Wire.write(bin2bcd(dt.minute()));
	Wire.write(bin2bcd(dt.hour()));
	Wire.write(bin2bcd(dt.dayOfTheWeek()));
	Wire.write(bin2bcd(dt.day()));
	Wire.write(bin2bcd(dt.month()));
	Wire.write(bin2bcd(dt.year() - 2000));
	Wire.endTransmission();
	
	//read the status register
    uint8_t s = readReg(RTC_STATUS);
    
    //clear the Oscillator Stop Flag
    writeReg(RTC_STATUS, s & ~_BV(OSF) );
}

DateTime FishinoRTCClass::now() const
{
	Wire.beginTransmission(RTC_ADDRESS);
	Wire.write((uint8_t)RTC_SECONDS);
	Wire.endTransmission();

	Wire.requestFrom(RTC_ADDRESS, 7);
	uint8_t ss		= bcd2bin(Wire.read() & ~_BV(DS1307_CH));
	uint8_t mm		= bcd2bin(Wire.read());
	uint8_t hh		= bcd2bin(Wire.read() & ~_BV(HR1224));
	/* uint8_t wday	= */ Wire.read();
	uint8_t d		= bcd2bin(Wire.read());
	uint8_t m		= bcd2bin(Wire.read() & ~_BV(CENTURY));
	uint16_t y = bcd2bin(Wire.read()) + 2000;

	return DateTime(y, m, d, hh, mm, ss);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// ALARM1 STUFFS ///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

// set the alarm to a specific date and time
void FishinoRTCClass::setAlarm(const DateTime &dt, bool enable)
{
	_alarmSeconds = 0;
	setAlarmHelper(false, dt, ALM1_MATCH_DATE, enable);
}

// set the alarm after some seconds
// alarm can be single, repeated from 1 to 255 times or repeated forever
// repeated delays are re-set automatically from inside interrupt routine
void FishinoRTCClass::setAlarm(uint32_t seconds, bool enable)
{
	DateTime dt = now() + seconds;
	_alarmSeconds = seconds;
	setAlarmHelper(false, dt, ALM1_MATCH_DATE, enable);
}

void FishinoRTCClass::setSecondsAlarm(uint8_t second, bool enable)
{
	DateTime dt = now();
	dt.setSecond(second);
	_alarmSeconds = 0;
	setAlarmHelper(false, dt, ALM1_MATCH_SECONDS, enable);
}

void FishinoRTCClass::setMinuteAlarm(uint8_t minute, uint8_t second, bool enable)
{
	DateTime dt = now();
	dt.setSecond(second);
	dt.setMinute(minute);
	_alarmSeconds = 0;
	setAlarmHelper(false, dt, ALM1_MATCH_MINUTES, enable);
}

void FishinoRTCClass::setHourlyAlarm(uint8_t hour, uint8_t minute, uint8_t second, bool enable)
{
	DateTime dt = now();
	dt.setSecond(second);
	dt.setMinute(minute);
	dt.setHour(hour);
	_alarmSeconds = 0;
	setAlarmHelper(false, dt, ALM1_MATCH_HOURS, enable);
}

void FishinoRTCClass::setWeeklyAlarm(uint8_t weekDay, uint8_t hour, uint8_t minute, uint8_t second, bool enable)
{
	DateTime dt = now();
	dt.setSecond(second);
	dt.setMinute(minute);
	dt.setHour(hour);
	dt.setDay(weekDay);
	_alarmSeconds = 0;
	setAlarmHelper(false, dt, ALM1_MATCH_DAY, enable);
}

void FishinoRTCClass::setMonthlyAlarm(uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, bool enable)
{
	DateTime dt = now();
	dt.setSecond(second);
	dt.setMinute(minute);
	dt.setHour(hour);
	dt.setDay(day);
	_alarmSeconds = 0;
	setAlarmHelper(false, dt, ALM1_MATCH_DATE, enable);
}

// get current alarm date
// it will get NEXT FULL DATE of alarm
DateTime FishinoRTCClass::getAlarm(void) const
{
	return getAlarmHelper(false);
}

// get seconds till next alarm
uint32_t FishinoRTCClass::getAlarmSeconds(void) const
{
	TimeSpan ts = getAlarmHelper(false) - now();
	return ts.totalSeconds();
}

// set alarm mode
void FishinoRTCClass::setAlarmMode(AlarmModes mode, uint16_t rept)
{
	_alarmMode = mode;
	_alarmCount = rept;
}

// get alarm mode and repeat count
AlarmModes FishinoRTCClass::getAlarmMode(void) const
{
	return _alarmMode;
}

uint16_t FishinoRTCClass::getAlarmRepeatCount(void) const
{
	return _alarmCount;
}

// enable/disable alarm
void FishinoRTCClass::enableAlarm(void)
{
	enableAlarmHelper(false, true);
}

void FishinoRTCClass::disableAlarm(void)
{
	enableAlarmHelper(false, false);
}

bool FishinoRTCClass::isAlarmEnabled(void) const
{
	return isAlarmEnabledHelper(false);
}

// save/restore alarm state
void FishinoRTCClass::saveAlarmState(RTCAlarmState &state) const
{
	saveAlarmStateHelper(false, state);
}

void FishinoRTCClass::restoreAlarmState(RTCAlarmState const &state)
{
	restoreAlarmStateHelper(false, state);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// ALARM2 STUFFS ///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

// set the alarm to a specific date and time
void FishinoRTCClass::setAlarm2(const DateTime &dt, bool enable)
{
	setAlarmHelper(true, dt, ALM1_MATCH_DATE, enable);
}

// the alarm at some specified times
// those are triggered when the given times matches the RTC clock
void FishinoRTCClass::setMinuteAlarm2(uint8_t minute, uint8_t second, bool enable)
{
	DateTime dt = now();
	dt.setSecond(second);
	dt.setMinute(minute);
	setAlarmHelper(true, dt, ALM1_MATCH_MINUTES, enable);
}

void FishinoRTCClass::setHourlyAlarm2(uint8_t hour, uint8_t minute, uint8_t second, bool enable)
{
	DateTime dt = now();
	dt.setSecond(second);
	dt.setMinute(minute);
	dt.setHour(hour);
	setAlarmHelper(true, dt, ALM1_MATCH_HOURS, enable);
}

void FishinoRTCClass::setWeeklyAlarm2(uint8_t weekDay, uint8_t hour, uint8_t minute, uint8_t second, bool enable)
{
	DateTime dt = now();
	dt.setSecond(second);
	dt.setMinute(minute);
	dt.setHour(hour);
	dt.setDay(weekDay);
	setAlarmHelper(true, dt, ALM1_MATCH_DAY, enable);
}

void FishinoRTCClass::setMonthlyAlarm2(uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, bool enable)
{
	DateTime dt = now();
	dt.setSecond(second);
	dt.setMinute(minute);
	dt.setHour(hour);
	dt.setDay(day);
	setAlarmHelper(true, dt, ALM1_MATCH_DATE, enable);
}

// get current alarm date
DateTime FishinoRTCClass::getAlarm2(void) const
{
	return getAlarmHelper(true);
}

// get seconds till next alarm
uint32_t FishinoRTCClass::getAlarm2Seconds(void) const
{
	TimeSpan ts = getAlarmHelper(true) - now();
	return ts.totalSeconds();
}

// set alarm mode
void FishinoRTCClass::setAlarm2Mode(AlarmModes mode, uint16_t rept)
{
	_alarm2Mode = mode;
	_alarm2Count = rept;
}

// get alarm mode and repeat count
AlarmModes FishinoRTCClass::getAlarm2Mode(void) const
{
	return _alarm2Mode;
}

uint16_t FishinoRTCClass::getAlarm2RepeatCount(void) const
{
	return _alarm2Count;
}

// enable/disable alarm
void FishinoRTCClass::enableAlarm2(void)
{
	enableAlarmHelper(true, true);
}

void FishinoRTCClass::disableAlarm2(void)
{
	enableAlarmHelper(true, false);
}

bool FishinoRTCClass::isAlarm2Enabled(void) const
{
	return isAlarmEnabledHelper(true);
}

// save/restore alarm state
void FishinoRTCClass::saveAlarm2State(RTCAlarmState &state) const
{
	saveAlarmStateHelper(true, state);
}

void FishinoRTCClass::restoreAlarm2State(RTCAlarmState const &state)
{
	restoreAlarmStateHelper(true, state);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

#endif
