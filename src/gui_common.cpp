//*****************************************************
//  gui.cpp
//  Handles all interaction with the user through the GUI.
//*****************************************************
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <string.h>
#include <DS3232RTC.h> 
#include <Time.h> 
#include "siplaminator.h"
#include "config.h"
#include "touchscreen.h"
#include "gui.h"
#include "motor_test.h"
#include "io.h"
#include "Logger.h"
#include "beading.h"

//*****************************************************
// Text strings for pages are in gui.cpp.
//*****************************************************

//*****************************************************
// railHomePageDef page.
//*****************************************************

const struct touchscreen::pageFieldDef_t guiClass::railHomePageDef[] PROGMEM =
{
  {PID_TITLE, touchscreen::TEXT,    70, 40, 200, 70,  ILI9341_BLACK,  0,  ILI9341_WHITE, 1, (char*)guiClass::railToHomeStr,     40, 45, touchscreen::NOT_ACTIVE},
  {PID_STOP,  touchscreen::BUTTON,  60, 60, 200, 100, ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::stopStr, 65, 60, touchscreen::ACTIVE},
};

//*****************************************************
// railHomeToBeadPageDef page.
//*****************************************************

const struct touchscreen::pageFieldDef_t guiClass::railHomeToBeadPageDef[] PROGMEM =
{
  {PID_TITLE, touchscreen::TEXT,    80, 40, 200, 70,  ILI9341_BLACK,  0,  ILI9341_WHITE, 1, (char*)guiClass::railToBeadStr,     45, 45, touchscreen::NOT_ACTIVE},
  {PID_STOP,  touchscreen::BUTTON,  60, 60, 200, 100, ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::stopStr, 60, 60, touchscreen::ACTIVE},
};

//*****************************************************
// railBeadToEndPageDef page.
//*****************************************************

const struct touchscreen::pageFieldDef_t guiClass::railBeadToEndPageDef[] PROGMEM =
{
  {PID_TITLE, touchscreen::TEXT,    80, 40, 200, 70,  ILI9341_BLACK,  0,  ILI9341_WHITE, 1, (char*)guiClass::railToEndStr,      45, 45, touchscreen::NOT_ACTIVE},
  {PID_STOP,  touchscreen::BUTTON,  60, 60, 200, 100, ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::stopStr, 60, 60, touchscreen::ACTIVE},
};

//*****************************************************
// railEndToBeadPageDef page.
//*****************************************************

const struct touchscreen::pageFieldDef_t guiClass::railEndToBeadPageDef[] PROGMEM =
{
  {PID_TITLE, touchscreen::TEXT,    80, 40, 200, 70,  ILI9341_BLACK,  0,  ILI9341_WHITE, 1, (char*)guiClass::railToBeadStr,         45, 45, touchscreen::NOT_ACTIVE},
  {PID_STOP,  touchscreen::BUTTON,  60, 60, 200, 100, ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::stopStr, 60, 60, touchscreen::ACTIVE},
};

//*****************************************************
// railHomeToWetPageDef page.
//*****************************************************

const struct touchscreen::pageFieldDef_t guiClass::railHomeToWetPageDef[] PROGMEM =
{
  {PID_TITLE, touchscreen::TEXT,    80, 40, 200, 70,  ILI9341_BLACK,  0,  ILI9341_WHITE, 1, (char*)guiClass::railToWetStr,         45, 45, touchscreen::NOT_ACTIVE},
  {PID_STOP,  touchscreen::BUTTON,  60, 60, 200, 100, ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::stopStr, 60, 60, touchscreen::ACTIVE},
};


//*****************************************************
// applicatorHomePageDef page.
//*****************************************************

const struct touchscreen::pageFieldDef_t guiClass::applicatorHomePageDef[] PROGMEM =
{
  {PID_TITLE, touchscreen::TEXT,    60, 40, 200, 70,  ILI9341_BLACK,  0,  ILI9341_WHITE, 1, (char*)guiClass::applicatorToHomeStr,   45, 45, touchscreen::NOT_ACTIVE},
  {PID_STOP,  touchscreen::BUTTON,  60, 60, 200, 100, ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::stopStr,     60, 60, touchscreen::ACTIVE},
};

