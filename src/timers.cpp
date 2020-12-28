//*****************************************************
//  timers.cpp
//  Providing interrupt driven timers
//*****************************************************
#include <Arduino.h>
#include <TimerOne.h>
#include "siplaminator.h"
#include "timers.h"
#include "logger.h"
#include "io.h"
#include "touchscreen.h"
#include "gui.h"


timersClass timers;                                     // The timers object
int timersClass::ticks;

//*****************************************************
//  begin.cpp
//  Init timer and attach interrupt service routine.
//  Set timer for 1ms
//*****************************************************
void timersClass::begin()
{
  Timer1.initialize(1000);                             // 1ms interrupt
  Timer1.attachInterrupt(timersClass::timerIsr);       // attach the ISR routine here  
}


//*****************************************************
//  timerIsr.cpp
//  ISR for the timer.
//*****************************************************
void timersClass::timerIsr()
{
  if (digitalRead(IO_DIN_EMERGENCY_STOP) == HIGH)
  {
    gui.emergencyStop();
  }
}

