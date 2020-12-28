//////////////////////////////////////////////////
// enc28j60_ethernet.c
// Ethernet servicess
//////////////////////////////////////////////////

#include <EtherCard.h>
#include <DS3232RTC.h> 
#include <Time.h> 
#include "siplaminator.h"
#include "config.h"
#include "logger.h"
#include "beading.h"
#include "enc28j60_ethernet.h"
#include "statistics.h"
#include "console.h"
#include "touchscreen.h"
#include "gui.h"

byte Ethernet::buffer[700];
char enc28j60ethernet::m_sendBuffer[ENC28_SEND_BUFFER_SIZE + 1];
const char enc28j60ethernet::m_reqHeader[] PROGMEM = "Authorization: Basic c2lwOmVzZTIxcGxpZXM=";

//BufferFiller bfill;                                                                
//Stash stash;


enc28j60ethernet iServ;                                               // The Ethernet services object
const char enc28j60ethernet::webHost[] PROGMEM                    = API_WEBHOST;
const char enc28j60ethernet::webAppStatsPath[] PROGMEM            = API_URL_STATS_PART;
const char enc28j60ethernet::webAppConfigPutPath[] PROGMEM        = API_URL_CONFIGPUT_PART;
const char enc28j60ethernet::webAppBeadPutPath[] PROGMEM          = API_URL_BEADPUT_PART;
const char enc28j60ethernet::webAppGetServerCmdsPath[] PROGMEM    = API_URL_SERVER_CMD_REQ_PART;
//static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
byte enc28j60ethernet::csPin = 53;
//byte enc28j60ethernet::session;
bool enc28j60ethernet::m_responseReceived = false;
bool enc28j60ethernet::m_dirtyConfig = false;                     // Config has not been changed

//////////////////////////////////////////////////
// begin()
// Call this from startup after logging is initialised.
//////////////////////////////////////////////////

void enc28j60ethernet::begin () 
{
  const char* funcIDStr = PSTR("enc28j60ethernet::begin: ");
  char logBuf[128];

  logger.log(F("%SMAC=[%s] pin=%d bufSize=%u"), funcIDStr, configGetMACAddressStr(logBuf, sizeof(logBuf)), csPin, sizeof(Ethernet::buffer));

  do
  {
    if (ether.begin(sizeof Ethernet::buffer, configGetMACAddress(), csPin) == 0)    // The system seems to be fussy about the MAC
    //if (ether.begin(sizeof Ethernet::buffer, mymac, csPin) == 0)
    {
      logger.log(F("%SFailed to access Ethernet controller"), funcIDStr);
      break;
    }

    //ether.persistTcpConnection(true);
    ether.persistTcpConnection(false);
     
    logger.log(F("%SDHCP Initialising"), funcIDStr);
      
    if(!ether.dhcpSetup())
    {
      logger.log(F("%SDHCP Failed"), funcIDStr);
      break;
    }
      
    dumpEthernetParams ();
    
    altWebHost[0] = '\0';
    
    lineBufPos = 0;                         // Rx line buffer position
    rxLineBuf[0] = '\0';                    // Rx line buffer
    m_expectingResponse = false;            // No response pending
    
    //reqUploadStats = false;
    reqUploadConfig = false;
    reqGetSvrCmds = false;
    
    enableEManager(true);                   // Enable the manager to run
    break;
  } while (1);

}

//////////////////////////////////////////////////
// enableEManager()
// Enable or disable the manager().
//////////////////////////////////////////////////
void enc28j60ethernet::enableEManager (bool status) 
{
  const char* funcIDStr = PSTR("enableEManager: ");
  
  logger.log(F("%S %d"), funcIDStr, status);
  m_enabled = status;
}

//////////////////////////////////////////////////
// dumpEthernetParams()
// Dump the addresses to the log.
//////////////////////////////////////////////////
void enc28j60ethernet::dumpEthernetParams () 
{
  char logBuf[128];
  
  logger.log(F("IP %s GW %s Mask %s DHCP %s DNS %s"),
              ipToStr(ether.myip,logBuf),
              ipToStr(ether.gwip, logBuf + 20),
              ipToStr(ether.netmask, logBuf + 40),
              ipToStr(ether.dhcpip, logBuf + 60),
              ipToStr(ether.dnsip, logBuf + 80));
}

