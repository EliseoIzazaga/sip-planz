//*****************************************************
//  config.cpp
//  Configuration data and utilities.
//*****************************************************
#include <EEPROM.h>
#include "config.h"
#include "Logger.h"
#include "console.h"


static struct configData_t configData;                          // Working copy of config in RAM

//*****************************************************
// configDataDefault
// Default config structure. Used to populate a blank EEPROM.
// The first part of the structure up to 'startVariableConfig' 
// is copied only on first power up with a fresh processor. 
// Or more accurately one in which the newbornID does not match 
// what is in the EEPROM.
//*****************************************************

static const struct configData_t configDataDefault PROGMEM =
{
  CONFIG_CONFIGURED,                                            // newbornID Indicates EEPROM initialised
  CONFIG_SIP_LAMINATOR_UID_DEFAULT,                             // Unique identifier for this unit
  { 0x0c,0x81,0x12,0x67,0xfe,0x70 },                            // MAC address Private for Ethernet
  0,                                                            // startVariableConfig separator
  CONFIG_STRUCT_VERSION,                                        // Configuration structure version.
  CONFIG_BASE_URL_DEFAULT,                                      // Base url for web requests
  CONFIG_ACCESS_CODE_DEFAULT,                                   // Access code
  CONFIG_BACK_DOOR_DEFAULT,                                     // Back door code
  2438,                                                         // Selected panel length mm
  152,                                                          // Offset Bottom bead recess mm
  152,                                                          // Offset Top bead recess mm
  //1.0,                                                          // Applicator offset inches
  {2000, 4000, 150},                                            // Panel length spin parameters
  {50, 800, 50},                                                // Offset spin parameters
  1.08,                                                         // mm per counter pulse
  1188L,                                                        // # pulses from home to bead (was 'start')
  3328L,                                                        // # pulses from bead (was 'start') to end
  4424L,                                                        // # pulses from home to wet
  1,                                                            // Indicators enabled
  NS_METRIC_CM,                                                 // Metric centimeters
  {2000, 2001, 2002, 2003, 2004, 2005},                          // CONFIG_MAX_TIMERS number of Timers 
  {3000, 3001, 3002, 3003, 3004, 3005, 3006, 3007, 3008, 3009, 3010, 3011} // CONFIG_MAX_DELAYS number of Timers 
};

//*****************************************************
// configInit()
// Read config data from the EEPROM into the working copy.
//*****************************************************
void configInit()
{
  configCopyEEPROMOrDefaultToWorking();                         // Get config or default
  logger.log(F("configInit: Loaded version %d size %d bytes EE Base %d"),
                configGetWorkingConfigVersion(), sizeof(configData_t), EEPROM_CONFIG_BASE);
  /*
  Serial.print(F("configInit: Loaded version "));
  Serial.print(configGetWorkingConfigVersion());
  Serial.print(F(" Size "));
  Serial.print(sizeof(configData_t));
  Serial.print(F(" bytes EE Base "));
  Serial.println(EEPROM_CONFIG_BASE);
  */              
  //console.consoleConfig(NULL, NULL);                           // Print the config
}

//*****************************************************
// Working copy Accessor Get functions.
// Returns values from the working copy
//*****************************************************
uint16_t  configGetWorkingConfigVersion(){return configData.configVersion;}
char*     configGetWorkingBaseURL(){return configData.baseURL;}
uint16_t  configGetUID(){return configData.sipLaminatorUID;}

char*     configGetAccessCode(){return configData.accessCode;}
uint16_t  configGetPanelLength(){return configData.panelLength;}
int16_t  configGetOffsetBottom(){return configData.offsetBottom;}
int16_t  configGetOffsetTop(){return configData.offsetTop;}
struct valueSpinnerParams_t* configGetPanelLengthSpinnerParams() {return &configData.panelLengthSpinnerParams;}
struct valueSpinnerParams_t* configGetOffsetSpinnerParams() {return &configData.offsetSpinnerParams;}
//float     configGetapplicatorOffset(){return configData.applicatorOffset;}
float     configGetMmPP(){return configData.mmPP;}
uint32_t  configGetPulsesHomeToBead(){return configData.pulsesHomeToBead;}
uint32_t  configGetPulsesBeadToEnd(){return configData.pulsesBeadToEnd;}
uint32_t  configGetPulsesHomeToWet(){return configData.pulsesHomeToWet;}
uint8_t   configGetIndicatorsEnabled(){return configData.indicatorsEnabled;}
uint32_t  configGetTimeOut(int timer) {return configData.timeOut[timer - 1];}
uint32_t  configGetDelay(int timer) {return configData.delay[timer - 1];}

