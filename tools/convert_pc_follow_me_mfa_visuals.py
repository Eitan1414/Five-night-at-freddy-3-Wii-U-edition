#!/usr/bin/env python3
from __future__ import annotations

import argparse
from pathlib import Path
from PIL import Image

TRANSPARENT_INDEX = 255
COLOURS = 48
NUM = 5
DEN = 8

# symbol, PC General Sprites image id, MFA hotspot x/y, transparent RGB.
# Every image is converted from the original 1024x768 Clickteam coordinate
# space to the centered 640x480 Wii U content area at an exact 5/8 scale.
ASSETS = (
    ("Checker", 335, 0, 0, None),
    ("BackdropWide", 31, 0, 0, None),
    ("BackdropTall", 42, 0, 0, None),
    ("Stage", 333, 310, 210, (0, 0, 0)),
    ("ChicaDecor", 330, 61, 79, (0, 0, 0)),
    ("BonnieDecor", 331, 60, 98, (0, 0, 0)),
    ("TableFan", 326, 175, 151, (0, 0, 0)),
    ("Gift", 340, 223, 189, (0, 0, 0)),
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
    ("DropFall", 368, 8, 15, (0, 0, 0)),
    ("Blood", 364, 199, 79, (0, 0, 0)),
    ("Err", 371, 99, 47, (0, 0, 0)),
    ("FollowText", 1005, -4, 0, (90, 90, 90)),
    ("Controls", 348, 128, 80, (0, 0, 0)),
    ("Blocked", 372, 65, 200, (0, 0, 0)),

    # Playable Freddy, Bonnie, Chica and Foxy: directions E/N/W/S.
    ("PlayerFreddyRight0", 21, 106, 99, (0, 0, 0)),
    ("PlayerFreddyRight1", 22, 106, 101, (0, 0, 0)),
    ("PlayerFreddyUp0", 23, 104, 101, (0, 0, 0)),
    ("PlayerFreddyUp1", 24, 103, 102, (0, 0, 0)),
    ("PlayerFreddyLeft0", 25, 93, 99, (0, 0, 0)),
    ("PlayerFreddyLeft1", 26, 93, 101, (0, 0, 0)),
    ("PlayerFreddyDown0", 27, 104, 99, (0, 0, 0)),
    ("PlayerFreddyDown1", 28, 103, 99, (0, 0, 0)),

    ("PlayerBonnieRight0", 402, 101, 92, (0, 0, 0)),
    ("PlayerBonnieRight1", 403, 101, 94, (0, 0, 0)),
    ("PlayerBonnieUp0", 404, 105, 95, (0, 0, 0)),
    ("PlayerBonnieUp1", 405, 104, 95, (0, 0, 0)),
    ("PlayerBonnieLeft0", 406, 98, 92, (0, 0, 0)),
    ("PlayerBonnieLeft1", 407, 98, 94, (0, 0, 0)),
    ("PlayerBonnieDown0", 408, 105, 93, (0, 0, 0)),
    ("PlayerBonnieDown1", 409, 104, 93, (0, 0, 0)),

    ("PlayerChicaRight0", 412, 105, 108, (0, 0, 0)),
    ("PlayerChicaRight1", 413, 104, 108, (0, 0, 0)),
    ("PlayerChicaUp0", 414, 104, 110, (0, 0, 0)),
    ("PlayerChicaUp1", 415, 103, 110, (0, 0, 0)),
    ("PlayerChicaLeft0", 416, 105, 108, (0, 0, 0)),
    ("PlayerChicaLeft1", 417, 104, 108, (0, 0, 0)),
    ("PlayerChicaDown0", 418, 105, 108, (0, 0, 0)),
    ("PlayerChicaDown1", 419, 104, 108, (0, 0, 0)),

    ("PlayerFoxyRight0", 444, 104, 98, (0, 0, 0)),
    ("PlayerFoxyRight1", 445, 104, 99, (0, 0, 0)),
    ("PlayerFoxyUp0", 446, 104, 102, (0, 0, 0)),
    ("PlayerFoxyUp1", 447, 104, 103, (0, 0, 0)),
    ("PlayerFoxyLeft0", 448, 105, 98, (0, 0, 0)),
    ("PlayerFoxyLeft1", 449, 105, 98, (0, 0, 0)),
    ("PlayerFoxyDown0", 450, 103, 102, (0, 0, 0)),
    ("PlayerFoxyDown1", 451, 102, 102, (0, 0, 0)),

    # Active 360 `child`; handle 366 `child 2` uses the Up pair 423/424.
    ("PlayerGhostRight0", 421, 25, 43, (0, 0, 0)),
    ("PlayerGhostRight1", 422, 25, 38, (0, 0, 0)),
    ("PlayerGhostUp0", 423, 25, 42, (0, 0, 0)),
    ("PlayerGhostUp1", 424, 25, 36, (0, 0, 0)),
    ("PlayerGhostLeft0", 429, 25, 43, (0, 0, 0)),
    ("PlayerGhostLeft1", 440, 25, 38, (0, 0, 0)),
    ("PlayerGhostDown0", 442, 25, 43, (0, 0, 0)),
    ("PlayerGhostDown1", 443, 25, 40, (0, 0, 0)),

    # Shadow Freddy guide objects 347/348/349.
    ("ShadowRight0", 375, 106, 100, (0, 0, 0)),
    ("ShadowRight1", 376, 106, 101, (0, 0, 0)),
    ("ShadowDown0", 377, 104, 99, (0, 0, 0)),
    ("ShadowDown1", 378, 103, 99, (0, 0, 0)),
    ("ShadowUp0", 379, 104, 102, (0, 0, 0)),
    ("ShadowUp1", 380, 103, 102, (0, 0, 0)),

    # Ch.1-4 Purple Guy and exact dismantled-parts Actives.
    ("HuntMan0", 382, 103, 101, (0, 0, 0)),
    ("HuntMan1", 383, 103, 104, (0, 0, 0)),
    ("WreckFreddy", 401, 145, 60, (0, 0, 0)),
    ("WreckBonnie", 410, 203, 77, (0, 0, 0)),
    ("WreckChica", 420, 179, 63, (0, 0, 0)),
    ("WreckFoxy", 452, 168, 50, (0, 0, 0)),

    # Secret-room clue Actives 373..384.
    ("ClueBBDouble", 1007, -3, 0, (90, 90, 90)),
    ("Clue6", 1009, 43, 35, (0, 0, 0)),
    ("Clue7", 1011, 43, 35, (0, 0, 0)),
    ("Clue8", 1012, 43, 35, (0, 0, 0)),
    ("Clue9", 1013, 43, 35, (0, 0, 0)),
    ("ClueButtons", 171, 38, 44, (0, 0, 0)),
    ("Clue14", 1016, -3, 0, (90, 90, 90)),
    ("Clue15", 1006, -3, 0, (90, 90, 90)),
    ("Clue16", 173, 54, 112, (0, 0, 0)),

    # Final Follow Me actors / suit / springlock.
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
    ("Brud", 502, 12, 12, (0, 0, 0)),
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