//////////////////////////////////////////////////
// manageEthernet()
// Call from the main loop to manage the ethernet services.
// Returns 1 if busy 0 otherwise.
//////////////////////////////////////////////////
int enc28j60ethernet::manageEthernet () 
{
  const char* funcIDStr = PSTR("manageEthernet: ");
  word len = ether.packetReceive();                                 // Copy received data into buffer[]
  word pos = ether.packetLoop(len);                                 // Parse the received data
  static uploadManagerStates_t state = EM_IDLE;
  static uploadManagerStates_t lastState = EM_INVALID;
  int retVal = 1;

  switch(state)
  {
    case EM_IDLE:
      if (m_enabled)                                                // If the manager is allowed to run
      {
        scheduler();                                                // Scheduled operations
    
        if (reqUploadConfig)
        {
          reqUploadConfig = false;
          uploadConfig(true);
          state = EM_UPLOAD_CONFIG;
        }
        else if (reqUploadBead)
        {
          reqUploadBead = false;
          uploadBead(true);
          state = EM_UPLOAD_BEAD;
        }
        else if (reqGetSvrCmds)
        {
          reqGetSvrCmds = false;
          getServerCommands(true);
          state = EM_REQUEST_COMMANDS;
        }
        else
        {
          retVal = 0;                                              // Nothing to do so we are idle
        }
      }
      
      if ((state != EM_IDLE) || (ether.myip[3] == 0))
      {
        tsc.setIndicatorValue(tsc.IND_NETWORK_ACTIVITY, 1);
      }
      else
      {
        tsc.setIndicatorValue(tsc.IND_NETWORK_ACTIVITY, 0);
      }
      
      break;
      
    case EM_UPLOAD_CONFIG:
      state = (uploadManagerStates_t)gui.runFSM(state, EM_DONE, EM_ERROR, uploadConfig());
      break;
      
    case EM_UPLOAD_BEAD:
      state = (uploadManagerStates_t)gui.runFSM(state, EM_DONE, EM_ERROR, uploadBead());
      break;
      
    case EM_REQUEST_COMMANDS:
      state = (uploadManagerStates_t)gui.runFSM(state, EM_DONE, EM_ERROR, getServerCommands());
      if ((state == EM_DONE) && (m_dirtyConfig))
      {
        m_dirtyConfig = false;
        requestConfigUpload();                                                    // Upload new config
      }

      break;
      
    case EM_ERROR:
      logger.log(F("%SError"), funcIDStr);
      state = EM_IDLE;
      break;
    
    case EM_DONE:
      //logger.log(F("%SDone"), funcIDStr);
      state = EM_IDLE;
      break;  
      
    default:
      logger.log(F("%SStats Error"), funcIDStr);
      state = EM_IDLE;
      break;
  }
 
  if (state != lastState)
  {
    logger.log(F("%Sstate %d -> %d"), funcIDStr, lastState, state);
    lastState = state;
  }
  
  return retVal;
}

//////////////////////////////////////////////////
// scheduler()
// Schedules the various upload/download activities.
//////////////////////////////////////////////////
void enc28j60ethernet::scheduler() 
{
  uint32_t nowTime = now();
  static uint32_t lastBeadTime = nowTime;                  // Do not do a bead upload at startup
                                                           // Do an initial server command poll 5 minutes
  static uint32_t lastServerCommandTime = nowTime - API_CMD_POLL_INTERVAL + 300;                    
  
  if ((nowTime - lastBeadTime) >= 30)                      // Look in the uploads dir to see if there
  {                                                        // is anything to upload
    if (*getNextUploadfile (BEAD_FILE_EXT, m_currentFileName, LG_MAX_fILENAME_LEN))
    {
      requestBeadUpload();
    }
    
    lastBeadTime = nowTime;
  }

  if ((nowTime - lastServerCommandTime) >= API_CMD_POLL_INTERVAL) // Poll the server for commands
  {
    requestGetSvrCmds();
    lastServerCommandTime = nowTime;
  }
}

//////////////////////////////////////////////////
// httpCallback()
// This gets called multiple times during a transaction and the responseRecieved
// flag get set each time. This can be confusing because we don't know if there
// is more coming.
//////////////////////////////////////////////////
void enc28j60ethernet::httpCallback (byte status, word off, word len) 
{
  const char* funcIDStr = PSTR("httpCallback: ");
  uint16_t bytesLogged = 0;
  uint16_t offset = 0;
  char buf[8];
  char c;
  const char* reply = (char *)Ethernet::buffer + off;
  int i;

  logger.log(F("%SGot reply len %d off %d status %u"), funcIDStr, len, off, status);
  
  if (!iServ.m_expectingResponse)
  {
    logger.log(F("%SUnexpected Callback - Aborting"), funcIDStr);
    return;
  }
  
  while (bytesLogged < len)
  {
    c = *reply++;
    if ((c == '\n'))
    {
      iServ.rxLineBuf[iServ.lineBufPos] = '\0';
      iServ.lineBufPos = 0;
      logger.log(F("%S%s"), funcIDStr, iServ.rxLineBuf);
      
      if (strcmp_P(iServ.rxLineBuf, PSTR("SUCCESS")) == 0)
      {
        m_responseReceived = true;
      }
      else if (strcmp_P(iServ.rxLineBuf, PSTR("ERROR")) == 0)
      {
        logger.log(F("%SError received"), funcIDStr);
      }
    }
    else
    {
      if ((c != '\r'))
      {
        iServ.rxLineBuf[iServ.lineBufPos++] = c;
        if (iServ.lineBufPos >= ENC28_RX_LINE_BUFFER_SIZE - 2)
        {
          iServ.rxLineBuf[ENC28_RX_LINE_BUFFER_SIZE - 1] = '\0';
          logger.log(F("%S%s"), funcIDStr, iServ.rxLineBuf);
          iServ.lineBufPos = 0;
        }
      }
    }    
       
    bytesLogged++;
  }
    
}

