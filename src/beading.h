//*****************************************************
//  beading.h
//  Runs the beading procedure
//*****************************************************
#ifndef BEADING_H
#define BEADING_H

class beadingClass
{
  public:
    #define BATCH_VERSION      4
  
    enum beadingManagerStates_t
    {
      BED_START = 0,
      BED_SETUP,
      BED_RAIL_TO_HOME,
      BED_RAIL_TO_BEAD,
      BED_APPLIC_TO_HOME,
      BED_APPLIC_TO_BEAD,
      BED_START_BEADING,
      BED_RAIL_TO_END,
      BED_STOP_BEADING,
      BED_STOP_DELAY,
      BED_RAIL_END_TO_BEAD,
      BED_APPLIC_TO_HOME_2,
      BED_RAIL_TO_HOME_2,
      BED_ERROR,
      BED_DONE
    };

    enum beadRecordType_t
    {
      BRT_END_OF_BEAD   = 0,
      BRT_END_OF_BATCH  = 1
    };
    
    enum beadStatus_t
    {
      BST_OK            = 0,
      BST_RESET         = 1,
      BST_FAIL          = 2,
      BST_ESTOP         = 3,
      BST_ERROR         = 999
    };
    
  struct bead_t
  {
    uint32_t  beadID;                             // Current accumulated bead id
    uint32_t  batchID;                            // id for this batch. id=0 is indicates no previous batch
    beadRecordType_t  recordType;                 // End Of Bead | End Of Batch
    beadStatus_t  status;                         // 'OK' | 'RESET' | 'ERROR'
    uint16_t  beadInBatch;                        // Bead number in this batch
    uint32_t  completionTime;                     // Time this bead completed
    uint16_t  panelLength;                        // Length of this panel
    uint16_t  topOffset;                          // Top Offset
    uint16_t  bottomOffset;                       // Bottom offset
    uint32_t  projectID;                          // Entered by user
  };
  
  struct batch_t                                  // Data for the current batch
  {
    uint16_t  version;                            // Version of this structure
    uint32_t  startTime;                          // Time the batch started
    uint32_t  workLifeStartTime;                  // Started after the first bead is laid
    uint8_t   inBatch;                            // Non zero if batch is active
    uint8_t   inBead;                             // Non zero if bead is active
    struct bead_t  bead;                          // The current bead data
  };
    
  typedef  struct batch_t BATCH;
  
  public:
    beadingClass();
    
    static char m_sdBatchFileName[];              // Batch data file name
    static char m_sdBatchLogDirName[];            // Dir name for server uploads
        
    void begin();                                 // Initialise
    void clearBatchData();                        // Resets the batch and bead data
    void dumpBatchData(batch_t* batch);           // Print the batch data
    void dumpBatchData();                         // Print current & saved batch data
    int loadCurrentBatchData(struct batch_t* bp = NULL);  // Load batch data
    void setInBatch(int val);                     // Non zero indicates batch in progress
    void setInBead(int val);                      // Non zero indicates bead in progress
    int getInBead();                              // Returns the inBead flag
    int getInBatch();                             // Returns the inBatch flag
    void setBatchStartTime();                     // Record the start of batch time
    void setWLStartTime();                        // Secs start for WL
    uint32_t getWLStartTime();                    // Returns WL start secs
    //void setBatchID(uint32_t id);                 // Set the batch number
    //uint32_t getBatchID();                        // Get the batch number
    void setBeadID(uint32_t id);                  // Set the totalised bead number 
    uint32_t getBeadID();                         // Get the totalised bead number
    uint32_t getBeadInBatch();                    // Get bead number in this batch
    void setBeadInBatch(uint32_t id);             // Set bead number in the batch
    void incBeadIDs();                            // Increment the bead ids
    void endOfBatch();                            // Reset batch and send record
    void removeBeadUploadFile(char* fileName);    // Delete a file
    uint32_t getProjectID();                      // Get current project ID
    void setProjectID(uint32_t val);              // Set current project ID
    uint32_t getBatchID();                        // Get current batch ID
    void setBatchID(uint32_t val);                // Set current batch ID

  public:
    int manageBeading(bool doBegin = false);      // Beading manager
    void dumpUploadBatchData(char* fileName);     // Dump a bead file from upload
    int loadUploadBatchData(struct batch_t* bp, char* fileName);// Load beading file from upload
    void saveEndOfBeadForUpload(beadRecordType_t type, beadStatus_t status);// Save a bead record in upload

  private:
    int saveCurrentBatchData();                   // Save the default batch data
    int saveBatchDataForUpload(struct batch_t* bp);// Save batch data for upload
    char* makeBeadUploadPath(char* buf, size_t bufSize); // Get a file path for a bead upload
    
    struct batch_t m_currentBatch;                // Batch and bead data
  
};

extern beadingClass beader;                       // The one and only beader


#endif

