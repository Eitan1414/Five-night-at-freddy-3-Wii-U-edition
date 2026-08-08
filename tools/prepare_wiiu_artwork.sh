#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT="$ROOT/.wiiu-artwork"
ASSETS="$ROOT/assets/wiiu"

for tool in ffmpeg ffprobe python3 sha256sum; do
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

  # Each repository chunk was encoded independently.  Decoding the concatenated
  # base64 text is invalid as soon as an intermediate chunk contains '=' padding.
  # Decode every chunk on its own, then concatenate the resulting binary bytes.
  python3 - "$OUT/$output" "${parts[@]}" <<'PY'
import base64
import re
import sys
from pathlib import Path

out = Path(sys.argv[1])
payload = bytearray()
for name in sys.argv[2:]:
    path = Path(name)
    raw = path.read_bytes()
    cleaned = re.sub(rb'[^A-Za-z0-9+/=]', b'', raw)
    if not cleaned:
        raise SystemExit(f'{path}: empty base64 chunk')
    cleaned += b'=' * (-len(cleaned) % 4)
    try:
        decoded = base64.b64decode(cleaned, validate=False)
    except Exception as exc:
        raise SystemExit(f'{path}: base64 decode failed: {exc}')
    if not decoded:
        raise SystemExit(f'{path}: decoded to zero bytes')
    payload.extend(decoded)
    print(f'{path.name}: {len(decoded)} decoded bytes')

out.write_bytes(payload)
print(f'{out.name}: {len(payload)} total decoded bytes')
PY

  # This prevents a tolerant decoder from silently accepting missing or altered
  # data. Only the exact known-good image payload is allowed past this point.
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

rebuild_jpeg boot-tv-q65 boot-tv-clean.jpg \
  7c19a128361a6e8b8cd735967c184080c452aa0f9ab9f9ad9ea11973ddefda92 1280x720
rebuild_jpeg boot-drc-q65 boot-drc-clean.jpg \
  e18bc31fa8b20e381b72b6e3e87f358b72221cfdb75274d609c084f4f8ef7c1e 854x480

convert_png "$ROOT/icon.jpg" icon.png 128 128
convert_png "$OUT/boot-tv-clean.jpg" boot-tv.png 1280 720
convert_png "$OUT/boot-drc-clean.jpg" boot-drc.png 854 480
rm -f "$OUT/boot-tv-clean.jpg" "$OUT/boot-drc-clean.jpg"

echo "Prepared Wii U artwork in $OUT"