//////////////////////////////////////////////////
// httpSvrCmdsCallback()
// Executes the commands coming from the server.
//////////////////////////////////////////////////
void enc28j60ethernet::httpSvrCmdsCallback (byte status, word off, word len) 
{
  const char* funcIDStr = PSTR("httpSvrCmdsCallback: ");
  uint16_t bytesLogged = 0;
  uint16_t offset = 0;
  char buf[8];
  char c;
  const char* reply = (char *)Ethernet::buffer + off;
  //int i;
  int numBlanks = 0;
  bool inBody = false;
  
  //logger.log(F("%SGot reply len %d off %d status %u"), funcIDStr, len, off, status);
  
  if (!iServ.m_expectingResponse)
  {
    logger.log(F("%SUnexpected - Aborting"), funcIDStr);
    return;
  }
  
  while (bytesLogged < len)
  {
    c = *reply++;
    if ((c == '\n'))
    {
      if (++numBlanks > 1)
      {
        inBody = true;
      }
      
      iServ.rxLineBuf[iServ.lineBufPos] = '\0';
      iServ.lineBufPos = 0;
      if ((iServ.rxLineBuf[0] != '\0') && inBody)
      {
        logger.log(F("%S%s"), funcIDStr, iServ.rxLineBuf);
      }
      
      if (strcmp_P(iServ.rxLineBuf, PSTR("SUCCESS")) == 0)
      {
        m_responseReceived = true;
      }
      else if (strcmp_P(iServ.rxLineBuf, PSTR("ERROR")) == 0)
      {
        logger.log(F("%SError received"), funcIDStr);
      }
      else          // Execute the command
      {
        if ((iServ.rxLineBuf[0] != '\0') && (inBody))
        {
          logger.log(F("%SExecuting [%s]"), funcIDStr, iServ.rxLineBuf);
          console.doCommands(iServ.rxLineBuf);
          m_dirtyConfig = true;
        }
      }
    }
    else
    {
      if ((c != '\r'))
      {
        numBlanks = 0;
        iServ.rxLineBuf[iServ.lineBufPos++] = c;
        if (iServ.lineBufPos >= ENC28_RX_LINE_BUFFER_SIZE - 2)
        {
          iServ.rxLineBuf[ENC28_RX_LINE_BUFFER_SIZE - 1] = '\0';
          logger.log(F("%S%s"), funcIDStr, iServ.rxLineBuf);
          iServ.lineBufPos = 0;
        }
      }
    }    
       
    bytesLogged++;
  }
    
}


