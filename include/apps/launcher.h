#pragma once

#include "app.h"

namespace apps {

class LauncherApp : public App {
 public:
  const char* title() const override { return "R28T OS"; }
  void build(lv_obj_t* screen) override;

 private:
  static void cardClicked(lv_event_t* e);
};

}  // namespace apps
