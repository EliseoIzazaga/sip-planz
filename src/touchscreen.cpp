//*****************************************************
//  touchscreen.cpp
//  Primitives to paint the screen and get touch coordinates
//*****************************************************
#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_FT6206.h>

#include "Fonts\FreeSansBold12pt7b.h"
#include "siplaminator.h"
#include "touchscreen.h"
#include "Logger.h"
#include "gui.h"

#include "config.h"

// The FT6206 uses hardware I2C (SCL/SDA)
Adafruit_FT6206 ts = Adafruit_FT6206();

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10
#define TS_BACKGROUND_COLOUR    ILI9341_BLACK                                            // A common background

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
struct touchscreen::pageFieldDef_t touchscreen::currentPageDefs[TS_MAX_FIELDS_ON_PAGE];  // The table of commands
uint16_t touchscreen::currentNumFieldsOnPage;                                            // Number of fields in current page
struct touchscreen::pageFieldDef_t* touchscreen::m_touchedField;                         // Last field touched

//*****************************************************
// Indicator definitions.
// These are indicators of the state of io or other operations.
//*****************************************************

struct touchscreen::indicatorFieldDef_t touchscreen::indicatorDefs[] =
{//x    y    Label Val LastVal
  {0,  0,   "DS", 0,  -1},               // Distance Switch
  {0,  10,  "RC", 0,  -1},               // Rail Counter
  {0,  20,  "PC", 0,  -1},               // Pump Counter
  {0,  30,  "MF", 0,  -1},               // Motor Feedback
  {0,  40,  "PF", 0,  -1},               // Power Fail
  {0,  50,  "PR", 0,  -1},               // Press cycle in progress
  {0,  60,  "ES", 0,  -1},               // Emergency Stop

  {0,  80,  "DP", 0,  -1},               // Distance Piston
  {0,  90,  "FM", 0,  -1},               // Fam Mister water
  {0,  100, "BN", 0,  -1},               // Bead Nozzles
  {0,  110, "ST", 0,  -1},               // Sealer Tray for bead nozzles
  {0,  120, "1F", 0,  -1},               // Motor 1 Pump Forward
  {0,  130, "1R", 0,  -1},               // Motor 1 Pump Reverse
  {0,  140, "2F", 0,  -1},               // Motor 2 Rail Forward
  {0,  150, "2R", 0,  -1},               // Motor 2 Rail Reverse
  {0,  160, "3F", 0,  -1},               // Motor 3 Applicator Forward
  {0,  170, "3R", 0,  -1},               // Motor 3 Applicator Reverse
  {0,  180, "BZ", 0,  -1},               // Buzzer
  
  {0,  190, "NA", 0,  -1},               // Network Activity
};


struct touchscreen::analogueIndicatorFieldDef_t analogueIndicatorDefs[] =
{
  {0,  200, "A8", 0,  -1},               // Distance sensor
  {0,  210, "A9", 0,  -1},               // Potentiometer input
  {100,230, "RC", 0,  -1},               // Rail counter
  {0,  230, "PC", 0,  -1},               // Pump counter
  {200,230, "WL", 0,  -1},               // Work Life counter
};



//*****************************************************
// begin
// Initialise stuff.
//*****************************************************
touchscreen::touchscreen()
{
}

void touchscreen::begin()
{
  m_touchedField = NULL;                                              // No fields touched yet

  Serial.print(F("touchscreen::begin:"));
  
  tft.begin();
  
  if (!ts.begin(40)) 
  { 
    Serial.print(F(" Unable to start touchscreen"));
  } 
  
  tft.setRotation(1);                                               // Set to top left closest the USB port
  setBackground();                                                  // Clear the background

  tsc.useCustomFont(true);                                           // Set the font  
  Serial.println();
}

//*****************************************************
//  manageTouchScreen
//  Manage the touchscreen events. Must be called frequently.
//*****************************************************
void touchscreen::manageTouchScreen()
{
  TS_Point p;
  uint16_t x, y;
  pageFieldDef_t* field;
  
  if (ts.touched())                                                   // Test for a touch
  {
    p = ts.getPoint();                                                // Get the touch point coordinates
    x = 320 - p.y;                                                    // Convert from touch to LCD coords 
    y = p.x;
    logger.log(F("manageTouchScreen: Touch at %d,%d -> %d,%d"), p.x, p.y, x, y);
    field = findButton(x, y);                                         // Match the touch to a button
    if (field != NULL)
    {                                                                 // Found one
      m_touchedField = field;                                         // Record the touch for later
      //flashField(field);                                              // Indicate a touch
    }
  }
}


