#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>

#include <vector>

enum class WifiState {
  Disabled,    // sin credenciales guardadas
  Connecting,  // intento en curso
  Connected,
  Failed
};

struct WifiNetwork {
  String ssid;
  int32_t rssi = -100;
  bool secure = true;
};

class WifiService {
 public:
  void begin();
  void loop();

  void connect(const String& ssid, const String& password);
  void disconnect();
  void forget();

  WifiState state() const { return state_; }
  bool connected() const { return state_ == WifiState::Connected; }
  bool hasSaved() const { return savedSsid_.length() > 0; }
  String savedSsid() const { return savedSsid_; }

  String ssid() const;
  String ip() const;
  int rssi() const;
  String stateText() const;

  /** Escaneo síncrono (bloquea ~2 s). Devuelve redes ordenadas por señal. */
  std::vector<WifiNetwork> scan();

 private:
  void saveCredentials(const String& ssid, const String& password);
  void startAttempt(const String& ssid, const String& password, bool saveOnSuccess);
  void clearSaved();

  Preferences prefs_;
  WifiState state_ = WifiState::Disabled;
  String savedSsid_;
  String savedPass_;
  String attemptSsid_;
  String attemptPass_;
  bool saveOnSuccess_ = false;
  uint32_t attemptStart_ = 0;
  uint32_t lastRetry_ = 0;
};

extern WifiService wifi;
