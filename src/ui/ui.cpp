#include "ui/ui.h"

#include <Arduino.h>
#include <lvgl.h>

#include "config.h"
#include "hal/display.h"
#include "hal/touch.h"
#include "ui/router.h"
#include "ui/theme.h"

namespace ui {
namespace {

// Doble búfer parcial: 2 x 240x16 px = ~15 KB.
// El búfer es pequeño a propósito: en el ESP32 el DRAM es escaso y
// WiFi + LVGL necesitan el resto. Con SPI a 40 MHz 16 líneas se
// envían en ~1,5 ms, así que no se nota.
lv_color_t drawBuf1[UI_SCREEN_W * 16];
lv_color_t drawBuf2[UI_SCREEN_W * 16];
lv_disp_draw_buf_t drawBuf;
lv_disp_drv_t dispDrv;

void flushCb(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* colorP) {
  const uint32_t w = area->x2 - area->x1 + 1;
  const uint32_t h = area->y2 - area->y1 + 1;
  display.tft().pushImage(area->x1, area->y1, w, h,
                          reinterpret_cast<uint16_t*>(colorP));
  lv_disp_flush_ready(drv);
}

void touchCb(lv_indev_drv_t* drv, lv_indev_data_t* data) {
  (void)drv;
  uint16_t rawX = 0;
  uint16_t rawY = 0;
  int16_t x = 0;
  int16_t y = 0;
  if (touch.read(rawX, rawY) && touch.mapToScreen(rawX, rawY, x, y)) {
    data->point.x = x;
    data->point.y = y;
    data->state = LV_INDEV_STATE_PRESSED;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

}  // namespace

void begin() {
  lv_init();

  lv_disp_draw_buf_init(&drawBuf, drawBuf1, drawBuf2, UI_SCREEN_W * 16);
  lv_disp_drv_init(&dispDrv);
  dispDrv.hor_res = UI_SCREEN_W;
  dispDrv.ver_res = UI_SCREEN_H;
  dispDrv.flush_cb = flushCb;
  dispDrv.draw_buf = &drawBuf;
  lv_disp_t* disp = lv_disp_drv_register(&dispDrv);

  static lv_indev_drv_t indevDrv;
  lv_indev_drv_init(&indevDrv);
  indevDrv.type = LV_INDEV_TYPE_POINTER;
  indevDrv.read_cb = touchCb;
  lv_indev_drv_register(&indevDrv);

  lv_theme_t* theme =
      lv_theme_default_init(disp, color(col::ACCENT), color(col::ACCENT_2),
                            true, &ui_font_14);
  lv_disp_set_theme(disp, theme);

  lv_obj_set_style_bg_color(lv_scr_act(), color(col::BG_TOP), 0);

  routerBegin();
}

void loop() {
  routerLoop();
  lv_timer_handler();
}

}  // namespace ui
