#include "apps/weather.h"

#include <Arduino.h>
#include <math.h>

#include "config.h"
#include "core/settings.h"
#include "core/time_service.h"
#include "core/weather_service.h"
#include "core/wifi_service.h"
#include "ui/icons.h"
#include "ui/router.h"
#include "ui/status_bar.h"
#include "ui/theme.h"

namespace apps {
namespace {

lv_obj_t* chip(lv_obj_t* parent, int x, int y, const String& value,
               const char* caption) {
  lv_obj_t* box = lv_obj_create(parent);
  lv_obj_remove_style_all(box);
  lv_obj_set_pos(box, x, y);
  lv_obj_set_size(box, 72, 46);
  lv_obj_set_style_bg_color(box, ui::color(0x0E1930), 0);
  lv_obj_set_style_bg_opa(box, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(box, 10, 0);
  lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t* val = ui::makeLabel(box, value.c_str(), &ui_font_14,
                                ui::col::TEXT);
  lv_obj_align(val, LV_ALIGN_TOP_MID, 0, 7);

  lv_obj_t* cap = ui::makeLabel(box, caption, &ui_font_10,
                                ui::col::MUTED);
  lv_obj_align(cap, LV_ALIGN_BOTTOM_MID, 0, -5);
  return box;
}

lv_obj_t* actionButton(lv_obj_t* parent, const char* text, int y,
                       lv_event_cb_t cb, void* data) {
  lv_obj_t* btn = lv_obj_create(parent);
  lv_obj_remove_style_all(btn);
  lv_obj_set_size(btn, 150, 34);
  lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, y);
  lv_obj_set_style_bg_color(btn, ui::color(ui::col::ACCENT), 0);
  lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(btn, 10, 0);
  lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, data);

  lv_obj_t* label = ui::makeLabel(btn, text, &ui_font_12, 0x06231F);
  lv_obj_center(label);
  return btn;
}

String temperature(float value) {
  if (isnan(value)) return "--";
  return String((int)lroundf(value)) + "°";
}

}  // namespace

