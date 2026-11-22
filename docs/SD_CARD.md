# Tarjeta SD y fotos

## Formato

- FAT32.
- 32 GB o menos es lo más seguro.
- Archivos en la **raíz** de la tarjeta.
- Extensiones soportadas: `.jpg`, `.jpeg`, `.bmp`.

## Recomendaciones para que se vean perfectas

- Tamaño: **240×320** (vertical) o 320×240 (horizontal).
- JPEG baseline (no progresivo), calidad 80–90.
- BMP de 24 bits.
- Nombres cortos y sin tildes: `playa.jpg`, `gato_01.jpg`.

El visor escala JPEG mayores (1/2, 1/4, 1/8) y los centra. Aun así, una imagen ya recortada a 240×320 se ve más nítida y tarda menos en abrir.

## Convertir fotos en Windows (PowerShell)

Con [ImageMagick](https://imagemagick.org/) instalado:

```powershell
magick mogrify -path .\salida -auto-orient -resize 240x320^ -gravity center -extent 240x320 -quality 88 *.jpg
```

Copia el contenido de `salida` a la raíz de la tarjeta.

## Consejos

- Evita imágenes de más de 5 MB: el ESP32 no tiene PSRAM y la decodificación es más lenta.
- Si una foto sale con colores raros, comprueba el orden RGB/BGR en `platformio.ini`.
- Si el visor no abre una imagen, prueba a guardarla de nuevo como JPEG baseline.
