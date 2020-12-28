//*****************************************************
//  io.cpp
//  Handles interaction with digital and analogue IO pins.
//*****************************************************
#include <Arduino.h>
#include "siplaminator.h"
#include "io.h"
#include "touchscreen.h"
#include "Logger.h"
#include "gui.h"


//volatile uint16_t ioClass::railCounter = 0;
//volatile uint16_t ioClass::pumpCounter = 0;
//volatile int32_t ioClass::railCounter = 0L;
//volatile int32_t ioClass::pumpCounter = 0L;
//struct pulseComparatorDef_t ioClass::railCounter = {};
//struct pulseComparatorDef_t ioClass::pumpCounter = {};

ioClass::ioClass()
{
}

//*****************************************************
// begin.
//*****************************************************

void ioClass::begin()
{ // Initialise the digital IO inputs
  pinMode(IO_DIN_DISTANCE_SWITCH, INPUT_PULLUP);
  pinMode(IO_DIN_RAIL_COUNTER, INPUT_PULLUP);
  pinMode(IO_DIN_PUMP_COUNTER, INPUT_PULLUP);
  pinMode(IO_DIN_MOTOR_FEEDBACK, INPUT_PULLUP);
  pinMode(IO_DIN_POWER_FAIL, INPUT_PULLUP);
  pinMode(IO_DIN_PRESS_CYCLE_IN_PROG, INPUT_PULLUP);
  pinMode(IO_DIN_EMERGENCY_STOP, INPUT_PULLUP);

  // Initialise the digital IO outputs
  pinMode(IO_DOUT_DISTANCE_PISTON, OUTPUT);
  digitalWrite(IO_DOUT_DISTANCE_PISTON, HIGH);
  
  pinMode(IO_DOUT_FAN_MISTER, OUTPUT);
  digitalWrite(IO_DOUT_FAN_MISTER, HIGH);
  
  pinMode(IO_DOUT_BEAD_NOZZLES, OUTPUT);
  digitalWrite(IO_DOUT_BEAD_NOZZLES, HIGH);
  
  pinMode(IO_DOUT_SEALER_TRAY, OUTPUT);
  digitalWrite(IO_DOUT_SEALER_TRAY, HIGH);
  
  pinMode(IO_DOUT_WARNING_BUZZER, OUTPUT);
  digitalWrite(IO_DOUT_WARNING_BUZZER, LOW);

  pinMode(IO_DOUT_PUMP_MOTOR_FWD, OUTPUT);
  digitalWrite(IO_DOUT_PUMP_MOTOR_FWD, HIGH);
  
  pinMode(IO_DOUT_PUMP_MOTOR_REV, OUTPUT);
  digitalWrite(IO_DOUT_PUMP_MOTOR_REV, HIGH);
  
  pinMode(IO_DOUT_RAIL_MOTOR_FWD, OUTPUT);
  digitalWrite(IO_DOUT_RAIL_MOTOR_FWD, HIGH);
  
  pinMode(IO_DOUT_RAIL_MOTOR_REV, OUTPUT);
  digitalWrite(IO_DOUT_RAIL_MOTOR_REV, HIGH);
  
  pinMode(IO_DOUT_APPLICATOR_MOTOR_FWD, OUTPUT);
  digitalWrite(IO_DOUT_APPLICATOR_MOTOR_FWD, HIGH);
  
  pinMode(IO_DOUT_APPLICATOR_MOTOR_REV, OUTPUT); 
  digitalWrite(IO_DOUT_APPLICATOR_MOTOR_REV, HIGH); 

  pinMode(IO_DOUT_DEBUG_1, OUTPUT); 
  pinMode(IO_DOUT_DEBUG_2, OUTPUT); 

  allOff();                                               // To set the indicators
  
  railCounter.enable = false;                             // Disable rail counter
  pumpCounter.enable = false;                             // Disable pump counter

  attachInterrupt(digitalPinToInterrupt(IO_DIN_RAIL_COUNTER), ioClass::railCounterISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(IO_DIN_PUMP_COUNTER), ioClass::pumpCounterISR, CHANGE);
}

