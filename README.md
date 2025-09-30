# ESP32 Radio Panel

### Arduino IDE Prereqs
- Bits and Droids Flight Sim Library
- ESP32Encoder: Kevin Harrington
- Joystick: Giuseppe
- TFT_eSPI: Bodmer
- Install ESP32 Dev Module

### TFT_eSPI Config (User_Setup.h)
Can be found in `Documents\Arduino\libraries\TFT_eSPI\User_Setup.h`
- Uncomment `#define ST7796_DRIVER` (don't forget to comment others)

```
#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST   4
```

### TODO
1. Implement freq inputs from Connector
1. [Fix rotary encoder inputs](https://github.com/mo-thunderz/RotaryEncoder/blob/main/Arduino/ArduinoRotaryEncoder/ArduinoRotaryEncoder.ino)
1. [Fix button debounce](https://www.programmingelectronics.com/debouncing-a-button-with-arduino/#:~:text=Step%2Dby%2DStep%20Debounce%20Instructions&text=Connect%20the%20220%2Dohm%20resistor,other%20side%20of%20the%20pushbutton)
1. Fix Freq Label colors on tap
1. Create buttons for radio swap and Rx
1. Refactor code for reusability/cleanliness (Pointers?)
1. General optimizations
1. Change display to imitate FS2024 A321LR com panel?
1. COM freqs: Utilize SimVars `COM ACTIVE FREQ TYPE`, `COM STANDBY FREQ TYPE` to display [ATC station ident and type](https://docs.flightsimulator.com/html/Programming_Tools/SimVars/Aircraft_SimVars/Aircraft_RadioNavigation_Variables.htm)

### Etc. Notes
- 480x320 Screen, ST7796S driver, ESP32 dev board
- Display: COM1 and NAV freqs
- Tapping STBY freq allows Rotary Encoder to edit freq
- Tapping Transfer button swaps freqs
- Tapping ACT COM changes Tx to selected
```
    |-----------------|                   |-----------------|
    |                 |      ------       |                 |
    |     123.456     |      | Rx |       |     123.456     |
    |                 |      ------       |                 |
    |-----------------|                   |-----------------|
```