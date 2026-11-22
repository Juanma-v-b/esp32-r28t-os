#pragma once

#include <Arduino.h>
#include <Preferences.h>

#include "config.h"

/**
 * Ajustes persistentes (NVS). Todo lo que el usuario cambia desde
 * Ajustes acaba aquí para sobrevivir a reinicios.
 */
class SettingsStore {
 public:
  void begin();

  uint8_t brightness() const { return brightness_; }
  void setBrightness(uint8_t value);

  uint8_t clockTheme() const { return clockTheme_; }
  void setClockTheme(uint8_t value);

  uint8_t tzIndex() const { return tzIndex_; }
  void setTzIndex(uint8_t value);

  String tzLabel() const;
  String tzPosix() const;

  String city() const { return city_; }
  void setCity(const String& value);

  float lat() const { return lat_; }
  void setLat(float value);

  float lon() const { return lon_; }
  void setLon(float value);

 private:
  Preferences prefs_;
  uint8_t brightness_ = 75;
  uint8_t clockTheme_ = 0;
  uint8_t tzIndex_ = 0;
  String city_ = DEFAULT_CITY;
  float lat_ = DEFAULT_LAT;
  float lon_ = DEFAULT_LON;
};

extern SettingsStore settings;
