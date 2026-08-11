#!/usr/bin/env python3
"""Generate authentic PC ending and minigame visuals for the Wii U port.

Input 1 is the numbered General Sprites DataDump3 directory.
Input 2 contains the seven original PC minigame sprite sheets downloaded from
The Spriters Resource. The generated C stays out of git and is rebuilt in CI.
"""
from __future__ import annotations

import argparse
from collections import deque
from pathlib import Path
from PIL import Image

TRANSPARENT_INDEX = 255
TEAL = (55, 177, 182)
BLACK = (0, 0, 0)

# Exact PC full-screen images from General Sprites.
# Bad Ending, Good Ending / "the end", and the post-Night-6 newspaper.
ENDING_ASSETS = (
    ("BadEnding", 346, (532, 399), 64),
    ("GoodEnding", 172, (532, 399), 64),
    ("NewspaperEnding", 123, (532, 399), 64),
)

# symbol, sheet filename, crop box (left, top, right, bottom).
# Boxes intentionally include a little black/teal padding; border flood-fill
# removes that sheet background while preserving internal black pixel detail.
SPRITE_CROPS = (
    # Follow Me: four animatronics, ghost child, guide, murderer, spring suit.
    ("FollowFreddy", "follow_me.png", (1, 1, 201, 201)),
    ("FollowBonnie", "follow_me.png", (1, 310, 201, 510)),
    ("FollowChica", "follow_me.png", (1, 708, 201, 908)),
    ("FollowFoxy", "follow_me.png", (1, 1077, 201, 1277)),
    ("FollowShadowFreddy", "follow_me.png", (1, 1379, 201, 1579)),
    ("FollowPurpleGuy", "follow_me.png", (1, 1678, 201, 1898)),
    ("FollowGhost", "follow_me.png", (1, 2618, 118, 2863)),
    ("FollowSpringBonnie", "follow_me.png", (297, 2128, 406, 2406)),

    # Secret minigame player/NPC sprites from the actual PC sheets.
    ("SecretBBPlayer", "bb_air_adventure.png", (0, 42, 110, 158)),
    ("SecretChicaPlayer", "chicas_party.png", (0, 56, 113, 210)),
    ("SecretChicaChildBlue", "chicas_party.png", (309, 133, 402, 210)),
    ("SecretChicaChildGreen", "chicas_party.png", (402, 133, 496, 210)),
    ("SecretManglePlayer", "mangles_quest.png", (0, 0, 115, 122)),
    ("SecretStagePlayer", "stage_01.png", (0, 55, 145, 222)),
    ("SecretStageChild", "stage_01.png", (0, 410, 95, 504)),
    ("SecretShadowBonniePlayer", "shadow_bonnie.png", (0, 0, 132, 189)),
    ("SecretHappiestPlayer", "happiest_day.png", (0, 56, 113, 176)),
    ("SecretHappiestChild1", "happiest_day.png", (0, 528, 95, 706)),
    ("SecretHappiestChild2", "happiest_day.png", (95, 528, 190, 706)),
    ("SecretHappiestChild3", "happiest_day.png", (190, 528, 285, 706)),
    ("SecretHappiestChild4", "happiest_day.png", (285, 528, 380, 706)),
)


def fmt(values, pattern: str, per_line: int) -> str:
    lines = []
    for start in range(0, len(values), per_line):
        chunk = values[start:start + per_line]
        lines.append("    " + ", ".join(pattern.format(value) for value in chunk) + ",")
    return "\n".join(lines)


def quantize_opaque(path: Path, size: tuple[int, int]) -> Image.Image:
    image = Image.open(path).convert("RGB").resize(size, Image.Resampling.LANCZOS)
    return image.quantize(colors=80, method=Image.Quantize.MEDIANCUT,
                          dither=Image.Dither.NONE)


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


def emit_tiled(source: list[str], root: Path, name: str, sprite_id: int,
               size: tuple[int, int], tile_width: int) -> None:
    path = root / f"{sprite_id}.png"
    if not path.is_file():
        raise FileNotFoundError(path)
    image = quantize_opaque(path, size)
    palette = opaque_palette(image)
    prefix = f"kPc{name}"
    source.append(f"/* PC General Sprites ID {sprite_id}; {size[0]}x{size[1]}. */\n")
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
        source.append(
            f"static const TextureRle {symbol} = {{\n"
            f"    {x1-x}u, {size[1]}u, 255u,\n"
            f"    {symbol}RowOffsets, {symbol}Runs, {prefix}Palette,\n"
            f"}};\n"
        )
        tile_symbols.append(symbol)
        x = x1
        tile_index += 1
    source.append(f"static const TextureRle *const {prefix}Tiles[{len(tile_symbols)}] = {{\n")
    source.extend(f"    &{symbol},\n" for symbol in tile_symbols)
    source.append("};\n")
    source.append(
        f"const PcTiledTexture gPc{name}Texture = {{\n"
        f"    {size[0]}u, {size[1]}u, {len(tile_symbols)}u, {prefix}Tiles,\n"
        f"}};\n\n"
    )


