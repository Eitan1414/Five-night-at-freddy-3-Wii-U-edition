#!/usr/bin/env python3
"""Generate PC-only compatibility textures for the legacy Wii U renderer.

The final renderer already uses high-resolution tiled PC assets. A few older
render paths still expect single TextureRle objects with the historical symbol
names. Generate those compatibility objects from the PC General Sprites bank
or, for the warning screen, from the exact text stored in the supplied PC MFA.
No PlayStation TIM input is used by this converter.
"""
from __future__ import annotations

import argparse
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

COLOURS = 48
TRANSPARENT_INDEX = 255

CAMERA_IDS = (106, 97, 104, 105, 109, 98, 100, 112, 115, 116)
SPRINGTRAP_CAMERA_IDS = (295, 146, 121, 122, 119, 117, 126, 127, 130, 140)

# The five full 1024x768 images used by the PC title frame's MFA object
# "Active 2". They are backgrounds, not Springtrap cut-outs.
TITLE_IDS = (862, 855, 864, 859, 861)

# Exact title/menu image-bank objects from the supplied PC MFA / General Sprites.
TITLE_UI_ASSETS = (
    ("gPcCompatTitleLogoTexture", 155, 240),
    ("gPcCompatTitleNewGameTexture", 594, 300),
    ("gPcCompatTitleLoadGameTexture", 301, 300),
    ("gPcCompatTitleNightmareTexture", 625, 300),
    ("gPcCompatTitleExtraTexture", 826, 220),
    ("gPcCompatTitleStarTexture", 840, 80),
    ("gPcCompatTitleCursorTexture", 833, 80),
    ("gPcCompatTitleCopyrightTexture", 849, 220),
    ("gPcCompatTitleResetTexture", 1021, 300),
    ("gPcCompatTitleVersionTexture", 289, 120),
)

# Transparent horizontal/glitch-line animation frames used by the PC title.
TITLE_LINE_IDS = (863, 865, 866, 867, 868, 869, 871)

# symbol, PC General Sprites id, maximum output dimension
PHANTOMS = (
    ("gPcCompatPhantomFoxyOfficeTexture", 302, 230),
    ("gPcCompatPhantomFreddyOfficeTexture", 653, 230),
    ("gPcCompatPhantomChicaOfficeTexture", 399, 230),
    ("gPcCompatPhantomMangleOfficeTexture", 202, 206),
    ("gPcCompatPhantomPuppetTexture", 320, 230),
    ("gPcCompatPhantomBBTexture", 70, 230),
)

# Exact full-camera composites recovered from fivenights3-94.mfa.  These are
# deliberately generated as compact single-RLE textures for real Wii U hardware
# instead of falling back to the unrelated office sprites when the tiled path
# is disabled for stability.
PHANTOM_CAMERA_COMPOSITES = (
    ("gPcCompatPhantomMangleCameraTexture", 38),
    ("gPcCompatPhantomChicaCameraTexture", 387),
    ("gPcCompatPhantomPuppetCameraTexture", 298),
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
                 source_label: str) -> None:
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
    source.append(f"/* {source_label}. */\n")
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


