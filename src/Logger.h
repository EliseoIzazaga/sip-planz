//*****************************************************
//  Logger.h
//  Provides utilities for logging to serial port and SD card.
//*****************************************************
#ifndef LOGGER_H
#define LOGGER_H

#include <SPI.h>
#include <SD.h>

// Note that the directory structure is <= 2 deep
#define LG_BUFFER_SIZE          300
#define LG_MAX_fILENAME_LEN     13                        // Max filename length 8.3 format
#define LG_MAX_DIRNAME_LEN      13                        // Max dirname length 8.3 format
#define LG_MAX_PATHNAME_LEN     LG_MAX_DIRNAME_LEN + LG_MAX_fILENAME_LEN    // Max file path name

class Logger 
{
  public:
    Logger();
    void begin();

    char m_buffer[LG_BUFFER_SIZE];
    static const char m_logErrorStr[];                    // String to denote error
    
    void log(const char * fmt, ...);                      // Log printf style
    void log(const __FlashStringHelper * fmt, ...);       // Log printf style
    void logBufLn(char * buf);                            // Log with newline
    void logBuf(char * buf);                              // Log without newline
    void timeStamp();                                     // Add a timestamp
    void print(char * buf);                               // Print to the log and console
    void sdDir();                                         // Print the SD card directory tree
    void sdFileDump(char* file);                          // Print a file to console
    void sdFileTail(char* file);                          // Print last part of file
    void sdMkDir(char* dirPath);                          // Make a directory
    void sdRmDir(char* dirPath);                          // remove a directory that is empty
    void sdRm(char* filePath);                            // Remove a file
    //void sdInfo(char* file);                              // Information about the SD card
    void sdClearUploadDir();                              // Empty the upload director


  private:
    void printDirectory(File dir, int numTabs);           // recursively print directory tree
    void sdDbPrint(char * buf);                           // Print to SD card log file
    char* makeLogIndexPath(char* buf, size_t bufSize);    // Get path to log index file
    void writeLogIndexFile(uint16_t fileIndex);           // Write a new log index
    uint16_t getLogIndexFromFile();                       // Get the index number from file
    void makeLogFilePath(uint16_t newIndex);              // Make the log file path
    void checkLogFileSize();                              // Make a new log file if required

    static char m_sdDbLogFileName[LG_MAX_fILENAME_LEN];   // Debug log filename in 8.3 format
    static const char m_sdDbDirName[LG_MAX_DIRNAME_LEN];  // Debug log directory
    static char m_logPath[LG_MAX_PATHNAME_LEN];           // Full path to current log file
    static const char m_logIndexName[LG_MAX_PATHNAME_LEN];// Full path to current log file

    static const char m_sdUploadDirName[LG_MAX_DIRNAME_LEN];// Server upload directory
};

extern Logger logger;

#endif
