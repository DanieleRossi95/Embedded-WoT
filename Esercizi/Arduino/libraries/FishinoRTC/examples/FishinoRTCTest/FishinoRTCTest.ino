// Example for library 'FishinoRTC'
#include <FishinoFlash.h>
#include <FishinoRTC.h>

#define DEBUG_LEVEL_INFO
#include <FishinoDebug.h>

#ifdef _FISHINO_PIC32_
void alarmISR(void)
{
	static bool on = true;
	digitalWrite(25, on);
	on = !on;
}
#endif

void setup()
{
	Serial.begin(115200);
	while(!Serial)
		;
	
	if (!RTC.isrunning())
	{
		DEBUG_INFO("\nRTC is NOT running!\n\n");
		
		// following line sets the RTC to the date & time this sketch was compiled
		DEBUG_INFO("Date : %s -- Time : %s\n", __DATE__, __TIME__);
		DateTime now(__DATE__, __TIME__);
		DEBUG_INFO("NOW:%04u/%02u/%02u(%s) %2u:%2u:%2u\n",
			now.year(),
			now.month(),
			now.day(),
			now.dayOfTheWeekStr().c_str(),
			now.hour(),
			now.minute(),
			now.second()
		);
		DEBUG_INFO("2000 time:%lu\n", now.getSecondsTime());
		DEBUG_INFO("Unix time:%lu\n", now.getUnixTime());
		DEBUG_INFO("Ntp  time:%lu\n", now.getNtpTime());
		RTC.adjust(now);
	}
	

#ifdef _FISHINO_PIC32_

	// for Fishino32 boards, setup an alarm event every 5 seconds
	// which blinks a led
	pinMode(25, OUTPUT);
	digitalWrite(25, LOW);
	RTC.setAlarmMode(AlarmModes::ALARM_REPEAT_FOREVER);
	RTC.setAlarmHandler(alarmISR);
	RTC.setAlarm(5, true);
	
#endif

}

void loop()
{
	DateTime now = RTC.now();

	DEBUG_INFO("%04u/%02u/%02u(%s) %2u:%2u:%2u\n",
		now.year(),
		now.month(),
		now.day(),
		now.dayOfTheWeekStr().c_str(),
		now.hour(),
		now.minute(),
		now.second()
	);
	
	DEBUG_INFO(" since midnight 1/1/1970 = %lu s = %lu d\n", now.getUnixTime(), now.getUnixTime() / 86400L);

	// calculate a date which is 7 days, 12 hours, 30 minutes and 6 seconds into the future
	DateTime future(now + TimeSpan(7, 12, 30, 6));

	DEBUG_INFO(" now + 7d + 12h + 30m + 6s: %04u/%02u/%02u(%s) %2u:%2u:%2u\n",
		future.year(),
		future.month(),
		future.day(),
		future.dayOfTheWeekStr().c_str(),
		future.hour(),
		future.minute(),
		future.second()
	);

	Serial.println();
	delay(3000);
}
