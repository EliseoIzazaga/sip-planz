//*****************************************************
//  touchscreen.h
//  Primitives to paint the screen and get touch coordinates
//*****************************************************
#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

#define TS_MAX_FIELDS_ON_PAGE         10          // Maximum number of fields on a page    

class touchscreen
{
  public:
    touchscreen();
    
    void begin();                                 // Initialise
    void lcdTest();                               // Test the LCD
    unsigned long testFillScreen();
    unsigned long testText(); 
    unsigned long testRects(uint16_t color); 
    unsigned long testRoundRects();
    unsigned long setBackground();
    void drawText(int x, int y, int size, uint16_t colour, char* text);

    enum mode_t                                                           // Mode of the field
    {
      ACTIVE,                                                             // Button active and responding to touch
      NOT_ACTIVE,                                                         // Button inactive does not respond to touch
      HIDDEN                                                              // Button not shown and not active
    };

    enum type_t                                                           // What type of field is it
    {
      TEXT,                                                               // Text
      NATIVE_TEXT,                                                        // Default text font
      BUTTON                                                              // Button
    };

    enum indicatorIndex_t                                                 // Indexes into the indicatorDefs[]
    {                                                                     // These must stay in sync with the table
      IND_DISTANCE = 0,                                                   // Inputs
      IND_RAIL_COUNTER,
      IND_PUMP_COUNTER,
      IND_MOTOR_FEEDBACK,
      IND_POWER_FAIL,
      IND_PRESS_CYCLE,
      IND_EMERGENCY_STOP,
      
      IND_DISTANCE_PISTON,                                                // Outputs
      IND_FAN_MISTER,
      IND_BEAD_NOZZLES,
      IND_SEALER_TRAY,
      IND_MOTOR_1_PUMP_FORWARD,
      IND_MOTOR_1_PUMP_REVERSE,
      IND_MOTOR_2_RAIL_FORWARD,
      IND_MOTOR_2_RAIL_REVERSE,
      IND_MOTOR_3_APPLICATOR_FORWARD,
      IND_MOTOR_3_APPLICATOR_REVERSE,
      IND_BUZZER,
      
      IND_NETWORK_ACTIVITY                                                // Other
    };

    enum analogueIndicatorIndex_t                                         // Indexes into the analogueIndicatorDefs[]
    {                                                                     // These must stay in sync with the table
      AIND_POSITION = 0,                                                  // Inputs
      AIND_POTENTIOMETER, 
      AIND_RAIL_COUNTER, 
      AIND_PUMP_COUNTER, 
      AIND_WORK_LIFE 
    };
    

    
  private:
    /*
    enum textAlign_t                              // Aligning text
    {
      LEFT,
      RIGHT,
      CENTER
    };
    */
    

    void drawButton(int x, int y, int w, int h, uint16_t bgColour, uint16_t borderColour,
                    uint16_t textColour, char* text, uint16_t textXOS, uint16_t textYOS, int textSize);

    public:
      struct pageFieldDef_t                                                 // Page field definition
      {
        int16_t id;                                                         // An id unique on the page
        type_t type;                                                        // Text or button
        uint16_t x;                                                         // x left pixel position
        uint16_t y;                                                         // y top pixel position
        uint16_t width;                                                     // Width in pixels
        uint16_t height;                                                    // height in pixels
        uint16_t bgColour;                                                  // Background colour
        uint16_t borderColour;                                              // Border colour
        uint16_t textColour;                                                // Text colour
        uint8_t textSize;                                                   // Size 1-5
        char* text;                                                         // Text for the field
        uint16_t textXOS;                                                   // Text X offset from x
        uint16_t textYOS;                                                   // Text Y offset from y
        mode_t mode;                                                        // Active, inactive, hidden
      };

      struct indicatorFieldDef_t                                            // Define an indicator field
      {
        uint16_t x;                                                         // x left pixel position
        uint16_t y;                                                         // y top pixel position
        char text[3];                                                       // 2 characters only
        int8_t value;                                                       // Limited values on, off etc.
        int8_t lastValue;                                                   // Only render when changed
      };


      struct analogueIndicatorFieldDef_t                                    // Define an analogue indicator field
      {
        uint16_t x;                                                         // x left pixel position
        uint16_t y;                                                         // y top pixel position
        char text[3];                                                       // 2 characters only
        //int16_t value;                                                      // Analogue value.
        //int16_t lastValue;                                                  // Only render when changed
        int32_t value;                                                      // Analogue value.
        int32_t lastValue;                                                  // Only render when changed
      };
    
      void renderPage(const pageFieldDef_t* page, size_t pageSize);         // Render a page from a page def table
      void renderField(pageFieldDef_t* field, char* text = NULL);           // Render a single filed from a page def table
      void renderIndicators();                                              // Update indicators
      void manageTouchScreen();                                             // Manager called frequently
      void setIndicatorValue(int index, int val);                           // Set indicator value
      //void setAnalogueIndicatorValue(int index, uint16_t val);              // Set an analogue indicator value
      void setAnalogueIndicatorValue(int index, int32_t val);              // Set an analogue indicator value
      pageFieldDef_t* isTouchedField();                                     // Read the touched field
      pageFieldDef_t* findFieldByID(int id);                                // Find a field by ID
      void fillField(pageFieldDef_t* field);                                // Fill field with the bckground colour
      void dirtyIndicators();                                               // Make indicators refresh
      void simulateTouch(int id);                                           // Pretend to touch a button
      void logIndicators();                                                 // Dump inputs & outputs

    
    private:
      static struct pageFieldDef_t currentPageDefs[];                       // The table of commands
      static struct indicatorFieldDef_t indicatorDefs[];                    // Indicator fields
      static uint16_t currentNumFieldsOnPage;                               // Number of fields in current page
      static struct pageFieldDef_t* m_touchedField;                         // Last field touched
      
      pageFieldDef_t* findButton(uint16_t x, uint16_t y);                   // Locate a button
      void flashField(pageFieldDef_t* field);                               // Flash aa field to show touch hit
      void useCustomFont(bool custom = false);                              // true = custom font, false = system font 
      int getNumIndicatorFields();                                          // Return number of indicators
      int getNumAnalogueIndicatorFields();                                  // Return number of analogue indicators
      
};

extern touchscreen tsc;                                                     // The one and only touchscreen

#endif