numberSystem_t   configGetMetricImperial(){return configData.metricImperial;}
byte*     configGetMACAddress(){return configData.ethernetMAC;}

char*     configGetMACAddressStr(char* buf, size_t bufSize)
{
  if (bufSize < sizeof(configData_t::ethernetMAC)) 
  {
    logger.log(F("configGetMACAddressStr: Buf too short"));
    return "";
  }
  
  byte* macIndex = configGetMACAddress();
  
  *buf = '\0';
  sprintf_P(buf, PSTR("%02x %02x %02x %02x %02x %02x"), 
              macIndex[0], macIndex[1], macIndex[2], macIndex[3], macIndex[4], macIndex[5]);

  return buf;
}

//*****************************************************
// Working copy Accessor Set functions.
// Sets values in the working version
//*****************************************************
void configSetWorkingConfigVersion(uint16_t version){configData.configVersion = version;}
void configSetPanelLength(uint16_t len){configData.panelLength = len;}
void configSetUID(uint16_t uid){configData.sipLaminatorUID = uid;}
void configSetOffsetBottom(uint16_t val){configData.offsetBottom = val;}
void configSetOffsetTop(uint16_t val){configData.offsetTop = val;}
void configSetPanelLengthSpinnerParams(struct valueSpinnerParams_t* val) {memcpy (&configData.panelLengthSpinnerParams, val, sizeof(valueSpinnerParams_t));}
void configSetOffsetSpinnerParams(struct valueSpinnerParams_t* val) {memcpy (&configData.offsetSpinnerParams, val, sizeof(valueSpinnerParams_t));}
//void configSetApplicatorOS(float val){configData.applicatorOffset = val;}
void configSetMmPerPulse(float val){configData.mmPP = val;}
void configSetPulsesHomeToBead(uint32_t val){configData.pulsesHomeToBead = val;}
void configSetPulsesBeadToEnd(uint32_t val){configData.pulsesBeadToEnd = val;}
void configSetPulsesHomeToWet(uint32_t val){configData.pulsesHomeToWet = val;}
void configSetIndicatorsEnabled(uint8_t val){configData.indicatorsEnabled = val;}
void configSetTimeOut(uint16_t timer, uint32_t val){configData.timeOut[timer - 1] = val;}
void configSetDelay(uint16_t timer, uint32_t val){configData.delay[timer - 1] = val;}
void configSetMetricImperial(numberSystem_t val){configData.metricImperial = val;}
void configSetAccessCode(char* code){strncpy(configData.accessCode, code, CONFIG_ACCESS_CODE_LEN);}

void configSetMACAddress(byte* val)
{
  int i;

  for (i = 0; i < 6; i++)
  {
    configData.ethernetMAC[i] = val[i];
  }
}

void configSetWorkingBaseURL(char* baseUrl)
{
  strncpy(configData.baseURL, baseUrl, CONFIG_BASE_URL_LEN);
  configData.baseURL[CONFIG_BASE_URL_LEN] = '\0';
}


//*****************************************************
// configDumpEEPROMStruct()
// Reads and dumps the contents of the EEPROM config struct
//*****************************************************
void configDumpEEPROMStruct()
{
  int i;
  configData_t config;                                              // Copy of EEPROM on stack
  size_t configSize = sizeof(configData_t);
  char* ptr = (char *)&config;
  int EEIndex = EEPROM_CONFIG_BASE;

  for (i = 0; i < configSize; i++)                                  // Copy it out byte by byte
  {
    *ptr++ =  EEPROM.read(EEIndex++);
  }

  logger.log(F("configDumpEEPROMStruct"));
  configPrintConfig(&config);                                       // Print it
}

