#include "ui/icons.h"

#include "ui/theme.h"

namespace ui {
namespace {

lv_obj_t* root(lv_obj_t* parent, int size) {
  lv_obj_t* o = lv_obj_create(parent);
  lv_obj_remove_style_all(o);
  lv_obj_set_size(o, size, size);
  lv_obj_set_style_bg_opa(o, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(o, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  return o;
}

lv_obj_t* part(lv_obj_t* parent, int x, int y, int w, int h, uint32_t fill,
               int radius) {
  lv_obj_t* o = lv_obj_create(parent);
  lv_obj_remove_style_all(o);
  lv_obj_set_pos(o, x, y);
  lv_obj_set_size(o, w, h);
  lv_obj_set_style_bg_color(o, color(fill), 0);
  lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(o, radius, 0);
  lv_obj_clear_flag(o, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  return o;
}

lv_obj_t* ring(lv_obj_t* parent, int x, int y, int d, uint32_t stroke, int width) {
  lv_obj_t* o = lv_obj_create(parent);
  lv_obj_remove_style_all(o);
  lv_obj_set_pos(o, x, y);
  lv_obj_set_size(o, d, d);
  lv_obj_set_style_bg_opa(o, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(o, width, 0);
  lv_obj_set_style_border_color(o, color(stroke), 0);
  lv_obj_set_style_radius(o, LV_RADIUS_CIRCLE, 0);
  lv_obj_clear_flag(o, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  return o;
}

void cloud(lv_obj_t* parent, int s, uint32_t fill, int dy = 0) {
  part(parent, s * 15 / 100, s * 52 / 100 + dy, s * 70 / 100, s * 22 / 100,
       fill, s * 10 / 100);
  part(parent, s * 18 / 100, s * 36 / 100 + dy, s * 30 / 100, s * 30 / 100,
       fill, LV_RADIUS_CIRCLE);
  part(parent, s * 36 / 100, s * 28 / 100 + dy, s * 34 / 100, s * 34 / 100,
       fill, LV_RADIUS_CIRCLE);
  part(parent, s * 55 / 100, s * 40 / 100 + dy, s * 28 / 100, s * 28 / 100,
       fill, LV_RADIUS_CIRCLE);
}

void sun(lv_obj_t* parent, int s, int x, int y, int d, uint32_t fill) {
  ring(parent, x - d / 10, y - d / 10, d + d / 5, fill, 2);
  part(parent, x, y, d, d, fill, LV_RADIUS_CIRCLE);
}

}  // namespace

lv_obj_t* iconClock(lv_obj_t* parent, int size) {
  lv_obj_t* o = root(parent, size);
  const int s = size;
  ring(o, s * 10 / 100, s * 10 / 100, s * 80 / 100, col::ACCENT, s * 7 / 100);
  part(o, s * 48 / 100, s * 28 / 100, s * 5 / 100, s * 26 / 100, col::TEXT,
       s * 3 / 100);
  part(o, s * 47 / 100, s * 46 / 100, s * 23 / 100, s * 5 / 100, col::TEXT,
       s * 3 / 100);
  part(o, s * 45 / 100, s * 45 / 100, s * 10 / 100, s * 10 / 100,
       col::ACCENT, LV_RADIUS_CIRCLE);
  return o;
}

lv_obj_t* iconWeather(lv_obj_t* parent, int size, int type) {
  lv_obj_t* o = root(parent, size);
  const int s = size;

  switch (type) {
    case 0:  // sol
      sun(o, s, s * 28 / 100, s * 28 / 100, s * 44 / 100, col::SUN);
      break;

    case 1:  // sol + nube
      sun(o, s, s * 12 / 100, s * 12 / 100, s * 34 / 100, col::SUN);
      cloud(o, s, col::CLOUD, s * 10 / 100);
      break;

    case 2:  // nublado
      cloud(o, s, col::CLOUD, 0);
      break;

    case 3:  // niebla
      cloud(o, s, col::CLOUD, -s * 10 / 100);
      part(o, s * 16 / 100, s * 74 / 100, s * 68 / 100, s * 7 / 100,
           col::CLOUD, s * 4 / 100);
      part(o, s * 26 / 100, s * 86 / 100, s * 48 / 100, s * 7 / 100,
           col::CLOUD, s * 4 / 100);
      break;

    case 4:  // lluvia
      cloud(o, s, col::CLOUD, -s * 8 / 100);
      for (int i = 0; i < 3; i++) {
        part(o, s * (24 + i * 20) / 100, s * 76 / 100, s * 6 / 100,
             s * 16 / 100, col::RAIN, s * 3 / 100);
      }
      break;

    case 5:  // nieve
      cloud(o, s, col::CLOUD, -s * 8 / 100);
      for (int i = 0; i < 3; i++) {
        part(o, s * (22 + i * 22) / 100, s * 78 / 100, s * 12 / 100,
             s * 12 / 100, col::TEXT, LV_RADIUS_CIRCLE);
      }
      break;

    default:  // tormenta
      cloud(o, s, col::CLOUD, -s * 10 / 100);
      part(o, s * 46 / 100, s * 68 / 100, s * 18 / 100, s * 9 / 100,
           col::WARN, s * 2 / 100);
      part(o, s * 38 / 100, s * 77 / 100, s * 18 / 100, s * 9 / 100,
           col::WARN, s * 2 / 100);
      part(o, s * 30 / 100, s * 86 / 100, s * 14 / 100, s * 8 / 100,
           col::WARN, s * 2 / 100);
      break;
  }
  return o;
}

lv_obj_t* iconGallery(lv_obj_t* parent, int size) {
  lv_obj_t* o = root(parent, size);
  const int s = size;

  ring(o, s * 14 / 100, s * 18 / 100, s * 72 / 100, col::ACCENT, s * 5 / 100);
  part(o, s * 56 / 100, s * 30 / 100, s * 12 / 100, s * 12 / 100, col::SUN,
       LV_RADIUS_CIRCLE);

  for (int i = 0; i < 4; i++) {
    part(o, s * (26 + i * 5) / 100, s * (68 - i * 8) / 100,
         s * (46 - i * 9) / 100, s * 6 / 100, col::TEXT, 2);
  }
  return o;
}

lv_obj_t* iconSettings(lv_obj_t* parent, int size) {
  lv_obj_t* o = root(parent, size);
  const int s = size;

  for (int i = 0; i < 3; i++) {
    int y = s * (28 + i * 22) / 100;
    part(o, s * 16 / 100, y, s * 68 / 100, s * 7 / 100, col::MUTED,
         s * 4 / 100);
    part(o, s * (24 + i * 18) / 100, y - s * 6 / 100, s * 19 / 100,
         s * 19 / 100, col::ACCENT, LV_RADIUS_CIRCLE);
  }
  return o;
}

}  // namespace ui
