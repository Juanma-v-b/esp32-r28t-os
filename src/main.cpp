#include <Arduino.h>

#include "config.h"
#include "core/sd_service.h"
#include "core/settings.h"
#include "core/time_service.h"
#include "core/weather_service.h"
#include "core/wifi_service.h"
#include "hal/display.h"
#include "hal/rgb_led.h"
#include "hal/touch.h"
#include "ui/router.h"
#include "ui/ui.h"

namespace {

bool bootWasDown = false;
uint32_t lastBootPress = 0;

void handleBootButton() {
  const bool down = digitalRead(PIN_BOOT_BTN) == LOW;
  const uint32_t now = millis();

  if (down && !bootWasDown && now - lastBootPress > 300) {
    lastBootPress = now;
    if (ui::current() != ui::AppId::Launcher) ui::home();
  }
  bootWasDown = down;
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(120);
  Serial.printf("\n%s %s\n", FW_NAME, FW_VERSION);

  randomSeed(micros());

  settings.begin();

  display.begin(settings.brightness());
  touch.begin();
  rgbLed.begin();

  pinMode(PIN_BOOT_BTN, INPUT_PULLUP);
  pinMode(PIN_AUDIO_EN, OUTPUT);
  digitalWrite(PIN_AUDIO_EN, LOW);  // amplificador desactivado

  ui::begin();

  wifi.begin();
  timeService.begin();
  sdCard.begin();

  rgbLed.set(0, 0, 18);  // azul tenue = encendido
}

void loop() {
  wifi.loop();
  timeService.loop();
  ui::loop();
  handleBootButton();
  delay(2);
}
