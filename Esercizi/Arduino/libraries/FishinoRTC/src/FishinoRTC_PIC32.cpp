//////////////////////////////////////////////////////////////////////////////////////
//						 		FishinoRTC_PIC32.cpp								//
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

#include <FishinoFlash.h>

#if defined(_FISHINO_PIC32_)

// binary/bcd conversions
static uint8_t bcd2bin(uint8_t val)
{
//	return (val >> 4) * 10 + (val & 0x0f);
	return val - 6 * (val >> 4);
}
static uint8_t bin2bcd(uint8_t val)
{
//	return ((val / 10) << 4) | (val % 10);
	return val + 6 * (val / 10);
}

// enable writing to RTC registers
static void enableRTCWrite(void)
{
	disableInterrupts();
	SYSKEY = 0x0;
	SYSKEY = 0xaa996655;
	SYSKEY = 0x556699aa;
	RTCCONbits.RTCWREN = 1;
	SYSKEY = 0x33333333;
	enableInterrupts();
}

// disable writing to RTC registers
static void disableRTCWrite(void)
{
	disableInterrupts();
	SYSKEY = 0x0;
	SYSKEY = 0xaa996655;
	SYSKEY = 0x556699aa;
	RTCCONbits.RTCWREN = 0;
	SYSKEY = 0x33333333;
	enableInterrupts();
}

// wait for sync bit to avoid rollover problems
static inline void waitSync(void)
{
	while(RTCCONbits.RTCSYNC)
		;
}

// alarm ISR handler
void FishinoRTCClass::_global_isr(void)
{
	// call internal ISR
	RTC._isr();
	
	// clear interrupt flag on exit
	IFS0CLR = _IFS0_RTCCIF_MASK;
}

// the internal ISR
void FishinoRTCClass::_isr(void)
{
	// if we've set an handler, just call it
	if (_alarmHandler)
		_alarmHandler();
	
	// here we must reset the alarm time
	// if we use the setAlarm(seconds) mode
	if(_alarmSeconds && (RTCALRMbits.CHIME || RTCALRMbits.ARPT))
		setAlarm(_alarmSeconds);
}


// constructor
FishinoRTCClass::FishinoRTCClass()
{
	// initialize the RTC
	// something should be done inside the bootloader, maybe

	// enable register write
	enableRTCWrite();
	
	// do not stop clock on idle modes
	RTCCONbits.SIDL = 0;
	
	// enable RTC clock
	RTCCONbits.RTCCLKON = 1;
	
	// no clock output
	RTCCONbits.RTCOE = 0;

	// disable register write
	disableRTCWrite();
	
	// reset the interrupt handler
	_alarmHandler = NULL;
	
	// connect the RTC interrupt to our ISR handler
	disableInterrupts();
	setIntVector(_RTCC_VECTOR, _global_isr);
	setIntPriority(_RTCC_VECTOR, _RTCC_IPL_IPC, _RTCC_SPL_IPC);
	clearIntFlag(_RTCC_IRQ);
	setIntEnable(_RTCC_IRQ);
	enableInterrupts();
}

// destructor
FishinoRTCClass::~FishinoRTCClass()
{
}

bool FishinoRTCClass::isrunning(void) const
{
	return RTCCONbits.ON;
}

void FishinoRTCClass::adjust(const DateTime& dt)
{
	// PIC32 RTC year can be from 2000 to 2099
	// we just truncate it if out of range
	uint8_t year;
	if(dt.year() < 2000)
		year = 0;
	else if(dt.year() > 2099)
		year = 99;
	else
		year = (uint8_t)(dt.year() - 2000);
	year = bin2bcd(year);
	
	uint8_t month	= bin2bcd(dt.month());
	uint8_t day		= bin2bcd(dt.day());
	uint8_t hour	= bin2bcd(dt.hour());
	uint8_t minute	= bin2bcd(dt.minute());
	uint8_t second	= bin2bcd(dt.second());
	
	// it seems that fields must be set at once and not
	// by single field... otherwise time gets bad!
	uint32_t date =
		((uint32_t)year)	<< 24 |
		((uint32_t)month)	<< 16 |
		((uint32_t)day)		<<  8 |
		(uint32_t)dt.dayOfTheWeek()
	;

	uint32_t time =
		((uint32_t)hour)	<< 24 |
		((uint32_t)minute)	<< 16 |
		((uint32_t)second)	<<  8
	;

	enableRTCWrite(); 

	waitSync();
	RTCTIME = time;
	RTCDATE = date;

	// enable the RTC, if not already enabled
	RTCCONbits.ON = 1;
	
	disableRTCWrite(); 
}

