# R28T OS

Pequeño “sistema operativo” para la placa **E32R28T**: ESP32-WROOM-32E con pantalla táctil resistiva de 2,8" (240×320), pensado para sacarle todo el partido a la pantalla.

Firmware en **PlatformIO + Arduino + LVGL 8**, con lanzador de apps y cada aplicación en su propio módulo.

```
┌──────────────────────────────┐
│ R28T OS        ● WiFi  82%   │  barra de estado
├──────────────────────────────┤
│ Buenos días                 │
│ jueves, 19 de noviembre      │
│                              │
│  ┌────────┐   ┌────────┐     │
│  │  Reloj │   │ Tiempo │     │
│  └────────┘   └────────┘     │
│  ┌────────┐   ┌────────┐     │
│  │Galería │   │Ajustes │     │
│  └────────┘   └────────┘     │
└──────────────────────────────┘
```

## Apps incluidas

| App | Qué hace |
| --- | --- |
| **Reloj** | Hora y fecha en grande, segundos, fondo degradado animado con estrellas y luna. 3 ambientes (toca el fondo para cambiar): Aurora, Medianoche y Atardecer. |
| **Tiempo** | Datos actuales y previsión de 3 días con [Open-Meteo](https://open-meteo.com/) (sin API key). Ciudad y coordenadas configurables. Iconos dibujados con LVGL. |
| **WiFi** | Escaneo de redes, conexión con teclado en pantalla, estado/IP/señal, desconectar y olvidar red. Credenciales guardadas en NVS. |
| **Galería SD** | Lista las fotos `.jpg`/`.bmp` de la tarjeta. Visor a pantalla completa con anterior/siguiente. |
| **Ajustes** | Brillo, ambiente del reloj, zona horaria, ciudad/coordenadas, acceso a WiFi, calibración táctil de 4 puntos, LED RGB y información del sistema. |

Extras: barra con hora, WiFi y batería; botón **BOOT** para volver al inicio; LED RGB de la placa; NTP automático al conectar WiFi.

## Hardware

Placa **E32R28T** (DIYmalls / familia LCDWiki), ESP32-WROOM-32E, 2,8" 240×320.

| Función | Pines |
| --- | --- |
| Pantalla ST7789 — CS / DC / SCLK / MOSI / MISO | 15 / 2 / 14 / 13 / 12 |
| Retroiluminación | 21 (alto = encendida) |
| Reset de pantalla | Compartido con EN |
| Táctil XPT2046 — CLK / MOSI / MISO / CS / IRQ | 25 / 32 / 39 / 33 / 36 |
| microSD — CS / SCLK / MISO / MOSI | 5 / 18 / 19 / 23 |
| LED RGB (activo en bajo) | R 22 · G 16 · B 17 |
| Batería (ADC) | 34 |
| Botón BOOT | 0 |
| Enable de audio | 4 (se deja desactivado) |
| DAC de audio | 26 (no usado) |

> La pantalla se ha vendido con **ST7789** e **ILI9341** según el lote. Este proyecto arranca con ST7789. Si sale en blanco o con colores raros, mira [Solución de problemas](#solución-de-problemas).

Detalles y variantes: [`docs/HARDWARE.md`](docs/HARDWARE.md).

## Estructura

```
esp32-r28t-os/
├── platformio.ini          # placa, flags de TFT_eSPI, librerías
├── include/
│   ├── config.h            # pines y valores por defecto
│   ├── lv_conf.h           # configuración de LVGL
│   ├── app.h               # interfaz común de apps
│   ├── core/               # ajustes, WiFi, hora, tiempo, SD
│   ├── hal/                # pantalla, táctil, LED
│   ├── ui/                 # router, tema, iconos, barra, teclado
│   └── apps/               # cabeceras de las apps
├── src/
│   ├── main.cpp
│   ├── core/
│   ├── hal/
│   ├── ui/
│   └── apps/               # reloj, tiempo, wifi, galería, ajustes
├── docs/
├── tools/
│   └── generate_fonts.ps1  # regenera las fuentes con acentos
└── .github/workflows/      # compilación automática en GitHub
```

### Cómo añadir una app

1. Crea `include/apps/miapp.h` y `src/apps/miapp.cpp` heredando de `apps::App`.
2. Implementa `title()`, `build(lv_obj_t* screen)` y, si usas `lv_timer`, `onDestroy()`.
3. Añádela al enum `ui::AppId` y al `switch` de `src/ui/router.cpp`.
4. Añade una tarjeta en `src/apps/launcher.cpp`.

## Compilar y flashear

Requisitos: [PlatformIO Core](https://platformio.org/install/cli) o la extensión PlatformIO de VS Code, y un cable USB de datos.

```bash
pip install platformio          # si no lo tienes
pio run                         # compilar
pio run -t upload               # flashear
pio device monitor              # ver logs (115200)
```

También puedes abrir la carpeta con VS Code + PlatformIO y usar los botones **Build** / **Upload**.

Las fuentes con acentos ya están generadas en `src/assets/fonts/`, no hay que hacer nada. Solo si quieres cambiar tamaños o caracteres: `npm install -g lv_font_conv` y luego `tools\generate_fonts.ps1`.

Si la subida falla, mantén pulsado **BOOT** al empezar a flashear o baja `upload_speed` a `460800` en `platformio.ini`.

## Primer arranque

1. **WiFi**: `Ajustes › WiFi › Escanear`, elige tu red (2,4 GHz) y escribe la contraseña. La hora se sincroniza sola por NTP.
2. **Tiempo**: `Ajustes › Tiempo > Ciudad / Latitud / Longitud`. Por defecto Madrid (40,4168 / -3,7038). Abre la app Tiempo y pulsa refrescar.
3. **Zona horaria**: `Ajustes › Reloj › Zona horaria` alterna España peninsular / Canarias.
4. **Fotos**: copia `.jpg` o `.bmp` en la raíz de una microSD **FAT32**. Recomendado 240×320. El visor también acepta imágenes mayores y las reduce, pero se verán mejor ajustadas.
5. **Táctil**: si los toques no caen donde deben, `Ajustes › Táctil › Calibrar pantalla` y sigue los 4 puntos.

## Controles

- **Tocar**: navegación normal.
- **BOOT**: volver al lanzador desde cualquier app.
- **Reloj**: toca el fondo para cambiar entre Aurora, Medianoche y Atardecer.
- **Galería**: en el visor, tercio izquierdo = anterior, centro = volver, tercio derecho = siguiente.

## Solución de problemas

### La pantalla se queda en blanco

El driver no es el esperado. En `platformio.ini`, dentro de `build_flags`, cambia:

```ini
-DST7789_DRIVER=1
```

por alguno de estos según tu lote:

```ini
-DST7789_2_DRIVER=1
# o
-DILI9341_DRIVER=1
```

### Colores invertidos o rojo/azul cambiados

- Colores en negativo: quita `-DTFT_INVERSION_ON=1`.
- Rojo y azul intercambiados: añade `-DTFT_RGB_ORDER=TFT_BGR`.
- Si la imagen sale desplazada, prueba `tft_.setRotation(1)` o `3` en `src/hal/display.cpp` (el táctil habrá que recalibrarlo).

### El táctil no responde o va cruzado

- Comprueba que el IRQ (GPIO 36) está conectado; el driver lo usa para detectar el contacto.
- Ejes cambiados: pon `#define TOUCH_SWAP_XY 1` en `include/config.h`.
- Ejes invertidos: `TOUCH_INVERT_X` / `TOUCH_INVERT_Y`.
- Siempre puedes recalibrar desde Ajustes.

### El tiempo no carga

- Necesita WiFi conectado y salida a Internet.
- Comprueba que la ciudad/coordenadas son válidas.
- Open-Meteo se consulta por HTTP; si tu red bloquea puertos no estándar, cambia el firmware a HTTPS (requiere más RAM).

### La SD no aparece

- Formato FAT32 (no exFAT), contacto limpio y tarjeta de 32 GB o menos.
- Nombres de archivo sin caracteres raros; el firmware lista `.jpg`, `.jpeg` y `.bmp`.
- JPEG progresivos no están soportados; guarda en JPEG baseline.

### Se reinicia o no cabe el firmware

El proyecto usa `huge_app.csv` (3 MB para la app). Asegúrate de que la placa tiene 4 MB de flash o más.

## Subir el repositorio a GitHub

```bash
git init
git add .
git commit -m "R28T OS: launcher, reloj, WiFi, tiempo y galería SD"
gh auth login
gh repo create esp32-r28t-os --public --source=. --remote=origin --push
```

Cambia `--public` por `--private` si prefieres el repo privado.

## Hoja de ruta

- [ ] Control domótico por MQTT (luces, enchufes).
- [ ] Reproductor de audio por DAC (GPIO 26).
- [ ] Widgets en el lanzador (tiempo, próxima alarma).
- [ ] Fondo del reloj con imágenes desde SD.
- [ ] OTA desde Ajustes.
- [ ] Modo reloj de noche (brillo mínimo).

## Licencia

MIT. Consulta [`LICENSE`](LICENSE).
