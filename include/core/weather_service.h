#pragma once

#include <Arduino.h>

class SettingsStore;

struct WeatherDay {
  String date;      // YYYY-MM-DD
  int code = 0;
  float tMax = NAN;
  float tMin = NAN;
};

struct WeatherData {
  bool valid = false;
  String error;
  uint32_t updatedAt = 0;  // millis()

  float temp = NAN;
  float feels = NAN;
  float wind = NAN;
  int humidity = -1;
  int code = -1;
  bool isDay = true;

  WeatherDay days[3];
  int dayCount = 0;
};

class WeatherService {
 public:
  /** Descarga Open-Meteo. Devuelve false y rellena error si falla. */
  bool refresh(const SettingsStore& settings, String& error);
  void invalidate();

  const WeatherData& data() const { return data_; }
  bool stale(uint32_t maxAgeMs = 10UL * 60UL * 1000UL) const;

  static const char* describe(int code);
  /** 0 sol, 1 nubes y sol, 2 nublado, 3 niebla, 4 lluvia, 5 nieve, 6 tormenta */
  static int iconType(int code, bool isDay);
  static const char* weekdayShort(const String& isoDate);

 private:
  WeatherData data_;
};

extern WeatherService weather;
