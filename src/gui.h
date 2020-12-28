//*****************************************************
//  gui.h
//  Handles all interaction with the user through the GUI.
//*****************************************************
#ifndef GUI_H
#define GUI_H

#include "config.h"

class guiClass
{
  private:
    bool emergencyStopActivated;
  
  public:

    enum pageID_t
    {
      PID_NO_FIELD    = 0,
      PID_FIELD_1     = 1,
      PID_FIELD_2     = 2,
      PID_FIELD_3     = 3,
      PID_FIELD_4     = 4,
      PID_FIELD_5     = 5,
      PID_FIELD_6     = 6,
      PID_FIELD_7     = 7,
      PID_FIELD_8     = 8,
      PID_FIELD_9     = 9,
      PID_FIELD_10    = 10,
      PID_WL_COUNTER  = 11,

      PID_BEADING     = 20,
      PID_MOTOR_TEST  = 21,
      PID_WET_TEST    = 22,
      PID_TEST        = 23,

      PID_START       = 100,
      PID_RESET       = 101,
      PID_NEXT        = 102,
      PID_RETURN      = 103,
      PID_RCHEVRON    = 104,
      PID_TITLE       = 105,
      PID_SELECTED    = 106,
      PID_CONFIRMED   = 107,
      PID_MODIFY      = 108,
      PID_STOP        = 109,
      PID_POSITION    = 110,
      PID_PROXIMITY   = 111,
      PID_DISTANCE    = 112,
      PID_CONTINUE    = 113,
      PID_REPEAT      = 114,
      PID_END         = 115,
      PID_GLUE        = 116,
      PID_MISTER      = 117,
      PID_EXIT        = 118,
      PID_PROCEED     = 119,

      PID_VALUE_1     = 200,
      PID_VALUE_2     = 201,
      PID_VALUE_3     = 202,
    };
    
    enum guiMotorDefs_t
    {
      MOTOR_RAIL,
      MOTOR_APPLIC
    };
  
    enum guiRailMoveDefs_t
    {
      HOME_TO_BEAD,
      BEAD_TO_END,
      END_TO_BEAD,
      HOME_TO_WET
    };
          
    enum guiManagerStates_t
    {
      GMS_INVALID = -1,
      GMS_START = 0,
      GMS_RESET,
      GMS_START_PAGE,
      GMS_BEAD_TEST,
      GMS_PRE_BEADING,
      GMS_BEADING,
      GMS_PRE_TEST,  
      GMS_TEST,  
      GMS_POWER_FAIL,
      GMS_STOP,
      GMS_EMERGENCY_STOP_ACTIVATED,
      GMS_EMERGENCY_STOP,
      GMS_ERROR    
    };

    enum guiEmergencyStopStates_t
    {
      GES_START = 0,
      GES_E_STOP
    };
    
    enum guiTestManagerStates_t
    {
      GMT_START = 0,
      GMT_SELECT,
      GMT_MOTOR_TEST,
      GMT_WET_TEST,  
      GMT_DONE,
      GMT_ERROR    
    };

    enum guiResetManagerStates_t
    {
      GMR_START = 0,
      GMR_RESET,
      GMR_DELAY,
      GMR_STOP_BEADING,
      GMR_APPLIC_TO_HOME,  
      GMR_RAIL_TO_HOME,
      GMR_ERROR,    
      GMR_DONE   
    };
    
    enum menuSpinnerStates_t
    {
      MS_INVALID = -1,
      MS_START = 0,
      MS_SELECT,
      MS_DONE
    };

    enum valueSpinnerStates_t
    {
      VS_INVALID = -1,
      VS_START = 0,
      VS_SELECT,
      VS_DONE
    };

    struct menuSpinnerItems_t                       // Each item in a menu selector
    {
      const char* text;
      float value;
    };

    enum setupStates_t
    {
      SS_INVALID = -1,
      SS_START = 0,
      SS_PANEL_SIZE,
      SS_TOP_OFFSET,
      SS_BOTTOM_OFFSET,
      SS_CONFIRM,
      SS_DONE
    };

