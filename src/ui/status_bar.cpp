#include "ui/status_bar.h"

#include <Arduino.h>

#include "config.h"
#include "core/time_service.h"
#include "core/wifi_service.h"
#include "ui/router.h"
#include "ui/theme.h"

namespace ui {
namespace {

struct BarRefs {
  lv_obj_t* time = nullptr;
  lv_obj_t* wifiLbl = nullptr;
  lv_obj_t* battLbl = nullptr;
  lv_timer_t* timer = nullptr;
  uint32_t ticks = 0;
};

bool gPaused = false;

int readBatteryPercent() {
  static bool adcReady = false;
  if (!adcReady) {
    pinMode(PIN_BATTERY, INPUT);
    analogSetPinAttenuation(PIN_BATTERY, ADC_11db);
    adcReady = true;
  }
  uint32_t mv = 0;
  for (int i = 0; i < 8; i++) mv += analogReadMilliVolts(PIN_BATTERY);
  float volts = (mv / 8.0f) * 2.0f / 1000.0f;  // divisor 1:1 en la placa
  if (volts < 2.8f) return -1;
  int pct = (int)((volts - 3.3f) / (4.2f - 3.3f) * 100.0f);
  return constrain(pct, 0, 100);
}

void refreshBar(BarRefs* r) {
  if (!r) return;

  lv_label_set_text(r->time, TimeService::format("%H:%M").c_str());

  if (wifi.connected()) {
    lv_obj_set_style_text_color(r->wifiLbl, color(col::OK), 0);
    lv_label_set_text(r->wifiLbl, LV_SYMBOL_WIFI);
  } else {
    lv_obj_set_style_text_color(r->wifiLbl, color(col::MUTED), 0);
    lv_label_set_text(r->wifiLbl, LV_SYMBOL_WIFI);
  }

  if (r->ticks % 5 == 0) {
    int pct = readBatteryPercent();
    if (pct < 0) {
      lv_label_set_text(r->battLbl, "--");
      lv_obj_set_style_text_color(r->battLbl, color(col::MUTED), 0);
    } else {
      char buf[8];
      snprintf(buf, sizeof(buf), "%d%%", pct);
      lv_label_set_text(r->battLbl, buf);
      uint32_t c = pct > 40 ? col::OK : (pct > 15 ? col::WARN : col::DANGER);
      lv_obj_set_style_text_color(r->battLbl, color(c), 0);
    }
  }
  r->ticks++;
}

void barTimer(lv_timer_t* t) {
  if (gPaused) return;
  refreshBar(static_cast<BarRefs*>(t->user_data));
}

void barDeleted(lv_event_t* e) {
  BarRefs* r = static_cast<BarRefs*>(lv_event_get_user_data(e));
  if (!r) return;
  if (r->timer) lv_timer_del(r->timer);
  delete r;
}

void backClicked(lv_event_t* e) {
  (void)e;
  home();
}

}  // namespace

lv_obj_t* createStatusBar(lv_obj_t* parent, const char* title, bool withBack) {
  lv_obj_t* bar = lv_obj_create(parent);
  lv_obj_remove_style_all(bar);
  lv_obj_set_size(bar, UI_SCREEN_W, UI_STATUSBAR_H);
  lv_obj_set_pos(bar, 0, 0);
  lv_obj_set_style_bg_color(bar, color(0x0A1220), 0);
  lv_obj_set_style_bg_opa(bar, LV_OPA_80, 0);
  lv_obj_set_style_border_color(bar, color(0x1A2942), 0);
  lv_obj_set_style_border_width(bar, 1, 0);
  lv_obj_set_style_border_side(bar, LV_BORDER_SIDE_BOTTOM, 0);
  lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

  int titleX = 10;

  if (withBack) {
    lv_obj_t* back = lv_label_create(bar);
    lv_label_set_text(back, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_font(back, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(back, color(col::TEXT), 0);
    lv_obj_set_pos(back, 6, 2);
    lv_obj_add_flag(back, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(back, 8);
    lv_obj_add_event_cb(back, backClicked, LV_EVENT_CLICKED, nullptr);
    titleX = 26;
  }

  lv_obj_t* titleLbl = makeLabel(bar, title, &ui_font_12, col::MUTED);
  lv_obj_set_pos(titleLbl, titleX, 6);

  lv_obj_t* wifiLbl = makeLabel(bar, LV_SYMBOL_WIFI, &ui_font_12,
                                col::MUTED);
  lv_obj_align(wifiLbl, LV_ALIGN_RIGHT_MID, -84, 0);

  lv_obj_t* battLbl = makeLabel(bar, "--", &ui_font_12, col::MUTED);
  lv_obj_align(battLbl, LV_ALIGN_RIGHT_MID, -46, 0);

  lv_obj_t* timeLbl = makeLabel(bar, "00:00", &ui_font_14, col::TEXT);
  lv_obj_align(timeLbl, LV_ALIGN_RIGHT_MID, -8, 0);

  BarRefs* refs = new BarRefs();
  refs->time = timeLbl;
  refs->wifiLbl = wifiLbl;
  refs->battLbl = battLbl;
  refs->timer = lv_timer_create(barTimer, 1000, refs);

  lv_obj_add_event_cb(bar, barDeleted, LV_EVENT_DELETE, refs);
  refreshBar(refs);
  return bar;
}

void setChromePaused(bool paused) { gPaused = paused; }
bool chromePaused() { return gPaused; }

}  // namespace ui
