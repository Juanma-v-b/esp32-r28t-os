#include "core/weather_service.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <math.h>
#include <time.h>

#include "core/settings.h"
#include "core/wifi_service.h"

WeatherService weather;

bool WeatherService::refresh(const SettingsStore& settings, String& error) {
  error = "";

  if (!wifi.connected()) {
    error = "Sin conexión WiFi";
    data_.error = error;
    return false;
  }

  String url = "http://api.open-meteo.com/v1/forecast?latitude=";
  url += String(settings.lat(), 4);
  url += "&longitude=";
  url += String(settings.lon(), 4);
  url += "&current=temperature_2m,relative_humidity_2m,apparent_temperature,";
  url += "is_day,weather_code,wind_speed_10m";
  url += "&daily=weather_code,temperature_2m_max,temperature_2m_min";
  url += "&timezone=auto&forecast_days=4&wind_speed_unit=kmh";

  WiFiClient client;
  HTTPClient http;
  http.setConnectTimeout(6000);
  http.setTimeout(9000);

  if (!http.begin(client, url)) {
    error = "No se pudo iniciar HTTP";
    data_.error = error;
    return false;
  }

  const int status = http.GET();
  if (status != HTTP_CODE_OK) {
    http.end();
    error = String("HTTP ") + status;
    data_.error = error;
    return false;
  }

  String payload = http.getString();
  http.end();

  JsonDocument doc;
  DeserializationError jsonError = deserializeJson(doc, payload);
  if (jsonError) {
    error = String("JSON: ") + jsonError.c_str();
    data_.error = error;
    return false;
  }

  JsonObject current = doc["current"].as<JsonObject>();
  if (current.isNull()) {
    error = "Respuesta sin datos actuales";
    data_.error = error;
    return false;
  }

  data_.temp = current["temperature_2m"] | NAN;
  data_.feels = current["apparent_temperature"] | NAN;
  data_.humidity = current["relative_humidity_2m"] | -1;
  data_.wind = current["wind_speed_10m"] | NAN;
  data_.code = current["weather_code"] | -1;
  data_.isDay = (current["is_day"] | 1) != 0;

  data_.dayCount = 0;
  JsonArray times = doc["daily"]["time"].as<JsonArray>();
  JsonArray codes = doc["daily"]["weather_code"].as<JsonArray>();
  JsonArray tMax = doc["daily"]["temperature_2m_max"].as<JsonArray>();
  JsonArray tMin = doc["daily"]["temperature_2m_min"].as<JsonArray>();

  const size_t count = times.size();
  for (size_t i = 1; i < count && data_.dayCount < 3; i++) {
    WeatherDay& day = data_.days[data_.dayCount];
    day.date = times[i] | "";
    day.code = codes[i] | -1;
    day.tMax = tMax[i] | NAN;
    day.tMin = tMin[i] | NAN;
    data_.dayCount++;
  }

  data_.valid = true;
  data_.error = "";
  data_.updatedAt = millis();
  return true;
}

void WeatherService::invalidate() {
  data_.valid = false;
  data_.updatedAt = 0;
}

bool WeatherService::stale(uint32_t maxAgeMs) const {
  if (!data_.valid || data_.updatedAt == 0) return true;
  return (millis() - data_.updatedAt) > maxAgeMs;
}

const char* WeatherService::describe(int code) {
  if (code == 0) return "Despejado";
  if (code == 1) return "Mayormente despejado";
  if (code == 2) return "Parcialmente nublado";
  if (code == 3) return "Nublado";
  if (code == 45 || code == 48) return "Niebla";
  if (code >= 51 && code <= 57) return "Llovizna";
  if (code >= 61 && code <= 67) return "Lluvia";
  if (code >= 71 && code <= 77) return "Nieve";
  if (code >= 80 && code <= 82) return "Chubascos";
  if (code == 85 || code == 86) return "Nieve";
  if (code >= 95) return "Tormenta";
  return "—";
}

int WeatherService::iconType(int code, bool isDay) {
  (void)isDay;
  if (code == 0) return 0;
  if (code == 1 || code == 2) return 1;
  if (code == 3) return 2;
  if (code == 45 || code == 48) return 3;
  if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82)) return 4;
  if ((code >= 71 && code <= 77) || code == 85 || code == 86) return 5;
  if (code >= 95) return 6;
  return 2;
}

const char* WeatherService::weekdayShort(const String& isoDate) {
  int y = 0, m = 0, d = 0;
  if (sscanf(isoDate.c_str(), "%d-%d-%d", &y, &m, &d) != 3) return "";

  struct tm tmDate = {};
  tmDate.tm_year = y - 1900;
  tmDate.tm_mon = m - 1;
  tmDate.tm_mday = d;
  tmDate.tm_hour = 12;
  time_t t = mktime(&tmDate);
  struct tm out;
  localtime_r(&t, &out);

  static const char* names[] = {"Dom", "Lun", "Mar", "Mié", "Jue", "Vie", "Sáb"};
  return names[out.tm_wday % 7];
}
