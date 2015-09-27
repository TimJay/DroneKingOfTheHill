#include "ColorSensor.h"

// ADJD-S311's I2C address, don't change
#define ADJD_S311_ADDRESS 0x74

// ADJD-S311's register list
#define CTRL 0x00
#define CONFIG 0x01
#define CAP_RED 0x06
#define CAP_GREEN 0x07
#define CAP_BLUE 0x08
#define CAP_CLEAR 0x09
#define INT_RED_LO 0xA
#define INT_RED_HI 0xB
#define INT_GREEN_LO 0xC
#define INT_GREEN_HI 0xD
#define INT_BLUE_LO 0xE
#define INT_BLUE_HI 0xF
#define INT_CLEAR_LO 0x10
#define INT_CLEAR_HI 0x11
#define DATA_RED_LO 0x40
#define DATA_RED_HI 0x41
#define DATA_GREEN_LO 0x42
#define DATA_GREEN_HI 0x43
#define DATA_BLUE_LO 0x44
#define DATA_BLUE_HI 0x45
#define DATA_CLEAR_LO 0x46
#define DATA_CLEAR_HI 0x47
#define OFFSET_RED 0x48
#define OFFSET_GREEN 0x49
#define OFFSET_BLUE 0x4A
#define OFFSET_CLEAR 0x4B

ColorSensor::ColorSensor(uint8_t led_pin) { _led_pin = led_pin; }

void ColorSensor::setup() {
  pinMode(_led_pin, OUTPUT);

  Wire.begin();
  delay(1);

  setPassive();
}

void ColorSensor::setPassive() {
  // set to highest sensitivity with led off
  ledOff();
  writeRegister(CAP_RED, 0);
  writeRegister(CAP_GREEN, 0);
  writeRegister(CAP_BLUE, 0);
  writeRegister(CAP_CLEAR, 0);
  writeRegisterLong(INT_RED_LO, 4095);
  writeRegisterLong(INT_GREEN_LO, 4095);
  writeRegisterLong(INT_BLUE_LO, 4095);
  writeRegisterLong(INT_CLEAR_LO, 4095);
}

void ColorSensor::setActive() {
  ledOn();
  writeRegister(CAP_RED, 15);
  writeRegister(CAP_GREEN, 15);
  writeRegister(CAP_BLUE, 15);
  writeRegister(CAP_CLEAR, 15);
  // calibrated to ~800 covered with white paper
  writeRegisterLong(INT_RED_LO, 333);
  writeRegisterLong(INT_GREEN_LO, 354);
  writeRegisterLong(INT_BLUE_LO, 436);
  writeRegisterLong(INT_CLEAR_LO, 130);
}

bool ColorSensor::isCovered() {
  setPassive();
  delay(10);
  RGBC color = read();
  return color.clear < 400;
}

Team ColorSensor::getTeam() {
  setActive();
  delay(10);
  RGBC color = read();
  setPassive();
  uint16_t rdiffg = (10 * color.red) / color.green;
  uint16_t rdiffb = (10 * color.red) / color.blue;
  if (rdiffg > 20 && rdiffb > 20) {
    return RED;
  } else if (rdiffg < 9 && rdiffb > 11) {
    return GREEN;
  } else if (rdiffg < 9 && rdiffb < 9) {
    return BLUE;
  } else if (rdiffg < 13 && rdiffb > 20) {
    return YELLOW;
  } else {
    return UNDEF;
  }
}

void ColorSensor::ledOn() { digitalWrite(_led_pin, HIGH); }

void ColorSensor::ledOff() { digitalWrite(_led_pin, LOW); }

RGBC ColorSensor::read() {
  RGBC color = RGBC();

  writeRegister(CTRL, 0x01);
  while (readRegister(CTRL) != 0)
    ;

  color.red = readRegisterLong(DATA_RED_LO);
  color.green = readRegisterLong(DATA_GREEN_LO);
  color.blue = readRegisterLong(DATA_BLUE_LO);
  color.clear = readRegisterLong(DATA_CLEAR_LO);

  return color;
}

void ColorSensor::writeRegister(uint8_t address, uint8_t data) {
  Wire.beginTransmission(ADJD_S311_ADDRESS);
  Wire.write(address);
  Wire.write(data);
  Wire.endTransmission();
}

void ColorSensor::writeRegisterLong(uint8_t address, uint16_t data) {
  if (data < 4096) {
    uint8_t msb = data >> 8;
    uint8_t lsb = data;

    writeRegister(address, lsb);
    writeRegister(address + 1, msb);
  }
}

uint8_t ColorSensor::readRegister(uint8_t address) {
  Wire.beginTransmission(ADJD_S311_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();

  Wire.requestFrom(ADJD_S311_ADDRESS, 1);
  while (!Wire.available())
    ;

  return Wire.read();
}

uint16_t ColorSensor::readRegisterLong(uint8_t address) {
  return readRegister(address) + (readRegister(address + 1) << 8);
}
