//////////////////////////////////////////////////////////////////////////////////////
//						 			FishinoRTC.h									//
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
#ifndef __FISHINORTC_FISHINORTC_H
#define __FISHINORTC_FISHINORTC_H

#include <Arduino.h>

#if defined(_FISHINO_UNO_R2_) || defined(_FISHINO_MEGA_R2_)
	#define _HAS_DS3231_
#elif defined(_FISHINO_UNO_) || defined(_FISHINO_MEGA_) || defined(_FISHINO_GUPPY_)
	#define _HAS_DS1307_
#elif defined(_FISHINO_PIC32_)
	#define _HAS_PIC32_
#else
	#error "Unsupported board"
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////

// Timespan which can represent changes in time with seconds accuracy.
class TimeSpan
{
	protected:
		int32_t _seconds;

	public:
		TimeSpan(int32_t seconds = 0);
		TimeSpan(int32_t days, int8_t hours, int8_t minutes, int8_t seconds);
		
		int16_t days() const
		{
			return _seconds / 86400L;
		}
		int8_t  hours() const
		{
			return _seconds / 3600 % 24;
		}
		int8_t  minutes() const
		{
			return _seconds / 60 % 60;
		}
		int8_t  seconds() const
		{
			return _seconds % 60;
		}
		int32_t totalSeconds() const
		{
			return _seconds;
		}

		TimeSpan operator+(const TimeSpan& right);
		TimeSpan operator-(const TimeSpan& right);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////

// Simple general-purpose date/time class (no TZ / DST / leap second handling!)
class DateTime
{
	public:
		
		enum EPOCH
		{
			EPOCH_2000,	// seconds since 1/1/2000
			EPOCH_UNIX,	// seconds since 1/1/1970
			EPOCH_NTP	// seconds since 1/1/1900
		};
		
	protected:
		
		// date/time is stored y-m-d : hh-mm-ss format
		uint16_t _year;
		uint8_t  _month;
		uint8_t  _day;

		uint8_t  _hour;
		uint8_t  _minute;
		uint8_t  _second;
		
	public:
		// create from seconds and epoch
		DateTime(uint32_t tim, EPOCH epochType = EPOCH_2000);
		
		// create from date and time
		DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour = 0, uint8_t min = 0, uint8_t sec = 0);
		
		// create from string
		DateTime(const char* date, const char* time);
		DateTime(const __FlashStringHelper* date, const __FlashStringHelper* time);
		
		// create from various epochs
		static DateTime FromSecondsTime(uint32_t tim)	{ return DateTime(tim, EPOCH_2000); }
		static DateTime FromUnixTime(uint32_t tim)		{ return DateTime(tim, EPOCH_UNIX); }
		static DateTime FromNtpTime(uint32_t tim)		{ return DateTime(tim, EPOCH_NTP); }
		
		uint16_t year() const	{ return _year;		}
		uint8_t month() const	{ return _month;	}
		uint8_t day() const		{ return _day;		}
		uint8_t hour() const	{ return _hour;		}
		uint8_t minute() const	{ return _minute;	}
		uint8_t second() const	{ return _second;	}
		uint8_t dayOfTheWeek() const;
		String dayOfTheWeekStr() const;
		
		// set parts of DateTime
		void setYear(uint16_t y)	{ _year		= y; }
		void setMonth(uint8_t m)	{ _month	= m; }
		void setDay(uint8_t d)		{ _day		= d; }
		void setHour(uint8_t h)		{ _hour		= h; }
		void setMinute(uint8_t m)	{ _minute	= m; }
		void setSecond(uint8_t s)	{ _second	= s; }

		// get elapsed seconds from various epochs

		// 32-bit times as seconds since 1/1/2000
		uint32_t getSecondsTime(void) const;
		
		// 32-bit times as seconds since 1/1/1970
		uint32_t getUnixTime(void) const;
		
		// time from NTP epoch (1/1/1900)
		uint32_t getNtpTime(void) const;

		DateTime operator+(const TimeSpan& span);
		DateTime operator-(const TimeSpan& span);
		TimeSpan operator-(const DateTime& right);
		
