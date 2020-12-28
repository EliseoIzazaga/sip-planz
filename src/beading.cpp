//*****************************************************
//  beading.cpp
//  Runs the beading procedure
//*****************************************************
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <DS3232RTC.h> 
#include <Time.h> 
#include "siplaminator.h"
#include "touchscreen.h"
#include "gui.h"
#include "beading.h"
#include "io.h"
#include "Logger.h"
#include "console.h"

char   beadingClass::m_sdBatchFileName[]  = BEADER_BATCH_FILENAME;    // Name of the batch data file

beadingClass::beadingClass()
{
}

//*****************************************************
// begin.
//*****************************************************

void beadingClass::begin()
{
  const char* funcIDStr = PSTR("beadingClass::begin: ");
  char buf[LG_MAX_PATHNAME_LEN];
  
  memset(&m_currentBatch, 0, sizeof(m_currentBatch));
  
  if (!SD.exists(m_sdBatchFileName))
  {
    logger.log(F("%SBatch file [%s] not found"), funcIDStr, m_sdBatchFileName);
    saveCurrentBatchData();
  }
  else
  {
    logger.log(F("%SIndex file [%s] exists"), funcIDStr, m_sdBatchFileName);
    loadCurrentBatchData();
    if (m_currentBatch.version != BATCH_VERSION)
    {
      logger.log(F("%SWrong batch version. Replacing"), funcIDStr, m_sdBatchFileName);
      logger.sdRm(m_sdBatchFileName);
      memset(&m_currentBatch, 0, sizeof(m_currentBatch));
      m_currentBatch.version = BATCH_VERSION;
      saveCurrentBatchData();
    }
      
    //m_currentBatch.bead.projectID = 9999;    // Just for testing purposes
  }
  
  if (getInBatch() > 0)
  {
    logger.log(F("%SSaving part batch"), funcIDStr);
    saveEndOfBeadForUpload(BRT_END_OF_BATCH, BST_RESET);
  }

  setBeadInBatch(0);
  setInBatch(0);
  setInBead(0);
}

//*****************************************************
// getProjectID.
// Get the project ID from the current batch.
//*****************************************************

uint32_t beadingClass::getProjectID()
{
  return m_currentBatch.bead.projectID;
}

//*****************************************************
// setProjectID.
// Set the project id in the current batch.
//*****************************************************

void beadingClass::setProjectID(uint32_t val)
{
  m_currentBatch.bead.projectID = val;
  saveCurrentBatchData();
}

//*****************************************************
// getBatchID.
// Get the Batch ID from the current batch.
//*****************************************************

uint32_t beadingClass::getBatchID()
{
  return m_currentBatch.bead.batchID;
}

//*****************************************************
// setBatchID.
// Set the batch id in the current batch.
//*****************************************************

void beadingClass::setBatchID(uint32_t val)
{
  m_currentBatch.bead.batchID = val;
  saveCurrentBatchData();
}

//*****************************************************
// clearBatchData.
// Reset the batch data to all zeros.
//*****************************************************

void beadingClass::clearBatchData()
{
  memset(&m_currentBatch, 0, sizeof(m_currentBatch));
  saveCurrentBatchData();
}

//*****************************************************
// loadCurrentBatchData.
// Load batch data from SD card. If bp == NULL then load to
// current batch data.
// Returns 0 or -1 on error
//*****************************************************

int beadingClass::loadCurrentBatchData(struct batch_t* bp)
{
  File dbFile;
  struct batch_t* destP;
  int retVal = -1;
  
  if (bp)
  {
    destP = bp;
  }
  else
  {
    destP = &m_currentBatch;
  }
  
  dbFile = SD.open(m_sdBatchFileName);
  if (dbFile)
  {
    dbFile.read((char*)(destP), (int)sizeof(m_currentBatch));
    dbFile.close();
    retVal = 0;
  }

  return retVal;
}


//*****************************************************
// loadUploadBatchData.
// Load batch data from SD card upload directory.
// Returns 0 or -1 on error
//*****************************************************

