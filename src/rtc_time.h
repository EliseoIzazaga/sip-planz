//*****************************************************
//  time.h
//  RTC features
//*****************************************************
#ifndef TIME_H
#define TIME_H

class rtcClass
{
  //void printDateTime(time_t t);
  //void printTime(time_t t);
  //void printDate(time_t t);
  //void printI00(int val, char delim);

  public:
    void begin(void);
    void loop(void);        // Test purposes

};

extern rtcClass rtc;                              // The one and only rtc

#endif
