#include "FishinoRTC.h"

#include <FishinoFlash.h>

#define DEBUG_LEVEL_INFO
#include <FishinoDebug.h>

#define UNIX_TO_NTP_TIME	2208988800L
#define E2000_TO_UNIX_TIME	 946684800L
#define E2000_TO_NTP_TIME	3155673600L

// the days per month (non-leap years...)
const uint8_t _daysInMonth [] PROGMEM = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

FlashStringArray _daysOfTheWeek = FStringArray(
			"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
		);

// Rata Die day one is 0001-01-01
static uint32_t Date2RD(int32_t y, int32_t m, int32_t d)
{
	if (m < 3)
		y--, m += 12;
	return 365 * y + y / 4 - y / 100 + y / 400 + (153 * m - 457) / 5 + d - 306;
}

// convert from Rata Die to year/month/day
static void RD2Date(uint32_t rd, uint16_t &year, uint8_t &month, uint8_t &day)
{
	// calculation constants
	const int32_t y = 4716;
	const int32_t j = 1401;
	const int32_t m = 2;
	const int32_t n = 12;
	const int32_t r = 4;
	const int32_t p = 1461;
	const int32_t v = 3;
	const int32_t u = 5;
	const int32_t s = 153;
	const int32_t w = 2;
	const int32_t B = 274277;
	const int32_t C = -38;

	// go to julian day
	uint32_t J = rd + 1721425;
	
	int32_t  f = J + j + (((4 * J + B) / 146097) * 3) / 4 + C;
	int32_t  e = r * f + v;
	int32_t  g = (e % p) / r;
	int32_t  h = u * g + w;
	day = (uint8_t)((h % s) / u + 1);
	month = (uint8_t)((h / s + m) % n + 1);
	year = (uint16_t)((e / p) - y + (n + m - month) / n); 
}

// number of days since 1900/01/01
static uint32_t date2NtpDays(uint16_t y, uint8_t m, uint8_t d)
{
	return Date2RD(y, m, d) - Date2RD(1900, 1, 1);
}

