/**
 * Configuración mínima de LVGL 8.4 para R28T OS.
 * lv_conf_internal.h rellena con valores por defecto todo lo que
 * no aparezca aquí, así que solo tocamos lo importante.
 */
#pragma once

#if 1 /* activar este archivo */

#define LV_CONF_H

#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0

/* Memoria: 48 KB para objetos/estilos. Las pantallas se destruyen
 * al salir de cada app, así que es de sobra; el resto del DRAM queda
 * para el heap de WiFi/TCP. */
#define LV_MEM_CUSTOM 0
#define LV_MEM_SIZE (48U * 1024U)

#define LV_DISP_DEF_REFR_PERIOD 16
#define LV_INDEV_DEF_READ_PERIOD 20

/* Usar millis() de Arduino como tick */
#define LV_TICK_CUSTOM 1
#define LV_TICK_CUSTOM_INCLUDE "Arduino.h"
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())

#define LV_USE_LOG 0
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MALLOC 1

#define LV_USE_THEME_DEFAULT 1
#define LV_USE_THEME_BASIC 1
#define LV_THEME_DEFAULT_DARK 1

#define LV_USE_FLEX 1
#define LV_USE_GRID 1

/* Fuentes usadas por la interfaz */
#define LV_FONT_MONTSERRAT_10 1
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_MONTSERRAT_32 1
#define LV_FONT_MONTSERRAT_36 1
#define LV_FONT_MONTSERRAT_40 1
#define LV_FONT_MONTSERRAT_48 1
#define LV_FONT_DEFAULT &lv_font_montserrat_14

#define LV_USE_PERF_MONITOR 0
#define LV_USE_MEM_MONITOR 0

#endif
