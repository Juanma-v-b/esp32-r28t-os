#include "apps/clock.h"

#include <Arduino.h>
#include <math.h>
#include <time.h>

#include "config.h"
#include "core/settings.h"
#include "core/time_service.h"
#include "core/weather_service.h"
#include "ui/status_bar.h"
#include "ui/theme.h"

namespace apps {
namespace {

struct ThemeDef {
  const char* name;
  uint32_t top;
  uint32_t bottom;
  uint32_t accent;
  uint32_t star;
  uint32_t moon;
};

const ThemeDef kThemes[CLOCK_THEME_COUNT] = {
    {"Aurora", 0x05121F, 0x0B3A52, 0x7FE7D4, 0xD8F7FF, 0xEAF8FF},
    {"Medianoche", 0x05060F, 0x24164A, 0x9C8CFF, 0xDDD6FF, 0xF1EEFF},
    {"Atardecer", 0x260D36, 0x83313D, 0xFFB36B, 0xFFE3BE, 0xFFF1D6},
};

const ThemeDef& theme() {
  return kThemes[settings.clockTheme() % CLOCK_THEME_COUNT];
}

}  // namespace

void ClockApp::build(lv_obj_t* screen) {
  screen_ = screen;
  ui::styleScreen(screen_);
  ui::createStatusBar(screen_, title(), true);

  bg_ = lv_obj_create(screen_);
  lv_obj_remove_style_all(bg_);
  lv_obj_set_pos(bg_, 0, UI_STATUSBAR_H);
  lv_obj_set_size(bg_, UI_SCREEN_W, UI_SCREEN_H - UI_STATUSBAR_H);
  lv_obj_clear_flag(bg_, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(bg_, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_bg_opa(bg_, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_grad_dir(bg_, LV_GRAD_DIR_VER, 0);
  lv_obj_add_event_cb(bg_, backgroundClicked, LV_EVENT_CLICKED, this);

  buildStars();

  moon_ = lv_obj_create(bg_);
  lv_obj_remove_style_all(moon_);
  lv_obj_set_size(moon_, 38, 38);
  lv_obj_set_pos(moon_, 178, 18);
  lv_obj_set_style_radius(moon_, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_opa(moon_, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(moon_, 5, 0);
  lv_obj_set_style_border_opa(moon_, LV_OPA_20, 0);
  lv_obj_clear_flag(moon_, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

  dateLbl_ = ui::makeLabel(bg_, "—", &ui_font_12, ui::col::MUTED);
  lv_obj_set_width(dateLbl_, 220);
  lv_obj_set_style_text_align(dateLbl_, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(dateLbl_, LV_ALIGN_TOP_MID, 0, 64);

  timeLbl_ = ui::makeLabel(bg_, "--:--", &lv_font_montserrat_48, ui::col::TEXT);
  lv_obj_set_width(timeLbl_, 220);
  lv_obj_set_style_text_align(timeLbl_, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(timeLbl_, LV_ALIGN_TOP_MID, 0, 92);

  secondsLbl_ = ui::makeLabel(bg_, "-- s", &ui_font_20,
                              ui::col::ACCENT);
  lv_obj_set_width(secondsLbl_, 120);
  lv_obj_set_style_text_align(secondsLbl_, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(secondsLbl_, LV_ALIGN_TOP_MID, 0, 158);

  weatherLbl_ = ui::makeLabel(bg_, "", &ui_font_12, ui::col::MUTED);
  lv_obj_set_width(weatherLbl_, 220);
  lv_obj_set_style_text_align(weatherLbl_, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(weatherLbl_, LV_ALIGN_TOP_MID, 0, 198);

  hintLbl_ = ui::makeLabel(bg_, "Toca el fondo para cambiar de ambiente",
                           &ui_font_10, ui::col::MUTED);
  lv_obj_align(hintLbl_, LV_ALIGN_BOTTOM_MID, 0, -8);

  applyTheme();
  updateClock();

  clockTimer_ = lv_timer_create(clockTick, 500, this);
  starTimer_ = lv_timer_create(starTick, 90, this);
}

void ClockApp::buildStars() {
  starCount_ = 30;
  for (int i = 0; i < starCount_; i++) {
    Star& star = stars_[i];
    star.size = (uint8_t)(1 + (rand() % 2));
    star.x = (uint16_t)(6 + rand() % (UI_SCREEN_W - 12));
    star.y = (uint16_t)(6 + rand() % (UI_SCREEN_H - UI_STATUSBAR_H - 12));
    star.phase = (uint16_t)(rand() % 628);

    lv_obj_t* dot = lv_obj_create(bg_);
    lv_obj_remove_style_all(dot);
    lv_obj_set_size(dot, star.size, star.size);
    lv_obj_set_pos(dot, star.x, star.y);
    lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(dot, LV_OPA_70, 0);
    lv_obj_set_style_bg_color(dot, ui::color(ui::col::TEXT), 0);
    lv_obj_clear_flag(dot, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    star.obj = dot;
  }
}

void ClockApp::applyTheme() {
  const ThemeDef& t = theme();
  lv_obj_set_style_bg_color(bg_, ui::color(t.top), 0);
  lv_obj_set_style_bg_grad_color(bg_, ui::color(t.bottom), 0);

  for (int i = 0; i < starCount_; i++) {
    if (stars_[i].obj) {
      lv_obj_set_style_bg_color(stars_[i].obj, ui::color(t.star), 0);
    }
  }

  if (moon_) {
    lv_obj_set_style_bg_color(moon_, ui::color(t.moon), 0);
    lv_obj_set_style_border_color(moon_, ui::color(t.accent), 0);
  }
  if (secondsLbl_) {
    lv_obj_set_style_text_color(secondsLbl_, ui::color(t.accent), 0);
  }
}

void ClockApp::updateClock() {
  if (!timeService.synced()) {
    lv_label_set_text(timeLbl_, "--:--");
    lv_label_set_text(secondsLbl_, "-- s");
    lv_label_set_text(dateLbl_, "Conecta el WiFi para la hora");
  } else {
    time_t now = time(nullptr);
    struct tm local;
    localtime_r(&now, &local);

    static const char* weekdays[] = {"domingo", "lunes", "martes", "miércoles",
                                     "jueves", "viernes", "sábado"};
    static const char* months[] = {"enero", "febrero", "marzo", "abril", "mayo",
                                   "junio", "julio", "agosto", "septiembre",
                                   "octubre", "noviembre", "diciembre"};

    char timeBuf[8];
    char secBuf[16];
    char dateBuf[80];
    strftime(timeBuf, sizeof(timeBuf), "%H:%M", &local);
    snprintf(secBuf, sizeof(secBuf), "%02d s", local.tm_sec);
    snprintf(dateBuf, sizeof(dateBuf), "%s, %d de %s",
             weekdays[local.tm_wday % 7], local.tm_mday,
             months[local.tm_mon % 12]);

    lv_label_set_text(timeLbl_, timeBuf);
    lv_label_set_text(secondsLbl_, secBuf);
    lv_label_set_text(dateLbl_, dateBuf);
  }

  const WeatherData& w = weather.data();
  if (w.valid && !isnan(w.temp)) {
    char buf[80];
    snprintf(buf, sizeof(buf), "%d° · %s", (int)lroundf(w.temp),
             WeatherService::describe(w.code));
    lv_label_set_text(weatherLbl_, buf);
  } else {
    lv_label_set_text(weatherLbl_, "");
  }
}

void ClockApp::clockTick(lv_timer_t* t) {
  auto* self = static_cast<ClockApp*>(t->user_data);
  if (self) self->updateClock();
}

void ClockApp::starTick(lv_timer_t* t) {
  auto* self = static_cast<ClockApp*>(t->user_data);
  if (!self) return;

  const float base = (float)millis() / 620.0f;
  for (int i = 0; i < self->starCount_; i++) {
    Star& star = self->stars_[i];
    if (!star.obj) continue;
    const float wave = sinf(base + (float)star.phase / 100.0f);
    const lv_opa_t opa = (lv_opa_t)(50 + (wave + 1.0f) * 75.0f);
    lv_obj_set_style_bg_opa(star.obj, opa, 0);
  }
}

void ClockApp::backgroundClicked(lv_event_t* e) {
  auto* self = static_cast<ClockApp*>(lv_event_get_user_data(e));
  if (!self) return;
  settings.setClockTheme((settings.clockTheme() + 1) % CLOCK_THEME_COUNT);
  self->applyTheme();
}

void ClockApp::onDestroy() {
  if (clockTimer_) {
    lv_timer_del(clockTimer_);
    clockTimer_ = nullptr;
  }
  if (starTimer_) {
    lv_timer_del(starTimer_);
    starTimer_ = nullptr;
  }
}

}  // namespace apps
