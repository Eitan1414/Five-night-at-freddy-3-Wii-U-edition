#!/usr/bin/env python3
from __future__ import annotations

import argparse
from pathlib import Path
from PIL import Image

# Collision masks used by the original Follow Me movement events. They stay in
# the 1024x768 PC source coordinate system: movement is evaluated before the
# 5/8 Wii U rendering transform.
# symbol, General Sprites image id, MFA hotspot x/y, transparent colour key.
MASKS = (
    ("PartyTable0", 293, 149, 101, (0, 0, 0)),
    ("PartyTable12", 294, 149, 99, (0, 0, 0)),
    ("PartyTable13", 296, 149, 98, (0, 0, 0)),
    ("PartyTable14", 307, 147, 112, (0, 0, 0)),
    ("PartyTable15", 309, 148, 103, (0, 0, 0)),
    ("TableFan", 326, 175, 151, (0, 0, 0)),
    ("Gift", 340, 223, 189, (0, 0, 0)),
)


def mask_bits(image: Image.Image, key: tuple[int, int, int]) -> bytes:
    rgb = image.convert("RGB")
    out = bytearray((rgb.width * rgb.height + 7) // 8)
    for y in range(rgb.height):
        for x in range(rgb.width):
            if rgb.getpixel((x, y)) == key:
                continue
            index = y * rgb.width + x
            out[index >> 3] |= 1 << (index & 7)
    return bytes(out)


def fmt(values: bytes, per_line: int = 24) -> str:
    return "\n".join(
        "    " + ", ".join(f"{value}u" for value in values[i:i + per_line]) + ","
        for i in range(0, len(values), per_line)
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("root", type=Path)
    parser.add_argument("out_inc", type=Path)
    parser.add_argument("out_h", type=Path)
    args = parser.parse_args()

    source = [
        "/* Generated from original PC General Sprites for MFA collision tests. */\n",
        '#include "assets/follow_me_mfa_collisions.h"\n\n',
    ]
    header = [
        "#pragma once\n\n",
        "#include <stdint.h>\n\n",
        "typedef struct FollowMeMfaCollisionMask {\n"
        "    uint16_t width;\n"
        "    uint16_t height;\n"
        "    int16_t hotspot_x;\n"
        "    int16_t hotspot_y;\n"
        "    const uint8_t *bits;\n"
        "} FollowMeMfaCollisionMask;\n\n",
    ]

    for name, sprite_id, hotspot_x, hotspot_y, key in MASKS:
        path = args.root / f"{sprite_id}.png"
        if not path.is_file():
            raise FileNotFoundError(path)
        image = Image.open(path)
        bits = mask_bits(image, key)
        array_name = f"kFollowMfaCollision{name}Bits"
        symbol = f"gFollowMfaCollision{name}"
        source.append(
            f"/* PC image {sprite_id}; MFA hotspot ({hotspot_x},{hotspot_y}). */\n"
            f"static const uint8_t {array_name}[{len(bits)}] = {{\n"
            f"{fmt(bits)}\n}};\n"
            f"const FollowMeMfaCollisionMask {symbol} = {{\n"
            f"    {image.width}u, {image.height}u, {hotspot_x}, {hotspot_y},\n"
            f"    {array_name}\n"
            f"}};\n\n"
        )
        header.append(f"extern const FollowMeMfaCollisionMask {symbol};\n")
        print(name, sprite_id, image.size, "mask", len(bits))

    args.out_inc.parent.mkdir(parents=True, exist_ok=True)
    args.out_h.parent.mkdir(parents=True, exist_ok=True)
    args.out_inc.write_text("".join(source), encoding="utf-8")
    args.out_h.write_text("".join(header), encoding="utf-8")


if __name__ == "__main__":
    main()