DateTime FishinoRTCClass::now() const
{
	waitSync();
	uint8_t day		= bcd2bin((RTCDATEbits.DAY10 << 4) | RTCDATEbits.DAY01);
	uint8_t month	= bcd2bin((RTCDATEbits.MONTH10 << 4) | RTCDATEbits.MONTH01);
	uint16_t year	= bcd2bin((RTCDATEbits.YEAR10 << 4) | RTCDATEbits.YEAR01) + 2000;
	uint8_t second	= bcd2bin((RTCTIMEbits.SEC10 << 4) | RTCTIMEbits.SEC01);
	uint8_t minute	= bcd2bin((RTCTIMEbits.MIN10 << 4) | RTCTIMEbits.MIN01);
	uint8_t hour	= bcd2bin((RTCTIMEbits.HR10 << 4) | RTCTIMEbits.HR01);
	
	return DateTime(year, month, day, hour, minute, second);
}

// enable RTC
void FishinoRTCClass::enable(void)
{
	enableRTCWrite();
	RTCCONbits.ON = 1;
	disableRTCWrite();
}

// disable RTC
void FishinoRTCClass::disable(void)
{
	enableRTCWrite();
	RTCCONbits.ON = 0;
	disableRTCWrite();
}

// set the alarm to a specific date
void FishinoRTCClass::setAlarm(const DateTime &dt, bool enable)
{
	uint8_t month	= bin2bcd(dt.month());
	uint8_t day		= bin2bcd(dt.day());
	uint8_t hour	= bin2bcd(dt.hour());
	uint8_t minute	= bin2bcd(dt.minute());
	uint8_t second	= bin2bcd(dt.second());
	
	enableRTCWrite(); 
	
	// disable the alarm before starting
	RTCALRMbits.ALRMEN = 0;

	waitSync();
	ALRMDATEbits.WDAY01	= dt.dayOfTheWeek() - 1;
	ALRMDATEbits.DAY10	= day >> 4;
	ALRMDATEbits.DAY01	= day & 0x0f;
	ALRMDATEbits.MONTH10	= month >> 4;
	ALRMDATEbits.MONTH01	= month & 0x0f;

	ALRMTIMEbits.SEC10	= second >> 4;
	ALRMTIMEbits.SEC01	= second & 0x0f;
	ALRMTIMEbits.MIN10	= minute >> 4;
	ALRMTIMEbits.MIN01	= minute & 0x0f;
	ALRMTIMEbits.HR10	= hour >> 4;
	ALRMTIMEbits.HR01	= hour & 0x0f;
	
	// setting the whole stuff we assume that we want to
	// check ALL fields for alarm
	RTCALRMbits.AMASK = 0b1001;
	
	// re-enable alarm if requested
	if(enable)
	{
		setIntEnable(_RTCC_IRQ);
		clearIntFlag(_RTCC_IRQ);
		RTCALRMbits.ALRMEN = 1;
	}
	
	disableRTCWrite(); 
	_alarmSeconds = 0;
}

// set the alarm after some seconds
// alarm can be single, repeated from 1 to 255 times or repeated forever
void FishinoRTCClass::setAlarm(uint32_t seconds, bool enable)
{
	// first we shall check the time and build the correct mask
	// to do that we read current date, add requested time and
	// check which values change. If time is larger than 1 year
	// we simply do nothing, as it's invalid
	// for repeated, timed alarms we shall re-set the time from
	// inside the interrupt handler... ugly method, indeed
	if(seconds > 366L * 24L * 60L * 60L)
	{
		disableAlarm();
		return;
	}
	DateTime next = now() + TimeSpan(seconds);
	setAlarm(next, enable);
	_alarmSeconds = seconds;
}

// the alarm at some specified times
// those are triggered when the given times matches the RTC clock
void FishinoRTCClass::setSecondsAlarm(uint8_t second, bool enable)
{
	second = bin2bcd(second);
	
	enableRTCWrite(); 
	
	// disable the alarm before starting
	RTCALRMbits.ALRMEN = 0;
	
	// setup seconds alarm
	waitSync();
	ALRMTIMEbits.SEC01	= second & 0x0f;
	ALRMTIMEbits.SEC10	= second >> 4;

	// set mask for seconds alarm
	RTCALRMbits.AMASK = 0b0001;

	// re-enable alarm if requested
	if(enable)
	{
		setIntEnable(_RTCC_IRQ);
		clearIntFlag(_RTCC_IRQ);
		RTCALRMbits.ALRMEN = 1;
	}
	
	disableRTCWrite(); 
	_alarmSeconds = 0;
}

