//*****************************************************
//  gui.cpp
//  Handles all interaction with the user through the GUI.
//*****************************************************
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <string.h>
#include "siplaminator.h"
#include "config.h"
#include "touchscreen.h"
#include "gui.h"
#include "motor_test.h"
#include "wet_test.h"
#include "beading.h"
#include "io.h"
#include "Logger.h"
#include "enc28j60_ethernet.h"

//*****************************************************
// Text strings for pages.
//*****************************************************

const char guiClass::beadingStr[] PROGMEM                   = "BEADING";
const char guiClass::motorTestStr[] PROGMEM                 = "MOTOR TEST";
const char guiClass::wetTestStr[] PROGMEM                   = "WET TEST";
const char guiClass::testStr[] PROGMEM                      = "TEST";
const char guiClass::nextStr[] PROGMEM                      = "NEXT";
const char guiClass::returnStr[] PROGMEM                    = "RETURN";
const char guiClass::confirmedStr[] PROGMEM                 = "CONFIRMED";
const char guiClass::modifyStr[] PROGMEM                    = "MODIFY";
const char guiClass::powerFailureStr[] PROGMEM              = "POWER FAILURE";
const char guiClass::panelSizeStr[] PROGMEM                 = "PANEL SIZE";
const char guiClass::offsetTopStr[] PROGMEM                 = "OFFSET TOP";
const char guiClass::offsetBottomStr[] PROGMEM              = "OFFSET BOT";
const char guiClass::railStr[] PROGMEM                      = "RAIL";
const char guiClass::applicStr[] PROGMEM                    = "APPLICATOR";
const char guiClass::applicRailStr[] PROGMEM                = "APPLICATOR/RAIL";
const char guiClass::toStartStr[] PROGMEM                   = "TO START";
const char guiClass::toHomeStr[] PROGMEM                    = "TO HOME";
const char guiClass::railToBeadStr[] PROGMEM                = "RAIL TO BEAD";
const char guiClass::railToEndStr[] PROGMEM                 = "RAIL TO END";
const char guiClass::railToWetStr[] PROGMEM                 = "RAIL TO WET";
const char guiClass::railToHomeStr[] PROGMEM                = "RAIL TO HOME";
const char guiClass::railToStartStr[] PROGMEM               = "RAIL TO START";
const char guiClass::applicatorToHomeStr[] PROGMEM          = "APPLIC TO HOME";
const char guiClass::applicatorToStartStr[] PROGMEM         = "APPLIC TO START";
const char guiClass::settingUpStr[] PROGMEM                 = "SETTING UP";
const char guiClass::waitingStr[] PROGMEM                   = "Waiting";
const char guiClass::rightChevronStr[] PROGMEM              = ">>";
const char guiClass::blankStr[] PROGMEM                     = "";
const char guiClass::emergencyStopStr[] PROGMEM             = "EMERGENCY STOP";
const char guiClass::pleaseWaitStr[] PROGMEM                = "PLEASE WAIT...";
const char guiClass::startStr[] PROGMEM                     = "START";
const char guiClass::stopStr[] PROGMEM                      = "STOP";
const char guiClass::resetStr[] PROGMEM                     = "RESET";
const char guiClass::resettingStr[] PROGMEM                 = "RESETTING";
const char guiClass::positionStr[] PROGMEM                  = "POSITION";
const char guiClass::ProximityStr[] PROGMEM                 = "PROXIMITY";
const char guiClass::distanceStr[] PROGMEM                  = "DISTANCE";
const char guiClass::continueStr[] PROGMEM                  = "CONTINUE";
const char guiClass::repeatStr[] PROGMEM                    = "REPEAT";
const char guiClass::endStr[] PROGMEM                       = "END";
const char guiClass::glueStr[] PROGMEM                      = "GLUE";
const char guiClass::misterStr[] PROGMEM                    = "MISTER";
const char guiClass::exitStr[] PROGMEM                      = "EXIT";
const char guiClass::proceedStr[] PROGMEM                   = "PROCEED";
const char guiClass::startBeadingStr[] PROGMEM              = "START BEAD";

//uint32_t guiClass::m_workLifeStartTime = 0;                                  // Time that the current adheshive application started

//*****************************************************
// Start page.
//*****************************************************
const struct touchscreen::pageFieldDef_t guiClass::startPageDef[] PROGMEM =
{
  {PID_START,     touchscreen::BUTTON,  60, 60,  200, 100,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::startStr,      60, 60, touchscreen::ACTIVE}
};

//*****************************************************
// Reset page.
//*****************************************************
const struct touchscreen::pageFieldDef_t guiClass::resetPageDef[] PROGMEM =
{
  {PID_RESET,     touchscreen::BUTTON,  60, 60,  200, 100,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::resetStr,      65, 60, touchscreen::ACTIVE}
};