//*****************************************************
//  dirtyIndicators
//  Invalidate the lastValue to force a rewrite of indicators.
//*****************************************************
void touchscreen::dirtyIndicators()
{
  int numIndIcatorFields = getNumIndicatorFields();  // The number of indiators
  indicatorFieldDef_t* indic;
  int numAnalogueIndIcatorFields = getNumAnalogueIndicatorFields();   // The number of analogue indicators
  analogueIndicatorFieldDef_t* aIndic;
  int i;
  
  for (i = 0; i < numIndIcatorFields; i++)
  {
    indic = &indicatorDefs[i];
    indic->lastValue = -1;
  }
  
  for (i = 0; i < numAnalogueIndIcatorFields; i++)
  {
    aIndic = &analogueIndicatorDefs[i];
    aIndic->lastValue = -1L;
  }
}

//*****************************************************
//  setIndicatorValue()
//  Set the value of a digital indicator.
//*****************************************************
void touchscreen::setIndicatorValue(int index, int val)
{
  indicatorDefs[index].value = val;
  //logger.log(F("setIndicatorValue: %d %d"), index, val);
}

//*****************************************************
//  setAnalogueIndicatorValue()
//  Set the value of a digital indicator.
//*****************************************************
//void touchscreen::setAnalogueIndicatorValue(int index, uint16_t val)
void touchscreen::setAnalogueIndicatorValue(int index, int32_t val)
{
  analogueIndicatorDefs[index].value = val;
  //logger.log(F("setIndicatorValue: %d %d"), index, val);
}

//*****************************************************
//  getNumIndicatorFields()
//  Return the number of indicators.
//*****************************************************
int touchscreen::getNumIndicatorFields()
{
  return sizeof(indicatorDefs) / sizeof(indicatorFieldDef_t);  // The number of indiators
}

//*****************************************************
//  getNumAnalogueIndicatorFields()
//  Return the number of indicators.
//*****************************************************
int touchscreen::getNumAnalogueIndicatorFields()
{
  return sizeof(analogueIndicatorDefs) / sizeof(analogueIndicatorFieldDef_t);  // The number of indiators
}

//*****************************************************
//  renderIndicators
//  Draw the indicators but only if they have changed
//*****************************************************
void touchscreen::renderIndicators()
{
  if (!configGetIndicatorsEnabled()) return;

  int numIndIcatorFields = getNumIndicatorFields();                   // The number of indicators
  indicatorFieldDef_t* indic;
  int numAnalogueIndIcatorFields = getNumAnalogueIndicatorFields();   // The number of analogue indicators
  analogueIndicatorFieldDef_t* aIndic;
  int i;
  int width;
  uint16_t fillColour;
  char analogueValueStr[10];

  useCustomFont(false);
  
  for (i = 0; i < numIndIcatorFields; i++)
  {
    indic = &indicatorDefs[i];

    if (indic->value != indic->lastValue)
    {
      //(indic->value == 1) ? fillColour = ILI9341_RED : fillColour = ILI9341_DARKGREY;
      (indic->value == 1) ? fillColour = ILI9341_RED : fillColour = ILI9341_BLACK;
      tft.fillRect(indic->x, indic->y, 20, 9, fillColour);
      //drawText(indic->x + 4, indic->y + 1, 1, ILI9341_BLACK, indic->text);
      drawText(indic->x + 4, indic->y + 1, 1, ILI9341_WHITE, indic->text);
      indic->lastValue = indic->value;
    }
  }

  for (i = 0; i < numAnalogueIndIcatorFields; i++)
  {
    aIndic = &analogueIndicatorDefs[i];

    if (aIndic->value != aIndic->lastValue)
    {
      //(aIndic->value == 1) ? fillColour = ILI9341_RED : fillColour = ILI9341_DARKGREY;
      fillColour = TS_BACKGROUND_COLOUR;
      
      sprintf(analogueValueStr, "%s %ld", aIndic->text, aIndic->value);
      width = aIndic->y > 225 ? 70 : 50;
      tft.fillRect(aIndic->x, aIndic->y, width, 9, fillColour);
      drawText(aIndic->x + 4, aIndic->y + 1, 1, ILI9341_WHITE, analogueValueStr);
      aIndic->lastValue = aIndic->value;
    }
  }
  
  useCustomFont(true);
}


