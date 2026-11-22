# ============================================================
#  Regenera las fuentes de la interfaz (src/assets/fonts).
#
#  Hace falta lv_font_conv:
#      npm install -g lv_font_conv
#
#  Las fuentes de LVGL no traen acentos, así que generamos
#  versiones propias con Latin-1 (0xA0-0xFF) y unos cuantos
#  signos tipográficos que usa la interfaz:
#      0x2013 –   0x2014 —   0x2022 •   0x2026 …   0x203A ›
#
#  Uso (desde la raíz del proyecto):
#      powershell -ExecutionPolicy Bypass -File tools\generate_fonts.ps1
# ============================================================

$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..")

$lvgl = ".pio/libdeps/esp32dev/lvgl/scripts/built_in_font"
$montserrat = "$lvgl/Montserrat-Medium.ttf"
$awesome = "$lvgl/FontAwesome5-Solid+Brands+Regular.woff"

if (-not (Test-Path $montserrat)) {
    throw "No encuentro $montserrat. Ejecuta antes: python -m platformio run"
}

# Símbolos de FontAwesome que usa LVGL (los mismos que las fuentes de serie).
$symbols = "61441,61448,61451,61452,61452,61453,61457,61459,61461,61465," +
           "61468,61473,61478,61479,61480,61502,61507,61512,61515,61516," +
           "61517,61521,61522,61523,61524,61543,61544,61550,61552,61553," +
           "61556,61559,61560,61561,61563,61587,61589,61636,61637,61639," +
           "61641,61664,61671,61674,61683,61724,61732,61787,61931,62016," +
           "62017,62018,62019,62020,62087,62099,62212,62189,62810,63426,63650"

$latin = "0x20-0x7F,0xA0-0xFF,0x2013,0x2014,0x2022,0x2026,0x203A"

foreach ($size in 10, 12, 14, 16, 20) {
    $out = "src/assets/fonts/ui_font_$size.c"
    Write-Host "Generando $out ..."
    lv_font_conv --size $size --bpp 4 --format lvgl -o $out `
        --font $montserrat -r $latin `
        --font $awesome -r $symbols `
        --force-fast-kern-format --no-compress --no-prefilter `
        --lv-font-name "ui_font_$size"
    if ($LASTEXITCODE -ne 0) { throw "lv_font_conv falló para $size px" }

    # lv_font_conv usa "lvgl/lvgl.h" por defecto; PlatformIO ya añade
    # el directorio de LVGL a la ruta de include, así que lo dejamos simple.
    $text = [IO.File]::ReadAllText((Resolve-Path $out))
    $text = $text.Replace('#include "lvgl/lvgl.h"', '#include <lvgl.h>')
    [IO.File]::WriteAllText((Resolve-Path $out), $text)
}

Write-Host "Listo. Vuelve a compilar con: python -m platformio run"
