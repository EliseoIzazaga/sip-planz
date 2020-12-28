//*****************************************************
//  Logger.cpp
//  Provides utilities for logging to serial port and SD card.
//*****************************************************
#include <Arduino.h>
#include <Time.h>

#include "siplaminator.h"
#include "Logger.h"
#include "config.h"
#include "rtc_time.h"

//*****************************************************
// Initialise static member variables
//*****************************************************

char        Logger::m_sdDbLogFileName[]                             = DB_LOG_FILENAME;            // Name of the current log file
const char  Logger::m_sdDbDirName[] PROGMEM                         = DB_LOG_DIR;                 // Name of the log directory
char        Logger::m_logPath[LG_MAX_PATHNAME_LEN];                                               // Full path to current log file
const char  Logger::m_logIndexName[LG_MAX_PATHNAME_LEN] PROGMEM     = DB_LOG_INDEX_FILENAME;      // Name of the log index file
const char  Logger::m_logErrorStr[] PROGMEM                         = "***ERROR***";              // String to denote error
const char  Logger::m_sdUploadDirName[] PROGMEM                     = SERVER_UPLOAD_DIR;          // Name of the server upload directory

Logger::Logger() 
{
}

//*****************************************************
// begin()
//*****************************************************

void Logger::begin()
{
  char buf[LG_MAX_PATHNAME_LEN];
  uint16_t logIndex = 0;
  
  if (!SD.begin(4)) 
  {
    Serial.println("Logger::begin: SD init failed!");
    return;
  }

  snprintf_P(buf, LG_MAX_PATHNAME_LEN - 1, PSTR("%S"), m_sdDbDirName);                            // Create log directory
  sdMkDir(buf);
  
  snprintf_P(buf, LG_MAX_PATHNAME_LEN - 1, PSTR("%S"), m_sdUploadDirName);                        // Create server upload directory   
  sdMkDir(buf);

  //snprintf_P(buf, LG_MAX_PATHNAME_LEN - 1, PSTR("%S/%S"), m_sdDbDirName, m_logIndexName);
  makeLogIndexPath(buf, LG_MAX_PATHNAME_LEN - 1);
  Serial.print(F("Logger::begin: "));
  Serial.println(buf);
  
  if (!SD.exists(buf))
  {
    Serial.println(F("Index file not found"));
    writeLogIndexFile(0);
  }
  else
  {
    Serial.println(F("Index file exists"));
  }

  if (SD.exists(buf))
  {
    logIndex = getLogIndexFromFile();
    Serial.print(F("Log index: "));
    Serial.println(logIndex);
    makeLogFilePath(logIndex);
  }
  else
  {
    makeLogFilePath(0);                                 // Set the path to a default   
  }
  
  Serial.println(F("Logger Started"));
}

//*****************************************************
// sdClearUploadDir
// Deletes all the files in the upload directory
//*****************************************************

void Logger::sdClearUploadDir()
{
  const char* funcIDStr = PSTR("clearUploadDir: ");
  char buf[32];
  File upFileDir;
  File upFile;
  
  upFileDir = SD.open(SERVER_UPLOAD_DIR);
  
  if (upFileDir)
  {
    while(upFile = upFileDir.openNextFile())
    {      
      snprintf_P(buf, sizeof(buf) - 1, PSTR("%S/%s"), PSTR(SERVER_UPLOAD_DIR), upFile.name());
      log(F("%SRemoving %s"), funcIDStr, buf);
      SD.remove(buf);
    }
  }
}

//*****************************************************
// checkLogFileSize()
// Checks the size of the log file and makes a new one if required.
//*****************************************************
/*
void Logger::checkLogFileSize()
{
  const char* funcIDStr = PSTR("checkLogFileSize: ");
  uint32_t fileSize;
  uint16_t logIndex;
  char buffer[128];
  
  //Serial.println(funcIDStr);
  File dbFile = SD.open(m_logPath);
  if (dbFile)
  {
    fileSize = dbFile.size();
    dbFile.close();
    //snprintf_P(buffer, sizeof(buffer) - 1, PSTR("%S Log size %lu max %lu"), funcIDStr, fileSize, DB_LOG_MAX_FILE_SIZE);
    //Serial.println(buffer);
    
    if (fileSize > DB_LOG_MAX_FILE_SIZE)
    {
      logIndex = getLogIndexFromFile();
      logIndex++;
      snprintf_P(buffer, sizeof(buffer) - 1, PSTR("%SNew log index: %u"), funcIDStr, logIndex);
      Serial.println(buffer);
      writeLogIndexFile(logIndex);
      makeLogFilePath(logIndex);
    }
  }
}
*/
void Logger::checkLogFileSize()
{
  const char* funcIDStr = PSTR("checkLogFileSize: ");
  uint32_t fileSize;
  uint16_t logIndex;
  char buffer[128];
  
  File dbFile = SD.open(m_logPath);
  if (dbFile)
  {
    fileSize = dbFile.size();
    dbFile.close();
    
    if (fileSize > DB_LOG_MAX_FILE_SIZE)
    {
      logIndex = getLogIndexFromFile();
      logIndex++;
      snprintf_P(buffer, sizeof(buffer) - 1, PSTR("%SNew log index: %u"), funcIDStr, logIndex);
      Serial.println(buffer);
      writeLogIndexFile(logIndex);
      
      makeLogFilePath(logIndex - DB_LOG_MAX_FILES);
      Serial.print(F("Removing "));   
      Serial.println(m_logPath);   
      SD.remove(m_logPath);
      
      makeLogFilePath(logIndex);
    }
  }
}