//*****************************************************
// Resetting page.
//*****************************************************
const struct touchscreen::pageFieldDef_t guiClass::resettingPageDef[] PROGMEM =
{
  {PID_FIELD_1, touchscreen::TEXT,  80, 60,    200, 70,  ILI9341_BLACK,  0,  ILI9341_WHITE, 1, (char*)guiClass::resettingStr,   45, 45, touchscreen::NOT_ACTIVE}
};


//*****************************************************
// Beading/Test page.
//*****************************************************
const struct touchscreen::pageFieldDef_t guiClass::beadingTestPageDef[] PROGMEM =
{
  {PID_BEADING,  touchscreen::BUTTON,  60, 10,   200, 100,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::beadingStr,   45, 60, touchscreen::ACTIVE},
  {PID_TEST,     touchscreen::BUTTON,  60, 120,  200, 100,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::testStr,      65, 60, touchscreen::ACTIVE}
};


//*****************************************************
// Test page.
//*****************************************************
const struct touchscreen::pageFieldDef_t guiClass::testPageDef[] PROGMEM =
{
  {PID_MOTOR_TEST,  touchscreen::BUTTON,  60, 10,   200, 100,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::motorTestStr, 20, 60, touchscreen::ACTIVE},
  {PID_WET_TEST,    touchscreen::BUTTON,  60, 120,  200, 100,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::wetTestStr,   35, 60, touchscreen::ACTIVE}
};

//*****************************************************
// Power Fail page
//*****************************************************

const struct touchscreen::pageFieldDef_t guiClass::powerFailPageDef[] PROGMEM =
{
  {PID_FIELD_1, touchscreen::TEXT,  80, 60,    200, 70,  ILI9341_BLACK,  0,  ILI9341_WHITE, 1, (char*)guiClass::powerFailureStr,   45, 45, touchscreen::NOT_ACTIVE},
};

//*****************************************************
// Emergency Stop page
//*****************************************************

const struct touchscreen::pageFieldDef_t guiClass::emergencyStopPageDef[] PROGMEM =
{
  {PID_FIELD_1, touchscreen::TEXT,  60, 60,    200, 70,  ILI9341_BLACK,  0,  ILI9341_WHITE, 1, (char*)guiClass::emergencyStopStr,   45, 45, touchscreen::NOT_ACTIVE},
};

//*****************************************************
// Please Wait page
//*****************************************************

const struct touchscreen::pageFieldDef_t guiClass::pleaseWaitPageDef[] PROGMEM =
{
  {PID_FIELD_1, touchscreen::TEXT,  60, 60,    200, 70,  ILI9341_BLACK,  0,  ILI9341_WHITE, 1, (char*)guiClass::pleaseWaitStr,   45, 45, touchscreen::NOT_ACTIVE},
};

//*****************************************************
// Setup pages.
//*****************************************************

const struct touchscreen::pageFieldDef_t guiClass::setupPanelSizePageDef[] PROGMEM =
{
  {PID_TITLE,    touchscreen::TEXT,         80, 30,  200, 70,  ILI9341_BLACK,  0,  ILI9341_WHITE, 1, (char*)guiClass::panelSizeStr,               45, 45, touchscreen::NOT_ACTIVE},
  {PID_SELECTED, touchscreen::NATIVE_TEXT,  80, 50,  200, 20,  ILI9341_BLACK,  0,  ILI9341_WHITE, 2, (char*)guiClass::blankStr,                   45, 45, touchscreen::NOT_ACTIVE},
  {PID_RCHEVRON, touchscreen::BUTTON,       60, 75,  200, 70,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 2, (char*)guiClass::rightChevronStr,  60, 45, touchscreen::ACTIVE},
  {PID_NEXT,     touchscreen::BUTTON,       60, 155, 200, 70,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::nextStr,          60, 45, touchscreen::ACTIVE},
};

const struct touchscreen::pageFieldDef_t guiClass::setupOffsetTopPageDef[] PROGMEM =
{
  {PID_TITLE,    touchscreen::TEXT,         80, 30,  200, 70,  ILI9341_BLACK,  0,  ILI9341_WHITE, 1, (char*)guiClass::offsetTopStr,               45, 45, touchscreen::NOT_ACTIVE},
  {PID_SELECTED, touchscreen::NATIVE_TEXT,  80, 50,  200, 20,  ILI9341_BLACK,  0,  ILI9341_WHITE, 2, (char*)guiClass::blankStr,                   45, 45, touchscreen::NOT_ACTIVE},
  {PID_RCHEVRON, touchscreen::BUTTON,       60, 75,  200, 70,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 2, (char*)guiClass::rightChevronStr,  60, 45, touchscreen::ACTIVE},
  {PID_NEXT,     touchscreen::BUTTON,       60, 155, 200, 70,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::nextStr,          60, 45, touchscreen::ACTIVE},
};

