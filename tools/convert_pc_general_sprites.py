#!/usr/bin/env python3
"""Generate verified high-resolution PC visuals from FNaF 3 General Sprites.

The office and all ten main-camera base backgrounds are sourced from the
numbered PC General Sprites dump. Dynamic character/rare-state overlays remain
separate so they can be migrated without destabilising camera navigation.
"""
from __future__ import annotations

import argparse
from pathlib import Path
from PIL import Image

COLOURS = 80
TRANSPARENT_INDEX = 255

# name, General Sprites PNG id, Wii U render size, tile width, camera index.
# camera index is zero-based (CAM01=0); None marks non-camera assets.
ASSETS = (
    ("Office", 203, (1120, 430), 128, None),
    ("Camera01", 106, (532, 295), 192, 0),
    ("Camera02", 97, (532, 295), 192, 1),
    ("Camera03", 104, (532, 295), 192, 2),
    ("Camera04", 105, (532, 295), 192, 3),
    ("Camera05", 109, (532, 295), 192, 4),
    ("Camera06", 98, (532, 295), 192, 5),
    ("Camera07", 100, (532, 295), 192, 6),
    ("Camera08", 112, (532, 295), 192, 7),
    ("Camera09", 115, (532, 295), 192, 8),
    ("Camera10", 116, (532, 295), 192, 9),
)


def fmt(values, pattern: str, per_line: int) -> str:
    lines = []
    for start in range(0, len(values), per_line):
        chunk = values[start:start + per_line]
        lines.append("    " + ", ".join(pattern.format(value) for value in chunk) + ",")
    return "\n".join(lines)


def quantize_image(path: Path, size: tuple[int, int]) -> Image.Image:
    image = Image.open(path).convert("RGB")
    image = image.resize(size, Image.Resampling.LANCZOS)
    return image.quantize(colors=COLOURS,
                          method=Image.Quantize.MEDIANCUT,
                          dither=Image.Dither.NONE)


def palette_rgba(image: Image.Image) -> list[int]:
    raw = image.getpalette() or []
    result = []
    for index in range(COLOURS):
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
            while (x + length < x1 and
                   int(pixels[x + length, y]) == value and
                   length < 255):
                length += 1
            runs.extend((length, value))
            x += length
        offsets.append(len(runs))
    if len(runs) > 0xFFFF:
        raise ValueError(
            f"tile {x0}:{x1} encodes to {len(runs)} bytes; split it further"
        )
    return offsets, bytes(runs)


def emit_asset(source: list[str], root: Path, name: str, sprite_id: int,
               size: tuple[int, int], tile_width: int) -> None:
    path = root / f"{sprite_id}.png"
    if not path.is_file():
        raise FileNotFoundError(path)

    image = quantize_image(path, size)
    palette = palette_rgba(image)
    prefix = f"kPc{name}"
    source.append(
        f"/* General Sprites ID {sprite_id}; resized to {size[0]}x{size[1]}. */\n"
    )
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
            f"    {x1 - x}u, {size[1]}u, {TRANSPARENT_INDEX}u,\n"
            f"    {symbol}RowOffsets, {symbol}Runs, {prefix}Palette,\n"
            f"}};\n"
        )
        tile_symbols.append(symbol)
        x = x1
        tile_index += 1

    source.append(
        f"static const TextureRle *const {prefix}Tiles[{len(tile_symbols)}] = {{\n"
    )
    source.append("\n".join(f"    &{symbol}," for symbol in tile_symbols))
    source.append("\n};\n")
    source.append(
        f"const PcTiledTexture gPc{name}Texture = {{\n"
        f"    {size[0]}u, {size[1]}u, {len(tile_symbols)}u, {prefix}Tiles,\n"
        f"}};\n\n"
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("sprite_root", type=Path,
                        help="DataDump3 directory containing numbered PNGs")
    parser.add_argument("output_c", type=Path)
    parser.add_argument("output_h", type=Path)
    args = parser.parse_args()

    source = [
        "/* Generated high-resolution PC visual core. Do not edit by hand. */\n",
        '#include "assets/pc_core_visuals.h"\n',
        '#include "renderer/texture.h"\n\n',
        "#include <stddef.h>\n",
        "#include <stdint.h>\n\n",
    ]
    for name, sprite_id, size, tile_width, _camera_index in ASSETS:
        emit_asset(source, args.sprite_root, name, sprite_id, size, tile_width)

    source.append(r'''void pc_tiled_texture_draw(uint32_t targets,
                           int x,
                           int y,
                           int width,
                           int height,
                           const PcTiledTexture *texture)
{
    if (texture == NULL || texture->tiles == NULL ||
        texture->width == 0u || texture->height == 0u ||
        texture->tile_count == 0u || width <= 0 || height <= 0) {
        return;
    }

    uint32_t source_x = 0u;
    for (uint16_t index = 0u; index < texture->tile_count; ++index) {
        const TextureRle *tile = texture->tiles[index];
        if (tile == NULL || tile->width == 0u) continue;
        const uint32_t next_source_x = source_x + tile->width;
        const int destination_x0 = x +
            (int) ((source_x * (uint32_t) width) / texture->width);
        const int destination_x1 = x +
            (int) ((next_source_x * (uint32_t) width) / texture->width);
        texture_draw_rle(targets, destination_x0, y,
                         destination_x1 - destination_x0, height, tile);
        source_x = next_source_x;
    }
}

const PcTiledTexture *pc_core_camera_texture(int camera_index)
{
    switch (camera_index) {
''')
    for name, _sprite_id, _size, _tile_width, camera_index in ASSETS:
        if camera_index is not None:
            source.append(f"        case {camera_index}: return &gPc{name}Texture;\n")
    source.append(r'''        default: return NULL;
    }
}
''')

    header_lines = [
        "#pragma once\n\n",
        "#include <stdint.h>\n",
        '#include "renderer/texture.h"\n\n',
        "typedef struct PcTiledTexture {\n",
        "    uint16_t width;\n",
        "    uint16_t height;\n",
        "    uint16_t tile_count;\n",
        "    const TextureRle *const *tiles;\n",
        "} PcTiledTexture;\n\n",
    ]
    for name, _sprite_id, _size, _tile_width, _camera_index in ASSETS:
        header_lines.append(f"extern const PcTiledTexture gPc{name}Texture;\n")
    header_lines.extend([
        "\nvoid pc_tiled_texture_draw(uint32_t targets,\n",
        "                           int x,\n",
        "                           int y,\n",
        "                           int width,\n",
        "                           int height,\n",
        "                           const PcTiledTexture *texture);\n\n",
        "const PcTiledTexture *pc_core_camera_texture(int camera_index);\n",
    ])

    args.output_c.parent.mkdir(parents=True, exist_ok=True)
    args.output_h.parent.mkdir(parents=True, exist_ok=True)
    args.output_c.write_text("".join(source), encoding="utf-8")
    args.output_h.write_text("".join(header_lines), encoding="utf-8")

    mapping = [f"{name}={sprite_id}" for name, sprite_id, *_rest in ASSETS]
    print("PC visual core: " + " ".join(mapping))


if __name__ == "__main__":
    main()
