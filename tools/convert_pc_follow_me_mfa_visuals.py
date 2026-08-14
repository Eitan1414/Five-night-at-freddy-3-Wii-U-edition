#!/usr/bin/env python3
from __future__ import annotations
import argparse
from pathlib import Path
from PIL import Image

TRANSPARENT_INDEX = 255
COLOURS = 48
NUM = 5
DEN = 8

# symbol, General Sprites image id, hotspot x/y from fivenights3-94.mfa,
# transparent RGB (None = opaque). Follow Me runs in the PC 1024x768 frame;
# the Wii U renderer keeps that 4:3 image at 640x480 with side pillars.
ASSETS = (
    ("Checker", 335, 0, 0, None),
    ("BackdropWide", 31, 0, 0, None),
    ("BackdropTall", 42, 0, 0, None),
    ("Stage", 333, 310, 210, (0, 0, 0)),
    ("ChicaDecor", 330, 61, 79, (0, 0, 0)),
    ("BonnieDecor", 331, 60, 98, (0, 0, 0)),
    ("Arcade", 457, 68, 104, (0, 0, 0)),
    ("Trash", 332, 376, 261, (0, 0, 0)),
    ("PartyTable0", 293, 149, 101, (0, 0, 0)),
    ("PartyTable12", 294, 149, 99, (0, 0, 0)),
    ("PartyTable13", 296, 149, 98, (0, 0, 0)),
    ("PartyTable14", 307, 147, 112, (0, 0, 0)),
    ("PartyTable15", 309, 148, 103, (0, 0, 0)),
    ("Hallway", 367, 0, 0, (0, 0, 0)),
    ("Curtain", 327, 99, 219, (0, 0, 0)),
    ("Raindrop", 273, 16, 16, (0, 0, 0)),
    ("Blood", 364, 199, 79, (0, 0, 0)),
    ("Err", 371, 99, 47, (0, 0, 0)),
    ("FollowText", 1005, -4, 0, (90, 90, 90)),
    ("Controls", 348, 128, 80, (0, 0, 0)),
    ("Blocked", 372, 65, 200, (0, 0, 0)),

    # Exact secret-room clue objects, handles 373..384. Image IDs, hotspots
    # and colour keys are read directly from the MFA image bank.
    ("ClueBBDouble", 1007, -3, 0, (90, 90, 90)),
    ("Clue6", 1009, 43, 35, (0, 0, 0)),
    ("Clue7", 1011, 43, 35, (0, 0, 0)),
    ("Clue8", 1012, 43, 35, (0, 0, 0)),
    ("Clue9", 1013, 43, 35, (0, 0, 0)),
    ("ClueButtons", 171, 38, 44, (0, 0, 0)),
    ("Clue14", 1016, -3, 0, (90, 90, 90)),
    ("Clue15", 1006, -3, 0, (90, 90, 90)),
    ("Clue16", 173, 54, 112, (0, 0, 0)),

    # Final Follow Me (chapter 5). These are the actual frames referenced by
    # handles 363 (`man`), 368 (`man run`) and 369 (`Active 4`) in the MFA.
    # No hand-drawn Purple Guy / Spring Bonnie reconstruction is used here.
    ("FinalManRight0", 453, 104, 94, (0, 0, 0)),
    ("FinalManRight1", 454, 108, 99, (0, 0, 0)),
    ("FinalManLeft0", 455, 95, 94, (0, 0, 0)),
    ("FinalManLeft1", 456, 91, 99, (0, 0, 0)),
    ("FinalManRun0", 458, 106, 101, (0, 0, 0)),
    ("FinalManRun1", 460, 103, 105, (0, 0, 0)),
    ("FinalSuitIdle", 471, 73, 150, (0, 0, 0)),
    ("Spring12", 472, 73, 150, (0, 0, 0)),
    ("Spring13", 473, 54, 274, (0, 0, 0)),
    ("Spring14A", 486, 51, 275, (0, 0, 0)),
    ("Spring14B", 489, 51, 270, (0, 0, 0)),
    ("Spring15A", 490, 104, 271, (0, 0, 0)),
    ("Spring15B", 494, 104, 271, (0, 0, 0)),
    ("Spring16A", 495, 102, 199, (0, 0, 0)),
    ("Spring16B", 496, 102, 196, (0, 0, 0)),
    ("Spring17A", 497, 101, 162, (0, 0, 0)),
    ("Spring17B", 498, 101, 160, (0, 0, 0)),
)