const struct touchscreen::pageFieldDef_t guiClass::setupOffsetBottomPageDef[] PROGMEM =
{
  {PID_TITLE,    touchscreen::TEXT,         80, 30,  200, 70,  ILI9341_BLACK,  0,  ILI9341_WHITE, 1, (char*)guiClass::offsetBottomStr,            45, 45, touchscreen::NOT_ACTIVE},
  {PID_SELECTED, touchscreen::NATIVE_TEXT,  80, 50,  200, 20,  ILI9341_BLACK,  0,  ILI9341_WHITE, 2, (char*)guiClass::blankStr,                   45, 45, touchscreen::NOT_ACTIVE},
  {PID_RCHEVRON, touchscreen::BUTTON,       60, 75,  200, 70,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 2, (char*)guiClass::rightChevronStr,  60, 45, touchscreen::ACTIVE},
  {PID_NEXT,     touchscreen::BUTTON,       60, 155, 200, 70,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::nextStr,          60, 45, touchscreen::ACTIVE},
};


const struct touchscreen::pageFieldDef_t guiClass::setupConfirmPageDef[] PROGMEM =
{
  {PID_FIELD_1,   touchscreen::NATIVE_TEXT, 50, 10,  200, 20,  ILI9341_BLACK,  0,  ILI9341_WHITE, 2, (char*)guiClass::panelSizeStr,               45, 45, touchscreen::NOT_ACTIVE},
  {PID_FIELD_2,   touchscreen::NATIVE_TEXT, 50, 30,  200, 20,  ILI9341_BLACK,  0,  ILI9341_WHITE, 2, (char*)guiClass::offsetTopStr,               45, 45, touchscreen::NOT_ACTIVE},
  {PID_FIELD_3,   touchscreen::NATIVE_TEXT, 50, 50,  200, 20,  ILI9341_BLACK,  0,  ILI9341_WHITE, 2, (char*)guiClass::offsetBottomStr,            45, 45, touchscreen::NOT_ACTIVE},
  {PID_VALUE_1,   touchscreen::NATIVE_TEXT, 180, 10, 200, 20,  ILI9341_BLACK,  0,  ILI9341_WHITE, 2, (char*)guiClass::blankStr,                   45, 45, touchscreen::NOT_ACTIVE},
  {PID_VALUE_2,   touchscreen::NATIVE_TEXT, 180, 30, 200, 20,  ILI9341_BLACK,  0,  ILI9341_WHITE, 2, (char*)guiClass::blankStr,                   45, 45, touchscreen::NOT_ACTIVE},
  {PID_VALUE_3,   touchscreen::NATIVE_TEXT, 180, 50, 200, 20,  ILI9341_BLACK,  0,  ILI9341_WHITE, 2, (char*)guiClass::blankStr,                   45, 45, touchscreen::NOT_ACTIVE},
  {PID_CONFIRMED, touchscreen::BUTTON,      60, 75,  200, 70,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::confirmedStr,     30, 45, touchscreen::ACTIVE},
  {PID_MODIFY,    touchscreen::BUTTON,      60, 155, 200, 70,  ILI9341_WHITE,  ILI9341_RED,  ILI9341_BLACK, 1, (char*)guiClass::modifyStr,        55, 45, touchscreen::ACTIVE},
};


//*****************************************************
// Constants for menu spinner
//*****************************************************

static const char foot8[] PROGMEM         = "8\'";
static const char foot10[] PROGMEM        = "10\'";
static const char foot12[] PROGMEM        = "12\'";
static const char point75[] PROGMEM       = "0.75\"";
static const char onePoint75[] PROGMEM    = "1.75\"";

static struct guiClass::menuSpinnerItems_t panelSizeItems[] =
{
  {foot8,     8.0},
  {foot10,    10.0},
  {foot12,    12.0},
  {0,       0}
};

static struct guiClass::menuSpinnerItems_t offsetItems[] =
{
  {point75,       0.75},
  {onePoint75,    1.75},
  {0,       0}
};

int guiClass::m_menuSpinnerSelection;            // Currently selected menu selection

guiClass::guiClass()
{
  emergencyStopActivated = false;
}

//*****************************************************
// begin.
//*****************************************************

void guiClass::begin()
{
}

//*****************************************************
// runFSM
// Run an FSM and determine the action with regard to its return code.
//
// Returns: 'state'       if returnFromFSM == 1
//          'doneState'   if returnFromFSM == 0
//          'errorState'  if returnFromFSM == -1
//          'errorState'  if default
//*****************************************************

int guiClass::runFSM(int state, int doneState, int errorState, int returnFromFSM)
{
  const char* funcIDStr = PSTR("runFSM: ");
  int retVal = errorState;
  
  switch(returnFromFSM)
  {
    case 0:
      retVal = doneState;
      break;
    case 1:
      retVal = state;
      break;
  }
  
  /*
  if (retVal != state)
  {
    logger.log(F("%SCurr %d DS %d ES %d New %d"), funcIDStr, state, doneState, errorState, retVal);
  }
  */
  return retVal;
}

