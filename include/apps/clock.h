#pragma once

#include "app.h"

namespace apps {

class ClockApp : public App {
 public:
  const char* title() const override { return "Reloj"; }
  void build(lv_obj_t* screen) override;
  void onDestroy() override;

 private:
  struct Star {
    lv_obj_t* obj = nullptr;
    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t phase = 0;
    uint8_t size = 2;
  };

  static void clockTick(lv_timer_t* t);
  static void starTick(lv_timer_t* t);
  static void backgroundClicked(lv_event_t* e);

  void updateClock();
  void applyTheme();
  void buildStars();

  lv_obj_t* screen_ = nullptr;
  lv_obj_t* bg_ = nullptr;
  lv_obj_t* dateLbl_ = nullptr;
  lv_obj_t* timeLbl_ = nullptr;
  lv_obj_t* secondsLbl_ = nullptr;
  lv_obj_t* weatherLbl_ = nullptr;
  lv_obj_t* hintLbl_ = nullptr;
  lv_obj_t* moon_ = nullptr;

  Star stars_[30];
  int starCount_ = 0;

  lv_timer_t* clockTimer_ = nullptr;
  lv_timer_t* starTimer_ = nullptr;
};

}  // namespace apps
