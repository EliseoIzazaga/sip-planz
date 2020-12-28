//*****************************************************
//  wet_test.cpp
//  Runs the 'Wet Test'
//*****************************************************
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "siplaminator.h"
#include "touchscreen.h"
#include "gui.h"
#include "wet_test.h"
#include "io.h"
#include "Logger.h"

//*****************************************************
// Select motor test page.
//*****************************************************
const struct touchscreen::pageFieldDef_t wetTestClass::selectWetGlueMisterPageDef[] PROGMEM =
{
  {guiClass::PID_GLUE,      touchscreen::BUTTON,  60, 10,  200, 60,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::glueStr,   56, 38, touchscreen::ACTIVE},
  {guiClass::PID_MISTER,    touchscreen::BUTTON,  60, 80,  200, 60,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::misterStr, 50, 38, touchscreen::ACTIVE},
  {guiClass::PID_EXIT,      touchscreen::BUTTON,  60, 150, 200, 60,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::exitStr,   56, 38, touchscreen::ACTIVE}
};

//*****************************************************
// Start/return wet test page.
//*****************************************************
const struct touchscreen::pageFieldDef_t wetTestClass::startReturnPageDef[] PROGMEM =
{
  {guiClass::PID_START,  touchscreen::BUTTON,  60, 10,  200, 100,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::startStr, 40, 60, touchscreen::ACTIVE},
  {guiClass::PID_RETURN, touchscreen::BUTTON,  60, 120, 200, 100,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::returnStr,   40, 60, touchscreen::ACTIVE}
};

//*****************************************************
// Wet glue test STOP page.
//*****************************************************
const struct touchscreen::pageFieldDef_t wetTestClass::wetGlueTestStopPageDef[] PROGMEM =
{
  {guiClass::PID_STOP,  touchscreen::BUTTON,  60, 60,  200, 100,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::stopStr, 65, 60, touchscreen::ACTIVE},
};

wetTestClass::wetTestClass()
{
}

void wetTestClass::begin()
{
}

//*****************************************************
// manageWetMisterTest.
// Returns: 0   - Done
//          1   - ongoing
//          -1  - Error done
//*****************************************************

int wetTestClass::manageWetMisterTest(bool doBegin)
{
  const char* funcIDStr = PSTR("manageWetMisterTest: ");
  static wetMisterTestManagerStates_t state;
  static wetMisterTestManagerStates_t lastState;
  touchscreen::pageFieldDef_t* field;
  int ret = 1;

  if (doBegin) 
  {
    state = WMT_START;
    lastState = WMT_START;
    logger.log(F("%SStart"), funcIDStr);
  }

  switch(state)
  {
    case WMT_START:
      tsc.renderPage(startReturnPageDef, sizeof(startReturnPageDef));
      state = WMT_START_RETURN;
      break;
      
    case WMT_START_RETURN:
      if(field = tsc.isTouchedField())
      {
        switch(field->id)
        {
          case guiClass::PID_START:
            io.setOutputPin(IO_DOUT_FAN_MISTER, LOW);
            tsc.renderPage(wetGlueTestStopPageDef, sizeof(wetGlueTestStopPageDef));
            state = WMT_STOP_PAGE;
            break;
          case guiClass::PID_RETURN:
            state = WMT_DONE;
            break;
        }
      }
      break;
      
    case WMT_STOP_PAGE:
      if(field = tsc.isTouchedField())
      {
        switch(field->id)
        {
          case guiClass::PID_STOP:
            io.setOutputPin(IO_DOUT_FAN_MISTER, HIGH);
            logger.log(F("%SStop misting"), funcIDStr);
            state = WMT_DONE;
            break;
        }
      }
      break;
      
    case WMT_ERROR:
      logger.log(F("%SError"), funcIDStr);
      ret = -1; 
      break;
      
    case WMT_DONE:
      logger.log(F("%SDone"), funcIDStr);
      ret = 0; 
      break;
      
    default:
      break;
  }
  
  if (state != lastState)
  {
    logger.log(F("%Sstate %d -> %d"), funcIDStr, lastState, state);
    lastState = state;
  }

  return ret;
}


//*****************************************************
// manageWetGlueTest.
// Returns: 0   - Done
//          1   - ongoing
//          -1  - Error done
//*****************************************************

