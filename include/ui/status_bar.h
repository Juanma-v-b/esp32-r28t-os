#pragma once

#include <lvgl.h>

namespace ui {

/** Barra superior compartida: título, volver, WiFi, batería y hora. */
lv_obj_t* createStatusBar(lv_obj_t* parent, const char* title, bool withBack);

/** Pausa las actualizaciones periódicas (p. ej. al ver una foto a pantalla completa). */
void setChromePaused(bool paused);
bool chromePaused();

}  // namespace ui
