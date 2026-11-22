#pragma once

// ============================================================
//  R28T OS - configuración de hardware y valores por defecto
//  Placa: E32R28T (ESP32-WROOM-32E, 2.8" 240x320)
// ============================================================

#define FW_NAME    "R28T OS"
#define FW_VERSION "0.1.0"

// ---------------- Pantalla ----------------
// El bus de la pantalla se configura entero desde platformio.ini
// (TFT_eSPI). Aquí solo el pin de retroiluminación.
#define PIN_TFT_BL          21
#define BL_LEDC_CHANNEL     0

// ---------------- Táctil XPT2046 ----------------
// Bus SPI por software: así no choca con la pantalla (HSPI)
// ni con la tarjeta SD (VSPI).
#define PIN_TOUCH_CLK       25
#define PIN_TOUCH_MOSI      32
#define PIN_TOUCH_MISO      39
#define PIN_TOUCH_CS        33
#define PIN_TOUCH_IRQ       36

// Si el táctil responde con los ejes cambiados, ponlo a 1.
#define TOUCH_SWAP_XY       0
#define TOUCH_INVERT_X      0
#define TOUCH_INVERT_Y      0

// Calibración de fábrica aproximada (se puede rehacer desde Ajustes).
#define TOUCH_DEFAULT_X_MIN 240
#define TOUCH_DEFAULT_X_MAX 3760
#define TOUCH_DEFAULT_Y_MIN 280
#define TOUCH_DEFAULT_Y_MAX 3740

// ---------------- microSD ----------------
#define PIN_SD_CS           5
#define PIN_SD_SCLK         18
#define PIN_SD_MISO         19
#define PIN_SD_MOSI         23

// ---------------- LED RGB (activo en LOW) ----------------
#define PIN_LED_R           22
#define PIN_LED_G           16
#define PIN_LED_B           17

// ---------------- Otros ----------------
#define PIN_BATTERY         34
#define PIN_BOOT_BTN        0
#define PIN_AUDIO_EN        4

// ---------------- Ajustes por defecto ----------------
#define DEFAULT_CITY        "Madrid"
#define DEFAULT_LAT         40.4168f
#define DEFAULT_LON         (-3.7038f)

// Zona horaria 0 = España peninsular, 1 = Canarias
#define TZ_MAINLAND_POSIX   "CET-1CEST,M3.5.0,M10.5.0/3"
#define TZ_MAINLAND_LABEL   "Europe/Madrid"
#define TZ_CANARY_POSIX     "WET0WEST,M3.5.0/1,M10.5.0"
#define TZ_CANARY_LABEL     "Atlantic/Canary"

// ---------------- UI ----------------
#define UI_SCREEN_W         240
#define UI_SCREEN_H         320
#define UI_STATUSBAR_H      26
#define CLOCK_THEME_COUNT   3