//*****************************************************
// startBeadingStopPageDef page.
//*****************************************************

const struct touchscreen::pageFieldDef_t guiClass::startBeadingStopPageDef[] PROGMEM =
{
  {PID_TITLE,       touchscreen::TEXT,         80, 30, 200, 70,  ILI9341_BLACK,  0,  ILI9341_WHITE, 1, (char*)guiClass::startBeadingStr,   45, 45, touchscreen::NOT_ACTIVE},
  {PID_WL_COUNTER,  touchscreen::NATIVE_TEXT,  80, 50,  200, 20, ILI9341_BLACK,  0,  ILI9341_WHITE, 2, (char*)guiClass::blankStr,          45, 45, touchscreen::NOT_ACTIVE},
  {PID_STOP,        touchscreen::BUTTON,       60, 75, 200, 100, ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::stopStr, 60, 60, touchscreen::ACTIVE},
};


//*****************************************************
// sendRailToStartPageDef page.
//*****************************************************

const struct touchscreen::pageFieldDef_t guiClass::sendRailToBeadPageDef[] PROGMEM =
{
  {PID_TITLE,    touchscreen::TEXT,         80, 40,    200, 70,  ILI9341_BLACK,  0,  ILI9341_WHITE, 1, (char*)guiClass::railToStartStr,        45, 45, touchscreen::NOT_ACTIVE},
};

//*****************************************************
// Continue/Return page.
//*****************************************************
const struct touchscreen::pageFieldDef_t guiClass::continueReturnPageDef[] PROGMEM =
{
  {guiClass::PID_CONTINUE,  touchscreen::BUTTON,  60, 10,  200, 100,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::continueStr, 40, 60, touchscreen::ACTIVE},
  {guiClass::PID_RETURN,    touchscreen::BUTTON,  60, 120, 200, 100,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::returnStr,   40, 60, touchscreen::ACTIVE}
};

//*****************************************************
// Proceed page.
//*****************************************************
const struct touchscreen::pageFieldDef_t guiClass::proceedPageDef[] PROGMEM =
{
  {guiClass::PID_PROCEED,  touchscreen::BUTTON,  60, 60,  200, 100,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::proceedStr, 40, 60, touchscreen::ACTIVE}
};

//*****************************************************
// getWLTimeToGo
//*****************************************************

uint32_t guiClass::getWLTimeToGo()
{
  return m_wlTimeToGo;
}

//*****************************************************
// manageApplicatorToBead.
// Lower the applicator motor to the bead position.
// Call with: 
//    doBegin - true only when initialising
//
// Returns:
//    0   - Done
//    1   - in progress
//    -1  - Error
//*****************************************************

