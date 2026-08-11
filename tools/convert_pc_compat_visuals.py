#!/usr/bin/env python3
"""Generate PC-only compatibility textures for the legacy Wii U renderer.

The final renderer already uses high-resolution tiled PC assets.  A few older
render paths still expect single TextureRle objects with the historical symbol
names.  Generate those compatibility objects from the *same PC General
Sprites* bank so the build no longer needs any PlayStation TIM fallback.
"""
from __future__ import annotations

import argparse
from pathlib import Path
from PIL import Image

COLOURS = 48
TRANSPARENT_INDEX = 255

CAMERA_IDS = (106, 97, 104, 105, 109, 98, 100, 112, 115, 116)
SPRINGTRAP_CAMERA_IDS = (295, 146, 121, 122, 119, 117, 126, 127, 130, 140)

# symbol, PC General Sprites id, maximum output dimension
PHANTOMS = (
    ("gPcCompatPhantomFoxyOfficeTexture", 302, 230),
    ("gPcCompatPhantomFreddyOfficeTexture", 653, 230),
    ("gPcCompatPhantomChicaOfficeTexture", 399, 230),
    ("gPcCompatPhantomMangleOfficeTexture", 202, 206),
    ("gPcCompatPhantomPuppetTexture", 320, 230),
    ("gPcCompatPhantomBBTexture", 70, 230),
)


def fmt(values, pattern: str, per_line: int) -> str:
    lines = []
    for start in range(0, len(values), per_line):
        lines.append("    " + ", ".join(
            pattern.format(value) for value in values[start:start + per_line]
        ) + ",")
    return "\n".join(lines)


def indexed_rgba(image: Image.Image) -> tuple[list[int], bytes]:
    rgba = image.convert("RGBA")
    alpha = rgba.getchannel("A")
    rgb = Image.new("RGB", rgba.size, (0, 0, 0))
    rgb.paste(rgba.convert("RGB"), mask=alpha)
    quantized = rgb.quantize(colors=COLOURS, method=Image.Quantize.MEDIANCUT,
                             dither=Image.Dither.NONE)
    raw = quantized.getpalette() or []
    palette = []
    for index in range(COLOURS):
        base = index * 3
        r = raw[base] if base < len(raw) else 0
        g = raw[base + 1] if base + 1 < len(raw) else 0
        b = raw[base + 2] if base + 2 < len(raw) else 0
        palette.append((r << 24) | (g << 16) | (b << 8) | 0xFF)
    palette.extend([0x000000FF] * (256 - len(palette)))
    pixels = bytearray(quantized.tobytes())
    for index, value in enumerate(alpha.tobytes()):
        if value < 32:
            pixels[index] = TRANSPARENT_INDEX
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
        raise ValueError(f"encoded texture is {len(runs)} bytes")
    return offsets, bytes(runs)


def fit(image: Image.Image, max_size: tuple[int, int] | None = None,
        max_dimension: int | None = None) -> Image.Image:
    if max_size is not None:
        return image.convert("RGBA").resize(max_size, Image.Resampling.LANCZOS)
    assert max_dimension is not None
    rgba = image.convert("RGBA")
    scale = min(1.0, max_dimension / max(rgba.width, rgba.height))
    if scale < 1.0:
        rgba = rgba.resize((max(1, round(rgba.width * scale)),
                            max(1, round(rgba.height * scale))),
                           Image.Resampling.LANCZOS)
    return rgba


def emit_texture(source: list[str], symbol: str, image: Image.Image,
                 sprite_id: int) -> None:
    current = image
    while True:
        palette, pixels = indexed_rgba(current)
        try:
            offsets, runs = encode_rows(current.width, current.height, pixels)
            break
        except ValueError:
            if max(current.width, current.height) <= 96:
                raise
            current = current.resize((max(1, round(current.width * 0.82)),
                                      max(1, round(current.height * 0.82))),
                                     Image.Resampling.LANCZOS)

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
        f"    {current.width}u, {current.height}u, {TRANSPARENT_INDEX}u,\n"
        f"    {unique}RowOffsets, {unique}Runs, {unique}Palette,\n"
        f"}};\n\n"
    )


