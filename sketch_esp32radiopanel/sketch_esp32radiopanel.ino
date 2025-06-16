/*
  Libraries, I don't know if the .ino has this data when cloning from Github :-/
  - Bits and Droids flight sim library
  - ESP32Encoder
  - Joystick
  - TFT_eSPI
*/

#include <TFT_eSPI.h>
#include <ESP32Encoder.h>
#include <BitsAndDroidsFlightConnector.h>

TFT_eSPI tft = TFT_eSPI();
BitsAndDroidsFlightConnector connector = BitsAndDroidsFlightConnector();

//#define TFT_GREY 0x5AEB // New colour

/*
Notes
Screen: 480x320
COM1, COM2, NAV1, NAV2, and maybe squawk
Active ACT and Standby STBY freqs
Tapping Standby allows rotary encoder to edit freq
Tapping Transfer button switches ACT and STBY freqs
Tapping Active COM changes airplane Tx to selected

Extra:
  - COM1 and COM2 lookup frequency database and return station name
  - Rx mute buttons for COM freqs

    |-----------------|                   |-----------------|
    |                 |      ------       |                 |
C1  |     123.456     |      | Rx |       |     123.456     |
    |                 |      ------       |                 |
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
  setCom2Rx   = 7
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
  30,
  160,
  60,
  4,
  TFT_GREEN,
  setCom1Tx
};

RoundedRectangle com1STBY = { 
  300, //com1ACT.x + com1ACT.width + xMargin
  com1ACT.y,
  160,
  60,
  4,
  TFT_WHITE,
  setCom1Edit
};

RoundedRectangle com2ACT = { 
  xMargin,
  100, //com1ACT.y + com1ACT.height + yMargin
  160,
  60,
  4,
  TFT_WHITE,
  setCom2Tx
};

RoundedRectangle com2STBY = { 
  300, //com2ACT.x + com2ACT.width + xMargin
  com2ACT.y, //com1ACT.y + com1ACT.height + yMargin
  160,
  60,
  4,
  TFT_WHITE,
  setCom2Edit
};

RoundedRectangle nav1ACT = { 
  xMargin,
  190,
  160,
  50,
  4,
  TFT_WHITE,
  setNav1Edit
};

RoundedRectangle nav1STBY = { 
  300,
  nav1ACT.y,
  160,
  50,
  4,
  TFT_WHITE,
  setNav1Edit
};

RoundedRectangle nav2ACT = { 
  xMargin,
  250,
  160,
  50,
  4,
  TFT_WHITE,
  setNav2Edit
};

RoundedRectangle nav2STBY = { 
  300,
  nav2ACT.y,
  160,
  50,
  4,
  TFT_WHITE,
  setNav2Edit
};



void startTouchGestureRecognizer() { 
  if(tft.getTouch(&tX, &tY)) { 
    tY = 0 + (320 - tY);
    // if touch is between (button.x) and (button.x + width) and (button.y) and (button.y + height)
    // Com ACT
    if((tX > com1ACT.x && tX < com1ACT.x + com1ACT.width) && (tY > com1ACT.y && tY < com1ACT.y + com1ACT.height)) { 
      handleCom1ACTTouched(com1ACT);
    }
    if((tX > com2ACT.x && tX < com2ACT.x + com2ACT.width) && (tY > com2ACT.y && tY < com2ACT.y + com2ACT.height)) { 
      handleCom2ACTTouched(com2ACT);
    }
    // Com STBY
    if((tX > com1STBY.x && tX < com1STBY.x + com1STBY.width) && (tY > com1STBY.y && tY < com1STBY.y + com1STBY.height)) { 
      handleStbyTouched(com1STBY);
    }
    if((tX > com2STBY.x && tX < com2STBY.x + com2STBY.width) && (tY > com2STBY.y && tY < com2STBY.y + com2STBY.height)) { 
      handleStbyTouched(com2STBY);
    }

    // Nav STBY
    if((tX > nav1STBY.x && tX < nav1STBY.x + nav1STBY.width) && (tY > nav1STBY.y && tY < nav1STBY.y + nav1STBY.height)) { 
      handleStbyTouched(nav1STBY);
    }
    if((tX > nav2STBY.x && tX < nav2STBY.x + nav2STBY.width) && (tY > nav2STBY.y && tY < nav2STBY.y + nav2STBY.height)) { 
      handleStbyTouched(nav2STBY);
    }
  }
}

/*
  Handle COM 1 ACT being touched
    - Makes COM 1 ACT RoundedRectangle green
    - Makes COM 2 ACT RoundedRectangle black
  Params: rect: RoundedRectangle
*/
void handleCom1ACTTouched(RoundedRectangle rect) { 
  //RoundedRectangle temp = rect;
  //temp.color = TFT_GREEN;
  //drawFillRoundedRect(temp);

  rect.color = TFT_GREEN;
  drawFillRoundedRect(rect);

  // Reset Com 2 ACT
  RoundedRectangle temp = com2ACT;
  temp.color = TFT_BLACK;
  drawFillRoundedRect(temp);
  temp.color = TFT_WHITE;
  drawOutlineRoundedRect(temp);

  Serial.println("Set COM 1 ACT Tx");
}

void handleCom2ACTTouched(RoundedRectangle rect) { 
  //RoundedRectangle temp = rect;
  //temp.color = TFT_GREEN;
  //drawFillRoundedRect(temp);
  rect.color = TFT_GREEN;
  drawFillRoundedRect(rect);

  // reset Com 1 ACT
  RoundedRectangle temp = com1ACT;
  temp.color = TFT_BLACK;
  drawFillRoundedRect(temp);
  temp.color = TFT_WHITE;
  drawOutlineRoundedRect(temp);

  Serial.println("Set COM 2 ACT Tx");
}


void handleStbyTouched(RoundedRectangle rect) { 
  // Fill rect with white, make text black
  // Fill other rects black, make text white

}

void drawFillRoundedRect(RoundedRectangle rect) { 
  tft.fillRoundRect(rect.x, rect.y, rect.width, rect.height, rect.cornerRadius, rect.color);
}

void drawOutlineRoundedRect(RoundedRectangle rect) { 
  tft.drawRoundRect(rect.x, rect.y, rect.width, rect.height, rect.cornerRadius, rect.color);
}

void drawStaticLabels() { 
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("ACT", 80, 10); //((com1ACT.x + com1ACT.width) / 2) - 20, 10
  tft.drawString("STBY", 360, 10); //((com1STBY.x + com1STBY.width) / 2) - 30, 10
  tft.drawString("COM", 222, 10);
  tft.drawString("NAV", 222, 170);
}

/*
  Freq labels -> x,y
  com1act: 40,40
  com1stby: 320,40
  com2act: 40,120
  com2stby: 320,120

  nav1act: 50,200
  nav1stby: 330,200
  nav2act: 50,260
  nav2stby: 330,260
*/ 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  tft.setRotation(1);
  tft.init();

  tft.fillScreen(TFT_BLACK);

  drawFillRoundedRect(com1ACT);
  drawFillRoundedRect(com1STBY);

  drawOutlineRoundedRect(com2ACT);
  drawOutlineRoundedRect(com2STBY);

  drawOutlineRoundedRect(nav1ACT);
  drawOutlineRoundedRect(nav1STBY);
  drawOutlineRoundedRect(nav2ACT);
  drawOutlineRoundedRect(nav2STBY);

  drawStaticLabels();
}

void loop() {
  startTouchGestureRecognizer();
}
