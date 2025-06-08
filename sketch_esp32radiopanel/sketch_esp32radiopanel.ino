#include <TFT_eSPI.h>
#include <ESP32Encoder.h>
#include <BitsAndDroidsFlightConnector.h>

TFT_eSPI tft = TFT_eSPI();
BitsAndDroidsFlightConnector connector = BitsAndDroidsFlightConnector();

//#define TFT_GREY 0x5AEB // New colour

/*
COM1, COM2, NAV1, NAV2, and maybe squawk
Active ACT and Standby STBY freqs
Tapping Standby allows rotary encoder to edit freq
Tapping Transfer button switches ACT and STBY freqs
Tapping Active COM changes airplane Tx to selected

Extra:
  - COM1 and COM2 lookup frequency database and return station name
  - Rx mute buttons for COM freqs

    |-----------------|                   |-----------------|
    |                 |                   |                 |
C1  |     123.456     |    Rx    Swap     |     123.456     |
    |                 |                   |                 |
    |-----------------|                   |-----------------|
*/

boolean isSelected = false;

uint16_t tX = 0, tY = 0;
byte xMargin = 20;
byte yMargin = 20;

enum CustomCommands { 
  setCom1Edit = 0,
  setCom2Edit = 1,
  setNav1Edit = 2,
  setNav2Edit = 3,

  setCom1Tx   = 4,
  setCom1Rx   = 5,
  setCom2Tx   = 6,
  setCom3Rx   = 7
};

struct RoundedRectangle { 
  int x;
  int y;
  int width;
  int height;
  int cornerRadius;
  int color;
  int command;
};

RoundedRectangle com1ACT = { 
  xMargin,
  yMargin,
  150,
  75,
  4,
  TFT_WHITE,
  sendSwapCom1
};

RoundedRectangle com1STBY = { 
  com1ACT.x + com1ACT.width + xMargin,
  yMargin,
  150,
  75,
  4,
  TFT_WHITE,
  setCom1Edit
};

RoundedRectangle com2ACT = { 
  xMargin,
  com1ACT.y + com1ACT.height + yMargin,
  150,
  75,
  4,
  TFT_WHITE,
  sendSwapCom2
};

RoundedRectangle com2STBY = { 
  com2ACT.x + com2ACT.width + xMargin,
  com1ACT.y + com1ACT.height + yMargin,
  150,
  75,
  4,
  TFT_WHITE,
  setCom2Edit
};

void startTouchGestureRecognizer() { 
  if(tft.getTouch(&tX, &tY)) { 
    tY = 0 + (320 - tY);
    // if touch is between (button.x) and (button.x + width) and (button.y) and (button.y + height)
    // Com ACT
    if((tX > com1ACT.x && tX < com1ACT.x + com1ACT.width) && (tY > com1ACT.y && tY < com1ACT.y + com1ACT.height)) { 
      handleComActTouched(com1ACT);
    }
    if((tX > com2ACT.x && tX < com2ACT.x + com2ACT.width) && (tY > com2ACT.y && tY < com2ACT.y + com2ACT.height)) { 
      handleComActTouched(com2ACT);
    }
    // Com STBY
    if((tX > com1STBY.x && tX < com1STBY.x + com1STBY.width) && (tY > com1STBY.y && tY < com1STBY.y + com1STBY.height)) { 
      handlStbyTouched(com1STBY);
    }
    if((tX > com2STBY.x && tX < com2STBY.x + com2STBY.width) && (tY > com2STBY.y && tY < com2STBY.y + com2STBY.height)) { 
      handleStbyTouched(com2STBY);
    }
    // Nav ACT
    
  }
}

/*
  Handle COM ACT being touched
  Draw a filled white rect, perform action
  After 500ms, call resetComACT
  Params: rect: RoundedRectangle
*/
void handleComActTouched(RoundedRectangle rect) { 
  RoundedRectangle temp = rect;
  temp.color = TFT_WHITE;
  drawFillRoundedRect(temp);
  Serial.println("Swap Com");
  delay(500);
  resetComACT(rect);
}

/*
  Reset ComACT Rect
  Draw a black square with a white outline
  Params: RoundedRectangle
*/
void resetComACT(RoundedRectangle rect) { 
  RoundedRectangle temp = rect;
  temp.color = TFT_BLACK; 
  drawFillRoundedRect(temp);
  drawOutlineRoundedRect(rect);
}


void handleStbyTouched(RoundedRectangle rect) { 
  // one square filled white, others outline white
}

void drawFillRoundedRect(RoundedRectangle rect) { 
  tft.fillRoundRect(rect.x, rect.y, rect.width, rect.height, rect.cornerRadius, rect.color);
}

void drawOutlineRoundedRect(RoundedRectangle rect) { 
  tft.drawRoundRect(rect.x, rect.y, rect.width, rect.height, rect.cornerRadius, rect.color);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  tft.setRotation(1);
  tft.init();

  tft.fillScreen(TFT_BLACK);
  drawOutlineRoundedRect(com1ACT);
  drawFillRoundedRect(com1STBY);
  drawOutlineRoundedRect(com2ACT);
  drawOutlineRoundedRect(com2STBY);
}

void loop() {
  startTouchGestureRecognizer();
}

// old stuff
/*
float com1ACT = 122.800;
float com1STBY = 121.900;
float com2ACT = 121.500;
float com2STBY = 122.800;
float nav1ACT = 109.10;
float nav1STBY = 110.10;
float nav2ACT = 110.45;
float nav2STBY = 109.45;

void testSetupStuff() { 
  tft.fillScreen(TFT_BLACK); 
  tft.setTextColor(TFT_WHITE);
  tft.setTextFont(4);
  tft.setCursor(50, 0);
  tft.println("ACT");
  tft.setCursor(100, 0);
  tft.println("STBY");
}

void testLoopStuff() { 
  // Set "cursor" at top left corner of display (0,0) and select font 2
  // (cursor will move to next line automatically during printing with 'tft.println'
  //  or stay on the line is there is room for the text with tft.print)
  tft.setCursor(0, 0);
  tft.setTextFont(4);
  tft.setTextSize(2);
  tft.println("COM1");
  tft.setTextSize(3);
  char buffer[10];
  sprintf(buffer, "%.3f", com1ACT);
  tft.println(buffer);
  tft.setTextSize(2);
  tft.println("COM2");
  tft.setTextSize(3);
  tft.println(com2ACT);
  tft.setTextSize(2);
  tft.println("NAV1");
  tft.setTextSize(3);
  tft.println(nav1ACT);
  tft.setTextSize(2);
  tft.println("NAV2");
  tft.setTextSize(3);
  tft.println(nav2ACT);
}
*/