int wetTestClass::manageWetGlueTest(bool doBegin)
{
  const char* funcIDStr = PSTR("manageWetGlueTest: ");
  static wetGlueTestManagerStates_t state;
  static wetGlueTestManagerStates_t lastState;
  touchscreen::pageFieldDef_t* field;
  int ret = 1;

  if (doBegin) 
  {
    state = WGT_START;
    lastState = WGT_START;
    logger.log(F("%SStartup"), funcIDStr);
  }

  switch(state)
  {
    case WGT_START:
      tsc.renderPage(startReturnPageDef, sizeof(startReturnPageDef));
      state = WGT_START_RETURN;
      break;
      
    case WGT_START_RETURN:
      if(field = tsc.isTouchedField())
      {
        switch(field->id)
        {
          case guiClass::PID_START:
            gui.manageStartBeading(true);
            state = WGT_START_BEADING;
            break;
          case guiClass::PID_RETURN:
            state = WGT_DONE;
            break;
        }
      }
      break;
      
    case WGT_START_BEADING:
      state = (wetGlueTestManagerStates_t)gui.runFSM(state, WGT_STOP_PAGE, WGT_ERROR, gui.manageStartBeading());
      if (state == WGT_STOP_PAGE) tsc.renderPage(wetGlueTestStopPageDef, sizeof(wetGlueTestStopPageDef));
      break;
      
    case WGT_STOP_PAGE:
      if(field = tsc.isTouchedField())
      {
        switch(field->id)
        {
          case guiClass::PID_STOP:
            logger.log(F("%SStop beading"), funcIDStr);
            gui.manageStopBeading(true);
            state = WGT_STOP_BEADING;
            break;
        }
      }
      break;
      
    case WGT_STOP_BEADING:
      state = (wetGlueTestManagerStates_t)gui.runFSM(state, WGT_DONE, WGT_ERROR, gui.manageStopBeading());
      break;
      
    case WGT_ERROR:
      logger.log(F("%SError"), funcIDStr);
      ret = -1; 
      break;
      
    case WGT_DONE:
      logger.log(F("%SDone"), funcIDStr);
      ret = 0; 
      break;
      
    default:
      break;
  }
  
  if (state != lastState)
  {
    logger.log(F("%Sstate %d -> %d"), funcIDStr, lastState, state);
    lastState = state;
  }

  return ret;
}

//*****************************************************
// manageWetTest.
// Returns: 0   - Done
//          1   - ongoing
//          -1  - Error done
//*****************************************************

int wetTestClass::manageWetTest(bool doBegin)
{
  const char* funcIDStr = PSTR("manageWetTest: ");
  static wetTestManagerStates_t state;
  static wetTestManagerStates_t lastState;
  touchscreen::pageFieldDef_t* field;
  int ret = 1;

  if (doBegin) 
  {
    state = WTM_START;
    lastState = WTM_START;
    logger.log(F("%SStartup"), funcIDStr);
  }

  switch(state)
  {
    case WTM_START:                                                                 // Starts the main page
      gui.manageMotorToHome(gui.MOTOR_RAIL, true);
      state = WTM_RAIL_TO_HOME;
      break;
      
    case WTM_RAIL_TO_HOME:
      state = (wetTestManagerStates_t)gui.runFSM(state, WTM_RAIL_TO_WET, WTM_ERROR, gui.manageMotorToHome(gui.MOTOR_RAIL));
      if (state == WTM_RAIL_TO_WET) gui.manageRailMove(gui.HOME_TO_WET, true);
      break;
      
    case WTM_RAIL_TO_WET:                                                           // Wait for rail to get to wet
      state = (wetTestManagerStates_t)gui.runFSM(state, WTM_APPLIC_TO_HOME, WTM_ERROR, gui.manageRailMove(gui.HOME_TO_WET));
      if (state == WTM_APPLIC_TO_HOME) gui.manageMotorToHome(gui.MOTOR_APPLIC, true);
      break;
     
    case WTM_APPLIC_TO_HOME:
      state = (wetTestManagerStates_t)gui.runFSM(state, WTM_APPLIC_TO_BEAD, WTM_ERROR, gui.manageMotorToHome(gui.MOTOR_APPLIC));
      if (state == WTM_APPLIC_TO_BEAD) gui.manageRailMove(gui.HOME_TO_BEAD, true);
      break;
      
    case WTM_APPLIC_TO_BEAD:
      state = (wetTestManagerStates_t)gui.runFSM(state, WTM_RENDER_GLUE_MISTER, WTM_ERROR, gui.manageRailMove(gui.HOME_TO_BEAD));
      break;      
      
    case WTM_RENDER_GLUE_MISTER:
      tsc.renderPage(selectWetGlueMisterPageDef, sizeof(selectWetGlueMisterPageDef));
      state = WTM_SELECT_GLUE_MISTER;
      break;
      
    case WTM_SELECT_GLUE_MISTER:
      if(field = tsc.isTouchedField())
      {
        switch(field->id)
        {
          case guiClass::PID_GLUE:
            logger.log(F("%SGLUE TEST"), funcIDStr);
            manageWetGlueTest(true);
            state = WTM_GLUE_TEST;
            break;
          case guiClass::PID_MISTER:
            logger.log(F("%SMISTER TEST"), funcIDStr);
            manageWetMisterTest(true);
            state = WTM_MISTER_TEST;
            break;
          case guiClass::PID_EXIT:
            logger.log(F("%SEXIT WET TEST"), funcIDStr);
            state = WTM_DONE;
            break;
        }
      }
      break;   
      
    case WTM_GLUE_TEST:
      state = (wetTestManagerStates_t)gui.runFSM(state, WTM_RENDER_GLUE_MISTER, WTM_ERROR, manageWetGlueTest());
      break;
      
    case WTM_MISTER_TEST:
      state = (wetTestManagerStates_t)gui.runFSM(state, WTM_RENDER_GLUE_MISTER, WTM_ERROR, manageWetMisterTest());
      break;
      
    case WTM_ERROR:
      logger.log(F("%SError"), funcIDStr);
      ret = -1; 
      break;
      
    case WTM_DONE:
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



wetTestClass wetTest;                             // The one and only global Wet test
