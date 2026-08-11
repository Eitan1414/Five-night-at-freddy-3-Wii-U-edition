#!/usr/bin/env python3
"""Extract named FNaF 3 secret-minigame instances from a Clickteam MFA.

This is intentionally a small forensic reader rather than a complete MFA
implementation. It locates the six known 3072x2304 frames, identifies object
info records by their MFA string header, finds the fixed-size instance table
immediately before the frame event sheet, and emits named world coordinates.
"""

from __future__ import annotations

import argparse
import json
import struct
from pathlib import Path

FRAME_NAMES = ("BB", "Mangle", "Toy Chica", "GFreddy", "RWQFSFASXC", "Marion")


def i16(data: bytes, offset: int) -> int:
    return struct.unpack_from("<h", data, offset)[0]


def i32(data: bytes, offset: int) -> int:
    return struct.unpack_from("<i", data, offset)[0]


def u32(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def find_frames(data: bytes) -> list[tuple[str, int, int]]:
    candidates: list[tuple[int, str, int, int]] = []
    pos = 0
    while True:
        marker = data.find(b"\x00\x80", pos)
        if marker < 0:
            break
        pos = marker + 2
        if marker < 6:
            continue
        length = i16(data, marker - 2)
        if not 1 <= length <= 80:
            continue
        string_start = marker + 2
        string_end = string_start + length * 2
        if string_end + 8 > len(data):
            continue
        try:
            name = data[string_start:string_end].decode("utf-16le")
        except UnicodeDecodeError:
            continue
        size_x, size_y = struct.unpack_from("<ii", data, string_end)
        if name in FRAME_NAMES and size_x == 3072 and size_y == 2304:
            candidates.append((marker - 6, name, size_x, size_y))

    candidates.sort()
    frames: list[tuple[str, int, int]] = []
    for index, (start, name, _, _) in enumerate(candidates):
        end = candidates[index + 1][0] if index + 1 < len(candidates) else len(data)
        frames.append((name, start, end))
    return frames


def scan_object_infos(data: bytes, start: int, end: int) -> dict[int, dict[str, object]]:
    result: dict[int, dict[str, object]] = {}
    pos = start
    while True:
        marker = data.find(b"\x00\x80", pos, end)
        if marker < 0:
            break
        pos = marker + 2
        if marker < 10:
            continue
        length = i16(data, marker - 2)
        if not 1 <= length <= 100:
            continue
        string_start = marker + 2
        string_end = string_start + length * 2
        if string_end + 8 > end:
            continue
        try:
            name = data[string_start:string_end].decode("utf-16le")
        except UnicodeDecodeError:
            continue
        if not name or any(ord(ch) < 32 for ch in name):
            continue
        object_type, handle = struct.unpack_from("<ii", data, marker - 10)
        if not ((0 <= object_type <= 7) or (32 <= object_type <= 1000)):
            continue
        if not 0 <= handle < 10000:
            continue
        transparent, ink = struct.unpack_from("<ii", data, string_end)
        if not (-10 <= transparent <= 10 and -1000 <= ink <= 10000):
            continue
        result[handle] = {"name": name, "object_type": object_type}
    return result


def find_instances(data: bytes, start: int, end: int) -> list[dict[str, int]]:
    event_marker = data.find(b"Evts", start, end)
    if event_marker < 0:
        raise ValueError("frame event sheet not found")
    event_start = event_marker - 4

    for count in range(1, 5000):
        count_offset = event_start - 4 - 32 * count
        if count_offset < start:
            break
        if i32(data, count_offset) != count:
            continue
        instances: list[dict[str, int]] = []
        valid = True
        for index in range(count):
            offset = count_offset + 4 + 32 * index
            x, y, layer, handle = struct.unpack_from("<iiIi", data, offset)
            flags, instance = struct.unpack_from("<hh", data, offset + 16)
            parent_type, item_handle, parent_handle = struct.unpack_from("<IIi", data, offset + 20)
            if not (-100000 < x < 100000 and -100000 < y < 100000 and layer < 100):
                valid = False
                break
            instances.append(
                {
                    "x": x,
                    "y": y,
                    "layer": layer,
                    "handle": handle,
                    "flags": flags,
                    "instance": instance,
                    "parent_type": parent_type,
                    "item_handle": item_handle,
                    "parent_handle": parent_handle,
                }
            )
        if valid:
            return instances
    raise ValueError("instance table not found")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("mfa", type=Path)
    parser.add_argument("output", type=Path, nargs="?")
    args = parser.parse_args()

    data = args.mfa.read_bytes()
    payload: dict[str, object] = {"source": args.mfa.name, "frames": {}}
    frames = find_frames(data)
    for index, (name, start, end) in enumerate(frames):
        if index + 1 < len(frames):
            end = frames[index + 1][1]
        infos = scan_object_infos(data, start, end)
        instances = find_instances(data, start, end)
        named = []
        for instance in instances:
            info = infos.get(instance["item_handle"])
            if info is None:
                continue
            named.append({**instance, **info})
        payload["frames"][name] = {
            "size": [3072, 2304],
            "named_instances": named,
        }

    text = json.dumps(payload, indent=2, ensure_ascii=False) + "\n"
    if args.output:
        args.output.write_text(text, encoding="utf-8")
    else:
        print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
