#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT="$ROOT/.wiiu-artwork"

for tool in ffmpeg ffprobe python3; do
  command -v "$tool" >/dev/null 2>&1 || {
    echo "$tool is required to prepare Wii U channel artwork." >&2
    exit 1
  }
done

for asset in icon.jpg boot-tv.jpg boot-drc.jpg; do
  [[ -s "$ROOT/$asset" ]] || {
    echo "Missing source artwork: $asset" >&2
    exit 1
  }
done

rm -rf "$OUT"
mkdir -p "$OUT"

probe_source() {
  local source="$1"
  local dimensions
  dimensions="$(ffprobe -v error -select_streams v:0 \
    -show_entries stream=width,height -of csv=s=x:p=0 "$source")"
  [[ -n "$dimensions" ]] || {
    echo "$(basename "$source") is not a decodable image" >&2
    exit 1
  }
  echo "$(basename "$source"): source $dimensions"
}

convert_png() {
  local source="$1"
  local output="$2"
  local width="$3"
  local height="$4"

  ffmpeg -hide_banner -loglevel error -y \
    -i "$source" \
    -vf "scale=${width}:${height}:flags=lanczos,format=rgb24" \
    -frames:v 1 "$OUT/$output"

  local dimensions
  dimensions="$(ffprobe -v error -select_streams v:0 \
    -show_entries stream=width,height -of csv=s=x:p=0 "$OUT/$output")"
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

# Historical channel artwork is intentionally kept in its original source
# resolution. The Wii U output sizes are enforced only on the generated PNGs.
probe_source "$ROOT/icon.jpg"
probe_source "$ROOT/boot-tv.jpg"
probe_source "$ROOT/boot-drc.jpg"

convert_png "$ROOT/icon.jpg" icon.png 128 128
convert_png "$ROOT/boot-tv.jpg" boot-tv.png 1280 720
convert_png "$ROOT/boot-drc.jpg" boot-drc.png 854 480

echo "Prepared Wii U artwork in $OUT"