//*****************************************************
//  logIndicators
//  Dump the indicators to the log.
//*****************************************************
void touchscreen::logIndicators()
{
  const char* funcIDStr = PSTR("logIndicators: ");
  
  int numIndIcatorFields = getNumIndicatorFields();                   // The number of indicators
  indicatorFieldDef_t* indic;
  int numAnalogueIndIcatorFields = getNumAnalogueIndicatorFields();   // The number of analogue indicators
  analogueIndicatorFieldDef_t* aIndic;
  int i;

  for (i = 0; i < numIndIcatorFields; i++)
  {
    indic = &indicatorDefs[i];
    logger.log(F("%S%3s %d"), funcIDStr, indic->text, indic->value);
  }

  for (i = 0; i < numAnalogueIndIcatorFields; i++)
  {
    aIndic = &analogueIndicatorDefs[i];
    logger.log(F("%S%3s %d"), funcIDStr, aIndic->text, aIndic->value);
  }
}


//*****************************************************
//  useCustomFont
//  Set the custom font or the system font.
//*****************************************************
void touchscreen::useCustomFont(bool custom)
{
  if (custom)
  {
    tft.setFont(&FreeSansBold12pt7b);                                // Set the custom font
  }
  else
  {
    tft.setFont();                                                    // Set default font
  }
}

//*****************************************************
//  findFieldByID
//  Scan the screen definitions for a field with a matching ID.
//*****************************************************
touchscreen::pageFieldDef_t* touchscreen::findFieldByID(int id)
{
  int i;
  pageFieldDef_t* field;
  pageFieldDef_t* ret = NULL;

  for (i = 0; i < currentNumFieldsOnPage; i++)
  {
    field = &currentPageDefs[i];
    if (field->id == id)
    {
      ret = field;
    }
  }

  return ret;
}

//*****************************************************
//  simulateTouch
//  Simulates a button touch for a given ID.
//*****************************************************
void touchscreen::simulateTouch(int id)
{
  m_touchedField = findFieldByID(id);                               // Pretend to touch the button
}

//*****************************************************
//  findButton
//  Scan the screen definitions for a matching button field.
//*****************************************************
touchscreen::pageFieldDef_t* touchscreen::findButton(uint16_t x, uint16_t y)
{
  int i;
  pageFieldDef_t* field;
  pageFieldDef_t* ret = NULL;

  for (i = 0; i < currentNumFieldsOnPage; i++)
  {
    field = &currentPageDefs[i];
    switch(field->mode)
    {
      case ACTIVE:
        switch(field->type)
        {
          case BUTTON:
            if ((x > field->x) && (x < field->x + field->width) && (y > field->y) && (y < field->y + field->height))
            {
              ret = field;
            }
            break;
        }
        break;
      case NOT_ACTIVE:
      case HIDDEN: 
        break;
    }

    if (ret) 
    {
      logger.log(F("findButton: Touched field %d %S"), ret->id, ret->text);
      break;
    }
  }

  return ret;
}

//*****************************************************
//  flashField
//  Flash the field by erasing and redrawing it.
//*****************************************************
void touchscreen::flashField(pageFieldDef_t* field)
{
  if (field != NULL)
  {
    field->mode = HIDDEN;
    renderField(field);
    field->mode = ACTIVE;
    renderField(field);
  }
}

//*****************************************************
//  isTouchedField
//  Get a ptr to a recent touch field then reset it.
//*****************************************************
touchscreen::pageFieldDef_t* touchscreen::isTouchedField()
{
  pageFieldDef_t* ret = m_touchedField;                                       // Read the touched field
  m_touchedField = NULL;                                                      // Null it so it isn't read twice
  return ret;
}

//*****************************************************
//  fillField
//  Fill a field with background colour
//*****************************************************
void touchscreen::fillField(pageFieldDef_t* field)
{
  tft.fillRect(field->x, field->y, field->width, field->height, TS_BACKGROUND_COLOUR);  
}

