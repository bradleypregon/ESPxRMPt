
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
auto connector = BitsAndDroidsFlightConnector();
ESP32Encoder smallEncoder;
ESP32Encoder bigEncoder;

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
//unsigned long _lastBigIncReadTime = micros();
//unsigned long _lastBigDecReadTime = micros();
//unsigned long _lastSmallIncReadTime = micros();
//unsigned long _lastSmallDecReadTime = micros();
//int _encDelay = 25000;
//int _fastInc = 10;

// Encoder Switch Init
unsigned long _swLastDebounceTime = 0;
long _swDebounceDelay = 50;
int _swButtonState = LOW;

enum Commands {
  setCom1Tx = 0,
  setCom1Rx = 1,
  setCom2Tx = 2,
  setCom2Rx = 3
};

enum ActiveEdit { COM1,
                  COM2,
                  NAV1,
                  NAV2 };
ActiveEdit currEdit = ActiveEdit::COM1;

// Initial freqs
char com1act[comBuff]  = "122.800";
char com1stby[comBuff] = "123.800";
char com2act[comBuff]  = "121.500";
char com2stby[comBuff] = "122.500";

char nav1act[navBuff]  = "109.00";
char nav1stby[navBuff] = "109.50";
char nav2act[navBuff]  = "110.00";
char nav2stby[navBuff] = "110.50";

struct RoundedRectangle {
  int x, y, width, height;
  int command = -1;

  void draw() {
    tft.drawRoundRect(x, y, width, height, 4, TFT_GREY);
  }
};

RoundedRectangle com1ACT = {
  xMargin,
  30,
  160,
  60,
  setCom1Tx
};

RoundedRectangle com1STBY = {
  300,  //com1ACT.x + com1ACT.width + xMargin
  com1ACT.y,
  160,
  60
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
  int color = TFT_GREY;

  void draw() {
    tft.setTextSize(3);
    tft.setTextColor(color, TFT_BLACK, true);
    tft.drawString(label, x, y);
  }
};