//*****************************************************
// allOff.
// Reset all the outputs to a safe condition. Called from emergencyStop().
//*****************************************************
void ioClass::allOff()
{
  setOutputPin(IO_DOUT_DISTANCE_PISTON, HIGH);
  setOutputPin(IO_DOUT_FAN_MISTER, HIGH);
  setOutputPin(IO_DOUT_BEAD_NOZZLES, HIGH);
  setOutputPin(IO_DOUT_SEALER_TRAY, HIGH);
  setOutputPin(IO_DOUT_PUMP_MOTOR_FWD, HIGH);
  setOutputPin(IO_DOUT_PUMP_MOTOR_REV, HIGH);
  setOutputPin(IO_DOUT_RAIL_MOTOR_FWD, HIGH);
  setOutputPin(IO_DOUT_RAIL_MOTOR_REV, HIGH);
  setOutputPin(IO_DOUT_APPLICATOR_MOTOR_FWD, HIGH);
  setOutputPin(IO_DOUT_APPLICATOR_MOTOR_REV, HIGH); 
  tsc.dirtyIndicators();
}

//*****************************************************
// manageIO.
// Periodicaly read the inputs and send to the touch screen indicators.
//*****************************************************

void ioClass::manageIO()
{
  static uint32_t lastRun = 0;                          // Timer

  if ((millis() - lastRun) > 400)                       // Read inputs every 400ms
  {
    lastRun = millis();
    tsc.setIndicatorValue(touchscreen::IND_DISTANCE, digitalRead(IO_DIN_DISTANCE_SWITCH));
    tsc.setIndicatorValue(touchscreen::IND_RAIL_COUNTER, digitalRead(IO_DIN_RAIL_COUNTER));
    tsc.setIndicatorValue(touchscreen::IND_PUMP_COUNTER, digitalRead(IO_DIN_PUMP_COUNTER));
    tsc.setIndicatorValue(touchscreen::IND_MOTOR_FEEDBACK, digitalRead(IO_DIN_MOTOR_FEEDBACK));
    tsc.setIndicatorValue(touchscreen::IND_POWER_FAIL, digitalRead(IO_DIN_POWER_FAIL));

    tsc.setIndicatorValue(touchscreen::IND_PRESS_CYCLE, digitalRead(IO_DIN_PRESS_CYCLE_IN_PROG));
    tsc.setIndicatorValue(touchscreen::IND_EMERGENCY_STOP, digitalRead(IO_DIN_EMERGENCY_STOP));
    
    tsc.setAnalogueIndicatorValue(touchscreen::AIND_POSITION, analogRead(A8));    
    tsc.setAnalogueIndicatorValue(touchscreen::AIND_POTENTIOMETER, analogRead(A9));    
    tsc.setAnalogueIndicatorValue(touchscreen::AIND_RAIL_COUNTER, getRailCounter());    
    tsc.setAnalogueIndicatorValue(touchscreen::AIND_PUMP_COUNTER, getPumpCounter());
    tsc.setAnalogueIndicatorValue(touchscreen::AIND_WORK_LIFE, gui.getWLTimeToGo());
  }
    
  //testRailCounter();                                 // Debugging    
}

//*****************************************************
// setOutputPin.
// Set an output pin 'HIGH' or 'LOW'
//*****************************************************

void ioClass::setOutputPin(int pin, int value)
{
  logger.log(F("setOutputPin: Pin %d, value %d"), pin, value);
  digitalWrite(pin, value);

  switch(pin)
  {
    case IO_DOUT_DISTANCE_PISTON:
      tsc.setIndicatorValue(touchscreen::IND_DISTANCE_PISTON, value);
      break;
    case IO_DOUT_FAN_MISTER:
      tsc.setIndicatorValue(touchscreen::IND_FAN_MISTER, value);
      break;
    case IO_DOUT_BEAD_NOZZLES:
      tsc.setIndicatorValue(touchscreen::IND_BEAD_NOZZLES, value);
      break;
    case IO_DOUT_SEALER_TRAY:
      tsc.setIndicatorValue(touchscreen::IND_SEALER_TRAY, value);
      break;
    case IO_DOUT_PUMP_MOTOR_FWD:
      tsc.setIndicatorValue(touchscreen::IND_MOTOR_1_PUMP_FORWARD, value);
      break;
    case IO_DOUT_PUMP_MOTOR_REV:
      tsc.setIndicatorValue(touchscreen::IND_MOTOR_1_PUMP_REVERSE, value);
      break;
    case IO_DOUT_RAIL_MOTOR_FWD:
      tsc.setIndicatorValue(touchscreen::IND_MOTOR_2_RAIL_FORWARD, value);
      break;
    case IO_DOUT_RAIL_MOTOR_REV:
      tsc.setIndicatorValue(touchscreen::IND_MOTOR_2_RAIL_REVERSE, value);
      break;
    case IO_DOUT_APPLICATOR_MOTOR_FWD:
      tsc.setIndicatorValue(touchscreen::IND_MOTOR_3_APPLICATOR_FORWARD, value);
      break;
    case IO_DOUT_APPLICATOR_MOTOR_REV:
      tsc.setIndicatorValue(touchscreen::IND_MOTOR_3_APPLICATOR_REVERSE, value);
      break;
    case IO_DOUT_WARNING_BUZZER:
      tsc.setIndicatorValue(touchscreen::IND_BUZZER, value);
      break;
  }
}

