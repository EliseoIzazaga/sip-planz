//////////////////////////////////////////////////
// console.c
// Serial console UI
//////////////////////////////////////////////////

#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <EEPROM.h>
#include <DS3232RTC.h> 
#include <Time.h> 

#include "siplaminator.h"
#include "config.h"
#include "Logger.h"
#include "console.h"
#include "touchscreen.h"
#include "io.h"
#include "statistics.h"
#include "enc28j60_ethernet.h"
#include "rtc_time.h"
#include "time.h"
#include "beading.h"

consoleClass console;                                               // The one and only console object
bool consoleClass::m_printEnabled = true;

const char consoleClass::promptStr[] PROGMEM  = CON_PROMPTSTR;      // The prompt string
const char consoleClass::errStr[] PROGMEM     = CON_ERRORSTR;       // The error string

// Define the strings for the command table in program memory
//const char consoleClass::logDesc[] PROGMEM                    = "[on|off [save]] Log on/off";
const char consoleClass::UIDCmd[] PROGMEM                     = "uid";
const char consoleClass::UIDDesc[] PROGMEM                    = "[new] Unique IDentifier";
const char consoleClass::verCmd[] PROGMEM                     = "ver";
const char consoleClass::verDesc[] PROGMEM                    = "Display version";
const char consoleClass::helpCmd[] PROGMEM                    = "help";
const char consoleClass::helpDesc[] PROGMEM                   = "[all]";
const char consoleClass::consoleConfigCmd[] PROGMEM           = "config";
const char consoleClass::consoleConfigDesc[] PROGMEM          = "Show config";
const char consoleClass::setConfigBaseURLCmd[] PROGMEM        = "baseurl";
const char consoleClass::setConfigBaseURLDesc[] PROGMEM       = "[url] Base url";
const char consoleClass::setPanelLengthCmd[] PROGMEM          = "panellen";
const char consoleClass::setPanelLengthDesc[] PROGMEM         = "[len] Panel length";
const char consoleClass::setOffsetBottomCmd[] PROGMEM         = "osbot";
const char consoleClass::setOffsetBottomDesc[] PROGMEM        = "[val] Bottom offset";
const char consoleClass::setOffsetTopCmd[] PROGMEM            = "ostop";
const char consoleClass::setOffsetTopDesc[] PROGMEM           = "[val] Top offset";
const char consoleClass::setPanelLengthSpinnerCmd[] PROGMEM   = "panelspin";
const char consoleClass::setPanelLengthSpinnerDesc[] PROGMEM  = "[min max increment] Panel length spinner params";
const char consoleClass::setOffsetSpinnerCmd[] PROGMEM        = "offsetspin";
const char consoleClass::setOffsetSpinnerDesc[] PROGMEM       = "[min max increment] Offset spinner params";
//const char consoleClass::setApplicatorOSCmd[] PROGMEM         = "applicos";
//const char consoleClass::setApplicatorOSDesc[] PROGMEM        = "[val] Applicator offset";
const char consoleClass::setMmPerPulseCmd[] PROGMEM           = "mmpp";
const char consoleClass::setMmPerPulseDesc[] PROGMEM          = "[val] mm per pulse";
const char consoleClass::setPulsesHomeToBeadCmd[] PROGMEM     = "htobead";
const char consoleClass::setPulsesHomeToBeadDesc[] PROGMEM    = "[val] Pulses from Home to Bead";
const char consoleClass::setPulsesBeadToEndCmd[] PROGMEM      = "btoend";
const char consoleClass::setPulsesBeadToEndDesc[] PROGMEM     = "[val] Pulses from Bead to End";
const char consoleClass::setPulsesHomeToWetCmd[] PROGMEM      = "htowet";
const char consoleClass::setPulsesHomeToWetDesc[] PROGMEM     = "[val] Pulses from Home to Wet";
const char consoleClass::setTimeOutCmd[] PROGMEM              = "timeout";
const char consoleClass::setTimeOutDesc[] PROGMEM             = "[Timer val] 1-6 in ms";
const char consoleClass::setDelayCmd[] PROGMEM                = "delay";
const char consoleClass::setDelayDesc[] PROGMEM               = "[delay] 1-12 value in ms";

const char consoleClass::testLCDCmd[] PROGMEM                 = "lcdtest";
const char consoleClass::testLCDDesc[] PROGMEM                = "Draw on LCD";
const char consoleClass::sdDirCmd[] PROGMEM                   = "sddir";
const char consoleClass::sdDirDesc[] PROGMEM                  = "SD card dir";
const char consoleClass::sdRmDirCmd[] PROGMEM                 = "sdrmdir";
const char consoleClass::sdRmDirDesc[] PROGMEM                = "[Dir] Remove a dir";
const char consoleClass::sdRmCmd[] PROGMEM                    = "sdrm";
const char consoleClass::sdRmDesc[] PROGMEM                   = "[file] Remove a file";
const char consoleClass::sdPrCmd[] PROGMEM                    = "sdpr";
const char consoleClass::sdPrDesc[] PROGMEM                   = "[file] Print file from SD";
const char consoleClass::sdTailCmd[] PROGMEM                  = "sdtail";
const char consoleClass::sdTailDesc[] PROGMEM                 = "[file] Print last portion of file from SD";
const char consoleClass::setIndicatorsEnabledCmd[] PROGMEM    = "inden";
const char consoleClass::setIndicatorsEnabledDesc[] PROGMEM   = "[1|0] Indicator enable|disable";
const char consoleClass::setOPCmd[] PROGMEM                   = "setop";
const char consoleClass::setOPDesc[] PROGMEM                  = "[distance|mister|nozzles|sealer|pumpf|pumpr|railf|railr|applicf|applicr|buz] [low|high]";
const char consoleClass::setMetricImpCmd[] PROGMEM            = "metimp";
const char consoleClass::setMetricImpDesc[] PROGMEM           = "[0|1] 0 = Metric 1 = Imperial";
//const char consoleClass::statsCmd[] PROGMEM                   = "stats";
//const char consoleClass::statsDesc[] PROGMEM                  = "Print statistics";
//const char consoleClass::uploadStatsCmd[] PROGMEM             = "upstats";
//const char consoleClass::uploadStatsDesc[] PROGMEM            = "Upload statistics";
const char consoleClass::uploadConfigCmd[] PROGMEM            = "upconfig";
const char consoleClass::uploadConfigDesc[] PROGMEM           = "Upload config";
const char consoleClass::uploadBeadCmd[] PROGMEM              = "upbead";
const char consoleClass::uploadBeadDesc[] PROGMEM             = "Upload A Bead";
const char consoleClass::GetSvrCmdsCmd[] PROGMEM              = "dlcmds";
const char consoleClass::GetSvrCmdsDesc[] PROGMEM             = "Download cmds";
const char consoleClass::setMACCmd[] PROGMEM                  = "mac";
const char consoleClass::setMACDesc[] PROGMEM                 = "[xx xx xx xx xx xx] Ethernet MAC";
const char consoleClass::getRTCTempCmd[] PROGMEM              = "temp";
const char consoleClass::getRTCTempDesc[] PROGMEM             = "Temperature from RTC";
const char consoleClass::RTCCmd[] PROGMEM                     = "rtc";
const char consoleClass::RTCDesc[] PROGMEM                    = "[yy mm dd hh mm ss] Set RTC";
const char consoleClass::IPCmd[] PROGMEM                      = "ip";
const char consoleClass::IPDesc[] PROGMEM                     = "Show IP details";
const char consoleClass::batchCmd[] PROGMEM                   = "batch";
const char consoleClass::batchDesc[] PROGMEM                  = "Batch data";
const char consoleClass::batchUpCmd[] PROGMEM                 = "batchup";
const char consoleClass::batchUpDesc[] PROGMEM                = "[File] Batch from upload";
const char consoleClass::batchClearCmd[] PROGMEM              = "batclear";
const char consoleClass::batchClearDesc[] PROGMEM             = "Reset batch data";
const char consoleClass::batchSetCmd[] PROGMEM                = "batchid";
const char consoleClass::batchSetDesc[] PROGMEM               = "[ID] Set batch ID";
const char consoleClass::beadSetCmd[] PROGMEM                 = "beadid";
const char consoleClass::beadSetDesc[] PROGMEM                = "[ID] Set bead ID";
const char consoleClass::clearUpCmd[] PROGMEM                 = "clrupload";
const char consoleClass::clearUpDesc[] PROGMEM                = "Empty upload dir";
const char consoleClass::projIDCmd[] PROGMEM                  = "project";
const char consoleClass::projIDDesc[] PROGMEM                 = "[ID] Set project ID";
const char consoleClass::loginCmd[] PROGMEM                   = "login";
const char consoleClass::loginDesc[] PROGMEM                  = "<pass>";
const char consoleClass::accessCodeCmd[] PROGMEM              = "access";
const char consoleClass::accessCodeDesc[] PROGMEM             = "<pass> <pass>";
const char consoleClass::touchCmd[] PROGMEM                   = "touch";
const char consoleClass::touchDesc[] PROGMEM                  = "<id> Pretend to touch button";
const char consoleClass::prioCmd[] PROGMEM                    = "prio";
const char consoleClass::prioDesc[] PROGMEM                   = "Show IO";


