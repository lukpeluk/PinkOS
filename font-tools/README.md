# Scripts para la importación de tipografías a PinkOS
Básicamente permite usar en nuestro OS cualquier tipografía que nos guste, y crear las nuestras sin tener que escribirlas bit a bit
En esta carpeta hay un par de demos y pruebas que fui haciendo 

## Formatos soportados:
- SVG, TTF, PNG

TODO: actualizar esto porque cambió el formato, ahora son punteros los caracteres
El formato de fuentes de PinkOS es por ahora un arreglo de caracteres donde cada caracter es un arreglo de bytes, donde cada byte es una línea horizontal, representando cada bit un pixel.
Un 1 representa un pixel encendido.

Ej:
El siguiente código es una fuente con 2 caracteres
```C
uint8_t font[][16] = {
    { 0b00000000, 0b01111100, 0b11101110, 0b10000010, 0b10000111, 0b10011110, 0b10111010, 0b11100011, 0b11100010, 0b10000010, 0b11101110, 0b01111100, 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
    { 0b00000000, 0b00010000, 0b00110000, 0b01110000, 0b00011000, 0b00010000, 0b00010000, 0b00010000, 0b00011000, 0b00010000, 0b00111000, 0b01111100, 0b00000000, 0b00000000, 0b00000000, 0b00000000 }
}
```

## Scripts disponibles:
* rasterToCFont.py
    - dependencias: pillow
    - uso: python script.py <input_folder> <output_file>
    - lee todas las imágenes (png) de la carpeta indicada y las guarda como caracteres de una fuente de PinkOS en el archivo de salida
    - ignora imágenes que no tengan la resolución correcta (podría reescalarlas igual, no sería complicado)
    - las lee en orden alfabético según el nombre del archivo, a futuro tendría que usar PinkMappings

* fontToImgs.sh
    - dependencias: [imagemagick](https://imagemagick.org/index.php)
    - uso: ./fontToImgs.sh <font> <outputDir>
    - guarda la fuente TTF pasada como imágenes en la carpeta indicada
    - por ahora solo guarda del 0 al 9 y la a a la z

* svgToPng.py
    - dependencias: cairosvg, pillow
    - uso: python convert_svg_to_png.py <input_svg> <output_folder>
    - convierte el SVG a PNG, y lo guarda en la carpeta de salida
    - guarda una versión a mejor resolución, además de la de las dimensiones de la fuente
    - nombra las imágenes siguiendo el orden de PinkMapping (por ahora del 0 al 9 y de la a a la z)
    - la idea de este script era poder usar el trabajo de POO para hacer fuentes