int beadingClass::loadUploadBatchData(struct batch_t* bp, char* fileName)
{
  const char* funcIDStr = PSTR("loadUploadBatchData: ");
  File dbFile;
  char buf[32];
  int retVal = -1;
  
  snprintf_P(buf, sizeof(buf) - 1, PSTR("%S/%s"), PSTR(SERVER_UPLOAD_DIR), fileName);
  logger.log(F("%SFile [%s]"), funcIDStr, buf);
  
  dbFile = SD.open(buf);
  if (dbFile)
  {
    dbFile.read((char*)(bp), (int)sizeof(struct batch_t));
    dbFile.close();
    retVal = 0;
  }

  return retVal;
}


//*****************************************************
// saveCurrentBatchData.
// Write batch data to SD card.
// Returns 0 or -1 on error
//*****************************************************

int beadingClass::saveCurrentBatchData()
{
  File dbFile;
  int retVal = -1;
  
  dbFile = SD.open(m_sdBatchFileName, FILE_WRITE);
  if (dbFile)
  {
    dbFile.seek(0);
    dbFile.write((char*)(&m_currentBatch), (int)sizeof(m_currentBatch));
    dbFile.close();
    retVal = 0;
  }

  return retVal;
}

//*****************************************************
// saveBatchDataForUpload.
// Writes a batch struct into the upload directory ready for
// upload to the server. 
// Returns 0 or -1 on error
//*****************************************************

int beadingClass::saveBatchDataForUpload(struct batch_t* bp)
{
  const char* funcIDStr = PSTR("saveBatchDataForUpload: ");
  File dbFile;
  char buf[32];
  int retVal = -1;
  
  makeBeadUploadPath(buf, sizeof(buf));
  logger.log(F("%SFile [%s]"), funcIDStr, buf);
  dbFile = SD.open(buf, FILE_WRITE);
  if (dbFile)
  {
    dbFile.write((char*)(bp), (int)sizeof(struct batch_t));
    dbFile.close();
    retVal = 0;
  }

  return retVal;
}

//*****************************************************
// saveEndOfBeadForUpload.
// 
//*****************************************************

void beadingClass::saveEndOfBeadForUpload(beadRecordType_t type, beadStatus_t status)
{
  const char* funcIDStr = PSTR("saveEndOfBeadForUpload: ");
  struct batch_t locBatch; 
  
  logger.log(F("%S type: %d status: %d"), funcIDStr, type, status);
  m_currentBatch.bead.completionTime = now();
  memcpy(&locBatch, &m_currentBatch, sizeof(struct batch_t));
  locBatch.bead.recordType = type;
  locBatch.bead.status = status;
  saveCurrentBatchData();                                       // Save the current batch with end time
  saveBatchDataForUpload(&locBatch);                            // Save for upload with type and status
}

//*****************************************************
// makeBeadUploadPath()
// Returns the path to a bead file in the upload directory.
//*****************************************************

char* beadingClass::makeBeadUploadPath(char* buf, size_t bufSize)
{
  snprintf_P(buf, bufSize - 1, PSTR("%S/%lu.%S"), PSTR(SERVER_UPLOAD_DIR), now() % 100000000L, PSTR(BEAD_FILE_EXT));
  return buf;
}

//*****************************************************
// removeBeadUploadFile()
// Deletes a file from the upload dir.
//*****************************************************

void beadingClass::removeBeadUploadFile(char* fileName)
{
  const char* funcIDStr = PSTR("removeBeadUploadFile: ");
  char buf[LG_MAX_PATHNAME_LEN + 1];
  
  snprintf_P(buf, LG_MAX_PATHNAME_LEN, PSTR("%S/%s"), PSTR(SERVER_UPLOAD_DIR), fileName);
  logger.log(F("%SRemoving [%s]"), funcIDStr, buf);
  SD.remove(buf);
}


//*****************************************************
// dumpBatchData.
//*****************************************************