//*****************************************************
//  renderField
//  Draw a field from the page table
//*****************************************************
void touchscreen::renderField(pageFieldDef_t* field, char* text)
{
  #define BUFSIZE 32
  char textBuf[BUFSIZE] = "";
  uint16_t backgroundColour = field->bgColour;

  if (text != NULL)
  {
    strcpy(textBuf, text);
  } else
  if (strlen_P(field->text) < (BUFSIZE - 1))
  {
    strcpy_P(textBuf, field->text);
  }
  
  switch(field->mode)
  {
    case NOT_ACTIVE:
      backgroundColour = ILI9341_DARKGREY;
    case ACTIVE:
      switch(field->type)
      {
        case TEXT:
          drawText(field->x, field->y, field->textSize, field->textColour, textBuf);
          break;
        case NATIVE_TEXT:
          useCustomFont(false);
          drawText(field->x, field->y, field->textSize, field->textColour, textBuf);
          useCustomFont(true);
          break;
        case BUTTON:
          drawButton(field->x, field->y, field->width, field->height, backgroundColour,
                           field->borderColour, field->textColour, textBuf, field->textXOS, field->textYOS, field->textSize);
          break;
      }
      
      break;
    case HIDDEN: 
      tft.fillRect(field->x, field->y, field->width, field->height, TS_BACKGROUND_COLOUR);
      break;
  }
}

//*****************************************************
//  renderPage
//  Copy a page table to RAM then render it on screen.
//*****************************************************
void touchscreen::renderPage(const pageFieldDef_t* page, size_t pageSize)
{
  const char* funcIDStr = PSTR("renderPage: ");
  currentNumFieldsOnPage = pageSize / sizeof(pageFieldDef_t);       // Set the number of fields on the page
  pageFieldDef_t* field;
  int i;
  
  m_touchedField = NULL;                                            // No fields touched yet
  setBackground();                                                  // Clear the background

  if (currentNumFieldsOnPage <= TS_MAX_FIELDS_ON_PAGE)
  {
    memcpy_P(currentPageDefs, page, pageSize);

    for (i = 0; i < currentNumFieldsOnPage; i++)
    {
      field = &currentPageDefs[i];
      logger.log(F("%S%d %d %d %d [%S]"), funcIDStr, i,field->id, field->type, field->mode, field->text == NULL ? PSTR("") : field->text);
      renderField(field);                                             // Render a field from the page
    }

    dirtyIndicators();                                                // Make the indicators refresh
  }
  else
  {
    logger.log(F("renderPage: Error too many fields %d"), currentNumFieldsOnPage);
  }
}


//*****************************************************
//  drawButton
//  Draw text on the screen
//*****************************************************

void touchscreen::drawButton(int x, int y, int w, int h, uint16_t bgColour, uint16_t borderColour, 
                             uint16_t textColour, char* text, uint16_t textXOS, uint16_t textYOS, int textSize)
{        
  int textX;
  int textY;
                                                              
  //int textWidth = (strlen(text) * 6 * textSize) - (textSize / 2);     // Center the text in the button
  //int textHeight = (textSize * 8) - (textSize / 2);                   // Chars are 6x8 wide including 1 pixel top right and bottom
  uint16_t bT = (h * 15) / 100;                                       // Border thickness 15% of height 
  uint16_t bT2 = 2 * bT;

  if(textXOS)
  {
    textX = x + textXOS;
    textY = y + textYOS;
  } else
    {
      int textX = x + bT + 18;            
      int textY = y + h - bT - 18;
    }
  
  //logger.log(F("x: %d y: %d textSize: %d textWidth: %d textHeight: %d textX: %d textY: %d"),
  //              x, y, textSize, textWidth, textHeight, textX, textY);
  
  tft.fillRoundRect(x, y, w, h, w/8, borderColour);
  //tft.fillRoundRect(x + 4, y + 4, w - 8, h - 8, w/8, bgColour);
  tft.fillRoundRect(x + bT, y + bT, w - bT2, h - bT2, (w - bT2)/8, bgColour);
  drawText(textX, textY, textSize, textColour, text);
}

/*
void touchscreen::drawButton(int x, int y, int w, int h, uint16_t bgColour,
                             uint16_t borderColour, uint16_t textColour, char* text, int textSize)
{                                                                    
  Adafruit_GFX_Button button;

    // x, y, w, h, outline, fill, text
  button.initButton(&tft, x, y, w, h, borderColour, bgColour, textColour, text, textSize); 
  button.drawButton();
}
*/
//*****************************************************
//  drawText
//  Draw text on the screen
//*****************************************************

void touchscreen::drawText(int x, int y, int size, uint16_t colour, char* text)
{
    if (x >= 0) tft.setCursor(x, y);
    tft.setTextColor(colour);  
    tft.setTextSize(size);
    tft.print(text);
}

//*****************************************************
// lcdTest
// Test the LCD
//*****************************************************