		// printing for debug purposes
		void print(Stream &s);

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(_HAS_PIC32_) || defined(_HAS_DS3231_)
enum class AlarmModes {
	ALARM_ONCE,
	ALARM_REPEAT_N,
	ALARM_REPEAT_FOREVER
};

// the alarm handler
typedef void (*RTCAlarmHandler)(void);

// this struct is used to store/restore alarm state
// we use this from inside FishinoLowPower library
// to use RTC alarm to wake up the board without
// disturbing normal alarm processes
struct RTCAlarmState
{
	bool enabled;
	AlarmModes		mode;
	uint8_t			repeatCount;

#if defined(_HAS_PIC32_)
	uint8_t			amask;
	uint8_t			weekDay;
	uint8_t			month;
#endif

	uint8_t			day;
	uint8_t			hour;
	uint8_t			minute;
	uint8_t			second;
	RTCAlarmHandler	handler;
	uint32_t		alarmSeconds;
};

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _FISHINO_UNO_R2_
extern "C" void PCINT2_vect(void);
#endif

class FishinoRTCClass
{
	friend FishinoRTCClass &__getFishinoRTC(void);
	
	private:

		// constructor
		// do not allow construction
		FishinoRTCClass();
		
		// do not allow copy either
		FishinoRTCClass(FishinoRTCClass const &) = delete;
		
#if defined(_HAS_PIC32_)

		// the global ISR
		static void _global_isr(void) __attribute__((interrupt(), nomips16));
		
		// the internal ISR
		void _isr(void);

		// the alarm handler
		RTCAlarmHandler _alarmHandler;

		// the 'seconds' alarm time, used to re-set
		// the timer from inside interrupt handler
		uint32_t _alarmSeconds;
		
#endif

#ifdef _HAS_DS3231_

		// the internal ISR
		friend void PCINT2_vect(void);
		friend void _ALARM_ISR(void);
		void _isr(void);

		// the alarm mode
		AlarmModes _alarmMode, _alarm2Mode;
		
		// the alarm counts
		uint8_t _alarmCount, _alarm2Count;
		
		// the alarm handler
		RTCAlarmHandler _alarmHandler, _alarm2Handler;

		// the 'seconds' alarm time, used to re-set
		// the timer from inside interrupt handler
		uint32_t _alarmSeconds;
		
		// enable/disable/request helpers
		void enableAlarmHelper(bool alarm2, bool enable);
		bool isAlarmEnabledHelper(bool alarm2) const;
		void clearAlarmStatusHelper(bool alarm2);
		
		// set alarm - helper function
		void setAlarmHelper(bool alarm2, DateTime const &dt, uint8_t flags, bool enable);
		
		// get alarm - helper function
		DateTime getAlarmHelper(bool alarm2) const;
		
		// save/restore alarm state helpers
		void saveAlarmStateHelper(bool alarm2, RTCAlarmState &state) const;
		void restoreAlarmStateHelper(bool alarm2, RTCAlarmState const &state);

#endif

	protected:

	public:

		// destructor
		~FishinoRTCClass();

		// check if RTC is running
		bool isrunning(void) const;
		
		// set RTC time/date
		void adjust(const DateTime& dt);
		inline void set(const DateTime &dt) { adjust(dt); }
		
#ifdef _HAS_PIC32_

		// enable RTC
		void enable(void);
		
		// disable RTC
		void disable(void);
		
#endif

#if defined(_HAS_PIC32_) || defined(_HAS_DS3231_)

		// set the alarm to a specific date and time
		void setAlarm(const DateTime &dt, bool enable = true);
		
		// set the alarm after some seconds
		// alarm can be single, repeated from 1 to 255 times or repeated forever
		// repeated delays are re-set automatically from inside interrupt routine
		void setAlarm(uint32_t seconds, bool enable = true);
		