//*****************************************************
// makeLogFilePath()
// Creates a new path to the log file
//*****************************************************

void Logger::makeLogFilePath(uint16_t newIndex)
{
  snprintf_P(m_logPath, LG_MAX_PATHNAME_LEN - 1, PSTR("%S/%04u.txt"), m_sdDbDirName, newIndex);
  Serial.print(F("makeLogFilePath: "));
  Serial.println(m_logPath);
}

//*****************************************************
// makeLogIndexPath()
// Returns the path to the log index file
//*****************************************************

char* Logger::makeLogIndexPath(char* buf, size_t bufSize)
{
  const char* funcIDStr = PSTR("Logger::makeLogIndexPath: ");

  snprintf_P(buf, bufSize - 1, PSTR("%S/%S"), m_sdDbDirName, m_logIndexName);
  return buf;
}

//*****************************************************
// getLogIndexFromFile()
// 
//*****************************************************
uint16_t Logger::getLogIndexFromFile()
{
  char buf[LG_MAX_PATHNAME_LEN];
  char numBuf[10];
  uint16_t retVal = 0;

  makeLogIndexPath(buf, LG_MAX_PATHNAME_LEN - 1);
  File dbFile = SD.open(buf);                           // Open for reading
  if (dbFile)
  {
    dbFile.read(numBuf, sizeof(numBuf) - 1);
    retVal = (uint16_t)strtoul(numBuf, NULL, 10);
    dbFile.close();
  }

  return retVal;
}

//*****************************************************
// writeLogIndexFile()
// 
//*****************************************************
void Logger::writeLogIndexFile(uint16_t fileIndex)
{
  char buf[LG_MAX_PATHNAME_LEN];
  char numBuf[10];

  makeLogIndexPath(buf, LG_MAX_PATHNAME_LEN - 1);
  SD.remove(buf);                                       // Remove it first
  File dbFile = SD.open(buf, FILE_WRITE);               // Open a new one
  if (dbFile)
  {
    sprintf_P(numBuf, PSTR("%u"), fileIndex); 
    Serial.print(F("writeLogIndexFile: LogIndex: "));
    Serial.println(numBuf);                             // No point trying to log here
    dbFile.print(numBuf);                               // Write the number into the file
    dbFile.close();
  }
}

//*****************************************************
// sdDbPrint() - Print to log on SD card
// 
//*****************************************************
void Logger::sdDbPrint(char * buf)
{
  int charsWritten;

  checkLogFileSize();

  if (strlen(buf) != 0)
  {
    File dbFile = SD.open(m_logPath, FILE_WRITE);
    if (dbFile)
    {
      if (!dbFile.print(buf))
      {
        Serial.print("\nError writing ");
        //Serial.println(m_sdDbLogFileName);
        //Serial.println(logPath);
        Serial.println(m_logPath);
      }
    
      dbFile.close();
    } else
      {
        Serial.print("\nError opening ");
        //Serial.println(logPath);
        Serial.println(m_logPath);
      }
  }
}


//*****************************************************
// print() - Print to log
// All printing goes through this function.
//*****************************************************

void Logger::print(char * buf)
{
//  if (!(configGetWorkingFlags() & CF_ENABLE_SERIAL_LOG)) return;
  Serial.print(buf);
  sdDbPrint(buf);
}

//*****************************************************
// timeStamp() - Print a time
//*****************************************************