//*****************************************************
// mmToDisplayStr.
//
//*****************************************************

char* guiClass::mmToDisplayStr(char* buf, uint16_t bufSize, uint16_t* mmValue, numberSystem_t metricImperial)
{
  const char* funcIDStr = PSTR("mmToDisplayStr: ");
  uint16_t feet, inches, mmInches, decInches;

  logger.log(F("%S mmValue %u"), funcIDStr, *mmValue);
  
  switch(metricImperial)
  {
    case NS_IMPERIAL:
      feet = *mmValue / 305;                                                       // Whole feet
      mmInches = (*mmValue % 305) * 10;                                            // mm left over * 10
      inches = mmInches / 254;                                                     // Whole inches
      decInches = (mmInches % 254) / 10;                                           // mm left over / 10 for decimal inches
      snprintf_P(buf, bufSize, PSTR("%u' %u.%u\""), feet, inches, decInches);
      break;
    case NS_METRIC_CM:
    default:
      snprintf_P(buf, bufSize, PSTR("%u.%ucm"), *mmValue / 10, *mmValue % 10);
      break;
  }
  
  logger.log(F("%S [%s]"), funcIDStr, buf);
  return buf;
}

//*****************************************************
// manageValueSpinner.
// When the 'Chevron right' key is touched the value displayed in the PID_SELECTED
// field is overwritten with a formatted version of the value.
// Call with: 
//    &mmValue        - Provides initial value and return selected value.
//    max             - Max mm value. When exceeded *value = min
//    min             - Min mm value
//    increment       - mms to add to value when spin button pressed
//    page            - A ptr to the page array
//    pageSize        - Size of the page
//    metricImperial  - 0 = metric conversion 1 = imperial conversion
//    boolian doBegin - true only when initialising
//
// Returns:
//    0   - Done
//    1   - in progress
//    -1  - Error
//
//*****************************************************

