//////////////////////////////////////////////////
// console.h
// Serial console UI
//////////////////////////////////////////////////

#ifndef CONSOLE_H
#define CONSOLE_H

#define CON_BUFSIZE        128
#define CON_PROMPTSTR      "OK> "
#define CON_ERRORSTR       "ERR:"

class consoleClass
{
  private:
    static bool m_printEnabled;                                           // false when interpreting a file
    uint32_t m_loginStart;                                                // Login start time
  
    struct consoleBuf_t
    {
      int   state;                                                        // Handler state
      bool  msgAvailable;                                                 // There is a complete message
      bool  savedLogFlag;                                                 // Saves the original config log flag
      int   bufInd;                                                       // Buffer index
      char  buf[CON_BUFSIZE];                                             // Holds input characters
    };
    
    typedef void (*commandFunc)(void* row, char* argStr);                 // Function pointer for the command table
    
    enum visibility_t
    {
      CON_HIDE,                                                           // Don't show in listings
      CON_SHOW                                                            // Show in user listings
    };
    
    struct commandDefTable_t                                              // Table of commands
    {
      const char* command;                                                // The command string
      commandFunc cmdFunc;                                                // Ptr to function to handle the command
      enum visibility_t visibilityMode;                                   // Determines how it is listed
      const char* description;                                            // Help for the command
    };
  
    static const char promptStr[];      // The prompt string
    static const char errStr[];       // The error string
    
    // Define the strings for the command table in program memory
    // This is the only way I can find to ensure that everything goes into progmem
    //static const char logDesc[];
    static const char UIDDesc[];
    static const char verDesc[];
    static const char helpCmd[];
    static const char helpDesc[];
    static const char consoleConfigCmd[];
    static const char consoleConfigDesc[];
    static const char setConfigBaseURLDesc[];
    static const char setPanelLengthDesc[];
    static const char setOffsetBottomDesc[];
    static const char setOffsetTopDesc[];
    static const char setPanelLengthSpinnerDesc[];
    static const char setOffsetSpinnerDesc[];
    //static const char setApplicatorOSCmd[];
    //static const char setApplicatorOSDesc[];
    static const char setMmPerPulseDesc[];
    //static const char setPulsesHomeToStartDesc[];
    //static const char setPulsesStartToEndDesc[];
    
    static const char setPulsesHomeToBeadDesc[];
    static const char setPulsesBeadToEndDesc[];
    static const char setPulsesHomeToWetDesc[];
    
    static const char setTimeOutDesc[];
    static const char setDelayDesc[];

    static const char testLCDCmd[];
    static const char testLCDDesc[];
    static const char sdDirCmd[];
    static const char sdDirDesc[];
    static const char sdRmDirCmd[];
    static const char sdRmDirDesc[];
    static const char sdRmCmd[];
    static const char sdRmDesc[];
    static const char sdPrCmd[];
    static const char sdPrDesc[];
    static const char sdTailCmd[];
    static const char sdTailDesc[];
    static const char setIndicatorsEnabledDesc[];
    static const char restartCmd[];
    static const char restartDesc[];
    static const char setOPCmd[];
    static const char setOPDesc[];
    static const char setMetricImpDesc[];
    //static const char statsCmd[];
    //static const char statsDesc[];
    //static const char uploadStatsCmd[];
    //static const char uploadStatsDesc[];
    static const char uploadConfigCmd[];
    static const char uploadBeadCmd[];
    static const char uploadBeadDesc[];
    static const char uploadConfigDesc[];
    static const char GetSvrCmdsCmd[];
    static const char GetSvrCmdsDesc[];
    static const char setMACDesc[];
    static const char getRTCTempCmd[];
    static const char getRTCTempDesc[];
    static const char RTCCmd[] PROGMEM;
    static const char RTCDesc[] PROGMEM;
    static const char IPCmd[];
    static const char IPDesc[];
    static const char batchCmd[];
    static const char batchDesc[];
    static const char batchUpCmd[];
    static const char batchUpDesc[];
    static const char batchClearCmd[];
    static const char batchClearDesc[];
    static const char batchSetCmd[];
    static const char batchSetDesc[];
    static const char beadSetCmd[];
    static const char beadSetDesc[];
    static const char clearUpCmd[];
    static const char clearUpDesc[];
    static const char projIDCmd[];
    static const char projIDDesc[];
    static const char loginCmd[];
    static const char loginDesc[];
    static const char accessCodeCmd[];
    static const char accessCodeDesc[];
    static const char touchCmd[];
    static const char touchDesc[];
    static const char prioCmd[];
    static const char prioDesc[];
   
