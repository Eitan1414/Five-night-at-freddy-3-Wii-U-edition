#!/usr/bin/env python3
"""Convert the user-supplied 17-frame Phantom Chica PNG strip to Wii U row-RLE."""
from __future__ import annotations

import argparse
from collections import Counter
from pathlib import Path

from PIL import Image, ImageFile

# The checked-in Chica sheet is reconstructed from split development chunks.
# Pillow is deliberately strict with a PNG whose stream ends a little early,
# even when all useful scanlines are present. Allow that specific case so the
# asset pipeline can recover the supplied frames instead of aborting the build.
ImageFile.LOAD_TRUNCATED_IMAGES = True

FRAME_WIDTH = 303
FRAME_HEIGHT = 227
FRAME_COUNT = 17
TRANSPARENT_INDEX = 255
ALPHA_THRESHOLD = 96


def encode_rows(width: int, height: int, pixels: bytes) -> tuple[list[int], bytes]:
    offsets = [0]
    encoded = bytearray()
    for row_index in range(height):
        row = pixels[row_index * width:(row_index + 1) * width]
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
        raise ValueError("encoded Chica frame exceeds 16-bit row offsets")
    return offsets, bytes(encoded)


def fmt(values, pattern: str, per_line: int) -> str:
    lines = []
    for start in range(0, len(values), per_line):
        chunk = values[start:start + per_line]
        lines.append("    " + ", ".join(pattern.format(v) for v in chunk) + ",")
    return "\n".join(lines)


def rgba_frame_to_indexed(frame: Image.Image) -> tuple[list[int], bytes]:
    rgba = frame.convert("RGBA")
    pixels = list(rgba.getdata())

    frequencies = Counter(
        (r, g, b) for r, g, b, a in pixels if a >= ALPHA_THRESHOLD
    )
    colours = [colour for colour, _ in frequencies.most_common(254)]
    if not colours:
        colours = [(0, 0, 0)]
    colour_to_index = {colour: index for index, colour in enumerate(colours)}

    def nearest_index(r: int, g: int, b: int) -> int:
        exact = colour_to_index.get((r, g, b))
        if exact is not None:
            return exact
        return min(
            range(len(colours)),
            key=lambda i: ((colours[i][0] - r) ** 2 +
                           (colours[i][1] - g) ** 2 +
                           (colours[i][2] - b) ** 2),
        )

    indexed = bytearray()
    for r, g, b, a in pixels:
        if a < ALPHA_THRESHOLD:
            indexed.append(TRANSPARENT_INDEX)
        else:
            indexed.append(nearest_index(r, g, b))

    palette = [
        (r << 24) | (g << 16) | (b << 8) | 0xFF
        for r, g, b in colours
    ]
    return palette, bytes(indexed)


def emit_texture(source: list[str], symbol: str, frame: Image.Image) -> None:
    palette, pixels = rgba_frame_to_indexed(frame)
    offsets, runs = encode_rows(FRAME_WIDTH, FRAME_HEIGHT, pixels)

    source.append(f"static const uint32_t {symbol}Palette[{len(palette)}] = {{\n")
    source.append(fmt(palette, "0x{:08X}u", 6))
    source.append("\n};\n")
    source.append(f"static const uint16_t {symbol}RowOffsets[{len(offsets)}] = {{\n")
    source.append(fmt(offsets, "{}u", 12))
    source.append("\n};\n")
    source.append(f"static const uint8_t {symbol}Runs[{len(runs)}] = {{\n")
    source.append(fmt(runs, "{}u", 20))
    source.append("\n};\n")
    source.append(
        f"static const TextureRle {symbol} = {{\n"
        f"    {FRAME_WIDTH}u, {FRAME_HEIGHT}u, {TRANSPARENT_INDEX}u,\n"
        f"    {symbol}RowOffsets, {symbol}Runs, {symbol}Palette,\n"
        f"}};\n\n"
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("input_png", type=Path)
    parser.add_argument("output_c", type=Path)
    parser.add_argument("output_h", type=Path)
    args = parser.parse_args()

    sheet = Image.open(args.input_png)
    sheet.load()
    expected_size = (FRAME_WIDTH, FRAME_HEIGHT * FRAME_COUNT)
    if sheet.size != expected_size:
        raise ValueError(
            f"unexpected Chica strip size {sheet.size}; expected {expected_size}"
        )

    source = [
        "/* Generated from the user-supplied Phantom Chica sprite sheet. */\n",
        '#include "assets/phantom_chica_user_jumpscare.h"\n\n',
        "#include <stdint.h>\n\n",
    ]
    frame_symbols = []
    for index in range(FRAME_COUNT):
        frame = sheet.crop((
            0,
            index * FRAME_HEIGHT,
            FRAME_WIDTH,
            (index + 1) * FRAME_HEIGHT,
        ))
        symbol = f"kPhantomChicaUserFrame{index:02d}"
        emit_texture(source, symbol, frame)
        frame_symbols.append(symbol)

    source.append(
        f"static const TextureRle *const kPhantomChicaUserFrames[{FRAME_COUNT}] = {{\n"
    )
    source.append("\n".join(f"    &{symbol}," for symbol in frame_symbols))
    source.append("\n};\n\n")
    source.append(
        "const JumpscareSequence gPhantomChicaUserJumpscare = {\n"
        f"    kPhantomChicaUserFrames, {FRAME_COUNT}u, 1u,\n"
        "};\n"
    )

    header = """#pragma once

#include \"assets/jumpscare_assets.h\"

/*
 * 17-frame Phantom Chica jumpscare supplied for the Wii U edition.
 * Camera visuals stay owned by the existing camera pack.
 */
extern const JumpscareSequence gPhantomChicaUserJumpscare;

/* Redirect only the jumpscare sequence. */
#define gPhantomChicaRealJumpscare gPhantomChicaUserJumpscare
"""

    args.output_c.parent.mkdir(parents=True, exist_ok=True)
    args.output_h.parent.mkdir(parents=True, exist_ok=True)
    args.output_c.write_text("".join(source), encoding="utf-8")
    args.output_h.write_text(header, encoding="utf-8")
    print(f"Phantom Chica: {FRAME_COUNT} user frames at 1 tick/frame")


if __name__ == "__main__":
    main()
