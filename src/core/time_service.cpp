#include "core/time_service.h"

#include <time.h>

#include "core/settings.h"
#include "core/wifi_service.h"

TimeService timeService;

void TimeService::begin() {
  applyTimezone();
  lastAttempt_ = millis() - 20000;  // intenta en cuanto haya WiFi
}

void TimeService::applyTimezone() {
  setenv("TZ", settings.tzPosix().c_str(), 1);
  tzset();
  configured_ = false;
  lastAttempt_ = millis() - 20000;
}

void TimeService::syncNow() {
  configTzTime(settings.tzPosix().c_str(), "pool.ntp.org", "time.google.com",
               "es.pool.ntp.org");
  configured_ = true;
  lastAttempt_ = millis();
  Serial.println("[ntp] sincronizando hora");
}

void TimeService::loop() {
  if (!wifi.connected()) return;

  const uint32_t now = millis();
  if (!configured_) {
    if (now - lastAttempt_ > 3000) syncNow();
  } else if (!synced() && now - lastAttempt_ > 30000) {
    syncNow();
  }
}

bool TimeService::synced() const { return time(nullptr) > 1700000000; }

String TimeService::format(const char* fmt) {
  time_t now = time(nullptr);
  struct tm local;
  localtime_r(&now, &local);
  char buf[64];
  strftime(buf, sizeof(buf), fmt, &local);
  return String(buf);
}