def make_pc_warning() -> Image.Image:
    """Recreate the PC Clickteam warning text without using WARNING.TIM.

    The supplied MFA stores this screen as text rather than an image object.
    Therefore the important source-faithful data here is the MFA wording; the
    Wii U rasterization uses Pillow's built-in bitmap font as a platform
    adaptation and stays intentionally independent of any PSX texture.
    """
    image = Image.new("RGBA", (360, 69), (0, 0, 0, 255))
    draw = ImageDraw.Draw(image)
    font = ImageFont.load_default()
    fg = (220, 220, 220, 255)
    draw.text((147, 8), "WARNING!", font=font, fill=fg)
    draw.text((16, 31), "This game contains flashing lights, loud noises,", font=font, fill=fg)
    draw.text((91, 45), "and lots of jumpscares!", font=font, fill=fg)
    return image


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("sprite_root", type=Path)
    parser.add_argument("output_c", type=Path)
    parser.add_argument("output_h", type=Path)
    args = parser.parse_args()
    root = args.sprite_root

    required = {203, *CAMERA_IDS, *SPRINGTRAP_CAMERA_IDS, *TITLE_IDS,
                *TITLE_LINE_IDS}
    required.update(sprite_id for _, sprite_id, _ in PHANTOMS)
    required.update(sprite_id for _, sprite_id in PHANTOM_CAMERA_COMPOSITES)
    required.update(sprite_id for _, sprite_id, _ in TITLE_UI_ASSETS)
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
                 fit(load(root, 203), max_size=(420, 161)),
                 "PC General Sprites ID 203")
    header.append("extern const TextureRle gPcCompatOfficeTexture;\n")

    emit_texture(source, "gPcCompatWarningTexture", make_pc_warning(),
                 "PC MFA warning text rasterized for Wii U")
    header.append("extern const TextureRle gPcCompatWarningTexture;\n")

    title_symbols = []
    for index, sprite_id in enumerate(TITLE_IDS, start=1):
        symbol = f"gPcCompatTitleSpringtrap{index}Texture"
        emit_texture(source, symbol,
                     fit(load(root, sprite_id), max_dimension=384),
                     f"PC full title background image ID {sprite_id} from fivenights3-94.mfa")
        header.append(f"extern const TextureRle {symbol};\n")
        title_symbols.append(symbol)

    for symbol, sprite_id, max_dimension in TITLE_UI_ASSETS:
        emit_texture(source, symbol,
                     fit(load(root, sprite_id), max_dimension=max_dimension),
                     f"PC title UI image ID {sprite_id} from fivenights3-94.mfa")
        header.append(f"extern const TextureRle {symbol};\n")

    title_line_symbols = []
    for index, sprite_id in enumerate(TITLE_LINE_IDS):
        symbol = f"gPcCompatTitleLines{index}Texture"
        emit_texture(source, symbol,
                     fit(load(root, sprite_id), max_size=(384, 288)),
                     f"PC title scan/glitch line image ID {sprite_id}")
        header.append(f"extern const TextureRle {symbol};\n")
        title_line_symbols.append(symbol)

    camera_symbols = []
    for index, sprite_id in enumerate(CAMERA_IDS, start=1):
        symbol = f"gPcCompatCamera{index:02d}Texture"
        emit_texture(source, symbol,
                     fit(load(root, sprite_id), max_size=(266, 148)),
                     f"PC General Sprites ID {sprite_id}")
        header.append(f"extern const TextureRle {symbol};\n")
        camera_symbols.append(symbol)

    springtrap_symbols = []
    for index, sprite_id in enumerate(SPRINGTRAP_CAMERA_IDS, start=1):
        symbol = f"gPcCompatSpringtrapCamera{index:02d}Texture"
        emit_texture(source, symbol,
                     fit(load(root, sprite_id), max_size=(266, 148)),
                     f"PC General Sprites ID {sprite_id}")
        header.append(f"extern const TextureRle {symbol};\n")
        springtrap_symbols.append(symbol)

    for symbol, sprite_id, max_dimension in PHANTOMS:
        emit_texture(source, symbol,
                     fit(load(root, sprite_id), max_dimension=max_dimension),
                     f"PC General Sprites ID {sprite_id}")
        header.append(f"extern const TextureRle {symbol};\n")

    for symbol, sprite_id in PHANTOM_CAMERA_COMPOSITES:
        emit_texture(source, symbol,
                     fit(load(root, sprite_id), max_size=(266, 148)),
                     f"PC MFA full-camera composite, General Sprites ID {sprite_id}")
        header.append(f"extern const TextureRle {symbol};\n")

    source.append("const TextureRle *const gPcCompatTitleLineTextures[7] = {\n")
    source.extend(f"    &{symbol},\n" for symbol in title_line_symbols)
    source.append("};\n\n")

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
        "\nextern const TextureRle *const gPcCompatTitleLineTextures[7];\n",
        "extern const TextureRle *const gPcCompatCameraTextures[10];\n",
        "extern const TextureRle *const gPcCompatSpringtrapCameraTextures[10];\n",
        "extern const JumpscareSequence gPcCompatPhantomPuppetAnimation;\n\n",
        "/* Historical renderer names now resolve to PC-derived objects. */\n",
        "#define gWarningTexture gPcCompatWarningTexture\n",
        "#define gMenuSpringtrapTexture gPcCompatTitleSpringtrap1Texture\n",
        "#define gMenuSpringtrapTexture2 gPcCompatTitleSpringtrap2Texture\n",
        "#define gMenuSpringtrapTexture3 gPcCompatTitleSpringtrap3Texture\n",
        "#define gMenuSpringtrapTexture4 gPcCompatTitleSpringtrap4Texture\n",
        "#define gMenuSpringtrapTexture5 gPcCompatTitleSpringtrap5Texture\n",
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
        "#define gPhantomChicaCameraTexture gPcCompatPhantomChicaCameraTexture\n",
        "#define gPhantomMangleCameraTexture gPcCompatPhantomMangleCameraTexture\n",
        "#define gPhantomPuppetCameraTexture gPcCompatPhantomPuppetCameraTexture\n",
        "#define gPhantomPuppetRealAnimation gPcCompatPhantomPuppetAnimation\n",
    ])

    args.output_c.parent.mkdir(parents=True, exist_ok=True)
    args.output_h.parent.mkdir(parents=True, exist_ok=True)
    args.output_c.write_text("".join(source), encoding="utf-8")
    args.output_h.write_text("".join(header), encoding="utf-8")
    print("PC compatibility visuals generated without PSX input")


if __name__ == "__main__":
    main()
