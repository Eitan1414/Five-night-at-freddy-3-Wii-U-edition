#!/usr/bin/env python3
"""Strip optional JPEG APP/COM metadata while preserving encoded image data.

Some of the original Wii U artwork JPEGs contain APP metadata that strict
homebrew image loaders reject even though the actual JPEG scan is intact.
This creates a metadata-free JPEG without re-encoding the image.
"""
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


def sanitize(data: bytes) -> bytes:
    if len(data) < 4 or not data.startswith(SOI):
        raise ValueError("not a JPEG stream")

    out = bytearray(SOI)
    pos = 2
    removed = 0

    while pos < len(data):
        marker_start = pos
        if data[pos] != 0xFF:
            raise ValueError(f"expected JPEG marker at offset {pos}")

        while pos < len(data) and data[pos] == 0xFF:
            pos += 1
        if pos >= len(data):
            raise ValueError("truncated JPEG marker")

        marker = data[pos]
        pos += 1

        if marker == 0x00:
            raise ValueError("unexpected stuffed byte before SOS")

        if marker == SOS:
            if pos + 2 > len(data):
                raise ValueError("truncated SOS")
            seg_len = int.from_bytes(data[pos:pos + 2], "big")
            if seg_len < 2 or pos + seg_len > len(data):
                raise ValueError("invalid SOS length")
            # Preserve SOS header and the entire entropy-coded scan verbatim.
            out.extend(data[marker_start:])
            if not out.endswith(b"\xff\xd9"):
                raise ValueError("JPEG scan has no EOI marker")
            return bytes(out)

        if marker == EOI:
            out.extend(b"\xff\xd9")
            return bytes(out)

        # Standalone markers have no length field.
        if marker == TEM or RST_FIRST <= marker <= RST_LAST:
            out.extend(data[marker_start:pos])
            continue

        if pos + 2 > len(data):
            raise ValueError("truncated segment length")
        seg_len = int.from_bytes(data[pos:pos + 2], "big")
        if seg_len < 2:
            raise ValueError(f"invalid segment length for marker FF{marker:02X}")
        seg_end = pos + seg_len
        if seg_end > len(data):
            raise ValueError(f"segment FF{marker:02X} overruns file")

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

    clean = sanitize(args.source.read_bytes())
    args.destination.parent.mkdir(parents=True, exist_ok=True)
    args.destination.write_bytes(clean)
    print(f"sanitized {args.source.name}: {len(clean)} bytes")


if __name__ == "__main__":
    main()
