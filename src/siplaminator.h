//////////////////////////////////////////////////
// siplaminator.h
// All important configuration definitions etc.s
//////////////////////////////////////////////////
#ifndef SIPLAMINATOR_H
#define SIPLAMINATOR_H

#define APP_VERSION                     "1.1"
#define APP_NAME                        "SIP Laminator"

#define DB_LOG_DIR                      "logs"                      // Directory for log files
#define DB_LOG_FILENAME                 "0000.txt"                  // Default log file name
#define DB_LOG_INDEX_FILENAME           "index.txt"                 // Name of the file containing current log file name
#define DB_LOG_MAX_FILE_SIZE            2000000L                    // Max size of any one log file
#define DB_LOG_MAX_FILES                20                          // Max number of log files

//#define BEADER_BATCH_DIR        ""                          // The batch file goes in the root
#define BEADER_BATCH_FILENAME           "batch.dat"                 // The batch file name
//#define BEADER_BEADS_DIR        "beads"                     // The bead files goes in the root

#define SERVER_UPLOAD_DIR               "upload"                    // Files for server upload
#define BEAD_FILE_EXT                   "bed"                       // Extension for bead files
#define CONFIG_FILE_EXT                 "cnf"                       // Extension for config files
#define TEXT_FILE_EXT                   "txt"                       // Extension for text files
#define DATA_FILE_EXT                   "dat"                       // Extension for data files

#define API_WEBHOST                     "castlenetware.com"         // Website to upload to
#define API_URL_STATS_PART              "/mbc/api/stats.php"        // Path on website for stats upload
//#define API_URL_CONFIGPUT_PART          "/mbc/api/configput.php"    // Path on website for config upload
#define API_URL_CONFIGPUT_PART          "/mbc/index.php/api/view/upconfig"// Path on website for config upload
#define API_URL_BEADPUT_PART            "/mbc/index.php/api/view/upbead"  // Path on website for config upload
#define API_URL_SERVER_CMD_REQ_PART     "/mbc/index.php/api/view/getcmds"  // Path on website to request command download

#define API_CMD_POLL_INTERVAL           3600                        // Poll server for commands interval in seconds

extern const char appName[];                                // Name of the App

#endif