int guiClass::manageApplicatorToBead(bool doBegin)
{
  const char* funcIDStr = PSTR("manageApplicatorToBead: ");
  static applicatorToBeadStates_t state;                                                      // State variable
  static applicatorToBeadStates_t lastState;                                                  // Last state to detect changes
  static uint32_t timerStart;                                                                 // Timer
  const touchscreen::pageFieldDef_t* pageDef;                                                 // The page to display
  size_t pageSize;
  touchscreen::pageFieldDef_t* field;
  int retVal = 1;

  if (doBegin == true)                                                                        // doBegin true to initialise
  {
    logger.log(F("%SStart"), funcIDStr);
    state = ATB_START;
    lastState = ATB_START;
  }

  switch(state)
  {
    case ATB_START:
      io.setOutputPin(IO_DOUT_DISTANCE_PISTON, LOW);                                          // Move distance piston down
      timerStart = millis();                                                                  // Start the exit delay
      state = ATB_PISTON_DOWN_DELAY;
      break;
      
    case ATB_PISTON_DOWN_DELAY:
      if ((millis() - timerStart) > configGetDelay(11))
      {
        logger.log(F("%STO Applic forward"), funcIDStr);
        io.setOutputPin(IO_DOUT_APPLICATOR_MOTOR_FWD, LOW);                                  // Applicator forward
        state = ATB_WAIT_DISTANCE;
      }
      break;
      
    case ATB_WAIT_DISTANCE:
      if (analogRead(A8) < analogRead(A9))                                                   // If input voltage < reference pot
      {
        logger.log(F("%SDistance reached"), funcIDStr);
        io.setOutputPin(IO_DOUT_APPLICATOR_MOTOR_FWD, HIGH);                                 // Applicator forward
        state = ATB_DONE;                                                                    // We are done
      }
      else
      {
        //if (digitalRead(IO_DIN_DISTANCE_SWITCH) == HIGH)
        if (digitalRead(IO_DIN_MOTOR_FEEDBACK) == HIGH)
        {
          io.setOutputPin(IO_DOUT_APPLICATOR_MOTOR_FWD, HIGH);                               // Stop applicator forward
          io.setOutputPin(IO_DOUT_WARNING_BUZZER, HIGH);                                     // Sound the buzzer
          //logger.log(F("%SDistance switch detected"), funcIDStr);
          tsc.renderPage(continueReturnPageDef, sizeof(continueReturnPageDef));              // Render the page
          state = ATB_SELECT_CONTINUE;
        }
      }
      break;
      
    case ATB_SELECT_CONTINUE:
      if(field = tsc.isTouchedField())                                                       // React to the STOP button
      {
        switch(field->id)
        {
          case guiClass::PID_CONTINUE:
            io.setOutputPin(IO_DOUT_WARNING_BUZZER, LOW);                                    // Buzzer off just in case
            state = ATB_DONE;
            break;
          case guiClass::PID_RETURN:
            logger.log(F("%SAborting the test"), funcIDStr);
            state = ATB_ERROR;
            break;
        }
      }
      break;
            
    case ATB_ERROR:
      retVal = -1;
      io.setOutputPin(IO_DOUT_WARNING_BUZZER, LOW);                                         // Buzzer off just in case
      logger.log(F("%SError"), funcIDStr);
      state = ATB_START;
      break;
      
    case ATB_DONE:
      //io.setOutputPin(IO_DOUT_WARNING_BUZZER, LOW);                                         // Buzzer off just in case
      io.setOutputPin(IO_DOUT_DISTANCE_PISTON, HIGH);                                       // Move distance piston up
      logger.log(F("%SDone"), funcIDStr);
      retVal = 0;
      state = ATB_START;
      break;
  }
  
  if (state != lastState)
  {
    logger.log(F("%Sstate %d -> %d"), funcIDStr, lastState, state);
    lastState = state;
  }

  return retVal;
}

//*****************************************************
// manageRailMove.
// Sends the rail motor M2 to a position
// Call with: 
//    journey - HOME_TO_BEAD | BEAD_TO_END | END_TO_BEAD | HOME_TO_WET
//    doBegin - true only when initialising
//
// Returns:
//    0   - Done
//    1   - in progress
//    -1  - Error
//*****************************************************

