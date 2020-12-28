//////////////////////////////////////////////////
// enc28j60_ethernet.h
// Ethernet servicess
//////////////////////////////////////////////////

#ifndef ENC28J60ETHERNET_H
#define ENC28J60ETHERNET_H

#include "beading.h"

// Ethernet interface mac address, must be unique on the LAN
//static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
#define ENC28_MAX_HOST_SIZE                   32
#define ENC28_MAX_APP_PATH_SIZE               32
#define ENC28_SEND_BUFFER_SIZE                512
#define ENC28_RX_LINE_BUFFER_SIZE             64

class enc28j60ethernet
{
  public:

  private:
    //#define STATS_APP_PATH                  "/mbc/api/stats.php"
    
    static const char webHost[];
    static const char webAppStatsPath[];
    static const char webAppConfigPutPath[];
    static const char webAppBeadPutPath[];
    static const char webAppGetServerCmdsPath[];
    
    char rxLineBuf[ENC28_RX_LINE_BUFFER_SIZE + 1];                 // Line buffer for incoming chars
    int16_t lineBufPos;                                            // Position in line buffer
     
    char altWebHost[ENC28_MAX_HOST_SIZE];
    //const char* getReplyFromServer (byte session);                 // Get reply 
    int loadStatisics (char* sendBuf);                             // Load stats to buffer
    int loadConfig (char* sendBuf, int16_t chunk = 0);             // Load config to buffer
    char* loadPostPrelim(char* sendBuf, uint16_t* pos);            // Preliminary string in a post payload
    void scheduler();                                              // Trigger operations at regular intervals 
    
    bool m_enabled;                                                // true if enabled otherwise not.
    bool m_expectingResponse;                                      // True if a response pending
    char m_currentFileName[LG_MAX_fILENAME_LEN];                   // Current filename for uploads etc.
    
    enum uploadManagerStates_t
    {
      EM_INVALID = -1,
      EM_IDLE = 0,
      //EM_UPLOAD_STATS,
      EM_UPLOAD_CONFIG,
      EM_UPLOAD_BEAD,
      EM_REQUEST_COMMANDS,
      EM_DONE,
      EM_ERROR,
    };
    
    enum uploadStatsStates_t
    {
      US_INVALID = -1,
      US_IDLE = 0,
      US_RESOLVE_DNS,
      US_UPLOAD,
      US_WAIT_REPLY,
      US_ERROR
    };

    enum uploadConfigStates_t
    {
      //UC_INVALID = -1,
      UC_IDLE = 0,
      UC_RESOLVE_DNS,
      UC_UPLOAD,
      UC_WAIT_REPLY,
      UC_CHUNK_DWELL,
      UC_DONE,
      UC_CHUNK_ERROR,
      UC_ERROR
    };

    enum uploadBeadStates_t
    {
      UB_IDLE = 0,
      UB_GET_FILE_TO_SEND,
      UB_RESOLVE_DNS,
      UB_PREP_FILE_TO_SEND,
      UB_UPLOAD,
      UB_WAIT_REPLY,
      UB_DONE,
      UB_ERROR
    };
    
    enum getServerCmdsStates_t
    {
      GSC_IDLE = 0,
      GSC_RESOLVE_DNS,
      GSC_SEND_REQUEST,
      GSC_WAIT_REPLY,
      GSC_DONE,
      GSC_ERROR
    };
    
    static byte csPin;
    //static byte session;
    static char m_sendBuffer[];                                                 // String for sending
    static const char m_reqHeader[];                                            // Header string
    static bool m_responseReceived;
    static bool m_dirtyConfig;                                                  // Config has been changed

    //bool reqUploadStats;                                                        // true to request stats upload
    bool reqUploadConfig;                                                       // true to request config upload
    bool reqUploadBead;                                                         // true to request bead upload
    bool reqGetSvrCmds;                                                         // Request commands from server
    char* ipToStr (const uint8_t *ip, char *buf); 
    int uploadStatistics (bool begin = false);                                  // Post stats to server 
    int uploadConfig (bool begin = false);                                      // Post config to server
    int uploadBead (bool begin = false);                                        // Upload bead record 
    char* loadSvrCmdReq(char* sendBuf);                                         // Get the server commands payload
    int getServerCommands (bool begin = false);                                 // Request commands from the server 
    static void httpCallback (byte status, word off, word len);                 // Callback for post
    static void httpSvrCmdsCallback (byte status, word off, word len);          // callback to process commands 
    bool lookUpDNS();                                                           // Resolve DNS
    char* loadBeadData(beadingClass::BATCH* batch, char* sendBuf);              // Get JSON for bead record
    char* getNextUploadfile (char* ext, char* buf, size_t bufSize);             // Get file from upload dir 
    

  public:
    void begin ();                                                              // Startup 
    int manageEthernet ();                                                      // Call frequently
    //void requestStatisticsUpload(char* host = NULL, char* appPath = NULL);      // Request upload of stats 
    void requestConfigUpload(char* host = NULL, char* appPath = NULL);          // Request upload of config 
    void requestBeadUpload(char* host = NULL, char* appPath = NULL);            // Request upload of bead
    void requestGetSvrCmds(char* host = NULL, char* appPath = NULL);            // Request commands from server 
    void dumpEthernetParams ();                                                 // Dump ip addresses to log 
    void enableEManager (bool status);                                          // Enable or disable manager 
  
};

extern enc28j60ethernet iServ;

#endif