    // Obsolete
    enum railHomeStates_t
    {
      MRH_INVALID = -1,
      MRH_START = 0,
      MRH_WAIT_FEEDBACK,
      MRH_DONE
    };
    // ^ Obsolete

    enum motorHomeStates_t
    {
      MH_INVALID = -1,
      MH_START = 0,
      MH_WAIT_FEEDBACK,
      MH_EXIT_DELAY,
      MH_ERROR,
      MH_DONE
    };
        
    enum railMoveStates_t
    {
      RM_INVALID = -1,
      RM_START = 0,
      RM_WAIT_COUNT,
      RM_EXIT_DELAY,
      RM_ERROR,
      RM_DONE
    };
    
    enum railStartStates_t
    {
      MRS_INVALID = -1,
      MRS_START = 0,
      MRS_WAIT_COUNT,
      MRS_DONE
    };
    
    enum manageStartBeadingStates_t
    {
      MGB_START = 0,
      MGB_PROCEED,
      MGB_MOVE_NOZZLES_FWD_DELAY,
      MGB_PUMP_ON_DELAY,
      MGB_NOZZLE_VALVES_ON_DELAY,
      MGB_ERROR,
      MGB_DONE
    };
    
    enum manageStopBeadingStates_t
    {
      MSB_START = 0,
      MSB_PUMP_FWD_OFF_DELAY,
      MSB_PUMP_REV_ON_DELAY,
      MSB_PUMP_REV_OFF_DELAY,
      MSB_NOZZLES_CLOSE_DELAY,
      MSB_NOZZLES_TO_SEALER_DELAY,
      MSB_ERROR,
      MSB_DONE
    };

    enum applicatorToBeadStates_t
    {
      ATB_START = 0,
      ATB_PISTON_DOWN_DELAY,
      ATB_WAIT_DISTANCE,
      ATB_SELECT_CONTINUE,
      ATB_ERROR,
      ATB_DONE
    };
    
    enum manageWorkLifeStates_t
    {
      WL_START = 0,
      WL_IDLE,
      WL_GLUE_ACTIVE,
      WL_DWELL,
      WL_WAIT_PRESS,
      WL_ERROR,
      WL_DONE
    };
    
  public:
    guiClass();
    
    void begin();                                                         // Initialise
    void manageGui();                                                     // The main start page manager
    int manageReset(bool doBegin = false);                                // The reset machine
    int manageStopBeading(bool doBegin = false);                          // Stop the beading process
    int manageStartBeading(bool doBegin = false);                         // Start beading
    void manageWorkLife();                                                // Manage the Work Life machine & display 
    uint32_t getWLTimeToGo();                                             // Get Work Life time to go

    static int m_menuSpinnerSelection;                                    // Currently selected menu selection
    //static uint32_t m_workLifeStartTime;                                  // Time that the current adheshive application started
    
    uint32_t m_wlTimeToGo;                                                // Secs to go on work life      
                                                                          // Menu selection spinner
    int manageMenuSpinner(struct menuSpinnerItems_t* menuItems, touchscreen::pageFieldDef_t* page, size_t pageSize, int selection = 0, bool doBegin = false);
                                                                          // Convert mm to string number system smart
    char* mmToDisplayStr(char* buf, uint16_t bufSize, uint16_t* mmValue, numberSystem_t metricImperial);
                                                                          // Manage selection by value spinner
    int manageValueSpinner(uint16_t* mmValue, uint16_t max, uint16_t min, uint16_t increment, 
                                const touchscreen::pageFieldDef_t* page, size_t pageSize,
                                numberSystem_t metricImperial = NS_METRIC_CM, bool doBegin = false);
                                
    int manageParameterSetup(bool doBegin = false);                       // Setup the parameters
    int findMenuItemByValue(menuSpinnerItems_t* itemArray, float value);  // Find a menu item in list by the value
    