void FishinoRTCClass::setMinuteAlarm(uint8_t minute, uint8_t second, bool enable)
{
	minute	= bin2bcd(minute);
	second	= bin2bcd(second);
	
	enableRTCWrite(); 
	
	// disable the alarm before starting
	RTCALRMbits.ALRMEN = 0;
	
	// setup minutes alarm
	waitSync();
	ALRMTIMEbits.SEC01	= second & 0x0f;
	ALRMTIMEbits.SEC10	= second >> 4;
	ALRMTIMEbits.MIN01	= minute & 0x0f;
	ALRMTIMEbits.MIN10	= minute >> 4;

	// set mask for minutes alarm
	RTCALRMbits.AMASK = 0b0011;

	// re-enable alarm if requested
	if(enable)
	{
		setIntEnable(_RTCC_IRQ);
		clearIntFlag(_RTCC_IRQ);
		RTCALRMbits.ALRMEN = 1;
	}
	
	disableRTCWrite(); 
	_alarmSeconds = 0;
}

void FishinoRTCClass::setHourlyAlarm(uint8_t hour, uint8_t minute, uint8_t second, bool enable)
{
	hour	= bin2bcd(hour);
	minute	= bin2bcd(minute);
	second	= bin2bcd(second);
	
	enableRTCWrite(); 
	
	// disable the alarm before starting
	RTCALRMbits.ALRMEN = 0;
	
	// setup hourly alarm
	waitSync();
	ALRMTIMEbits.SEC01	= second & 0x0f;
	ALRMTIMEbits.SEC10	= second >> 4;
	ALRMTIMEbits.MIN01	= minute & 0x0f;
	ALRMTIMEbits.MIN10	= minute >> 4;
	ALRMTIMEbits.HR01	= hour & 0x0f;
	ALRMTIMEbits.HR10	= hour >> 4;

	// set mask for hourly alarm
	RTCALRMbits.AMASK = 0b0101;

	// re-enable alarm if requested
	if(enable)
	{
		setIntEnable(_RTCC_IRQ);
		clearIntFlag(_RTCC_IRQ);
		RTCALRMbits.ALRMEN = 1;
	}
	
	disableRTCWrite(); 
	_alarmSeconds = 0;
}

void FishinoRTCClass::setWeeklyAlarm(uint8_t weekDay, uint8_t hour, uint8_t minute, uint8_t second, bool enable)
{
	hour	= bin2bcd(hour);
	minute	= bin2bcd(minute);
	second	= bin2bcd(second);
	
	enableRTCWrite(); 
	
	// disable the alarm before starting
	RTCALRMbits.ALRMEN = 0;
	
	// setup weekly alarm
	waitSync();
	ALRMTIMEbits.SEC01	= second & 0x0f;
	ALRMTIMEbits.SEC10	= second >> 4;
	ALRMTIMEbits.MIN01	= minute & 0x0f;
	ALRMTIMEbits.MIN10	= minute >> 4;
	ALRMTIMEbits.HR01	= hour & 0x0f;
	ALRMTIMEbits.HR10	= hour >> 4;
	ALRMDATEbits.WDAY01	= weekDay - 1;

	// set mask for weekly alarm
	RTCALRMbits.AMASK = 0b0111;

	// re-enable alarm if requested
	if(enable)
	{
		setIntEnable(_RTCC_IRQ);
		clearIntFlag(_RTCC_IRQ);
		RTCALRMbits.ALRMEN = 1;
	}
	
	disableRTCWrite(); 
	_alarmSeconds = 0;
}

