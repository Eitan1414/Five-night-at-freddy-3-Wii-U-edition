#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT="$ROOT/.wiiu-artwork"
ASSETS="$ROOT/assets/wiiu"

for tool in ffmpeg ffprobe python3 base64 sha256sum; do
  command -v "$tool" >/dev/null 2>&1 || {
    echo "$tool is required to prepare Wii U channel artwork." >&2
    exit 1
  }
done

[[ -s "$ROOT/icon.jpg" ]] || {
  echo 'Missing source artwork: icon.jpg' >&2
  exit 1
}

rm -rf "$OUT"
mkdir -p "$OUT"

rebuild_jpeg() {
  local prefix="$1"
  local output="$2"
  local expected_sha="$3"
  local expected_dims="$4"

  local parts=("$ASSETS/${prefix}.b64.part"*)
  [[ -e "${parts[0]}" ]] || {
    echo "Missing encoded artwork chunks for $prefix" >&2
    exit 1
  }

  # Some historical chunks picked up non-base64 transport bytes.  Ignore only
  # those bytes here, then verify the exact decoded payload with SHA-256 below.
  # This keeps the build strict: any missing/altered image data still fails.
  cat "${parts[@]}" | tr -d '\r\n' | base64 --decode --ignore-garbage > "$OUT/$output"
  echo "$expected_sha  $OUT/$output" | sha256sum -c -

  local dimensions
  dimensions="$(ffprobe -v error -select_streams v:0 \
    -show_entries stream=width,height -of csv=s=x:p=0 "$OUT/$output")"
  [[ "$dimensions" == "$expected_dims" ]] || {
    echo "$output has invalid dimensions: $dimensions (expected $expected_dims)" >&2
    exit 1
  }
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

# The original boot JPG files in the historical branch are truncated. These
# chunk files are clean, visually identical derivatives of the supplied PNG
# artwork. They are ASCII on purpose so GitHub preserves the bytes reliably.
rebuild_jpeg boot-tv-q65 boot-tv-clean.jpg \
  7c19a128361a6e8b8cd735967c184080c452aa0f9ab9f9ad9ea11973ddefda92 1280x720
rebuild_jpeg boot-drc-q65 boot-drc-clean.jpg \
  e18bc31fa8b20e381b72b6e3e87f358b72221cfdb75274d609c084f4f8ef7c1e 854x480

convert_png "$ROOT/icon.jpg" icon.png 128 128
convert_png "$OUT/boot-tv-clean.jpg" boot-tv.png 1280 720
convert_png "$OUT/boot-drc-clean.jpg" boot-drc.png 854 480
rm -f "$OUT/boot-tv-clean.jpg" "$OUT/boot-drc-clean.jpg"

echo "Prepared Wii U artwork in $OUT"