def scaled(value: int) -> int:
    if value >= 0:
        return (value * NUM + DEN // 2) // DEN
    return -(((-value) * NUM + DEN // 2) // DEN)


def fmt(values, pattern: str, per_line: int) -> str:
    return "\n".join(
        "    " + ", ".join(pattern.format(v) for v in values[i:i + per_line]) + ","
        for i in range(0, len(values), per_line)
    )


def apply_key(image: Image.Image, key: tuple[int, int, int] | None) -> Image.Image:
    rgba = image.convert("RGBA")
    if key is None:
        return rgba
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
    quantized = rgb.quantize(
        colors=COLOURS,
        method=Image.Quantize.MEDIANCUT,
        dither=Image.Dither.NONE,
    )
    raw = quantized.getpalette() or []
    palette = []
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
        raise ValueError(f"RLE {len(runs)} > 65535 for {width}x{height}")
    return offsets, bytes(runs)


def emit(source: list[str], header: list[str], name: str, image: Image.Image,
         hotspot_x: int, hotspot_y: int, sprite_id: int) -> None:
    width = scaled(image.width)
    height = scaled(image.height)
    image = image.resize((width, height), Image.Resampling.NEAREST)
    palette, pixels = indexed(image)
    offsets, runs = encode_rows(width, height, pixels)
    prefix = f"kFollowMfa{name}"
    symbol = f"gFollowMfa{name}"
    source.append(
        f"/* PC General Sprites ID {sprite_id}; MFA hotspot "
        f"({hotspot_x},{hotspot_y}); 1024x768 -> 640x480. */\n"
    )
    source.append(
        f"static const uint32_t {prefix}Palette[256] = {{\n"
        f"{fmt(palette, '0x{:08X}u', 6)}\n}};\n"
    )
    source.append(
        f"static const uint16_t {prefix}RowOffsets[{len(offsets)}] = {{\n"
        f"{fmt(offsets, '{}u', 12)}\n}};\n"
    )
    source.append(
        f"static const uint8_t {prefix}Runs[{len(runs)}] = {{\n"
        f"{fmt(list(runs), '{}u', 24)}\n}};\n"
    )
    source.append(
        f"const FollowMeMfaTexture {symbol} = {{\n"
        f"    {{ {width}u, {height}u, {TRANSPARENT_INDEX}u, "
        f"{prefix}RowOffsets, {prefix}Runs, {prefix}Palette }},\n"
        f"    {scaled(hotspot_x)}, {scaled(hotspot_y)},\n"
        f"}};\n\n"
    )
    header.append(f"extern const FollowMeMfaTexture {symbol};\n")
    print(name, sprite_id, image.size, "hot", scaled(hotspot_x),
          scaled(hotspot_y), "rle", len(runs))


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("root", type=Path)
    parser.add_argument("out_c", type=Path)
    parser.add_argument("out_h", type=Path)
    args = parser.parse_args()

    source = [
        "/* Generated from the original PC FNaF 3 image bank / supplied MFA metadata. */\n",
        '#include "assets/follow_me_mfa_visuals.h"\n\n',
        "#include <stdint.h>\n\n",
    ]
    header = [
        "#pragma once\n\n",
        '#include "renderer/texture.h"\n\n',
        "#include <stdint.h>\n\n",
        "typedef struct FollowMeMfaTexture {\n"
        "    TextureRle texture;\n"
        "    int16_t hotspot_x;\n"
        "    int16_t hotspot_y;\n"
        "} FollowMeMfaTexture;\n\n",
    ]

    for name, sprite_id, hotspot_x, hotspot_y, key in ASSETS:
        path = args.root / f"{sprite_id}.png"
        if not path.is_file():
            raise FileNotFoundError(path)
        emit(source, header, name, apply_key(Image.open(path), key),
             hotspot_x, hotspot_y, sprite_id)

    args.out_c.parent.mkdir(parents=True, exist_ok=True)
    args.out_h.parent.mkdir(parents=True, exist_ok=True)
    args.out_c.write_text("".join(source), encoding="utf-8")
    args.out_h.write_text("".join(header), encoding="utf-8")


if __name__ == "__main__":
    main()