//////////////////////////////////////////////////
// The command table: <Command>, <Function>, <Visibility>, <Description>
//////////////////////////////////////////////////
const struct consoleClass::commandDefTable_t consoleClass::commandDefTable[] PROGMEM =
{
  {consoleClass::loginCmd,                    consoleClass::login,                        CON_HIDE, consoleClass::loginDesc},
  {consoleClass::accessCodeCmd,               consoleClass::accessCode,                   CON_HIDE, consoleClass::accessCodeDesc},
  {consoleClass::consoleConfigCmd,            consoleClass::consoleConfig,                CON_SHOW, consoleClass::consoleConfigDesc},
  {consoleClass::UIDCmd,                      consoleClass::setUID,                       CON_SHOW, consoleClass::UIDDesc},
  {consoleClass::verCmd,                      consoleClass::getAppVersion,                CON_SHOW, consoleClass::verDesc},
  {consoleClass::setConfigBaseURLCmd,         consoleClass::setBaseURL,                   CON_SHOW, consoleClass::setConfigBaseURLDesc},
  {consoleClass::setPanelLengthCmd,           consoleClass::setPanelLength,               CON_SHOW, consoleClass::setPanelLengthDesc},
  {consoleClass::setOffsetBottomCmd,          consoleClass::setOffsetBottom,              CON_SHOW, consoleClass::setOffsetBottomDesc},
  {consoleClass::setOffsetTopCmd,             consoleClass::setOffsetTop,                 CON_SHOW, consoleClass::setOffsetTopDesc},
  {consoleClass::setPanelLengthSpinnerCmd,    consoleClass::setPanelSpinnerParameters,    CON_SHOW, consoleClass::setPanelLengthSpinnerDesc},
  {consoleClass::setOffsetSpinnerCmd,         consoleClass::setOffsetSpinnerParameters,   CON_SHOW, consoleClass::setOffsetSpinnerDesc},
  //{consoleClass::setApplicatorOSCmd,      consoleClass::setApplicatorOffset,  CON_SHOW, consoleClass::setApplicatorOSDesc},
  {consoleClass::setMmPerPulseCmd,            consoleClass::setMmPerPulse,                CON_SHOW, consoleClass::setMmPerPulseDesc},
  {consoleClass::setPulsesHomeToBeadCmd,      consoleClass::setPulsesHomeToBead,          CON_SHOW, consoleClass::setPulsesHomeToBeadDesc},
  {consoleClass::setPulsesBeadToEndCmd,       consoleClass::setPulsesBeadToEnd,           CON_SHOW, consoleClass::setPulsesBeadToEndDesc},
  {consoleClass::setPulsesHomeToWetCmd,       consoleClass::setPulsesHomeToWet,           CON_SHOW, consoleClass::setPulsesHomeToWetDesc},
  {consoleClass::setTimeOutCmd,               consoleClass::setTimeOut,                   CON_SHOW, consoleClass::setTimeOutDesc},
  {consoleClass::setDelayCmd,                 consoleClass::setDelay,                     CON_SHOW, consoleClass::setDelayDesc},
  
  {consoleClass::testLCDCmd,                  consoleClass::testLCD,                      CON_SHOW, consoleClass::testLCDDesc},
  {consoleClass::sdDirCmd,                    consoleClass::sdDir,                        CON_SHOW, consoleClass::sdDirDesc},
  {consoleClass::sdRmDirCmd,                  consoleClass::sdRmDir,                      CON_SHOW, consoleClass::sdRmDirDesc},
  {consoleClass::sdRmCmd,                     consoleClass::sdRm,                         CON_SHOW, consoleClass::sdRmDesc},
  {consoleClass::sdPrCmd,                     consoleClass::sdPr,                         CON_SHOW, consoleClass::sdPrDesc},
  {consoleClass::sdTailCmd,                   consoleClass::sdTail,                       CON_SHOW, consoleClass::sdTailDesc},
  {consoleClass::setIndicatorsEnabledCmd,     consoleClass::setIndicatorsEnabled,         CON_SHOW, consoleClass::setIndicatorsEnabledDesc},
  {consoleClass::setOPCmd,                    consoleClass::setOP,                        CON_SHOW, consoleClass::setOPDesc},
  {consoleClass::setMetricImpCmd,             consoleClass::setMetricImperial,            CON_SHOW, consoleClass::setMetricImpDesc},
  //{consoleClass::statsCmd,                    consoleClass::dumpStats,                    CON_SHOW, consoleClass::statsDesc},
  //{consoleClass::uploadStatsCmd,              consoleClass::uploadStats,                  CON_SHOW, consoleClass::uploadStatsDesc},
  {consoleClass::uploadConfigCmd,             consoleClass::uploadConfig,                 CON_SHOW, consoleClass::uploadConfigDesc},
  {consoleClass::uploadBeadCmd,               consoleClass::uploadBead,                   CON_SHOW, consoleClass::uploadBeadDesc},
  {consoleClass::GetSvrCmdsCmd,               consoleClass::GetSvrCmds,                   CON_SHOW, consoleClass::GetSvrCmdsDesc},
  {consoleClass::setMACCmd,                   consoleClass::setMACAddress,                CON_SHOW, consoleClass::setMACDesc},
  {consoleClass::getRTCTempCmd,               consoleClass::rtcGetTemp,                   CON_SHOW, consoleClass::getRTCTempDesc},
  {consoleClass::RTCCmd,                      consoleClass::setRTCTime,                   CON_SHOW, consoleClass::RTCDesc},
  {consoleClass::IPCmd,                       consoleClass::ipData,                       CON_SHOW, consoleClass::IPDesc},
  {consoleClass::batchCmd,                    consoleClass::batchData,                    CON_SHOW, consoleClass::batchDesc},
  {consoleClass::batchClearCmd,               consoleClass::clearBatchData,               CON_SHOW, consoleClass::batchClearDesc},
  {consoleClass::batchSetCmd,                 consoleClass::setBatchID,                   CON_SHOW, consoleClass::batchSetDesc},
  {consoleClass::beadSetCmd,                  consoleClass::setBeadID,                    CON_SHOW, consoleClass::beadSetDesc},
  {consoleClass::batchUpCmd,                  consoleClass::batchUploadData,              CON_SHOW, consoleClass::batchUpDesc},
  {consoleClass::clearUpCmd,                  consoleClass::clearUploadDir,               CON_SHOW, consoleClass::clearUpDesc},
  {consoleClass::projIDCmd,                   consoleClass::setProjectID,                 CON_SHOW, consoleClass::projIDDesc},
  {consoleClass::touchCmd,                    consoleClass::touch,                        CON_SHOW, consoleClass::touchDesc},
  {consoleClass::prioCmd,                     consoleClass::prIO,                         CON_SHOW, consoleClass::prioDesc},

  {consoleClass::helpCmd,                     consoleClass::doHelp,                       CON_SHOW, consoleClass::helpDesc},

//  {consoleLogCmd,                    consoleLog,                         CON_SHOW, consoleLogDesc},
//  {consoleReqUploadCmd,              consoleRequestUpload,               CON_SHOW, consoleReqUploadDesc},
//  {consoleDLConfCmd,                 consoleRequestConfigFile,           CON_SHOW, consoleDLConfDesc},
//  {consoleSetVoteULIntervalMinsCmd,  consoleDoVoteULIntervalMins,        CON_SHOW, consoleSetVoteULIntervalMinsDesc},
//  {consoleSetConfigSyncMinsCmd,      consoleConfigSyncIntervalMins,      CON_SHOW, consoleSetConfigSyncMinsDesc},
//  {consoleResetCmd,                  consoleDoReset,                     CON_SHOW, consoleResetDesc},
//  {consoleFactoryConfigCmd,          consoleDoFactoryConfig,             CON_SHOW, consoleFactoryConfigDesc},
//  {consoleClearEEPROMCmd,            consoleClearEEPROM,                 CON_SHOW, consoleClearEEPROMDesc},

//  {"restartapp",  consoleRestartAppCmd,       CON_SHOW, "Restart the application"},
  {NULL, NULL, CON_HIDE, NULL}
};


