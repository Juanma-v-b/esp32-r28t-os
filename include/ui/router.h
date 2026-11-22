#pragma once

#include <lvgl.h>

namespace ui {

enum class AppId {
  Launcher,
  Clock,
  Weather,
  Gallery,
  Settings,
  Wifi,
};

/** Navegación diferida (segura desde callbacks de eventos LVGL). */
void open(AppId id);
void home();
AppId current();

// Interno
void routerBegin();
void routerLoop();

}  // namespace ui
