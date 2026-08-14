#!/usr/bin/env python3
from __future__ import annotations

import argparse
import struct
import zlib
from pathlib import Path

F3I2_HEADER = struct.Struct(">4sHHHHI")
PDD1_HEADER = struct.Struct(">4sHHHHHHI")


def rgb332_palette() -> bytes:
    palette = bytearray()
    for packed in range(256):
        r3 = (packed >> 5) & 0x07
        g3 = (packed >> 2) & 0x07
        b2 = packed & 0x03

        # Match the optimized Wii U F3I2 player exactly: RGB332 -> RGB565,
        # then express those quantized RGB565 levels as RGBA8888 for the
        # current startup renderer.
        r5 = (r3 * 31 + 3) // 7
        g6 = (g3 * 63 + 3) // 7
        b5 = (b2 * 31 + 1) // 3
        r8 = (r5 * 255 + 15) // 31
        g8 = (g6 * 255 + 31) // 63
        b8 = (b5 * 255 + 15) // 31
        palette.extend((r8, g8, b8, 255))
    return bytes(palette)


def convert(source: Path, target: Path) -> None:
    packed = source.read_bytes()
    if len(packed) < F3I2_HEADER.size:
        raise SystemExit("F3I2 source is truncated")

    magic, width, height, fps, frame_count, frame_bytes = F3I2_HEADER.unpack_from(packed)
    if magic != b"F3I2":
        raise SystemExit(f"unexpected intro magic: {magic!r}")
    if width == 0 or height == 0 or fps == 0 or frame_count == 0:
        raise SystemExit("invalid F3I2 geometry/timing")
    if frame_bytes != width * height:
        raise SystemExit(
            f"unexpected F3I2 frame size: {frame_bytes} != {width}*{height}"
        )

    cursor = F3I2_HEADER.size
    frame = bytearray(frame_bytes)
    previous = bytearray(frame_bytes)
    records: list[bytes] = []

    for index in range(frame_count):
        if cursor + 5 > len(packed):
            raise SystemExit(f"F3I2 frame {index}: truncated record header")
        frame_type = packed[cursor]
        compressed_size = struct.unpack_from(">I", packed, cursor + 1)[0]
        cursor += 5
        if compressed_size == 0 or cursor + compressed_size > len(packed):
            raise SystemExit(f"F3I2 frame {index}: invalid compressed size")

        compressed = packed[cursor:cursor + compressed_size]
        cursor += compressed_size
        try:
            decoded = zlib.decompress(compressed)
        except zlib.error as exc:
            raise SystemExit(f"F3I2 frame {index}: zlib error: {exc}") from exc
        if len(decoded) != frame_bytes:
            raise SystemExit(
                f"F3I2 frame {index}: decoded {len(decoded)} bytes, expected {frame_bytes}"
            )

        if frame_type == 0:
            frame[:] = decoded
        elif frame_type == 1:
            for pos, value in enumerate(decoded):
                frame[pos] ^= value
        else:
            raise SystemExit(f"F3I2 frame {index}: unsupported type {frame_type}")

        delta = bytes(current ^ old for current, old in zip(frame, previous))
        encoded = zlib.compress(delta, 9)
        records.append(struct.pack(">I", len(encoded)) + encoded)
        previous[:] = frame

    if cursor != len(packed):
        raise SystemExit(
            f"F3I2 has {len(packed) - cursor} unexpected trailing byte(s)"
        )

    target.parent.mkdir(parents=True, exist_ok=True)
    output = bytearray()
    output += PDD1_HEADER.pack(
        b"PDD1",
        1,
        width,
        height,
        frame_count,
        fps,
        1,
        0,
    )
    output += rgb332_palette()
    output += b"".join(records)
    target.write_bytes(output)

    # Parse the output header again so an accidental struct/layout change fails
    # here rather than later on Wii U hardware.
    check = target.read_bytes()
    if len(check) < PDD1_HEADER.size + 256 * 4:
        raise SystemExit("generated PDD1 file is unexpectedly small")
    out_magic, version, out_w, out_h, out_count, fps_num, fps_den, _ = PDD1_HEADER.unpack_from(check)
    if (
        out_magic != b"PDD1"
        or version != 1
        or out_w != width
        or out_h != height
        or out_count != frame_count
        or fps_num != fps
        or fps_den != 1
    ):
        raise SystemExit("generated PDD1 header verification failed")

    print(
        f"PDD1 startup intro: {width}x{height}, {frame_count} frames @ {fps} fps, "
        f"{len(packed)} -> {len(output)} bytes"
    )


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Convert the verified Wii U F3I2 PDD intro to embedded PDD1"
    )
    parser.add_argument("source", type=Path)
    parser.add_argument("target", type=Path)
    args = parser.parse_args()
    convert(args.source, args.target)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