void beadingClass::dumpBatchData(batch_t* batch)
{
  char buf[32];
  size_t bufsize = sizeof(buf);
  
  logger.log(F("%10S: %d"), PSTR("version"), batch->version);
  console.timeToStr(buf, bufsize, batch->startTime);
  logger.log(F("%10S: %s"), PSTR("Start"), buf);
  console.timeToStr(buf, bufsize, batch->workLifeStartTime);
  logger.log(F("%10S: %s"), PSTR("WL Start"), buf);
  //logger.log(F("%10S: %lu"), PSTR("WL Start"), batch->workLifeStartMillis);
  logger.log(F("%10S: %d"), PSTR("inBatch"), batch->inBatch);
  logger.log(F("%10S: %d"), PSTR("inBead"), batch->inBead);
  logger.log(F("%10S: %lu"), PSTR("beadID"), batch->bead.beadID);
  logger.log(F("%10S: %d"), PSTR("recordType"), batch->bead.recordType);
  logger.log(F("%10S: %d"), PSTR("status"), batch->bead.status);
  logger.log(F("%10S: %u"), PSTR("BedInBatch"), batch->bead.beadInBatch);
  logger.log(F("%10S: %lu"), PSTR("batchID"), batch->bead.batchID);
  console.timeToStr(buf, bufsize, batch->bead.completionTime);
  logger.log(F("%10S: %s"), PSTR("Bead end"), buf);
  logger.log(F("%10S: %d"), PSTR("Panel len"), batch->bead.panelLength);
  logger.log(F("%10S: %d"), PSTR("Top OS"), batch->bead.topOffset);
  logger.log(F("%10S: %d"), PSTR("Bottom OS"), batch->bead.bottomOffset);
  logger.log(F("%10S: %lu"), PSTR("Project ID"), batch->bead.projectID);
}



//*****************************************************
// dumpSavedBatchData.
//*****************************************************

void beadingClass::dumpBatchData()
{
  const char* funcIDStr = PSTR("dumpBatchData: ");
  struct batch_t bp;
  
  logger.log(F("%SCurrent"), funcIDStr);
  dumpBatchData(&m_currentBatch);
  loadCurrentBatchData(&bp);
  logger.log(F("%SSaved"), funcIDStr);
  dumpBatchData(&bp);
}

//*****************************************************
// dumpUploadBatchData.
//*****************************************************

void beadingClass::dumpUploadBatchData(char* fileName)
{
  const char* funcIDStr = PSTR("dumpUploadBatchData: ");
  struct batch_t bp;

  logger.log(F("%S"), funcIDStr);
    
  if (fileName != NULL)
  {
    if (loadUploadBatchData(&bp, fileName) >= 0)
    {
      dumpBatchData(&bp);
    }
  }
}

//*****************************************************
// setInBatch
// Sets or clears the inBatch flag indicating that a batch is in progress.
//*****************************************************

void beadingClass::setInBatch(int val)
{
  const char* funcIDStr = PSTR("setInBatch: ");
  m_currentBatch.inBatch = val;
  
  logger.log(F("%S %d"), funcIDStr, m_currentBatch.inBatch);
  saveCurrentBatchData();
}

//*****************************************************
// setInBead
// Sets or clears the inBead flag indicating that a bead is in progress.
//*****************************************************

void beadingClass::setInBead(int val)
{
  const char* funcIDStr = PSTR("setInBead: ");
  m_currentBatch.inBead = val;
  
  logger.log(F("%S %d"), funcIDStr, m_currentBatch.inBead);
  saveCurrentBatchData();
}

//*****************************************************
// getInBead
// gets the inBead flag.
//*****************************************************

int beadingClass::getInBead()
{
  return m_currentBatch.inBead;
}


//*****************************************************
// endOfBatch
// resets the batch and sends an end of batch record.
//*****************************************************

void beadingClass::endOfBatch()
{
  const char* funcIDStr = PSTR("endOfBatch: ");
  
  setInBatch(0);
  setInBead(0);
  saveEndOfBeadForUpload(BRT_END_OF_BATCH, BST_OK);
  logger.log(F("%S"), funcIDStr);
}


//*****************************************************
// setBatchStartTime
// Called when the batch starts to set the start time.
//*****************************************************

void beadingClass::setBatchStartTime()
{
  m_currentBatch.startTime = now();
}

//*****************************************************
// setWLStartTime
// Start of the work life counter.
//*****************************************************

void beadingClass::setWLStartTime()
{
  m_currentBatch.workLifeStartTime = now();
}

//*****************************************************
// getWLStartTime
// Start of the work life counter.
//*****************************************************

uint32_t beadingClass::getWLStartTime()
{
  return m_currentBatch.workLifeStartTime;
}

//*****************************************************
// getInBatch
// Returns the inBatch flag.
//*****************************************************

