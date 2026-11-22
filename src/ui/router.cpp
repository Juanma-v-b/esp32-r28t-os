#include "ui/router.h"

#include "app.h"
#include "apps/clock.h"
#include "apps/gallery.h"
#include "apps/launcher.h"
#include "apps/settings.h"
#include "apps/weather.h"
#include "apps/wifi.h"

namespace ui {
namespace {

struct PendingNav {
  bool active = false;
  AppId id = AppId::Launcher;
  bool back = false;
};

PendingNav gPending;
apps::App* gActive = nullptr;
lv_obj_t* gScreen = nullptr;
AppId gCurrent = AppId::Launcher;
bool gStarted = false;

apps::App* createApp(AppId id) {
  switch (id) {
    case AppId::Launcher:
      return new apps::LauncherApp();
    case AppId::Clock:
      return new apps::ClockApp();
    case AppId::Weather:
      return new apps::WeatherApp();
    case AppId::Gallery:
      return new apps::GalleryApp();
    case AppId::Settings:
      return new apps::SettingsApp();
    case AppId::Wifi:
      return new apps::WifiApp();
  }
  return nullptr;
}

void screenDeleted(lv_event_t* e) {
  auto* app = static_cast<apps::App*>(lv_event_get_user_data(e));
  delete app;
}

void mount(AppId id, bool back) {
  apps::App* next = createApp(id);
  if (!next) return;

  lv_obj_t* screen = lv_obj_create(NULL);
  lv_obj_add_event_cb(screen, screenDeleted, LV_EVENT_DELETE, next);
  next->build(screen);

  if (gActive) gActive->onDestroy();

  gActive = next;
  gCurrent = id;
  gScreen = screen;

  if (!gStarted) {
    lv_scr_load(screen);
    gStarted = true;
  } else {
    lv_scr_load_anim(screen,
                     back ? LV_SCR_LOAD_ANIM_MOVE_RIGHT : LV_SCR_LOAD_ANIM_MOVE_LEFT,
                     200, 0, true);
  }
}

}  // namespace

void open(AppId id) {
  gPending.id = id;
  gPending.back = false;
  gPending.active = true;
}

void home() {
  gPending.id = AppId::Launcher;
  gPending.back = true;
  gPending.active = true;
}

AppId current() { return gCurrent; }

void routerBegin() { mount(AppId::Launcher, false); }

void routerLoop() {
  if (!gPending.active) return;
  gPending.active = false;
  mount(gPending.id, gPending.back);
}

}  // namespace ui