int guiClass::manageRailMove(guiRailMoveDefs_t journey, bool doBegin)
{
  const char* funcIDStr = PSTR("manageRailMove: ");
  static railMoveStates_t state;                                              // State variable
  static railMoveStates_t lastState;                                          // Last state to detect changes
  int outPin;
  uint32_t pulseCountTarget;                                                  // pulses to be counted
  touchscreen::pageFieldDef_t* field;
  const touchscreen::pageFieldDef_t* pageDef;                                 // The page to display
  size_t pageSize;
  uint32_t exitTimer = configGetDelay(5);                                     // Timer to use after complete
  static uint32_t timerStart;                                                 // Timer
  int outPinISRAction = HIGH;                                                 // Level for the ISR to switch to
  int retVal = 1;
  
  if (doBegin == true)                                                        // doBegin true to initialise
  {
    switch(journey)
    {
      case HOME_TO_BEAD:
        logger.log(F("%SHome->Bead"), funcIDStr);
        break;
      case BEAD_TO_END:
        logger.log(F("%SBead->End"), funcIDStr);
        break;
      case END_TO_BEAD:
        logger.log(F("%SEnd->Bead"), funcIDStr);
        break;
      case HOME_TO_WET:
        logger.log(F("%SHome->Wet"), funcIDStr);
        break;
      default:
        logger.log(F("%SUnknown"), funcIDStr);
        break;
    }
    state = RM_START;
    lastState = RM_START;
  }
  
  switch(journey)
  {
    case HOME_TO_BEAD:
      outPin = IO_DOUT_RAIL_MOTOR_FWD;
      pulseCountTarget = configGetPulsesHomeToBead();
      pageDef = railHomeToBeadPageDef;
      pageSize = sizeof(railHomeToBeadPageDef);
      break;
    case BEAD_TO_END:
      outPin = IO_DOUT_RAIL_MOTOR_FWD;
      pulseCountTarget = configGetPulsesBeadToEnd();
      pageDef = railBeadToEndPageDef;
      pageSize = sizeof(railBeadToEndPageDef);
      break;
    case END_TO_BEAD:
      outPin = IO_DOUT_RAIL_MOTOR_REV;
      pulseCountTarget = configGetPulsesBeadToEnd();
      pageDef = railEndToBeadPageDef;
      pageSize = sizeof(railEndToBeadPageDef);
      break;
    case HOME_TO_WET:
    default:
      outPin = IO_DOUT_RAIL_MOTOR_FWD;
      pulseCountTarget = configGetPulsesHomeToWet();
      pageDef = railHomeToWetPageDef;
      pageSize = sizeof(railHomeToWetPageDef);
      break;
  }
    
  switch(state)
  {
    case RM_START:
      tsc.renderPage(pageDef, pageSize);                                      // Render the page
      io.startRailCounter(pulseCountTarget, outPin, outPinISRAction);         // Start the interrupt pulse counter
      io.setOutputPin(outPin, LOW);                                           // Start the motor
      state = RM_WAIT_COUNT;
      break;
      
    case RM_WAIT_COUNT:
      if (digitalRead(IO_DIN_MOTOR_FEEDBACK) == HIGH)
      {                                                                       // Feedback has gone low
        io.setOutputPin(outPin, HIGH);                                        // Stop the motor
        logger.log(F("%SMotor FB high"), funcIDStr);
        emergencyStop();
        state = RM_ERROR;
        break;
      }
      
      if (io.getRailCounter() <= 0L)
      {                                                                       // Pulse counted out
        logger.log(F("%SPulse position reached"), funcIDStr);
        io.setOutputPin(outPin, outPinISRAction);                             // Pin has been set in ISR. This is to log it only 
        timerStart = millis();                                                // Start the exit delay
        state = RM_EXIT_DELAY;
      }
      
      if(field = tsc.isTouchedField())                                        // React to the STOP button
      {
        switch(field->id)
        {
          case guiClass::PID_STOP:
            io.setOutputPin(outPin, HIGH);                                    // Stop the motor
            emergencyStop();
            logger.log(F("%SStop touched"), funcIDStr);
            state = RM_ERROR;
            break;
        }
      }
      break;
      
    case RM_EXIT_DELAY:
      if ((millis() - timerStart) > exitTimer)
      {
        logger.log(F("%SExit time out"), funcIDStr);
        state = RM_DONE;
      }
      break;
      
    case RM_ERROR:
      logger.log(F("%SRail move error"), funcIDStr);
      retVal = -1;
      break;
      
    case RM_DONE:
    default:
      retVal = 0;
      break;
  }
  
  if (state != lastState)
  {
    logger.log(F("%Sstate %d -> %d"), funcIDStr, lastState, state);
    lastState = state;
  }

  return retVal;
}

//*****************************************************
// manageMotorToHome.
// Sends a motor to home
// Call with: 
//    motor   - MOTOR_RAIL | MOTOR_APPLIC
//    doBegin - true only when initialising
// Where:
//    MOTOR_RAIL    == M2
//    MOTOR_APPLIC  == M3
//
// Returns:
//    0   - Done
//    1   - in progress
//    -1  - Error
//*****************************************************

