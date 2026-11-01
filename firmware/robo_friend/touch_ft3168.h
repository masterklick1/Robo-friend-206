// Minimal FT3168 capacitive touch reader (single finger), polling over I2C.
// Written from the chip's register map instead of using Waveshare's
// Arduino_DriveBus library, which is GPL-3.0 and would force the whole
// firmware to be GPL when distributed.
#pragma once
#include <Arduino.h>
#include <Wire.h>

class TouchFT3168 {
 public:
  static const uint8_t kAddr = 0x38;

  bool begin(int sda, int scl) {
    Wire.begin(sda, scl);
    Wire.setClock(400000);
    // 0xA5 = power mode, 0x01 = "monitor" (same setting the vendor demo uses)
    Wire.beginTransmission(kAddr);
    Wire.write((uint8_t)0xA5);
    Wire.write((uint8_t)0x01);
    ok_ = (Wire.endTransmission() == 0);
    return ok_;
  }

  bool ok() const { return ok_; }

  // Returns true while a finger is on the glass and fills x / y.
  bool read(int& x, int& y) {
    Wire.beginTransmission(kAddr);
    Wire.write((uint8_t)0x02);                 // TD_STATUS: number of touch points
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom((uint8_t)kAddr, (uint8_t)5) != 5) return false;
    uint8_t n = Wire.read() & 0x0F;
    uint8_t xh = Wire.read();
    uint8_t xl = Wire.read();
    uint8_t yh = Wire.read();
    uint8_t yl = Wire.read();
    if (n == 0 || n > 2) return false;
    x = ((xh & 0x0F) << 8) | xl;
    y = ((yh & 0x0F) << 8) | yl;
    return true;
  }

 private:
  bool ok_ = false;
};
