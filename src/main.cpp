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

  // Medida rápida de ancho de banda de la pantalla (153 KB por frame).
  {
    const uint32_t tb = millis();
    display.tft().fillScreen(TFT_RED);
    const uint32_t fullMs = millis() - tb;
    display.tft().fillScreen(TFT_BLACK);
    Serial.printf("[bench] frame completo: %lu ms (%lu KB/s)\n",
                  (unsigned long)fullMs,
                  (unsigned long)(153600UL / (fullMs ? fullMs : 1)));
  }

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
  const uint32_t t0 = millis();
  wifi.loop();
  const uint32_t t1 = millis();
  timeService.loop();
  const uint32_t t2 = millis();
  ui::loop();
  const uint32_t t3 = millis();
  handleBootButton();
  const uint32_t t4 = millis();
  delay(2);

  // Aviso de bloqueos: si una vuelta tarda más de 120 ms algo está
  // reteniendo el bucle (SD, HTTP, scan de WiFi...).
  if (t4 - t0 > 120) {
    Serial.printf("[slow] total=%lums wifi=%lu time=%lu ui=%lu heap=%u\n",
                  (unsigned long)(t4 - t0), (unsigned long)(t1 - t0),
                  (unsigned long)(t2 - t1), (unsigned long)(t3 - t2),
                  ESP.getFreeHeap());
  }
}
