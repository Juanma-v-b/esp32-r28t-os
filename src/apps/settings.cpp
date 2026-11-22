#include "apps/settings.h"

#include <Arduino.h>
#include <math.h>

#include "config.h"
#include "core/settings.h"
#include "core/time_service.h"
#include "core/weather_service.h"
#include "core/wifi_service.h"
#include "hal/display.h"
#include "hal/rgb_led.h"
#include "hal/touch.h"
#include "ui/router.h"
#include "ui/status_bar.h"
#include "ui/theme.h"

namespace apps {
namespace {

const char* kThemeNames[CLOCK_THEME_COUNT] = {"Aurora", "Medianoche", "Atardecer"};

lv_obj_t* sectionLabel(lv_obj_t* parent, const char* text) {
  lv_obj_t* label = ui::makeLabel(parent, text, &ui_font_10,
                                  ui::col::ACCENT);
  lv_obj_set_style_pad_top(label, 4, 0);
  return label;
}

lv_obj_t* rowCard(lv_obj_t* parent) {
  lv_obj_t* row = lv_obj_create(parent);
  lv_obj_remove_style_all(row);
  lv_obj_set_size(row, 220, 44);
  lv_obj_set_style_bg_color(row, ui::color(ui::col::CARD), 0);
  lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(row, 12, 0);
  lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
  return row;
}

lv_obj_t* valueRow(lv_obj_t* parent, const char* title, const String& value,
                   lv_event_cb_t cb, void* userData) {
  lv_obj_t* row = rowCard(parent);
  lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(row, cb, LV_EVENT_CLICKED, userData);

  lv_obj_t* label = ui::makeLabel(row, title, &ui_font_12,
                                  ui::col::TEXT);
  lv_obj_set_pos(label, 10, 14);

  lv_obj_t* val = ui::makeLabel(row, value.c_str(), &ui_font_10,
                                ui::col::MUTED);
  lv_obj_set_width(val, 120);
  lv_obj_set_pos(val, 90, 15);
  lv_obj_set_style_text_align(val, LV_TEXT_ALIGN_RIGHT, 0);
  lv_label_set_long_mode(val, LV_LABEL_LONG_DOT);
  return row;
}

lv_obj_t* colorButton(lv_obj_t* parent, int x, const char* text, uint32_t bg,
                      uintptr_t id, lv_event_cb_t cb, void* userData) {
  lv_obj_t* btn = lv_obj_create(parent);
  lv_obj_remove_style_all(btn);
  lv_obj_set_pos(btn, x, 7);
  lv_obj_set_size(btn, 48, 30);
  lv_obj_set_style_bg_color(btn, ui::color(bg), 0);
  lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(btn, 8, 0);
  lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_user_data(btn, reinterpret_cast<void*>(id));
  lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, userData);

  lv_obj_t* label = ui::makeLabel(btn, text, &ui_font_10, 0xFFFFFF);
  lv_obj_center(label);
  return btn;
}

}  // namespace

