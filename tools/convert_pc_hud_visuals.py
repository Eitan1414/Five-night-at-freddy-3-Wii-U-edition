#!/usr/bin/env python3
"""Generate the retail-PC office HUD sprites used by the Wii U renderer.

The image IDs below come from the supplied FNaF 3 PC General Sprites bank and
match the Clickteam counter/image sequences in fivenights3-94.mfa:

- 245: `night`
- 246: `AM`
- night/counter digits: 318,195,247,248,249,250,306 for 0..6
- small clock digits: 230..239 for 0..9

The Spriters Resource PNGs preserve Clickteam's black colour key as opaque
black.  The original Active/Counter objects key that black out at runtime, so
this converter restores the same transparency before generating TextureRle.
"""
from __future__ import annotations

import argparse
from pathlib import Path
from PIL import Image

COLOURS = 16
TRANSPARENT_INDEX = 255

NIGHT_LABEL_ID = 245
AM_LABEL_ID = 246
NIGHT_DIGIT_IDS = (318, 195, 247, 248, 249, 250, 306)
CLOCK_DIGIT_IDS = tuple(range(230, 240))


def fmt(values, pattern: str, per_line: int) -> str:
    lines: list[str] = []
    for start in range(0, len(values), per_line):
        lines.append("    " + ", ".join(
            pattern.format(value) for value in values[start:start + per_line]
        ) + ",")
    return "\n".join(lines)


def black_key(image: Image.Image) -> Image.Image:
    rgba = image.convert("RGBA")
    pixels = []
    for red, green, blue, alpha in rgba.getdata():
        if red <= 8 and green <= 8 and blue <= 8:
            pixels.append((0, 0, 0, 0))
        else:
            pixels.append((red, green, blue, alpha))
    rgba.putdata(pixels)
    return rgba


def indexed_rgba(image: Image.Image) -> tuple[list[int], bytes]:
    rgba = black_key(image)
    alpha = rgba.getchannel("A")
    rgb = Image.new("RGB", rgba.size, (0, 0, 0))
    rgb.paste(rgba.convert("RGB"), mask=alpha)
    quantized = rgb.quantize(colors=COLOURS,
                             method=Image.Quantize.MEDIANCUT,
                             dither=Image.Dither.NONE)
    raw = quantized.getpalette() or []
    palette: list[int] = []
    for index in range(COLOURS):
        base = index * 3
        red = raw[base] if base < len(raw) else 0
        green = raw[base + 1] if base + 1 < len(raw) else 0
        blue = raw[base + 2] if base + 2 < len(raw) else 0
        palette.append((red << 24) | (green << 16) | (blue << 8) | 0xFF)
    palette.extend([0x000000FF] * (256 - len(palette)))

    pixels = bytearray(quantized.tobytes())
    for index, value in enumerate(alpha.tobytes()):
        if value < 32:
            pixels[index] = TRANSPARENT_INDEX
    return palette, bytes(pixels)


def encode_rows(width: int, height: int,
                pixels: bytes) -> tuple[list[int], bytes]:
    offsets = [0]
    encoded = bytearray()
    for y in range(height):
        row = pixels[y * width:(y + 1) * width]
        x = 0
        while x < width:
            value = row[x]
            length = 1
            while (x + length < width and
                   row[x + length] == value and length < 255):
                length += 1
            encoded.extend((length, value))
            x += length
        offsets.append(len(encoded))
    if len(encoded) > 0xFFFF:
        raise ValueError(f"encoded texture is {len(encoded)} bytes")
    return offsets, bytes(encoded)


def emit_texture(source: list[str], header: list[str], root: Path,
                 symbol: str, sprite_id: int) -> None:
    path = root / f"{sprite_id}.png"
    if not path.is_file():
        raise FileNotFoundError(path)
    image = Image.open(path).convert("RGBA")
    palette, pixels = indexed_rgba(image)
    offsets, runs = encode_rows(image.width, image.height, pixels)
    unique = "k" + symbol[1:]

    source.append(f"/* PC General Sprites ID {sprite_id}. */\n")
    source.append(f"static const uint32_t {unique}Palette[256] = {{\n")
    source.append(fmt(palette, "0x{:08X}u", 6) + "\n};\n")
    source.append(f"static const uint16_t {unique}RowOffsets[{len(offsets)}] = {{\n")
    source.append(fmt(offsets, "{}u", 12) + "\n};\n")
    source.append(f"static const uint8_t {unique}Runs[{len(runs)}] = {{\n")
    source.append(fmt(list(runs), "{}u", 24) + "\n};\n")
    source.append(
        f"const TextureRle {symbol} = {{\n"
        f"    {image.width}u, {image.height}u, {TRANSPARENT_INDEX}u,\n"
        f"    {unique}RowOffsets, {unique}Runs, {unique}Palette,\n"
        f"}};\n\n"
    )
    header.append(f"extern const TextureRle {symbol};\n")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("sprite_root", type=Path)
    parser.add_argument("output_c", type=Path)
    parser.add_argument("output_h", type=Path)
    args = parser.parse_args()
    root = args.sprite_root

    required = {NIGHT_LABEL_ID, AM_LABEL_ID,
                *NIGHT_DIGIT_IDS, *CLOCK_DIGIT_IDS}
    missing = sorted(sprite_id for sprite_id in required
                     if not (root / f"{sprite_id}.png").is_file())
    if missing:
        raise SystemExit(f"Missing PC office HUD General Sprites IDs: {missing}")

    source = [
        "/* Generated retail-PC office HUD visuals. Do not edit. */\n",
        '#include "assets/pc_hud_visuals.h"\n\n',
        "#include <stdint.h>\n\n",
    ]
    header = [
        "#pragma once\n\n",
        '#include "renderer/texture.h"\n\n',
    ]

    emit_texture(source, header, root,
                 "gPcHudNightLabelTexture", NIGHT_LABEL_ID)
    emit_texture(source, header, root,
                 "gPcHudAmLabelTexture", AM_LABEL_ID)

    night_symbols: list[str] = []
    for value, sprite_id in enumerate(NIGHT_DIGIT_IDS):
        symbol = f"gPcHudNightDigit{value}Texture"
        emit_texture(source, header, root, symbol, sprite_id)
        night_symbols.append(symbol)

    clock_symbols: list[str] = []
    for value, sprite_id in enumerate(CLOCK_DIGIT_IDS):
        symbol = f"gPcHudClockDigit{value}Texture"
        emit_texture(source, header, root, symbol, sprite_id)
        clock_symbols.append(symbol)

    source.append("const TextureRle *const gPcHudNightDigits[7] = {\n")
    source.extend(f"    &{symbol},\n" for symbol in night_symbols)
    source.append("};\n\n")
    source.append("const TextureRle *const gPcHudClockDigits[10] = {\n")
    source.extend(f"    &{symbol},\n" for symbol in clock_symbols)
    source.append("};\n")

    header.extend([
        "\nextern const TextureRle *const gPcHudNightDigits[7];\n",
        "extern const TextureRle *const gPcHudClockDigits[10];\n",
    ])

    args.output_c.parent.mkdir(parents=True, exist_ok=True)
    args.output_h.parent.mkdir(parents=True, exist_ok=True)
    args.output_c.write_text("".join(source), encoding="utf-8")
    args.output_h.write_text("".join(header), encoding="utf-8")
    print("Retail-PC office HUD assets generated")


if __name__ == "__main__":
    main()