//*****************************************************
// pulseComparator.
// Manage the interrupt driven pulse comparators.
//*****************************************************
void ioClass::pulseComparator(volatile pulseComparatorDef_t* comparator)
{
  if (comparator->enable)
  {
    comparator->count--;
    if (comparator->count <= 0)
    {
      //io.setOutputPin(comparator->pin, comparator->action);   // setOutputPin logs so cannot be used in ISR
      digitalWrite(comparator->pin, comparator->action);        // setOutputPin logs so cannot be used in ISR
      comparator->enable = false;
    }
  }
}

//*****************************************************
// railCounterISR.
// Interrupt routing for the Rail counter.
//*****************************************************

void ioClass::railCounterISR()
{
  //io.setOutputPin(IO_DOUT_DEBUG_2, HIGH);  
  io.pulseComparator(&io.railCounter);
  //io.setOutputPin(IO_DOUT_DEBUG_2, LOW);
}

//*****************************************************
// pumpCounterISR.
// Interrupt roouting for the Pump counter.
//*****************************************************

void ioClass::pumpCounterISR()
{
  io.pulseComparator(&io.pumpCounter);
}

//*****************************************************
// setPulseComparator.
// Set a pulse comparator ready to go then start it.
//*****************************************************
void ioClass::setPulseComparator(volatile pulseComparatorDef_t* comparator, uint32_t target, int pin, int action)
{
  comparator->count = target;
  comparator->pin = pin;
  comparator->action = action;
  comparator->enable = true;                          // Enable must be last
}

//*****************************************************
// startRailCounter.
// Set the rail pulse comparator ready to go then start it.
//*****************************************************
void ioClass::startRailCounter(uint32_t target, int pin, int action)
{
  setPulseComparator(&railCounter, target, pin, action);
}

//*****************************************************
// getRailCounter.
// Return the value of the Rail counter 
//*****************************************************

int32_t ioClass::getRailCounter()
{
  int32_t counter;
  
  noInterrupts();
  counter = railCounter.count;
  interrupts();
  
  return counter;
}


//*****************************************************
// getPumpCounter.
// Return the value of the Pump counter 
//*****************************************************

int32_t ioClass::getPumpCounter()
{
  int32_t counter;
  
  noInterrupts();
  counter = pumpCounter.count;
  interrupts();
  
  return counter;
}

//*****************************************************
// startPumpCounter.
// Set the pump pulse comparator ready to go then start it.
//*****************************************************
void ioClass::startPumpCounter(uint32_t target, int pin, int action)
{
  setPulseComparator(&pumpCounter, target, pin, action);
}


ioClass io;                                     // The one and only global io

//*****************************************************
// testRailCounter.
// Set the buzzer output then start the comparator then do again.
//*****************************************************
void ioClass::testRailCounter()
{
  static int state = 0;
  static int32_t timer;
  
  switch (state)
  {
    case 0:
      startRailCounter(0, IO_DOUT_WARNING_BUZZER, LOW);
      state = 1;
      break;

    case 1:
      if (getRailCounter() <= 0)
      {
        startRailCounter(2, IO_DOUT_WARNING_BUZZER, HIGH);
        state = 2;
      }
      break;
      
    case 2:   
      if ((millis() - timer) > 4)
      {
        state = 3;
      }
      break;
   
    case 3:
      if (getRailCounter() <= 0)
      {
        startRailCounter(2, IO_DOUT_WARNING_BUZZER, LOW);
        state = 4;
        timer = millis();
      }
      break;
      
    case 4:   
      if ((millis() - timer) > 4)
      {
        state = 1;
      }
      break;
   
  }
}