#pragma once

#include <Arduino.h>
#include <lvgl.h>

#include <vector>

#include "app.h"
#include "core/wifi_service.h"
#include "ui/keyboard_prompt.h"

namespace apps {

class WifiApp : public App {
 public:
  const char* title() const override { return "WiFi"; }
  void build(lv_obj_t* screen) override;
  void onDestroy() override;

 private:
  static void scanClicked(lv_event_t* e);
  static void networkClicked(lv_event_t* e);
  static void disconnectClicked(lv_event_t* e);
  static void forgetClicked(lv_event_t* e);
  static void statusTimer(lv_timer_t* t);
  static void scanLater(lv_timer_t* t);

  void rebuildStatus();
  void rebuildNetworks();
  void startScan();
  void doScan();

  lv_obj_t* screen_ = nullptr;
  lv_obj_t* statusCard_ = nullptr;
  lv_obj_t* list_ = nullptr;
  lv_obj_t* scanBtn_ = nullptr;

  std::vector<WifiNetwork> networks_;
  WifiState lastState_ = WifiState::Disabled;
  String message_;
  bool scanned_ = false;
  bool scanning_ = false;

  lv_timer_t* timer_ = nullptr;
  lv_timer_t* scanTimer_ = nullptr;
  ui::KeyboardPrompt prompt_;
};

}  // namespace apps
