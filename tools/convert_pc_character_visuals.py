#!/usr/bin/env python3
"""Generate PC Springtrap/Phantom gameplay visuals from General Sprites.

This complements convert_pc_general_sprites.py: backgrounds stay in the core
converter while this file migrates dynamic character states that previously
came from the PSX reference or from hand-drawn Wii U placeholders.
"""
from __future__ import annotations

import argparse
from pathlib import Path
from PIL import Image


COLOURS = 64
TRANSPARENT_INDEX = 255


def fmt(values, pattern: str, per_line: int) -> str:
    lines = []
    for start in range(0, len(values), per_line):
        chunk = values[start:start + per_line]
        lines.append("    " + ", ".join(pattern.format(value) for value in chunk) + ",")
    return "\n".join(lines)


def quantize_opaque(path: Path, size: tuple[int, int]) -> Image.Image:
    image = Image.open(path).convert("RGB").resize(size, Image.Resampling.LANCZOS)
    return image.quantize(colors=80, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)


def opaque_palette(image: Image.Image) -> list[int]:
    raw = image.getpalette() or []
    result = []
    for index in range(80):
        base = index * 3
        red = raw[base] if base < len(raw) else 0
        green = raw[base + 1] if base + 1 < len(raw) else 0
        blue = raw[base + 2] if base + 2 < len(raw) else 0
        result.append((red << 24) | (green << 16) | (blue << 8) | 0xFF)
    return result


def encode_tile(image: Image.Image, x0: int, x1: int) -> tuple[list[int], bytes]:
    pixels = image.load()
    offsets = [0]
    runs = bytearray()
    for y in range(image.height):
        x = x0
        while x < x1:
            value = int(pixels[x, y])
            length = 1
            while x + length < x1 and int(pixels[x + length, y]) == value and length < 255:
                length += 1
            runs.extend((length, value))
            x += length
        offsets.append(len(runs))
    if len(runs) > 0xFFFF:
        raise ValueError(f"tile {x0}:{x1} is {len(runs)} bytes")
    return offsets, bytes(runs)


def emit_asset(source: list[str], root: Path, name: str, sprite_id: int,
               size: tuple[int, int], tile_width: int) -> None:
    path = root / f"{sprite_id}.png"
    if not path.is_file():
        raise FileNotFoundError(path)
    image = quantize_opaque(path, size)
    palette = opaque_palette(image)
    prefix = f"kPc{name}"
    source.append(f"/* PC General Sprites ID {sprite_id}; full-frame {size[0]}x{size[1]}. */\n")
    source.append(f"static const uint32_t {prefix}Palette[{len(palette)}] = {{\n")
    source.append(fmt(palette, "0x{:08X}u", 6) + "\n};\n")
    tile_symbols = []
    x = 0
    tile_index = 0
    while x < size[0]:
        x1 = min(x + tile_width, size[0])
        offsets, runs = encode_tile(image, x, x1)
        symbol = f"{prefix}Tile{tile_index:02d}"
        source.append(f"static const uint16_t {symbol}RowOffsets[{len(offsets)}] = {{\n")
        source.append(fmt(offsets, "{}u", 12) + "\n};\n")
        source.append(f"static const uint8_t {symbol}Runs[{len(runs)}] = {{\n")
        source.append(fmt(list(runs), "{}u", 24) + "\n};\n")
        source.append(f"static const TextureRle {symbol} = {{\n    {x1-x}u, {size[1]}u, 255u,\n    {symbol}RowOffsets, {symbol}Runs, {prefix}Palette,\n}};\n")
        tile_symbols.append(symbol)
        x = x1
        tile_index += 1
    source.append(f"static const TextureRle *const {prefix}Tiles[{len(tile_symbols)}] = {{\n")
    source.extend(f"    &{symbol},\n" for symbol in tile_symbols)
    source.append("};\n")
    source.append(f"const PcTiledTexture gPc{name}Texture = {{\n    {size[0]}u, {size[1]}u, {len(tile_symbols)}u, {prefix}Tiles,\n}};\n\n")


# Exact full-frame PC camera composites (camera index is zero-based).
SPRINGTRAP_CAMERAS = (
    (0, 295), (1, 146), (2, 121), (3, 122), (4, 119),
    (5, 117), (6, 126), (7, 127), (8, 130), (9, 140),
)

FULL_CAMERA_ASSETS = (
    ("PhantomMangleCamera", 38),
    ("PhantomChicaCamera", 387),
)

# Transparent PC sprites used in the office / monitor overlays.
SPRITES = (
    ("SpringtrapOfficeWindow", 207, 280),
    ("SpringtrapOfficeLeft", 214, 280),
    ("PhantomFoxyOffice", 170, 300),
    ("PhantomBB", 338, 300),
    ("PhantomPuppet", 320, 300),
    ("PhantomChicaOffice", 399, 300),
)

