#!/usr/bin/env python3
"""Batch-convert the original FNaF 3 PSX screamer TIM sequences into Wii U row-RLE textures."""
from __future__ import annotations

import argparse
import struct
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class SequenceSpec:
    symbol: str
    relative_dir: str
    file_prefix: str
    count: int


SEQUENCES = (
    SequenceSpec("gSpringtrapJumpscareLeft", "Springtrap", "", 13),
    SequenceSpec("gSpringtrapJumpscareRight", "Springtrap2", "", 12),
    SequenceSpec("gPhantomBBRealJumpscare", "BB", "BBJUMP", 9),
    SequenceSpec("gPhantomChicaRealJumpscare", "Chica", "CHJUMP", 6),
    SequenceSpec("gPhantomFreddyRealJumpscare", "Freddy", "FJUMP", 7),
    SequenceSpec("gPhantomFoxyRealJumpscare", "Foxy", "", 11),
)


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
        raise ValueError(f"{path}: TIM pixel block is truncated")
    return width, height, colours, bytes(indices[:pixel_count])


def psx_colour_to_rgbx(colour: int) -> int:
    red = (colour & 0x1F) * 255 // 31
    green = ((colour >> 5) & 0x1F) * 255 // 31
    blue = ((colour >> 10) & 0x1F) * 255 // 31
    return (red << 24) | (green << 16) | (blue << 8) | 0xFF


def pad_frame(width: int, height: int, pixels: bytes,
              canvas_width: int, canvas_height: int) -> bytes:
    """Centre cropped PSX frames horizontally and align them to the bottom."""
    result = bytearray(canvas_width * canvas_height)
    offset_x = (canvas_width - width) // 2
    offset_y = canvas_height - height
    for source_y in range(height):
        source_start = source_y * width
        destination_start = (offset_y + source_y) * canvas_width + offset_x
        result[destination_start:destination_start + width] = \
            pixels[source_start:source_start + width]
    return bytes(result)


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


def frame_path(root: Path, spec: SequenceSpec, number: int) -> Path:
    return root / spec.relative_dir / f"{spec.file_prefix}{number}.tim"


def emit_texture(source: list[str], unique: str, width: int, height: int,
                 colours: list[int], pixels: bytes) -> None:
    offsets, runs = encode_rows(width, height, pixels)
    palette_size = 16 if max(pixels, default=0) < 16 else 256
    palette = [psx_colour_to_rgbx(colour) for colour in colours[:palette_size]]
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
        f"static const TextureRle {unique} = {{\n"
        f"    {width}u, {height}u, 0u,\n"
        f"    {unique}RowOffsets, {unique}Runs, {unique}Palette,\n"
        f"}};\n\n"
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("input_root", type=Path,
                        help="directory containing Springtrap/, BB/, Chica/, etc.")
    parser.add_argument("output_c", type=Path)
    parser.add_argument("output_h", type=Path)
    args = parser.parse_args()

    source = [
        "/* Generated from the original FNaF 3 PSX screamer TIM sequences. */\n",
        '#include "assets/jumpscare_assets.h"\n\n',
        "#include <stdint.h>\n\n",
    ]
    header = [
        "#pragma once\n\n",
        '#include "renderer/texture.h"\n\n',
        "typedef struct JumpscareSequence {\n",
        "    const TextureRle *const *frames;\n",
        "    uint32_t frame_count;\n",
        "    uint32_t ticks_per_frame;\n",
        "} JumpscareSequence;\n\n",
    ]

    for spec in SEQUENCES:
        parsed = [parse_tim(frame_path(args.input_root, spec, i))
                  for i in range(1, spec.count + 1)]
        canvas_width = max(frame[0] for frame in parsed)
        canvas_height = max(frame[1] for frame in parsed)
        frame_symbols = []
        for index, (width, height, colours, pixels) in enumerate(parsed, start=1):
            unique = f"k{spec.symbol[1:]}Frame{index}"
            padded = pad_frame(width, height, pixels, canvas_width, canvas_height)
            emit_texture(source, unique, canvas_width, canvas_height, colours, padded)
            frame_symbols.append(unique)

        array_symbol = f"{spec.symbol}Frames"
        source.append(f"static const TextureRle *const {array_symbol}[{spec.count}] = {{\n")
        source.append("\n".join(f"    &{name}," for name in frame_symbols))
        source.append("\n};\n")
        source.append(
            f"const JumpscareSequence {spec.symbol} = {{\n"
            f"    {array_symbol}, {spec.count}u, 4u,\n"
            f"}};\n\n"
        )
        header.append(f"extern const JumpscareSequence {spec.symbol};\n")
        print(f"{spec.symbol}: {spec.count} frames on {canvas_width}x{canvas_height}")

    args.output_c.parent.mkdir(parents=True, exist_ok=True)
    args.output_h.parent.mkdir(parents=True, exist_ok=True)
    args.output_c.write_text("".join(source), encoding="utf-8")
    args.output_h.write_text("".join(header), encoding="utf-8")


if __name__ == "__main__":
    main()
