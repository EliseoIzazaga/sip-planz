//*****************************************************
//  motor_test.cpp
//  Runs the 'Motor Test'
//*****************************************************
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "siplaminator.h"
#include "touchscreen.h"
#include "gui.h"
#include "motor_test.h"
#include "io.h"
#include "Logger.h"

//*****************************************************
// Select motor test page.
//*****************************************************
const struct touchscreen::pageFieldDef_t motorTestClass::selectMotorTestPageDef[] PROGMEM =
{
  {guiClass::PID_POSITION,  touchscreen::BUTTON,  60, 10,  200, 60,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::positionStr,   40, 38, touchscreen::ACTIVE},
  {guiClass::PID_PROXIMITY, touchscreen::BUTTON,  60, 80,  200, 60,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::ProximityStr,  40, 38, touchscreen::ACTIVE},
  {guiClass::PID_DISTANCE,  touchscreen::BUTTON,  60, 150, 200, 60,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::distanceStr,   40, 38, touchscreen::ACTIVE}
};

//*****************************************************
// Select motor Continue/Return page.
//*****************************************************
const struct touchscreen::pageFieldDef_t motorTestClass::selectMotorContinueReturnPageDef[] PROGMEM =
{
  {guiClass::PID_CONTINUE,  touchscreen::BUTTON,  60, 10,  200, 100,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::continueStr, 40, 60, touchscreen::ACTIVE},
  {guiClass::PID_RETURN,    touchscreen::BUTTON,  60, 120, 200, 100,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::returnStr,   40, 60, touchscreen::ACTIVE}
};

//*****************************************************
// Select motor Continue page.
//*****************************************************
const struct touchscreen::pageFieldDef_t motorTestClass::selectMotorContinuePageDef[] PROGMEM =
{
  {guiClass::PID_CONTINUE,  touchscreen::BUTTON,  60, 60,  200, 100,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::continueStr, 40, 60, touchscreen::ACTIVE}
};

//*****************************************************
// Repeat/End page.
//*****************************************************
const struct touchscreen::pageFieldDef_t motorTestClass::repeatEndPageDef[] PROGMEM =
{
  {guiClass::PID_REPEAT,  touchscreen::BUTTON,  60, 10,  200, 100,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::repeatStr, 40, 60, touchscreen::ACTIVE},
  {guiClass::PID_END,     touchscreen::BUTTON,  60, 120, 200, 100,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::endStr,    40, 60, touchscreen::ACTIVE}
};

//*****************************************************
// Distance test page.
//*****************************************************

const struct touchscreen::pageFieldDef_t motorTestClass::distanceTestPageDef[] PROGMEM =
{
  {guiClass::PID_TITLE, touchscreen::TEXT,    80, 40, 200, 70,  ILI9341_BLACK,  0,            ILI9341_WHITE, 1, (char*)guiClass::distanceStr, 45, 45, touchscreen::NOT_ACTIVE},
  {guiClass::PID_STOP,  touchscreen::BUTTON,  60, 60, 200, 100, ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::stopStr,     60, 60, touchscreen::ACTIVE},
};

motorTestClass::motorTestClass()
{
}

//*****************************************************
// begin.
//*****************************************************

void motorTestClass::begin()
{
}

//*****************************************************
// manageMTDistanceSwitch.
// Returns: 0   - Done
//          1   - ongoing
//          -1  - Error done - Used here for a return
//*****************************************************

