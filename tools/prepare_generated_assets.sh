#!/bin/sh
set -eu

SOURCE_ARCHIVE="source/generated/camera_springtrap_textures.c.xz.b64"
OUTPUT_SOURCE="source/camera_springtrap_textures.c"

if command -v xz >/dev/null 2>&1; then
    base64 -d "$SOURCE_ARCHIVE" | xz -dc > "$OUTPUT_SOURCE"
elif command -v python3 >/dev/null 2>&1; then
    python3 - "$SOURCE_ARCHIVE" "$OUTPUT_SOURCE" <<'PY'
import base64
import lzma
import pathlib
import sys

source = pathlib.Path(sys.argv[1]).read_bytes()
pathlib.Path(sys.argv[2]).write_bytes(lzma.decompress(base64.b64decode(source)))
PY
else
    echo "xz or python3 is required to prepare generated assets" >&2
    exit 1
fi
