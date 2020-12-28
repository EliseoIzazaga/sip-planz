//*****************************************************
//  time.cpp
//  RTC features
//*****************************************************
/*----------------------------------------------------------------------*
 * Display the date and time from a DS3231 or DS3232 RTC every second.  *
 * Display the temperature once per minute. (The DS3231 does a          *
 * temperature conversion once every 64 seconds. This is also the       *
 * default for the DS3232.)                                             *
 *                                                                      *
 * Set the date and time by entering the following on the Arduino       *
 * serial monitor:                                                      *
 *    year,month,day,hour,minute,second,                                *
 *                                                                      *
 * Where                                                                *
 *    year can be two or four digits,                                   *
 *    month is 1-12,                                                    *
 *    day is 1-31,                                                      *
 *    hour is 0-23, and                                                 *
 *    minute and second are 0-59.                                       *
 *                                                                      *
 * Entering the final comma delimiter (after "second") will avoid a     *
 * one-second timeout and will allow the RTC to be set more accurately. *
 *                                                                      *
 * No validity checking is done, invalid values or incomplete syntax    *
 * in the input will result in an incorrect RTC setting.                *
 *                                                                      *
 * Jack Christensen 08Aug2013                                           *
 *                                                                      *
 * Tested with Arduino 1.0.5, Arduino Uno, DS3231/Chronodot, DS3232.    *
 *                                                                      *
 * This work is licensed under the Creative Commons Attribution-        *
 * ShareAlike 3.0 Unported License. To view a copy of this license,     *
 * visit http://creativecommons.org/licenses/by-sa/3.0/ or send a       *
 * letter to Creative Commons, 171 Second Street, Suite 300,            *
 * San Francisco, California, 94105, USA.                               *
 *----------------------------------------------------------------------*/ 
 
#include <DS3232RTC.h>        //http://github.com/JChristensen/DS3232RTC
#include "Streaming.h"        //http://arduiniana.org/libraries/streaming/
#include <Time.h>             //http://playground.arduino.cc/Code/Time
#include <Wire.h>             //http://arduino.cc/en/Reference/Wire
#include "logger.h"
#include "rtc_time.h"
#include "config.h"

rtcClass rtc;                                     // The one and only global rtc

void rtcClass::begin(void)
{
  const char* funcIDStr = PSTR("rtcClass::begin: ");

  //setSyncProvider() causes the Time library to synchronize with the
  //external RTC by calling RTC.get() every five minutes by default.
  setSyncProvider(RTC.get);
  logger.log(F("%SRTC Sync"), funcIDStr);
  if (timeStatus() != timeSet) 
  {
    logger.log(F("%SFailed to sync"), funcIDStr);
  }
}

// loop() used for test purposes
void rtcClass::loop(void)
{
    static time_t tLast;
    time_t t;
    tmElements_t tm;
    char buffer[20];
    int bufferSize = sizeof(buffer);
    char buffer1[20];
    int buffer1Size = sizeof(buffer1);

/*
    //check for input to set the RTC, minimum length is 12, i.e. yy,m,d,h,m,s
    if (Serial.available() >= 12) {
        //note that the tmElements_t Year member is an offset from 1970,
        //but the RTC wants the last two digits of the calendar year.
        //use the convenience macros from Time.h to do the conversions.
        int y = Serial.parseInt();
        if (y >= 100 && y < 1000)
            Serial << F("Error: Year must be two digits or four digits!") << endl;
        else {
            if (y >= 1000)
                tm.Year = CalendarYrToTm(y);
            else    //(y < 100)
                tm.Year = y2kYearToTm(y);
            tm.Month = Serial.parseInt();
            tm.Day = Serial.parseInt();
            tm.Hour = Serial.parseInt();
            tm.Minute = Serial.parseInt();
            tm.Second = Serial.parseInt();
            t = makeTime(tm);
            RTC.set(t);        //use the time_t value to ensure correct weekday is set
            setTime(t);        
            Serial << F("RTC set to: ");
            printDateTime(t);
            Serial << endl;
            //dump any extraneous input
            while (Serial.available() > 0) Serial.read();
        }
    }
*/
    
    t = now();
    if (t != tLast) 
    {
        tLast = t;
        //printDateTime(t);
        logger.log(F("%02d/%02d/%02d %02d:%02d:%02d"), year(t) - 2000, month(t), day(t), hour(t), minute(t), second(t));
        if (second(t) == 0) 
        {
            float c = RTC.temperature() / 4.;
            float f = c * 9. / 5. + 32.;
            //Serial << F("  ") << c << F(" C  ") << f << F(" F");
            fToA(c, buffer, bufferSize);
            fToA(f, buffer1, buffer1Size);
            logger.log(F("%s C %s F"), buffer, buffer1);
        }
        //Serial << endl;
    }
}

/*
//print date and time to Serial
void rtcClass::printDateTime(time_t t)
{
    printDate(t);
    Serial << ' ';
    printTime(t);
}

//print time to Serial
void rtcClass::printTime(time_t t)
{
    printI00(hour(t), ':');
    printI00(minute(t), ':');
    printI00(second(t), ' ');
}

//print date to Serial
void rtcClass::printDate(time_t t)
{
    printI00(day(t), 0);
    Serial << monthShortStr(month(t)) << _DEC(year(t));
}

//Print an integer in "00" format (with leading zero),
//followed by a delimiter character to Serial.
//Input value assumed to be between 0 and 99.
void rtcClass::printI00(int val, char delim)
{
    if (val < 10) Serial << '0';
    Serial << _DEC(val);
    if (delim > 0) Serial << delim;
    return;
}
*/
