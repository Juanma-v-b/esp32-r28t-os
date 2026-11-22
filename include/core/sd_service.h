#pragma once

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

#include <vector>

struct SdImage {
  String name;
  String path;
  uint32_t size = 0;
  bool jpeg = false;
};

class SdService {
 public:
  bool begin();
  bool remount();
  bool ready() const { return ready_; }

  /** Lista .jpg/.jpeg/.bmp de la raíz, ordenados alfabéticamente. */
  std::vector<SdImage> listImages(size_t maxFiles = 100);

  String lastError() const { return lastError_; }

 private:
  SPIClass spi_{VSPI};
  bool ready_ = false;
  String lastError_;
};

extern SdService sdCard;
