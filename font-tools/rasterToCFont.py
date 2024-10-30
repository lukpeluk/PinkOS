from PIL import Image
import sys
import os

# Dimensions of a single character
CHAR_WIDTH = 8
CHAR_HEIGHT = 16

def image_to_font_array(image):
    pixels = image.load()
    width, height = image.size
    
    # Check if image dimensions are correct
    if width != CHAR_WIDTH or height != CHAR_HEIGHT:
        raise ValueError(f"Expected {CHAR_WIDTH}x{CHAR_HEIGHT} image, got {width}x{height}.")
    
    font_data = []
    
    # Loop through each row (height of character)
    for y in range(CHAR_HEIGHT):
        byte = 0
        # Loop through each column (width of character)
        for x in range(CHAR_WIDTH):
            # Get pixel: 0 for white (off), 1 for black (on)
            pixel_value = 1 if pixels[x, y] == 0 else 0  # Black pixels are "on"
            byte |= (pixel_value << (7 - x))  # Shift bit to correct position
        font_data.append(byte)
    
    return font_data


# Generate a preview of the font for the terminal
def format_for_preview(data):
    # return ', '.join([f"0b{byte:08b}" for byte in data])

    string = '\n'.join([f"{byte:08b}" for byte in data])
    string = string.replace("1", "■")
    string = string.replace("0", " ")
    return string

# Generate the C-style output
def format_as_c_array(data):
    return ', '.join([f"0b{byte:08b}" for byte in data])



def process_images(input_folder, output_file):
    with open(output_file, 'w') as f:
        f.write("uint8_t font[][16] = {\n")

        # itera alfabéticamente
        # no sé si esto considera el orden de todo ASCII
        for filename in sorted(os.listdir(input_folder)):
            if filename.endswith(".png"):  # Process only PNG files
                try:
                    image_path = os.path.join(input_folder, filename)
                    image = Image.open(image_path).convert('1')
                    font_array = image_to_font_array(image)
                    char_name = filename[:-4]  # Remove the .png extension

                    # Write to the output file in the desired format
                    f.write(f"    // Character '{char_name}'\n")
                    f.write(f"    {{ {format_as_c_array(font_array)} }},\n")
                    print(format_for_preview(font_array))
                    print()
                except Exception as e:
                    print(f"Error processing {filename}: {e}")

        f.write("};\n")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python script.py <input_folder> <output_file>")
        sys.exit(1)

    input_folder = sys.argv[1]
    output_file = sys.argv[2]

    process_images(input_folder, output_file)
    print(f"Font data written to {output_file}")











