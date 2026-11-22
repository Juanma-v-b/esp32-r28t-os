#include "ui/theme.h"

namespace ui {

void styleScreen(lv_obj_t* screen, uint32_t top, uint32_t bottom) {
  lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(screen, color(top), 0);
  lv_obj_set_style_bg_grad_color(screen, color(bottom), 0);
  lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, 0);
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(screen, 0, 0);
  lv_obj_set_style_pad_all(screen, 0, 0);
}

lv_obj_t* makeLabel(lv_obj_t* parent, const char* text, const lv_font_t* font,
                    uint32_t textColor) {
  lv_obj_t* label = lv_label_create(parent);
  lv_label_set_text(label, text);
  lv_obj_set_style_text_font(label, font, 0);
  lv_obj_set_style_text_color(label, color(textColor), 0);
  lv_obj_clear_flag(label, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  return label;
}

lv_obj_t* makeCard(lv_obj_t* parent) {
  lv_obj_t* card = lv_obj_create(parent);
  lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(card, color(col::CARD), 0);
  lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(card, 1, 0);
  lv_obj_set_style_border_color(card, color(col::BORDER), 0);
  lv_obj_set_style_radius(card, 16, 0);
  lv_obj_set_style_pad_all(card, 10, 0);
  return card;
}

lv_obj_t* makePill(lv_obj_t* parent, const char* text, uint32_t bg, uint32_t fg) {
  lv_obj_t* pill = lv_obj_create(parent);
  lv_obj_clear_flag(pill, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(pill, color(bg), 0);
  lv_obj_set_style_bg_opa(pill, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(pill, 0, 0);
  lv_obj_set_style_radius(pill, 10, 0);
  lv_obj_set_style_pad_hor(pill, 8, 0);
  lv_obj_set_style_pad_ver(pill, 4, 0);
  lv_obj_t* label = makeLabel(pill, text, &ui_font_10, fg);
  lv_obj_center(label);
  return pill;
}

}  // namespace ui