void WeatherApp::build(lv_obj_t* screen) {
  screen_ = screen;
  ui::styleScreen(screen_);
  ui::createStatusBar(screen_, title(), true);

  content_ = lv_obj_create(screen_);
  lv_obj_remove_style_all(content_);
  lv_obj_set_pos(content_, 0, UI_STATUSBAR_H);
  lv_obj_set_size(content_, UI_SCREEN_W, UI_SCREEN_H - UI_STATUSBAR_H);
  lv_obj_clear_flag(content_, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_opa(content_, LV_OPA_TRANSP, 0);

  render();

  timer_ = lv_timer_create(autoTimer, 60000, this);
  if (wifi.connected() && weather.stale()) startFetch();
}

void WeatherApp::render() {
  if (!content_) return;
  lv_obj_clean(content_);

  // Botón de refresco (siempre visible, salvo en estado de carga).
  if (!fetching_) {
    lv_obj_t* refresh = lv_obj_create(content_);
    lv_obj_remove_style_all(refresh);
    lv_obj_set_pos(refresh, 202, 2);
    lv_obj_set_size(refresh, 30, 30);
    lv_obj_set_style_bg_color(refresh, ui::color(ui::col::CARD), 0);
    lv_obj_set_style_bg_opa(refresh, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(refresh, LV_RADIUS_CIRCLE, 0);
    lv_obj_clear_flag(refresh, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(refresh, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(refresh, refreshClicked, LV_EVENT_CLICKED, this);
    lv_obj_t* icon = ui::makeLabel(refresh, LV_SYMBOL_REFRESH,
                                   &ui_font_14, ui::col::ACCENT);
    lv_obj_center(icon);
  }

  if (fetching_) {
    lv_obj_t* spinner = lv_spinner_create(content_, 1000, 60);
    lv_obj_set_size(spinner, 46, 46);
    lv_obj_align(spinner, LV_ALIGN_TOP_MID, 0, 100);
    lv_obj_set_style_arc_color(spinner, ui::color(ui::col::ACCENT),
                               LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(spinner, 4, LV_PART_INDICATOR);

    lv_obj_t* label = ui::makeLabel(content_, "Consultando el cielo…",
                                    &ui_font_12, ui::col::MUTED);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 160);
    return;
  }

  if (!wifi.connected()) {
    lv_obj_t* icon = ui::makeLabel(content_, LV_SYMBOL_WIFI,
                                   &lv_font_montserrat_32, ui::col::MUTED);
    lv_obj_align(icon, LV_ALIGN_TOP_MID, 0, 78);

    lv_obj_t* titleLbl = ui::makeLabel(content_, "Sin conexión WiFi",
                                       &ui_font_16, ui::col::TEXT);
    lv_obj_align(titleLbl, LV_ALIGN_TOP_MID, 0, 126);

    lv_obj_t* hint = ui::makeLabel(content_,
                                   "Conecta una red para ver el tiempo",
                                   &ui_font_10, ui::col::MUTED);
    lv_obj_align(hint, LV_ALIGN_TOP_MID, 0, 150);

    actionButton(content_, "Abrir ajustes WiFi", 180, wifiClicked, this);
    return;
  }

  const WeatherData& data = weather.data();
  if (!data.valid) {
    lv_obj_t* icon = ui::iconWeather(content_, 54, 2);
    lv_obj_align(icon, LV_ALIGN_TOP_MID, 0, 46);

    lv_obj_t* titleLbl = ui::makeLabel(content_, "Aún no hay datos",
                                       &ui_font_16, ui::col::TEXT);
    lv_obj_align(titleLbl, LV_ALIGN_TOP_MID, 0, 112);

    String msg = error_.length() ? error_ : "Pulsa consultar para empezar";
    lv_obj_t* detail = ui::makeLabel(content_, msg.c_str(),
                                     &ui_font_10, ui::col::MUTED);
    lv_obj_set_width(detail, 210);
    lv_obj_set_style_text_align(detail, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(detail, LV_ALIGN_TOP_MID, 0, 136);

    actionButton(content_, "Consultar ahora", 180, refreshClicked, this);

    lv_obj_t* hint = ui::makeLabel(content_,
                                   "Cambia la ciudad en Ajustes › Tiempo",
                                   &ui_font_10, ui::col::MUTED);
    lv_obj_align(hint, LV_ALIGN_TOP_MID, 0, 226);
    return;
  }

  lv_obj_t* city = ui::makeLabel(content_, settings.city().c_str(),
                                 &ui_font_12, ui::col::MUTED);
  lv_obj_set_pos(city, 12, 8);

  lv_obj_t* temp = ui::makeLabel(content_, temperature(data.temp).c_str(),
                                 &lv_font_montserrat_48, ui::col::TEXT);
  lv_obj_set_pos(temp, 10, 22);

  lv_obj_t* condition = ui::makeLabel(content_,
                                      WeatherService::describe(data.code),
                                      &ui_font_12, ui::col::ACCENT);
  lv_obj_set_width(condition, 140);
  lv_label_set_long_mode(condition, LV_LABEL_LONG_WRAP);
  lv_obj_set_pos(condition, 12, 84);

  lv_obj_t* bigIcon = ui::iconWeather(content_, 66,
                                      WeatherService::iconType(data.code,
                                                               data.isDay));
  lv_obj_set_pos(bigIcon, 154, 28);

  chip(content_, 8, 118, temperature(data.feels), "Sensación");
  chip(content_, 84, 118,
       data.humidity >= 0 ? String(data.humidity) + "%" : "--", "Humedad");
  chip(content_, 160, 118,
       isnan(data.wind) ? "--" : String((int)lroundf(data.wind)), "km/h viento");

  lv_obj_t* forecastLbl = ui::makeLabel(content_, "Próximos días",
                                        &ui_font_10,
                                        ui::col::MUTED);
  lv_obj_set_pos(forecastLbl, 12, 178);

  for (int i = 0; i < data.dayCount && i < 3; i++) {
    const WeatherDay& day = data.days[i];
    const int y = 198 + i * 30;

    lv_obj_t* row = lv_obj_create(content_);
    lv_obj_remove_style_all(row);
    lv_obj_set_pos(row, 8, y);
    lv_obj_set_size(row, 224, 28);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* name = ui::makeLabel(row, WeatherService::weekdayShort(day.date),
                                   &ui_font_12, ui::col::TEXT);
    lv_obj_set_pos(name, 4, 6);

    lv_obj_t* icon = ui::iconWeather(row, 24,
                                     WeatherService::iconType(day.code, true));
    lv_obj_set_pos(icon, 58, 2);

    lv_obj_t* maxLbl = ui::makeLabel(row, temperature(day.tMax).c_str(),
                                     &ui_font_14, ui::col::TEXT);
    lv_obj_align(maxLbl, LV_ALIGN_RIGHT_MID, -54, 0);

    lv_obj_t* minLbl = ui::makeLabel(row, temperature(day.tMin).c_str(),
                                     &ui_font_14, ui::col::MUTED);
    lv_obj_align(minLbl, LV_ALIGN_RIGHT_MID, -10, 0);
  }

  uint32_t ageMin = (millis() - data.updatedAt) / 60000UL;
  String updated = ageMin < 1 ? "Actualizado ahora"
                              : ("Actualizado hace " + String(ageMin) + " min");
  lv_obj_t* footer = ui::makeLabel(content_, updated.c_str(),
                                   &ui_font_10, ui::col::MUTED);
  lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, -3);
}

void WeatherApp::startFetch() {
  if (fetching_) return;

  if (!wifi.connected()) {
    error_ = "Sin conexión WiFi";
    render();
    return;
  }

  fetching_ = true;
  error_ = "";
  render();

  if (fetchTimer_) return;
  fetchTimer_ = lv_timer_create(fetchLater, 60, this);
  lv_timer_set_repeat_count(fetchTimer_, 1);
}

void WeatherApp::fetchLater(lv_timer_t* t) {
  auto* self = static_cast<WeatherApp*>(t->user_data);
  if (!self) return;
  self->fetchTimer_ = nullptr;

  self->fetching_ = true;
  self->render();
  lv_refr_now(NULL);

  String error;
  const bool ok = weather.refresh(settings, error);

  self->fetching_ = false;
  self->error_ = ok ? "" : error;
  self->render();
}

void WeatherApp::refreshClicked(lv_event_t* e) {
  auto* self = static_cast<WeatherApp*>(lv_event_get_user_data(e));
  if (self) self->startFetch();
}

void WeatherApp::wifiClicked(lv_event_t* e) {
  (void)e;
  ui::open(ui::AppId::Wifi);
}

void WeatherApp::autoTimer(lv_timer_t* t) {
  auto* self = static_cast<WeatherApp*>(t->user_data);
  if (!self || self->fetching_) return;
  if (wifi.connected() && weather.stale()) self->startFetch();
}

void WeatherApp::onDestroy() {
  if (timer_) {
    lv_timer_del(timer_);
    timer_ = nullptr;
  }
  if (fetchTimer_) {
    lv_timer_del(fetchTimer_);
    fetchTimer_ = nullptr;
  }
}

}  // namespace apps
