#pragma once

#include <Arduino.h>

/** LED RGB de la placa (ánodo común: activo en LOW). */
class RgbLed {
 public:
  void begin();
  void set(uint8_t red, uint8_t green, uint8_t blue);
  void off() { set(0, 0, 0); }

 private:
  void writeChannel(uint8_t channel, uint8_t value);
};

extern RgbLed rgbLed;