//*****************************************************
// configCopyDefaultToEEPROM()
// Copy the default struct in progmem to the EEPROM
// This is not a frequent operation so the default is written
// regardless of EEPROM state.
//*****************************************************
void configCopyDefaultToEEPROM()
{
  const char* funcIDStr = PSTR("configCopyDefaultToEEPROM: ");
  int i, c;
  int EEIndex = EEPROM_CONFIG_BASE;                                // Start of config in EEPROM
  const char * startPtr = (const char *)&configDataDefault;        // Start of default config in progmem
                                                                   // Start of variable config after header
  const char * startVarPtr = (const char *)&configDataDefault.startVariableConfig;
  size_t configSize = sizeof(configData_t);                        // Size of the config struct
  size_t headerSize = startVarPtr - startPtr;                      // Size of the constant header
  size_t copySize;                                                 // Size to copy
  const char * ptr;                                                // Ptr used for the copy
  
  if (configData.newbornID == CONFIG_CONFIGURED)
  {                                                                // Only copy variable part
    ptr = startVarPtr;
    copySize = configSize - headerSize;
    EEIndex = EEPROM_CONFIG_BASE + headerSize;
    logger.log(F("%SConfigured"), funcIDStr);
  }
  else
  {                                                                // Unless this is a newborn Controller
    ptr = startPtr;
    copySize = configSize;
    EEIndex = EEPROM_CONFIG_BASE;
    logger.log(F("%SNewborn"), funcIDStr);
  }

  logger.log(F("%SStart %p Size %u EEIndex %d"), funcIDStr, ptr, copySize, EEIndex);
    
  for (i = 0; i < copySize; i++)                                   // Copy the default struct in progmem
  {
    c = pgm_read_byte(ptr++);
    EEPROM.write(EEIndex++, c);
  }
  
}


//*****************************************************
// configCopyDefault()
// Copy the default struct in progmem.
//*****************************************************
void configCopyDefault(uint8_t* buf, size_t bufLen)
{
  const char* funcIDStr = PSTR("configCopyDefault: ");
  int i;
  int c;
  size_t configSize = sizeof(configData_t);

  if (configSize > bufLen)
  {
    logger.log(F("%SError: buf short"), funcIDStr);
    return;
  }

  const char * ptr = (const char *)&configDataDefault;
  int index = 0;

  logger.log(F("%S"), funcIDStr);

  for (i = 0; i < configSize; i++)                                  // Copy the default struct in progmem
  {
    c = pgm_read_byte(ptr++);
    buf[index++] = c;
  }

  //configPrintConfig((configData_t*)buf);
}

//*****************************************************
// configGetDefaultBaseURL()
// Get the default base URL.
//*****************************************************
void configGetDefaultBaseURL(uint8_t* buf, size_t bufLen)
{
  configData_t defConfig;                                                 // To hold the whole config

  configCopyDefault((uint8_t*)&defConfig, sizeof(defConfig));             // Get all config
  strncpy((char*)buf, defConfig.baseURL, bufLen);                         // Copy the base url to caller
  buf[bufLen - 1] = '\0';
  logger.log(F("configGetDefaultBaseURL: bufLen %d [%s]"), bufLen, buf);
}

//*****************************************************
// fToA()
// Converts a float to a string.
//*****************************************************
char* fToA(float val, char* buf, int bufLen)
{
  int16_t fAsLongBP;
  uint16_t fAsLongAP;
  uint32_t fAsLong;

  fAsLong = val * 100;
  fAsLongBP = fAsLong / 100;
  fAsLongAP = fAsLong % 100;
  snprintf_P(buf, bufLen, PSTR("%d.%d"), fAsLongBP, fAsLongAP);
  return buf;
}