    int manageRailHome(bool doBegin = false);                             // Send the rail motor home
    int manageMotorToHome(guiMotorDefs_t motor, bool doBegin = false);    // Send a motor home
    int manageRailMove(guiRailMoveDefs_t journey, bool doBegin = false);  // Move rail to position
    int manageTestPage(bool doBegin = false);                             // Select a test
    int manageApplicatorToBead(bool doBegin = false);                     // Applicator to bead position
    
    //int manageRailStart(bool doBegin = false);                            // Send the rail motor to start
    int manageRailToBead(bool doBegin = false);                            // Send the rail motor to start
    int runFSM(int state, int doneState, int errorState, int returnFromFSM);// Manage FSM return codes
    void emergencyStop();                                                 // Emergency stop routine

    static const char beadingStr[];
    static const char motorTestStr[];
    static const char wetTestStr[];
    static const char testStr[];
    static const char nextStr[];
    static const char returnStr[];
    static const char confirmedStr[];
    static const char modifyStr[];
    static const char powerFailureStr[];
    static const char railStr[];
    static const char panelSizeStr[];
    static const char offsetTopStr[];
    static const char offsetBottomStr[];
    static const char applicStr[];
    static const char applicRailStr[];
    static const char toStartStr[];
    static const char toHomeStr[];
    static const char railToBeadStr[];
    static const char railToEndStr[];
    static const char railToWetStr[];
    static const char settingUpStr[];
    static const char waitingStr[];
    static const char rightChevronStr[];
    static const char blankStr[];
    static const char railToHomeStr[];
    static const char railToStartStr[];
    static const char applicatorToHomeStr[];
    static const char applicatorToStartStr[];
    static const char emergencyStopStr[];
    static const char pleaseWaitStr[];
    static const char startStr[];
    static const char stopStr[];
    static const char resetStr[];
    static const char resettingStr[];
    static const char positionStr[];
    static const char ProximityStr[];
    static const char distanceStr[];
    static const char continueStr[];
    static const char repeatStr[];
    static const char endStr[];
    static const char glueStr[];
    static const char misterStr[];
    static const char exitStr[];
    static const char startBeadingStr[];
    static const char proceedStr[];


    static const struct touchscreen::pageFieldDef_t startPageDef[];
    static const struct touchscreen::pageFieldDef_t resetPageDef[];
    static const struct touchscreen::pageFieldDef_t resettingPageDef[];
    static const struct touchscreen::pageFieldDef_t beadingTestPageDef[];
    static const struct touchscreen::pageFieldDef_t powerFailPageDef[];
    static const struct touchscreen::pageFieldDef_t emergencyStopPageDef[];
    static const struct touchscreen::pageFieldDef_t pleaseWaitPageDef[];
    static const struct touchscreen::pageFieldDef_t setupPanelSizePageDef[];
    static const struct touchscreen::pageFieldDef_t setupOffsetTopPageDef[];
    static const struct touchscreen::pageFieldDef_t setupOffsetBottomPageDef[];
    static const struct touchscreen::pageFieldDef_t setupConfirmPageDef[];
    static const struct touchscreen::pageFieldDef_t railHomePageDef[];
    static const struct touchscreen::pageFieldDef_t railHomeToBeadPageDef[];
    static const struct touchscreen::pageFieldDef_t railBeadToEndPageDef[];
    static const struct touchscreen::pageFieldDef_t railEndToBeadPageDef[];
    static const struct touchscreen::pageFieldDef_t railHomeToWetPageDef[];
    static const struct touchscreen::pageFieldDef_t applicatorHomePageDef[];
    static const struct touchscreen::pageFieldDef_t sendRailToBeadPageDef[];
    static const struct touchscreen::pageFieldDef_t testPageDef[];
    static const struct touchscreen::pageFieldDef_t continueReturnPageDef[];
    static const struct touchscreen::pageFieldDef_t startBeadingStopPageDef[];
    static const struct touchscreen::pageFieldDef_t proceedPageDef[];

};

extern guiClass gui;                              // The one and only gui


#endif

