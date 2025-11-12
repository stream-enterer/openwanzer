#!/usr/bin/env python3
"""
Create placeholder images for OpenWanzer development
Uses only built-in Python modules to create minimal PNG files
"""

import struct
import zlib
import os

def create_png(width, height, color=(128, 128, 128, 255)):
    """Create a simple PNG image with a solid color"""
    # PNG signature
    png_data = b'\x89PNG\r\n\x1a\n'

    # IHDR chunk (image header)
    ihdr_data = struct.pack('>IIBBBBB', width, height, 8, 6, 0, 0, 0)
    ihdr_chunk = create_chunk(b'IHDR', ihdr_data)
    png_data += ihdr_chunk

    # IDAT chunk (image data)
    raw_data = b''
    for y in range(height):
        raw_data += b'\x00'  # Filter type: None
        for x in range(width):
            raw_data += bytes(color)

    compressed_data = zlib.compress(raw_data, 9)
    idat_chunk = create_chunk(b'IDAT', compressed_data)
    png_data += idat_chunk

    # IEND chunk (end of file)
    iend_chunk = create_chunk(b'IEND', b'')
    png_data += iend_chunk

    return png_data

def create_chunk(chunk_type, data):
    """Create a PNG chunk with CRC"""
    length = struct.pack('>I', len(data))
    crc = zlib.crc32(chunk_type + data) & 0xffffffff
    crc_bytes = struct.pack('>I', crc)
    return length + chunk_type + data + crc_bytes

def ensure_dir(path):
    """Create directory if it doesn't exist"""
    os.makedirs(path, exist_ok=True)

# Create placeholder images
def main():
    print("Creating placeholder images...")

    # Map background (600x450 pixels, light green terrain)
    ensure_dir('resources/maps/images')
    with open('resources/maps/images/tutorial_map.png', 'wb') as f:
        f.write(create_png(600, 450, (120, 180, 100, 255)))
    print("✓ Created map background")

    # Unit images (sprite sheet: 9 orientations side by side, 36x32 each = 324x32)
    ensure_dir('resources/units/images')
    unit_colors = [
        (100, 100, 150, 255),  # Blue-grey for vehicles
        (80, 120, 80, 255),     # Green for infantry
        (150, 100, 80, 255),    # Brown for artillery
    ]

    for i in range(10):
        color = unit_colors[i % len(unit_colors)]
        with open(f'resources/units/images/unit{i}.png', 'wb') as f:
            f.write(create_png(324, 32, color))
    print("✓ Created unit sprite sheets")

    # Flags (sprite sheet: multiple flags, 21x14 each, 10 flags = 210x14)
    ensure_dir('resources/ui/flags')
    with open('resources/ui/flags/flags_med.png', 'wb') as f:
        f.write(create_png(630, 14, (200, 50, 50, 255)))
    print("✓ Created flags")

    # Unit fire indicator (16x16)
    ensure_dir('resources/ui/indicators')
    with open('resources/ui/indicators/unit-fire.png', 'wb') as f:
        f.write(create_png(16, 16, (255, 100, 0, 255)))
    print("✓ Created unit indicators")

    print("\nAll placeholder images created successfully!")

if __name__ == '__main__':
    main()
