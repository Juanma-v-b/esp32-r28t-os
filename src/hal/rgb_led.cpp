#include "hal/rgb_led.h"

#include "config.h"

RgbLed rgbLed;

void RgbLed::begin() {
  ledcSetup(1, 5000, 8);
  ledcSetup(2, 5000, 8);
  ledcSetup(3, 5000, 8);
  ledcAttachPin(PIN_LED_R, 1);
  ledcAttachPin(PIN_LED_G, 2);
  ledcAttachPin(PIN_LED_B, 3);
  off();
}

void RgbLed::writeChannel(uint8_t channel, uint8_t value) {
  ledcWrite(channel, 255 - value);  // ánodo común: LOW enciende
}

void RgbLed::set(uint8_t red, uint8_t green, uint8_t blue) {
  writeChannel(1, red);
  writeChannel(2, green);
  writeChannel(3, blue);
}