    //static const char consoleLogCmd[] PROGMEM                     = "log";
    //static const char consoleDLConfCmd[] PROGMEM                  = "dlconfig";
    //static const char consoleDLConfDesc[] PROGMEM                 = "Download config";
    //static const char consoleSetConfigBaseURLCmd[] PROGMEM        = "baseurl";
    //static const char consoleSetConfigBaseURLDesc[] PROGMEM       = "[url] Set base url";
    //static const char consoleSetVoteULIntervalMinsCmd[] PROGMEM   = "uploadfreq";
    //static const char consoleSetVoteULIntervalMinsDesc[] PROGMEM  = "[Mins] Set votes upload interval";
    //static const char consoleSetConfigSyncMinsCmd[] PROGMEM       = "configfreq";
    //static const char consoleSetConfigSyncMinsDesc[] PROGMEM      = "[Mins] Config Interval";
    //static const char consoleResetCmd[] PROGMEM                   = "reset";
    //static const char consoleResetDesc[] PROGMEM                  = "Reboot with watchdog";
    //static const char consoleFactoryConfigCmd[] PROGMEM           = "default";
    //static const char consoleFactoryConfigDesc[] PROGMEM          = "Factory default";
    //sstatic const char consoleClearEEPROMCmd[] PROGMEM             = "cleareeprom";
    //static const char consoleClearEEPROMDesc[] PROGMEM            = "Zero EEPROM";

    void interpret(char *buf, int nbytes);                           // Interpret incoming command
    void prompt();                                                   // Send the prompt string
    bool isLoggedIn();                                               // true if logged in
    void logMeIn();                                                  // Set loginStart to now()
    //void consoleWriteWorkingConfigToEEPROM();                      // Save EEPROM config
    
    static void doHelp(void *row, char *buf);                        // List the available commands
    static void getAppVersion(void *row, char* buf);                 // Get version
    static void setBaseURL(void *row, char *buf);                    // Set a new base url
    static void setPanelLength(void *row, char *buf);                // Set a new panel length
    static void setOffsetBottom(void *row, char *buf);               // Bottom bead recess
    static void setOffsetTop(void *row, char *buf);                  // Top bead recess
    //static void uploadStats(void *row, char* buf);                   // Upload statistics to server
    static void uploadConfig(void *row, char* buf);                  // Upload config to server
    static void uploadBead(void *row, char* buf);                    // Upload a bead to server
    static void GetSvrCmds(void *row, char* buf);                    // Request commands from server
                                                                     // Spinner parameters
    static void setSpinnerParameters(struct valueSpinnerParams_t* val, void *row, char *buf);
    static void setPanelSpinnerParameters(void *row, char *buf);     // Panel length spinner parameters
    static void setUID(void *row, char *buf);                        // Set unique identifier
    static void setOffsetSpinnerParameters(void *row, char *buf);    // Offset spinner parameters
    //static void setApplicatorOffset(void *row, char *buf);           // Applicator offset
    static void setMmPerPulse(void *row, char *buf);                 // Milimeters per pulse
    //static void setPulsesHomeToStart(void *row, char *buf);          // Distance home to start
    static void setPulsesHomeToBead(void *row, char *buf);            // Distance home to Bead
    //static void setPulsesStartToEnd(void *row, char *buf);           // Distance start to end
    static void setPulsesBeadToEnd(void *row, char *buf);             // Distance Bead to End
    static void setPulsesHomeToWet(void *row, char *buf);            // Distance Home to Wet
    static void setTimeOut(void *row, char *buf);                    // Set timeouts
    static void setDelay(void *row, char *buf);                      // Set delays
    
