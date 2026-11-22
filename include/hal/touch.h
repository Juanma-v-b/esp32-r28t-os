#pragma once

#include <Arduino.h>

struct TouchCalibration {
  int32_t xMin = 240;
  int32_t xMax = 3760;
  int32_t yMin = 280;
  int32_t yMax = 3740;
};

/**
 * Driver del XPT2046 por SPI software.
 *
 * La pantalla usa HSPI y la SD usa VSPI, así que bit-bangear el táctil
 * (4 pines) evita cualquier conflicto de bus y permite leerlo mientras
 * se está pintando una imagen desde la SD.
 */
class Touch {
 public:
  void begin();

  /** Lee una muestra física. Devuelve false si no hay contacto válido. */
  bool read(uint16_t& rawX, uint16_t& rawY);

  bool mapToScreen(uint16_t rawX, uint16_t rawY, int16_t& x, int16_t& y) const;

  void loadCalibration();
  void saveCalibration(const TouchCalibration& calibration);

  TouchCalibration calibration() const { return cal_; }
  uint16_t lastRawX() const { return lastRawX_; }
  uint16_t lastRawY() const { return lastRawY_; }

 private:
  uint16_t readChannel(uint8_t command);
  uint8_t transfer(uint8_t out);
  int32_t mapRange(int32_t value, int32_t inMin, int32_t inMax, int32_t outMin,
                   int32_t outMax) const;

  TouchCalibration cal_;
  uint16_t lastRawX_ = 0;
  uint16_t lastRawY_ = 0;
  uint8_t plausibleCount_ = 0;
};

extern Touch touch;
