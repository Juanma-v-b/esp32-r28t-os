#include "hal/display.h"

#include <Arduino.h>

#include "config.h"

Display display;

bool Display::begin(uint8_t brightnessPercent) {
  pinMode(PIN_TFT_BL, OUTPUT);
  digitalWrite(PIN_TFT_BL, LOW);  // apagada hasta que todo esté listo

  tft_.init();
  tft_.setRotation(0);
  tft_.setSwapBytes(true);
  tft_.fillScreen(TFT_BLACK);

  ledcSetup(BL_LEDC_CHANNEL, 5000, 8);
  ledcAttachPin(PIN_TFT_BL, BL_LEDC_CHANNEL);
  setBrightness(brightnessPercent);

  return true;
}

void Display::setBrightness(uint8_t percent) {
  brightness_ = constrain(percent, 5, 100);
  const uint32_t duty = (uint32_t)brightness_ * 255U / 100U;
  ledcWrite(BL_LEDC_CHANNEL, duty);
}
