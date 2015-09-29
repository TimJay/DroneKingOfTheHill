#ifndef COLORSENSOR_h
#define COLORSENSOR_h

#include "Arduino.h"
#include "Wire.h"

struct RGBC {
  uint16_t red;
  uint16_t blue;
  uint16_t green;
  uint16_t clear;
};

typedef enum { RED, GREEN, BLUE, YELLOW, UNDEF } Team;

class ColorSensor {
public:
  ColorSensor(uint8_t led_pin);

  void setup();
  void setPassive();
  void setActive();
  bool isCovered();
  Team getTeam();

private:
  uint8_t _led_pin;

  void ledOn();
  void ledOff();

  RGBC read();

  void writeRegister(uint8_t address, uint8_t data);
  void writeRegisterLong(uint8_t address, uint16_t data);

  uint8_t readRegister(uint8_t address);
  uint16_t readRegisterLong(uint8_t address);
};

#endif // COLORSENSOR_h
