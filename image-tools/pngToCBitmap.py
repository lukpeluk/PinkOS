from PIL import Image
import sys
import os

def png_to_c_bitmap(file_path):
    # Load the image
    img = Image.open(file_path).convert("RGBA")  # Convert to RGBA to ensure 32-bit color
    
    # Get width and height
    width, height = img.size
    
    # Initialize the output array as a list of hex values
    bitmap_array = []

    for y in range(height):
        for x in range(width):
            r, g, b, a = img.getpixel((x, y))
            # Convert RGBA to a single 32-bit integer (0xAARRGGBB format)
            color_hex = (a << 24) | (r << 16) | (g << 8) | b
            bitmap_array.append(f"0x{color_hex:08X}")
    
    # Convert list to the desired C-style output
    array_str = ",\n\t".join(", ".join(bitmap_array[i:i+width]) for i in range(0, len(bitmap_array), width))
    
    # Print the final C array code
    print(f"#include <stdint.c>\n\n// {width} by {height} px bitmap\nuint32_t bitmap[{width * height}] = {{\n\t{array_str}\n}};")


# convert the first argument recieved to a C-style bitmap
png_to_c_bitmap(sys.argv[1])



