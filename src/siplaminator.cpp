//*****************************************************
//  siplaminator.ino
//  SIP Laminator Controller Update
//*****************************************************
#include <Time.h>

#include "siplaminator.h"
#include "Logger.h"
#include "console.h"
#include "config.h"
#include "touchscreen.h"
#include "gui.h"
#include "io.h"
#include "statistics.h"
#include "enc28j60_ethernet.h"
#include "rtc_time.h"
#include "timers.h"
#include "beading.h"

const char appName[] PROGMEM = APP_NAME;                        // Name of the App

void setup() 
{
  const char* funcIDStr = PSTR("setup: ");
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println(F("Starting up"));
  tsc.begin();                                                  // Init LCD touchscreen
  logger.begin();                                               // Init the debug logger
  rtc.begin();                                                  // Start the real time clock
  configInit();                                                 // Init configuration utilities
  gui.begin();                                                  // Start the GUI
  io.begin();                                                   // Initilise
                                                                // Announce the app 
  logger.log(""); 
  logger.log("*****************", funcIDStr, appName, APP_VERSION, configGetUID()); 
  logger.log("%S%S %s Unit ID %u", funcIDStr, appName, APP_VERSION, configGetUID()); 
  logger.log("*****************", funcIDStr, appName, APP_VERSION, configGetUID()); 
  
  //stats.begin();                                                // Init stats module
  beader.begin();                                               // The beader module
  iServ.begin();                                                // Init Ethernet Services
  console.begin();                                              // Init USB serial console
  timers.begin();                                               // Interrupt driven timers
}

// the loop function runs over and over again forever
void loop() 
{
  console.manageCommands();                                     // Manage console commands
  tsc.manageTouchScreen();                                      // Manage LCD and touch screen
  gui.manageGui();                                              // Manage the GUI
  gui.manageWorkLife();                                         // Manage the Work-Life counter
  io.manageIO();                                                // Regular IO management
  iServ.manageEthernet();                                       // Handle network stuff
}