//*****************************************************
// configPrintConfig()
// Prints config data.
//*****************************************************
void configPrintConfig(configData_t* config)
{
  const char* onStr   = PSTR("ON");
  const char* offStr  = PSTR("OFF");
  char buf[32];
  int i;
  
  logger.log(F("%10S: %04x"), PSTR("ID"), config->newbornID);
  logger.log(F("%10S: %u"), PSTR("UID"), config->sipLaminatorUID);
  logger.log(F("%10S: %d"), PSTR("Ver"), config->configVersion);
  logger.log(F("%10S: %s"), PSTR("Base URL"), config->baseURL);
  logger.log(F("%10S: %s"), PSTR("Access Cde"), config->accessCode);

  logger.log(F("%10S: %u"), PSTR("Panel Len"), config->panelLength);
  
  logger.log(F("%10S: %d"), PSTR("Bottom OS"), config->offsetBottom);
  logger.log(F("%10S: %d"), PSTR("Top OS"), config->offsetTop);

  logger.log(F("%10S: %u %u %u"), PSTR("Panel spin"), config->panelLengthSpinnerParams.minimum, config->panelLengthSpinnerParams.maximum, config->panelLengthSpinnerParams.increment);
  logger.log(F("%10S: %u %u %u"), PSTR("OS spin"), config->offsetSpinnerParams.minimum, config->offsetSpinnerParams.maximum, config->offsetSpinnerParams.increment);
  
  fToA(config->mmPP, buf, sizeof(buf) - 1);
  logger.log(F("%10S: %s"), PSTR("mm Per P"), buf);
  
  logger.log(F("%10S: %lu"), PSTR("Hm->Bead"), config->pulsesHomeToBead);
  logger.log(F("%10S: %lu"), PSTR("Bead->End"), config->pulsesBeadToEnd);
  logger.log(F("%10S: %lu"), PSTR("Home->Wet"), config->pulsesHomeToWet);
  logger.log(F("%10S: %d"), PSTR("Indicators"), config->indicatorsEnabled);
  logger.log(F("%10S: %S"), PSTR("Num System"), (config->metricImperial == NS_METRIC_CM) ? PSTR("Metric cm") : PSTR("Imperial"));

  logger.log(F("%7S %02d: %lu"), PSTR("WL Time"), 1, config->timeOut[0]);
  for (i = 1; i < CONFIG_MAX_TIMEOUTS; i++)
  {
    logger.log(F("%7S %02d: %lu"), PSTR("Timeout"), i + 1, config->timeOut[i]);
  }
  
  for (i = 0; i < CONFIG_MAX_DELAYS; i++)
  {
    logger.log(F("%7S %02d: %lu"), PSTR("Delay"), i + 1, config->delay[i]);
  }
  
  logger.log(F("%10S: %s"), PSTR("MAC"), configGetMACAddressStr(buf, sizeof(buf)));
}


//*****************************************************
// configDumpWorkingStruct()
// Dumps the contents of the working config struct
//*****************************************************
void configDumpWorkingStruct()
{
  logger.log(F("configDumpWorkingStruct"));
  configPrintConfig(&configData);
}

//*****************************************************
//  configCopyEEPROMToWorking()
//  Copy the EEPROM config structure to the working copy in RAM.
//*****************************************************
void configCopyEEPROMToWorking()
{
  int i;
  size_t configSize = sizeof(configData_t);
  char* ptr = (char *)&configData;
  int EEIndex = EEPROM_CONFIG_BASE;

  logger.log(F("configCopyEEPROMToWorking"));

  for (i = 0; i < configSize; i++)
  {
    *ptr++ =  EEPROM.read(EEIndex++);
  }
}

//*****************************************************
// configCopyEEPROMOrDefaultToWorking()
// Copy the EEPROM config structure to the working copy in RAM
// then check the newbornID to determine if the EEPROM copy is valid.
// If it is not then copy the default to the EEPROM and read the
// EEPROM to working again.
//*****************************************************
void configCopyEEPROMOrDefaultToWorking()
{
  configCopyEEPROMToWorking();                                // Get the EEPROM version to working
  if ((configData.newbornID != CONFIG_CONFIGURED) ||          // Check it's valid
      (configData.configVersion != CONFIG_STRUCT_VERSION))    // and it has the correct version
  {
    logger.log(F("configCEODTW: ID [%04x] Old V [%d] New V [%d] Using default"),
            configData.newbornID, configData.configVersion, CONFIG_STRUCT_VERSION);
    configCopyDefaultToEEPROM();                              // If not write default
    configCopyEEPROMToWorking();                              // and read to working
  }
}