int beadingClass::getInBatch()
{
  return m_currentBatch.inBatch;
}

//*****************************************************
// setBatchID.
//*****************************************************
/*
void beadingClass::setBatchID(uint32_t id)
{
  m_currentBatch.bead.batchID = id;
}
*/
//*****************************************************
// getBatchID.
//*****************************************************
/*
uint32_t beadingClass::getBatchID()
{
  return m_currentBatch.bead.batchID;
}
*/
//*****************************************************
// setBeadID.
//*****************************************************

void beadingClass::setBeadID(uint32_t id)
{
  m_currentBatch.bead.beadID = id;
  saveCurrentBatchData();
}


//*****************************************************
// getBeadID.
//*****************************************************

uint32_t beadingClass::getBeadID()
{
  return m_currentBatch.bead.beadID;
}

//*****************************************************
// setBeadInBatch.
// Set the bead number in this batch.
//*****************************************************
void beadingClass::setBeadInBatch(uint32_t id)
{
  m_currentBatch.bead.beadInBatch = id;
  saveCurrentBatchData();
}

//*****************************************************
// getBeadInBatch.
//*****************************************************

uint32_t beadingClass::getBeadInBatch()
{
  return m_currentBatch.bead.beadInBatch;
}

//*****************************************************
// incBeadIDs.
// Increments the total bead number and the bead in batch.
//*****************************************************

void beadingClass::incBeadIDs()
{
  const char* funcIDStr = PSTR("incBeadIDs: ");
  m_currentBatch.bead.beadID++;
  m_currentBatch.bead.beadInBatch++;
  //logger.log(F("%SbeadID %u beadInBatch %u"), funcIDStr, m_currentBatch.bead.beadID, m_currentBatch.bead.beadInBatch);
}

//*****************************************************
// manageBeading.
// Returns: 0   - Done
//          1   - ongoing
//          -1  - Error done
//*****************************************************