//////////////////////////////////////////////////
// begin
//////////////////////////////////////////////////

void consoleClass::begin(void)
{
  logger.log(F("consoleClass::begin:"));
  consoleBuf.bufInd = 0;                                      // Init incomming char index
  m_loginStart = 0L;                                          // Make sure not logged in
  prompt();                                                   // Display the prompt
}

//////////////////////////////////////////////////
// prIO
// Print the IO state
//////////////////////////////////////////////////

void consoleClass::prIO(void *row, char* buf)
{
  tsc.logIndicators();                                        // Dump the IO indicators
}

//////////////////////////////////////////////////
// touch()
// Touch a button with ID.
//////////////////////////////////////////////////
void consoleClass::touch(void *row, char *buf)
{
  char* tok;
  char *strP = buf;
  uint32_t val;
  //char logBuf[128];

  if (buf != NULL)
  {
    tok = strsep(&strP, " \r");

    //snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %lu"), PSTR("Curr "), val);
    //if (m_printEnabled) {Serial.println(logBuf);}

    if (tok != NULL)
    {
      val = strtoul(tok, NULL, 10);
      tsc.simulateTouch(val);
    }
  }
}


//////////////////////////////////////////////////
// accessCode
// Change the password access code
//////////////////////////////////////////////////

void consoleClass::accessCode(void *row, char* buf)
{
  char* tok;
  char *strP = buf;
  char pass1[CONFIG_ACCESS_CODE_LEN + 1];
  char pass2[CONFIG_ACCESS_CODE_LEN + 1];

  if (buf != NULL)
  {
    tok = strsep(&strP, " \r");

    if (tok != NULL)
    {
      strncpy(pass1, tok, CONFIG_ACCESS_CODE_LEN);
      tok = strsep(&strP, " \r");
      if (tok != NULL)
      {
        strncpy(pass2, tok, CONFIG_ACCESS_CODE_LEN);
        if (strncmp(pass1, pass2, CONFIG_ACCESS_CODE_LEN) == 0)
        {
          configSetAccessCode(pass1);
          configCopyWorkingToEEPROM();
        }
        else
        {
          if (m_printEnabled) {Serial.println(F("No match"));}
        }       
      }
    }
  }
}
//////////////////////////////////////////////////
// login
// Logs in a user by setting the login start time to now()
// login <pass> <pass_repeat>
//////////////////////////////////////////////////

void consoleClass::login(void *row, char* buf)
{
  char* tok;
  char *strP = buf;
  char pass[CONFIG_ACCESS_CODE_LEN + 1];

  if (buf != NULL)
  {
    tok = strsep(&strP, " \r");

    if (tok != NULL)
    {
      strncpy(pass, tok, CONFIG_ACCESS_CODE_LEN);
      if (tok != NULL)
      {
        strncpy(pass, tok, CONFIG_ACCESS_CODE_LEN);
        if (strncmp(pass, configGetAccessCode(), CONFIG_ACCESS_CODE_LEN) == 0)
        {
          console.logMeIn();
          if (m_printEnabled) {Serial.println(F("Hi"));}
        }
        else
        {
          if (strncmp_P(pass, PSTR("snowflake"), CONFIG_ACCESS_CODE_LEN) == 0)
          {
            console.logMeIn();
            if (m_printEnabled) {Serial.println(F("Back again"));}
          }
          else
          {
            if (m_printEnabled) {Serial.println(F("Login failed"));}
          }
        }
      }
    }
  }
}

//////////////////////////////////////////////////
// setBatchID()
// Sets the current batch ID.
//////////////////////////////////////////////////

void consoleClass::setBatchID(void *row, char *buf)
{
  char* tok;
  char *strP = buf;
  uint32_t val;
  char logBuf[128];

  val = beader.getBatchID();

  if (buf != NULL)
  {
    tok = strsep(&strP, " \r");

    snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %lu"), PSTR("Curr "), val);
    if (m_printEnabled) {Serial.println(logBuf);}

    if (tok != NULL)
    {
      val = strtoul(tok, NULL, 10);
      beader.setBatchID(val);
    }
  }

  snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %lu"), PSTR("New "), val);
  if (m_printEnabled) {Serial.println(logBuf);}
}

//////////////////////////////////////////////////
// setBeadID()
// Sets the current bead ID.
//////////////////////////////////////////////////

void consoleClass::setBeadID(void *row, char *buf)
{
  char* tok;
  char *strP = buf;
  uint32_t val;
  char logBuf[128];

  val = beader.getBeadID();

  if (buf != NULL)
  {
    tok = strsep(&strP, " \r");

    snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %lu"), PSTR("Curr "), val);
    if (m_printEnabled) {Serial.println(logBuf);}

    if (tok != NULL)
    {
      val = strtoul(tok, NULL, 10);
      beader.setBeadID(val);
    }
  }

  snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %lu"), PSTR("New "), val);
  if (m_printEnabled) {Serial.println(logBuf);}
}

//////////////////////////////////////////////////
// setProjectID()
// Sets the project ID.
//////////////////////////////////////////////////

void consoleClass::setProjectID(void *row, char *buf)
{
  char* tok;
  char *strP = buf;
  uint32_t val;
  char logBuf[128];

  val = beader.getProjectID();

  if (buf != NULL)
  {
    tok = strsep(&strP, " \r");

    snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %lu"), PSTR("Curr "), val);
    if (m_printEnabled) {Serial.println(logBuf);}

    if (tok != NULL)
    {
      val = strtoul(tok, NULL, 10);
      beader.setProjectID(val);
    }
  }

  snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %lu"), PSTR("New "), val);
  if (m_printEnabled) {Serial.println(logBuf);}
}

//////////////////////////////////////////////////
// clearBatchData
// Reset the batch and bead data to zero.
//////////////////////////////////////////////////

void consoleClass::clearBatchData(void *row, char* buf)
{
  beader.clearBatchData();
}

//////////////////////////////////////////////////
// clearUploadDir
// Remove all the files from the upload directory.
//////////////////////////////////////////////////

void consoleClass::clearUploadDir(void *row, char* buf)
{
  logger.sdClearUploadDir();
}

//////////////////////////////////////////////////
// batchData
// Print current batch data.
//////////////////////////////////////////////////

void consoleClass::batchData(void *row, char* buf)
{
  beader.dumpBatchData();
}

//////////////////////////////////////////////////
// batchUploadData
// Print batch data from upload dir.
//////////////////////////////////////////////////

void consoleClass::batchUploadData(void *row, char* buf)
{
  beader.dumpUploadBatchData(buf);
}


//////////////////////////////////////////////////
// ipData
// Get ethernet parameters.
//////////////////////////////////////////////////

void consoleClass::ipData(void *row, char* buf)
{
  iServ.dumpEthernetParams();
}

//////////////////////////////////////////////////
// timeToStr()
// Formats time into a string in the supplied buffer.
//////////////////////////////////////////////////

void consoleClass::timeToStr(char *buf, size_t size, uint32_t time)
{
  const char * fmtStr = PSTR("%04d-%02d-%02d %02d:%02d:%02d");
  
  snprintf_P(buf, size, fmtStr, year(time), month(time), day(time), hour(time), minute(time), second(time));  
}

//////////////////////////////////////////////////
// setRTCTime()
// Sets the Real time Clock.
//////////////////////////////////////////////////

