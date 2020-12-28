//*****************************************************
//  io.h
//  Handles interaction with digital and analogue IO pins.
//*****************************************************
#ifndef IO_H
#define IO_H

#define IO_DOUT_DEBUG_1                 23
#define IO_DOUT_DEBUG_2                 24

class ioClass
{
  public:
    ioClass();

    struct pulseComparatorDef_t
    {
      bool enable;                                // Enables counting to proceed
      int32_t count;                              // The value to count to
      int pin;                                    // The pin to perform action at timeout
      int action;                                 // Action to perform at timout HIGH | LOW
    };
    
    #define IO_DIN_DISTANCE_SWITCH          22
    //#define IO_DIN_RAIL_COUNTER             23
    //#define IO_DIN_PUMP_COUNTER             24
    #define IO_DIN_RAIL_COUNTER             18
    #define IO_DIN_PUMP_COUNTER             19
    #define IO_DIN_MOTOR_FEEDBACK           25
    #define IO_DIN_POWER_FAIL               31
    #define IO_DIN_PRESS_CYCLE_IN_PROG      40
    #define IO_DIN_EMERGENCY_STOP           38
    
    //#define IO_DOUT_DEBUG_1                 23
    //#define IO_DOUT_DEBUG_2                 24
    
    #define IO_DOUT_DISTANCE_PISTON         26
    #define IO_DOUT_FAN_MISTER              27
    #define IO_DOUT_BEAD_NOZZLES            28
    #define IO_DOUT_SEALER_TRAY             29
    #define IO_DOUT_PUMP_MOTOR_FWD          37
    #define IO_DOUT_PUMP_MOTOR_REV          36
    #define IO_DOUT_RAIL_MOTOR_FWD          35
    #define IO_DOUT_RAIL_MOTOR_REV          34
    #define IO_DOUT_APPLICATOR_MOTOR_FWD    33
    #define IO_DOUT_APPLICATOR_MOTOR_REV    32
    
    #define IO_DOUT_WARNING_BUZZER          39
       
    void begin();                                 // Initialise
    void manageIO();                              // Regular IO management
    void setOutputPin(int pin, int value);        // Set an output pin
    void allOff();                             // Set all outputs to safe
    //uint16_t getRailCounter();                    // Get value of railCounter
    //uint16_t getPumpCounter();                    // Get value of pumpCounter
    int32_t getRailCounter();                     // Get value of railCounter
    int32_t getPumpCounter();                     // Get value of pumpCounter
    void startRailCounter(uint32_t target, int pin, int action);       // Setup and start the rail counter
    void startPumpCounter(uint32_t target, int pin, int action);       // Setup and start the pump counter
    
  private:
  
    volatile struct pulseComparatorDef_t railCounter;      // Counter for Rail interrupts 
    volatile struct pulseComparatorDef_t pumpCounter;      // Counter for Pump interrupts 
    static void railCounterISR();                 // Rail counter interrupt routine
    static void pumpCounterISR();                 // Pump counter interrupt routine

    void pulseComparator(volatile pulseComparatorDef_t* comparator); // Manage the comparators
                                                  // Setup a pulse comparator
    void setPulseComparator(volatile pulseComparatorDef_t* comparator, uint32_t target, int pin, int action);
    void testRailCounter();                       // Just to test it
};

extern ioClass io;                                // The one and only io


#endif