void touchscreen::lcdTest()
{
  Serial.println("ILI9341 Test!"); 
  // read diagnostics (optional but can help debug problems)
  uint8_t x = tft.readcommand8(ILI9341_RDMODE);
  Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDMADCTL);
  Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDPIXFMT);
  Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDIMGFMT);
  Serial.print("Image Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX); 
  
  Serial.println(F("Benchmark                Time (microseconds)"));
  delay(10);
  Serial.print(F("Screen fill              "));
  Serial.println(testFillScreen());
  delay(500);

  Serial.print(F("Text                     "));
  Serial.println(testText());
  delay(3000);

/*
  Serial.print(F("Lines                    "));
  Serial.println(testLines(ILI9341_CYAN));
  delay(500);

  Serial.print(F("Horiz/Vert Lines         "));
  Serial.println(testFastLines(ILI9341_RED, ILI9341_BLUE));
  delay(500);
*/
  Serial.print(F("Rectangles (outline)     "));
  Serial.println(testRects(ILI9341_GREEN));
  delay(500);
/*
  Serial.print(F("Rectangles (filled)      "));
  Serial.println(testFilledRects(ILI9341_YELLOW, ILI9341_MAGENTA));
  delay(500);

  Serial.print(F("Circles (filled)         "));
  Serial.println(testFilledCircles(10, ILI9341_MAGENTA));

  Serial.print(F("Circles (outline)        "));
  Serial.println(testCircles(10, ILI9341_WHITE));
  delay(500);

  Serial.print(F("Triangles (outline)      "));
  Serial.println(testTriangles());
  delay(500);

  Serial.print(F("Triangles (filled)       "));
  Serial.println(testFilledTriangles());
  delay(500);
*/
  Serial.print(F("Rounded rects (outline)  "));
  Serial.println(testRoundRects());
  delay(500);
/*
  Serial.print(F("Rounded rects (filled)   "));
  Serial.println(testFilledRoundRects());
  delay(500);
*/
  Serial.println(F("Done!"));
}

unsigned long touchscreen::setBackground() 
{
  tft.fillScreen(TS_BACKGROUND_COLOUR);
}

unsigned long touchscreen::testFillScreen() 
{
  unsigned long start = micros();
  tft.fillScreen(ILI9341_BLACK);
  yield();
  tft.fillScreen(ILI9341_RED);
  yield();
  tft.fillScreen(ILI9341_GREEN);
  yield();
  tft.fillScreen(ILI9341_BLUE);
  yield();
  tft.fillScreen(ILI9341_BLACK);
  yield();
  return micros() - start;
}


unsigned long touchscreen::testText() 
{
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  unsigned long start = micros();
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(ILI9341_YELLOW); tft.setTextSize(2);
  tft.println(1234.56);
  tft.setTextColor(ILI9341_RED);    tft.setTextSize(3);
  tft.println(0xDEADBEEF, HEX);
  tft.println();
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(5);
  tft.println("Groop");
  tft.setTextSize(2);
  tft.println("I implore thee,");
  tft.setTextSize(1);
  tft.println("my foonting turlingdromes.");
  tft.println("And hooptiously drangle me");
  tft.println("with crinkly bindlewurdles,");
  tft.println("Or I will rend thee");
  tft.println("in the gobberwarts");
  tft.println("with my blurglecruncheon,");
  tft.println("see if I don't!");
  return micros() - start;
}

unsigned long touchscreen::testRects(uint16_t color) 
{
  unsigned long start;
  int           n, i, i2,
                cx = tft.width()  / 2,
                cy = tft.height() / 2;

  tft.fillScreen(ILI9341_BLACK);
  n     = min(tft.width(), tft.height());
  start = micros();
  for(i=2; i<n; i+=6) {
    i2 = i / 2;
    tft.drawRect(cx-i2, cy-i2, i, i, color);
  }

  return micros() - start;
}

unsigned long touchscreen::testRoundRects() 
{
  unsigned long start;
  int           w, i, i2,
                cx = tft.width()  / 2 - 1,
                cy = tft.height() / 2 - 1;

  tft.fillScreen(ILI9341_BLACK);
  w     = min(tft.width(), tft.height());
  start = micros();
  for(i=0; i<w; i+=6) {
    i2 = i / 2;
    tft.drawRoundRect(cx-i2, cy-i2, i, i, i/8, tft.color565(i, 0, 0));
  }

  return micros() - start;
}

touchscreen tsc;                              // The one and only touchscreen