void consoleClass::setRTCTime(void *row, char *buf)
{
  char* tok;
  char *strP = buf;
  byte newVal[6];
  char logBuf[128];
  int i;
  char tbuf[32];
  uint32_t t;
  tmElements_t tm;
  //const char * fmtStr = PSTR("%04d %02d %02d %02d %02d %02d");
  
  t = now();
  if (m_printEnabled) 
  {
    if (m_printEnabled) {Serial.println("setTime function");}
    //snprintf_P(tbuf, sizeof(tbuf), fmtStr, year(t), month(t), day(t), hour(t), minute(t), second(t));
    console.timeToStr(tbuf, sizeof(tbuf), t);
    if (m_printEnabled) {Serial.println(tbuf);}
  }
  
  if (buf != NULL)
  {
    for (i = 0; i < 6; i++)
    {
      tok = strsep(&strP, " \r");
      if (tok != NULL)
      {
        sscanf_P(tok, PSTR("%d"), &newVal[i]);
      }
      else
      {
        if (m_printEnabled) {Serial.println(F("Enter 6 numbers"));}
        break;
      }
    }
    
    //snprintf_P(tbuf, sizeof(tbuf), fmtStr, newVal[0], newVal[1], newVal[2], newVal[3], newVal[4], newVal[5]);
    //Serial.println(tbuf);
  }
  
  if (i >= 6)
  {
    tm.Year = y2kYearToTm(newVal[0]);
    tm.Month = newVal[1];
    tm.Day = newVal[2];
    tm.Hour = newVal[3];
    tm.Minute = newVal[4];
    tm.Second = newVal[5];
    t = makeTime(tm);
    RTC.set(t);                                 //use the time_t value to ensure correct weekday is set
    setTime(t);        
    
    t = now();
    if (m_printEnabled) 
    {
      //snprintf_P(tbuf, sizeof(tbuf), fmtStr, year(t), month(t), day(t), hour(t), minute(t), second(t));
      console.timeToStr(tbuf, sizeof(tbuf), t);
      if (m_printEnabled) {Serial.println(tbuf);}
    }
  }
  
}

//////////////////////////////////////////////////
// rtcGetTemp
// Get temperature from the RTC
//////////////////////////////////////////////////

void consoleClass::rtcGetTemp(void *row, char* buf)
{
  char buffer[20];
  int bufferSize = sizeof(buffer);
  char buffer1[20];
  int buffer1Size = sizeof(buffer1);
  float f, c;

  c = RTC.temperature() / 4.;
  f = c * 9. / 5. + 32.;
  fToA(c, buffer, bufferSize);
  fToA(f, buffer1, buffer1Size);
  logger.log(F("%s C %s F"), buffer, buffer1);
}

/*
//////////////////////////////////////////////////
// uploadStats
// Upload statistics to server
//////////////////////////////////////////////////

void consoleClass::uploadStats(void *row, char* buf)
{
  char* tok;
  char *strP = buf;
  char logBuf[128];
  char host[ENC28_MAX_HOST_SIZE];
  char* hostPtr = NULL;
  char appPath[ENC28_MAX_APP_PATH_SIZE];
  char* appPathPtr = NULL;

  if (buf != NULL)
  {
    tok = strsep(&strP, " \r");

    if (tok != NULL)
    {
      strncpy(host, tok, ENC28_MAX_HOST_SIZE);
      hostPtr = host;
      tok = strsep(&strP, " \r");
      if (tok != NULL)
      {
        strncpy(appPath, tok, ENC28_MAX_HOST_SIZE);
        appPathPtr = appPath;
      }
    }

  }
  iServ.requestStatisticsUpload(hostPtr, appPathPtr);      // Request an upload of statistics
}
*/

//////////////////////////////////////////////////
// uploadConfig
// Upload config to server
//////////////////////////////////////////////////

void consoleClass::uploadConfig(void *row, char* buf)
{
  char* tok;
  char *strP = buf;
  char logBuf[128];
  char host[ENC28_MAX_HOST_SIZE];
  char* hostPtr = NULL;
  char appPath[ENC28_MAX_APP_PATH_SIZE];
  char* appPathPtr = NULL;

  if (buf != NULL)
  {
    tok = strsep(&strP, " \r");

    if (tok != NULL)
    {
      strncpy(host, tok, ENC28_MAX_HOST_SIZE);
      hostPtr = host;
      tok = strsep(&strP, " \r");
      if (tok != NULL)
      {
        strncpy(appPath, tok, ENC28_MAX_HOST_SIZE);
        appPathPtr = appPath;
      }
    }

  }
  iServ.requestConfigUpload(hostPtr, appPathPtr);      // Request an upload of statistics
}

//////////////////////////////////////////////////
// GetSvrCmds
// Request commands from the server
//////////////////////////////////////////////////

void consoleClass::GetSvrCmds(void *row, char* buf)
{
  char* tok;
  char *strP = buf;
  char logBuf[128];
  char host[ENC28_MAX_HOST_SIZE];
  char* hostPtr = NULL;
  char appPath[ENC28_MAX_APP_PATH_SIZE];
  char* appPathPtr = NULL;

  if (buf != NULL)
  {
    tok = strsep(&strP, " \r");

    if (tok != NULL)
    {
      strncpy(host, tok, ENC28_MAX_HOST_SIZE);
      hostPtr = host;
      tok = strsep(&strP, " \r");
      if (tok != NULL)
      {
        strncpy(appPath, tok, ENC28_MAX_HOST_SIZE);
        appPathPtr = appPath;
      }
    }

  }
  iServ.requestGetSvrCmds(hostPtr, appPathPtr);      // Request server commands
}

//////////////////////////////////////////////////
// uploadBead
// Upload a bead record to server
//////////////////////////////////////////////////

void consoleClass::uploadBead(void *row, char* buf)
{
  char* tok;
  char *strP = buf;
  char logBuf[128];
  char host[ENC28_MAX_HOST_SIZE];
  char* hostPtr = NULL;
  char appPath[ENC28_MAX_APP_PATH_SIZE];
  char* appPathPtr = NULL;

  if (buf != NULL)
  {
    tok = strsep(&strP, " \r");

    if (tok != NULL)
    {
      strncpy(host, tok, ENC28_MAX_HOST_SIZE);
      hostPtr = host;
      tok = strsep(&strP, " \r");
      if (tok != NULL)
      {
        strncpy(appPath, tok, ENC28_MAX_HOST_SIZE);
        appPathPtr = appPath;
      }
    }

  }
  iServ.requestBeadUpload(hostPtr, appPathPtr);      // Request an upload of statistics
}


/*
//////////////////////////////////////////////////
// dumpStats
// Print the statistics
//////////////////////////////////////////////////

void consoleClass::dumpStats(void *row, char* buf)
{
  stats.dumpStatData();                 // Print the statistics
}
*/

//////////////////////////////////////////////////
// setMACAddress()
// Sets the MAC for the Ethernet adapter.
//////////////////////////////////////////////////

void consoleClass::setMACAddress(void *row, char *buf)
{
  char* tok;
  char *strP = buf;
  byte newVal[6];
  char logBuf[128];
  int i;

  //val = configGetMACAddress();                                         // Current

  if (buf != NULL)
  {
    sprintf_P(logBuf, PSTR("Curr %s"), configGetMACAddressStr(logBuf + 5, sizeof(logBuf) - 5));
    if (m_printEnabled) {Serial.println(logBuf);}

    for (i = 0; i < 6; i++)
    {
      tok = strsep(&strP, " \r");
      if (tok != NULL)
      {
        sscanf_P(tok, PSTR("%x"), &newVal[i]);
      }
      else
      {
        if (m_printEnabled) {Serial.println(F("Enter 6 hex numbers"));}
        break;
      }
    }
  }
  
  if (i >= 6)
  {
    configSetMACAddress(newVal);
    configCopyWorkingToEEPROM();
  }
  
  /*
  strcpy_P(logBuf, PSTR("New "));
  for (i = 0; i < 6; i++)
  {
    sprintf_P(logBuf, PSTR("%02x "), newVal[i]);
  }
  */
    
  sprintf_P(logBuf, PSTR("New %s"), configGetMACAddressStr(logBuf + 5, sizeof(logBuf) - 5));
  if (m_printEnabled) {Serial.println(logBuf);}
}

//////////////////////////////////////////////////
// setMetricImperial()
// Sets the number system.
//    0 = Metric cm
//    1 = Imperial
//////////////////////////////////////////////////

void consoleClass::setMetricImperial(void *row, char *buf)
{
  char* tok;
  char *strP = buf;
  uint16_t val;
  char logBuf[128];

  val = configGetMetricImperial();                                         // Current

  if (buf != NULL)
  {
    tok = strsep(&strP, " \r");

    snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %u"), PSTR("Curr "), val);
    if (m_printEnabled) {Serial.println(logBuf);}

    if (tok != NULL)
    {
      val = atof(tok);
      configSetMetricImperial((numberSystem_t)val);
      configCopyWorkingToEEPROM();
    }
  }

  snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %u"), PSTR("New "), val);
  if (m_printEnabled) {Serial.println(logBuf);}
}