int motorTestClass::manageMTDistanceSwitch(bool doBegin)
{
  const char* funcIDStr = PSTR("manageMTDistanceSwitch: ");
  static motorTDistanceSwitchStates_t state;
  static motorTDistanceSwitchStates_t lastState;
  touchscreen::pageFieldDef_t* field;
  int retVal = 1;
  
  if (doBegin == true)                                                                        // doBegin true to initialise
  {
    logger.log(F("%SStart"), funcIDStr);
    state = MTDW_START;
    lastState = MTDW_START;
  }
  
  switch(state)
  {
    case MTDW_START:
      gui.manageMotorToHome(gui.MOTOR_APPLIC, true);                                         // Send applicator home
      state = MTDW_APPLICATOR_TO_HOME;
      break;
      
    case MTDW_APPLICATOR_TO_HOME:
      state = (motorTDistanceSwitchStates_t)gui.runFSM(state, MTDW_MOVE_APPLICATOR_TO_SWITCH, MTDW_ERROR, gui.manageMotorToHome(gui.MOTOR_APPLIC));
      if (state == MTDW_MOVE_APPLICATOR_TO_SWITCH) 
      {
        tsc.renderPage(distanceTestPageDef, sizeof(distanceTestPageDef));
        io.setOutputPin(IO_DOUT_DISTANCE_PISTON, LOW);
        io.setOutputPin(IO_DOUT_APPLICATOR_MOTOR_FWD, LOW);
      }
      break;
      
    case MTDW_MOVE_APPLICATOR_TO_SWITCH:
      //if (digitalRead(IO_DIN_DISTANCE_SWITCH) == HIGH)
      if (digitalRead(IO_DIN_MOTOR_FEEDBACK) == HIGH)
      {
        io.setOutputPin(IO_DOUT_APPLICATOR_MOTOR_FWD, HIGH);
        io.setOutputPin(IO_DOUT_DISTANCE_PISTON, HIGH);
        io.setOutputPin(IO_DOUT_APPLICATOR_MOTOR_REV, LOW);  // ******* Is this correct? What turns it off? *******
        //logger.log(F("%SDistance switch ON"), funcIDStr);
        state = MTDW_DONE;
      }

      if(field = tsc.isTouchedField())                                                            // React to the STOP button
      {
        switch(field->id)
        {
          case guiClass::PID_STOP:
            io.setOutputPin(IO_DOUT_APPLICATOR_MOTOR_FWD, HIGH);                                 // Stop the motor
            gui.emergencyStop();
            logger.log(F("%SStop touched"), funcIDStr);
            state = MTDW_ERROR;
            break;
        }
      }
      break;
      
    case MTDW_ERROR:
      logger.log(F("%SError"), funcIDStr);
      state = MTDW_START;
      retVal = -1;
      break;
      
    case MTDW_DONE:
      logger.log(F("%SDone"), funcIDStr);
      retVal = 0;
      state = MTDW_START;
      break;
      
    default:
      state = MTDW_START;
      retVal = -1;
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
// manageMTDistanceSensor.
// Returns: 0   - Done
//          1   - ongoing
//          -1  - Error done - Used here for a return
//*****************************************************

int motorTestClass::manageMTDistanceSensor(bool doBegin)
{
  const char* funcIDStr = PSTR("manageMTDistanceSensor: ");
  static motorTDistanceSensorStates_t state;
  static motorTDistanceSensorStates_t lastState;
  touchscreen::pageFieldDef_t* field;
  int retVal = 1;
  
  if (doBegin == true)                                                                        // doBegin true to initialise
  {
    logger.log(F("%SStart"), funcIDStr);
    state = MTDS_START;
    lastState = MTDS_START;
  }
  
  switch(state)
  {
    case MTDS_START:
      gui.manageMotorToHome(gui.MOTOR_APPLIC, true);                                         // Send applicator home
      state = MTDS_APPLICATOR_TO_HOME;
      break;
    case MTDS_APPLICATOR_TO_HOME:
      state = (motorTDistanceSensorStates_t)gui.runFSM(state, MTDS_APPLICATOR_TO_BEAD, MTDS_ERROR, gui.manageMotorToHome(gui.MOTOR_APPLIC));
      if (state == MTDS_APPLICATOR_TO_BEAD) gui.manageApplicatorToBead(true);                // Send applicator home
      break;
    case MTDS_APPLICATOR_TO_BEAD:
      state = (motorTDistanceSensorStates_t)gui.runFSM(state, MTDS_REPEAT_END, MTDS_ERROR, gui.manageApplicatorToBead());
      if (state == MTDS_REPEAT_END) tsc.renderPage(repeatEndPageDef, sizeof(repeatEndPageDef));// Render the page
      break;
    case MTDS_REPEAT_END:
      if(field = tsc.isTouchedField())
      {
        switch(field->id)
        {
          case guiClass::PID_REPEAT:
            state = MTDS_START;
            break;
          case guiClass::PID_END:                                                        // Treat a return like an error
            state = MTDS_DONE;
            break;
        }
      }
      break;
    case MTDS_ERROR:
      logger.log(F("%SError"), funcIDStr);
      state = MTDS_START;
      retVal = -1;
      break;
    case MTDS_DONE:
      logger.log(F("%SDone"), funcIDStr);
      retVal = 0;
      state = MTDS_START;
      break;
    default:
      state = MTDS_START;
      retVal = -1;
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
// manageMTRailPosition.
// Returns: 0   - Done
//          1   - ongoing
//          -1  - Error done - Used here for a return
//*****************************************************

int motorTestClass::manageMTRailPosition(bool doBegin)
{
  const char* funcIDStr = PSTR("manageMTRailPosition: ");
  static motorTMRailPositionStates_t state;
  static motorTMRailPositionStates_t lastState;
  //static uint32_t timerStart;
  touchscreen::pageFieldDef_t* field;
  int retVal = 1;
  
  if (doBegin == true)                                                                        // doBegin true to initialise
  {
    logger.log(F("%SStart"), funcIDStr);
    tsc.renderPage(selectMotorContinueReturnPageDef, sizeof(selectMotorContinueReturnPageDef));
    state = MTMRP_START;
    lastState = MTMRP_START;
  }
  
  switch(state)
  {
    case MTMRP_START:
      if(field = tsc.isTouchedField())
      {
        switch(field->id)
        {
          case guiClass::PID_CONTINUE:
            gui.manageRailMove(gui.BEAD_TO_END, true);                                      //  Move to End
            state = MTMRP_BEAD_TO_END;
            break;
          case guiClass::PID_RETURN:                                                        // Treat a return like an error
            logger.log(F("%SReturn Key as error"), funcIDStr);
            state = MTMRP_ERROR;
            break;
        }
      }
      break;
    case MTMRP_BEAD_TO_END:
      state = (motorTMRailPositionStates_t)gui.runFSM(state, MTMRP_CONTINUE_1, MTMRP_ERROR, gui.manageRailMove(gui.BEAD_TO_END));
      if (state == MTMRP_CONTINUE_1)
      {
        tsc.renderPage(selectMotorContinuePageDef, sizeof(selectMotorContinuePageDef));
      }
      break;
    case MTMRP_CONTINUE_1:
      if(field = tsc.isTouchedField())
      {
        switch(field->id)
        {
          case guiClass::PID_CONTINUE:
            gui.manageRailMove(gui.END_TO_BEAD, true);                                      //  Move to End
            state = MTMRP_END_TO_BEAD;
            break;
        }
      }
      break;
    case MTMRP_END_TO_BEAD:
      state = (motorTMRailPositionStates_t)gui.runFSM(state, MTMRP_CONTINUE_2, MTMRP_ERROR, gui.manageRailMove(gui.END_TO_BEAD));
      if (state == MTMRP_CONTINUE_2)
      {
        tsc.renderPage(selectMotorContinuePageDef, sizeof(selectMotorContinuePageDef));
      }
      break;
    case MTMRP_CONTINUE_2:
      if(field = tsc.isTouchedField())
      {
        switch(field->id)
        {
          case guiClass::PID_CONTINUE:
            state = MTMRP_DONE;
            break;
        }
      }
      break;
    case MTMRP_ERROR:
      logger.log(F("%SError"), funcIDStr);
      retVal = -1;
      break;
    case MTMRP_DONE:
      retVal = 0;
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

//*****************************************************
// manageMotorTest.
// Returns: 0   - Done
//          1   - ongoing
//          -1  - Error done
//*****************************************************

int motorTestClass::manageMotorTest(bool doBegin)
{
  const char* funcIDStr = PSTR("manageMotorTest: ");
  static motorTestManagerStates_t state = MTM_START;
  static motorTestManagerStates_t lastState = MTM_INVALID;
  //static uint32_t timerStart;
  touchscreen::pageFieldDef_t* field;
  int ret = 1;

  if (doBegin) state = MTM_START;

  switch(state)
  {
    case MTM_START:                                                                 // Starts the main page
      logger.log(F("%SStartup"), funcIDStr);
      gui.manageParameterSetup(true);
      state = MTM_SETUP;
      break;
      
    case MTM_SETUP:
      state = (motorTestManagerStates_t)gui.runFSM(state, MTM_RAIL_TO_HOME, MTM_ERROR, gui.manageParameterSetup());
      if (state == MTM_RAIL_TO_HOME) gui.manageMotorToHome(gui.MOTOR_RAIL, true);    // Send rail motor home
      break;
      
    case MTM_RAIL_TO_HOME:                                                          // Wait for rail to go home
      state = (motorTestManagerStates_t)gui.runFSM(state, MTM_RAIL_TO_BEAD, MTM_ERROR, gui.manageMotorToHome(gui.MOTOR_RAIL));
      if (state == MTM_RAIL_TO_BEAD) gui.manageRailMove(gui.HOME_TO_BEAD, true);    // Send rail motor to bead position
      break;
      
    case MTM_RAIL_TO_BEAD:                                                          // Send rail motor to bead position
      state = (motorTestManagerStates_t)gui.runFSM(state, MTM_SELECT_TEST, MTM_ERROR, gui.manageRailMove(gui.HOME_TO_BEAD));
      if (state == MTM_SELECT_TEST) 
      {
        tsc.renderPage(selectMotorTestPageDef, sizeof(selectMotorTestPageDef));
      }
      break;

    case MTM_SELECT_TEST:
      if(field = tsc.isTouchedField())
      {
        switch(field->id)
        {
          case guiClass::PID_POSITION:
            manageMTRailPosition(true);
            state = MTM_RAIL_TEST;
            break;
          case guiClass::PID_PROXIMITY:
            manageMTDistanceSensor(true);
            state = MTM_PROXIMITY_TEST;
            break;
          case guiClass::PID_DISTANCE:
            manageMTDistanceSwitch(true);
            state = MTM_DISTANCE_SWITCH_TEST;
            break;
        }
      }

      break;
      
    case MTM_RAIL_TEST:
      state = (motorTestManagerStates_t)gui.runFSM(state, MTM_DONE, MTM_ERROR, manageMTRailPosition());
      break;
      
    case MTM_PROXIMITY_TEST:
      state = (motorTestManagerStates_t)gui.runFSM(state, MTM_DONE, MTM_ERROR, manageMTDistanceSensor());
      break;
      
    case MTM_DISTANCE_SWITCH_TEST:
      state = (motorTestManagerStates_t)gui.runFSM(state, MTM_DONE, MTM_ERROR, manageMTDistanceSwitch());
      break;
            
    case MTM_ERROR:
      logger.log(F("%SError"), funcIDStr);
      ret = -1; 
      break;
      
    case MTM_DONE:
      logger.log(F("%SDone"), funcIDStr);
      ret = 0; 
      break;
  }
    
  if (state != lastState)
  {
    logger.log(F("%Sstate %d -> %d"), funcIDStr, lastState, state);
    lastState = state;
  }

  return ret;
}



motorTestClass motorTest;                             // The one and only global Motor test