int guiClass::manageMotorToHome(guiMotorDefs_t motor, bool doBegin)
{
  const char* funcIDStr = PSTR("manageMotorToHome: ");
  static motorHomeStates_t state;                                                             // State variable
  static motorHomeStates_t lastState;                                                         // Last state to detect changes
  uint32_t exitTimer;
  static uint32_t timerStart;                                                                 // Timer
  int outPin;
  const touchscreen::pageFieldDef_t* pageDef;                                                 // The page to display
  size_t pageSize;
  touchscreen::pageFieldDef_t* field;
  int retVal = 1;

  switch(motor)                                                                               // Setup for the motor
  {
    case MOTOR_RAIL:
      outPin = IO_DOUT_RAIL_MOTOR_REV;
      pageDef = railHomePageDef;
      pageSize = sizeof(railHomePageDef);
      exitTimer = configGetDelay(5);                                                          // Timer to use after complete
      break;
    case MOTOR_APPLIC:
    default:
      outPin = IO_DOUT_APPLICATOR_MOTOR_REV;
      exitTimer = configGetDelay(7);                                                          // Timer to use after complete
      pageDef = applicatorHomePageDef;
      pageSize = sizeof(applicatorHomePageDef);
      break;
  }
  
  if (doBegin == true)                                                                        // doBegin true to initialise
  {
    switch(motor)                                                                               // Setup for the motor
    {
      case MOTOR_RAIL:
        logger.log(F("%SRail"), funcIDStr);
        break;
      case MOTOR_APPLIC:
        logger.log(F("%SApplic"), funcIDStr);
        break;
      default:
        logger.log(F("%SUnknown"), funcIDStr);
        break;
    }
    state = MH_START;
    lastState = MH_START;
  }

  switch(state)
  {
    case MH_START:
      tsc.renderPage(pageDef, pageSize);                                                      // Render the page
      io.setOutputPin(outPin, LOW);                                                           // Start the motor
      state = MH_WAIT_FEEDBACK;
      break;
      
    case MH_WAIT_FEEDBACK:                                                                    // Sit here until the motor goes home
      if (digitalRead(IO_DIN_MOTOR_FEEDBACK) == HIGH)
      {
        io.setOutputPin(outPin, HIGH);                                                        // Stop the motor
        logger.log(F("%SMotor %d at home"), funcIDStr, motor);
        timerStart = millis();                                                                // Start the exit delay
        state = MH_EXIT_DELAY;
        break;
      }

      if(field = tsc.isTouchedField())                                                            // React to the STOP button
      {
        switch(field->id)
        {
          case guiClass::PID_STOP:
            io.setOutputPin(outPin, HIGH);                                                        // Stop the motor
            emergencyStop();
            logger.log(F("%SStop touched"), funcIDStr);
            state = MH_ERROR;
            break;
        }
      }
      break;
      
    case MH_EXIT_DELAY:
      if ((millis() - timerStart) > exitTimer)
      {
        logger.log(F("%SDelayed out"), funcIDStr);
        state = MH_DONE;
      }
      break;
      
    case MH_ERROR:
      logger.log(F("%SError"), funcIDStr);
      retVal = -1;
      state = MH_START;
      break;
      
    case MH_DONE:
      logger.log(F("%SDone"), funcIDStr);
      retVal = 0;
      state = MH_START;
      break;
  }
  
  if (state != lastState)
  {
    logger.log(F("%Sstate %d -> %d"), funcIDStr, lastState, state);
    lastState = state;
  }

  return retVal;
}