		// the alarm at some specified times
		// those are triggered when the given times matches the RTC clock
		void setSecondsAlarm(uint8_t seconds, bool enable = true);
		void setMinuteAlarm(uint8_t minute, uint8_t second, bool enable = true);
		void setHourlyAlarm(uint8_t hour, uint8_t minute, uint8_t second, bool enable = true);
		void setWeeklyAlarm(uint8_t weekDay, uint8_t hour, uint8_t minute, uint8_t second, bool enable = true);
		void setMonthlyAlarm(uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, bool enable = true);

// DS3231 has no full-year alarm
#ifdef _HAS_PIC32_
		void setYearlyAlarm(uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, bool enable = true);
#endif
		
		// get current alarm date
		DateTime getAlarm(void) const;
		
		// get seconds till next alarm
		uint32_t getAlarmSeconds(void) const;
		
		// set alarm mode
		void setAlarmMode(AlarmModes mode, uint16_t rept = 1);
		
		// get alarm mode and repeat count
		AlarmModes getAlarmMode(void) const;
		uint16_t getAlarmRepeatCount(void) const;
		
		// enable/disable alarm
		void enableAlarm(void);
		void disableAlarm(void);
		bool isAlarmEnabled(void) const;
		
		// set alarm handler
		void setAlarmHandler(RTCAlarmHandler handler)
			{ _alarmHandler = handler; }

		// get alarm handler
		RTCAlarmHandler getAlarmHandler(void) const
			{ return _alarmHandler; }
		
		// clear alarm handler
		void clearAlarmHandler(void)
			{ _alarmHandler = NULL; }
			
		// save/restore alarm state
		void saveAlarmState(RTCAlarmState &state) const;
		void restoreAlarmState(RTCAlarmState const &state);

#endif

// DS3231 has 2 alarms
#if defined(_HAS_DS3231_)

		// set the alarm to a specific date and time
		void setAlarm2(const DateTime &dt, bool enable = true);
		
		// the alarm at some specified times
		// those are triggered when the given times matches the RTC clock
		void setMinuteAlarm2(uint8_t minute, uint8_t second, bool enable = true);
		void setHourlyAlarm2(uint8_t hour, uint8_t minute, uint8_t second, bool enable = true);
		void setWeeklyAlarm2(uint8_t weekDay, uint8_t hour, uint8_t minute, uint8_t second, bool enable = true);
		void setMonthlyAlarm2(uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, bool enable = true);

		// get current alarm date
		DateTime getAlarm2(void) const;
		
		// get seconds till next alarm
		uint32_t getAlarm2Seconds(void) const;
		
		// set alarm mode
		void setAlarm2Mode(AlarmModes mode, uint16_t rept = 1);
		
		// get alarm mode and repeat count
		AlarmModes getAlarm2Mode(void) const;
		uint16_t getAlarm2RepeatCount(void) const;
		
		// enable/disable alarm
		void enableAlarm2(void);
		void disableAlarm2(void);
		bool isAlarm2Enabled(void) const;
		
		// set alarm handler
		void setAlarm2Handler(RTCAlarmHandler handler)
			{ _alarm2Handler = handler; }

		// get alarm handler
		RTCAlarmHandler getAlarm2Handler(void) const
			{ return _alarm2Handler; }
		
		// clear alarm handler
		void clearAlarm2Handler(void)
			{ _alarm2Handler = NULL; }
			
		// save/restore alarm state
		void saveAlarm2State(RTCAlarmState &state) const;
		void restoreAlarm2State(RTCAlarmState const &state);

#endif
		
		// get current date/time
		DateTime now(void) const;
		inline DateTime get(void) const { return now(); }
		
		// some DS1307-only functions to handle chip non volatile RAM
#ifdef _HAS_DS1307_
		void readnvram(uint8_t* buf, uint8_t size, uint8_t address) const;
		uint8_t readnvram(uint8_t address) const;
		void writenvram(uint8_t address, uint8_t* buf, uint8_t size);
		void writenvram(uint8_t address, uint8_t data);
		
#endif

		// dummy, just for some sketch that use it
		inline void begin(void) {};
};

extern FishinoRTCClass &__getFishinoRTC(void);

#define RTC __getFishinoRTC()



#endif
