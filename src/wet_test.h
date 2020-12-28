//*****************************************************
//  wet_test.h
//  Runs the 'Wet Test'
//*****************************************************
#ifndef WET_TEST_H
#define WET_TEST_H

class wetTestClass
{
  public:
    enum wetTestManagerStates_t
    {
      WTM_INVALID = -1,
      WTM_START = 0,
      WTM_RAIL_TO_HOME,
      WTM_RAIL_TO_WET,
      WTM_APPLIC_TO_HOME,
      WTM_APPLIC_TO_BEAD,
      WTM_RENDER_GLUE_MISTER,
      WTM_SELECT_GLUE_MISTER,
      WTM_GLUE_TEST,
      WTM_MISTER_TEST,
      WTM_ERROR,
      WTM_DONE
    };

    enum wetGlueTestManagerStates_t
    {
      WGT_START = 0,
      WGT_START_RETURN,
      WGT_START_BEADING,
      WGT_STOP_PAGE,
      WGT_STOP_BEADING,
      WGT_ERROR,
      WGT_DONE
    };
    
    enum wetMisterTestManagerStates_t
    {
      WMT_START = 0,
      WMT_START_RETURN,
      WMT_STOP_PAGE,
      WMT_ERROR,
      WMT_DONE
    };
    
  public:
    wetTestClass();
    
    void begin();                                 // Initialise

  public:
    int manageWetTest(bool doBegin = false);        // Wet test manager

  public:
    static const struct touchscreen::pageFieldDef_t selectWetGlueMisterPageDef[];
    static const struct touchscreen::pageFieldDef_t startReturnPageDef[];
    static const struct touchscreen::pageFieldDef_t wetGlueTestStopPageDef[];
    
  private:
    //int manageMTRailPosition(bool doBegin = false);           // Motor Test Rail Position
    //int manageMTDistanceSensor(bool doBegin = false);         // Motor test distance sensor setup                          
    //int manageMTDistanceSwitch(bool doBegin = false);         // Motor test switch setup   
    int manageWetGlueTest(bool doBegin = false);                // Glue test
    int manageWetMisterTest(bool doBegin = false);              // Mister test
  
};

extern wetTestClass wetTest;                        // The one and only wetTest


#endif

