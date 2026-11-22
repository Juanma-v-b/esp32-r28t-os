#include "hal/touch.h"

#include <Arduino.h>
#include <Preferences.h>

#include "config.h"

Touch touch;

namespace {
constexpr uint8_t CMD_X = 0xD0;
constexpr uint8_t CMD_Y = 0x90;
constexpr uint16_t RAW_MIN = 80;
constexpr uint16_t RAW_MAX = 4090;
}  // namespace

void Touch::begin() {
  pinMode(PIN_TOUCH_CS, OUTPUT);
  digitalWrite(PIN_TOUCH_CS, HIGH);

  pinMode(PIN_TOUCH_CLK, OUTPUT);
  digitalWrite(PIN_TOUCH_CLK, LOW);

  pinMode(PIN_TOUCH_MOSI, OUTPUT);
  digitalWrite(PIN_TOUCH_MOSI, LOW);

  pinMode(PIN_TOUCH_MISO, INPUT);
  pinMode(PIN_TOUCH_IRQ, INPUT);

  loadCalibration();
}

uint8_t Touch::transfer(uint8_t out) {
  uint8_t in = 0;
  for (int i = 7; i >= 0; i--) {
    digitalWrite(PIN_TOUCH_MOSI, (out >> i) & 1);
    delayMicroseconds(2);
    digitalWrite(PIN_TOUCH_CLK, HIGH);
    delayMicroseconds(2);
    in = (uint8_t)((in << 1) | (digitalRead(PIN_TOUCH_MISO) ? 1 : 0));
    digitalWrite(PIN_TOUCH_CLK, LOW);
    delayMicroseconds(2);
  }
  return in;
}

uint16_t Touch::readChannel(uint8_t command) {
  digitalWrite(PIN_TOUCH_CS, LOW);
  delayMicroseconds(2);
  transfer(command);
  delayMicroseconds(2);
  const uint8_t hi = transfer(0x00);
  const uint8_t lo = transfer(0x00);
  digitalWrite(PIN_TOUCH_CS, HIGH);
  delayMicroseconds(2);

  const uint16_t value = (uint16_t)(((uint16_t)hi << 8) | lo);
  return (uint16_t)((value >> 3) & 0x0FFF);
}

bool Touch::read(uint16_t& rawX, uint16_t& rawY) {
  // El IRQ del XPT2046 es activo en bajo y evita leer cuando no hay dedo.
  if (digitalRead(PIN_TOUCH_IRQ) != LOW) {
    plausibleCount_ = 0;
    return false;
  }

  uint32_t sumX = 0;
  uint32_t sumY = 0;
  for (int i = 0; i < 2; i++) {
    sumX += readChannel(CMD_X);
    sumY += readChannel(CMD_Y);
  }
  const uint16_t x = (uint16_t)(sumX / 2);
  const uint16_t y = (uint16_t)(sumY / 2);

  const bool plausible = x > RAW_MIN && x < RAW_MAX && y > RAW_MIN && y < RAW_MAX;
  if (!plausible) {
    plausibleCount_ = 0;
    return false;
  }

  if (plausibleCount_ < 2) {
    plausibleCount_++;
    if (plausibleCount_ < 2) return false;
  }

  lastRawX_ = x;
  lastRawY_ = y;
  rawX = x;
  rawY = y;
  return true;
}

int32_t Touch::mapRange(int32_t value, int32_t inMin, int32_t inMax,
                        int32_t outMin, int32_t outMax) const {
  if (inMin == inMax) return outMin;
  return outMin + (int32_t)((int64_t)(value - inMin) * (outMax - outMin) /
                            (inMax - inMin));
}

bool Touch::mapToScreen(uint16_t rawX, uint16_t rawY, int16_t& x,
                        int16_t& y) const {
  int32_t xr = rawX;
  int32_t yr = rawY;

#if TOUCH_SWAP_XY
  const int32_t tmp = xr;
  xr = yr;
  yr = tmp;
#endif

  int32_t sx = mapRange(xr, cal_.xMin, cal_.xMax, 0, UI_SCREEN_W - 1);
  int32_t sy = mapRange(yr, cal_.yMin, cal_.yMax, 0, UI_SCREEN_H - 1);

#if TOUCH_INVERT_X
  sx = (UI_SCREEN_W - 1) - sx;
#endif
#if TOUCH_INVERT_Y
  sy = (UI_SCREEN_H - 1) - sy;
#endif

  x = (int16_t)constrain(sx, 0, UI_SCREEN_W - 1);
  y = (int16_t)constrain(sy, 0, UI_SCREEN_H - 1);
  return true;
}

void Touch::loadCalibration() {
  Preferences prefs;
  prefs.begin("touch", true);
  cal_.xMin = prefs.getInt("xmin", TOUCH_DEFAULT_X_MIN);
  cal_.xMax = prefs.getInt("xmax", TOUCH_DEFAULT_X_MAX);
  cal_.yMin = prefs.getInt("ymin", TOUCH_DEFAULT_Y_MIN);
  cal_.yMax = prefs.getInt("ymax", TOUCH_DEFAULT_Y_MAX);
  prefs.end();

  if (cal_.xMin == cal_.xMax || cal_.yMin == cal_.yMax) {
    cal_.xMin = TOUCH_DEFAULT_X_MIN;
    cal_.xMax = TOUCH_DEFAULT_X_MAX;
    cal_.yMin = TOUCH_DEFAULT_Y_MIN;
    cal_.yMax = TOUCH_DEFAULT_Y_MAX;
  }
}

void Touch::saveCalibration(const TouchCalibration& calibration) {
  cal_ = calibration;
  Preferences prefs;
  prefs.begin("touch", false);
  prefs.putInt("xmin", cal_.xMin);
  prefs.putInt("xmax", cal_.xMax);
  prefs.putInt("ymin", cal_.yMin);
  prefs.putInt("ymax", cal_.yMax);
  prefs.end();
}
