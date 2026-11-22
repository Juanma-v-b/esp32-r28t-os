#pragma once

#include <Arduino.h>

/**
 * Hora local/NTP. Se configura la zona horaria al arrancar y se
 * resincroniza cuando hay WiFi.
 */
class TimeService {
 public:
  void begin();
  void loop();

  bool synced() const;
  void applyTimezone();
  void syncNow();

  /** Formatea la hora local actual, p. ej. "%H:%M". */
  static String format(const char* fmt);

 private:
  uint32_t lastAttempt_ = 0;
  bool configured_ = false;
};

extern TimeService timeService;
