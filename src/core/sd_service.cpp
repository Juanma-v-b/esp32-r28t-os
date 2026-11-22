#include "core/sd_service.h"

#include <algorithm>

#include "config.h"

SdService sdCard;

bool SdService::begin() {
  if (ready_) return true;

  lastError_ = "";
  pinMode(PIN_SD_CS, OUTPUT);
  digitalWrite(PIN_SD_CS, HIGH);

  spi_.begin(PIN_SD_SCLK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);

  if (!SD.begin(PIN_SD_CS, spi_, 4000000, "/sd", 5)) {
    lastError_ = "No se pudo montar la tarjeta";
    ready_ = false;
    Serial.println("[sd] montaje fallido");
    return false;
  }

  ready_ = true;
  Serial.printf("[sd] tarjeta montada, %llu MB\n",
                (unsigned long long)(SD.cardSize() / (1024ULL * 1024ULL)));
  return true;
}

bool SdService::remount() {
  ready_ = false;
  SD.end();
  delay(80);
  return begin();
}

std::vector<SdImage> SdService::listImages(size_t maxFiles) {
  std::vector<SdImage> out;
  if (!ready_ && !begin()) return out;

  File root = SD.open("/");
  if (!root) {
    lastError_ = "No se pudo abrir la raíz";
    return out;
  }

  File file = root.openNextFile();
  while (file && out.size() < maxFiles) {
    if (!file.isDirectory()) {
      String name = file.name();
      const int slash = name.lastIndexOf('/');
      if (slash >= 0) name = name.substring(slash + 1);

      String lower = name;
      lower.toLowerCase();

      const bool jpeg = lower.endsWith(".jpg") || lower.endsWith(".jpeg");
      const bool bmp = lower.endsWith(".bmp");

      if ((jpeg || bmp) && !name.startsWith(".")) {
        SdImage image;
        image.name = name;
        image.path = String("/") + name;
        image.size = file.size();
        image.jpeg = jpeg;
        out.push_back(image);
      }
    }
    file = root.openNextFile();
  }
  root.close();

  std::sort(out.begin(), out.end(), [](const SdImage& a, const SdImage& b) {
    String la = a.name;
    String lb = b.name;
    la.toLowerCase();
    lb.toLowerCase();
    return la < lb;
  });

  return out;
}