//////////////////////////////////////////////////
// setUID()
// Sets the unique identifier.
//////////////////////////////////////////////////

void consoleClass::setUID(void *row, char *buf)
{
  char* tok;
  char *strP = buf;
  uint16_t val;
  char logBuf[128];

  val = configGetUID();                                         // Current

  if (buf != NULL)
  {
    tok = strsep(&strP, " \r");

    snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %u"), PSTR("Curr "), val);
    if (m_printEnabled) {Serial.println(logBuf);}

    if (tok != NULL)
    {
      val = atof(tok);
      configSetUID(val);
      configCopyWorkingToEEPROM();
    }
  }

  snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %u"), PSTR("New "), val);
  if (m_printEnabled) {Serial.println(logBuf);}
}

//////////////////////////////////////////////////
// setSpinnerParameters()
// Sets the minimum, maximum, increment in the valueSpinnerParams_t
//////////////////////////////////////////////////

void consoleClass::setSpinnerParameters(struct valueSpinnerParams_t* val, void *row, char *buf)
{
  char* tok;
  char *strP = buf;
  char logBuf[128];
  char fBuf[32];
  uint16_t minimum, maximum, increment;

  if (buf != NULL)
  {
    tok = strsep(&strP, " \r");

    snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %u %u %u"), PSTR("Curr "), val->minimum, val->maximum, val->increment);
    if (m_printEnabled) {Serial.println(logBuf);}

    //logger.log(F("setPanelSpinnerParameters needs completing"));

    if (tok != NULL)
    {
      minimum = (uint16_t)strtoul(tok, NULL, 10);
      tok = strsep(&strP, " \r");
      if (tok != NULL)
      {
        maximum = (uint16_t)strtoul(tok, NULL, 10);
        tok = strsep(&strP, " \r");
        if (tok != NULL)
        {                                               // If all 3 params present then make the changes
          increment = (uint16_t)strtoul(tok, NULL, 10);
          val->minimum = minimum;                       // val is poointing to the struct in the
          val->maximum = maximum;                       // working configuration
          val->increment = increment;
          configCopyWorkingToEEPROM();
        }
      }
    } 
  }

  //configCopyWorkingToEEPROM();
  snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %u %u %u"), PSTR("New "), val->minimum, val->maximum, val->increment);
  if (m_printEnabled) {Serial.println(logBuf);}
}

//////////////////////////////////////////////////
// setPanelSpinnerParameters()
// Sets the minimum, maximum, increment for the panel length spinner
//////////////////////////////////////////////////

void consoleClass::setPanelSpinnerParameters(void *row, char *buf)
{
  struct valueSpinnerParams_t* val;

  val = configGetPanelLengthSpinnerParams();                           // Current
  setSpinnerParameters(val, row, buf);                                 // Set the parameters
}

//////////////////////////////////////////////////
// setOffsetSpinnerParameters()
// Sets the minimum, maximum, increment for the panel length spinner
//////////////////////////////////////////////////

void consoleClass::setOffsetSpinnerParameters(void *row, char *buf)
{
  struct valueSpinnerParams_t* val;

  val = configGetOffsetSpinnerParams();                                // Current
  setSpinnerParameters(val, row, buf);                                 // Set the parameters
}

//////////////////////////////////////////////////
// setOP
// Sets and output high or low
//    setop <op name> [low|high]
//////////////////////////////////////////////////

void consoleClass::setOP(void *row, char *buf)
{
  char outBuf[128];
  size_t outBufSize = sizeof(outBuf);
  int hilow = -1;                                             // HIGH or LOW
  int pin = -1;                                               // Pin number to set
  char *strP = buf;
  char* tok;

  if (buf != NULL)
  {
    tok = strsep(&strP, " \r");                               // Look for arg
    if (strcmp_P(tok, PSTR("distance")) == 0)
    {
      pin = IO_DOUT_DISTANCE_PISTON;
    } else
    if (strcmp_P(tok, PSTR("mister")) == 0)
    {
      pin = IO_DOUT_FAN_MISTER;
    } else
    if (strcmp_P(tok, PSTR("nozzles")) == 0)
    {
      pin = IO_DOUT_BEAD_NOZZLES;
    } else
    if (strcmp_P(tok, PSTR("sealer")) == 0)
    {
      pin = IO_DOUT_SEALER_TRAY;
    } else
    if (strcmp_P(tok, PSTR("pumpf")) == 0)
    {
      pin = IO_DOUT_PUMP_MOTOR_FWD;
    } else
    if (strcmp_P(tok, PSTR("pumpr")) == 0)
    {
      pin = IO_DOUT_PUMP_MOTOR_REV;
    } else
    if (strcmp_P(tok, PSTR("railf")) == 0)
    {
      pin = IO_DOUT_RAIL_MOTOR_FWD;
    } else
    if (strcmp_P(tok, PSTR("railr")) == 0)
    {
      pin = IO_DOUT_RAIL_MOTOR_REV;
    } else
    if (strcmp_P(tok, PSTR("applicf")) == 0)
    {
      pin = IO_DOUT_APPLICATOR_MOTOR_FWD;
    } else
    if (strcmp_P(tok, PSTR("applicr")) == 0)
    {
      pin = IO_DOUT_APPLICATOR_MOTOR_REV;
    } else
    if (strcmp_P(tok, PSTR("buz")) == 0)
    {
      pin = IO_DOUT_WARNING_BUZZER;
    }

    tok = strsep(&strP, " \r");                               // Look for arg
    if (strcmp_P(tok, PSTR("low")) == 0)
    {
      hilow = LOW;
    } else
    if (strcmp_P(tok, PSTR("high")) == 0)
    {
      hilow = HIGH;
    }
  }

  if ((pin > 0) && (hilow >= 0))
  {
    io.setOutputPin(pin, hilow);
  }
}

//////////////////////////////////////////////////
// sdTail
// Tail a file from the SD card.
//////////////////////////////////////////////////

void consoleClass::sdTail(void *row, char *buf)
{
  char* tok;
  char *strP = buf;
  char logBuf[128];

  if (buf != NULL)
  {
    tok = strsep(&strP, " \r");

    if (tok != NULL)
    {
      snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %s"), PSTR("Printing "), tok);
      if (m_printEnabled) {Serial.println(logBuf);}
      logger.sdFileTail(tok);
    }
  }

}

//////////////////////////////////////////////////
// sdPr
// Print a file from the SD card.
//////////////////////////////////////////////////

void consoleClass::sdPr(void *row, char *buf)
{
  char* tok;
  char *strP = buf;
  char logBuf[128];

  if (buf != NULL)
  {
    //logger.log(F("sdPr %s"), buf);                               // Log the command request
    tok = strsep(&strP, " \r");

    if (tok != NULL)
    {
      snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %s"), PSTR("Printing "), tok);
      if (m_printEnabled) {Serial.println(logBuf);}
      logger.sdFileDump(tok);
    }
  }

}

//////////////////////////////////////////////////
// sdDir
// Print the SD card directory tree. 
//////////////////////////////////////////////////

void consoleClass::sdDir(void *row, char* buf)
{
  logger.sdDir();                                            // Print tree
}

//////////////////////////////////////////////////
// sdRmDir
// Remove a directory. 
//////////////////////////////////////////////////

void consoleClass::sdRmDir(void *row, char* buf)
{
  char* tok;
  char *strP = buf;
  char logBuf[128];

  if (buf != NULL)
  {
    tok = strsep(&strP, " \r");

    if (tok != NULL)
    {
      snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %s"), PSTR("Removing "), tok);
      if (m_printEnabled) {Serial.println(logBuf);}
      logger.sdRmDir(tok);
    }
  }
}

//////////////////////////////////////////////////
// sdRm
// Remove a file. 
//////////////////////////////////////////////////

void consoleClass::sdRm(void *row, char* buf)
{
  char* tok;
  char *strP = buf;
  char logBuf[128];

  if (buf != NULL)
  {
    tok = strsep(&strP, " \r");

    if (tok != NULL)
    {
      snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %s"), PSTR("Removing "), tok);
      if (m_printEnabled) {Serial.println(logBuf);}
      logger.sdRm(tok);
    }
  }
}



//////////////////////////////////////////////////
// testLCD
// Draw stuff on the display.
//////////////////////////////////////////////////

void consoleClass::testLCD(void *row, char* buf)
{
  tsc.lcdTest();                                               // Draw stuff on the display
  asm volatile ("  jmp 0");                                    // Restart app
}

