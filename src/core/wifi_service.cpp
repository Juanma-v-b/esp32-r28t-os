#include "core/wifi_service.h"

#include <algorithm>

#include "config.h"

WifiService wifi;

namespace {
constexpr uint32_t CONNECT_TIMEOUT_MS = 15000;
constexpr uint32_t RETRY_INTERVAL_MS = 45000;
}  // namespace

void WifiService::begin() {
  prefs_.begin("wifi", false);
  savedSsid_ = prefs_.getString("ssid", "");
  savedPass_ = prefs_.getString("pass", "");

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setHostname("r28t-os");

  if (savedSsid_.length() > 0) {
    startAttempt(savedSsid_, savedPass_, false);
  } else {
    state_ = WifiState::Disabled;
  }
}

void WifiService::startAttempt(const String& ssid, const String& password,
                               bool saveOnSuccess) {
  attemptSsid_ = ssid;
  attemptPass_ = password;
  saveOnSuccess_ = saveOnSuccess;
  attemptStart_ = millis();

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(false, false);
  WiFi.begin(attemptSsid_.c_str(), attemptPass_.c_str());
  state_ = WifiState::Connecting;
}

void WifiService::connect(const String& ssid, const String& password) {
  startAttempt(ssid, password, true);
}

void WifiService::disconnect() {
  WiFi.disconnect(true);
  state_ = WifiState::Failed;
  attemptStart_ = millis();
}

void WifiService::forget() {
  clearSaved();
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_OFF);
  state_ = WifiState::Disabled;
}

void WifiService::clearSaved() {
  savedSsid_ = "";
  savedPass_ = "";
  prefs_.remove("ssid");
  prefs_.remove("pass");
}

void WifiService::saveCredentials(const String& ssid, const String& password) {
  savedSsid_ = ssid;
  savedPass_ = password;
  prefs_.putString("ssid", savedSsid_);
  prefs_.putString("pass", savedPass_);
}

void WifiService::loop() {
  const uint32_t now = millis();

  switch (state_) {
    case WifiState::Connecting:
      if (WiFi.status() == WL_CONNECTED) {
        state_ = WifiState::Connected;
        if (saveOnSuccess_) {
          saveCredentials(attemptSsid_, attemptPass_);
          saveOnSuccess_ = false;
        }
        Serial.printf("[wifi] conectado a %s (%s)\n", WiFi.SSID().c_str(),
                      WiFi.localIP().toString().c_str());
      } else if (now - attemptStart_ > CONNECT_TIMEOUT_MS) {
        WiFi.disconnect(false, false);
        state_ = WifiState::Failed;
        attemptStart_ = now;
        Serial.println("[wifi] timeout de conexión");
      }
      break;

    case WifiState::Connected:
      if (WiFi.status() != WL_CONNECTED) {
        state_ = WifiState::Failed;
        attemptStart_ = now;
        Serial.println("[wifi] conexión perdida");
      }
      break;

    case WifiState::Failed:
      if (hasSaved() && now - attemptStart_ > RETRY_INTERVAL_MS) {
        attemptStart_ = now;
        startAttempt(savedSsid_, savedPass_, false);
      }
      break;

    case WifiState::Disabled:
      break;
  }
}

String WifiService::ssid() const {
  if (state_ == WifiState::Connected) return WiFi.SSID();
  return savedSsid_;
}

String WifiService::ip() const {
  if (state_ == WifiState::Connected) return WiFi.localIP().toString();
  return "";
}

int WifiService::rssi() const {
  if (state_ == WifiState::Connected) return WiFi.RSSI();
  return -100;
}

String WifiService::stateText() const {
  switch (state_) {
    case WifiState::Connected:
      return "Conectado";
    case WifiState::Connecting:
      return "Conectando…";
    case WifiState::Failed:
      return "Sin conexión";
    case WifiState::Disabled:
    default:
      return "Sin red configurada";
  }
}

std::vector<WifiNetwork> WifiService::scan() {
  std::vector<WifiNetwork> result;
  if (WiFi.getMode() == WIFI_OFF) WiFi.mode(WIFI_STA);

  const int n = WiFi.scanNetworks(false, true);
  for (int i = 0; i < n; i++) {
    String ssid = WiFi.SSID(i);
    if (ssid.isEmpty()) continue;

    bool merged = false;
    for (auto& net : result) {
      if (net.ssid == ssid) {
        if (WiFi.RSSI(i) > net.rssi) net.rssi = WiFi.RSSI(i);
        merged = true;
        break;
      }
    }
    if (merged) continue;

    WifiNetwork net;
    net.ssid = ssid;
    net.rssi = WiFi.RSSI(i);
    net.secure = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
    result.push_back(net);
  }
  WiFi.scanDelete();

  std::sort(result.begin(), result.end(),
            [](const WifiNetwork& a, const WifiNetwork& b) {
              return a.rssi > b.rssi;
            });
  return result;
}
