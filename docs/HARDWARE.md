# Hardware — E32R28T

## Identificación

- **Nombre comercial**: E32R28T (DIYmalls y otros vendedores de AliExpress/Amazon).
- **Módulo**: ESP32-WROOM-32E, 4 MB de flash (habitual), 240 MHz, sin PSRAM.
- **Pantalla**: 2,8" TFT 240×320, SPI de 4 hilos. Según lote: **ST7789** o **ILI9341**.
- **Táctil**: resistivo **XPT2046** por SPI propio.
- **Extras**: microSD, LED RGB, conector de batería con divisor en GPIO 34, botón BOOT, salida de audio por DAC.

No es una “Cheap Yellow Display” (ESP32-2432S028) al uso, aunque comparte familia y muchos pines. Los pines de la SD y del táctil cambian respecto a algunas variantes.

## Pinout completo

### Pantalla (ST7789)

| Señal | GPIO | Notas |
| --- | --- | --- |
| CS | 15 | |
| DC | 2 | |
| SCLK | 14 | |
| MOSI | 13 | |
| MISO | 12 | |
| BL | 21 | Activo en alto; PWM por LEDC |
| RST | EN | Compartido con el reset del ESP32 |

### Táctil (XPT2046)

| Señal | GPIO | Notas |
| --- | --- | --- |
| CLK | 25 | |
| MOSI | 32 | |
| MISO | 39 | Entrada only |
| CS | 33 | |
| IRQ | 36 | Entrada only, activo en bajo |

El driver usa **SPI por software** para el táctil. Así la pantalla puede usar HSPI y la SD VSPI sin conflictos, y se puede leer el táctil incluso mientras se dibuja una foto desde la SD.

### microSD

| Señal | GPIO |
| --- | --- |
| CS | 5 |
| SCLK | 18 |
| MISO | 19 |
| MOSI | 23 |

### Otros

| Función | GPIO | Notas |
| --- | --- | --- |
| LED RGB R | 22 | Ánodo común, activo en LOW |
| LED RGB G | 16 | |
| LED RGB B | 17 | |
| Batería | 34 | ADC, divisor 1:1 aproximado |
| BOOT | 0 | Activo en bajo, pull-up |
| Audio enable | 4 | Se mantiene en LOW |
| DAC audio | 26 | No usado |

## Configuración de TFT_eSPI

Toda la configuración está en `platformio.ini` con `-DUSER_SETUP_LOADED=1`, sin tocar la librería:

```ini
-DST7789_DRIVER=1
-DTFT_WIDTH=240
-DTFT_HEIGHT=320
-DTFT_MISO=12
-DTFT_MOSI=13
-DTFT_SCLK=14
-DTFT_CS=15
-DTFT_DC=2
-DTFT_RST=-1
-DTFT_BL=21
-DTFT_BACKLIGHT_ON=HIGH
-DTFT_INVERSION_ON=1
```

### Variantes

| Síntoma | Cambio |
| --- | --- |
| Pantalla blanca | `ST7789_DRIVER` → `ST7789_2_DRIVER` o `ILI9341_DRIVER` |
| Colores en negativo | Quitar `TFT_INVERSION_ON` |
| Rojo/azul intercambiados | Añadir `-DTFT_RGB_ORDER=TFT_BGR` |
| Imagen desplazada | Probar rotación 1/3 en `display.cpp` y recalibrar |

## Alimentación y batería

La placa admite alimentación por USB o por conector de batería LiPo. La lectura de batería está en GPIO 34 con un divisor resistivo; el firmware multiplica por 2 la lectura ADC y muestra el porcentaje en la barra superior. Si tu placa usa otro divisor, ajusta `readBatteryPercent()` en `src/ui/status_bar.cpp`.

No hay carga rápida ni gestión de carga controlada por el firmware: usa el cargador integrado de la placa.

## Notas de consumo

- Retroiluminación por PWM (5 kHz). Bajar el brillo reduce bastante el consumo.
- WiFi en modo estación. La app no crea punto de acceso.
- El LED RGB se apaga al arrancar y solo se enciende desde Ajustes.
