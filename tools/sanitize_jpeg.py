#!/usr/bin/env python3
"""Repair metadata/padding damage in JPEG artwork without re-encoding it."""
from __future__ import annotations

import argparse
from pathlib import Path

SOI = b"\xff\xd8"
SOS = 0xDA
EOI = 0xD9
TEM = 0x01
APP_FIRST = 0xE0
APP_LAST = 0xEF
COM = 0xFE
RST_FIRST = 0xD0
RST_LAST = 0xD7


def next_marker(data: bytes, pos: int) -> int:
    """Find the next plausible JPEG marker before the entropy-coded scan."""
    while True:
        pos = data.find(b"\xff", pos)
        if pos < 0:
            return -1
        j = pos + 1
        while j < len(data) and data[j] == 0xFF:
            j += 1
        if j >= len(data):
            return -1
        marker = data[j]
        # 00 is byte stuffing, and FF is fill. Neither begins a segment here.
        if marker not in (0x00, 0xFF):
            return pos
        pos = j + 1


def sanitize(data: bytes) -> tuple[bytes, int, int, bool]:
    if len(data) < 4 or not data.startswith(SOI):
        raise ValueError("not a JPEG stream")

    out = bytearray(SOI)
    pos = 2
    removed = 0
    resynced = 0
    appended_eoi = False

    while pos < len(data):
        if data[pos] != 0xFF:
            recovered = next_marker(data, pos + 1)
            if recovered < 0:
                raise ValueError(f"could not resync after offset {pos}")
            resynced += recovered - pos
            pos = recovered

        marker_start = pos
        while pos < len(data) and data[pos] == 0xFF:
            pos += 1
        if pos >= len(data):
            raise ValueError("truncated JPEG marker")

        marker = data[pos]
        pos += 1

        if marker == SOS:
            if pos + 2 > len(data):
                raise ValueError("truncated SOS")
            seg_len = int.from_bytes(data[pos:pos + 2], "big")
            if seg_len < 2 or pos + seg_len > len(data):
                raise ValueError("invalid SOS length")
            # Once SOS begins, preserve all entropy-coded data exactly as-is.
            out.extend(data[marker_start:])
            if not out.endswith(b"\xff\xd9"):
                # The supplied splash JPEGs have a complete entropy-coded scan
                # but lost their final EOI marker. JPEG decoders accept the
                # stream once the standard FF D9 terminator is restored.
                out.extend(b"\xff\xd9")
                appended_eoi = True
            return bytes(out), removed, resynced, appended_eoi

        if marker == EOI:
            out.extend(b"\xff\xd9")
            return bytes(out), removed, resynced, appended_eoi

        if marker == TEM or RST_FIRST <= marker <= RST_LAST:
            out.extend(data[marker_start:pos])
            continue

        if pos + 2 > len(data):
            recovered = next_marker(data, marker_start + 2)
            if recovered < 0:
                raise ValueError("truncated segment length")
            resynced += recovered - marker_start
            pos = recovered
            continue

        seg_len = int.from_bytes(data[pos:pos + 2], "big")
        seg_end = pos + seg_len
        if seg_len < 2 or seg_end > len(data):
            # A damaged length field should not prevent recovery of later
            # structural markers. Drop this damaged segment and resync.
            recovered = next_marker(data, marker_start + 2)
            if recovered < 0:
                raise ValueError(f"cannot recover marker FF{marker:02X}")
            removed += recovered - marker_start
            pos = recovered
            continue

        if APP_FIRST <= marker <= APP_LAST or marker == COM:
            removed += seg_end - marker_start
        else:
            out.extend(data[marker_start:seg_end])
        pos = seg_end

    raise ValueError("JPEG ended before SOS/EOI")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("destination", type=Path)
    args = parser.parse_args()

    clean, removed, resynced, appended_eoi = sanitize(args.source.read_bytes())
    args.destination.parent.mkdir(parents=True, exist_ok=True)
    args.destination.write_bytes(clean)
    print(
        f"sanitized {args.source.name}: {len(clean)} bytes "
        f"(removed {removed}, resynced {resynced}, "
        f"EOI restored={appended_eoi})"
    )


if __name__ == "__main__":
    main()