def clear_border_background(image: Image.Image) -> Image.Image:
    """Make border-connected teal/black sheet background transparent."""
    rgba = image.convert("RGBA")
    pixels = rgba.load()
    width, height = rgba.size
    seen = bytearray(width * height)
    queue: deque[tuple[int, int]] = deque()

    def candidate(x: int, y: int) -> bool:
        r, g, b, _a = pixels[x, y]
        return (r, g, b) == TEAL or (r, g, b) == BLACK

    def push(x: int, y: int) -> None:
        idx = y * width + x
        if seen[idx] or not candidate(x, y):
            return
        seen[idx] = 1
        queue.append((x, y))

    for x in range(width):
        push(x, 0)
        push(x, height - 1)
    for y in range(height):
        push(0, y)
        push(width - 1, y)

    while queue:
        x, y = queue.popleft()
        r, g, b, _a = pixels[x, y]
        pixels[x, y] = (r, g, b, 0)
        if x > 0: push(x - 1, y)
        if x + 1 < width: push(x + 1, y)
        if y > 0: push(x, y - 1)
        if y + 1 < height: push(x, y + 1)

    bbox = rgba.getchannel("A").getbbox()
    if bbox is None:
        raise ValueError("crop became fully transparent")
    return rgba.crop(bbox)


def rgba_indexed(image: Image.Image, colours: int = 48) -> tuple[list[int], bytes]:
    rgba = image.convert("RGBA")
    alpha = rgba.getchannel("A")
    rgb = Image.new("RGB", rgba.size, (0, 0, 0))
    rgb.paste(rgba.convert("RGB"), mask=alpha)
    quantized = rgb.quantize(colors=colours, method=Image.Quantize.MEDIANCUT,
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
    for index, value in enumerate(alpha.tobytes()):
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
        raise ValueError(f"encoded texture is {len(encoded)} bytes")
    return offsets, bytes(encoded)


def emit_sprite(source: list[str], header: list[str], name: str, image: Image.Image) -> None:
    palette, pixels = rgba_indexed(image)
    offsets, runs = encode_rows(image.width, image.height, pixels)
    prefix = f"kPc{name}"
    symbol = f"gPc{name}Texture"
    source.append(f"static const uint32_t {prefix}Palette[256] = {{\n")
    source.append(fmt(palette, "0x{:08X}u", 6) + "\n};\n")
    source.append(f"static const uint16_t {prefix}RowOffsets[{len(offsets)}] = {{\n")
    source.append(fmt(offsets, "{}u", 12) + "\n};\n")
    source.append(f"static const uint8_t {prefix}Runs[{len(runs)}] = {{\n")
    source.append(fmt(list(runs), "{}u", 24) + "\n};\n")
    source.append(
        f"const TextureRle {symbol} = {{\n"
        f"    {image.width}u, {image.height}u, {TRANSPARENT_INDEX}u,\n"
        f"    {prefix}RowOffsets, {prefix}Runs, {prefix}Palette,\n"
        f"}};\n\n"
    )
    header.append(f"extern const TextureRle {symbol};\n")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("general_root", type=Path)
    parser.add_argument("minigame_root", type=Path)
    parser.add_argument("output_c", type=Path)
    parser.add_argument("output_h", type=Path)
    args = parser.parse_args()

    source = [
        "/* Generated authentic PC endings/minigame sprites. Do not edit. */\n",
        '#include "assets/pc_finishing_visuals.h"\n',
        '#include "assets/pc_core_visuals.h"\n',
        '#include "renderer/texture.h"\n\n',
        "#include <stdint.h>\n\n",
    ]
    header = [
        "#pragma once\n\n",
        '#include "assets/pc_core_visuals.h"\n',
        '#include "renderer/texture.h"\n\n',
    ]

    for name, sprite_id, size, tile_width in ENDING_ASSETS:
        emit_tiled(source, args.general_root, name, sprite_id, size, tile_width)
        header.append(f"extern const PcTiledTexture gPc{name}Texture;\n")

    loaded: dict[str, Image.Image] = {}
    for name, filename, box in SPRITE_CROPS:
        if filename not in loaded:
            path = args.minigame_root / filename
            if not path.is_file():
                raise FileNotFoundError(path)
            loaded[filename] = Image.open(path).convert("RGBA")
        sprite = clear_border_background(loaded[filename].crop(box))
        emit_sprite(source, header, name, sprite)
        print(f"{name}: {filename} {box} -> {sprite.width}x{sprite.height}")

    args.output_c.parent.mkdir(parents=True, exist_ok=True)
    args.output_h.parent.mkdir(parents=True, exist_ok=True)
    args.output_c.write_text("".join(source), encoding="utf-8")
    args.output_h.write_text("".join(header), encoding="utf-8")
    print("PC finishing visuals: endings=123/172/346 + authentic minigame sprites")


if __name__ == "__main__":
    main()
