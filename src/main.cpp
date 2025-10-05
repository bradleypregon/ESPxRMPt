#include <main.h>

uint16_t tX = 0, tY = 0;
byte xMargin = 20;
byte yMargin = 20;

// Encoder Switch Init
unsigned long _swLastDebounceTime = 0;
long _swDebounceDelay = 50;
int _swButtonState = LOW;

ActiveEdit currEdit = ActiveEdit::COM1;

std::unordered_map<std::string, FreqPair> freqs = {
    {"COM1", {122800, 121500}},
    {"COM2", {123800, 122500}},
    {"NAV1", {10900, 10950}},
    {"NAV2", {11000, 11050}},
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

FreqLabel com1ACTLbl = {
  40,
  40,
  freqs["COM1"].active,
  TFT_GREEN
};

FreqLabel com1STBYLbl = {
  320,
  40,
  freqs["COM1"].standby,
  TFT_WHITE
};

FreqLabel com2ACTLbl = {
  40,
  120,
  freqs["COM2"].active,
};

FreqLabel com2STBYLbl = {
  320,
  120,
  freqs["COM2"].standby,
};

FreqLabel nav1ACTLbl = {
  50,
  200,
  freqs["NAV1"].active
};

FreqLabel nav1STBYLbl = {
  330,
  200,
  freqs["NAV1"].standby
};

FreqLabel nav2ACTLbl = {
  50,
  260,
  freqs["NAV2"].active
};

FreqLabel nav2STBYLbl = {
  330,
  260,
  freqs["NAV2"].standby
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

//void readSmallEncoder() {
//  bool a = digitalRead(smallA);
//  bool b = digitalRead(smallB);
//  Serial.println((a ^ b) ? "Small INC" : "Small DEC");
//  (a^b) ? connector.send(sendCom1FractInc) : connector.send(sendCom1FractDecr);
//}

//void readBigEncoder() {
//  bool a = digitalRead(bigA);
//  bool b = digitalRead(bigB);
//  Serial.println((a ^ b) ? "Big INC" : "Big DEC");
//  (a^b) ? connector.send(sendCom1WholeInc) : connector.send(sendCom1WholeDec);
//}

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
  updateFreq("COM1", FreqPair{connector.getActiveCom1(), connector.getStandbyCom1()});
  updateFreq("COM2", FreqPair{connector.getActiveCom2(), connector.getStandbyCom2()});
  updateFreq("NAV1", FreqPair{connector.getActiveNav1(), connector.getStandbyNav1()});
  updateFreq("NAV2", FreqPair{connector.getActiveNav2(), connector.getStandbyNav2()});
}

void updateFreq(std::string radio, FreqPair newFreqs) { 
  if (freqs[radio].active == newFreqs.active && freqs[radio].standby == newFreqs.standby) { return; }
  freqs[radio] = newFreqs;

  if (radio == "COM1") { 
    com1ACTLbl.draw();
    com1STBYLbl.draw();
  } else if (radio == "COM2") { 
    com2ACTLbl.draw();
    com2STBYLbl.draw();
  } else if (radio == "NAV1") { 
    nav1ACTLbl.draw();
    nav1STBYLbl.draw();
  } else if (radio == "NAV2") { 
    nav2ACTLbl.draw();
    nav2STBYLbl.draw();
  }
}

// Deprecated
//void convertFreq(long freq, char* out, size_t outSize) {
//  long whole = freq / 1000;
//  long frac = abs(freq % 1000);
//  snprintf(out, outSize, "%ld.%03d", whole, frac);
//}

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
