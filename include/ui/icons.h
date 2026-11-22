#pragma once

#include <lvgl.h>

namespace ui {

lv_obj_t* iconClock(lv_obj_t* parent, int size);
lv_obj_t* iconWeather(lv_obj_t* parent, int size, int type);
lv_obj_t* iconGallery(lv_obj_t* parent, int size);
lv_obj_t* iconSettings(lv_obj_t* parent, int size);

}  // namespace ui
