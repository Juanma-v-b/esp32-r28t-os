#include "apps/launcher.h"

#include <Arduino.h>
#include <time.h>

#include "config.h"
#include "core/time_service.h"
#include "ui/icons.h"
#include "ui/router.h"
#include "ui/status_bar.h"
#include "ui/theme.h"

namespace apps {
namespace {

struct Entry {
  ui::AppId id;
  const char* label;
};

const Entry kEntries[] = {
    {ui::AppId::Clock, "Reloj"},
    {ui::AppId::Weather, "Tiempo"},
    {ui::AppId::Gallery, "Galería"},
    {ui::AppId::Settings, "Ajustes"},
};

void addIcon(lv_obj_t* card, ui::AppId id) {
  lv_obj_t* icon = nullptr;
  switch (id) {
    case ui::AppId::Clock:
      icon = ui::iconClock(card, 46);
      break;
    case ui::AppId::Weather:
      icon = ui::iconWeather(card, 46, 1);
      break;
    case ui::AppId::Gallery:
      icon = ui::iconGallery(card, 46);
      break;
    case ui::AppId::Settings:
      icon = ui::iconSettings(card, 46);
      break;
    default:
      return;
  }
  lv_obj_align(icon, LV_ALIGN_TOP_MID, 0, 9);
}

String greeting() {
  time_t now = time(nullptr);
  struct tm local;
  localtime_r(&now, &local);
  if (local.tm_hour < 6) return "Buenas noches";
  if (local.tm_hour < 12) return "Buenos días";
  if (local.tm_hour < 20) return "Buenas tardes";
  return "Buenas noches";
}

String longDate() {
  if (!timeService.synced()) return "Conecta el WiFi para la hora";

  static const char* weekdays[] = {"domingo", "lunes", "martes", "miércoles",
                                   "jueves", "viernes", "sábado"};
  static const char* months[] = {"enero", "febrero", "marzo", "abril", "mayo",
                                 "junio", "julio", "agosto", "septiembre",
                                 "octubre", "noviembre", "diciembre"};
  time_t now = time(nullptr);
  struct tm local;
  localtime_r(&now, &local);
  char buf[80];
  snprintf(buf, sizeof(buf), "%s, %d de %s", weekdays[local.tm_wday % 7],
           local.tm_mday, months[local.tm_mon % 12]);
  return String(buf);
}

}  // namespace

void LauncherApp::build(lv_obj_t* screen) {
  ui::styleScreen(screen);
  ui::createStatusBar(screen, FW_NAME, false);

  lv_obj_t* hello = ui::makeLabel(screen, greeting().c_str(),
                                  &ui_font_20, ui::col::TEXT);
  lv_obj_align(hello, LV_ALIGN_TOP_MID, 0, 39);

  lv_obj_t* date = ui::makeLabel(screen, longDate().c_str(),
                                 &ui_font_12, ui::col::MUTED);
  lv_obj_align(date, LV_ALIGN_TOP_MID, 0, 68);

  constexpr int kCardW = 104;
  constexpr int kCardH = 104;
  constexpr int kGap = 8;
  constexpr int kX0 = 12;
  constexpr int kY0 = 94;

  for (int i = 0; i < 4; i++) {
    const int col = i % 2;
    const int row = i / 2;

    lv_obj_t* card = lv_obj_create(screen);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(card, kCardW, kCardH);
    lv_obj_set_pos(card, kX0 + col * (kCardW + kGap),
                   kY0 + row * (kCardH + kGap));
    lv_obj_set_style_bg_color(card, ui::color(ui::col::CARD), 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_border_color(card, ui::color(ui::col::BORDER), 0);
    lv_obj_set_style_radius(card, 18, 0);
    lv_obj_set_style_pad_all(card, 0, 0);
    lv_obj_set_style_bg_color(card, ui::color(ui::col::CARD_HOVER),
                              LV_STATE_PRESSED);
    lv_obj_set_style_border_color(card, ui::color(ui::col::ACCENT),
                                  LV_STATE_PRESSED);
    lv_obj_add_flag(card, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_user_data(card,
                         reinterpret_cast<void*>(static_cast<uintptr_t>(
                             static_cast<int>(kEntries[i].id))));
    lv_obj_add_event_cb(card, cardClicked, LV_EVENT_CLICKED, nullptr);

    addIcon(card, kEntries[i].id);

    lv_obj_t* label = ui::makeLabel(card, kEntries[i].label,
                                    &ui_font_14, ui::col::TEXT);
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -10);
  }

  lv_obj_t* footer = ui::makeLabel(screen, "v" FW_VERSION, &ui_font_10,
                                   ui::col::MUTED);
  lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, -3);
}

void LauncherApp::cardClicked(lv_event_t* e) {
  lv_obj_t* target = lv_event_get_target(e);
  const uintptr_t id =
      reinterpret_cast<uintptr_t>(lv_obj_get_user_data(target));
  ui::open(static_cast<ui::AppId>(id));
}

}  // namespace apps