int beadingClass::manageBeading(bool doBegin)
{
  const char* funcIDStr = PSTR("manageBeading: ");
  static beadingManagerStates_t state;
  static beadingManagerStates_t lastState;
  static uint32_t timerStart;                                                       // Timer
  int retVal = 1;

  if (doBegin) 
  {
    logger.log(F("%SStart"), funcIDStr);
    state = BED_START;
    lastState = BED_START;
  }

  switch(state)
  {
    case BED_START:                                                                 // Starts of a bead
      if (getInBatch() == 0)                                                        // Zero is start of batch
      {
        setBatchID(getBatchID() + 1L);                                              // Update the batch number
        setBeadInBatch(0L);
        setBatchStartTime();                                                        // Set the start time of batch
        setInBatch(1);                                                              // We must be in a batch
      }

      gui.manageParameterSetup(true);
      state = BED_SETUP;
      break;
      
    case BED_SETUP:
      state = (beadingManagerStates_t)gui.runFSM(state, BED_RAIL_TO_HOME, BED_ERROR, gui.manageParameterSetup());
      if (state == BED_RAIL_TO_HOME) 
      {
        m_currentBatch.bead.panelLength = configGetPanelLength();
        m_currentBatch.bead.topOffset = configGetOffsetTop();
        m_currentBatch.bead.bottomOffset = configGetOffsetBottom();
        saveCurrentBatchData();                                                       // Save the batch data
        gui.manageMotorToHome(gui.MOTOR_RAIL, true);                                // Send rail motor home
      }
      break;
      
    case BED_RAIL_TO_HOME:                                                          // Wait for rail to go home
      state = (beadingManagerStates_t)gui.runFSM(state, BED_RAIL_TO_BEAD, BED_ERROR, gui.manageMotorToHome(gui.MOTOR_RAIL));
      if (state == BED_RAIL_TO_BEAD) gui.manageRailMove(gui.HOME_TO_BEAD, true);    // Send rail motor to bead position
      break;
      
    case BED_RAIL_TO_BEAD:                                                          // Send rail motor to bead position
      state = (beadingManagerStates_t)gui.runFSM(state, BED_APPLIC_TO_HOME, BED_ERROR, gui.manageRailMove(gui.HOME_TO_BEAD));
      if (state == BED_APPLIC_TO_HOME) gui.manageMotorToHome(gui.MOTOR_APPLIC, true); // Send applicator motor to home
      break;

    case BED_APPLIC_TO_HOME:
      state = (beadingManagerStates_t)gui.runFSM(state, BED_APPLIC_TO_BEAD, BED_ERROR, gui.manageMotorToHome(gui.MOTOR_APPLIC));
      if (state == BED_APPLIC_TO_BEAD) gui.manageApplicatorToBead(true);            // Send applicator motor to bead
      break;

    case BED_APPLIC_TO_BEAD:
      state = (beadingManagerStates_t)gui.runFSM(state, BED_START_BEADING, BED_ERROR, gui.manageApplicatorToBead());
      if (state == BED_START_BEADING) gui.manageStartBeading(true);                 // Start beading
      break;

    case BED_START_BEADING:
      state = (beadingManagerStates_t)gui.runFSM(state, BED_RAIL_TO_END, BED_ERROR, gui.manageStartBeading());
      if (state == BED_RAIL_TO_END) gui.manageRailMove(gui.BEAD_TO_END, true);      // Rail to end
      break;

    case BED_RAIL_TO_END:
      state = (beadingManagerStates_t)gui.runFSM(state, BED_STOP_BEADING, BED_ERROR, gui.manageRailMove(gui.BEAD_TO_END));
      if (state == BED_STOP_BEADING) 
      {
        incBeadIDs();                                                               // Inc both bead counts
        setInBead(1);                                                               // We are starting a bead
        if (getBeadInBatch() == 1)                                                  // End of the first bead
        {
          setWLStartTime();                                                         // Start the Work Life
        }                                   
        gui.manageStopBeading(true);                                                // Stop beading
      }
      break;

    case BED_STOP_BEADING:
      state = (beadingManagerStates_t)gui.runFSM(state, BED_STOP_DELAY, BED_ERROR, gui.manageStopBeading());
      if (state == BED_STOP_DELAY) 
      {
        timerStart = millis();                                                      // Start the exit delay
        io.setOutputPin(IO_DOUT_FAN_MISTER, LOW);                                   // Start mister
      }
      break;

    case BED_STOP_DELAY:
      if ((millis() - timerStart) > configGetDelay(10))
      {
        logger.log(F("%STO stop beading"), funcIDStr);
        gui.manageRailMove(gui.END_TO_BEAD, true);
        state = BED_RAIL_END_TO_BEAD;
      }
      break;
      
    case BED_RAIL_END_TO_BEAD:
      state = (beadingManagerStates_t)gui.runFSM(state, BED_APPLIC_TO_HOME_2, BED_ERROR, gui.manageRailMove(gui.END_TO_BEAD));
      if (state == BED_APPLIC_TO_HOME_2) 
      {
        io.setOutputPin(IO_DOUT_FAN_MISTER, HIGH);                                   // Stop mister
        setInBead(0);                                                                // Not in a bead
        saveEndOfBeadForUpload(BRT_END_OF_BEAD, BST_OK);
        gui.manageMotorToHome(gui.MOTOR_APPLIC, true);
      }
      break;

    case BED_APPLIC_TO_HOME_2:
      state = (beadingManagerStates_t)gui.runFSM(state, BED_RAIL_TO_HOME_2, BED_ERROR, gui.manageMotorToHome(gui.MOTOR_APPLIC));
      if (state == BED_RAIL_TO_HOME_2) gui.manageMotorToHome(gui.MOTOR_RAIL, true);   // Send rail motor home
      break;

    case BED_RAIL_TO_HOME_2:
      state = (beadingManagerStates_t)gui.runFSM(state, BED_DONE, BED_ERROR, gui.manageMotorToHome(gui.MOTOR_RAIL));
      break;
      
    case BED_ERROR:
      logger.log(F("%SError"), funcIDStr);
      state = BED_START;
      retVal = -1;
      break;
      
    case BED_DONE:
      // beadCounter++;
    default:
      state = BED_START;
      retVal = 0;
      break;
  }

    
  if (state != lastState)
  {
    logger.log(F("%Sstate %d -> %d"), funcIDStr, lastState, state);
    lastState = state;
  }

  return retVal;
}



beadingClass beader;                             // The one and only global beader
