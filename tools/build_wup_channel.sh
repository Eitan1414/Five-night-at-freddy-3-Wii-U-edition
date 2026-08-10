#!/bin/sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$ROOT_DIR"

if [ ! -f fnaf3-wiiu.rpx ]; then
  echo "error: fnaf3-wiiu.rpx is missing; run make first" >&2
  exit 1
fi

if [ -z "${WIIU_COMMON_KEY:-}" ]; then
  echo "error: WIIU_COMMON_KEY is not set" >&2
  exit 1
fi

case "$WIIU_COMMON_KEY" in
  *[!0-9A-Fa-f]*|'')
    echo "error: WIIU_COMMON_KEY must be a 32-character hexadecimal value" >&2
    exit 1
    ;;
esac

if [ "${#WIIU_COMMON_KEY}" -ne 32 ]; then
  echo "error: WIIU_COMMON_KEY must be exactly 32 hexadecimal characters" >&2
  exit 1
fi

INPUT_DIR="$ROOT_DIR/build/wup-channel"
OUTPUT_DIR="$ROOT_DIR/dist/fnaf3-wiiu-wup"
NUSPACKER_JAR=${NUSPACKER_JAR:-"$ROOT_DIR/build/NUSPacker.jar"}

rm -rf "$INPUT_DIR" "$OUTPUT_DIR"
mkdir -p "$INPUT_DIR/code" "$INPUT_DIR/content" "$INPUT_DIR/meta" "$OUTPUT_DIR"

cp fnaf3-wiiu.rpx "$INPUT_DIR/code/fnaf3-wiiu.rpx"
cp wup/app.xml "$INPUT_DIR/code/app.xml"
cp wup/cos.xml "$INPUT_DIR/code/cos.xml"
cp wup/meta.xml "$INPUT_DIR/meta/meta.xml"

# NUSPacker requires the content directory to contain at least one file.
printf '%s\n' 'Five Nights at Freddy'''s 3 - Wii U Edition' > "$INPUT_DIR/content/channel.txt"

python3 - "$INPUT_DIR/meta" <<'PY'
from pathlib import Path
from PIL import Image, ImageOps
import sys

meta = Path(sys.argv[1])

try:
    resampling = Image.Resampling.LANCZOS
except AttributeError:
    resampling = Image.LANCZOS


def convert(source, target, size):
    with Image.open(source) as image:
        image = image.convert("RGB")
        image = ImageOps.fit(image, size, method=resampling, centering=(0.5, 0.5))
        image.save(meta / target, format="TGA")

convert("icon.jpg", "iconTex.tga", (128, 128))
convert("boot-tv.jpg", "bootTvTex.tga", (1280, 720))
convert("boot-drc.jpg", "bootDrcTex.tga", (854, 480))
PY

if [ ! -f "$NUSPACKER_JAR" ]; then
  mkdir -p "$(dirname "$NUSPACKER_JAR")"
  curl -L --fail --retry 3 \
    https://raw.githubusercontent.com/Maschell/nuspacker/master/NUSPacker.jar \
    -o "$NUSPACKER_JAR"
fi

java -jar "$NUSPACKER_JAR" \
  -in "$INPUT_DIR" \
  -out "$OUTPUT_DIR" \
  -encryptKeyWith "$WIIU_COMMON_KEY"

APP_COUNT=$(find "$OUTPUT_DIR" -maxdepth 1 -type f -name '*.app' | wc -l)
H3_COUNT=$(find "$OUTPUT_DIR" -maxdepth 1 -type f -name '*.h3' | wc -l)

if [ "$APP_COUNT" -lt 1 ] || [ "$H3_COUNT" -lt 1 ]; then
  echo "error: NUSPacker did not create both .app and .h3 files" >&2
  exit 1
fi

for required in title.tmd title.tik title.cert; do
  if [ ! -f "$OUTPUT_DIR/$required" ]; then
    echo "error: missing $required in WUP output" >&2
    exit 1
  fi
done

echo "WUP channel ready: $OUTPUT_DIR"
find "$OUTPUT_DIR" -maxdepth 1 -type f -printf '%f\n' | sort
