#pragma once

#include <Arduino.h>
#include <lvgl.h>

#include <vector>

#include "app.h"
#include "core/sd_service.h"

namespace apps {

class GalleryApp : public App {
 public:
  const char* title() const override { return "Galería SD"; }
  void build(lv_obj_t* screen) override;
  void onDestroy() override;

 private:
  static void fileClicked(lv_event_t* e);
  static void retryClicked(lv_event_t* e);
  static void viewerClicked(lv_event_t* e);

  void renderList();
  void openViewer(size_t index);
  void closeViewer();
  void drawCurrent();
  bool drawImage(const SdImage& image);

  lv_obj_t* screen_ = nullptr;
  lv_obj_t* listWrap_ = nullptr;
  lv_obj_t* overlay_ = nullptr;

  std::vector<SdImage> files_;
  size_t index_ = 0;
};

}  // namespace apps