//*****************************************************
// manageWorkLife.
// Updates the Work Life field PID_WL_COUNTER with the time to go.
// Updates the Work Time (Formerly 'Work Life') then monitors the
// time to go. When it reaches zero the buzzer is turned on. 60 secs
// later it is turned off. If the machine is still in a batch ie. the
// press has not been activated then the buzzer will sound for another
// 60 secs.
//*****************************************************
void guiClass::manageWorkLife()
{
  const char* funcIDStr = PSTR("manageWorkLife: ");
  static manageWorkLifeStates_t state = WL_START;                                 // State variable
  static manageWorkLifeStates_t lastState = WL_START;                             // Last state to detect changes
  static touchscreen::pageFieldDef_t* fieldPtr;                                   // Ptr to a field in the page table
  static uint32_t timerStart = 0;                                                 // To determine when to update
  touchscreen::pageFieldDef_t wlField;
  char buf[32];
  uint32_t timeElapsed;
  uint32_t timeOut;
  static uint32_t dwellTimer;

  if ((millis() - timerStart) >= 1000)                                            // Do this once per second                   
  {
    timerStart = millis();                                                        // Reset the tick counter
    //timeElapsed = millis() - m_workLifeStartTime;                                 // Compute time spent
    timeElapsed = now() - beader.getWLStartTime();                                // Compute time spent
    timeOut = configGetTimeOut(1) / 1000;                                         // Don't exceed the timeout
    if (timeElapsed >= timeOut)                                                   // Compute time to go
    {
      m_wlTimeToGo = 0L;
    }
    else
    {
      m_wlTimeToGo = timeOut - timeElapsed;                                     
    }
    
    fieldPtr = tsc.findFieldByID(PID_WL_COUNTER);                                 // Get a pointer to the work-life counter field
    if (fieldPtr)                                                                 // It may not be displayed
    {
      wlField = *fieldPtr;                                                        // Copy the field localy 
      tsc.fillField(&wlField);                                                    // Erase the field
      //m_wlTimeToGo /= 1000L;                                                      // and redraw
      snprintf_P(buf, sizeof(buf) - 1, PSTR("%lu:%02lu"), m_wlTimeToGo / 60L, m_wlTimeToGo % 60L);
      tsc.renderField(&wlField, buf);
    }

    switch(state)                                                                 // The state machine starts
    {
      case WL_START:
        logger.log(F("%SStart"), funcIDStr);
        state = WL_IDLE;      
        break;
        
      case WL_IDLE:
        if ((beader.getInBatch() > 0) && (beader.getBeadInBatch() > 0L))
        {
          state = WL_GLUE_ACTIVE;
        }
        break;
        
      case WL_GLUE_ACTIVE:
        if (m_wlTimeToGo == 0)
        {
          io.setOutputPin(IO_DOUT_WARNING_BUZZER, HIGH);                          // Sound the buzzer
          dwellTimer = millis();
          state = WL_DWELL;
        }
        break;
        
      case WL_DWELL:                                                              // Wait for WL time to be set
        if ((millis() - dwellTimer) > 60000)
        {
          io.setOutputPin(IO_DOUT_WARNING_BUZZER, LOW);                           // Turn off the buzzer
          state = WL_WAIT_PRESS;
        }
        
        if (digitalRead(IO_DIN_PRESS_CYCLE_IN_PROG) == HIGH)
        {
          io.setOutputPin(IO_DOUT_WARNING_BUZZER, LOW);                           // Turn off the buzzer
          state = WL_DONE;
        }
        break;
        
      case WL_WAIT_PRESS:
        if (beader.getInBatch() == 0)
        {
          state = WL_DONE;
        }
        break;
      
      case WL_ERROR:
        break;
        
      case WL_DONE:
        state = WL_IDLE;
        break;
        
      default:
        break;
    }  

    if (state != lastState)
    {
      logger.log(F("%Sstate %d -> %d"), funcIDStr, lastState, state);
      lastState = state;
    }
  }
}
  
//*****************************************************
// manageStartBeading.
// Stops all the beading activities.
// Returns:
//    0   - Done
//    1   - in progress
//    -1  - Error
//*****************************************************

