#pragma once

#include "ui/fonts.h"

#include <lvgl.h>

namespace ui {

namespace col {
constexpr uint32_t BG_TOP      = 0x070B16;
constexpr uint32_t BG_BOTTOM   = 0x111C33;
constexpr uint32_t CARD        = 0x111C31;
constexpr uint32_t CARD_HOVER  = 0x1A2A47;
constexpr uint32_t BORDER      = 0x243553;
constexpr uint32_t TEXT        = 0xF2F6FF;
constexpr uint32_t MUTED       = 0x91A4C2;
constexpr uint32_t ACCENT      = 0x5EEAD4;
constexpr uint32_t ACCENT_2    = 0x8B7CF6;
constexpr uint32_t WARN        = 0xFBBF24;
constexpr uint32_t DANGER      = 0xFB7185;
constexpr uint32_t OK          = 0x34D399;
constexpr uint32_t SUN         = 0xFBBF24;
constexpr uint32_t CLOUD       = 0xCBD5E1;
constexpr uint32_t RAIN        = 0x60A5FA;
}  // namespace col

inline lv_color_t color(uint32_t hex) { return lv_color_hex(hex); }

void styleScreen(lv_obj_t* screen, uint32_t top = col::BG_TOP,
                 uint32_t bottom = col::BG_BOTTOM);

lv_obj_t* makeLabel(lv_obj_t* parent, const char* text, const lv_font_t* font,
                    uint32_t textColor);
lv_obj_t* makeCard(lv_obj_t* parent);
lv_obj_t* makePill(lv_obj_t* parent, const char* text, uint32_t bg, uint32_t fg);

}  // namespace ui