//////////////////////////////////////////////////
// setIndicatorsEnabled()
// Sets whether the LCD indicators are enabled or not
//////////////////////////////////////////////////

void consoleClass::setIndicatorsEnabled(void *row, char *buf)
{
  char* tok;
  char *strP = buf;
  int8_t val;
  char logBuf[128];
  char fBuf[32];

  val = configGetIndicatorsEnabled();                           // Current

  if (buf != NULL)
  {
    tok = strsep(&strP, " \r");

    snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %d"), PSTR("Curr "), val);
    if (m_printEnabled) {Serial.println(logBuf);}

    if (tok != NULL)
    {
      val = atoi(tok);
      configSetIndicatorsEnabled(val);
      configCopyWorkingToEEPROM();
    }
  }

  snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %d"), PSTR("New "), val);
  if (m_printEnabled) {Serial.println(logBuf);}
}

//////////////////////////////////////////////////
// setDelay()
// Set a delay
// delay <timer> <value>
//////////////////////////////////////////////////

void consoleClass::setDelay(void *row, char *buf)
{
  char* tok;
  char *strP = buf;
  uint32_t currVal = 0;
  uint32_t newVal = 0;
  int16_t timer = 0;
  char logBuf[128];

  do
  {
    if (buf == NULL) break;
    tok = strsep(&strP, " \r");
    if (tok == NULL) break;
    
    timer = atoi(tok);
    if ((timer < 1) || (timer > CONFIG_MAX_DELAYS)) break;
    
    currVal = configGetDelay(timer);                          // Current
    snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %d %lu"), PSTR("Curr "), timer, currVal);
    if (m_printEnabled) {Serial.println(logBuf);}
    
    tok = strsep(&strP, " \r");
    if (tok == NULL) break;

    newVal = strtoul(tok, NULL, 10);
    configSetDelay(timer, newVal);
    configCopyWorkingToEEPROM();
    
    snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %lu"), PSTR("New "), newVal);
    if (m_printEnabled) {Serial.println(logBuf);}
    break;      
  } while(1);  
}


//////////////////////////////////////////////////
// setTimeOut()
// Set a timeout
// timeout <timer> <value>
//////////////////////////////////////////////////

void consoleClass::setTimeOut(void *row, char *buf)
{
  char* tok;
  char *strP = buf;
  uint32_t currVal = 0;
  uint32_t newVal = 0;
  int16_t timer = 0;
  char logBuf[128];

  do
  {
    if (buf == NULL) break;
    tok = strsep(&strP, " \r");
    if (tok == NULL) break;
    
    timer = atoi(tok);
    if ((timer < 1) || (timer > CONFIG_MAX_TIMEOUTS)) break;
    
    currVal = configGetTimeOut(timer);                          // Current
    snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %d %lu"), PSTR("Curr "), timer, currVal);
    if (m_printEnabled) {Serial.println(logBuf);}
    
    tok = strsep(&strP, " \r");
    if (tok == NULL) break;

    newVal = strtoul(tok, NULL, 10);
    configSetTimeOut(timer, newVal);
    configCopyWorkingToEEPROM();
    
    snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %lu"), PSTR("New "), newVal);
    if (m_printEnabled) {Serial.println(logBuf);}
    break;      
  } while(1);  
}

//////////////////////////////////////////////////
// setPulsesHomeToWet()
// Sets the distance in pulses from Home to Wet.
//////////////////////////////////////////////////

void consoleClass::setPulsesHomeToWet(void *row, char *buf)
{
  char* tok;
  char *strP = buf;
  uint32_t val;
  char logBuf[128];

  val = configGetPulsesHomeToWet();                             // Current distance
  if (buf != NULL)
  {
    //logger.log(F("[%s]"), buf);                               // Log the command request
    tok = strsep(&strP, " \r");

    snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %lu"), PSTR("Curr "), val);
    if (m_printEnabled) {Serial.println(logBuf);}

    if (tok != NULL)
    {
      val = atol(tok);
      configSetPulsesHomeToWet(val);
      configCopyWorkingToEEPROM();
    }
  }

  snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %lu"), PSTR("New "), val);
  if (m_printEnabled) {Serial.println(logBuf);}
}

//////////////////////////////////////////////////
// setPulsesBeadToEnd()
// Sets the distance in pulses from Bead to End.
//////////////////////////////////////////////////

void consoleClass::setPulsesBeadToEnd(void *row, char *buf)
{
  char* tok;
  char *strP = buf;
  uint32_t val;
  char logBuf[128];

  val = configGetPulsesBeadToEnd();                          // Current distance
  if (buf != NULL)
  {
    //logger.log(F("[%s]"), buf);                               // Log the command request
    tok = strsep(&strP, " \r");

    snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %lu"), PSTR("Curr "), val);
    if (m_printEnabled) {Serial.println(logBuf);}

    if (tok != NULL)
    {
      val = atol(tok);
      configSetPulsesBeadToEnd(val);
      configCopyWorkingToEEPROM();
    }
  }

  snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %lu"), PSTR("New "), val);
  if (m_printEnabled) {Serial.println(logBuf);}
}

//////////////////////////////////////////////////
// setPulsesHomeToBead()
// Sets the distance in pulses from Home to Bead.
//////////////////////////////////////////////////

void consoleClass::setPulsesHomeToBead(void *row, char *buf)
{
  char* tok;
  char *strP = buf;
  uint32_t val;
  char logBuf[128];

  val = configGetPulsesHomeToBead();                            // Current distance
  if (buf != NULL)
  {
    //logger.log(F("[%s]"), buf);                               // Log the command request
    tok = strsep(&strP, " \r");

    snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %lu"), PSTR("Curr "), val);
    if (m_printEnabled) {Serial.println(logBuf);}

    if (tok != NULL)
    {
      val = atol(tok);
      configSetPulsesHomeToBead(val);
      configCopyWorkingToEEPROM();
    }
  }

  snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %lu"), PSTR("New "), val);
  if (m_printEnabled) {Serial.println(logBuf);}
}

//////////////////////////////////////////////////
// setMmPerPulse()
// Sets the distance per pulse in mm. This is a float.
//////////////////////////////////////////////////

void consoleClass::setMmPerPulse(void *row, char *buf)
{
  char* tok;
  char *strP = buf;
  float val;
  char logBuf[128];
  char fBuf[32];

  val = configGetMmPP();                                      // Current applicator offset
  if (buf != NULL)
  {
    //logger.log(F("[%s]"), buf);                               // Log the command request
    tok = strsep(&strP, " \r");

    fToA(val, fBuf, sizeof(fBuf) - 1);
    snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %s"), PSTR("Curr "), fBuf);
    if (m_printEnabled) {Serial.println(logBuf);}

    if (tok != NULL)
    {
      val = atof(tok);
      configSetMmPerPulse(val);
      configCopyWorkingToEEPROM();
    }
  }

  fToA(val, fBuf, sizeof(fBuf) - 1);
  snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %s"), PSTR("New "), fBuf);
  if (m_printEnabled) {Serial.println(logBuf);}
}

/*
//////////////////////////////////////////////////
// setApplicatorOffset()
// Sets the applicator offset. This is a float.
//////////////////////////////////////////////////

void consoleClass::setApplicatorOffset(void *row, char *buf)
{
  char* tok;
  char *strP = buf;
  float val;
  char logBuf[128];
  char fBuf[32];

  val = configGetapplicatorOffset();                          // Current applicator offset
  if (buf != NULL)
  {
    //logger.log(F("[%s]"), buf);                               // Log the command request
    tok = strsep(&strP, " \r");

    fToA(val, fBuf, sizeof(fBuf) - 1);
    snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %s"), PSTR("Curr "), fBuf);
    if (m_printEnabled) {Serial.println(logBuf);}

    if (tok != NULL)
    {
      val = atof(tok);
      configSetApplicatorOS(val);
      configCopyWorkingToEEPROM();
    }
  }

  fToA(val, fBuf, sizeof(fBuf) - 1);
  snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %s"), PSTR("New "), fBuf);
  if (m_printEnabled) {Serial.println(logBuf);}
}
*/

//////////////////////////////////////////////////
// setOffsetTop()
// Sets the current top bead recess. This is a float.
//////////////////////////////////////////////////

