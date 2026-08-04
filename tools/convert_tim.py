#!/usr/bin/env python3
"""Convert a 4-bit or 8-bit PlayStation TIM into a row-RLE C texture."""

from __future__ import annotations

import argparse
import re
import struct
from pathlib import Path


def parse_tim(path: Path) -> tuple[int, int, list[int], bytes, int]:
    data = path.read_bytes()
    if len(data) < 20:
        raise ValueError("TIM file is too small")

    magic, flags = struct.unpack_from("<II", data, 0)
    if magic != 0x10:
        raise ValueError("Not a PlayStation TIM file")

    mode = flags & 0x7
    if mode not in (0, 1):
        raise ValueError("Only 4-bit and 8-bit indexed TIM files are supported")
    if (flags & 0x8) == 0:
        raise ValueError("Indexed TIM file has no CLUT")

    offset = 8
    clut_length, _, _, clut_width, clut_height = struct.unpack_from(
        "<IHHHH", data, offset
    )
    clut_data = data[offset + 12 : offset + clut_length]
    colour_count = clut_width * clut_height
    colours = list(struct.unpack_from(f"<{colour_count}H", clut_data))
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
        raise ValueError("TIM pixel block is truncated")

    return width, height, colours, bytes(indices[:pixel_count]), 0


def psx_colour_to_rgbx(colour: int) -> int:
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
            while (
                position + run_length < width
                and row[position + run_length] == row[position]
                and run_length < 255
            ):
                run_length += 1
            encoded.append(run_length)
            encoded.append(row[position])
            position += run_length
        offsets.append(len(encoded))

    if len(encoded) > 0xFFFF:
        raise ValueError("Encoded texture exceeds the 16-bit row-offset format")
    return offsets, bytes(encoded)


def format_array(values: list[int] | bytes, formatter: str, per_line: int) -> str:
    lines = []
    for start in range(0, len(values), per_line):
        chunk = values[start : start + per_line]
        lines.append("    " + ", ".join(formatter.format(value) for value in chunk) + ",")
    return "\n".join(lines)


def symbol_name(raw_name: str) -> str:
    clean = re.sub(r"[^A-Za-z0-9_]", "_", raw_name)
    if not clean or clean[0].isdigit():
        clean = "texture_" + clean
    return clean


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("input", type=Path)
    parser.add_argument("output_c", type=Path)
    parser.add_argument("output_h", type=Path)
    parser.add_argument("--symbol", required=True)
    parser.add_argument("--sample-step", type=int, default=1)
    args = parser.parse_args()

    symbol = symbol_name(args.symbol)
    width, height, colours, pixels, transparent_index = parse_tim(args.input)

    if args.sample_step < 1:
        raise ValueError("--sample-step must be at least 1")
    if args.sample_step > 1:
        source_width = width
        source_height = height
        step = args.sample_step
        sampled = bytearray()
        for source_y in range(0, source_height, step):
            for source_x in range(0, source_width, step):
                sampled.append(pixels[source_y * source_width + source_x])
        width = (source_width + step - 1) // step
        height = (source_height + step - 1) // step
        pixels = bytes(sampled)

    offsets, runs = encode_rows(width, height, pixels)
    palette = [psx_colour_to_rgbx(colour) for colour in colours]
    palette.extend([0x000000FF] * (256 - len(palette)))

    args.output_h.parent.mkdir(parents=True, exist_ok=True)
    args.output_c.parent.mkdir(parents=True, exist_ok=True)
    args.output_h.write_text(
        f"#pragma once\n\n#include \"renderer/texture.h\"\n\n"
        f"extern const TextureRle {symbol};\n",
        encoding="utf-8",
    )

    header_parts = args.output_h.as_posix().split("/")
    if "include" in header_parts:
        header_include = "/".join(header_parts[header_parts.index("include") + 1 :])
    else:
        header_include = args.output_h.as_posix()

    source = f'''/* Generated from {args.input.name} by tools/convert_tim.py. */
#include "{header_include}"

#include <stdint.h>

static const uint32_t kPalette[256] = {{
{format_array(palette, "0x{:08X}u", 6)}
}};

static const uint16_t kRowOffsets[{len(offsets)}] = {{
{format_array(offsets, "{}u", 12)}
}};

static const uint8_t kRuns[{len(runs)}] = {{
{format_array(runs, "{}u", 20)}
}};

const TextureRle {symbol} = {{
    {width}u,
    {height}u,
    {transparent_index}u,
    kRowOffsets,
    kRuns,
    kPalette,
}};
'''
    args.output_c.write_text(source, encoding="utf-8")

    print(
        f"Converted {args.input}: {width}x{height}, "
        f"{len(runs)} RLE bytes, {len(colours)} palette colours"
    )


if __name__ == "__main__":
    main()
