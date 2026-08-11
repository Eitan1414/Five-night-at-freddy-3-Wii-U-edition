#!/usr/bin/env python3
"""Generate the full original PC Springtrap jumpscare frame run.

General Sprites IDs 778..817 and 819..823 form the continuous Springtrap
approach/jumpscare animation. ID 818 is a separate floor/spotlight camera asset
and is intentionally skipped. The old converter sampled only 12 of these
frames; this keeps the original ordering while preserving roughly the same
48-tick total duration on Wii U.
"""
from __future__ import annotations

import argparse
from pathlib import Path
from PIL import Image

TRANSPARENT_INDEX = 255
COLOURS = 48
FRAME_IDS = tuple(range(778, 818)) + tuple(range(819, 824))
OUTPUT_SIZE = (320, 180)


def fmt(values, pattern: str, per_line: int) -> str:
    lines = []
    for start in range(0, len(values), per_line):
        chunk = values[start:start + per_line]
        lines.append("    " + ", ".join(pattern.format(value) for value in chunk) + ",")
    return "\n".join(lines)


def rgba_indexed(image: Image.Image) -> tuple[list[int], bytes]:
    rgba = image.convert("RGBA")
    alpha = rgba.getchannel("A")
    rgb = Image.new("RGB", rgba.size, (0, 0, 0))
    rgb.paste(rgba.convert("RGB"), mask=alpha)
    q = rgb.quantize(colors=COLOURS, method=Image.Quantize.MEDIANCUT,
                     dither=Image.Dither.NONE)
    raw = q.getpalette() or []
    palette = []
    for index in range(COLOURS):
        base = index * 3
        r = raw[base] if base < len(raw) else 0
        g = raw[base + 1] if base + 1 < len(raw) else 0
        b = raw[base + 2] if base + 2 < len(raw) else 0
        palette.append((r << 24) | (g << 16) | (b << 8) | 0xFF)
    palette.extend([0x000000FF] * (256 - len(palette)))
    pixels = bytearray(q.tobytes())
    for i, a in enumerate(alpha.tobytes()):
        if a < 32:
            pixels[i] = TRANSPARENT_INDEX
    return palette, bytes(pixels)


def encode_rows(width: int, height: int, pixels: bytes) -> tuple[list[int], bytes]:
    offsets = [0]
    runs = bytearray()
    for y in range(height):
        row = pixels[y * width:(y + 1) * width]
        x = 0
        while x < width:
            value = row[x]
            length = 1
            while x + length < width and row[x + length] == value and length < 255:
                length += 1
            runs.extend((length, value))
            x += length
        offsets.append(len(runs))
    if len(runs) > 0xFFFF:
        raise ValueError(f"frame RLE too large: {len(runs)} bytes")
    return offsets, bytes(runs)


def build_frames(root: Path) -> list[Image.Image]:
    originals = [Image.open(root / f"{sid}.png").convert("RGBA") for sid in FRAME_IDS]
    master_w = max(im.width for im in originals)
    master_h = max(im.height for im in originals)
    out_w, out_h = OUTPUT_SIZE
    scale = min(out_w / master_w, out_h / master_h)
    scaled_w = max(1, round(master_w * scale))
    scaled_h = max(1, round(master_h * scale))
    frames = []
    for image in originals:
        master = Image.new("RGBA", (master_w, master_h), (0, 0, 0, 0))
        master.alpha_composite(image, ((master_w - image.width) // 2,
                                       (master_h - image.height) // 2))
        master = master.resize((scaled_w, scaled_h), Image.Resampling.LANCZOS)
        canvas = Image.new("RGBA", OUTPUT_SIZE, (0, 0, 0, 0))
        canvas.alpha_composite(master, ((out_w - scaled_w) // 2,
                                        (out_h - scaled_h) // 2))
        frames.append(canvas)
    return frames


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("sprite_root", type=Path)
    parser.add_argument("output_c", type=Path)
    parser.add_argument("output_h", type=Path)
    args = parser.parse_args()

    frames = build_frames(args.sprite_root)
    source = [
        "/* Generated full PC Springtrap jumpscare. Do not edit. */\n",
        '#include "assets/pc_springtrap_sequence_extra.h"\n',
        '#include "renderer/texture.h"\n\n',
        "#include <stdint.h>\n\n",
    ]
    symbols = []
    for index, (sid, frame) in enumerate(zip(FRAME_IDS, frames), start=1):
        palette, pixels = rgba_indexed(frame)
        offsets, runs = encode_rows(frame.width, frame.height, pixels)
        prefix = f"kPcSpringtrapSmooth{index:02d}"
        symbol = f"{prefix}Texture"
        symbols.append(symbol)
        source.append(f"/* PC General Sprites ID {sid}. */\n")
        source.append(f"static const uint32_t {prefix}Palette[256] = {{\n")
        source.append(fmt(palette, "0x{:08X}u", 6) + "\n};\n")
        source.append(f"static const uint16_t {prefix}RowOffsets[{len(offsets)}] = {{\n")
        source.append(fmt(offsets, "{}u", 12) + "\n};\n")
        source.append(f"static const uint8_t {prefix}Runs[{len(runs)}] = {{\n")
        source.append(fmt(list(runs), "{}u", 24) + "\n};\n")
        source.append(
            f"static const TextureRle {symbol} = {{\n"
            f"    {frame.width}u, {frame.height}u, {TRANSPARENT_INDEX}u,\n"
            f"    {prefix}RowOffsets, {prefix}Runs, {prefix}Palette,\n"
            f"}};\n\n"
        )

    source.append(f"static const TextureRle *const kPcSpringtrapSmoothFrames[{len(symbols)}] = {{\n")
    source.extend(f"    &{symbol},\n" for symbol in symbols)
    source.append("};\n")
    source.append(
        "const JumpscareSequence gPcSpringtrapJumpscareSmooth = {\n"
        f"    kPcSpringtrapSmoothFrames, {len(symbols)}u, 1u,\n"
        "};\n"
    )

    header = (
        "#pragma once\n\n"
        '#include "assets/jumpscare_assets.h"\n\n'
        "extern const JumpscareSequence gPcSpringtrapJumpscareSmooth;\n"
    )
    args.output_c.parent.mkdir(parents=True, exist_ok=True)
    args.output_h.parent.mkdir(parents=True, exist_ok=True)
    args.output_c.write_text("".join(source), encoding="utf-8")
    args.output_h.write_text(header, encoding="utf-8")
    print(f"Springtrap full PC jumpscare: {len(symbols)} frames, IDs 778-817 + 819-823")


if __name__ == "__main__":
    main()
