#!/usr/bin/env python3
"""Convert the exact PC FNaF 3 night-intro Active frames to Wii U RLE.

Source-of-truth data recovered from fivenights3-94.mfa, frame `what day`:
- Active object handle 55 is placed at (512, 374) in the 1024x768 frame.
- all six frames use hotspot (100, 52) and native size 201x104.
- RGB(90,90,90) is the Clickteam transparent colour, not visible artwork.
- image handles: Night 1=1098, Night 2=837, Night 3=838,
  Night 4=842, Night 5=843, Night 6=844.

This script performs format conversion only; it does not redraw text or create a
replacement font/image.
"""
from __future__ import annotations

import argparse
from pathlib import Path
from PIL import Image

TRANSPARENT = (90, 90, 90)
TRANSPARENT_INDEX = 255
FRAMES = (
    (1, 1098, (194, 229, 80)),
    (2, 837, (255, 255, 255)),
    (3, 838, (255, 255, 255)),
    (4, 842, (255, 255, 255)),
    (5, 843, (255, 255, 255)),
    (6, 844, (255, 255, 255)),
)


def fmt(values: list[int], per_line: int, suffix: str = "u") -> str:
    return "\n".join(
        "    " + ", ".join(f"{value}{suffix}" for value in values[i:i + per_line]) + ","
        for i in range(0, len(values), per_line)
    )


def encode(image: Image.Image, foreground: tuple[int, int, int]) -> tuple[list[int], list[int]]:
    rgb = image.convert("RGB")
    if rgb.size != (201, 104):
        raise SystemExit(f"Unexpected night-intro size: {rgb.size}, expected 201x104")

    indexed: list[int] = []
    for colour in rgb.getdata():
        if colour == TRANSPARENT:
            indexed.append(TRANSPARENT_INDEX)
        elif colour == foreground:
            indexed.append(0)
        else:
            raise SystemExit(f"Unexpected source colour {colour}; refusing to approximate the PC sprite")

    offsets = [0]
    runs: list[int] = []
    width, height = rgb.size
    for y in range(height):
        row = indexed[y * width:(y + 1) * width]
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
        raise SystemExit("Night-intro RLE exceeds the Wii U uint16 row-offset format")
    return offsets, runs


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("sprite_root", type=Path)
    parser.add_argument("output_c", type=Path)
    args = parser.parse_args()

    source = [
        "/* Generated from exact PC night-intro sprites; do not hand-edit. */\n",
        '#include "renderer/texture.h"\n\n',
        "#include <stdint.h>\n\n",
        "/* MFA: frame `what day`, Active handle 55, position (512,374),\n",
        " * hotspot (100,52), source canvas 1024x768. */\n",
    ]
    symbols: list[str] = []

    for night, image_id, foreground in FRAMES:
        path = args.sprite_root / f"{image_id}.png"
        if not path.is_file():
            raise SystemExit(f"Missing exact PC General Sprites image: {path}")
        offsets, runs = encode(Image.open(path), foreground)
        symbol = f"gPcNightIntroNight{night}Texture"
        symbols.append(symbol)
        palette = (foreground[0] << 24) | (foreground[1] << 16) | (foreground[2] << 8) | 0xFF
        source.extend([
            f"\n/* PC General Sprites image {image_id}. */\n",
            f"static const uint32_t kNight{night}Palette[256] = {{ 0x{palette:08X}u }};\n",
            f"static const uint16_t kNight{night}Rows[{len(offsets)}] = {{\n",
            fmt(offsets, 12), "\n};\n",
            f"static const uint8_t kNight{night}Runs[{len(runs)}] = {{\n",
            fmt(runs, 24), "\n};\n",
            f"static const TextureRle {symbol} = {{\n",
            f"    201u, 104u, 255u, kNight{night}Rows, kNight{night}Runs, kNight{night}Palette,\n",
            "};\n",
        ])

    source.extend([
        "\nconst TextureRle *pc_night_intro_texture(int night)\n{\n",
        "    static const TextureRle *const textures[6] = {\n",
    ])
    source.extend(f"        &{symbol},\n" for symbol in symbols)
    source.extend([
        "    };\n",
        "    if (night < 1) night = 1;\n",
        "    if (night > 6) night = 6;\n",
        "    return textures[night - 1];\n",
        "}\n",
    ])

    args.output_c.parent.mkdir(parents=True, exist_ok=True)
    args.output_c.write_text("".join(source), encoding="utf-8")


if __name__ == "__main__":
    main()
