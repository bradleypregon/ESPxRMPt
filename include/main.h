// i am not a c++ programmer

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <ESP32Encoder.h>
#include <BitsAndDroidsFlightConnector.h>
#include <string>
#include <unordered_map>

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
#define bigA 32
#define bigB 33
#define smallA 25
#define smallB 26
#define sw 27

// Other
#define TFT_GREY 0x5AEB
#define roundRectRadius 4
//#define freqBuff 8

TFT_eSPI tft;
BitsAndDroidsFlightConnector connector;
ESP32Encoder smallEncoder;
ESP32Encoder bigEncoder;

// COM/NAV ACT/STBY Freqs
struct FreqPair {
  long active;
  long standby;
};

enum Commands {
  setCom1Tx = 0,
  setCom1Rx = 1,
  setCom2Tx = 2,
  setCom2Rx = 3
};

enum ActiveEdit { 
  COM1,
  COM2,
  NAV1,
  NAV2 
};

struct RoundedRectangle {
  int x, y, width, height;
  int command = -1;

  void draw() {
    tft.drawRoundRect(x, y, width, height, roundRectRadius, TFT_GREY);
  }
};

struct FreqLabel {
  int x;
  int y;
  long freq;
  int color = TFT_GREY;

  const char* convertFreq() {
    std::string label = std::to_string(freq);
    label.insert(4, ".");
    return label.c_str();
  }

  void draw() {
    tft.setTextSize(3);
    tft.setTextColor(color, TFT_BLACK, true);
    tft.drawString(convertFreq(), x, y);
  }
};

void handleCom1ACTTouched(RoundedRectangle);
void handleCom2ACTTouched(RoundedRectangle);
void handleCom1STBYTouched(RoundedRectangle);
void handleCom2STBYTouched(RoundedRectangle);

void updateFreq(std::string, FreqPair newFreqs);