int guiClass::manageValueSpinner(uint16_t* mmValue, uint16_t max, uint16_t min, uint16_t increment, 
                                const touchscreen::pageFieldDef_t* page, size_t pageSize,
                                numberSystem_t metricImperial, bool doBegin)
{
  const char* funcIDStr = PSTR("manageValueSpinner: ");
  static valueSpinnerStates_t state;                                                          // State variable
  static valueSpinnerStates_t lastState = VS_INVALID;                                         // Last state to detect changes
  static touchscreen::pageFieldDef_t* fieldPtr;                                               // Ptr to a field in the page table
  touchscreen::pageFieldDef_t* touchedFieldPtr;                                               // Ptr to the field just touched
  touchscreen::pageFieldDef_t field;                                                          // A local copy of a field in the page
  char selectionText[32];                                                                     // The string to display
  uint16_t selectionTextSize = sizeof(selectionText);                                         // Size of the buffer
  char valueBuf[16];                                                                          // For conversions
  int retVal = 1;

  if (doBegin == true)                                                                        // Call us with doBegin true to initialise
  {
    state = VS_START;
  }

  switch(state)
  {
    case VS_START:
      tsc.renderPage(page, pageSize);                                                         // Render the page
      fieldPtr = tsc.findFieldByID(PID_SELECTED);                                             // Get a pointer to the selected item in the table
      field = *fieldPtr;                                                                      // Copy the field localy 
                                                                                              // Render the field with initial value
      tsc.renderField(&field, mmToDisplayStr(selectionText, selectionTextSize, mmValue, metricImperial));       
      logger.log(F("%SInitial value [%u] [%s]"), funcIDStr, *mmValue, selectionText);
      state = VS_SELECT;
      break;
    case VS_SELECT:                                                                           // Sit here while user selects the item
      if(touchedFieldPtr = tsc.isTouchedField())
      {
        switch(touchedFieldPtr->id)
        {
          case PID_RCHEVRON:                                                                  // Spin to the next item
            *mmValue += increment;                                                            // Increment the value
            if (*mmValue > max)                                                               // and check for limits
            {
              *mmValue = min;
            }
                        
            field = *fieldPtr;                                                                // Copy the page field localy
            tsc.fillField(&field);                                                            // Erase the field
                                                                                              // Render the field with new value
            tsc.renderField(&field, mmToDisplayStr(selectionText, selectionTextSize, mmValue, metricImperial)); 
            logger.log(F("%SValue [%u] [%s]"), funcIDStr, *mmValue, selectionText);
           break;
          case PID_NEXT:                                                                      // Finished selecting
            state = VS_DONE;
            break;
        }
      }
      break;
    case VS_DONE:
      logger.log(F("%SDone [%u] [%s]"), funcIDStr, *mmValue, selectionText);
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
// manageParameterSetup.
// Configures panel size, top offset and bottom offset
// Call with: 
//    bool doBegin - true only when initialising
//
// Returns:
//    0   - Done
//    1   - in progress
//    -1  - Error
//
// Selections made are stored when each selection is completed
//*****************************************************

int guiClass::manageParameterSetup(bool doBegin)
{
  const char* funcIDStr = PSTR("manageParameterSetup: ");
  static setupStates_t state;                                                                       // State variable
  static setupStates_t lastState = SS_INVALID;                                                      // Last state to detect changes
  static uint16_t panelSize;                                                                        // Panel size
  static uint16_t offsetTop;                                                                        // Top offset
  static uint16_t offsetBottom;                                                                     // Bottom offset
  static struct valueSpinnerParams_t* spinnerParams;                                                // Pointer to the max, min, increment struct
  touchscreen::pageFieldDef_t* touchedFieldPtr;                                                     // Ptr to the field just touched
  touchscreen::pageFieldDef_t* fieldPtr;                                                            // Ptr to the page definition table
  char fBuf[32];
  int retVal = 1;

  if (doBegin == true)                                                                              // Execute once to start up
  { 
    logger.log(F("%SStart"), funcIDStr);
    panelSize = configGetPanelLength();                                                             // Initialise from current
    offsetTop = configGetOffsetTop();
    offsetBottom = configGetOffsetBottom();
    state = SS_START;
  }

  switch(state)
  {
      case SS_START:                                                                                // Render the panel size page
        tsc.renderPage(setupConfirmPageDef, sizeof(setupConfirmPageDef));                           // Render the confirmation page
        fieldPtr = tsc.findFieldByID(PID_VALUE_1);                                                  // Find the first value field Panel size
        tsc.fillField(fieldPtr);                                                                    // Erase the text
        mmToDisplayStr(fBuf, sizeof(fBuf), &panelSize, configGetMetricImperial());
        tsc.renderField(fieldPtr, fBuf);                                                            // Render the field with the value
        fieldPtr = tsc.findFieldByID(PID_VALUE_2);                                                  // Repeat for top offset
        tsc.fillField(fieldPtr);
        mmToDisplayStr(fBuf, sizeof(fBuf), &offsetTop, configGetMetricImperial());
        tsc.renderField(fieldPtr, fBuf);
        fieldPtr = tsc.findFieldByID(PID_VALUE_3);                                                  // Repeat for bottom offset
        tsc.fillField(fieldPtr);
        mmToDisplayStr(fBuf, sizeof(fBuf), &offsetBottom, configGetMetricImperial());
        tsc.renderField(fieldPtr, fBuf);
        state = SS_CONFIRM;
        break;
        
      case SS_CONFIRM:
        if(touchedFieldPtr = tsc.isTouchedField())
        {
          switch(touchedFieldPtr->id)
          {
            case PID_MODIFY:
              spinnerParams = configGetPanelLengthSpinnerParams();
              manageValueSpinner(&panelSize, spinnerParams->maximum, spinnerParams->minimum, spinnerParams->increment, 
                                      setupPanelSizePageDef, sizeof(setupPanelSizePageDef),
                                      configGetMetricImperial(), true);
              state = SS_PANEL_SIZE;
            break;
            case PID_CONFIRMED:
              state = SS_DONE;
              break;
          }
        }      
        break;
        
      case SS_PANEL_SIZE:                                                                           // Manage the panel size spinner
        switch(manageValueSpinner(&panelSize, spinnerParams->maximum, spinnerParams->minimum, spinnerParams->increment, 
                                setupPanelSizePageDef, sizeof(setupPanelSizePageDef),
                                configGetMetricImperial()))
        {
          case 0:                                                                                   // Spinner done
            logger.log(F("%SSelected panel size [%u]"), funcIDStr, panelSize);
            configSetPanelLength(panelSize);                                                        // Save the new panel size in config
            configCopyWorkingToEEPROM();                                                            // and save it in EEPROM
            spinnerParams = configGetOffsetSpinnerParams();
            manageValueSpinner(&offsetTop, spinnerParams->maximum, spinnerParams->minimum, spinnerParams->increment, 
                                setupOffsetTopPageDef, sizeof(setupOffsetTopPageDef),
                                configGetMetricImperial(), true);
             state = SS_TOP_OFFSET;
            break;
          case 1:
           break;
          case -1:
           retVal = -1;
           break;
        }
        break;
      case SS_TOP_OFFSET:
        switch(manageValueSpinner(&offsetTop, spinnerParams->maximum, spinnerParams->minimum, spinnerParams->increment, 
                                setupOffsetTopPageDef, sizeof(setupOffsetTopPageDef),
                                configGetMetricImperial()))
        {
          case 0:                                                                                   // Spinner done
            logger.log(F("%SSelected top offset [%u]"), funcIDStr, offsetTop);
            configSetOffsetTop(offsetTop);                                                          // Save new top offset in config
            configCopyWorkingToEEPROM();                                                            // Copy config to EEPROM
            spinnerParams = configGetOffsetSpinnerParams();
            manageValueSpinner(&offsetBottom, spinnerParams->maximum, spinnerParams->minimum, spinnerParams->increment, 
                                setupOffsetBottomPageDef, sizeof(setupOffsetBottomPageDef),
                                configGetMetricImperial(), true);
            state = SS_BOTTOM_OFFSET;
            break;
          case 1:
           break;
          case -1:
           retVal = -1;
           break;
        }
        break;
      case SS_BOTTOM_OFFSET:
        switch(manageValueSpinner(&offsetBottom, spinnerParams->maximum, spinnerParams->minimum, spinnerParams->increment, 
                                setupOffsetBottomPageDef, sizeof(setupOffsetBottomPageDef),
                                configGetMetricImperial()))
        {
          case 0:                                                                                   // Spinner done
            logger.log(F("%SSelected bottom offset [%u]"), funcIDStr, offsetBottom);
            configSetOffsetBottom(offsetBottom);                                                    // Save new bottom offset in config
            configCopyWorkingToEEPROM();                                                            // Copy config to EEPROM
            state = SS_START;
            break;
          case 1:
           break;
          case -1:
           retVal = -1;
           break;
        }
        break;
      case SS_DONE:
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
// manageTestPage.
// Returns: 0   - Done
//          1   - ongoing
//          -1  - Error done
//*****************************************************
int guiClass::manageTestPage(bool doBegin)
{
  const char* funcIDStr = PSTR("manageTestPage: ");
  static guiTestManagerStates_t state = GMT_START;
  static guiTestManagerStates_t lastState;
  touchscreen::pageFieldDef_t* field;
  static uint32_t timerStart;
  int retVal = 1;
  
  #define TIMEOUT_TIMER  2                                                          // Timer for timeout
  uint32_t timeOutMillis;                                                           // Millisecond timeout
  
  if (doBegin == true)                                                              // Call us with doBegin true to initialise
  {
    logger.log(F("%SStart"), funcIDStr);
    state = GMT_START;
    lastState = GMT_START;
  }

  switch(state)
  {
    case GMT_START:                                                                 // Starts the main page
      tsc.renderPage(testPageDef, sizeof(testPageDef));
      timerStart = millis();
      state = GMT_SELECT;
      break;
      
    case GMT_SELECT:
      timeOutMillis = configGetTimeOut(TIMEOUT_TIMER);
      if ((millis() - timerStart) > timeOutMillis)
      {
        logger.log(F("%STimeout %lu"), funcIDStr, timeOutMillis);
        retVal = 0;
        state = GMT_START;
        break;
      }
      
      if(field = tsc.isTouchedField())
      {
        switch(field->id)
        {
          case PID_MOTOR_TEST:
            motorTest.manageMotorTest(true);
            state = GMT_MOTOR_TEST;
            break;
          case PID_WET_TEST:                                 
            wetTest.manageWetTest(true);
            state = GMT_WET_TEST;
            break;
        }
      }
      break;
    case GMT_MOTOR_TEST:
      state = (guiTestManagerStates_t)runFSM(state, GMT_DONE, GMT_ERROR, motorTest.manageMotorTest());
      break;
      
    case GMT_WET_TEST:
      state = (guiTestManagerStates_t)runFSM(state, GMT_DONE, GMT_ERROR, wetTest.manageWetTest());
      break;
      
    case GMT_DONE:
      logger.log(F("%SDone"), funcIDStr);
      state = GMT_START;
      retVal = 0;
      break;  
      
    case GMT_ERROR:    
      logger.log(F("%S%S"), funcIDStr, logger.m_logErrorStr);
      state = GMT_START;
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
// manageReset.
// Returns: 0   - Done
//          1   - ongoing
//          -1  - Error done
//*****************************************************
int guiClass::manageReset(bool doBegin)
{
  const char* funcIDStr = PSTR("manageReset: ");
  static guiResetManagerStates_t state = GMR_START;
  static guiResetManagerStates_t lastState;
  touchscreen::pageFieldDef_t* field;
  static uint32_t timerStart;
  int retVal = 1;

  if (doBegin == true)                                                              // Call us with doBegin true to initialise
  {
    logger.log(F("%SStart"), funcIDStr);
    state = GMR_START;
    lastState = GMR_START;
  }

  switch(state)
  {
    case GMR_START:
      tsc.renderPage(resetPageDef, sizeof(resetPageDef));
      state = GMR_RESET;
      break;
      
    case GMR_RESET:
      if(field = tsc.isTouchedField())
      {
        switch(field->id)
        {
          case PID_RESET:
            io.setOutputPin(IO_DOUT_WARNING_BUZZER, LOW);                          // Reset the buzzer
            tsc.renderPage(resettingPageDef, sizeof(resettingPageDef));
            //if (digitalRead(IO_DOUT_PUMP_MOTOR_FWD) == HIGH)                       // If pump motor M1 is on forward
            if (digitalRead(IO_DOUT_PUMP_MOTOR_FWD) == LOW)                        // If pump motor M1 is on forward
            {
              //io.setOutputPin(IO_DOUT_RAIL_MOTOR_FWD, LOW);                        // Stop rail motor M2 FWD
              io.setOutputPin(IO_DOUT_RAIL_MOTOR_FWD, HIGH);                       // Stop rail motor M2 FWD
              timerStart = millis();                                               // Start the exit delay
              state = GMR_DELAY;
            }
            else
            {
              //io.setOutputPin(IO_DOUT_FAN_MISTER, LOW);                            // Stop the mister
              io.setOutputPin(IO_DOUT_FAN_MISTER, HIGH);                           // Stop the mister
              manageMotorToHome(MOTOR_APPLIC, true);                               // Start sending applicator M3 home
              state = GMR_APPLIC_TO_HOME;
            }
            break;
        }
      }
      break;
      
    case GMR_DELAY:
      if ((millis() - timerStart) > configGetDelay(5))
      {
        manageStopBeading(true);
        state = GMR_STOP_BEADING;
      }
      break;
    
    case GMR_STOP_BEADING:
      state = (guiResetManagerStates_t)runFSM(state, GMR_APPLIC_TO_HOME, GMR_ERROR, manageStopBeading());
      if (state == GMR_APPLIC_TO_HOME)
      {
        //io.setOutputPin(IO_DOUT_FAN_MISTER, LOW);                                 // Stop the mister
        io.setOutputPin(IO_DOUT_FAN_MISTER, HIGH);                                // Stop the mister
        manageMotorToHome(MOTOR_APPLIC, true);                                    // Start sending applicator M3 home
      }
      break;
      
    case GMR_APPLIC_TO_HOME:  
      state = (guiResetManagerStates_t)runFSM(state, GMR_RAIL_TO_HOME, GMR_ERROR, manageMotorToHome(MOTOR_APPLIC));
      if (state == GMR_RAIL_TO_HOME) manageMotorToHome(MOTOR_RAIL, true);       // Send rail motor home
      break;
      
    case GMR_RAIL_TO_HOME:
      //state = (guiResetManagerStates_t)runFSM(state, GMR_START, GMR_ERROR, manageMotorToHome(MOTOR_RAIL));
      state = (guiResetManagerStates_t)runFSM(state, GMR_DONE, GMR_ERROR, manageMotorToHome(MOTOR_RAIL));
      //if (state == GMR_START) retVal = 0;
      break;
      
    case GMR_ERROR:
      logger.log(F("%SError"), funcIDStr);
      retVal = -1;
      state = GMR_START;
      break;
      
    case GMR_DONE:
      logger.log(F("%SDone"), funcIDStr);
      retVal = 0;
      state = GMR_START;
      break;
      
    default:
      state = GMR_START;
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
// manageGui.
// This is the top level GUI manager.
//*****************************************************
void guiClass::manageGui()
{
  const char* funcIDStr = PSTR("manageGui: ");
  static guiManagerStates_t state = GMS_START;
  static guiManagerStates_t lastState = GMS_INVALID;
  touchscreen::pageFieldDef_t* field;

  if (lastState == GMS_INVALID)
  {
    logger.log(F("%SStart"), funcIDStr);
  }
  
  if (emergencyStopActivated == true)
  {
    if (state != GMS_EMERGENCY_STOP) state = GMS_EMERGENCY_STOP_ACTIVATED;
  }

  switch(state)
  {
    case GMS_START:                                                                // Starting up top of the loop
      manageReset(true);
      iServ.enableEManager(true);                                                  // Start the Ethernet
      state = GMS_RESET;
      break;
      
    case GMS_RESET:                                                                // Resets then shows the start page
      state = (guiManagerStates_t)runFSM(state, GMS_START_PAGE, GMS_ERROR, manageReset());
      if (state == GMS_START_PAGE) tsc.renderPage(startPageDef, sizeof(startPageDef));
      break;
      
    case GMS_START_PAGE:                                                           // Waits for START touched
      if(field = tsc.isTouchedField())
      {
        switch(field->id)
        {
          case PID_START:
            if (digitalRead(IO_DIN_PRESS_CYCLE_IN_PROG) == LOW)                    // Then if pressure not on
            {
              tsc.renderPage(beadingTestPageDef, sizeof(beadingTestPageDef));      // Show the Beading/Test page
              state = GMS_BEAD_TEST;
            }
            break;
        }
      }
      
      break;
      
    case GMS_BEAD_TEST:                                                             // AKA 'STANDBY'
      if (digitalRead(IO_DIN_POWER_FAIL) == HIGH)                                   // Monitor the power fail input
      {                                                                             // If no power then we can't do anything
        tsc.renderPage(powerFailPageDef, sizeof(powerFailPageDef));
        state = GMS_POWER_FAIL;
        break;
      }
    
      if (digitalRead(IO_DIN_PRESS_CYCLE_IN_PROG) == HIGH)                          // If pressure is on
      {                                                                             // then batch has ended
        beader.endOfBatch();
        state = GMS_START;
      }
      
      if(field = tsc.isTouchedField())
      {
        switch(field->id)
        {
          case PID_BEADING:
            //beader.manageBeading(true);
            //state = GMS_BEADING;
            tsc.renderPage(pleaseWaitPageDef, sizeof(pleaseWaitPageDef));
            state = GMS_PRE_BEADING;                                              
            break;
          case PID_TEST:          // TO-Do Put a new 'test' page in here                                  
            //manageTestPage(true);
            //state = GMS_TEST;
            tsc.renderPage(pleaseWaitPageDef, sizeof(pleaseWaitPageDef));
            state = GMS_PRE_TEST;
            break;
        }
      }
      break;
      
    case GMS_PRE_BEADING:
      if (iServ.manageEthernet() == 0)                                            // Only go when Ethernet idle
      {
        iServ.enableEManager(false);                                              // Stop the Ethernet
        beader.manageBeading(true);
        state = GMS_BEADING;
      }
      break;
      
    case GMS_BEADING:
      state = (guiManagerStates_t)runFSM(state, GMS_BEAD_TEST, GMS_ERROR, beader.manageBeading());
      if (state == GMS_BEAD_TEST)
      {
         tsc.renderPage(beadingTestPageDef, sizeof(beadingTestPageDef));          // Show the Beading/Test page
      }
      
      if (state != GMS_BEADING)
      {
        iServ.enableEManager(true);                                               // Start the Ethernet
      }
      break;
      
    case GMS_PRE_TEST:
      if (iServ.manageEthernet() == 0)                                            // Only go when Ethernet idle
      {
        iServ.enableEManager(false);                                              // Stop the Ethernet
        manageTestPage(true);
        state = GMS_TEST;
      }
      break;
      
    case GMS_TEST:
      state = (guiManagerStates_t)runFSM(state, GMS_START, GMS_ERROR, manageTestPage());
      if (state != GMS_TEST)
      {
        iServ.enableEManager(true);                                               // Start the Ethernet
      }
      break;
      
    case GMS_POWER_FAIL:
      if (digitalRead(IO_DIN_POWER_FAIL) == LOW)
      {
        state = GMS_START;
        logger.log(F("%SPower Fail: %d"), funcIDStr, digitalRead(IO_DIN_POWER_FAIL));
        break;
      }
      break;  
      
    case GMS_STOP:                                  // Come here when the STOP button has been activated
      state = GMS_START;
      break;
      
    case GMS_EMERGENCY_STOP_ACTIVATED:
      io.allOff();
      tsc.renderPage(emergencyStopPageDef, sizeof(emergencyStopPageDef));
      state = GMS_EMERGENCY_STOP;
      break;  
      
    case GMS_EMERGENCY_STOP:
      if (digitalRead(IO_DIN_EMERGENCY_STOP) == LOW)
      {
        logger.log(F("%SE STOP reset"), funcIDStr);
        emergencyStopActivated = false;
        state = GMS_START;
      }
      break;
      
    case GMS_ERROR:    
      logger.log(F("%S%S"), funcIDStr, logger.m_logErrorStr);
      state = GMS_START;
      break;  
  }

  if (state != lastState)
  {
    logger.log(F("%Sstate %d -> %d"), funcIDStr, lastState, state);
    lastState = state;
  }

  tsc.renderIndicators();                           // Update the indicators
}

//*****************************************************
// emergencyStop
// Called for emergency situations. Exactly what it does is yet
// to be specified.
//*****************************************************

void guiClass::emergencyStop()
{
  if (emergencyStopActivated) return;
  
  //logger.log(F("emergencyStop:"));
  io.allOff();                                    // Make everything safe
  if (beader.getInBatch() > 0)
  {
    beader.saveEndOfBeadForUpload(beader.BRT_END_OF_BEAD, beader.BST_ESTOP);
    if (beader.getInBead() && (beader.getBeadInBatch() > 0)) // If in a batch
    {
      beader.setBeadInBatch(beader.getBeadInBatch() - 1);    // This bead failed so decrement the bead in batch count.
    }
  }
  
  beader.setInBatch(0);                           // Not in a batch
  beader.setInBead(0);                            // Not in a bead
  emergencyStopActivated = true;
}

guiClass gui;                                     // The one and only global gui
