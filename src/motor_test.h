//*****************************************************
//  motor_test.h
//  Runs the 'Motor Test'
//*****************************************************
#ifndef MOTOR_TEST_H
#define MOTOR_TEST_H

//class motorTestClass: public guiClass
class motorTestClass
{
  public:
  /*
    enum motorTestManagerStates_t
    {
      MTM_INVALID = -1,
      MTM_START = 0,
      MTM_SETUP,
      MTM_RAIL_TO_HOME,
      //MTM_RAIL_TO_START,
      MTM_RAIL_TO_BEAD,
      MTM_CONFIRM_APPLIC_TO_HOME,
      MTM_APPLIC_TO_HOME,
      MTM_CONFIRM_APPLIC_TO_BEAD,
      MTM_APPLIC_TO_BEAD,
      MTM_CONFIRM_RAIL_TO_END,
      MTM_RAIL_TO_END,
      MTM_CONFIRM_RAIL_TO_START2,
      MTM_RAIL_TO_START2,
      MTM_CONFIRM_APPLIC_RAIL_TO_HOME,
      MTM_APPLIC_RAIL_TO_HOME,
      MTM_ERROR,
      MTM_STOP
    };
  */  

    enum motorTestManagerStates_t
    {
      MTM_INVALID = -1,
      MTM_START = 0,
      MTM_SETUP,
      MTM_RAIL_TO_HOME,
      //MTM_RAIL_TO_START,
      MTM_RAIL_TO_BEAD,
      MTM_SELECT_TEST,
      MTM_RAIL_TEST,
      MTM_PROXIMITY_TEST,
      MTM_DISTANCE_SWITCH_TEST,
      MTM_ERROR,
      MTM_DONE
    };

    enum motorTMRailPositionStates_t
    {
      MTMRP_START = 0,
      MTMRP_BEAD_TO_END,
      MTMRP_CONTINUE_1,
      MTMRP_END_TO_BEAD,
      MTMRP_CONTINUE_2,
      MTMRP_ERROR,
      MTMRP_DONE
    };
    
    enum motorTDistanceSensorStates_t
    {
      MTDS_START = 0,
      MTDS_APPLICATOR_TO_HOME,
      MTDS_APPLICATOR_TO_BEAD,
      MTDS_REPEAT_END,
      MTDS_ERROR,
      MTDS_DONE
    };
    
    enum motorTDistanceSwitchStates_t
    {
      MTDW_START = 0,
      MTDW_APPLICATOR_TO_HOME,
      MTDW_MOVE_APPLICATOR_TO_SWITCH,
      MTDW_ERROR,
      MTDW_DONE
    };
    
  public:
    motorTestClass();
    
    void begin();                                 // Initialise

  public:
    int manageMotorTest(bool doBegin = false);    // Motor test manager

  public:
    static const struct touchscreen::pageFieldDef_t motorTestSetupPageDef[];
    static const struct touchscreen::pageFieldDef_t selectMotorTestPageDef[];
    static const struct touchscreen::pageFieldDef_t selectMotorContinueReturnPageDef[];
    static const struct touchscreen::pageFieldDef_t selectMotorContinuePageDef[];
    static const struct touchscreen::pageFieldDef_t repeatEndPageDef[];
    static const struct touchscreen::pageFieldDef_t distanceTestPageDef[];
    
    //static const struct touchscreen::pageFieldDef_t motorConfirmRailToStartPageDef[];
    //static const struct touchscreen::pageFieldDef_t motorRailToStartPageDef[];
    //static const struct touchscreen::pageFieldDef_t motorConfirmApplicatorToHomePageDef[];
    //static const struct touchscreen::pageFieldDef_t motorApplicatorToHomePageDef[];
    //static const struct touchscreen::pageFieldDef_t motorConfirmApplicatorToBeadPageDef[];
    //static const struct touchscreen::pageFieldDef_t motorApplicatorToBeadPageDef[];
    //static const struct touchscreen::pageFieldDef_t motorConfirmRailToEndPageDef[];
    //static const struct touchscreen::pageFieldDef_t motorRailToEndPageDef[];
    //static const struct touchscreen::pageFieldDef_t motorConfirmApplicRailToHomePageDef[];
    //static const struct touchscreen::pageFieldDef_t motorApplicRailToHomePageDef[];
    
  private:
    int manageMTRailPosition(bool doBegin = false);           // Motor Test Rail Position
    int manageMTDistanceSensor(bool doBegin = false);         // Motor test distance sensor setup                          
    int manageMTDistanceSwitch(bool doBegin = false);         // Motor test switch setup   
  
};

extern motorTestClass motorTest;                              // The one and only motorTest


#endif

