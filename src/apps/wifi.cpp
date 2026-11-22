#include "apps/wifi.h"

#include <Arduino.h>

#include "config.h"
#include "ui/router.h"
#include "ui/status_bar.h"
#include "ui/theme.h"

namespace apps {
namespace {

lv_obj_t* button(lv_obj_t* parent, const char* text, int x, int y, int w, int h,
                 lv_event_cb_t cb, void* userData, uint32_t bg,
                 uint32_t fg) {
  lv_obj_t* btn = lv_obj_create(parent);
  lv_obj_remove_style_all(btn);
  lv_obj_set_pos(btn, x, y);
  lv_obj_set_size(btn, w, h);
  lv_obj_set_style_bg_color(btn, ui::color(bg), 0);
  lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(btn, 10, 0);
  lv_obj_set_style_border_width(btn, 0, 0);
  lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, userData);

  lv_obj_t* label = ui::makeLabel(btn, text, &ui_font_12, fg);
  lv_obj_center(label);
  return btn;
}

String rssiText(int32_t rssi) {
  char buf[16];
  snprintf(buf, sizeof(buf), "%d dBm", (int)rssi);
  return String(buf);
}

}  // namespace

void WifiApp::build(lv_obj_t* screen) {
  screen_ = screen;
  ui::styleScreen(screen_);
  ui::createStatusBar(screen_, title(), true);

  lv_obj_t* content = lv_obj_create(screen_);
  lv_obj_remove_style_all(content);
  lv_obj_set_pos(content, 0, UI_STATUSBAR_H);
  lv_obj_set_size(content, UI_SCREEN_W, UI_SCREEN_H - UI_STATUSBAR_H);
  lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);

  statusCard_ = lv_obj_create(content);
  lv_obj_remove_style_all(statusCard_);
  lv_obj_set_pos(statusCard_, 8, 8);
  lv_obj_set_size(statusCard_, 224, 106);
  lv_obj_set_style_bg_color(statusCard_, ui::color(ui::col::CARD), 0);
  lv_obj_set_style_bg_opa(statusCard_, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(statusCard_, 1, 0);
  lv_obj_set_style_border_color(statusCard_, ui::color(ui::col::BORDER), 0);
  lv_obj_set_style_radius(statusCard_, 14, 0);
  lv_obj_set_style_pad_all(statusCard_, 10, 0);
  lv_obj_clear_flag(statusCard_, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t* netsLbl = ui::makeLabel(content, "Redes cercanas",
                                    &ui_font_12, ui::col::MUTED);
  lv_obj_set_pos(netsLbl, 10, 122);

  scanBtn_ = button(content, "Escanear", 150, 116, 82, 26, scanClicked, this,
                    ui::col::ACCENT, 0x06231F);

  list_ = lv_obj_create(content);
  lv_obj_remove_style_all(list_);
  lv_obj_set_pos(list_, 8, 146);
  lv_obj_set_size(list_, 224, 140);
  lv_obj_set_style_bg_opa(list_, LV_OPA_TRANSP, 0);
  lv_obj_set_flex_flow(list_, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_row(list_, 6, 0);
  lv_obj_set_style_pad_all(list_, 0, 0);
  lv_obj_set_scrollbar_mode(list_, LV_SCROLLBAR_MODE_AUTO);

  lastState_ = wifi.state();
  rebuildStatus();
  rebuildNetworks();
  startScan();

  timer_ = lv_timer_create(statusTimer, 1000, this);
}

void WifiApp::rebuildStatus() {
  if (!statusCard_) return;
  lv_obj_clean(statusCard_);

  const WifiState state = wifi.state();
  const bool connected = state == WifiState::Connected;

  String ssid = connected ? wifi.ssid() : wifi.savedSsid();
  if (ssid.isEmpty()) ssid = "Sin red configurada";

  lv_obj_t* ssidLbl = ui::makeLabel(statusCard_, ssid.c_str(),
                                    &ui_font_16, ui::col::TEXT);
  lv_obj_set_width(ssidLbl, 202);
  lv_label_set_long_mode(ssidLbl, LV_LABEL_LONG_DOT);
  lv_obj_set_pos(ssidLbl, 0, 0);

  uint32_t stateColor = ui::col::MUTED;
  if (state == WifiState::Connected) stateColor = ui::col::OK;
  else if (state == WifiState::Connecting) stateColor = ui::col::WARN;
  else if (state == WifiState::Failed) stateColor = ui::col::DANGER;

  lv_obj_t* stateLbl = ui::makeLabel(statusCard_, wifi.stateText().c_str(),
                                     &ui_font_12, stateColor);
  lv_obj_set_pos(stateLbl, 0, 24);

  String ipText = connected ? ("IP " + wifi.ip()) : "Introduce una red para empezar";
  lv_obj_t* ipLbl = ui::makeLabel(statusCard_, ipText.c_str(),
                                  &ui_font_10, ui::col::MUTED);
  lv_obj_set_pos(ipLbl, 0, 42);

  if (connected) {
    button(statusCard_, "Desconectar", 0, 64, 98, 28, disconnectClicked, this,
           0x3A1D28, ui::col::DANGER);
    button(statusCard_, "Olvidar", 106, 64, 98, 28, forgetClicked, this,
           ui::col::CARD_HOVER, ui::col::MUTED);
  } else {
    button(statusCard_, "Buscar redes", 0, 64, 98, 28, scanClicked, this,
           ui::col::ACCENT, 0x06231F);
    if (wifi.hasSaved()) {
      button(statusCard_, "Olvidar", 106, 64, 98, 28, forgetClicked, this,
             ui::col::CARD_HOVER, ui::col::MUTED);
    }
  }
}

void WifiApp::rebuildNetworks() {
  if (!list_) return;
  lv_obj_clean(list_);

  if (scanning_) {
    lv_obj_t* spinner = lv_spinner_create(list_, 1000, 60);
    lv_obj_set_size(spinner, 34, 34);
    lv_obj_set_style_arc_color(spinner, ui::color(ui::col::ACCENT),
                               LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(spinner, 4, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(spinner, ui::color(ui::col::BORDER),
                               LV_PART_MAIN);
    lv_obj_t* label = ui::makeLabel(list_, "Buscando redes…",
                                    &ui_font_12, ui::col::MUTED);
    lv_obj_set_style_pad_top(label, 6, 0);
    return;
  }

  if (networks_.empty()) {
    const char* text = scanned_ ? "No se han encontrado redes"
                                : "Toca Escanear para buscar redes";
    ui::makeLabel(list_, text, &ui_font_12, ui::col::MUTED);
    return;
  }

  for (size_t i = 0; i < networks_.size(); i++) {
    const WifiNetwork& net = networks_[i];

    lv_obj_t* row = lv_obj_create(list_);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, 220, 42);
    lv_obj_set_style_bg_color(row, ui::color(ui::col::CARD), 0);
    lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(row, 10, 0);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_user_data(row, reinterpret_cast<void*>(i));
    lv_obj_add_event_cb(row, networkClicked, LV_EVENT_CLICKED, this);

    lv_obj_t* name = ui::makeLabel(row, net.ssid.c_str(),
                                   &ui_font_14, ui::col::TEXT);
    lv_obj_set_width(name, 145);
    lv_label_set_long_mode(name, LV_LABEL_LONG_DOT);
    lv_obj_set_pos(name, 10, 11);

    lv_obj_t* info = ui::makeLabel(row, rssiText(net.rssi).c_str(),
                                   &ui_font_10, ui::col::MUTED);
    lv_obj_align(info, LV_ALIGN_RIGHT_MID, -10, 0);

    if (net.secure) {
      lv_obj_t* lock = ui::makeLabel(row, "WPA",
                                     &ui_font_10, ui::col::MUTED);
      lv_obj_align(lock, LV_ALIGN_RIGHT_MID, -62, 0);
    }
  }
}

void WifiApp::startScan() {
  if (scanning_) return;
  scanning_ = true;
  scanned_ = false;
  rebuildNetworks();

  scanTimer_ = lv_timer_create(scanLater, 60, this);
  lv_timer_set_repeat_count(scanTimer_, 1);
}

void WifiApp::scanLater(lv_timer_t* t) {
  auto* self = static_cast<WifiApp*>(t->user_data);
  if (!self) return;
  self->scanTimer_ = nullptr;
  self->doScan();
}

void WifiApp::doScan() {
  networks_ = wifi.scan();
  scanning_ = false;
  scanned_ = true;
  rebuildNetworks();
}

void WifiApp::scanClicked(lv_event_t* e) {
  auto* self = static_cast<WifiApp*>(lv_event_get_user_data(e));
  if (self) self->startScan();
}

void WifiApp::networkClicked(lv_event_t* e) {
  auto* self = static_cast<WifiApp*>(lv_event_get_user_data(e));
  if (!self) return;

  lv_obj_t* target = lv_event_get_target(e);
  const size_t index =
      reinterpret_cast<uintptr_t>(lv_obj_get_user_data(target));
  if (index >= self->networks_.size()) return;

  const WifiNetwork net = self->networks_[index];

  if (!net.secure) {
    wifi.connect(net.ssid, "");
    self->message_ = "Conectando…";
    self->rebuildStatus();
    return;
  }

  const String ssid = net.ssid;
  String title = "Contraseña: ";
  title += ssid;
  self->prompt_.show(
      title.c_str(), "", false, true, [self, ssid](const String& password) {
        wifi.connect(ssid, password);
        self->message_ = "Conectando…";
        self->rebuildStatus();
      });
}

void WifiApp::disconnectClicked(lv_event_t* e) {
  (void)e;
  wifi.disconnect();
}

void WifiApp::forgetClicked(lv_event_t* e) {
  (void)e;
  wifi.forget();
}

void WifiApp::statusTimer(lv_timer_t* t) {
  auto* self = static_cast<WifiApp*>(t->user_data);
  if (!self) return;

  if (wifi.state() != self->lastState_) {
    self->lastState_ = wifi.state();
    self->rebuildStatus();
  }
}

void WifiApp::onDestroy() {
  prompt_.hide();
  if (timer_) {
    lv_timer_del(timer_);
    timer_ = nullptr;
  }
  if (scanTimer_) {
    lv_timer_del(scanTimer_);
    scanTimer_ = nullptr;
  }
}

}  // namespace apps