void FishinoRTCClass::setMonthlyAlarm(uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, bool enable)
{
	day		= bin2bcd(day);
	hour	= bin2bcd(hour);
	minute	= bin2bcd(minute);
	second	= bin2bcd(second);
	
	enableRTCWrite(); 
	
	// disable the alarm before starting
	RTCALRMbits.ALRMEN = 0;
	
	// setup monthly alarm
	waitSync();
	ALRMTIMEbits.SEC01	= second & 0x0f;
	ALRMTIMEbits.SEC10	= second >> 4;
	ALRMTIMEbits.MIN01	= minute & 0x0f;
	ALRMTIMEbits.MIN10	= minute >> 4;
	ALRMTIMEbits.HR01	= hour & 0x0f;
	ALRMTIMEbits.HR10	= hour >> 4;
	ALRMDATEbits.DAY01	= day & 0x0f;
	ALRMDATEbits.DAY10	= day >> 4;

	// set mask for monthly alarm
	RTCALRMbits.AMASK = 0b1000;

	// re-enable alarm if requested
	if(enable)
	{
		setIntEnable(_RTCC_IRQ);
		clearIntFlag(_RTCC_IRQ);
		RTCALRMbits.ALRMEN = 1;
	}
	
	disableRTCWrite(); 
	_alarmSeconds = 0;
}

void FishinoRTCClass::setYearlyAlarm(uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, bool enable)
{
	month	= bin2bcd(month);
	day		= bin2bcd(day);
	hour	= bin2bcd(hour);
	minute	= bin2bcd(minute);
	second	= bin2bcd(second);
	
	enableRTCWrite(); 
	
	// disable the alarm before starting
	RTCALRMbits.ALRMEN = 0;
	
	// setup yearly alarm
	waitSync();
	ALRMTIMEbits.SEC01	= second & 0x0f;
	ALRMTIMEbits.SEC10	= second >> 4;
	ALRMTIMEbits.MIN01	= minute & 0x0f;
	ALRMTIMEbits.MIN10	= minute >> 4;
	ALRMTIMEbits.HR01	= hour & 0x0f;
	ALRMTIMEbits.HR10	= hour >> 4;
	ALRMDATEbits.DAY01	= day & 0x0f;
	ALRMDATEbits.DAY10	= day >> 4;
	ALRMDATEbits.MONTH01	= month & 0x0f;
	ALRMDATEbits.MONTH10	= month >> 4;

	// set mask for yearly alarm
	RTCALRMbits.AMASK = 0b1001;

	// re-enable alarm if requested
	if(enable)
	{
		setIntEnable(_RTCC_IRQ);
		clearIntFlag(_RTCC_IRQ);
		RTCALRMbits.ALRMEN = 1;
	}
	
	disableRTCWrite(); 
	_alarmSeconds = 0;
}

// get current alarm date
DateTime FishinoRTCClass::getAlarm(void) const
{
	waitSync();
	uint8_t day		= bcd2bin((ALRMDATEbits.DAY10 << 4) | ALRMDATEbits.DAY01);
	uint8_t month	= bcd2bin((ALRMDATEbits.MONTH10 << 4) | ALRMDATEbits.MONTH01);
	
	// note : RTC alarm are for 1 year period, so we get the year from current date
	// is it not meaningful anyways
	uint16_t year	= bcd2bin((RTCDATEbits.YEAR10 << 4) | RTCDATEbits.YEAR01) + 2000;
	
	uint8_t second	= bcd2bin((ALRMTIMEbits.SEC10 << 4) | ALRMTIMEbits.SEC01);
	uint8_t minute	= bcd2bin((ALRMTIMEbits.MIN10 << 4) | ALRMTIMEbits.MIN01);
	uint8_t hour	= bcd2bin((ALRMTIMEbits.HR10 << 4) | ALRMTIMEbits.HR01);
	
	return DateTime(year, month, day, hour, minute, second);
}

// get seconds till next alarm
uint32_t FishinoRTCClass::getAlarmSeconds(void) const
{
	TimeSpan s = getAlarm() - now();
	return s.totalSeconds();
}

// set alarm mode
void FishinoRTCClass::setAlarmMode(AlarmModes mode, uint16_t rept)
{
	switch(mode)
	{
		case AlarmModes::ALARM_ONCE:
			RTCALRMbits.CHIME = 0;
			RTCALRMbits.ARPT = 0;
		default:
			break;
		case AlarmModes::ALARM_REPEAT_N :
			RTCALRMbits.CHIME = 0;
			RTCALRMbits.ARPT = (uint8_t)(rept - 1);
			break;
		case AlarmModes::ALARM_REPEAT_FOREVER :
			RTCALRMbits.CHIME = 1;
			RTCALRMbits.ARPT = 1;
			break;
	}
}

// get alarm mode and repeat count
AlarmModes FishinoRTCClass::getAlarmMode(void) const
{
	if(RTCALRMbits.CHIME)
		return AlarmModes::ALARM_REPEAT_FOREVER;
	if(!RTCALRMbits.ARPT)
		return AlarmModes::ALARM_ONCE;
	return AlarmModes::ALARM_REPEAT_N;
}

