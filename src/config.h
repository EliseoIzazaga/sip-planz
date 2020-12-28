//*****************************************************
//  config.h
//  Configuration data and utilities.
//*****************************************************

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

//*****************************************************
// Use of EEPROM:
// Total size of onboard EEPROM = 4096 bytes
//    0000  - ...     Config
//*****************************************************

#define CONFIG_CONFIGURED                 0xAA55                // Unit is 'Newborn' until configured
#define CONFIG_STRUCT_VERSION             4                     // The version of the config structure
#define CONFIG_SIP_LAMINATOR_UID_DEFAULT  0                     // Unique identifier for this unit
#define CONFIG_BASE_URL_DEFAULT           "castlenetware.com"   // Base url for all comms
#define CONFIG_ACCESS_CODE_DEFAULT        "1234"                // Default access code
#define CONFIG_BACK_DOOR_DEFAULT          "avatar"              // Default back door access code


#define CONFIG_CONFIG_URL_PART            PSTR("/mbc/devices/config?did=")

#define EEPROM_CONFIG_BASE                0000                  // Base location of config in EEPROM
#define CONFIG_BASE_URL_LEN               30                    // Max length of base url
#define CONFIG_ACCESS_CODE_LEN            9                     // Max length of access code
#define CONFIG_MAX_TIMEOUTS               6                     // CHANGE DEFAULT IN config.cpp IF CHANGED
#define CONFIG_MAX_DELAYS                12                     // CHANGE DEFAULT IN config.cpp IF CHANGED

#define CONFIG_BUFFER_SIZE                1023                  // Size of config file download buffer

/*
enum configFlags_t
{
  CF_ENABLE_SERIAL_LOG            = 0b00000000000000001,        // Output debugging to serial port
};
*/

#define CONFIG_TEST_MODE          CF_ENABLE_SERIAL_LOG
#define CONFIG_NORMAL_MODE        CF_ENABLE_AUTO_REMOTE_COMS | CF_ENABLE_SLEEP_MODE | CF_ENABLE_SERIAL_LOG
#define CONFIG_DEV_MODE           CF_ENABLE_SERIAL_LOG


enum configNBRunStates_t
{
  CNBR_IDLE = 0,                                                // Waiting for work
  CNBR_DL_CONFIG_START,                                         // Start a try
  CNBR_DL_CONFIG                                                // Donloadign config file
};

struct valueSpinnerParams_t
{
  uint16_t minimum;                                            // Minimum value for selection
  uint16_t maximum;                                            // Maximum value for selection
  uint16_t increment;                                          // Increment to be added each spin
};

enum numberSystem_t
{
  NS_METRIC_CM,
  NS_IMPERIAL
};

//************************************************
// struct configData_t
// This structure will be written and read from the EEPROM
// as a single block. The structure held in EEPROM must exactly
// match that of the PROGMEM and working RAM versions.
// The configVersion field serves to identify the structure version.
// Any updates to the structure in subsequent firmware versions must
// be accompanied by a run-time conversion routine which is capable
// of converting any earlier version to the current version.
// Fields may be added to the end of the structure without changing the
// configVersion if necessary.
//************************************************
struct configData_t
{
  uint16_t  newbornID;                                          // Uninitialised config does not have 'CONFIG_CONFIGURED'
  uint16_t  sipLaminatorUID;                                    // Unique ID for this controller
  byte      ethernetMAC[6];                                     // MAC address for Ethernet adapter
  byte      startVariableConfig;                                // Separates constant above from variable below
  uint16_t  configVersion;                                      // Configuration structure version.
  char      baseURL[CONFIG_BASE_URL_LEN + 1];                   // Base url for web requests
  char      accessCode[CONFIG_ACCESS_CODE_LEN];                 // Access code
  char      backDoorCode[CONFIG_ACCESS_CODE_LEN];               // Back door access code
  uint16_t  panelLength;                                        // Selected panel length mm
  int16_t   offsetBottom;                                       // Offset Bottom bead recess mm
  int16_t   offsetTop;                                          // Offset Top bead recess mm
  struct    valueSpinnerParams_t panelLengthSpinnerParams;      // Spin button parameters for panel length 
  struct    valueSpinnerParams_t offsetSpinnerParams;           // Spin button parameters for top and bottom offsets 
  float     mmPP;                                               // mm per counter pulse
  uint32_t  pulsesHomeToBead;                                   // # pulses from home to bead (was 'start')
  uint32_t  pulsesBeadToEnd;                                    // # pulses from bead (was 'start') to end
  uint32_t  pulsesHomeToWet;                                    // # pulses from home to wet
  uint8_t   indicatorsEnabled;                                  // Indicator display fields enabled
  numberSystem_t  metricImperial;                               // 0 = Metric 1 = Imperial
  uint32_t  timeOut[CONFIG_MAX_TIMEOUTS];                       // Timers - CHANGE DEFAULT IN config.cpp IF CHANGED
  uint32_t  delay[CONFIG_MAX_DELAYS];                           // Delays - CHANGE DEFAULT IN config.cpp IF CHANGED
};

