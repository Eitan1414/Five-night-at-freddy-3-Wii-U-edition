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

for source in icon.jpg boot-tv.jpg boot-drc.jpg; do
  [[ -s "$ROOT/$source" ]] || {
    echo "Missing source artwork: $source" >&2
    exit 1
  }
done

mkdir -p "$OUT"

convert_png() {
  local source="$1"
  local output="$2"
  local width="$3"
  local height="$4"

  ffmpeg -hide_banner -loglevel error -y \
    -i "$ROOT/$source" \
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

convert_png icon.jpg icon.png 128 128
convert_png boot-tv.jpg boot-tv.png 1280 720
convert_png boot-drc.jpg boot-drc.png 854 480

echo "Prepared Wii U artwork in $OUT"
