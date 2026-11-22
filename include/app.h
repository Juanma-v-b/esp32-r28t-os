#pragma once

#include <lvgl.h>

namespace apps {

/**
 * Contrato común de todas las apps.
 *
 * El router crea una pantalla LVGL nueva, llama a build() y, al salir,
 * llama a onDestroy() antes de destruir la instancia. Cualquier lv_timer
 * creado por la app debe borrarse en onDestroy().
 */
class App {
 public:
  virtual ~App() = default;

  virtual const char* title() const = 0;
  virtual void build(lv_obj_t* screen) = 0;
  virtual void onDestroy() {}
};

}  // namespace apps