/*
  Initial frequency labels
  Com1 Stby has black text with white background

  Freq labels -> x,y
  com1act: 40,40
  com1stby: 320,40
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
  TFT_GREEN
};

FreqLabel com1STBYLbl = {
  320,
  40,
  com1stby,
  TFT_WHITE
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
  // Redraw text labels
  com1ACTLbl.color = TFT_GREEN;
  com2ACTLbl.color = TFT_WHITE;
  com2ACTLbl.draw();
  com1ACTLbl.draw();

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
  // Redraw text labels
  com2ACTLbl.color = TFT_GREEN;
  com1ACTLbl.color = TFT_WHITE;
  com2ACTLbl.draw();
  com1ACTLbl.draw();

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
  // Redraw Freq labels
  com1STBYLbl.color = TFT_WHITE;
  com2STBYLbl.color = TFT_GREY;
  com1STBYLbl.draw();
  com2STBYLbl.draw();

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
  // Redraw Freq labels
  com1STBYLbl.color = TFT_GREY;
  com2STBYLbl.color = TFT_WHITE;
  com1STBYLbl.draw();
  com2STBYLbl.draw();

  Serial.println("Set COM 2 STBY Edit");
}


void drawInitRects() {
  com1ACT.draw();
  com1STBY.draw();
  com2ACT.draw();
  com2STBY.draw();
  nav1ACT.draw();
  nav1STBY.draw();
  nav2ACT.draw();
  nav2STBY.draw();
}

void drawInitFreqs() {
  com1ACTLbl.draw();
  com2ACTLbl.draw();
  com1STBYLbl.draw();
  com2STBYLbl.draw();
  nav1ACTLbl.draw();
  nav2ACTLbl.draw();
  nav1STBYLbl.draw();
  nav2STBYLbl.draw();
}

void drawStaticLabels() {
  tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
  tft.setTextSize(2);
  tft.drawString("ACT", 80, 10);    //((com1ACT.x + com1ACT.width) / 2) - 20, 10
  tft.drawString("STBY", 360, 10);  //((com1STBY.x + com1STBY.width) / 2) - 30, 10
  tft.drawString("COM", 222, 10);
  tft.drawString("NAV", 222, 170);
}

void readSmallEncoder() {
  bool a = digitalRead(smallA);
  bool b = digitalRead(smallB);
  Serial.println((a ^ b) ? "Small INC" : "Small DEC");
  (a^b) ? connector.send(sendCom1FractInc) : connector.send(sendCom1FractDecr);
}

void readBigEncoder() {
  bool a = digitalRead(bigA);
  bool b = digitalRead(bigB);
  Serial.println((a ^ b) ? "Big INC" : "Big DEC");
  (a^b) ? connector.send(sendCom1WholeInc) : connector.send(sendCom1WholeDec);
}

void readEncoderSw() {
  _swButtonState = digitalRead(sw);
  // Filter out noise with timed buffer
  if ((millis() - _swLastDebounceTime) > _swDebounceDelay) { 
    if(_swButtonState == HIGH) { 
      // Switch on activeEdit freq, send swap command
      connector.send(sendSwapCom1);
      connector.send(4000); // fenix id for swap com 1
      _swLastDebounceTime = millis();
    }
  }
}

int smallEncoderPrevCount = 0;
int bigEncoderPrevCount = 0;

// if encoder.getCount() == prevCount, do nothing
// if encoder.getCount() > prevCount, increase
// if encoder.getCount() < prevCount, decrease
void startSmallEncoderListen() { 
  int count = smallEncoder.getCount();
  if (count == smallEncoderPrevCount) { return; }
  if (count > smallEncoderPrevCount) { 
    Serial.println("Increase Fract");
    connector.send(sendCom1FractInc);
    connector.send(4001); 
  } else if (count < smallEncoderPrevCount) { 
    Serial.println("Decrease Fract");
    connector.send(sendCom1FractDecr);
    connector.send(4002);
  }

  smallEncoderPrevCount = count;
}

void startBigEncoderListen() { 
  int count = bigEncoder.getCount();
  if (count == bigEncoderPrevCount) { return; }
  if (count > bigEncoderPrevCount) { 
    Serial.println("Increase Whole");
    connector.send(sendCom1WholeInc);
    connector.send(4003);
  } else if (count < bigEncoderPrevCount) { 
    Serial.println("Decrease Whole");
    connector.send(sendCom1WholeDec);
    connector.send(4004);
  }

  bigEncoderPrevCount = count;
}

void getFreqs() {
  //convertFreq(connector.getActiveCom1(), com1act, sizeof(com1act));
  //convertFreq(connector.getActiveCom2(), com2act, sizeof(com2act));
  //convertFreq(connector.getStandbyCom1(), com1stby, sizeof(com1stby));
  //convertFreq(connector.getStandbyCom2(), com2stby, sizeof(com2stby));

  String temp = connector.getPlaneName();
  temp.toCharArray(com1act, 8);
  com1ACTLbl.draw();
}

void updateCom1(long newAct, long newStby) { 
  if (newAct == newAct || newStby == newStby) { return; }

}

void convertFreq(long freq, char* out, size_t outSize) {
  long whole = freq / 1000;
  long frac = abs(freq % 1000);
  snprintf(out, outSize, "%ld.%03d", whole, frac);
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

  pinMode(sw, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(sw), readEncoderSw, CHANGE);

  ESP32Encoder::useInternalWeakPullResistors = puType::up;
  smallEncoder.attachHalfQuad(25, 26);
  bigEncoder.attachHalfQuad(32, 33);
}

void loop() {
  startTouchGestureRecognizer();
  connector.dataHandling();

  startSmallEncoderListen();
  startBigEncoderListen();
  getFreqs();
}
