#include "ui/keyboard_prompt.h"

#include "ui/theme.h"

namespace ui {

KeyboardPrompt::~KeyboardPrompt() { hide(); }

void KeyboardPrompt::show(const char* title, const String& initial, bool numeric,
                          bool password,
                          std::function<void(const String&)> onAccept) {
  hide();
  onAccept_ = std::move(onAccept);

  lv_obj_t* layer = lv_layer_top();
  overlay_ = lv_obj_create(layer);
  lv_obj_remove_style_all(overlay_);
  lv_obj_set_size(overlay_, 240, 320);
  lv_obj_set_pos(overlay_, 0, 0);
  lv_obj_set_style_bg_color(overlay_, color(0x000000), 0);
  lv_obj_set_style_bg_opa(overlay_, LV_OPA_70, 0);
  lv_obj_add_flag(overlay_, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(overlay_, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t* panel = lv_obj_create(overlay_);
  lv_obj_remove_style_all(panel);
  lv_obj_set_size(panel, 228, 124);
  lv_obj_set_pos(panel, 6, 8);
  lv_obj_set_style_bg_color(panel, color(col::CARD), 0);
  lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(panel, 1, 0);
  lv_obj_set_style_border_color(panel, color(col::BORDER), 0);
  lv_obj_set_style_radius(panel, 14, 0);
  lv_obj_set_style_pad_all(panel, 10, 0);

  lv_obj_t* titleLbl = makeLabel(panel, title, &ui_font_12,
                                 col::MUTED);
  lv_obj_set_pos(titleLbl, 0, 0);

  ta_ = lv_textarea_create(panel);
  lv_obj_set_size(ta_, 206, 52);
  lv_obj_set_pos(ta_, 0, 22);
  lv_textarea_set_one_line(ta_, true);
  lv_textarea_set_text(ta_, initial.c_str());
  lv_textarea_set_max_length(ta_, 64);
  lv_obj_set_style_text_font(ta_, &ui_font_16, 0);
  lv_obj_set_style_bg_color(ta_, color(0x0B1526), 0);
  lv_obj_set_style_text_color(ta_, color(col::TEXT), 0);
  lv_obj_set_style_border_color(ta_, color(col::ACCENT), 0);
  if (password) lv_textarea_set_password_mode(ta_, true);
  if (numeric) {
    lv_textarea_set_accepted_chars(ta_, "0123456789.,-");
  }

  lv_obj_t* kb = lv_keyboard_create(overlay_);
  lv_obj_set_size(kb, 240, 180);
  lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_keyboard_set_textarea(kb, ta_);
  if (numeric) lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUMBER);

  lv_obj_set_style_bg_color(kb, color(0x0C1628), 0);
  lv_obj_set_style_bg_opa(kb, LV_OPA_COVER, 0);

  lv_obj_add_event_cb(kb, readyCb, LV_EVENT_READY, this);
  lv_obj_add_event_cb(kb, cancelCb, LV_EVENT_CANCEL, this);
}

void KeyboardPrompt::hide() {
  if (!overlay_) return;
  lv_obj_t* old = overlay_;
  overlay_ = nullptr;
  ta_ = nullptr;
  lv_obj_del_async(old);
}

void KeyboardPrompt::readyCb(lv_event_t* e) {
  KeyboardPrompt* self = static_cast<KeyboardPrompt*>(lv_event_get_user_data(e));
  if (!self || !self->ta_) return;
  String value = lv_textarea_get_text(self->ta_);
  auto cb = self->onAccept_;
  self->onAccept_ = nullptr;
  self->hide();
  if (cb) cb(value);
}

void KeyboardPrompt::cancelCb(lv_event_t* e) {
  KeyboardPrompt* self = static_cast<KeyboardPrompt*>(lv_event_get_user_data(e));
  if (self) self->hide();
}

}  // namespace ui