void      configInit();                                         // Initialise the module
uint16_t  configGetWorkingFlags();                              // Configuration flags
void      configDumpWorkingStruct();                            // Dump config working copy
void      configDumpEEPROMStruct();                             // Dump config from EEPROM
void      configCopyWorkingToEEPROM();                          // Copy RAM based working config to EEPROM
int       configNBRun();                                        // Config manager
char*     configGetWorkingBaseURL();                            // Get working base url
void      configGetDefaultBaseURL(uint8_t* buf, size_t bufLen); // Get a copy of the default base url
uint16_t  configGetPanelLength();                               // Current panel length
uint16_t  configGetUID();                                       // Unique identifier
int16_t   configGetOffsetBottom();                              // Offset Bottom bead recess
int16_t   configGetOffsetTop();                                 // Offset Top bead recess
struct    valueSpinnerParams_t* configGetPanelLengthSpinnerParams();// Panel length spinner parameters
struct    valueSpinnerParams_t* configGetOffsetSpinnerParams();    // Offset spinner parameters
float     configGetMmPP();                                      // Distance per pulse in mm
uint32_t  configGetPulsesHomeToBead();                          // Pulses home to bead (was 'start')
uint32_t  configGetPulsesBeadToEnd();                           // Pulses bead (was 'start') to end
uint32_t  configGetPulsesHomeToWet();                           // Pulses home to wet
uint8_t   configGetIndicatorsEnabled();                         // Display indicaors enabled
uint32_t  configGetTimeOut(int timer);                          // Timers
uint32_t  configGetDelay(int timer);                            // Delays
char*     configGetAccessCode();                                // Login access code

numberSystem_t   configGetMetricImperial();                            // 0 = Metric cm, 1 = Imperial
byte*     configGetMACAddress();                                // Get MAC address
char*     configGetMACAddressStr(char* buf, size_t bufSize);    // Get MAC address string
uint16_t  configGetWorkingConfigVersion();                      // Version of config struct

void      configCopyDefaultToEEPROM();                          // Copy default to EEPROM
void      configCopyEEPROMToWorking();                          // Copy EEPROM to working config
//void      configSetWorkingFlags(uint16_t flags);                // Set the flags field
void      configSetWorkingBaseURL(char* baseUrl);               // Set base url
void      configSetPanelLength(uint16_t len);                   // Set the length of the panel
void      configSetUID(uint16_t uid);                           // Unique identifier
//void      configSetBeadRecessBottom(float val);                 // Bottom bead recess
//void      configSetBeadRecessTop(float val);                    // Top bead recess
void      configSetOffsetBottom(uint16_t val);                  // Offset Bottom bead recess
void      configSetOffsetTop(uint16_t val);                     // Offset Top bead recess
void configSetPanelLengthSpinnerParams(struct valueSpinnerParams_t* val); // Panel length spinner parameters
void configSetOffsetSpinnerParams(struct valueSpinnerParams_t* val); // Offset spinner parameters
//void      configSetApplicatorOS(float val);                     // Applicator offset
void      configSetMmPerPulse(float val);                       // Distance per pulse in mm
void      configSetPulsesHomeToBead(uint32_t val);              // Distance Home to Bead
void      configSetPulsesBeadToEnd(uint32_t val);               // Distance Bead to End
void      configSetPulsesHomeToWet(uint32_t val);               // Distance Home to Wet
void      configSetIndicatorsEnabled(uint8_t val);              // Display indicators enabled
void      configSetTimeOut(uint16_t timer, uint32_t val);       // Timeouts
void      configSetDelay(uint16_t timer, uint32_t val);         // Delays
void      configSetAccessCode(char* code);                      // Set the login code


void      configSetMetricImperial(numberSystem_t val);                 // 0 = Metric cm, 1 = Imperial
void      configSetMACAddress(byte* val);                       // Ethernet address

char*     fToA(float val, char* buf, int bufLen);               // Convert float to string


static void configSetWorkingConfigVersion(uint16_t version);    // Set working config structure version
static void configCopyEEPROMOrDefaultToWorking();               // Copy EEPROM or default config to working
static void configPrintConfig(configData_t* config);            // Print config in readable form
static void configCopyDefault(uint8_t* buf, size_t bufLen);     // Get a copy of the default config



//static void configSetWorkingConfigVersion(uint16_t version);


#endif
