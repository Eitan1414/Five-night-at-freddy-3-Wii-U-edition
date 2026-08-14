#!/usr/bin/env python3
from __future__ import annotations

import argparse
from pathlib import Path
from PIL import Image

TRANSPARENT_INDEX = 255
COLOURS = 48
NUM = 5
DEN = 8

# Active 354 `take apart`, animation 0, speed 10.
# image id, symbol, hotspot x/y, transparent colour
FRAMES = (
    (386, "gFollowMfaTakeApart0", 110, 101, (0, 0, 0)),
    (400, "gFollowMfaTakeApart1", 108, 102, (0, 0, 0)),
)


def scaled(value: int) -> int:
    if value >= 0:
        return (value * NUM + DEN // 2) // DEN
    return -(((-value) * NUM + DEN // 2) // DEN)


def apply_key(image: Image.Image, key: tuple[int, int, int]) -> Image.Image:
    rgba = image.convert("RGBA")
    data = []
    for red, green, blue, alpha in rgba.getdata():
        if (red, green, blue) == key:
            data.append((red, green, blue, 0))
        else:
            data.append((red, green, blue, alpha))
    rgba.putdata(data)
    return rgba


def indexed(image: Image.Image) -> tuple[list[int], bytes]:
    rgba = image.convert("RGBA")
    alpha = rgba.getchannel("A")
    rgb = Image.new("RGB", rgba.size, (0, 0, 0))
    rgb.paste(rgba.convert("RGB"), mask=alpha)
    q = rgb.quantize(colors=COLOURS, method=Image.Quantize.MEDIANCUT,
                     dither=Image.Dither.NONE)
    raw = q.getpalette() or []
    palette = []
    for i in range(COLOURS):
        base = i * 3
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
        raise ValueError(f"RLE too large: {len(runs)}")
    return offsets, bytes(runs)


def fmt(values, pattern: str, per_line: int) -> str:
    return "\n".join(
        "    " + ", ".join(pattern.format(v) for v in values[i:i + per_line]) + ","
        for i in range(0, len(values), per_line)
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("root", type=Path)
    parser.add_argument("out_inc", type=Path)
    parser.add_argument("out_h", type=Path)
    args = parser.parse_args()

    source = [
        "/* Generated from Active 354 `take apart` in fivenights3-94.mfa. */\n",
        '#include "assets/follow_me_mfa_takeapart.h"\n\n',
    ]
    header = [
        "#pragma once\n\n",
        '#include "assets/follow_me_mfa_visuals.h"\n\n',
    ]

    for sprite_id, symbol, hot_x, hot_y, key in FRAMES:
        path = args.root / f"{sprite_id}.png"
        if not path.is_file():
            raise FileNotFoundError(path)
        image = apply_key(Image.open(path), key)
        width = scaled(image.width)
        height = scaled(image.height)
        image = image.resize((width, height), Image.Resampling.NEAREST)
        palette, pixels = indexed(image)
        offsets, runs = encode_rows(width, height, pixels)
        prefix = symbol.replace("gFollow", "kFollow")

        source.append(
            f"/* PC image {sprite_id}; MFA hotspot ({hot_x},{hot_y}); speed 10. */\n"
            f"static const uint32_t {prefix}Palette[256] = {{\n"
            f"{fmt(palette, '0x{:08X}u', 6)}\n}};\n"
            f"static const uint16_t {prefix}Offsets[{len(offsets)}] = {{\n"
            f"{fmt(offsets, '{}u', 12)}\n}};\n"
            f"static const uint8_t {prefix}Runs[{len(runs)}] = {{\n"
            f"{fmt(list(runs), '{}u', 24)}\n}};\n"
            f"const FollowMeMfaTexture {symbol} = {{\n"
            f"    {{ {width}u, {height}u, {TRANSPARENT_INDEX}u, "
            f"{prefix}Offsets, {prefix}Runs, {prefix}Palette }},\n"
            f"    {scaled(hot_x)}, {scaled(hot_y)}\n"
            f"}};\n\n"
        )
        header.append(f"extern const FollowMeMfaTexture {symbol};\n")

    args.out_inc.parent.mkdir(parents=True, exist_ok=True)
    args.out_h.parent.mkdir(parents=True, exist_ok=True)
    args.out_inc.write_text("".join(source), encoding="utf-8")
    args.out_h.write_text("".join(header), encoding="utf-8")


if __name__ == "__main__":
    main()
