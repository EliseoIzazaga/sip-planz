//*****************************************************
//  statistics.cpp
//*****************************************************
/*
#include <Arduino.h>
#include <SD.h>

#include "siplaminator.h"
#include "statistics.h"
#include "Logger.h"

//const char statsClass::dataFileName[] PROGMEM = STATS_FILE_NAME;                    // Name of the stats file
char statsClass::dataFileName[] = STATS_FILE_NAME;                    // Name of the stats file


statsClass::statsClass()
{
}

//*****************************************************
// begin.
// Note: SD is initialised in Logger::begin()
//*****************************************************

void statsClass::begin()
{
  const char* funcIDStr = PSTR("statsClass::begin: ");
  //char fname[32];

  //strncpy_P(fname, dataFileName, sizeof(dataFileName));
  //logger.log(F("%S File %s"), funcIDStr, fname);
  logger.log(F("%S File %s"), funcIDStr, dataFileName);
  
  //if (!SD.exists(fname))
  if (!SD.exists(dataFileName))
  {
    logger.log(F("%S File [%s] not found creating new"), funcIDStr, dataFileName);
    clearStatData();                                                                // Clear it and start again
    saveStatData();
  }
  
}

//*****************************************************
// incSizeStat.
// Add one to the count of the appropriate size.
// Retrieves the data from SD, updates and rewrites it.
//*****************************************************

void statsClass::incSizeStat(uint16_t panelSize)
{
  const char* funcIDStr = PSTR("statsClass::incSizeStat: ");
  char buf[128];
  int i;
  
  if (loadStatData() >= 0)
  {
    for(i = 0; i < statData.numSizes; i++)                                          // Look for an existing size entry
    {
      if (statData.sizeStats[i].size == panelSize)
      {                                                                             // Found the size existing
        break;
      }
    }

    if (i < MAX_SIZE_STATS)                                                         // i now points to where the size count is
    {                                                                               
      statData.sizeStats[i].count++;                                                // Increment the size count
      if (i == statData.numSizes)                                                   // If this is a new one then
      {
        statData.sizeStats[i].size = panelSize;                                     // Add the new size
        statData.numSizes++;                                                        // Increase the number of sizes
      }
    
      saveStatData();                                                               // Save the new data structure
    }
    else                                                                            // No room in the sizes array
    {
      logger.log(F("%SToo many sizes"), funcIDStr);
    }
  }
  else
  {
    logger.log(F("%SERROR: Failed to load %s"), funcIDStr, dataFileName);
  }
}


//*****************************************************
// saveStatData.
// Write stats to SD card.
// Returns 0 or -1 on error
//*****************************************************

int statsClass::saveStatData()
{
  //char fname[32];
  File dbFile;
  int retVal = -1;
  
  //strncpy_P(fname, dataFileName, sizeof(dataFileName));
  //strcpy(fname, "fred");
  dbFile = SD.open(dataFileName, FILE_WRITE);
  if (dbFile)
  {
    dbFile.seek(0);
    dbFile.write((char*)(&statData), (int)sizeof(statData));
    dbFile.close();
    retVal = 0;
  }

  return retVal;
}

//*****************************************************
// loadStatData.
// Load stats from SD card to statdata.
// Returns 0 or -1 on error
//*****************************************************

int statsClass::loadStatData()
{
  File dbFile;
  int retVal = -1;
  
  dbFile = SD.open(dataFileName);
  if (dbFile)
  {
    dbFile.read((char*)(&statData), (int)sizeof(statData));
    dbFile.close();
    retVal = 0;
  }

  return retVal;
}

//*****************************************************
// readStatsFromSD.
// Load stats from SD card and returns data.
// Returns ptr to data struct or NULL
//*****************************************************
struct statsClass::statData_t* statsClass::readStatsFromSD()
{
  struct statData_t* retVal = NULL;
  
  if(loadStatData() == 0)
  {
    retVal = &statData;
  }

  return retVal;
}

//*****************************************************
// clearStatData.
//*****************************************************

void statsClass::clearStatData()
{
  memset(&statData, 0, sizeof(struct statData_t));
  statData.version = STAT_DATA_VERSION;
}


//*****************************************************
// dumpStatData.
//*****************************************************

void statsClass::dumpStatData()
{
  char buf[128];
  int i;
  
  if (loadStatData() >= 0)
  {
    snprintf_P(buf, sizeof(buf) - 1, PSTR("Stats Version %u Sizes %u"), statData.version, statData.numSizes);
    Serial.println(buf);
    for(i = 0; i < statData.numSizes; i++)
    {
      snprintf_P(buf, sizeof(buf) - 1, PSTR("Size %u Count %u"), statData.sizeStats[i].size, statData.sizeStats[i].count);
      Serial.println(buf);
    }
  }
  else
  {
    logger.log(F("ERROR: Failed to load"));
  }
}


statsClass stats;                                                                 // The one and only global stats
*/