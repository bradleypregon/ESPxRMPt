/*
  Libraries, I don't know if the .ino has this data when cloning from Github :-/
  - Bits and Droids flight sim library
  - ESP32Encoder
  - Joystick
  - bodmer TFT_eSPI
*/

/*
  TODO:
    1. Implement the rest of frequency label inputs from connector
    2. Fix rotary encoder -> https://github.com/mo-thunderz/RotaryEncoder/blob/main/Arduino/ArduinoRotaryEncoder/ArduinoRotaryEncoder.ino
      - Debounce button: https://www.programmingelectronics.com/debouncing-a-button-with-arduino/#:~:text=Step%2Dby%2DStep%20Debounce%20Instructions&text=Connect%20the%20220%2Dohm%20resistor,other%20side%20of%20the%20pushbutton.
    3. Fix text label color on tap of rectangle
    4. Refactor code so it is reusable
      - use pointers?
    5. Make FreqLabel struct with char[] label, x: int, y: int
      - make function DrawLabel with params Label: FreqLabel, Color: Int
    6. Make radio swap buttons
    7. General optimizations
*/

/*
Notes
Screen: 480x320
COM1, COM2, NAV1, NAV2
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

/*
  Dual Encoder GPIO Pins
  Small knob:
    - A (cw)  -> GPIO25/D25 Pin 14
    - B (ccw) -> GPIO26/D26 Pin 15
  Big knob:
    - a (cw)  -> GPIO32/D32 Pin 12
    - b (ccw) -> GPIO33/D33 Pin 13
  Switch:
    - switch  -> GPIO27/D27 Pin 16
*/

#include <TFT_eSPI.h>
#include <ESP32Encoder.h>
#include <BitsAndDroidsFlightConnector.h>

TFT_eSPI tft = TFT_eSPI();
BitsAndDroidsFlightConnector connector = BitsAndDroidsFlightConnector();

#define TFT_GREY 0x5AEB
#define bigA 32
#define bigB 33
#define smallA 25
#define smallB 26
#define sw 27
#define comBuff 8
#define navBuff 7

uint16_t tX = 0, tY = 0;
byte xMargin = 20;
byte yMargin = 20;

// Encoder Inits
unsigned long _lastBigIncReadTime = micros();
unsigned long _lastBigDecReadTime = micros();
unsigned long _lastSmallIncReadTime = micros();
unsigned long _lastSmallDecReadTime = micros();
int _encDelay = 25000;
int _fastInc = 10;

// Encoder Switch Init
unsigned long _lastSwReadTime = micros();
long _swDelay = 100;

enum Commands {
  setCom1Tx = 0,
  setCom1Rx = 1,
  setCom2Tx = 2,
  setCom2Rx = 3
};

enum ActiveEdit { COM1, COM2, NAV1, NAV2 };
ActiveEdit currEdit = ActiveEdit::COM1;

// Initial freqs
char com1act[8] = "122.800";
char com1stby[8] = "122.800";
char com2act[8] = "121.500";
char com2stby[8] = "121.500";

char nav1act[7] = "109.00";
char nav1stby[7] = "109.00";
char nav2act[7] = "110.00";
char nav2stby[7] = "110.00";

struct RoundedRectangle {
  int x, y, width, height;
  int color = TFT_BLACK;
  int command = -1;
  void draw() {
    tft.fillRoundRect(x, y, width, height, 4, color);
    tft.drawRoundRect(x, y, width, height, 4, TFT_GREY);
  }
};

RoundedRectangle com1ACT = {
  xMargin,
  30,
  160,
  60,
  TFT_GREEN,
  setCom1Tx
};

RoundedRectangle com1STBY = {
  300,  //com1ACT.x + com1ACT.width + xMargin
  com1ACT.y,
  160,
  60,
  TFT_WHITE
};

RoundedRectangle com2ACT = {
  xMargin,
  100,  //com1ACT.y + com1ACT.height + yMargin
  160,
  60,
  setCom2Tx
};

RoundedRectangle com2STBY = {
  300,        //com2ACT.x + com2ACT.width + xMargin
  com2ACT.y,  //com1ACT.y + com1ACT.height + yMargin
  160,
  60
};

RoundedRectangle nav1ACT = {
  xMargin,
  190,
  160,
  50
};

RoundedRectangle nav1STBY = {
  300,
  nav1ACT.y,
  160,
  50
};

RoundedRectangle nav2ACT = {
  xMargin,
  250,
  160,
  50
};

RoundedRectangle nav2STBY = {
  300,
  nav2ACT.y,
  160,
  50
};

struct FreqLabel {
  int x;
  int y;
  char* label;
  int fgColor = TFT_WHITE;
  void draw { 
    
  }
};