int guiClass::manageStartBeading(bool doBegin)
{
  const char* funcIDStr = PSTR("manageStartBeading: ");
  static manageStartBeadingStates_t state;                                                    // State variable
  static manageStartBeadingStates_t lastState;                                                // Last state to detect changes
  uint32_t exitTimer;
  static uint32_t timerStart;                                                                 // Timer
  size_t pageSize;
  touchscreen::pageFieldDef_t* field;
  int retVal = 1;
  
  if (doBegin == true)                                                                        // doBegin true to initialise
  {
    logger.log(F("%SStart"), funcIDStr);
    state = MGB_START;
    lastState = MGB_START;
  }
  
  switch(state)
  {
    case MGB_START:
      tsc.renderPage(proceedPageDef, sizeof(proceedPageDef));
      state = MGB_PROCEED;
      break;
            
    case MGB_PROCEED:
      if(field = tsc.isTouchedField())                                                            // React to the STOP button
      {
        switch(field->id)
        {
          case guiClass::PID_PROCEED:
            io.setOutputPin(IO_DOUT_SEALER_TRAY, LOW);                                            // Nozzles forward
            timerStart = millis();
            tsc.renderPage(startBeadingStopPageDef, sizeof(startBeadingStopPageDef));
            state = MGB_MOVE_NOZZLES_FWD_DELAY;
            break;
        }
      }
      break;
            
    case MGB_MOVE_NOZZLES_FWD_DELAY:
      if ((millis() - timerStart) > configGetDelay(9))
      {
        io.setOutputPin(IO_DOUT_PUMP_MOTOR_FWD, LOW);
        timerStart = millis();
        logger.log(F("%SDelayed out Nozzles"), funcIDStr);
        state = MGB_PUMP_ON_DELAY;
      }
      
      if(field = tsc.isTouchedField())                                                            // React to the STOP button
      {
        switch(field->id)
        {
          case guiClass::PID_STOP:
            gui.emergencyStop();
            logger.log(F("%SStop touched"), funcIDStr);
            state = MGB_ERROR;
            break;
        }
      }
      break;
            
    case MGB_PUMP_ON_DELAY:
      if ((millis() - timerStart) > configGetDelay(1))
      {
        io.setOutputPin(IO_DOUT_BEAD_NOZZLES, LOW);
        timerStart = millis();
        logger.log(F("%SDelayed out Pump on"), funcIDStr);
        state = MGB_NOZZLE_VALVES_ON_DELAY;
      }
      
      if(field = tsc.isTouchedField())                                                            // React to the STOP button
      {
        switch(field->id)
        {
          case guiClass::PID_STOP:
            gui.emergencyStop();
            logger.log(F("%SStop touched"), funcIDStr);
            state = MGB_ERROR;
            break;
        }
      }
      break;
            
    case MGB_NOZZLE_VALVES_ON_DELAY:
      if ((millis() - timerStart) > configGetDelay(8))
      {
        logger.log(F("%SDelayed out Nozzle valves on"), funcIDStr);
        state = MGB_DONE;
      }
      
      if(field = tsc.isTouchedField())                                                            // React to the STOP button
      {
        switch(field->id)
        {
          case guiClass::PID_STOP:
            gui.emergencyStop();
            logger.log(F("%SStop touched"), funcIDStr);
            state = MGB_ERROR;
            break;
        }
      }
      break;
            
    case MGB_ERROR:
      logger.log(F("%SError"), funcIDStr);
      state = MGB_START;
      retVal = -1;
      break;
            
    case MGB_DONE:
      logger.log(F("%SDone"), funcIDStr);
      state = MGB_START;
      retVal = 0;
      break;
  }
    
  if (state != lastState)
  {
    logger.log(F("%Sstate %d -> %d"), funcIDStr, lastState, state);
    lastState = state;
  }

  return retVal;
}

//*****************************************************
// manageStopBeading.
// Stops all the beading activities.
// Returns:
//    0   - Done
//    1   - in progress
//    -1  - Error
//*****************************************************