# The numbered General Sprites frames preserve the original PC zoom/order.
SEQUENCES = (
    ("PhantomFreddyWalk", (190, 191, 192, 193, 194, 196, 197, 198), 4),
    ("PhantomFoxyJumpscare", (174, 175, 176, 177, 178, 179, 180, 181), 4),
    ("PhantomBBJumpscare", (341, 342, 343, 344, 345, 347, 349), 4),
    ("PhantomChicaJumpscare", (461, 462, 463, 464, 465, 466, 467, 468), 4),
    ("PhantomFreddyJumpscare", (475, 476, 477, 478, 479, 480, 482, 484, 488, 492, 499), 4),
    ("SpringtrapJumpscare", (778, 782, 786, 790, 794, 798, 802, 806, 810, 814, 819, 823), 4),
)


def rgba_indexed(image: Image.Image, colours: int = COLOURS) -> tuple[list[int], bytes]:
    rgba = image.convert("RGBA")
    alpha = rgba.getchannel("A")
    rgb = Image.new("RGB", rgba.size, (0, 0, 0))
    rgb.paste(rgba.convert("RGB"), mask=alpha)
    quantized = rgb.quantize(colors=colours,
                             method=Image.Quantize.MEDIANCUT,
                             dither=Image.Dither.NONE)
    raw_palette = quantized.getpalette() or []
    palette = []
    for index in range(colours):
        base = index * 3
        red = raw_palette[base] if base < len(raw_palette) else 0
        green = raw_palette[base + 1] if base + 1 < len(raw_palette) else 0
        blue = raw_palette[base + 2] if base + 2 < len(raw_palette) else 0
        palette.append((red << 24) | (green << 16) | (blue << 8) | 0xFF)
    palette.extend([0x000000FF] * (256 - len(palette)))

    q = bytearray(quantized.tobytes())
    a = alpha.tobytes()
    for index, value in enumerate(a):
        if value < 32:
            q[index] = TRANSPARENT_INDEX
    return palette, bytes(q)


def encode_rows(width: int, height: int, pixels: bytes) -> tuple[list[int], bytes]:
    offsets = [0]
    encoded = bytearray()
    for y in range(height):
        row = pixels[y * width:(y + 1) * width]
        x = 0
        while x < width:
            value = row[x]
            length = 1
            while x + length < width and row[x + length] == value and length < 255:
                length += 1
            encoded.extend((length, value))
            x += length
        offsets.append(len(encoded))
    if len(encoded) > 0xFFFF:
        raise ValueError(f"encoded texture is {len(encoded)} bytes (> uint16)")
    return offsets, bytes(encoded)


def emit_texture(source: list[str], symbol: str, image: Image.Image,
                 sprite_id: int | None = None, exported: bool = False) -> None:
    palette, pixels = rgba_indexed(image)
    offsets, runs = encode_rows(image.width, image.height, pixels)
    unique = f"k{symbol[1:]}"
    if sprite_id is not None:
        source.append(f"/* PC General Sprites ID {sprite_id}. */\n")
    source.append(f"static const uint32_t {unique}Palette[256] = {{\n")
    source.append(fmt(palette, "0x{:08X}u", 6) + "\n};\n")
    source.append(f"static const uint16_t {unique}RowOffsets[{len(offsets)}] = {{\n")
    source.append(fmt(offsets, "{}u", 12) + "\n};\n")
    source.append(f"static const uint8_t {unique}Runs[{len(runs)}] = {{\n")
    source.append(fmt(list(runs), "{}u", 24) + "\n};\n")
    storage = "" if exported else "static "
    source.append(
        f"{storage}const TextureRle {symbol} = {{\n"
        f"    {image.width}u, {image.height}u, {TRANSPARENT_INDEX}u,\n"
        f"    {unique}RowOffsets, {unique}Runs, {unique}Palette,\n"
        f"}};\n\n"
    )


def load_scaled_sprite(root: Path, sprite_id: int, max_dimension: int) -> Image.Image:
    image = Image.open(root / f"{sprite_id}.png").convert("RGBA")
    scale = min(1.0, max_dimension / max(image.width, image.height))
    if scale < 1.0:
        image = image.resize((max(1, round(image.width * scale)),
                              max(1, round(image.height * scale))),
                             Image.Resampling.LANCZOS)
    return image


