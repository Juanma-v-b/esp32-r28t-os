#pragma once

#include <Arduino.h>
#include <lvgl.h>

#include "app.h"

namespace apps {

class WeatherApp : public App {
 public:
  const char* title() const override { return "Tiempo"; }
  void build(lv_obj_t* screen) override;
  void onDestroy() override;

 private:
  static void refreshClicked(lv_event_t* e);
  static void wifiClicked(lv_event_t* e);
  static void fetchLater(lv_timer_t* t);
  static void autoTimer(lv_timer_t* t);

  void render();
  void startFetch();

  lv_obj_t* screen_ = nullptr;
  lv_obj_t* content_ = nullptr;
  lv_timer_t* timer_ = nullptr;
  lv_timer_t* fetchTimer_ = nullptr;
  bool fetching_ = false;
  String error_;
};

}  // namespace apps
