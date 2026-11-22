#pragma once

#include <Arduino.h>
#include <lvgl.h>

#include "app.h"
#include "ui/keyboard_prompt.h"

namespace apps {

class SettingsApp : public App {
 public:
  const char* title() const override { return "Ajustes"; }
  void build(lv_obj_t* screen) override;
  void onDestroy() override;

 private:
  static void brightnessChanged(lv_event_t* e);
  static void themeClicked(lv_event_t* e);
  static void timezoneClicked(lv_event_t* e);
  static void wifiClicked(lv_event_t* e);
  static void cityClicked(lv_event_t* e);
  static void latClicked(lv_event_t* e);
  static void lonClicked(lv_event_t* e);
  static void calibrateClicked(lv_event_t* e);
  static void ledClicked(lv_event_t* e);
  static void calibrationPressed(lv_event_t* e);
  static void rebuildLater(lv_timer_t* t);

  void renderContent();
  void scheduleRebuild();
  void openCalibration();
  void closeCalibration();

  lv_obj_t* screen_ = nullptr;
  lv_obj_t* content_ = nullptr;
  lv_obj_t* brightnessLbl_ = nullptr;
  lv_timer_t* rebuildTimer_ = nullptr;

  ui::KeyboardPrompt prompt_;

  lv_obj_t* calOverlay_ = nullptr;
  lv_obj_t* calTarget_ = nullptr;
  lv_obj_t* calLabel_ = nullptr;
  int calStep_ = 0;
  uint16_t calRawX_[4] = {0, 0, 0, 0};
  uint16_t calRawY_[4] = {0, 0, 0, 0};
};

}  // namespace apps