void SettingsApp::build(lv_obj_t* screen) {
  screen_ = screen;
  ui::styleScreen(screen_);
  ui::createStatusBar(screen_, title(), true);

  content_ = lv_obj_create(screen_);
  lv_obj_remove_style_all(content_);
  lv_obj_set_pos(content_, 0, UI_STATUSBAR_H);
  lv_obj_set_size(content_, UI_SCREEN_W, UI_SCREEN_H - UI_STATUSBAR_H);
  lv_obj_set_style_bg_opa(content_, LV_OPA_TRANSP, 0);
  lv_obj_set_style_pad_all(content_, 10, 0);
  lv_obj_set_style_pad_row(content_, 7, 0);
  lv_obj_set_flex_flow(content_, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_scrollbar_mode(content_, LV_SCROLLBAR_MODE_AUTO);

  renderContent();
}

void SettingsApp::renderContent() {
  if (!content_) return;
  lv_obj_clean(content_);
  brightnessLbl_ = nullptr;

  sectionLabel(content_, "PANTALLA");

  lv_obj_t* bright = rowCard(content_);
  lv_obj_set_height(bright, 56);
  lv_obj_t* brightTitle = ui::makeLabel(bright, "Brillo",
                                        &ui_font_12, ui::col::TEXT);
  lv_obj_set_pos(brightTitle, 10, 7);

  brightnessLbl_ = ui::makeLabel(bright, "", &ui_font_10,
                                 ui::col::MUTED);
  lv_obj_set_width(brightnessLbl_, 40);
  lv_obj_set_pos(brightnessLbl_, 170, 8);
  lv_obj_set_style_text_align(brightnessLbl_, LV_TEXT_ALIGN_RIGHT, 0);

  lv_obj_t* slider = lv_slider_create(bright);
  lv_obj_set_size(slider, 190, 14);
  lv_obj_set_pos(slider, 10, 32);
  lv_slider_set_range(slider, 10, 100);
  lv_slider_set_value(slider, settings.brightness(), LV_ANIM_OFF);
  lv_obj_set_style_bg_color(slider, ui::color(ui::col::BORDER), LV_PART_MAIN);
  lv_obj_set_style_bg_color(slider, ui::color(ui::col::ACCENT),
                            LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(slider, ui::color(ui::col::TEXT), LV_PART_KNOB);
  lv_obj_add_event_cb(slider, brightnessChanged, LV_EVENT_VALUE_CHANGED, this);
  lv_obj_add_event_cb(slider, brightnessChanged, LV_EVENT_RELEASED, this);

  sectionLabel(content_, "RELOJ");
  valueRow(content_, "Ambiente", kThemeNames[settings.clockTheme()],
           themeClicked, this);
  valueRow(content_, "Zona horaria", settings.tzLabel(), timezoneClicked, this);

  sectionLabel(content_, "TIEMPO");
  valueRow(content_, "Ciudad", settings.city(), cityClicked, this);
  valueRow(content_, "Latitud", String(settings.lat(), 4), latClicked, this);
  valueRow(content_, "Longitud", String(settings.lon(), 4), lonClicked, this);

  sectionLabel(content_, "CONECTIVIDAD");
  valueRow(content_, "WiFi",
           wifi.hasSaved() ? wifi.savedSsid() : "Sin configurar", wifiClicked,
           this);

  sectionLabel(content_, "TÁCTIL");
  valueRow(content_, "Calibrar pantalla", "Toca 4 puntos", calibrateClicked,
           this);

  sectionLabel(content_, "LED RGB");
  lv_obj_t* ledRow = rowCard(content_);
  colorButton(ledRow, 7, "Rojo", 0xB91C1C, 0, ledClicked, this);
  colorButton(ledRow, 59, "Verde", 0x15803D, 1, ledClicked, this);
  colorButton(ledRow, 111, "Azul", 0x1D4ED8, 2, ledClicked, this);
  colorButton(ledRow, 163, "Off", 0x334155, 3, ledClicked, this);

  sectionLabel(content_, "ACERCA DE");
  lv_obj_t* about = rowCard(content_);
  lv_obj_set_height(about, 64);

  lv_obj_t* line1 = ui::makeLabel(about, FW_NAME " v" FW_VERSION,
                                  &ui_font_12, ui::col::TEXT);
  lv_obj_set_pos(line1, 10, 9);

  lv_obj_t* line2 = ui::makeLabel(about,
                                  "ESP32-WROOM-32E · ST7789 240x320",
                                  &ui_font_10, ui::col::MUTED);
  lv_obj_set_pos(line2, 10, 27);

  lv_obj_t* line3 = ui::makeLabel(about, "LVGL 8.4 · PlatformIO · Arduino",
                                  &ui_font_10, ui::col::MUTED);
  lv_obj_set_pos(line3, 10, 42);

  if (brightnessLbl_) {
    lv_label_set_text_fmt(brightnessLbl_, "%d%%", settings.brightness());
  }
}

void SettingsApp::scheduleRebuild() {
  if (rebuildTimer_) return;
  rebuildTimer_ = lv_timer_create(rebuildLater, 30, this);
  lv_timer_set_repeat_count(rebuildTimer_, 1);
}

void SettingsApp::rebuildLater(lv_timer_t* t) {
  auto* self = static_cast<SettingsApp*>(t->user_data);
  if (!self) return;
  self->rebuildTimer_ = nullptr;
  self->renderContent();
}

void SettingsApp::brightnessChanged(lv_event_t* e) {
  auto* self = static_cast<SettingsApp*>(lv_event_get_user_data(e));
  lv_obj_t* slider = lv_event_get_target(e);
  const int value = lv_slider_get_value(slider);

  display.setBrightness((uint8_t)value);
  if (self && self->brightnessLbl_) {
    lv_label_set_text_fmt(self->brightnessLbl_, "%d%%", value);
  }
  if (lv_event_get_code(e) == LV_EVENT_RELEASED) {
    settings.setBrightness((uint8_t)value);
  }
}

void SettingsApp::themeClicked(lv_event_t* e) {
  auto* self = static_cast<SettingsApp*>(lv_event_get_user_data(e));
  if (!self) return;
  settings.setClockTheme((settings.clockTheme() + 1) % CLOCK_THEME_COUNT);
  self->scheduleRebuild();
}

void SettingsApp::timezoneClicked(lv_event_t* e) {
  auto* self = static_cast<SettingsApp*>(lv_event_get_user_data(e));
  if (!self) return;
  settings.setTzIndex(settings.tzIndex() == 0 ? 1 : 0);
  timeService.applyTimezone();
  self->scheduleRebuild();
}

void SettingsApp::wifiClicked(lv_event_t* e) {
  (void)e;
  ui::open(ui::AppId::Wifi);
}

void SettingsApp::cityClicked(lv_event_t* e) {
  auto* self = static_cast<SettingsApp*>(lv_event_get_user_data(e));
  if (!self) return;
  self->prompt_.show("Ciudad", settings.city(), false, false,
                     [self](const String& value) {
                       settings.setCity(value);
                       weather.invalidate();
                       self->scheduleRebuild();
                     });
}

void SettingsApp::latClicked(lv_event_t* e) {
  auto* self = static_cast<SettingsApp*>(lv_event_get_user_data(e));
  if (!self) return;
  self->prompt_.show(
      "Latitud (-90 a 90)", String(settings.lat(), 4), true, false,
      [self](const String& value) {
        const float lat = value.toFloat();
        if (lat >= -90.0f && lat <= 90.0f && !(lat == 0.0f && value != "0")) {
          settings.setLat(lat);
          weather.invalidate();
        }
        self->scheduleRebuild();
      });
}

void SettingsApp::lonClicked(lv_event_t* e) {
  auto* self = static_cast<SettingsApp*>(lv_event_get_user_data(e));
  if (!self) return;
  self->prompt_.show(
      "Longitud (-180 a 180)", String(settings.lon(), 4), true, false,
      [self](const String& value) {
        const float lon = value.toFloat();
        if (lon >= -180.0f && lon <= 180.0f) {
          settings.setLon(lon);
          weather.invalidate();
        }
        self->scheduleRebuild();
      });
}

void SettingsApp::ledClicked(lv_event_t* e) {
  lv_obj_t* target = lv_event_get_target(e);
  const uintptr_t id =
      reinterpret_cast<uintptr_t>(lv_obj_get_user_data(target));
  switch (id) {
    case 0:
      rgbLed.set(90, 0, 0);
      break;
    case 1:
      rgbLed.set(0, 90, 0);
      break;
    case 2:
      rgbLed.set(0, 0, 90);
      break;
    default:
      rgbLed.off();
      break;
  }
}

void SettingsApp::calibrateClicked(lv_event_t* e) {
  auto* self = static_cast<SettingsApp*>(lv_event_get_user_data(e));
  if (self) self->openCalibration();
}

void SettingsApp::openCalibration() {
  if (calOverlay_) return;

  calStep_ = 0;

  calOverlay_ = lv_obj_create(lv_layer_top());
  lv_obj_remove_style_all(calOverlay_);
  lv_obj_set_size(calOverlay_, UI_SCREEN_W, UI_SCREEN_H);
  lv_obj_set_pos(calOverlay_, 0, 0);
  lv_obj_set_style_bg_color(calOverlay_, ui::color(0x04070E), 0);
  lv_obj_set_style_bg_opa(calOverlay_, LV_OPA_COVER, 0);
  lv_obj_clear_flag(calOverlay_, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(calOverlay_, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(calOverlay_, calibrationPressed, LV_EVENT_PRESSED, this);

  calLabel_ = ui::makeLabel(calOverlay_, "Toca el punto 1 de 4",
                            &ui_font_14, ui::col::TEXT);
  lv_obj_align(calLabel_, LV_ALIGN_TOP_MID, 0, 8);

  lv_obj_t* hint = ui::makeLabel(calOverlay_,
                                 "Sigue el círculo con el dedo, sin arrastrar",
                                 &ui_font_10, ui::col::MUTED);
  lv_obj_align(hint, LV_ALIGN_TOP_MID, 0, 30);

  calTarget_ = lv_obj_create(calOverlay_);
  lv_obj_remove_style_all(calTarget_);
  lv_obj_set_size(calTarget_, 24, 24);
  lv_obj_set_style_bg_opa(calTarget_, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(calTarget_, 3, 0);
  lv_obj_set_style_border_color(calTarget_, ui::color(ui::col::ACCENT), 0);
  lv_obj_set_style_radius(calTarget_, LV_RADIUS_CIRCLE, 0);
  lv_obj_clear_flag(calTarget_, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_pos(calTarget_, 24 - 12, 64 - 12);
}

void SettingsApp::calibrationPressed(lv_event_t* e) {
  auto* self = static_cast<SettingsApp*>(lv_event_get_user_data(e));
  if (!self || !self->calOverlay_) return;

  if (self->calStep_ >= 4) {
    self->closeCalibration();
    return;
  }

  const uint16_t rx = touch.lastRawX();
  const uint16_t ry = touch.lastRawY();
  if (rx == 0 && ry == 0) return;

  self->calRawX_[self->calStep_] = rx;
  self->calRawY_[self->calStep_] = ry;
  self->calStep_++;

  if (self->calStep_ < 4) {
    static const int xs[4] = {24, 216, 24, 216};
    static const int ys[4] = {64, 64, 256, 256};
    lv_obj_set_pos(self->calTarget_, xs[self->calStep_] - 12,
                   ys[self->calStep_] - 12);
    lv_label_set_text_fmt(self->calLabel_, "Toca el punto %d de 4",
                          self->calStep_ + 1);
    return;
  }

  TouchCalibration calibration;
  calibration.xMin = (self->calRawX_[0] + self->calRawX_[2]) / 2;
  calibration.xMax = (self->calRawX_[1] + self->calRawX_[3]) / 2;
  calibration.yMin = (self->calRawY_[0] + self->calRawY_[1]) / 2;
  calibration.yMax = (self->calRawY_[2] + self->calRawY_[3]) / 2;

  if (abs(calibration.xMax - calibration.xMin) < 200 ||
      abs(calibration.yMax - calibration.yMin) < 200) {
    lv_label_set_text(self->calLabel_,
                      "Calibración no válida. Toca para reintentar");
    self->calStep_ = 0;
    lv_obj_set_pos(self->calTarget_, 24 - 12, 64 - 12);
    return;
  }

  touch.saveCalibration(calibration);
  lv_obj_add_flag(self->calTarget_, LV_OBJ_FLAG_HIDDEN);
  lv_label_set_text(self->calLabel_,
                    "Calibración guardada. Toca para salir");
}

void SettingsApp::closeCalibration() {
  if (!calOverlay_) return;
  lv_obj_t* old = calOverlay_;
  calOverlay_ = nullptr;
  calTarget_ = nullptr;
  calLabel_ = nullptr;
  lv_obj_del_async(old);
}

void SettingsApp::onDestroy() {
  prompt_.hide();
  if (rebuildTimer_) {
    lv_timer_del(rebuildTimer_);
    rebuildTimer_ = nullptr;
  }
  closeCalibration();
}

}  // namespace apps
