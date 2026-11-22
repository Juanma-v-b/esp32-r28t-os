#pragma once

#include <Arduino.h>
#include <lvgl.h>

#include <functional>

namespace ui {

/**
 * Teclado modal reutilizable (WiFi, ciudad, coordenadas...).
 * Se monta sobre la capa superior y se destruye al aceptar/cancelar.
 */
class KeyboardPrompt {
 public:
  ~KeyboardPrompt();

  void show(const char* title, const String& initial, bool numeric,
            bool password, std::function<void(const String&)> onAccept);
  void hide();
  bool visible() const { return overlay_ != nullptr; }

 private:
  static void readyCb(lv_event_t* e);
  static void cancelCb(lv_event_t* e);

  lv_obj_t* overlay_ = nullptr;
  lv_obj_t* ta_ = nullptr;
  std::function<void(const String&)> onAccept_;
};

}  // namespace ui