int guiClass::manageStopBeading(bool doBegin)
{
  const char* funcIDStr = PSTR("manageStopBeading: ");
  static manageStopBeadingStates_t state;                                                     // State variable
  static manageStopBeadingStates_t lastState;                                                 // Last state to detect changes
  uint32_t exitTimer;
  static uint32_t timerStart;                                                                 // Timer
  size_t pageSize;
  touchscreen::pageFieldDef_t* field;
  int retVal = 1;
  
  if (doBegin == true)                                                                        // doBegin true to initialise
  {
    logger.log(F("%SStart"), funcIDStr);
    state = MSB_START;
    lastState = MSB_START;
  }

  switch(state)
  {
    case MSB_START:
      io.setOutputPin(IO_DOUT_PUMP_MOTOR_FWD, HIGH);
      timerStart = millis();
      tsc.renderPage(startBeadingStopPageDef, sizeof(startBeadingStopPageDef));
      state = MSB_PUMP_FWD_OFF_DELAY;
      break;
      
    case MSB_PUMP_FWD_OFF_DELAY:
      if ((millis() - timerStart) > configGetDelay(2))
      {
        io.setOutputPin(IO_DOUT_PUMP_MOTOR_REV, LOW);
        timerStart = millis();
        logger.log(F("%SDelayed out Pump fwd off"), funcIDStr);
        state = MSB_PUMP_REV_ON_DELAY;
      }
      
      if(field = tsc.isTouchedField())                                                            // React to the STOP button
      {
        switch(field->id)
        {
          case guiClass::PID_STOP:
            gui.emergencyStop();
            logger.log(F("%SStop touched"), funcIDStr);
            state = MSB_ERROR;
            break;
        }
      }
      break;
      
    case MSB_PUMP_REV_ON_DELAY:
      if ((millis() - timerStart) > configGetDelay(3))
      {
        io.setOutputPin(IO_DOUT_PUMP_MOTOR_REV, HIGH);
        timerStart = millis();
        logger.log(F("%SDelayed out Pump rev on"), funcIDStr);
        state = MSB_PUMP_REV_OFF_DELAY;
      }
      
      if(field = tsc.isTouchedField())                                                            // React to the STOP button
      {
        switch(field->id)
        {
          case guiClass::PID_STOP:
            gui.emergencyStop();
            logger.log(F("%SStop touched"), funcIDStr);
            state = MSB_ERROR;
            break;
        }
      }
      break;
      
    case MSB_PUMP_REV_OFF_DELAY:
      if ((millis() - timerStart) > configGetDelay(2))
      {
        io.setOutputPin(IO_DOUT_BEAD_NOZZLES, HIGH);
        timerStart = millis();
        logger.log(F("%SDelayed out Pump rev off"), funcIDStr);
        state = MSB_NOZZLES_CLOSE_DELAY;
      }
      
      if(field = tsc.isTouchedField())                                                            // React to the STOP button
      {
        switch(field->id)
        {
          case guiClass::PID_STOP:
            gui.emergencyStop();
            logger.log(F("%SStop touched"), funcIDStr);
            state = MSB_ERROR;
            break;
        }
      }
      break;
      
    case MSB_NOZZLES_CLOSE_DELAY:
      if ((millis() - timerStart) > configGetDelay(8))
      {
        io.setOutputPin(IO_DOUT_SEALER_TRAY, HIGH);
        timerStart = millis();
        logger.log(F("%SDelayed out Nozzles close"), funcIDStr);
        state = MSB_NOZZLES_TO_SEALER_DELAY;
      }
      
      if(field = tsc.isTouchedField())                                                            // React to the STOP button
      {
        switch(field->id)
        {
          case guiClass::PID_STOP:
            gui.emergencyStop();
            logger.log(F("%SStop touched"), funcIDStr);
            state = MSB_ERROR;
            break;
        }
      }
      break;
      
    case MSB_NOZZLES_TO_SEALER_DELAY:
      if ((millis() - timerStart) > configGetDelay(9))
      {
        logger.log(F("%SDelayed out Nozzles to sealer"), funcIDStr);
        state = MSB_DONE;
      }
      
      if(field = tsc.isTouchedField())                                                            // React to the STOP button
      {
        switch(field->id)
        {
          case guiClass::PID_STOP:
            gui.emergencyStop();
            logger.log(F("%SStop touched"), funcIDStr);
            state = MSB_ERROR;
            break;
        }
      }
      break;
      
    case MSB_ERROR:
      logger.log(F("%SError"), funcIDStr);
      state = MSB_START;
      retVal = -1;
      break;
      
    case MSB_DONE:
      logger.log(F("%SError"), funcIDStr);
      state = MSB_START;
      retVal = 0;
      break;
      
    default:
      break;
}
  
  if (state != lastState)
  {
    logger.log(F("%Sstate %d -> %d"), funcIDStr, lastState, state);
    lastState = state;
  }

  return retVal;
}