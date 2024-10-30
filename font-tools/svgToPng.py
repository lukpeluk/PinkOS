import cairosvg
import sys
import os
from PIL import Image

CHAR_WIDTH = 8
CHAR_HEIGHT = 16

# Esto define con qué nombre se van guardando los archivos
# Debería usar el orden ascii
def increment_filename(last_filename):
    if last_filename == '9':
        return 'a'
    elif last_filename == 'z':
        return 'A'
    elif last_filename == 'Z':
        return 'a'
    else:
        return chr(ord(last_filename) + 1)


def convert_svg_to_png(svg_file, temp_folder):
    # Get the last filename in alphabetical order
    last_filename = '0'
    for filename in os.listdir(temp_folder):
        if filename.endswith(".png"):
            last_filename = max(last_filename, filename[0])

    last_filename = increment_filename(last_filename)

    # Convert SVG to temporary PNG file
    temp_png_file = os.path.join(temp_folder, f'{last_filename}_good_res.png')
    final_png_file = os.path.join(temp_folder, f'{last_filename}.png')
    cairosvg.svg2png(url=svg_file, write_to=temp_png_file)

    print(f"Converted '{svg_file}' to '{temp_png_file}'")

    # Open the temporary PNG file and maintain aspect ratio
    with Image.open(temp_png_file) as img:
        # Get original dimensions
        orig_width, orig_height = img.size
        
        # Calculate the aspect ratio
        aspect_ratio = orig_width / orig_height

        # Resize the image
        img = img.resize((CHAR_WIDTH, CHAR_HEIGHT), Image.LANCZOS)
        img.save(final_png_file)


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python convert_svg_to_png.py <input_svg> <output_folder>")
        sys.exit(1)

    input_svg = sys.argv[1]
    temp_folder = sys.argv[2]

    # Ensure the temporary folder exists
    os.makedirs(temp_folder, exist_ok=True)

    convert_svg_to_png(input_svg, temp_folder)




