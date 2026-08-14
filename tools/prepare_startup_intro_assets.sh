#!/bin/sh
set -eu

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
WORK="${TMPDIR:-/tmp}/fnaf3-startup-intro-$$"
DECODED="$WORK/decoded"

cleanup() {
    rm -rf "$WORK"
}
trap cleanup EXIT INT TERM

mkdir -p "$WORK" "$ROOT/data"

sh "$ROOT/tools/prepare_intro_assets.sh" "$DECODED"
python3 "$ROOT/tools/convert_f3i2_to_pdd1.py" \
    "$DECODED/intro.f3v" \
    "$ROOT/data/startup_intro_video.bin"
cp "$DECODED/intro_audio.bin" "$ROOT/data/startup_intro_audio.bin"

test -s "$ROOT/data/startup_intro_video.bin"
test -s "$ROOT/data/startup_intro_audio.bin"

python3 - "$ROOT/data/startup_intro_video.bin" "$ROOT/data/startup_intro_audio.bin" <<'PY'
from pathlib import Path
import struct
import sys

video = Path(sys.argv[1]).read_bytes()
audio = Path(sys.argv[2]).read_bytes()
if len(video) < 20 + 1024 or video[:4] != b"PDD1":
    raise SystemExit("generated startup video is not a valid PDD1 stream")
version, width, height, frames, fps_num, fps_den = struct.unpack_from(">HHHHHH", video, 4)
if version != 1 or width <= 0 or height <= 0 or frames <= 0 or fps_num <= 0 or fps_den <= 0:
    raise SystemExit("generated startup video has invalid metadata")
if len(audio) < 2 or len(audio) % 2:
    raise SystemExit("generated startup audio is not aligned 16-bit PCM")
print(f"Embedded PDD intro ready: {width}x{height}, {frames} frames, {fps_num}/{fps_den} fps; audio={len(audio)} bytes")
PY
