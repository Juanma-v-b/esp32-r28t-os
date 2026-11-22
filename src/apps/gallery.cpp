#include "apps/gallery.h"

#include <Arduino.h>
#include <FS.h>
#include <TJpg_Decoder.h>

#include "config.h"
#include "hal/display.h"
#include "ui/router.h"
#include "ui/status_bar.h"
#include "ui/theme.h"

namespace apps {
namespace {

bool jpegCallback(int16_t x, int16_t y, uint16_t w, uint16_t h,
                  uint16_t* bitmap) {
  display.tft().pushImage(x, y, w, h, bitmap);
  return true;
}

/**
 * BMP sin compresión (24/32 bits). TFT_eSPI 2.5.4x ya no trae drawBmp,
 * así que lo dibujamos por filas con conversión a RGB565.
 */
bool drawBmpFile(const String& path) {
  File file = SD.open(path);
  if (!file) return false;

  uint8_t header[54];
  if (file.read(header, sizeof(header)) != sizeof(header)) {
    file.close();
    return false;
  }

  const uint16_t type = (uint16_t)(header[0] | (header[1] << 8));
  if (type != 0x4D42) {  // "BM"
    file.close();
    return false;
  }

  uint32_t dataOffset = 0;
  int32_t width = 0;
  int32_t height = 0;
  uint32_t compression = 0;
  uint16_t bpp = 0;
  memcpy(&dataOffset, header + 10, 4);
  memcpy(&width, header + 18, 4);
  memcpy(&height, header + 22, 4);
  memcpy(&bpp, header + 28, 2);
  memcpy(&compression, header + 30, 4);

  if (width <= 0 || height == 0 || compression != 0 ||
      (bpp != 24 && bpp != 32)) {
    file.close();
    return false;
  }

  const int32_t absHeight = height < 0 ? -height : height;
  const int32_t bytesPerPixel = bpp / 8;
  const int32_t rowSize = ((width * bytesPerPixel + 3) / 4) * 4;

  const int32_t drawW = width < UI_SCREEN_W ? width : UI_SCREEN_W;
  const int32_t drawH = absHeight < UI_SCREEN_H ? absHeight : UI_SCREEN_H;
  const int32_t srcX = (width - drawW) / 2;
  const int32_t dstX = (UI_SCREEN_W - drawW) / 2;
  const int32_t dstY = (UI_SCREEN_H - drawH) / 2;
  const bool topDown = height < 0;

  uint8_t row[UI_SCREEN_W * 4];
  uint16_t line[UI_SCREEN_W];
  auto& tft = display.tft();

  for (int32_t y = 0; y < drawH; y++) {
    const int32_t srcRow = topDown ? y : (absHeight - 1 - y);
    const uint32_t offset = dataOffset + (uint32_t)srcRow * rowSize +
                            (uint32_t)srcX * bytesPerPixel;
    if (!file.seek(offset)) {
      file.close();
      return false;
    }

    const size_t bytesToRead = (size_t)drawW * bytesPerPixel;
    if (file.read(row, bytesToRead) != bytesToRead) {
      file.close();
      return false;
    }

    for (int32_t x = 0; x < drawW; x++) {
      uint8_t r = 0, g = 0, b = 0;
      if (bytesPerPixel == 3) {
        b = row[x * 3];
        g = row[x * 3 + 1];
        r = row[x * 3 + 2];
      } else {
        b = row[x * 4];
        g = row[x * 4 + 1];
        r = row[x * 4 + 2];
      }
      line[x] = tft.color565(r, g, b);
    }
    tft.pushImage(dstX, dstY + y, drawW, 1, line);
  }

  file.close();
  return true;
}

String humanSize(uint32_t bytes) {
  if (bytes >= 1024UL * 1024UL) return String(bytes / (1024.0f * 1024.0f), 1) + " MB";
  return String(bytes / 1024.0f, 0) + " KB";
}

}  // namespace

void GalleryApp::build(lv_obj_t* screen) {
  screen_ = screen;
  ui::styleScreen(screen_);
  ui::createStatusBar(screen_, title(), true);

  listWrap_ = lv_obj_create(screen_);
  lv_obj_remove_style_all(listWrap_);
  lv_obj_set_pos(listWrap_, 0, UI_STATUSBAR_H);
  lv_obj_set_size(listWrap_, UI_SCREEN_W, UI_SCREEN_H - UI_STATUSBAR_H);
  lv_obj_set_style_bg_opa(listWrap_, LV_OPA_TRANSP, 0);
  lv_obj_set_style_pad_all(listWrap_, 8, 0);
  lv_obj_set_style_pad_row(listWrap_, 6, 0);
  lv_obj_set_flex_flow(listWrap_, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_scrollbar_mode(listWrap_, LV_SCROLLBAR_MODE_AUTO);

  renderList();
}

void GalleryApp::renderList() {
  files_.clear();
  lv_obj_clean(listWrap_);

  if (!sdCard.ready() && !sdCard.begin()) {
    lv_obj_t* icon = ui::makeLabel(listWrap_, LV_SYMBOL_SD_CARD,
                                   &lv_font_montserrat_32, ui::col::MUTED);
    lv_obj_set_style_pad_top(icon, 46, 0);
    lv_obj_set_style_pad_bottom(icon, 10, 0);

    ui::makeLabel(listWrap_, "No se detecta la tarjeta microSD",
                  &ui_font_12, ui::col::TEXT);
    ui::makeLabel(listWrap_, "Inserta una tarjeta FAT32 y vuelve a intentarlo",
                  &ui_font_10, ui::col::MUTED);

    lv_obj_t* retry = lv_obj_create(listWrap_);
    lv_obj_remove_style_all(retry);
    lv_obj_set_size(retry, 130, 32);
    lv_obj_set_style_bg_color(retry, ui::color(ui::col::ACCENT), 0);
    lv_obj_set_style_bg_opa(retry, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(retry, 10, 0);
    lv_obj_clear_flag(retry, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(retry, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(retry, retryClicked, LV_EVENT_CLICKED, this);
    lv_obj_t* label = ui::makeLabel(retry, "Reintentar",
                                    &ui_font_12, 0x06231F);
    lv_obj_center(label);
    return;
  }

  files_ = sdCard.listImages(80);

  if (files_.empty()) {
    lv_obj_t* icon = ui::makeLabel(listWrap_, LV_SYMBOL_IMAGE,
                                   &lv_font_montserrat_32, ui::col::MUTED);
    lv_obj_set_style_pad_top(icon, 46, 0);
    lv_obj_set_style_pad_bottom(icon, 10, 0);

    ui::makeLabel(listWrap_, "La tarjeta está vacía",
                  &ui_font_12, ui::col::TEXT);
    ui::makeLabel(listWrap_,
                  "Copia fotos .jpg o .bmp de 240x320 en la raíz",
                  &ui_font_10, ui::col::MUTED);
    return;
  }

  lv_obj_t* count = ui::makeLabel(listWrap_, "",
                                  &ui_font_10, ui::col::MUTED);
  lv_label_set_text_fmt(count, "%d imágenes · toca una para abrir",
                        (int)files_.size());

  for (size_t i = 0; i < files_.size(); i++) {
    const SdImage& image = files_[i];

    lv_obj_t* row = lv_obj_create(listWrap_);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, 224, 46);
    lv_obj_set_style_bg_color(row, ui::color(ui::col::CARD), 0);
    lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(row, 10, 0);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_user_data(row, reinterpret_cast<void*>(i));
    lv_obj_add_event_cb(row, fileClicked, LV_EVENT_CLICKED, this);

    lv_obj_t* icon = ui::makeLabel(row, LV_SYMBOL_IMAGE,
                                   &ui_font_16, ui::col::ACCENT);
    lv_obj_set_pos(icon, 10, 14);

    lv_obj_t* name = ui::makeLabel(row, image.name.c_str(),
                                   &ui_font_12, ui::col::TEXT);
    lv_obj_set_width(name, 150);
    lv_obj_set_pos(name, 34, 9);
    lv_label_set_long_mode(name, LV_LABEL_LONG_DOT);

    lv_obj_t* meta = ui::makeLabel(row, humanSize(image.size).c_str(),
                                   &ui_font_10, ui::col::MUTED);
    lv_obj_set_pos(meta, 34, 26);
  }
}

void GalleryApp::fileClicked(lv_event_t* e) {
  auto* self = static_cast<GalleryApp*>(lv_event_get_user_data(e));
  if (!self) return;
  const size_t index =
      reinterpret_cast<uintptr_t>(lv_obj_get_user_data(lv_event_get_target(e)));
  self->openViewer(index);
}

void GalleryApp::retryClicked(lv_event_t* e) {
  auto* self = static_cast<GalleryApp*>(lv_event_get_user_data(e));
  if (!self) return;
  sdCard.remount();
  self->renderList();
}

void GalleryApp::openViewer(size_t index) {
  if (index >= files_.size()) return;

  index_ = index;

  overlay_ = lv_obj_create(screen_);
  lv_obj_remove_style_all(overlay_);
  lv_obj_set_pos(overlay_, 0, 0);
  lv_obj_set_size(overlay_, UI_SCREEN_W, UI_SCREEN_H);
  lv_obj_set_style_bg_color(overlay_, ui::color(0x000000), 0);
  lv_obj_set_style_bg_opa(overlay_, LV_OPA_COVER, 0);
  lv_obj_clear_flag(overlay_, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(overlay_, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(overlay_, viewerClicked, LV_EVENT_CLICKED, this);

  ui::setChromePaused(true);
  lv_refr_now(NULL);
  drawCurrent();
}

void GalleryApp::closeViewer() {
  if (overlay_) {
    lv_obj_t* old = overlay_;
    overlay_ = nullptr;
    lv_obj_del_async(old);
  }
  ui::setChromePaused(false);
  lv_obj_invalidate(screen_);
}

void GalleryApp::drawCurrent() {
  auto& tft = display.tft();
  tft.fillScreen(TFT_BLACK);

  const bool ok = drawImage(files_[index_]);
  if (!ok) {
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawString("No se pudo abrir la imagen", 120, 158, 2);
  }

  tft.fillRect(0, 300, 240, 20, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("< anterior    volver    siguiente >", 120, 310, 1);
  tft.setTextDatum(TL_DATUM);
}

bool GalleryApp::drawImage(const SdImage& image) {
  auto& tft = display.tft();

  if (image.jpeg) {
    uint16_t w = 0;
    uint16_t h = 0;
    if (TJpgDec.getFsJpgSize(&w, &h, image.path, SD) != JDR_OK) return false;

    uint8_t scale = 1;
    while ((w / scale > UI_SCREEN_W || h / scale > UI_SCREEN_H) && scale < 8) {
      scale *= 2;
    }

    TJpgDec.setJpgScale(scale);
    TJpgDec.setSwapBytes(false);
    TJpgDec.setCallback(jpegCallback);

    const int16_t x = (UI_SCREEN_W - w / scale) / 2;
    const int16_t y = (UI_SCREEN_H - h / scale) / 2;
    return TJpgDec.drawFsJpg(x, y, image.path, SD) == JDR_OK;
  }

  return drawBmpFile(image.path);
}

void GalleryApp::viewerClicked(lv_event_t* e) {
  auto* self = static_cast<GalleryApp*>(lv_event_get_user_data(e));
  if (!self || !self->overlay_) return;

  lv_point_t point = {120, 160};
  lv_indev_t* indev = lv_indev_get_act();
  if (indev) lv_indev_get_point(indev, &point);

  if (point.x < 70) {
    self->index_ =
        (self->index_ + self->files_.size() - 1) % self->files_.size();
    self->drawCurrent();
  } else if (point.x > 170) {
    self->index_ = (self->index_ + 1) % self->files_.size();
    self->drawCurrent();
  } else {
    self->closeViewer();
  }
}

void GalleryApp::onDestroy() {
  // La pantalla y el overlay se destruyen con el árbol LVGL.
  overlay_ = nullptr;
  ui::setChromePaused(false);
}

}  // namespace apps
