#!/usr/bin/env python3
from __future__ import annotations
import argparse
from collections import deque
from pathlib import Path
from PIL import Image

TRANSPARENT_INDEX = 255
TEAL = (55, 177, 182)
BLACK = (0, 0, 0)

# Exact crops from the original PC minigame sprite sheets.  The 1024x768
# entries are the original stripe/backdrop planes; the remaining crops are
# sprites/props used by the event-sheet animation states.
CROPS = (
    ("SecretBBBackdrop", "bb_air_adventure.png", (1, 807, 1025, 1575), True),
    ("SecretBBBalloonRed", "bb_air_adventure.png", (376, 49, 444, 163), False),
    ("SecretBBBalloonPurple", "bb_air_adventure.png", (445, 49, 513, 163), False),
    ("SecretBBBalloonGreen", "bb_air_adventure.png", (514, 49, 582, 163), False),
    ("SecretBBBalloonOrange", "bb_air_adventure.png", (583, 49, 651, 163), False),
    ("SecretMangleBackdrop", "mangles_quest.png", (1, 121, 1025, 889), True),
    ("SecretMangleAlt1", "mangles_quest.png", (114, 1, 226, 120), False),
    ("SecretMangleAlt2", "mangles_quest.png", (227, 1, 339, 120), False),
    ("SecretMangleAlt3", "mangles_quest.png", (340, 1, 452, 120), False),
    ("SecretMangleKid1", "mangles_quest.png", (1049, 10, 1217, 178), False),
    ("SecretMangleKid2", "mangles_quest.png", (1218, 10, 1386, 178), False),
    ("SecretMangleKid3", "mangles_quest.png", (1387, 10, 1555, 178), False),
    ("SecretMangleKid4", "mangles_quest.png", (1556, 10, 1724, 178), False),
    ("SecretChicaBackdrop", "chicas_party.png", (299, 210, 1323, 978), True),
    ("SecretChicaCupcakeSmall", "chicas_party.png", (227, 76, 287, 132), False),
    ("SecretChicaCupcakeBig", "chicas_party.png", (227, 133, 309, 209), False),
    ("SecretChicaWindow", "chicas_party.png", (925, 1, 1057, 209), False),
    ("SecretStagePlatform", "stage_01.png", (339, 471, 697, 563), False),
    ("SecretStageVertical538", "stage_01.png", (698, 25, 730, 563), False),
    ("SecretStageVertical562", "stage_01.png", (731, 1, 763, 563), False),
    ("SecretStageFloorBar", "stage_01.png", (1, 564, 763, 594), False),
    ("SecretStageSpringBonnie1", "stage_01.png", (1, 222, 131, 409), False),
    ("SecretStageSpringBonnie2", "stage_01.png", (132, 222, 262, 409), False),
    ("SecretShadowBackdrop", "shadow_bonnie.png", (34, 410, 1058, 1178), True),
    ("SecretShadowAlt1", "shadow_bonnie.png", (132, 1, 262, 188), False),
    ("SecretShadowAlt2", "shadow_bonnie.png", (263, 1, 393, 188), False),
    ("SecretShadowAlt3", "shadow_bonnie.png", (394, 1, 524, 188), False),
    ("SecretShadowAlt4", "shadow_bonnie.png", (525, 1, 655, 188), False),
    ("SecretShadowChild1", "shadow_bonnie.png", (1, 189, 95, 287), False),
    ("SecretShadowChild2", "shadow_bonnie.png", (96, 189, 190, 287), False),
    ("SecretHappiestPuppetAlt", "happiest_day.png", (114, 56, 226, 175), False),
    ("SecretHappiestGrayChild1", "happiest_day.png", (1, 176, 123, 303), False),
    ("SecretHappiestGrayChild2", "happiest_day.png", (184, 176, 306, 303), False),
    ("SecretHappiestGrayChild3", "happiest_day.png", (1, 304, 123, 431), False),
    ("SecretHappiestGrayChild4", "happiest_day.png", (189, 304, 311, 431), False),
    ("SecretHappiestGrayChild5", "happiest_day.png", (191, 432, 313, 559), False),
    ("SecretHappiestMask1", "happiest_day.png", (227, 126, 265, 175), False),
    ("SecretHappiestMask2", "happiest_day.png", (307, 241, 362, 303), False),
    ("SecretHappiestMask3", "happiest_day.png", (124, 374, 188, 431), False),
    ("SecretHappiestMask4", "happiest_day.png", (312, 373, 391, 431), False),
    ("SecretHappiestMask5", "happiest_day.png", (314, 503, 373, 559), False),
    ("SecretHappiestChild5", "happiest_day.png", (381, 560, 475, 706), False),
    ("SecretHappiestCakeColor", "happiest_day.png", (1, 707, 160, 875), False),
    ("SecretHappiestCakeGray", "happiest_day.png", (262, 707, 421, 875), False),
    ("SecretHappiestTableColor", "happiest_day.png", (1, 876, 261, 982), False),
    ("SecretHappiestTableGray", "happiest_day.png", (262, 876, 522, 982), False),
    ("SecretHappiestExit", "happiest_day.png", (1, 983, 104, 1107), False),
    ("SecretHappiestBalloonPurple", "happiest_day.png", (105, 993, 173, 1107), False),
    ("SecretHappiestBalloonGreen", "happiest_day.png", (174, 993, 242, 1107), False),
    ("SecretHappiestBalloonYellow", "happiest_day.png", (243, 993, 311, 1107), False),
    ("SecretHappiestBalloonBlue", "happiest_day.png", (312, 993, 380, 1107), False),
)