def load(root: Path, sprite_id: int) -> Image.Image:
    path = root / f"{sprite_id}.png"
    if not path.is_file():
        raise FileNotFoundError(path)
    return Image.open(path)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("sprite_root", type=Path)
    parser.add_argument("output_c", type=Path)
    parser.add_argument("output_h", type=Path)
    args = parser.parse_args()
    root = args.sprite_root

    required = {203, *CAMERA_IDS, *SPRINGTRAP_CAMERA_IDS}
    required.update(sprite_id for _, sprite_id, _ in PHANTOMS)
    missing = sorted(sprite_id for sprite_id in required
                     if not (root / f"{sprite_id}.png").is_file())
    if missing:
        raise SystemExit(f"Missing PC General Sprites IDs: {missing}")

    source = [
        "/* Generated PC-only legacy compatibility visuals. Do not edit. */\n",
        '#include "assets/pc_compat_visuals.h"\n\n',
        "#include <stdint.h>\n\n",
    ]
    header = [
        "#pragma once\n\n",
        '#include "assets/jumpscare_assets.h"\n',
        '#include "renderer/texture.h"\n\n',
    ]

    emit_texture(source, "gPcCompatOfficeTexture",
                 fit(load(root, 203), max_size=(420, 161)), 203)
    header.append("extern const TextureRle gPcCompatOfficeTexture;\n")

    camera_symbols = []
    for index, sprite_id in enumerate(CAMERA_IDS, start=1):
        symbol = f"gPcCompatCamera{index:02d}Texture"
        emit_texture(source, symbol,
                     fit(load(root, sprite_id), max_size=(266, 148)), sprite_id)
        header.append(f"extern const TextureRle {symbol};\n")
        camera_symbols.append(symbol)

    springtrap_symbols = []
    for index, sprite_id in enumerate(SPRINGTRAP_CAMERA_IDS, start=1):
        symbol = f"gPcCompatSpringtrapCamera{index:02d}Texture"
        emit_texture(source, symbol,
                     fit(load(root, sprite_id), max_size=(266, 148)), sprite_id)
        header.append(f"extern const TextureRle {symbol};\n")
        springtrap_symbols.append(symbol)

    for symbol, sprite_id, max_dimension in PHANTOMS:
        emit_texture(source, symbol,
                     fit(load(root, sprite_id), max_dimension=max_dimension), sprite_id)
        header.append(f"extern const TextureRle {symbol};\n")

    source.append("const TextureRle *const gPcCompatCameraTextures[10] = {\n")
    source.extend(f"    &{symbol},\n" for symbol in camera_symbols)
    source.append("};\n\n")
    source.append("const TextureRle *const gPcCompatSpringtrapCameraTextures[10] = {\n")
    source.extend(f"    &{symbol},\n" for symbol in springtrap_symbols)
    source.append("};\n\n")
    source.append(
        "static const TextureRle *const kPcCompatPuppetFrames[1] = {\n"
        "    &gPcCompatPhantomPuppetTexture,\n"
        "};\n"
        "const JumpscareSequence gPcCompatPhantomPuppetAnimation = {\n"
        "    kPcCompatPuppetFrames, 1u, 1u,\n"
        "};\n"
    )

    header.extend([
        "\nextern const TextureRle *const gPcCompatCameraTextures[10];\n",
        "extern const TextureRle *const gPcCompatSpringtrapCameraTextures[10];\n",
        "extern const JumpscareSequence gPcCompatPhantomPuppetAnimation;\n\n",
        "/* Historical renderer names now resolve to PC-derived objects. */\n",
        "#define gCamera01Texture gPcCompatCamera01Texture\n",
        "#define gCamera02Texture gPcCompatCamera02Texture\n",
        "#define gCamera03Texture gPcCompatCamera03Texture\n",
        "#define gCamera04Texture gPcCompatCamera04Texture\n",
        "#define gCamera05Texture gPcCompatCamera05Texture\n",
        "#define gCamera06Texture gPcCompatCamera06Texture\n",
        "#define gCamera07Texture gPcCompatCamera07Texture\n",
        "#define gCamera08Texture gPcCompatCamera08Texture\n",
        "#define gCamera09Texture gPcCompatCamera09Texture\n",
        "#define gCamera10Texture gPcCompatCamera10Texture\n",
        "#define gOfficeTexture gPcCompatOfficeTexture\n",
        "#define gOriginalCameraSpringtrapTextures gPcCompatSpringtrapCameraTextures\n",
        "#define gPhantomFoxyOfficeTexture gPcCompatPhantomFoxyOfficeTexture\n",
        "#define gPhantomFreddyOfficeTexture gPcCompatPhantomFreddyOfficeTexture\n",
        "#define gPhantomChicaOfficeTexture gPcCompatPhantomChicaOfficeTexture\n",
        "#define gPhantomMangleOfficeTexture gPcCompatPhantomMangleOfficeTexture\n",
        "#define gPhantomPuppetOfficeTexture gPcCompatPhantomPuppetTexture\n",
        "#define gPhantomBBCameraTexture gPcCompatPhantomBBTexture\n",
        "#define gPhantomBBOfficeTexture gPcCompatPhantomBBTexture\n",
        "#define gPhantomChicaCameraTexture gPcCompatPhantomChicaOfficeTexture\n",
        "#define gPhantomMangleCameraTexture gPcCompatPhantomMangleOfficeTexture\n",
        "#define gPhantomPuppetCameraTexture gPcCompatPhantomPuppetTexture\n",
        "#define gPhantomPuppetRealAnimation gPcCompatPhantomPuppetAnimation\n",
    ])

    args.output_c.parent.mkdir(parents=True, exist_ok=True)
    args.output_h.parent.mkdir(parents=True, exist_ok=True)
    args.output_c.write_text("".join(source), encoding="utf-8")
    args.output_h.write_text("".join(header), encoding="utf-8")
    print("PC compatibility visuals generated without PSX input")


if __name__ == "__main__":
    main()