uint16_t FishinoRTCClass::getAlarmRepeatCount(void) const
{
	if(RTCALRMbits.CHIME)
		return (uint16_t)-1;
	uint16_t ret = RTCALRMbits.ARPT;
	return ret  + 1;
}

// enable/disable alarm
void FishinoRTCClass::enableAlarm(void)
{
	disableInterrupts();

	SYSKEY = 0x0;
	SYSKEY = 0xaa996655;
	SYSKEY = 0x556699aa;
	RTCCONbits.RTCWREN = 1;

	RTCALRMbits.ALRMEN = 1;
	setIntEnable(_RTCC_IRQ);
	clearIntFlag(_RTCC_IRQ);
	RTCCONbits.RTCWREN = 0;
	
	SYSKEY = 0x33333333;
	enableInterrupts();
}

void FishinoRTCClass::disableAlarm(void)
{
	disableInterrupts();

	SYSKEY = 0x0;
	SYSKEY = 0xaa996655;
	SYSKEY = 0x556699aa;
	RTCCONbits.RTCWREN = 1;

	RTCALRMbits.ALRMEN = 0;
	clearIntEnable(_RTCC_IRQ);
	clearIntFlag(_RTCC_IRQ);
	RTCCONbits.RTCWREN = 0;
	
	SYSKEY = 0x33333333;
	enableInterrupts();
}

bool FishinoRTCClass::isAlarmEnabled(void) const
{
	return RTCALRMbits.ALRMEN;
}

// save/restore alarm state
void FishinoRTCClass::saveAlarmState(RTCAlarmState &state) const
{
	state.enabled		= RTCALRMbits.ALRMEN;
	state.mode			= getAlarmMode();
	state.amask			= RTCALRMbits.AMASK;
	state.repeatCount	= RTCALRMbits.ARPT;
	state.weekDay		= ALRMDATEbits.WDAY01;
	state.month			= bcd2bin((ALRMDATEbits.MONTH10 << 4) | ALRMDATEbits.MONTH01);
	state.day			= bcd2bin((ALRMDATEbits.DAY10 << 4) | ALRMDATEbits.DAY01);
	state.hour			= bcd2bin((ALRMTIMEbits.HR10 << 4) | ALRMTIMEbits.HR01);
	state.minute		= bcd2bin((ALRMTIMEbits.MIN10 << 4) | ALRMTIMEbits.MIN01);
	state.second		= bcd2bin((ALRMTIMEbits.SEC10 << 4) | ALRMTIMEbits.SEC01);
	state.handler		= _alarmHandler;
	state.alarmSeconds	= _alarmSeconds;
}

void FishinoRTCClass::restoreAlarmState(RTCAlarmState const &state)
{
	uint8_t month	= bin2bcd(state.month);
	uint8_t day		= bin2bcd(state.day);
	uint8_t hour	= bin2bcd(state.hour);
	uint8_t minute	= bin2bcd(state.minute);
	uint8_t second	= bin2bcd(state.second);

	enableRTCWrite(); 
	
	// disable the alarm before starting
	RTCALRMbits.ALRMEN = 0;

	waitSync();
	ALRMDATEbits.WDAY01		= state.weekDay;
	ALRMDATEbits.DAY01		= day & 0x0f;
	ALRMDATEbits.DAY10		= day >> 4;
	ALRMDATEbits.MONTH01	= month & 0x0f;
	ALRMDATEbits.MONTH10	= month >> 4;

	ALRMTIMEbits.SEC01		= second & 0x0f;
	ALRMTIMEbits.SEC10		= second >> 4;
	ALRMTIMEbits.MIN01		= minute & 0x0f;
	ALRMTIMEbits.MIN10		= minute >> 4;
	ALRMTIMEbits.HR01		= hour & 0x0f;
	ALRMTIMEbits.HR10		= hour >> 4;
	
	// setting the whole stuff we assume that we want to
	// check ALL fields for alarm
	RTCALRMbits.AMASK		= state.amask;
	
	// re-enable alarm if requested
	if(state.enabled)
	{
		setIntEnable(_RTCC_IRQ);
		clearIntFlag(_RTCC_IRQ);
		RTCALRMbits.ALRMEN = 1;
	}
	
	disableRTCWrite(); 

	_alarmHandler			= state.handler;
	_alarmSeconds			= state.alarmSeconds;
}

#endif