static uint32_t time2long(uint32_t days, uint8_t h, uint8_t m, uint8_t s)
{
	return ((days * 24UL + h) * 60U + m) * 60U + s;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

TimeSpan::TimeSpan(int32_t seconds):
		_seconds(seconds)
{}

TimeSpan::TimeSpan(int32_t days, int8_t hours, int8_t minutes, int8_t seconds):
		_seconds((int32_t)days*86400L + (int32_t)hours*3600 + (int32_t)minutes*60 + seconds)
{}

TimeSpan TimeSpan::operator+(const TimeSpan& right)
{
	return TimeSpan(_seconds + right._seconds);
}

TimeSpan TimeSpan::operator-(const TimeSpan& right)
{
	return TimeSpan(_seconds -right._seconds);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

// constructor
DateTime::DateTime(uint32_t tim, EPOCH epochType)
{
	// convert time to NTP one
	int64_t epoch = tim;
	switch (epochType)
	{
		case EPOCH_2000:	// seconds since 1/1/2000
			epoch += E2000_TO_NTP_TIME;
			break;

		case EPOCH_UNIX:	// seconds since 1/1/1970
			epoch += UNIX_TO_NTP_TIME;
			break;

		case EPOCH_NTP:		// seconds since 1/1/1900
		default:
			break;
	}

	// and now separate it
	_second = epoch % 60;
	epoch /= 60;

	_minute = epoch % 60;
	epoch /= 60;

	_hour = epoch % 24;
	int32_t days = epoch / 24;
	
	// now we've got days from unix epoch
	// we shall go to RD one
	days += Date2RD(1900, 1, 1) - Date2RD(1, 1, 1) + 1;

	// and now go to gregorian date
	RD2Date(days, _year, _month, _day);
}

// create from date and time
DateTime::DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec)
{
	_year		= year;
	_month		= month;
	_day		= day;
	_hour		= hour;
	_minute		= min;
	_second		= sec;
}

// utility for next 2 constructors
static uint8_t conv2d(const char* p)
{
	uint8_t v = 0;
	if ('0' <= *p && *p <= '9')
		v = *p - '0';
	return 10 * v + *++p - '0';
}

// create from string
// useful to create from compiler macro
// DateTime now(__DATE__, __TIME__);
DateTime::DateTime(const char* date, const char* time)
{
	// sample input: date = "Dec 26 2009", time = "12:34:56"
	_year = conv2d(date + 9) + 2000;
	// Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
	switch (date[0])
	{
		case 'J':
			_month = date[1] == 'a' ? 1 : (date[2] == 'n' ? 6 : 7);
			break;
		case 'F':
			_month = 2;
			break;
		case 'A':
			_month = date[2] == 'r' ? 4 : 8;
			break;
		case 'M':
			_month = date[2] == 'r' ? 3 : 5;
			break;
		case 'S':
			_month = 9;
			break;
		case 'O':
			_month = 10;
			break;
		case 'N':
			_month = 11;
			break;
		case 'D':
			_month = 12;
			break;
	}
	_day	= conv2d(date + 4);
	_hour	= conv2d(time);
	_minute	= conv2d(time + 3);
	_second	= conv2d(time + 6);
}

DateTime::DateTime(const __FlashStringHelper* date, const __FlashStringHelper* time)
{
	// sample input: date = "Dec 26 2009", time = "12:34:56"
	char buff[11];
	memcpy_P(buff, date, 11);
	_year = conv2d(buff + 9) + 2000;
	// Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
	switch (buff[0])
	{
		case 'J':
			_month = buff[1] == 'a' ? 1 : (buff[2] == 'n' ? 6 : 7);
			break;
		case 'F':
			_month = 2;
			break;
		case 'A':
			_month = buff[2] == 'r' ? 4 : 8;
			break;
		case 'M':
			_month = buff[2] == 'r' ? 3 : 5;
			break;
		case 'S':
			_month = 9;
			break;
		case 'O':
			_month = 10;
			break;
		case 'N':
			_month = 11;
			break;
		case 'D':
			_month = 12;
			break;
	}
	_day	= conv2d(buff + 4);
	memcpy_P(buff, time, 8);
	_hour	= conv2d(buff);
	_minute	= conv2d(buff + 3);
	_second	= conv2d(buff + 6);
}

uint8_t DateTime::dayOfTheWeek() const
{
	uint32_t day = date2NtpDays(_year, _month, _day);

	return (day + 1) % 7;
}


String DateTime::dayOfTheWeekStr() const
{
	return String(_daysOfTheWeek[dayOfTheWeek()]);
}

// get elapsed seconds from various epochs

// 32-bit times as seconds since 1/1/2000
uint32_t DateTime::getSecondsTime(void) const
{
	return getNtpTime() - E2000_TO_NTP_TIME;
}
// 32-bit times as seconds since 1/1/1970
uint32_t DateTime::getUnixTime(void) const
{
	return getNtpTime() - UNIX_TO_NTP_TIME;
}

// time from NTP epoch (1/1/1900)
uint32_t DateTime::getNtpTime(void) const
{
	long t;
	uint32_t days = date2NtpDays(_year, _month, _day);
	t = time2long(days, _hour, _minute, _second);
	return t;
}

DateTime DateTime::operator+(const TimeSpan& span)
{
	return DateTime(getNtpTime() + span.totalSeconds(), EPOCH_NTP);
}

DateTime DateTime::operator-(const TimeSpan& span)
{
	return DateTime(getNtpTime() - span.totalSeconds(), EPOCH_NTP);
}

TimeSpan DateTime::operator-(const DateTime& right)
{
	return TimeSpan(getNtpTime() - right.getNtpTime());
}

// printing for debug purposes
void DateTime::print(Stream &s)
{
	s
		<< (int)_year << "/" << (int)_month << "/" << (int)_day << " - "
		<< (int)_hour << ":" << (int)_minute << ":" << (int)_second
		<< "\n"
	;
}
