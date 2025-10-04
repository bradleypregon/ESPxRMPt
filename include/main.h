// i am not a c++ programmer

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <ESP32Encoder.h>
#include <BitsAndDroidsFlightConnector.h>
#include <string>

// Pins
#define bigA 32
#define bigB 33
#define smallA 25
#define smallB 26
#define sw 27

// Other
#define TFT_GREY 0x5AEB
#define comBuff 8
#define navBuff 7

TFT_eSPI tft;
BitsAndDroidsFlightConnector connector;
ESP32Encoder smallEncoder;
ESP32Encoder bigEncoder;

long c1a = 122800;
long c2a = 123800;
long c1s = 121500;
long c2s = 122500;

long n1a = 10900;
long n2a = 11000;
long n1s = 10950;
long n2s = 11050;

struct RoundedRectangle {
  int x, y, width, height;
  int command = -1;

  void draw() {
    tft.drawRoundRect(x, y, width, height, 4, TFT_GREY);
  }
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

void handleCom1ACTTouched(RoundedRectangle);
void handleCom2ACTTouched(RoundedRectangle);
void handleCom1STBYTouched(RoundedRectangle);
void handleCom2STBYTouched(RoundedRectangle);