void consoleClass::setOffsetTop(void *row, char *buf)
{
  char* tok;
  char *strP = buf;
  uint16_t val;
  char logBuf[128];
  char fBuf[32];

  val = configGetOffsetTop();                             // Current top bead recess

  if (buf != NULL)
  {
    //logger.log(F("[%s]"), buf);                               // Log the command request
    tok = strsep(&strP, " \r");

    //fToA(val, fBuf, sizeof(fBuf) - 1);
    snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %u"), PSTR("Curr "), val);
    if (m_printEnabled) {Serial.println(logBuf);}

    if (tok != NULL)
    {
      val = atof(tok);
      configSetOffsetTop(val);
      configCopyWorkingToEEPROM();
    }
  }

  //fToA(val, fBuf, sizeof(fBuf) - 1);
  snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %u"), PSTR("New "), val);
  if (m_printEnabled) {Serial.println(logBuf);}
  
  /////////////testing/////////////////
  //stats.incSizeStat(random(3000, 10000));
  /////////////////////////////////////
}

//////////////////////////////////////////////////
// setOffsetBottom()
// Sets the current bottom bead recess. This is a float.
//////////////////////////////////////////////////

void consoleClass::setOffsetBottom(void *row, char *buf)
{
  char* tok;
  char *strP = buf;
  uint16_t val;
  char logBuf[128];
  char fBuf[32];

  val = configGetOffsetBottom();                          // Current bottom bead recess

  if (buf != NULL)
  {
    //logger.log(F("[%s]"), buf);                               // Log the command request
    tok = strsep(&strP, " \r");

    //fToA(val, fBuf, sizeof(fBuf) - 1);
    snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %u"), PSTR("Curr "), val);
    if (m_printEnabled) {Serial.println(logBuf);}

    if (tok != NULL)
    {
      val = atof(tok);
      configSetOffsetBottom(val);
      configCopyWorkingToEEPROM();
    }
  }

  //fToA(val, fBuf, sizeof(fBuf) - 1);
  snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %u"), PSTR("New "), val);
  if (m_printEnabled) {Serial.println(logBuf);}
  
  /////////////testing/////////////////
  //stats.incSizeStat(2345);
  /////////////////////////////////////
}

//////////////////////////////////////////////////
// setPanelLength()
// Sets the current panel length. This is a float.
//////////////////////////////////////////////////

void consoleClass::setPanelLength(void *row, char *buf)
{
  char* tok;
  char *strP = buf;
  uint16_t val;
  char logBuf[128];
  char fBuf[32];

  val = configGetPanelLength();                                // Current panel length

  if (buf != NULL)
  {
    //logger.log(F("[%s]"), buf);                               // Log the command request
    tok = strsep(&strP, " \r");

    //fToA(val, fBuf, sizeof(fBuf) - 1);
    //logger.log(F("%S: %s"), PSTR("Curr "), fBuf);
    snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %u"), PSTR("Curr "), val);
    if (m_printEnabled) {Serial.println(logBuf);}

    if (tok != NULL)
    {
      val = atof(tok);
      configSetPanelLength(val);
      configCopyWorkingToEEPROM();
    }
  }

  //fToA(val, fBuf, sizeof(fBuf) - 1);
  //logger.log(F("%S: %s"), PSTR("New "), fBuf);

  snprintf_P(logBuf, sizeof(logBuf), PSTR("%S: %u"), PSTR("New "), val);
  if (m_printEnabled) {Serial.println(logBuf);}

  /////////////testing/////////////////
  //stats.incSizeStat(1111);
  /////////////////////////////////////
}

/*
//////////////////////////////////////////////////
// consoleClearEEPROM()
// Set the whole of the EEPROM to zeros.
//////////////////////////////////////////////////

void consoleClearEEPROM(void *row, char *buf)
{
  Serial.println(F("Clearing"));

  for (int i = 0 ; i < EEPROM.length() ; i++)
  {
    EEPROM.write(i, 0);
  }

  Serial.println(F("Done"));
}
*/

/*
//////////////////////////////////////////////////
// consoleDoReset
// Enable the watchdog timer and allow it to time out and reset.
//////////////////////////////////////////////////

void consoleDoReset(void *row, char *buf)
{
  Serial.println(F("Resetting"));
  wdt_enable(WDTO_15MS);

  while(1);                                                          // Wait to reset
}
*/

/*
//////////////////////////////////////////////////
// consoleDoFactoryConfig
// Copy factory config to eeprom and working config.
//////////////////////////////////////////////////

void consoleDoFactoryConfig(void *row, char *buf)
{
    configCopyDefaultToEEPROM();                              // If not write default
    configCopyEEPROMToWorking();                              // and read to working
}
*/




//////////////////////////////////////////////////
// prompt
// Outputs the prompt string
//////////////////////////////////////////////////

void consoleClass::prompt()
{
  char buf[8] = {0};

  strncpy_P(buf, promptStr,  sizeof(buf) - 1);
  if (m_printEnabled) {Serial.print(buf);}
}


//////////////////////////////////////////////////
// getAppVersion
// Get the app version.
//////////////////////////////////////////////////

void consoleClass::getAppVersion(void *row, char* buf)
{
  char outBuf[128];

  snprintf_P(outBuf, sizeof(outBuf), PSTR("%S Version %S"), appName, PSTR(APP_VERSION));
  if (m_printEnabled) {Serial.println(outBuf);}
}

/*
//////////////////////////////////////////////////
// consoleRequestUpload
// Request an upload of any votes in the repository.
//////////////////////////////////////////////////

void consoleRequestUpload(void *row, char* buf)
{
  logger.log(F("consoleRequestUpload"));
  upload_job.reqUpload();                                     // Start vote upload
}
*/

/*
//////////////////////////////////////////////////
// consoleRequestConfigFile
// Request the config file from the server.
//////////////////////////////////////////////////

void consoleRequestConfigFile(void *row, char* buf)
{
  logger.log(F("consoleRequestConfigFile"));
  configReqDownload();                                      // Start config file download
}
*/

//////////////////////////////////////////////////
// consoleConfig
// Print the config data
//////////////////////////////////////////////////

void consoleClass::consoleConfig(void *row, char* buf)
{
  char outBuf[128];                     // Send buffer

  //configDumpEEPROMStruct();             // Dump the raw EEPROM data
  //logger.log(F(""));
  configDumpWorkingStruct();            // Dump the working values
}



/*
//////////////////////////////////////////////////
// consoleLog
// Turns logging on or off
//    log [on|off [save]]
//      on      - Turn logging on
//      off     - Turn logging off
//      save    - Save to EEPROM
//      default - Turn logging on
// If the 'save' arg is present then the working config
// is written to EEPROM. Otherwise the change will not persist
// and not survive a reset.
//////////////////////////////////////////////////

void consoleLog(void *row, char *buf)
{
  char* tok;
  char *strP = buf;

  if (buf != NULL)
  {
    logger.log(F("[%s]"), buf);                               // Log the command request
    tok = strsep(&strP, " \r");

    if (tok != NULL)
    {
      if (strcmp_P(tok, PSTR("off")) == 0)
      {
        Serial.println(F("Logging OFF"));
        configEnableLogging(false);
      }
      else
      {
        Serial.println(F("Logging ON"));
        configEnableLogging(true);
      }

      tok = strsep(&strP, " \r");
      if (tok != NULL)
      {
        if (strcmp_P(tok, PSTR("save")) == 0)
        {
          Serial.println(F("Saving to EEPROM"));
          configCopyWorkingToEEPROM();                        // Save the whole config to EEPROM
        }
      }
    }
  }
}
*/


/*
//////////////////////////////////////////////////
// consoleWriteWorkingConfigToEEPROM
// Restores the state of logging flag before initiating a
// write to EEPROM.
//////////////////////////////////////////////////

void consoleWriteWorkingConfigToEEPROM()
{
  bool isLogEnabledNow = configIsLogEnabled();                // Save current state of the log flag

  configEnableLogging(consoleBuf.savedLogFlag);               // Restore original value of flag
  configCopyWorkingToEEPROM();                                // Save the whole config to EEPROM
  configEnableLogging(isLogEnabledNow);                       // Restore current state of the flag
}
*/




//////////////////////////////////////////////////
// setBaseURL
// Sets the base url
//////////////////////////////////////////////////

