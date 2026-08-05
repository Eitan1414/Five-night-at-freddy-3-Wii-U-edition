#!/usr/bin/env python3
"""Convert original PSX Phantom gameplay images into Wii U row-RLE textures."""
from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path

from convert_jumpscare_tim import (
    encode_rows,
    fmt,
    parse_tim,
    psx_colour_to_rgbx,
)


@dataclass(frozen=True)
class VisualSpec:
    symbol: str
    candidates: tuple[str, ...]


VISUALS = (
    VisualSpec("gPhantomFoxyOfficeTexture", (
        "office/FOXY.TIM",
        "office/FOXY.tim",
    )),
    VisualSpec("gPhantomFreddyOfficeTexture", (
        "office/FREDDYWANDERING.tim",
        "office/FREDDYWANDERING.TIM",
    )),
    VisualSpec("gPhantomChicaOfficeTexture", (
        "screamer/Chica/CHJUMP1.tim",
        "screamer/Chica/CHJUMP1.TIM",
    )),
    VisualSpec("gPhantomMangleOfficeTexture", (
        "office/mangle.tim",
        "office/MANGLE.TIM",
    )),
    VisualSpec("gPhantomPuppetOfficeTexture", (
        "office/animatronics/PUPPET.TIM",
        "office/animatronics/PUPPET.tim",
    )),
    VisualSpec("gPhantomBBOfficeTexture", (
        "office/animatronics/BBREAL.TIM",
        "office/animatronics/BBREAL.tim",
    )),
    VisualSpec("gPhantomBBCameraTexture", (
        "camera/cams/animatronics/BBONCAM.tim",
        "camera/cams/animatronics/BBONCAM.TIM",
    )),
    VisualSpec("gPhantomChicaCameraTexture", (
        "camera/cams/animatronics/CHICA.tim",
        "camera/cams/animatronics/CHICA.TIM",
    )),
    VisualSpec("gPhantomMangleCameraTexture", (
        "camera/cams/animatronics/MANGLE04.tim",
        "camera/cams/animatronics/MANGLE04.TIM",
    )),
    VisualSpec("gPhantomPuppetCameraTexture", (
        "camera/cams/animatronics/PUPPET.tim",
        "camera/cams/animatronics/PUPPET.TIM",
    )),
)


def resolve_path(root: Path, candidates: tuple[str, ...]) -> Path:
    for candidate in candidates:
        path = root / candidate
        if path.is_file():
            return path
    joined = ", ".join(str(root / candidate) for candidate in candidates)
    raise FileNotFoundError(f"none of the Phantom source paths exist: {joined}")


def emit_exported_texture(source: list[str], symbol: str,
                          width: int, height: int,
                          colours: list[int], pixels: bytes) -> None:
    offsets, runs = encode_rows(width, height, pixels)
    palette_size = 16 if max(pixels, default=0) < 16 else 256
    palette = [psx_colour_to_rgbx(colour) for colour in colours[:palette_size]]
    palette.extend([0x000000FF] * (palette_size - len(palette)))
    unique = f"k{symbol[1:]}"

    source.append(f"static const uint32_t {unique}Palette[{palette_size}] = {{\n")
    source.append(fmt(palette, "0x{:08X}u", 6))
    source.append("\n};\n")
    source.append(f"static const uint16_t {unique}RowOffsets[{len(offsets)}] = {{\n")
    source.append(fmt(offsets, "{}u", 12))
    source.append("\n};\n")
    source.append(f"static const uint8_t {unique}Runs[{len(runs)}] = {{\n")
    source.append(fmt(runs, "{}u", 20))
    source.append("\n};\n")
    source.append(
        f"const TextureRle {symbol} = {{\n"
        f"    {width}u, {height}u, 0u,\n"
        f"    {unique}RowOffsets, {unique}Runs, {unique}Palette,\n"
        f"}};\n\n"
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("input_root", type=Path,
                        help="root of the original PSX tim/ directory")
    parser.add_argument("output_c", type=Path)
    parser.add_argument("output_h", type=Path)
    args = parser.parse_args()

    source = [
        "/* Generated directly from original FNaF 3 PSX Phantom TIM files. */\n",
        '#include "assets/phantom_assets.h"\n\n',
        "#include <stdint.h>\n\n",
    ]
    header = [
        "#pragma once\n\n",
        '#include "renderer/texture.h"\n\n',
    ]

    for spec in VISUALS:
        path = resolve_path(args.input_root, spec.candidates)
        width, height, colours, pixels = parse_tim(path)
        emit_exported_texture(source, spec.symbol,
                              width, height, colours, pixels)
        header.append(f"extern const TextureRle {spec.symbol};\n")
        print(f"{spec.symbol}: {path.relative_to(args.input_root)} "
              f"({width}x{height})")

    args.output_c.parent.mkdir(parents=True, exist_ok=True)
    args.output_h.parent.mkdir(parents=True, exist_ok=True)
    args.output_c.write_text("".join(source), encoding="utf-8")
    args.output_h.write_text("".join(header), encoding="utf-8")


if __name__ == "__main__":
    main()
