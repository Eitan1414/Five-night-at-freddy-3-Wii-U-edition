#!/usr/bin/env python3
"""Convert the ten original PSX camera Springtrap TIMs to Wii U RLE textures."""
from __future__ import annotations

import argparse
import struct
from pathlib import Path


def parse_tim(path: Path) -> tuple[int, int, list[int], bytes]:
    data = path.read_bytes()
    if len(data) < 20:
        raise ValueError(f"{path}: TIM file is too small")
    magic, flags = struct.unpack_from("<II", data, 0)
    if magic != 0x10:
        raise ValueError(f"{path}: not a PlayStation TIM file")
    mode = flags & 0x7
    if mode not in (0, 1):
        raise ValueError(f"{path}: only 4-bit and 8-bit TIM files are supported")
    if (flags & 0x8) == 0:
        raise ValueError(f"{path}: indexed TIM has no CLUT")

    offset = 8
    clut_length, _, _, clut_width, clut_height = struct.unpack_from(
        "<IHHHH", data, offset
    )
    colour_count = clut_width * clut_height
    clut_data = data[offset + 12 : offset + clut_length]
    colours = list(struct.unpack_from(f"<{colour_count}H", clut_data))
    colours = colours[:clut_width]
    offset += clut_length

    image_length, _, _, width_words, height = struct.unpack_from(
        "<IHHHH", data, offset
    )
    image_data = data[offset + 12 : offset + image_length]
    if mode == 0:
        width = width_words * 4
        indices = bytearray()
        for value in image_data:
            indices.append(value & 0x0F)
            indices.append(value >> 4)
    else:
        width = width_words * 2
        indices = bytearray(image_data)

    pixel_count = width * height
    if len(indices) < pixel_count:
        # Several original camera TIMs end a few bytes early. The missing
        # tail is transparent padding, so preserve the intended dimensions.
        indices.extend(b"\x00" * (pixel_count - len(indices)))
    return width, height, colours, bytes(indices[:pixel_count])


def psx_colour_to_rgba(colour: int) -> int:
    red = (colour & 0x1F) * 255 // 31
    green = ((colour >> 5) & 0x1F) * 255 // 31
    blue = ((colour >> 10) & 0x1F) * 255 // 31
    return (red << 24) | (green << 16) | (blue << 8) | 0xFF


def encode_rows(width: int, height: int, pixels: bytes) -> tuple[list[int], bytes]:
    offsets = [0]
    encoded = bytearray()
    for row_index in range(height):
        row = pixels[row_index * width : (row_index + 1) * width]
        position = 0
        while position < width:
            run_length = 1
            while (position + run_length < width and
                   row[position + run_length] == row[position] and
                   run_length < 255):
                run_length += 1
            encoded.extend((run_length, row[position]))
            position += run_length
        offsets.append(len(encoded))
    if len(encoded) > 0xFFFF:
        raise ValueError("encoded texture exceeds 16-bit row offsets")
    return offsets, bytes(encoded)


def fmt(values, pattern: str, per_line: int) -> str:
    lines = []
    for start in range(0, len(values), per_line):
        chunk = values[start:start + per_line]
        lines.append("    " + ", ".join(pattern.format(v) for v in chunk) + ",")
    return "\n".join(lines)


def camera_path(root: Path, camera: int) -> Path:
    pair_start = ((camera - 1) // 2) * 2 + 1
    pair = f"{pair_start:02d}{pair_start + 1:02d}"
    return root / pair / f"SPRINGTRAP{camera}.tim"


def emit_texture(source: list[str], camera: int, width: int, height: int,
                 colours: list[int], pixels: bytes) -> str:
    symbol = f"gOriginalCamera{camera:02d}SpringtrapTexture"
    unique = f"kCamera{camera:02d}Springtrap"
    offsets, runs = encode_rows(width, height, pixels)
    palette_size = 16 if max(pixels, default=0) < 16 else 256
    palette = [psx_colour_to_rgba(colour) for colour in colours[:palette_size]]
    palette.extend([0x000000FF] * (palette_size - len(palette)))

    source.append(f"static const uint32_t {unique}Palette[{palette_size}] = {{\n")
    source.append(fmt(palette, "0x{:08X}u", 6))
    source.append("\n};\n")
    source.append(f"static const uint16_t {unique}RowOffsets[{len(offsets)}] = {{\n")
    source.append(fmt(offsets, "{}u", 12))
    source.append("\n};\n")
    source.append(f"static const uint8_t {unique}Runs[{len(runs)}] = {{\n")
    source.append(fmt(runs, "{}u", 20))
    source.append("\n};\n")
    source.append(
        f"const TextureRle {symbol} = {{\n"
        f"    {width}u, {height}u, 0u,\n"
        f"    {unique}RowOffsets, {unique}Runs, {unique}Palette,\n"
        f"}};\n\n"
    )
    return symbol


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("map_root", type=Path,
                        help="tim/camera/cams/map directory")
    parser.add_argument("output_c", type=Path)
    parser.add_argument("output_h", type=Path)
    args = parser.parse_args()

    source = [
        "/* Generated from the ten original PSX camera Springtrap TIM sprites. */\n",
        '#include "assets/camera_springtrap_assets.h"\n\n',
        "#include <stdint.h>\n\n",
    ]
    header = [
        "#pragma once\n\n",
        '#include "renderer/texture.h"\n\n',
    ]
    symbols: list[str] = []

    for camera in range(1, 11):
        path = camera_path(args.map_root, camera)
        width, height, colours, pixels = parse_tim(path)
        symbol = emit_texture(source, camera, width, height, colours, pixels)
        symbols.append(symbol)
        header.append(f"extern const TextureRle {symbol};\n")
        print(f"CAM {camera:02d}: {path.name} -> {width}x{height}")

    source.append("const TextureRle *const gOriginalCameraSpringtrapTextures[10] = {\n")
    source.extend(f"    &{symbol},\n" for symbol in symbols)
    source.append("};\n")
    header.append("\nextern const TextureRle *const gOriginalCameraSpringtrapTextures[10];\n")

    args.output_c.parent.mkdir(parents=True, exist_ok=True)
    args.output_h.parent.mkdir(parents=True, exist_ok=True)
    args.output_c.write_text("".join(source), encoding="utf-8")
    args.output_h.write_text("".join(header), encoding="utf-8")


if __name__ == "__main__":
    main()
