#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT="$ROOT/.wiiu-artwork"

command -v ffmpeg >/dev/null 2>&1 || {
  echo 'ffmpeg is required to prepare Wii U channel artwork.' >&2
  exit 1
}
command -v ffprobe >/dev/null 2>&1 || {
  echo 'ffprobe is required to validate Wii U channel artwork.' >&2
  exit 1
}
command -v python3 >/dev/null 2>&1 || {
  echo 'python3 is required to prepare Wii U channel artwork.' >&2
  exit 1
}

for source in icon.jpg boot-tv.jpg boot-drc.jpg; do
  [[ -s "$ROOT/$source" ]] || {
    echo "Missing source artwork: $source" >&2
    exit 1
  }
done

rm -rf "$OUT"
mkdir -p "$OUT"

sanitize_jpeg() {
  local source="$1"
  local output="$2"
  python3 "$ROOT/tools/sanitize_jpeg.py" "$ROOT/$source" "$OUT/$output"
}

convert_png() {
  local source_path="$1"
  local output="$2"
  local width="$3"
  local height="$4"

  ffmpeg -hide_banner -loglevel error -y \
    -i "$source_path" \
    -vf "scale=${width}:${height}:flags=lanczos,format=rgb24" \
    -frames:v 1 \
    "$OUT/$output"

  local dimensions
  dimensions="$(ffprobe -v error -select_streams v:0 \
    -show_entries stream=width,height \
    -of csv=s=x:p=0 "$OUT/$output")"
  [[ "$dimensions" == "${width}x${height}" ]] || {
    echo "$output has invalid dimensions: $dimensions" >&2
    exit 1
  }

  python3 - "$OUT/$output" <<'PY'
import sys
from pathlib import Path
p = Path(sys.argv[1])
data = p.read_bytes()
if not data.startswith(b'\x89PNG\r\n\x1a\n'):
    raise SystemExit(f'{p}: invalid PNG signature')
print(f'{p.name}: {len(data)} bytes')
PY
}

# icon.jpg is already accepted by the toolchain. The two splash JPEGs contain
# optional APP metadata that strict Wii U image loaders reject; strip only that
# metadata before decoding, leaving the encoded picture data untouched.
convert_png "$ROOT/icon.jpg" icon.png 128 128
sanitize_jpeg boot-tv.jpg boot-tv-clean.jpg
sanitize_jpeg boot-drc.jpg boot-drc-clean.jpg
convert_png "$OUT/boot-tv-clean.jpg" boot-tv.png 1280 720
convert_png "$OUT/boot-drc-clean.jpg" boot-drc.png 854 480
rm -f "$OUT/boot-tv-clean.jpg" "$OUT/boot-drc-clean.jpg"

echo "Prepared Wii U artwork in $OUT"