    static void testLCD(void *row, char* buf);                       // Test the LCD
    static void sdDir(void *row, char* buf);                         // Print SD dir
    static void sdRmDir(void *row, char* buf);                       // Remove a directory - must be empty
    static void sdRm(void *row, char* buf);                          // Remove a file
    static void sdPr(void *row, char *buf);                          // Print file from SD card
    static void setIndicatorsEnabled(void *row, char *buf);          // Enable/disable the LCD indicators
    static void restart(void *row, char* buf);                       // Soft reset
    static void sdTail(void *row, char *buf);                        // Tail a file from SD card
    static void setOP(void *row, char *buf);                         // Set an output pin
    static void setMetricImperial(void *row, char *buf);             // Set number system
    //static void dumpStats(void *row, char* buf);                     // Print statistics
    static void setMACAddress(void *row, char *buf);                 // Ethernet MAC address
    static void rtcGetTemp(void *row, char* buf);                    // Print temperature from the RTC
    static void setRTCTime(void *row, char *buf);                    // Set RTC time
    static void ipData(void *row, char* buf);                        // Dump ip addresses.
    static void batchData(void *row, char* buf);                     // Dump current batch data
    static void clearBatchData(void *row, char* buf);                // Reset batch and bead data
    static void setBatchID(void *row, char *buf);                    // Set current batch ID
    static void setBeadID(void *row, char *buf);                     // Set current bead ID
    static void batchUploadData(void *row, char* buf);               // Print batch data from upload
    static void clearUploadDir(void *row, char* buf);                // Empty the upload directory
    static void setProjectID(void *row, char *buf);                  // Set the project ID
    static void login(void *row, char* buf);                         // Login to access console
    static void accessCode(void *row, char* buf);                    // Set login code
    static void touch(void *row, char *buf);                         // Pretend button touch
    static void prIO(void *row, char* buf);                          // Dump the indicator states
   
    //void consoleRestartAppCmd(void *row, char *buf);               // Restart the application
    //char* consoleGetQuotedString(char **strP);                     // Get quoted string from the input
    //void consoleLog(void *row, char *buf);                         // Logging on or off
    //void consoleDoVoteULIntervalMins(void *row, char *buf);        // Minutes between vote upload checks
    //void consoleConfigSyncIntervalMins(void *row, char *buf);      // Minutes between config downloads
    //void consoleDoBaseUrl(void *row, char *buf);                   // Set base URL
    //void consoleDoFactoryConfig(void *row, char *buf);             // reset config to factory defaults
    //void consoleClearEEPROM(void *row, char *buf);                 // Wipe the EEPROM to zeros
    
    static const struct commandDefTable_t commandDefTable[];         // The table of commands
  
  public:
    void begin(void);                                                 // Start the console
    void manageCommands(void);                                        // Interpret incoming commands
    static void consoleConfig(void *row, char* buf);                  // Dump the config
    consoleBuf_t consoleBuf;                                          // Command string buffer
    void timeToStr(char *buf, size_t size, uint32_t time);            // Convert UNIX time to a string
    //void doCommands(char *buf);                                       // Process the commands
    void doCommands(char *buf, bool force = true);                    // Process the commands

    ///void consoleParseFile(char *buf);                                     // Parse a file of commands
    //void consoleRequestConfigFile(void *row, char* buf);                  // Start config download
    //void consoleRequestUpload(void *row, char* buf);                      // Request for votes upload
    //void consoleDoReset(void *row, char *buf);                            // Use watchdog to reset.

    static const char UIDCmd[];
    static const char verCmd[];
    static const char setConfigBaseURLCmd[];
    static const char setPanelLengthCmd[];
    static const char setOffsetBottomCmd[];
    static const char setOffsetTopCmd[];
    static const char setPanelLengthSpinnerCmd[];
    static const char setOffsetSpinnerCmd[];
    static const char setMmPerPulseCmd[];
    //static const char setPulsesHomeToStartCmd[];
    //static const char setPulsesStartToEndCmd[];

    static const char setPulsesHomeToBeadCmd[];
    static const char setPulsesBeadToEndCmd[];
    static const char setPulsesHomeToWetCmd[];

    static const char setTimeOutCmd[];
    static const char setDelayCmd[];
    
    static const char setIndicatorsEnabledCmd[];
    static const char setMetricImpCmd[];
    static const char setMACCmd[];
};

extern consoleClass console;

#endif