def build_sequence_frames(root: Path, ids: tuple[int, ...],
                          output_size: tuple[int, int] = (320, 180)) -> list[Image.Image]:
    originals = [Image.open(root / f"{sprite_id}.png").convert("RGBA") for sprite_id in ids]
    master_w = max(image.width for image in originals)
    master_h = max(image.height for image in originals)
    out_w, out_h = output_size
    scale = min(out_w / master_w, out_h / master_h)
    scaled_master_w = max(1, round(master_w * scale))
    scaled_master_h = max(1, round(master_h * scale))

    frames = []
    for image in originals:
        master = Image.new("RGBA", (master_w, master_h), (0, 0, 0, 0))
        master.alpha_composite(image, ((master_w - image.width) // 2,
                                       (master_h - image.height) // 2))
        master = master.resize((scaled_master_w, scaled_master_h),
                               Image.Resampling.LANCZOS)
        canvas = Image.new("RGBA", output_size, (0, 0, 0, 0))
        canvas.alpha_composite(master, ((out_w - scaled_master_w) // 2,
                                        (out_h - scaled_master_h) // 2))
        frames.append(canvas)
    return frames


def emit_sequence(source: list[str], header: list[str], root: Path,
                  name: str, ids: tuple[int, ...], ticks_per_frame: int) -> None:
    frames = build_sequence_frames(root, ids)
    frame_symbols = []
    for index, (sprite_id, frame) in enumerate(zip(ids, frames), start=1):
        symbol = f"kPc{name}Frame{index:02d}"
        emit_texture(source, symbol, frame, sprite_id)
        frame_symbols.append(symbol)
    array = f"kPc{name}Frames"
    source.append(f"static const TextureRle *const {array}[{len(frame_symbols)}] = {{\n")
    source.extend(f"    &{symbol},\n" for symbol in frame_symbols)
    source.append("};\n")
    source.append(
        f"const JumpscareSequence gPc{name} = {{\n"
        f"    {array}, {len(frame_symbols)}u, {ticks_per_frame}u,\n"
        f"}};\n\n"
    )
    header.append(f"extern const JumpscareSequence gPc{name};\n")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("sprite_root", type=Path)
    parser.add_argument("output_c", type=Path)
    parser.add_argument("output_h", type=Path)
    args = parser.parse_args()
    root = args.sprite_root

    source = [
        "/* Generated PC Springtrap/Phantom visuals. Do not edit by hand. */\n",
        '#include "assets/pc_character_visuals.h"\n',
        '#include "assets/pc_core_visuals.h"\n\n',
        "#include <stddef.h>\n",
        "#include <stdint.h>\n\n",
    ]
    header = [
        "#pragma once\n\n",
        '#include "assets/jumpscare_assets.h"\n',
        '#include "assets/pc_core_visuals.h"\n',
        '#include "renderer/texture.h"\n\n',
    ]

    for camera_index, sprite_id in SPRINGTRAP_CAMERAS:
        emit_asset(source, root, f"SpringtrapCamera{camera_index + 1:02d}",
                   sprite_id, (532, 295), 96)
    for name, sprite_id in FULL_CAMERA_ASSETS:
        emit_asset(source, root, name, sprite_id, (532, 295), 96)

    for name, sprite_id, max_dimension in SPRITES:
        symbol = f"gPc{name}Texture"
        image = load_scaled_sprite(root, sprite_id, max_dimension)
        # Detailed transparent sprites can exceed uint16 row offsets at the
        # first size. Reduce conservatively until the row-RLE stream fits.
        while True:
            try:
                emit_texture(source, symbol, image, sprite_id, exported=True)
                break
            except ValueError:
                if max(image.width, image.height) <= 160:
                    raise
                image = image.resize((max(1, round(image.width * 0.82)),
                                      max(1, round(image.height * 0.82))),
                                     Image.Resampling.LANCZOS)
        header.append(f"extern const TextureRle {symbol};\n")

    for name, ids, ticks in SEQUENCES:
        emit_sequence(source, header, root, name, ids, ticks)

    source.append("const PcTiledTexture *pc_springtrap_camera_texture(int camera_index)\n{\n")
    source.append("    switch (camera_index) {\n")
    for camera_index, _sprite_id in SPRINGTRAP_CAMERAS:
        source.append(
            f"        case {camera_index}: return &gPcSpringtrapCamera{camera_index + 1:02d}Texture;\n"
        )
    source.append("        default: return NULL;\n    }\n}\n")

    header.extend([
        "\nextern const PcTiledTexture gPcPhantomMangleCameraTexture;\n",
        "extern const PcTiledTexture gPcPhantomChicaCameraTexture;\n",
        "const PcTiledTexture *pc_springtrap_camera_texture(int camera_index);\n",
    ])

    args.output_c.parent.mkdir(parents=True, exist_ok=True)
    args.output_h.parent.mkdir(parents=True, exist_ok=True)
    args.output_c.write_text("".join(source), encoding="utf-8")
    args.output_h.write_text("".join(header), encoding="utf-8")

    print("PC character visuals generated: Springtrap cams, office states, Phantoms and jumpscares")


if __name__ == "__main__":
    main()