def fmt(values, pattern: str, per_line: int) -> str:
    return "\n".join(
        "    " + ", ".join(pattern.format(v) for v in values[i:i + per_line]) + ","
        for i in range(0, len(values), per_line)
    )

def clear_border_background(image: Image.Image) -> Image.Image:
    rgba = image.convert("RGBA")
    pixels = rgba.load()
    width, height = rgba.size
    seen = bytearray(width * height)
    queue: deque[tuple[int, int]] = deque()

    def candidate(x: int, y: int) -> bool:
        r, g, b, _ = pixels[x, y]
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
        r, g, b, _ = pixels[x, y]
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
    raw = quantized.getpalette() or []
    palette: list[int] = []
    for index in range(colours):
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
        raise ValueError(f"{width}x{height} encoded to {len(runs)} bytes")
    return offsets, bytes(runs)

def emit(source: list[str], header: list[str], name: str, image: Image.Image) -> None:
    palette, pixels = rgba_indexed(image)
    offsets, runs = encode_rows(image.width, image.height, pixels)
    prefix = f"kPc{name}"
    symbol = f"gPc{name}Texture"
    source.append(f"\n/* V8 authentic PC crop: {name}. */\n")
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
        f"}};\n"
    )
    header.append(f"extern const TextureRle {symbol};\n")

def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("minigame_root", type=Path)
    parser.add_argument("output_c", type=Path)
    parser.add_argument("output_h", type=Path)
    args = parser.parse_args()

    source = ["\n/* Appended V8 authentic PC minigame visual crops. */\n"]
    header = ["\n/* V8 authentic PC minigame visual crops. */\n"]
    loaded: dict[str, Image.Image] = {}
    for name, filename, box, opaque in CROPS:
        if filename not in loaded:
            path = args.minigame_root / filename
            if not path.is_file():
                raise FileNotFoundError(path)
            loaded[filename] = Image.open(path).convert("RGBA")
        crop = loaded[filename].crop(box)
        image = crop if opaque else clear_border_background(crop)
        emit(source, header, name, image)
        print(f"{name}: {filename} {box} -> {image.width}x{image.height}")

    with args.output_c.open("a", encoding="utf-8") as handle:
        handle.write("".join(source))
    with args.output_h.open("a", encoding="utf-8") as handle:
        handle.write("".join(header))

if __name__ == "__main__":
    main()
