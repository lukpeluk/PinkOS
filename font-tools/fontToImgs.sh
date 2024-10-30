#!/bin/bash

# Convert all the characters in the font to images, saving them in the directory specified as argument
# Usage: ./fontToImgs.sh <font> <outputDir>
# Depende de [imagemagick](https://imagemagick.org/index.php)

height=16
width=8

# Este bucle define el orden de los caracteres y qué caracteres se convierten
for i in {0..9} {a..z};
do
    magick -background white -fill black -font $1 -pointsize 16 -gravity center -size 8x16 label:"$i" $2/$i.png

done

# magick -background white -fill black -font pink-panther.TTF -pointsize 16 -gravity center -size 8x16 label:"Z" z.png