void Logger::timeStamp()
{
  char tbuf[32];
  time_t t;

  //sprintf(tbuf, "%lu: ", millis());
  t = now();
  snprintf_P(tbuf, sizeof(tbuf), PSTR("%02d%02d%02d%02d%02d%02d: "), year(t) - 2000, month(t), day(t), hour(t), minute(t), second(t));
  print(tbuf);
}


//*****************************************************
// logBufLn() - Send text to log with newline.
//*****************************************************

void Logger::logBufLn(char * buf)
{
  timeStamp();
  print(buf);
  print("\r\n");
}

//*****************************************************
// logBuf() - Send text to log.
//*****************************************************

void Logger::logBuf(char * buf)
{
  timeStamp();
  print(buf);
}

//*****************************************************
// log() Print debug info.
//*****************************************************

void Logger::log(const char * fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vsnprintf(m_buffer, LG_BUFFER_SIZE, fmt, args);
  va_end(args);

  strcat_P(m_buffer, PSTR("\r\n"));
  logBuf(m_buffer);
}

void Logger::log(const __FlashStringHelper * fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vsnprintf_P(m_buffer, LG_BUFFER_SIZE, (const char *)fmt, args);
  va_end(args);
  strcat_P(m_buffer, PSTR("\r\n"));
  logBuf(m_buffer);
}

//*****************************************************
// printDirectory - Recursively print sd card directories starting at dir.
//*****************************************************

void Logger::printDirectory(File dir, int numTabs) 
{
  while (true) 
  {
    File entry =  dir.openNextFile();
    
    if (! entry) 
    {
      // no more files
      break;
    }
    
    for (uint8_t i = 0; i < numTabs; i++) 
    {
      Serial.print('\t');
    }
    
    Serial.print(entry.name());
    
    if (entry.isDirectory()) 
    {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else 
    {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    
    entry.close();
  }

}

//*****************************************************
// sdDir - Print the sd card directory
//*****************************************************

void Logger::sdDir()
{
  File root = SD.open("/");                              // SD card root  
  root.rewindDirectory();
  printDirectory(root, 0);                               // Print the whole of the card
 
  /*
  uint32_t fileSize;
  char buffer[128];
  
  File dbFile = SD.open(m_logPath);
  if (dbFile)
  {
    fileSize = dbFile.size();
    dbFile.close();
    snprintf_P(buffer, sizeof(buffer) - 1, PSTR("\n%s size %lu max %lu"), m_logPath, fileSize, DB_LOG_MAX_FILE_SIZE);
    Serial.println(buffer);
  }
  */
}

//*****************************************************
// sdMkDir - Make a directory if it doesn't already exist.
// Tests for the file 3 times before calling mkdir().
//*****************************************************

void Logger::sdMkDir(char* dirPath)
{
  bool found = false;
  int i;
  
  for (i = 0; i < 3; i++)
  {
    Serial.print(F("sdMkDir: "));
    Serial.print(dirPath);
    Serial.print(F(" "));
    Serial.println(i);
    
    if (SD.exists(dirPath))
    {
      found = true;
      break;
    }
    
    delay(1000);
  }
  
  if (!found) 
  {
    Serial.println(F("Dir not found"));
    SD.mkdir(dirPath);
  }
}

//*****************************************************
// sdRmDir - Remove a directory. Must be empty
//*****************************************************

void Logger::sdRmDir(char* dirPath)
{
  SD.rmdir(dirPath);
}

//*****************************************************
// sdRm - Remove a file.
//*****************************************************

void Logger::sdRm(char* filePath)
{
  SD.remove(filePath);
}

//*****************************************************
// sdFileDump - Dump a file to the console.
//*****************************************************

void Logger::sdFileDump(char* file)
{
  File dataFile = SD.open(file);
  if (dataFile) 
  {
    while (dataFile.available()) 
    {
      Serial.write(dataFile.read());
    }
  
    dataFile.close();
  } else 
  {
    Serial.print("Error opening ");
    Serial.println(file);
  }
    
  Serial.println("");
}

//*****************************************************
// sdFileTail - Dump the last few lines of a file.
//*****************************************************

void Logger::sdFileTail(char* file)
{
  File dataFile = SD.open(file);
  unsigned long pos;
  unsigned long offset = 10000;
  
  if (dataFile) 
  {
    pos = dataFile.size();                                      // Current end of file
    pos = (pos > offset) ? (pos - offset) : 0;    
    dataFile.seek(pos);

    while (dataFile.available()) 
    {
      Serial.write(dataFile.read());
    }
  
    dataFile.close();
  } else 
  {
    Serial.print("Error opening ");
    Serial.println(file);
  }

}


Logger logger;

