/*
//*****************************************************
// statistics.h
//*****************************************************
#ifndef STATISTICS_H
#define STATISTICS_H

#define STAT_DATA_VERSION   1                       // The structure version in case it needs to change
#define MAX_SIZE_STATS      8                       // Maximum different sizes included in the stats
#define STATS_FILE_NAME     "stats.dat"             // File name for the statistics file

class statsClass
{
  private:

  public:
    statsClass();    
    void begin();

    struct sizeStats_t                                  // One size count
    {
      uint16_t size;                                    // The size in mm
      uint16_t count;                                   // The count of done panels
    };

    struct statData_t
    {
      uint16_t version;                                 // The version of this structure
      uint16_t numSizes;                                // Number of sizes recorded
      sizeStats_t sizeStats[MAX_SIZE_STATS];            // Array of size counts
    };

    struct statData_t* readStatsFromSD();               // Read stats from SD return ptr to statData
    int writeStatsToSD();                               // Write the statsData to SD
    int incSizeCount(uint16_t size);                    // Increment the size counter. Create if not exist
    void dumpStatData();                                // Dump to serial port
    void incSizeStat(uint16_t panelSize);               // Update stats with panel size
    
  private:
    struct statData_t statData;                         // A place to stash the statistics
    static char dataFileName[];                         // Name of the stats file
    
    void clearStatData();                               // Clear and init the data struct
    int saveStatData();                                 // Write data to SD card
    int loadStatData();                                 // Load data from SD card
};

extern statsClass stats;                                // The one and only stats


#endif

*/