//////////////////////////////////////////////////
// ipToStr()
// Convert an ip address in 4 byte form to ascii dotted.
//////////////////////////////////////////////////
char* enc28j60ethernet::ipToStr (const uint8_t *ip, char *buf) 
{
    sprintf_P(buf, PSTR("%d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
    return buf;
}

/*
//////////////////////////////////////////////////
// requestStatisticsUpload()
// Set the stats upload flag
// The appPath needs to be in Progmem for the Ethercard routines to work
//////////////////////////////////////////////////

void enc28j60ethernet::requestStatisticsUpload(char* host, char* appPath) 
{
  const char* funcIDStr = PSTR("requestStatisticsUpload: ");
  
  if (host)
  {
    strncpy(altWebHost, host, ENC28_MAX_HOST_SIZE);
  }
  else
  {
    strncpy(altWebHost, configGetWorkingBaseURL(), ENC28_MAX_HOST_SIZE);
  }

  logger.log(F("%SHost %S path %S"), funcIDStr, webHost, webAppStatsPath);  
  reqUploadStats = true;
}
*/

//////////////////////////////////////////////////
// requestConfigUpload()
// Set the config upload flag
//////////////////////////////////////////////////

void enc28j60ethernet::requestConfigUpload(char* host, char* appPath) 
{
  const char* funcIDStr = PSTR("requestConfigUpload: ");
  
  if (host)
  {
    strncpy(altWebHost, host, ENC28_MAX_HOST_SIZE);
  }
  else
  {
    strncpy(altWebHost, configGetWorkingBaseURL(), ENC28_MAX_HOST_SIZE);
  }

/* The appPath needs to be in Progmem for the Ethercard routines to work
 *  this next bit will have to wait until that problem is resolved.
  if (appPath)
  {
    strncpy(webAppPath, appPath, ENC28_MAX_APP_PATH_SIZE);
  }
  else
  {
    strncpy_P(webAppPath, PSTR(STATS_APP_PATH), ENC28_MAX_APP_PATH_SIZE);
  }
*/

  logger.log(F("%SHost %S"), funcIDStr, webHost);  
  reqUploadConfig = true;
}

//////////////////////////////////////////////////
// requestBeadUpload()
// Set the Bead upload flag
//////////////////////////////////////////////////

void enc28j60ethernet::requestBeadUpload(char* host, char* appPath) 
{
  const char* funcIDStr = PSTR("requestBeadUpload: ");
  
  if (host)
  {
    strncpy(altWebHost, host, ENC28_MAX_HOST_SIZE);
  }
  else
  {
    strncpy(altWebHost, configGetWorkingBaseURL(), ENC28_MAX_HOST_SIZE);
  }

  logger.log(F("%SHost %S"), funcIDStr, webHost);  
  reqUploadBead = true;
}

//////////////////////////////////////////////////
// requestGetSvrCmds()
// Set the get commands from server flag
//////////////////////////////////////////////////

void enc28j60ethernet::requestGetSvrCmds(char* host, char* appPath) 
{
  const char* funcIDStr = PSTR("requestGetSvrCmds: ");
  
  if (host)
  {
    strncpy(altWebHost, host, ENC28_MAX_HOST_SIZE);
  }
  else
  {
    strncpy(altWebHost, configGetWorkingBaseURL(), ENC28_MAX_HOST_SIZE);
  }

  //logger.log(F("%SHost %S"), funcIDStr, webHost);  
  reqGetSvrCmds = true;
}

/*
//////////////////////////////////////////////////
// loadStatisics()
// Loads the statistics into the buffer
// Returns string len.
//////////////////////////////////////////////////

int enc28j60ethernet::loadStatisics (char* sendBuf) 
{
  const char* funcIDStr = PSTR("loadStatisics: ");
  uint16_t sendBufferLen = 0;
  struct statsClass::statData_t* pStats;
  int i;
  
  sprintf_P(sendBuf,  PSTR("uid=%u&auth=12345&json={\"plen\":["), configGetUID());
  
  if (pStats = stats.readStatsFromSD())
  {
    logger.log(F("%SStats Version %u Sizes %u"), funcIDStr, pStats->version, pStats->numSizes);
    
    for(i = 0; i < pStats->numSizes; i++)
    {
      sendBufferLen = strlen(sendBuf);
      snprintf_P(sendBuf + sendBufferLen, 
                ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
                PSTR("{\"%u\":\"%u\"}%s"), pStats->sizeStats[i].size, pStats->sizeStats[i].count, ((i + 1) < pStats->numSizes) ? "," : "");
                
    }

    sendBufferLen = strlen(sendBuf);
    snprintf_P(sendBuf + sendBufferLen, ENC28_SEND_BUFFER_SIZE - sendBufferLen, PSTR("]}"));
    sendBufferLen = strlen(sendBuf);
  }

  sendBuf[ENC28_SEND_BUFFER_SIZE] = '\0';
  logger.log(F("%SPayload %u bytes"), funcIDStr, sendBufferLen);
  return sendBufferLen;
}
*/

//*****************************************************
// loadPostPrelim.
// Inserts the uid and authorisation and anything else that is common to a post.
//*****************************************************

char* enc28j60ethernet::loadPostPrelim(char* sendBuf, uint16_t* pos)
{
  sprintf_P(sendBuf,  PSTR("uid=%u&auth=12345"), configGetUID());
  *pos = strlen(sendBuf);
  return sendBuf;
}

//////////////////////////////////////////////////
// loadConfig()
// Loads the config data into the buffer
// Returns string len.
//////////////////////////////////////////////////
int enc28j60ethernet::loadConfig (char* sendBuf, int16_t chunk) 
{
  const char* funcIDStr = PSTR("loadConfig: ");
  uint16_t sendBufferLen = 0;
  byte* mac;
  struct valueSpinnerParams_t* spinP;
  int i;
  int retVal = 1;                             // Return 0 when no more config
  char buf[32];
  
  loadPostPrelim(sendBuf, &sendBufferLen);
  
  sprintf_P(sendBuf + sendBufferLen,  PSTR("&chunk=%d&json={\"config\":["), chunk);
  sendBufferLen = strlen(sendBuf);
  
  
  switch(chunk)
  {
    case 0:
      snprintf_P(sendBuf + sendBufferLen, 
                ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
                PSTR("{\"%S\":\"%u\"},"), consoleClass::setPanelLengthCmd, configGetPanelLength());          
      sendBufferLen = strlen(sendBuf);
      
      snprintf_P(sendBuf + sendBufferLen, 
                ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
                PSTR("{\"%S\":\"%u\"},"), consoleClass::setOffsetBottomCmd, configGetOffsetBottom());          
      sendBufferLen = strlen(sendBuf);
      
      snprintf_P(sendBuf + sendBufferLen, 
                ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
                PSTR("{\"%S\":\"%u\"},"), consoleClass::setOffsetTopCmd, configGetOffsetTop());          
      sendBufferLen = strlen(sendBuf);
            
      //snprintf_P(sendBuf + sendBufferLen, 
      //          ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
      //          PSTR("{\"%S\":\"%u\"},"), consoleClass::setMmPerPulseCmd, configGetMmPP());          
      fToA(configGetMmPP(), buf, sizeof(buf) - 1);
      snprintf_P(sendBuf + sendBufferLen, 
                ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
                PSTR("{\"%S\":\"%s\"},"), consoleClass::setMmPerPulseCmd, buf);          
      sendBufferLen = strlen(sendBuf);
      
      snprintf_P(sendBuf + sendBufferLen, 
                ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
                PSTR("{\"%S\":\"%lu\"},"), consoleClass::setPulsesHomeToBeadCmd, configGetPulsesHomeToBead());          
      sendBufferLen = strlen(sendBuf);
      
      snprintf_P(sendBuf + sendBufferLen, 
                ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
                PSTR("{\"%S\":\"%lu\"},"), consoleClass::setPulsesBeadToEndCmd, configGetPulsesBeadToEnd());          
      sendBufferLen = strlen(sendBuf);
      
      snprintf_P(sendBuf + sendBufferLen, 
                ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
                PSTR("{\"%S\":\"%lu\"}"), consoleClass::setPulsesHomeToWetCmd, configGetPulsesHomeToWet());          
      sendBufferLen = strlen(sendBuf);
      break;
      
    case 1:  
      // Get all the timers. To Do - Put this in a loop
      /*
      snprintf_P(sendBuf + sendBufferLen, 
                ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
                PSTR("{\"%S %02d\":\"%lu\"},"), consoleClass::setTimeOutCmd, 1, configGetTimeOut(0));          
      sendBufferLen = strlen(sendBuf);
        
      snprintf_P(sendBuf + sendBufferLen, 
                ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
                PSTR("{\"%S %02d\":\"%lu\"},"), consoleClass::setTimeOutCmd, 2, configGetTimeOut(1));          
      sendBufferLen = strlen(sendBuf);
        
      snprintf_P(sendBuf + sendBufferLen, 
                ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
                PSTR("{\"%S %02d\":\"%lu\"},"), consoleClass::setTimeOutCmd, 3, configGetTimeOut(2));          
      sendBufferLen = strlen(sendBuf);
        
      snprintf_P(sendBuf + sendBufferLen, 
                ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
                PSTR("{\"%S %02d\":\"%lu\"},"), consoleClass::setTimeOutCmd, 4, configGetTimeOut(3));          
      sendBufferLen = strlen(sendBuf);
        
      snprintf_P(sendBuf + sendBufferLen, 
                ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
                PSTR("{\"%S %02d\":\"%lu\"},"), consoleClass::setTimeOutCmd, 5, configGetTimeOut(4));          
      sendBufferLen = strlen(sendBuf);
        
      snprintf_P(sendBuf + sendBufferLen, 
                ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
                PSTR("{\"%S %02d\":\"%lu\"}"), consoleClass::setTimeOutCmd, 6, configGetTimeOut(5));          
      sendBufferLen = strlen(sendBuf);
      */
      
      for (i = 1; i <= CONFIG_MAX_TIMEOUTS; i++)
      {
        snprintf_P(sendBuf + sendBufferLen, 
                  ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
                  PSTR("{\"%S %02d\":\"%lu\"},"), consoleClass::setTimeOutCmd, i, configGetTimeOut(i)
                  );          
        sendBufferLen = strlen(sendBuf);
      }
      
      for (i = 1; i <= CONFIG_MAX_DELAYS; i++)
      {
        snprintf_P(sendBuf + sendBufferLen, 
                  ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
                  PSTR("{\"%S %02d\":\"%lu\"}%s"), consoleClass::setDelayCmd, i, configGetDelay(i),
                  (i == CONFIG_MAX_DELAYS) ? "" : ",");          
        sendBufferLen = strlen(sendBuf);
      }
            
      break;  
      
    case 2:
      snprintf_P(sendBuf + sendBufferLen, 
                ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
                PSTR("{\"%S\":\"%u\"},"), consoleClass::setIndicatorsEnabledCmd, configGetIndicatorsEnabled());          
      sendBufferLen = strlen(sendBuf);
      
      snprintf_P(sendBuf + sendBufferLen, 
                ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
                PSTR("{\"%S\":\"%u\"},"), consoleClass::setMetricImpCmd, configGetMetricImperial());          
      sendBufferLen = strlen(sendBuf);
      
      mac = configGetMACAddress();
      snprintf_P(sendBuf + sendBufferLen, 
                ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
                PSTR("{\"%S\":\"%02x %02x %02x %02x %02x %02x\"},"), consoleClass::setMACCmd, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);          
      sendBufferLen = strlen(sendBuf);
      
      spinP = configGetPanelLengthSpinnerParams();
      snprintf_P(sendBuf + sendBufferLen, 
                ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
                PSTR("{\"%S\":\"%u %u %u\"},"), consoleClass::setPanelLengthSpinnerCmd, spinP->minimum, spinP->maximum, spinP->increment);          
      sendBufferLen = strlen(sendBuf);
      
      spinP = configGetOffsetSpinnerParams();
      snprintf_P(sendBuf + sendBufferLen, 
                ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
                PSTR("{\"%S\":\"%u %u %u\"}"), consoleClass::setOffsetSpinnerCmd, spinP->minimum, spinP->maximum, spinP->increment);          
      sendBufferLen = strlen(sendBuf);
      break;
      
    default:
      retVal = 0;                             // Done
      break;
  }
  
  snprintf_P(sendBuf + sendBufferLen, ENC28_SEND_BUFFER_SIZE - sendBufferLen, PSTR("]}"));
  sendBufferLen = strlen(sendBuf);

  sendBuf[ENC28_SEND_BUFFER_SIZE] = '\0';
  logger.log(F("%SPayload %u bytes Chunk %d"), funcIDStr, sendBufferLen, chunk);
  return retVal;
}

//*****************************************************
// loadBeadData.
//*****************************************************
char* enc28j60ethernet::loadBeadData(beadingClass::BATCH* batch, char* sendBuf)
{
  const char* funcIDStr = PSTR("loadBeadData: ");
  uint16_t sendBufferLen = 0;
  int i;
  
  sprintf_P(sendBuf,  PSTR("uid=%u&auth=12345&json={\"bead\":["), configGetUID());
  sendBufferLen = strlen(sendBuf);
  
  snprintf_P(sendBuf + sendBufferLen, 
            ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
            PSTR("{\"%S\":\"%lu\","), PSTR("batchstart"), batch->startTime);          
  sendBufferLen = strlen(sendBuf);

  snprintf_P(sendBuf + sendBufferLen, 
            ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
            PSTR("\"%S\":\"%lu\","), PSTR("wlstart"), batch->workLifeStartTime);          
  sendBufferLen = strlen(sendBuf);
            
  snprintf_P(sendBuf + sendBufferLen, 
            ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
            PSTR("\"%S\":\"%u\","), PSTR("batchid"), batch->bead.batchID);          
  sendBufferLen = strlen(sendBuf);
              
  snprintf_P(sendBuf + sendBufferLen, 
            ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
            PSTR("\"%S\":\"%u\","), PSTR("beadid"), batch->bead.beadID);          
  sendBufferLen = strlen(sendBuf);

  snprintf_P(sendBuf + sendBufferLen, 
            ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
            PSTR("\"%S\":\"%d\","), PSTR("type"), batch->bead.recordType);          
  sendBufferLen = strlen(sendBuf);

  snprintf_P(sendBuf + sendBufferLen, 
            ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
            PSTR("\"%S\":\"%d\","), PSTR("status"), batch->bead.status);          
  sendBufferLen = strlen(sendBuf);

  snprintf_P(sendBuf + sendBufferLen, 
            ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
            PSTR("\"%S\":\"%u\","), PSTR("beadinbatch"), batch->bead.beadInBatch);          
  sendBufferLen = strlen(sendBuf);

  snprintf_P(sendBuf + sendBufferLen, 
            ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
            PSTR("\"%S\":\"%lu\","), PSTR("end"), batch->bead.completionTime);          
  sendBufferLen = strlen(sendBuf);

  snprintf_P(sendBuf + sendBufferLen, 
            ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
            PSTR("\"%S\":\"%u\","), PSTR("plen"), batch->bead.panelLength);          
  sendBufferLen = strlen(sendBuf);

  snprintf_P(sendBuf + sendBufferLen, 
            ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
            PSTR("\"%S\":\"%u\","), PSTR("topoff"), batch->bead.topOffset);          
  sendBufferLen = strlen(sendBuf);
                                                                                    // No comma on the last one
  snprintf_P(sendBuf + sendBufferLen, 
            ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
            PSTR("\"%S\":\"%u\","), PSTR("botoff"), batch->bead.bottomOffset);          
  sendBufferLen = strlen(sendBuf);

  snprintf_P(sendBuf + sendBufferLen, 
            ENC28_SEND_BUFFER_SIZE - sendBufferLen, 
            PSTR("\"%S\":\"%lu\"}"), PSTR("projid"), batch->bead.projectID);          
  sendBufferLen = strlen(sendBuf);
  
  snprintf_P(sendBuf + sendBufferLen, ENC28_SEND_BUFFER_SIZE - sendBufferLen, PSTR("]}"));
  sendBufferLen = strlen(sendBuf);  

  sendBuf[ENC28_SEND_BUFFER_SIZE] = '\0';
  logger.log(F("%SPayload %u bytes"), funcIDStr, sendBufferLen);
  
  return sendBuf;
}

//////////////////////////////////////////////////
// lookUpDNS()
// Resolves DNS
// Returns true for success or false
//////////////////////////////////////////////////

bool enc28j60ethernet::lookUpDNS () 
{
  const char* funcIDStr = PSTR("lookUpDNS: ");
  bool retVal = true;
  #define BUFSIZE 20
  char buf[BUFSIZE + 1];
  
  //logger.log(F("%S:Looking up %s"), funcIDStr,  altWebHost);    
  if (altWebHost[0] != '\0')                                                           // Is there a different host to look up?
  {
    if (isdigit(altWebHost[0]))
    {
      ether.parseIp(ether.hisip, altWebHost);
      ipToStr (ether.hisip, altWebHost); 
      //logger.log(F("%S:Parsed %s"), funcIDStr,  altWebHost);    
    }
    else
    {
      if (!ether.dnsLookup(altWebHost, true))
      {
        logger.log(F("%SDNS failed"), funcIDStr);
        retVal = false;
      }
      else
      {
        //logger.log(F("%S: SRV: %s"), funcIDStr,  ipToStr(ether.hisip, buf));
      }
    }
  }
  else
  {
    if (!ether.dnsLookup(API_WEBHOST, true))
    {
      logger.log(F("%SDNS failed"), funcIDStr);
      retVal = false;
    }
    else
    {
      //logger.log(F("%S: SRV: %s"), funcIDStr,  ipToStr(ether.hisip, buf));
    }
  }
  
  altWebHost[0] = '\0';                                                               // Reset the alternate host.
  return retVal;
}


//////////////////////////////////////////////////
// uploadConfig()
// Uploads selected config to server
// Returns 0 - complete, 1 - progressing, -1 - error
//////////////////////////////////////////////////
int enc28j60ethernet::uploadConfig (bool begin) 
{
  const char* funcIDStr = PSTR("uploadConfig: ");
  static uploadConfigStates_t state = UC_IDLE;
  static uploadConfigStates_t lastState = UC_IDLE;
  //#define BUFSIZE 128
  //char buf[BUFSIZE + 1];
  int retVal = 1;
  static uint32_t timerStart;
  int dataLen;
  static int16_t chunk;
  static int chunkTries;

  if (begin)
  {
    m_expectingResponse = false;
    state = UC_RESOLVE_DNS;
  }

  switch(state)
  {
    case UC_IDLE:
      break;
      
    case UC_RESOLVE_DNS:
      if(lookUpDNS())
      {
        chunk = 0;                              // First chunk of upload
        chunkTries = 0;
        state = UC_UPLOAD;
      }
      else
      {
        state = UC_ERROR;
      }
      break;
      
    case UC_UPLOAD:
      if (loadConfig(m_sendBuffer, chunk))
      {
        dataLen = strlen(m_sendBuffer);
        logger.log(F("%SSending: %u bytes"), funcIDStr, dataLen);
        //ether.httpPost(webAppConfigPutPath, webHost, NULL, m_sendBuffer, httpCallback);
        ether.httpPost(webAppConfigPutPath, webHost, m_reqHeader, m_sendBuffer, httpCallback);
        
        m_expectingResponse = true;
        logger.log(F("%SPost Path [%S] webHost [%S]"), funcIDStr, webAppConfigPutPath, webHost);
        m_responseReceived = false;
        timerStart = millis();
        state = UC_WAIT_REPLY;
      }
      else
      {
        logger.log(F("%S%d Chunks sent"), funcIDStr, chunk);
        state = UC_DONE;
      }
      break;

    case UC_WAIT_REPLY:
      if ((millis() - timerStart) > 10000L)
      {
        logger.log(F("%STO"), funcIDStr);
        state = UC_CHUNK_ERROR;
        break;
      }
    
      if (m_responseReceived)                                                               // Set by callback
      {
        logger.log(F("%SReply received"), funcIDStr);
        m_expectingResponse = false;
        timerStart = millis();
        chunk++;
        chunkTries = 0;
        state = UC_CHUNK_DWELL;
      }
      break;

    case UC_CHUNK_DWELL:
      if ((millis() - timerStart) > 2000L)
      {
        logger.log(F("%SChunk %d Dwelled out Try %d"), funcIDStr, chunk, chunkTries);
        state = UC_UPLOAD;
        break;
      }

      break;
            
    case UC_DONE:
      retVal = 0;
      logger.log(F("%SDone"), funcIDStr);
      state = UC_IDLE;
      break;
            
    case UC_CHUNK_ERROR:
      m_expectingResponse = false;
      if (++chunkTries < 5)
      {
        logger.log(F("%STry %u Error but carrying on"), funcIDStr, chunkTries);
        timerStart = millis();
        state = UC_CHUNK_DWELL;
      }
      else
      {
        logger.log(F("%SCatastrophic fail"), funcIDStr);
        state = UC_ERROR;
      }
      break;

    case UC_ERROR:
    default:
      m_expectingResponse = false;
      logger.log(F("%SError"), funcIDStr);
      timerStart = millis();
      state = UC_IDLE;
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


//////////////////////////////////////////////////
// getNextUploadfile()
// If there is a file in upload with the given extension then
// copy it into the buffer.
// Returns a ptr to the buffer. 
//////////////////////////////////////////////////
char* enc28j60ethernet::getNextUploadfile (char* ext, char* buf, size_t bufSize) 
{
  const char* funcIDStr = PSTR("getNextUploadfile: ");
  File uploadDir;
  File uploadFile;
  *buf = '\0';
  char extBuf[5];
  
  extBuf[0] = '.';
  extBuf[1] = toupper(ext[0]);
  extBuf[2] = toupper(ext[1]);
  extBuf[3] = toupper(ext[2]);
  extBuf[4] = '\0';
  
  if (uploadDir = SD.open(SERVER_UPLOAD_DIR))
  {
    while(uploadFile = uploadDir.openNextFile())
    {
      logger.log(F("%SFetching [%s]"), funcIDStr, uploadFile.name());
      if (strstr(uploadFile.name(), extBuf) != NULL)               // Test it for the right extension
      {
        strncpy(buf, uploadFile.name(), bufSize);
        buf[bufSize - 1] = '\0';
        uploadFile.close();
        logger.log(F("%SFound [%s]"), funcIDStr, buf);
        break;
      }
    }
    uploadDir.close();
  }
  
  return buf;
}

//////////////////////////////////////////////////
// uploadBead()
// If there is a '.bed' file in the upload directory then send
// it to the server and then delete it if upload successfull.
// Sends files one at a time 
// Returns 0 - complete, 1 - progressing, -1 - error
//////////////////////////////////////////////////
int enc28j60ethernet::uploadBead (bool begin) 
{
  const char* funcIDStr = PSTR("uploadBead: ");
  static uploadBeadStates_t state = UB_IDLE;
  static uploadBeadStates_t lastState = UB_IDLE;
  #define BUFSIZE 128
  char buf[BUFSIZE + 1];
  beadingClass::BATCH batch;
  int retVal = 1;
  static uint32_t timerStart;
  //const char* reply;
  //int dataLen;

  if (begin)
  {
    m_expectingResponse = false;
    state = UB_GET_FILE_TO_SEND;
  }

  switch(state)
  {
    case UB_IDLE:
      break;
      
    case UB_GET_FILE_TO_SEND:
      if (*getNextUploadfile(BEAD_FILE_EXT, m_currentFileName, LG_MAX_fILENAME_LEN))
      { state = UB_RESOLVE_DNS; }
      else
      { state = UB_DONE; }
      break;
    
    case UB_RESOLVE_DNS:
      if(lookUpDNS())
      { state = UB_PREP_FILE_TO_SEND; }
      else
      { state = UB_ERROR; }
      break;
      
    case UB_PREP_FILE_TO_SEND:
      state = UB_DONE;
      m_sendBuffer[0] = '\0';
      if (strlen(m_currentFileName))
      {
        if (beader.loadUploadBatchData(&batch, m_currentFileName) >= 0)     // Load data from file
        {                                                              
          loadBeadData(&batch, m_sendBuffer);                             // Load JSON into m_sendBuffer
          state = UB_UPLOAD;
        }
      }
      break;
      
    case UB_UPLOAD:
      logger.log(F("%S[%d] %s"), funcIDStr, strlen(m_sendBuffer), m_sendBuffer);
      //ether.httpPost(webAppBeadPutPath, webHost, NULL, m_sendBuffer, httpCallback);  // webAppBeadPutPath & webHost must be PROGMEM 
      ether.httpPost(webAppBeadPutPath, webHost, m_reqHeader, m_sendBuffer, httpCallback);  // webAppBeadPutPath & webHost must be PROGMEM 
      
      m_expectingResponse = true;
      logger.log(F("%SPost Path [%S] webHost [%S]"), funcIDStr, webAppBeadPutPath, webHost);
      m_responseReceived = false;
      timerStart = millis();
      state = UB_WAIT_REPLY;
      break;
      
    case UB_WAIT_REPLY:
      if ((millis() - timerStart) > 10000L)
      {
        logger.log(F("%STO"), funcIDStr);
        state = UB_ERROR;
        break;
      }
    
      if (m_responseReceived)                                                               // Set by callback
      {
        m_expectingResponse = false;
        logger.log(F("%SReply received Removing %s"), funcIDStr, m_currentFileName);
        beader.removeBeadUploadFile(m_currentFileName);
        state = UB_DONE;
      }
      break;
      
    case UB_DONE:
      logger.log(F("%SDone"), funcIDStr);
      retVal = 0;
      state = UB_IDLE;
      break;
      
    case UB_ERROR:
      logger.log(F("%SError"), funcIDStr);
      retVal = -1;
      state = UB_IDLE;
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

//*****************************************************
// loadSvrCmdReq.
//*****************************************************

char* enc28j60ethernet::loadSvrCmdReq(char* sendBuf)
{
  const char* funcIDStr = PSTR("loadSvrCmdReq: ");
  uint16_t sendBufferLen = 0;
  static uint16_t loadCount = 0;
  
  sprintf_P(sendBuf,  PSTR("uid=%u&timereq=%d"), configGetUID(), 
            (loadCount++ % 24) ? 0 : 1);            // Request time update every 24 hours
  sendBufferLen = strlen(sendBuf);
  
  sendBuf[ENC28_SEND_BUFFER_SIZE] = '\0';
  logger.log(F("%SPayload %u bytes [%s]"), funcIDStr, sendBufferLen, sendBuf);
  
  return sendBuf;
}

//////////////////////////////////////////////////
// getServerCommands()
// Requests the server to send any commands there might be waiting. 
// Returns 0 - complete, 1 - progressing, -1 - error
//////////////////////////////////////////////////
int enc28j60ethernet::getServerCommands (bool begin) 
{
  const char* funcIDStr = PSTR("getServerCommands: ");
  static getServerCmdsStates_t state = GSC_IDLE;
  static getServerCmdsStates_t lastState = GSC_IDLE;
  #define BUFSIZE 128
  char buf[BUFSIZE + 1];
  int retVal = 1;
  static uint32_t timerStart;

  if (begin)
  {
    m_expectingResponse = false;
    state = GSC_RESOLVE_DNS;
  }

  switch(state)
  {
    case GSC_IDLE:
      break;
      
    case GSC_RESOLVE_DNS:
      if(lookUpDNS())
      { state = GSC_SEND_REQUEST; }
      else
      { state = GSC_ERROR; }
      break;
      
    case GSC_SEND_REQUEST:
      loadSvrCmdReq(m_sendBuffer);
      //logger.log(F("%S[%d] %s"), funcIDStr, strlen(m_sendBuffer), m_sendBuffer);
      //ether.httpPost(webAppGetServerCmdsPath, webHost, NULL, m_sendBuffer, httpSvrCmdsCallback);
      ether.httpPost(webAppGetServerCmdsPath, webHost, m_reqHeader, m_sendBuffer, httpSvrCmdsCallback);
      m_expectingResponse = true;
      //logger.log(F("%SPost Path [%S] webHost [%S]"), funcIDStr, webAppGetServerCmdsPath, webHost);
      m_responseReceived = false;
      timerStart = millis();
      state = GSC_WAIT_REPLY;
      break;
      
    case GSC_WAIT_REPLY:
      if ((millis() - timerStart) > 10000L)
      {
        logger.log(F("%STO"), funcIDStr);
        state = GSC_ERROR;
        break;
      }
    
      if (m_responseReceived)                                                               // Set by callback
      {
        m_expectingResponse = false;
        //logger.log(F("%SReply received"), funcIDStr);
        state = GSC_DONE;
      }
      break;
      
    case GSC_DONE:
      //logger.log(F("%SDone"), funcIDStr);
      retVal = 0;
      state = GSC_IDLE;
      break;
      
    case GSC_ERROR:
      logger.log(F("%SError"), funcIDStr);
      retVal = -1;
      state = GSC_IDLE;
      break;
      
    default:
      break;
  }    
  
  if (state != lastState)
  {
    if ((lastState == 0) || (state == 0))                                                    // Called regularly so be
    {                                                                                        // economical with logging
      logger.log(F("%Sstate %d -> %d"), funcIDStr, lastState, state);
    }
    lastState = state;
  }
  
  return retVal;
}


