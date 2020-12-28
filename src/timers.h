//*****************************************************
//  timers.h
//  Providing interrupt driven timers
//*****************************************************
#ifndef TIMERS_H
#define TIMERS_H

class timersClass
{  
  private:
    static void timerIsr();
    static int ticks;

  public:
    void begin();

};

extern timersClass timers;

#endif