void consoleClass::setBaseURL(void *row, char *buf)
{
  char* tok;
  char *strP = buf;
  char logBuf[128];

  if (buf != NULL)
  {
    //logger.log(F("[%s]"), buf);                               // Log the command request
    tok = strsep(&strP, " \r");
    snprintf_P(logBuf,  sizeof(logBuf), PSTR("Curr: %s"), configGetWorkingBaseURL());
    logger.log(logBuf);

    if (tok != NULL)
    {
      configSetWorkingBaseURL(buf);
      configCopyWorkingToEEPROM();
    }
  }

  snprintf_P(logBuf,  sizeof(logBuf), PSTR("New: %s"), configGetWorkingBaseURL());
  logger.log(logBuf);
}




//////////////////////////////////////////////////
// doHelp
// Lists the commands in the parser.
//    list [all]  - 'all' Includes hidden commands
//////////////////////////////////////////////////

void consoleClass::doHelp(void *row, char *buf)
{
  struct commandDefTable_t cdtS;                       // Local copy of table row
  int i;
  char outBuf[128];
  size_t outBufSize = sizeof(outBuf);
  bool allFlag = false;
  char* tok;
  char *strP = buf;

  if (buf != NULL)
  {
    //logger.log(F("[%s]"), buf);                               // Log the command request
    tok = strsep(&strP, " \r");                               // Look for arg
    if (strcmp_P(tok, PSTR("all")) == 0)                      // 'all' - Display hidden commands too
    {
      allFlag = true;
    }
  }

  i = 0;                                                      // Row index

  do
  {                                                           // Copy row into the local buffer
    memcpy_P(&cdtS, &commandDefTable[i++], sizeof(commandDefTable_t));

    if (cdtS.command == NULL) break;                          // End of table

    if (allFlag || (cdtS.visibilityMode == CON_SHOW))         // Decide whether to list or not
    {
      snprintf_P(outBuf, outBufSize, PSTR("%-16S%S"), cdtS.command, cdtS.description);
      if (m_printEnabled) {Serial.println(outBuf);}
    }
  } while(1);
}

//////////////////////////////////////////////////
// isLoggedIn
// Returns true if user logged in. 
//////////////////////////////////////////////////
bool consoleClass::isLoggedIn()
{
  return (now() - m_loginStart) < 600;                        // Timeout after 10 minutes
}

//////////////////////////////////////////////////
// logMeIn
// Sets the login start time to now.. 
//////////////////////////////////////////////////
void consoleClass::logMeIn()
{
  m_loginStart = now();
}

//////////////////////////////////////////////////
// doCommands
// Parse and execute an input string
// Finds the comamnd in the consoleCommandDefTable and calls the function
// in the row found. The remainder of the input string after the command
// is sent to the function.
//////////////////////////////////////////////////
/*
void consoleClass::doCommands(char *buf)
{
  struct commandDefTable_t cdtS;                       // Local copy of table row
  int i;
  size_t commandSize;
  char* tok;
  char* strP = buf;
  char outBuf[64];
  size_t outBufSize = sizeof(outBuf);

  logger.log(F("doCommands: %s"), buf);
  tok = strsep(&strP, " \r");                                 // Split tokens by space char
  if (tok != NULL)                                            // Scan the table for a match
  {                                                           // to the entered command
    commandSize = strlen(tok);
    i = 0;                                                    // Row index

    do
    {                                                         // Copy row into the local buffer
      memcpy_P(&cdtS, &commandDefTable[i++], sizeof(commandDefTable_t));

      if (cdtS.command == NULL) break;                        // End of table

      if(strlen_P(cdtS.command) == commandSize)               // Compare the exact command length
      {
        if (strcmp_P(tok, cdtS.command) == 0)
        {
          cdtS.cmdFunc(&cdtS, strsep(&strP, "\r"));           // Call the table function
          break;
        }
      }
    } while(1);
  }
}
*/

void consoleClass::doCommands(char *buf, bool force)
{
  struct commandDefTable_t cdtS;                       // Local copy of table row
  int i;
  size_t commandSize;
  char* tok;
  char* strP = buf;
  char outBuf[64];
  size_t outBufSize = sizeof(outBuf);

  if (isLoggedIn())
  {
    logger.log(F("doCommands: %s"), buf);
  }
  tok = strsep(&strP, " \r");                                 // Split tokens by space char
  if (tok != NULL)                                            // Scan the table for a match
  {                                                           // to the entered command
    commandSize = strlen(tok);
    i = 0;                                                    // Row index

    do
    {
      if ((i != 0) && !force && !isLoggedIn())                // Only allow access if it is to the 1st
      {                                                       // Row in the table, or it is forced,
        if (m_printEnabled) {Serial.println(F("Not logged in"));}
        break;                                                // or we are logged in.
      }
      
      if (isLoggedIn())                                       // If logged in then re-login for more time
      {
        logMeIn();                                            // Refresh the login
      }
                                                              // Copy row into the local buffer
      memcpy_P(&cdtS, &commandDefTable[i++], sizeof(commandDefTable_t));

      if (cdtS.command == NULL) break;                        // End of table

      if(strlen_P(cdtS.command) == commandSize)               // Compare the exact command length
      {
        if (strcmp_P(tok, cdtS.command) == 0)
        {
          cdtS.cmdFunc(&cdtS, strsep(&strP, "\r"));           // Call the table function
          break;
        }
      }
    } while(1);
  }
}



//////////////////////////////////////////////////
// interpret
// Wait for a \r and then execute the command.
//////////////////////////////////////////////////

void consoleClass::interpret(char *buf, int nbytes)
{
  char c;
  int i;

  for (i = 0; i < nbytes; i++)                                // Process all characters presented
  {
    c = buf[i];
    if ((c == '\b') || (c == 0x7f))                           // Backspace or DEL
    {
      if (consoleBuf.bufInd > 0)
      {
        if (m_printEnabled) {Serial.print(F("\b \b"));}
        consoleBuf.bufInd--;
      }
    }
    else
    {
      if ((c != '\n') && (c != '\r'))                         // If not \r or \n then save the char
      {
        if (isLoggedIn())
        {
          if (m_printEnabled) {Serial.print(c);}              // Echo the char
        }
        consoleBuf.buf[consoleBuf.bufInd++] = c;              // Add char to the buffer
        if (consoleBuf.bufInd >= CON_BUFSIZE - 2)             // Overflow?
        {
          consoleBuf.bufInd = 0;
        }
      }
    }

    if (c == '\r')                                            // If \r then we have a command
    {
      consoleBuf.msgAvailable = true;
    }

    if (consoleBuf.msgAvailable == true)
    {
      if (m_printEnabled) {Serial.println("");}               // New line
      consoleBuf.buf[consoleBuf.bufInd++] = 0;                // Terminate the command buf
      doCommands(consoleBuf.buf, false);                      // Interpret the command
      prompt();                                               // Output the prompt
      consoleBuf.bufInd = 0;                                  // Reset the buffer
      consoleBuf.msgAvailable = false;
    }
  }
}


//////////////////////////////////////////////////
// manageCommands
// Non blocking command UI. If there are characters at the serial
// port then they are consumed. If a full command is received terminated
// by a '\r' then execute a command from the table if a match is found.
// If no characters are waiting then the routine returns.
//////////////////////////////////////////////////

void consoleClass::manageCommands(void)
{
  char buf[CON_BUFSIZE];                                      // buffer for client data
  int i;
  char c;

  i = 0;
  while (Serial.available() > 0)                              // While there are characters waiting
  {
    c = Serial.read();                                        // Read them
    buf[i++] = c;
    if (i >= CON_BUFSIZE)
    {
      break;
    }
  }

  interpret(buf, i);                                          // And interpret any commands
}

/*
//////////////////////////////////////////////////
// consoleGetQuotedString
// Returns the next quoted or unquoted token from a string pointed
// to by the pointer pointed to by strP.
// Returns a ptr to the token and *strP is set to point to the char
// after the token.
//////////////////////////////////////////////////

char* consoleGetQuotedString(char **strP)
{
  char* delim;
  char* ret = NULL;

  if (*strP != NULL)
  {
    if (*strP[0] == ' ')                                    // Skip over any space
    {
      (*strP)++;
    }

    if (*strP[0] == '\"')                                   // If it's quoted
    {
      delim = "\"";                                         // Set the end quote delimeter
      (*strP)++;                                            // and step past the opening quote
    }
    else delim = " ";                                       // Not quoted so delimit by space

    ret = strsep(strP, delim);                              // Return ptr to token & advance *strP

    logger.log(F("consoleGetQuotedString: delimiter [%s] ret [%s] [%s]"), delim, ret, *strP);
  }

  return ret;
}
*/