/*
  Draw initial frequency labels
  Com1 Stby has black text with white background

  Freq labels -> x,y
  com1act: 40,40
    - green background
    - white text
  com1stby: 320,40
    - white bg
    - black text
  com2act: 40,120
  com2stby: 320,120

  nav1act: 50,200
  nav1stby: 330,200
  nav2act: 50,260
  nav2stby: 330,260
*/

FreqLabel com1ACTLbl = {
  40,
  40,
  com1act,
  TFT_GREEN,
  TFT_WHITE
};

FreqLabel com1STBYLbl = {
  320,
  40,
  com1stby,
  TFT_WHITE,
  TFT_BLACK
};

FreqLabel com2ACTLbl = {
  40,
  120,
  com2act
};

FreqLabel com2STBYLbl = {
  320,
  120,
  com2stby
};

FreqLabel nav1ACTLbl = {
  50,
  200,
  nav1act
};

FreqLabel nav1STBYLbl = {
  330,
  200,
  nav1stby
};

FreqLabel nav2ACTLbl = {
  50,
  260,
  nav2act
};

FreqLabel nav2STBYLbl = {
  330,
  260,
  nav2stby
};

void startTouchGestureRecognizer() {
  if (tft.getTouch(&tX, &tY)) {
    tY = 0 + (320 - tY);
    // if touch is between (button.x) and (button.x + width) and (button.y) and (button.y + height)
    // Com ACT
    if ((tX > com1ACT.x && tX < com1ACT.x + com1ACT.width) && (tY > com1ACT.y && tY < com1ACT.y + com1ACT.height)) {
      handleCom1ACTTouched(com1ACT);
    }
    if ((tX > com2ACT.x && tX < com2ACT.x + com2ACT.width) && (tY > com2ACT.y && tY < com2ACT.y + com2ACT.height)) {
      handleCom2ACTTouched(com2ACT);
    }

    // Com STBY
    if ((tX > com1STBY.x && tX < com1STBY.x + com1STBY.width) && (tY > com1STBY.y && tY < com1STBY.y + com1STBY.height)) {
      handleCom1STBYTouched(com1STBY);
    }
    if ((tX > com2STBY.x && tX < com2STBY.x + com2STBY.width) && (tY > com2STBY.y && tY < com2STBY.y + com2STBY.height)) {
      handleCom2STBYTouched(com2STBY);
    }

    // Nav STBY
    if ((tX > nav1STBY.x && tX < nav1STBY.x + nav1STBY.width) && (tY > nav1STBY.y && tY < nav1STBY.y + nav1STBY.height)) {
      //handleNav1STBYTouched(nav1STBY);
    }
    if ((tX > nav2STBY.x && tX < nav2STBY.x + nav2STBY.width) && (tY > nav2STBY.y && tY < nav2STBY.y + nav2STBY.height)) {
      //handleNav2STBYTouched(nav2STBY);
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
  // Set COM1 ACT Rect Green
  rect.color = TFT_GREEN;
  drawFillRoundedRect(rect);

  // Reset Com 2 ACT
  RoundedRectangle temp = com2ACT;
  temp.color = TFT_BLACK;
  drawFillRoundedRect(temp);
  drawOutlineRoundedRect(temp);

  // Redraw text labels

  // Set COM 1 Tx
  Serial.println("Set COM 1 ACT Tx");
}

/*
  Handle COM 2 ACT being touched
    - Makes COM 2 ACT RoundedRectangle green
    - Makes COM 1 ACT RoundedRectangle black
  Params: rect: RoundedRectangle
*/
void handleCom2ACTTouched(RoundedRectangle rect) {
  // Set COM2 ACT Rect Green
  rect.color = TFT_GREEN;
  drawFillRoundedRect(rect);

  // Reset Com 1 ACT
  RoundedRectangle temp = com1ACT;
  temp.color = TFT_BLACK;
  drawFillRoundedRect(temp);
  drawOutlineRoundedRect(temp);

  // Redraw text labels

  // Set COM 2 Tx
  Serial.println("Set COM 2 ACT Tx");
}

/*
  Handle COM 1 STBY being touched
    - Makes COM 1 STBY RoundedRectangle white
    - Makes COM 2 STBY RoundedRectangle black
    - Set COM 1 STBY Text Black
    - Set COM 2 STBY Text White
  Params: rect: RoundedRectangle
*/
void handleCom1STBYTouched(RoundedRectangle rect) {
  // Set COM1 STBY Rect White
  rect.color = TFT_WHITE;
  drawFillRoundedRect(rect);

  // Reset Com 2 STBY
  RoundedRectangle temp = com2STBY;
  temp.color = TFT_BLACK;
  drawFillRoundedRect(temp);
  drawOutlineRoundedRect(temp);

  // Redraw Freq labels
  // Com2
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(com2stby, 320, 120);
  // Com1
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.drawString(com1stby, 320, 40);

  Serial.println("Set COM 1 STBY Edit");
}

/*
  Handle COM 2 STBY being touched
    - Makes COM 2 STBY RoundedRectangle white
    - Makes COM 1 STBY RoundedRectangle black
    - Set COM 2 STBY Text Black
    - Set COM 1 STBY Text White
  Params: rect: RoundedRectangle
*/
void handleCom2STBYTouched(RoundedRectangle rect) {
  // Set COM2 STBY Rect White
  rect.color = TFT_WHITE;
  drawFillRoundedRect(rect);

  // Reset Com 1 STBY
  RoundedRectangle temp = com1STBY;
  temp.color = TFT_BLACK;
  drawFillRoundedRect(temp);
  drawOutlineRoundedRect(temp);

  // Redraw Freq labels
  // Com1
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(com1stby, 320, 40);
  // Com2
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.drawString(com2stby, 320, 120);

  Serial.println("Set COM 2 STBY Edit");
}


void drawInitRects() {
  com1ACT.draw();
  com1STBY.draw();
  com2Act.draw();
  com2Stby.draw();
  nav1Act.draw();
  nav1Stby.draw();
  nav2Act.draw();
  nav2Stby.draw();
}

void drawInitFreqs() {
  drawFreqLabel(com1ACTLbl);
  drawFreqLabel(com1STBYLbl);
  drawFreqLabel(com2ACTLbl);
  drawFreqLabel(com2STBYLbl);
  drawFreqLabel(nav1ACTLbl);
  drawFreqLabel(nav1STBYLbl);
  drawFreqLabel(nav2ACTLbl);
  drawFreqLabel(nav2STBYLbl);
}

void drawFreqLabel(FreqLabel lbl) {
  tft.setTextSize(3);
  tft.setTextColor(lbl.fgColor, lbl.bgColor);
  tft.drawString(lbl.label, lbl.x, lbl.y);
}

void drawStaticLabels() {
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("ACT", 80, 10);    //((com1ACT.x + com1ACT.width) / 2) - 20, 10
  tft.drawString("STBY", 360, 10);  //((com1STBY.x + com1STBY.width) / 2) - 30, 10
  tft.drawString("COM", 222, 10);
  tft.drawString("NAV", 222, 170);
}

// Deprecate
void drawFreqLabels() {
  tft.setTextSize(3);

  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.drawString(com1stby, 320, 40);

  tft.setTextColor(TFT_WHITE, TFT_GREEN);
  tft.drawString(com1act, 40, 40);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(com2act, 40, 120);
  tft.drawString(com2stby, 320, 120);
  tft.drawString(nav1act, 50, 200);
  tft.drawString(nav1stby, 330, 200);
  tft.drawString(nav2act, 50, 260);
  tft.drawString(nav2stby, 330, 260);
}

void readSmallEncoder() {
  bool a = digitalRead(smallA);
  bool b = digitalRead(smallB);
  Serial.println((a ^ b) ? "Small INC" : "Small DEC");
}

void readBigEncoder() {
  bool a = digitalRead(bigA);
  bool b = digitalRead(bigB);
  Serial.println((a ^ b) ? "Big INC" : "Big DEC");
}

/*
  Swap Freqs for selected radio
  e.g. If Com 1 Stby is active -> swap Com 1 radios
  Switch currEdit<ActiveEdit>
*/
void readEncoderSw() {
  static uint32_t last = 0;
  uint32_t now = millis();
  if (now - last > 15) {
    Serial.println("Switch Button Pressed");
  }
  last = now;
}

void getFreqs() {
  long temp = connector.getActiveCom1();
  long whole = temp / 1000;
  long frac = abs(temp % 1000);

  char buf[8];
  snprintf(buf, sizeof(buf), "%ld.%03d", whole, frac);

  tft.drawString(buf, 40, 40);
}

void setup() {
  Serial.begin(115200);
  tft.setRotation(1);
  tft.init();
  tft.fillScreen(TFT_BLACK);

  // Init draw
  drawStaticLabels();
  drawInitRects();
  drawInitFreqs();

  // Encoder Setup
  pinMode(smallA, INPUT_PULLUP);
  pinMode(smallB, INPUT_PULLUP);
  pinMode(bigA, INPUT_PULLUP);
  pinMode(bigB, INPUT_PULLUP);
  pinMode(sw, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(smallA), readSmallEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(smallB), readSmallEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(bigA), readBigEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(bigB), readBigEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(sw), readEncoderSw, CHANGE);
}

void loop() {
  startTouchGestureRecognizer();

  connector.dataHandling();
  getFreqs();
}
