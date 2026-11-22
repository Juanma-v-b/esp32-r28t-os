#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>

class Display {
 public:
  bool begin(uint8_t brightnessPercent);
  TFT_eSPI& tft() { return tft_; }

  void setBrightness(uint8_t percent);
  uint8_t brightness() const { return brightness_; }

 private:
  TFT_eSPI tft_;
  uint8_t brightness_ = 75;
};

extern Display display;