//*****************************************************
// configCopyWorkingToEEPROM()
// Copy the working struct in RAM to the EEPROM
// Each byte is compared with the EEPROM and only written if it
// is different.
//*****************************************************
void configCopyWorkingToEEPROM()
{
  uint8_t c, ec;
  int i;
  size_t configSize = sizeof(configData_t);
  const char * ptr = (const char *)&configData;
  int EEIndex = EEPROM_CONFIG_BASE;

  char outbuf[64];

  logger.log(F("configCopyWorkingToEEPROM"));

  for (i = 0; i < configSize; i++)                            // Copy the working struct in RAM
  {
    c = *ptr++;
    ec = EEPROM.read(EEIndex);

    if (c != ec)                                              // Only save if different
    {
      EEPROM.write(EEIndex, c);
    }

    EEIndex++;
  }
}



/*
//*****************************************************
// configNBRun()
// Gets a config file from the server then parses it appropriately.
// Returns  0   - Idle or finished
//          1   - In progress
//         -1   - Failed
//*****************************************************
int configNBRun()
{
  static configNBRunStates_t state = CNBR_IDLE;                       // State variable
  configNBRunStates_t lastState = state;                              // Save state for reporting
  static uint32_t lastTime = 0;                                       // For scheduling in seconds
  uint32_t now;                                                       // Current time seconds
  uint32_t DownloadIntervalSecs;                                      // Interval between downloads
  int retVal = 1;
  bool requestedToGetConfig = false;                                  // Assume not yet
  char urlBuf[128];
  static int retries;                                                 // Number of retries

  switch (state)
  {
    case CNBR_IDLE:                                                   // Wait until required
      now = rtc.getRtcTickCounter();                                  // Current seconds from RTC interrupt

      DownloadIntervalSecs = ((uint32_t)configGetWorkingConfigSyncIntervalMins() * 60L);  // Seconds between downloads
      
      if ((now - lastTime) >= DownloadIntervalSecs)                   // Scheduled check for config download
      {
        if (DownloadIntervalSecs && configIsAutoRemoteComsEnabled())  // If its enabled
        {
          requestedToGetConfig = true;
          logger.log(F("configNBRun time %lu"), now);
        }

        lastTime = now;
      }

      if (configReqDownloadFlag == true)                              // Manual request
      {
        configReqDownloadFlag = false;                                // Reset the request flag
        requestedToGetConfig = true;                                  // And set the do-it flag
        snprintf(urlBuf, sizeof(urlBuf), "%s%S%s", configGetWorkingBaseURL(), CONFIG_CONFIG_URL_PART, app.get_device_id().c_str());
        logger.log(F("configNBRun url [%s]"), urlBuf);
     }

      if (requestedToGetConfig)
      {
        logger.log(F("configNBRun started"));
        retries = 0;
        state = CNBR_DL_CONFIG_START;                                 // Download the config
      }
      else
      {
        retVal = 0;                                                   // Nothing to do yet
      }

      break;
    case CNBR_DL_CONFIG_START:
      if (retries++ < 3)
      {
        logger.log(F("configNBRun Try %d"), retries);
        app.device.gprs.nbHttpGet(NULL, NULL, 0, true);
        state = CNBR_DL_CONFIG;                                       // Download the config
      }
      else
      {
        logger.log(F("configNBRun Failed %d retries"), retries);
        state = CNBR_IDLE;                                            // Tried out
        retVal = -1;                                                  // Error
      }
      break;
    case CNBR_DL_CONFIG:
      snprintf(urlBuf, sizeof(urlBuf), "%s%S%s", configGetWorkingBaseURL(), CONFIG_CONFIG_URL_PART, app.get_device_id().c_str());

      switch (app.device.gprs.nbHttpGet(urlBuf, configFileBuffer, CONFIG_BUFFER_SIZE, false))   // Download config
      {
        case 0:                                                       // Success
          logger.log(F("configNBRun Done\n\r%s\n"), configFileBuffer);
          consoleParseFile(configFileBuffer);                         // Parse the file
          state = CNBR_IDLE;                                          // Finished
          break;
        case -1:                                                      // Fail
          logger.log(F("Failed"));
          state = CNBR_DL_CONFIG_START;                               // Retry
          break;
        case 1:                                                       // In progress
        default:
          break;
      }
      break;
    default:
      retVal = -1;                                                    // Error
      state = CNBR_IDLE;
      break;
  }

  if (state != lastState)                 // Report state changes
  {
    logger.log(F("configNBRun: %d -> %d"), lastState, state);
    //Serial.print(F("configNBRun State: "));
    //Serial.println(state);
  }

  return retVal;
}
*/




