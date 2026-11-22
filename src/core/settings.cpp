#include "core/settings.h"

SettingsStore settings;

void SettingsStore::begin() {
  prefs_.begin("r28t", false);
  brightness_ = prefs_.getUChar("bright", 75);
  clockTheme_ = prefs_.getUChar("theme", 0);
  tzIndex_ = prefs_.getUChar("tz", 0);
  city_ = prefs_.getString("city", DEFAULT_CITY);
  lat_ = prefs_.getFloat("lat", DEFAULT_LAT);
  lon_ = prefs_.getFloat("lon", DEFAULT_LON);

  if (brightness_ < 10 || brightness_ > 100) brightness_ = 75;
  if (clockTheme_ >= CLOCK_THEME_COUNT) clockTheme_ = 0;
  if (tzIndex_ > 1) tzIndex_ = 0;
}

void SettingsStore::setBrightness(uint8_t value) {
  brightness_ = constrain(value, 10, 100);
  prefs_.putUChar("bright", brightness_);
}

void SettingsStore::setClockTheme(uint8_t value) {
  clockTheme_ = value % CLOCK_THEME_COUNT;
  prefs_.putUChar("theme", clockTheme_);
}

void SettingsStore::setTzIndex(uint8_t value) {
  tzIndex_ = value > 1 ? 0 : value;
  prefs_.putUChar("tz", tzIndex_);
}

String SettingsStore::tzLabel() const {
  return tzIndex_ == 1 ? TZ_CANARY_LABEL : TZ_MAINLAND_LABEL;
}

String SettingsStore::tzPosix() const {
  return tzIndex_ == 1 ? TZ_CANARY_POSIX : TZ_MAINLAND_POSIX;
}

void SettingsStore::setCity(const String& value) {
  city_ = value;
  city_.trim();
  if (city_.isEmpty()) city_ = DEFAULT_CITY;
  prefs_.putString("city", city_);
}

void SettingsStore::setLat(float value) {
  lat_ = value;
  prefs_.putFloat("lat", lat_);
}

void SettingsStore::setLon(float value) {
  lon_ = value;
  prefs_.putFloat("lon", lon_);
}
