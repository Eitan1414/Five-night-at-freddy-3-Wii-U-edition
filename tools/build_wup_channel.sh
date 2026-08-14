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
printf '%s\n' "Five Nights at Freddy's 3 - Wii U Edition" > "$INPUT_DIR/content/channel.txt"

python3 - "$INPUT_DIR" <<'PY'
from pathlib import Path
from PIL import Image, ImageOps, UnidentifiedImageError
import struct
import sys
import xml.etree.ElementTree as ET

root = Path(sys.argv[1])
code = root / "code"
meta = root / "meta"

# Real Wii U titles use the current menu metadata schema.  The previous WUP
# accidentally shipped a v1 meta.xml; the console could install the title but
# then treated its menu metadata as invalid ("???" / question-mark icon).
meta_xml = meta / "meta.xml"
tree = ET.parse(meta_xml)
menu = tree.getroot()
version = menu.find("version")
if version is None:
    raise SystemExit("error: meta.xml is missing <version>")
version.text = "33"
for required_tag in ("longname_en", "longname_fr", "shortname_en", "shortname_fr", "publisher_en"):
    node = menu.find(required_tag)
    if node is None or not (node.text or "").strip():
        raise SystemExit(f"error: meta.xml is missing a usable <{required_tag}>")
tree.write(meta_xml, encoding="utf-8", xml_declaration=True)

# Refuse to package obsolete/incomplete code metadata.
def require_xml(path, expected_version, required_tags):
    doc = ET.parse(path).getroot()
    node = doc.find("version")
    if node is None or node.text != str(expected_version):
        raise SystemExit(f"error: {path.name} must use schema version {expected_version}")
    for tag in required_tags:
        value = doc.find(tag)
        if value is None or not (value.text or "").strip():
            raise SystemExit(f"error: {path.name} is missing <{tag}>")

require_xml(code / "app.xml", 16, ("title_id", "sdk_version", "common_id"))
require_xml(
    code / "cos.xml",
    20,
    ("argstr", "overlay_arena", "default_stack1_size", "num_codearea_heap_blocks", "num_workarea_heap_blocks"),
)

try:
    resampling = Image.Resampling.LANCZOS
except AttributeError:
    resampling = Image.LANCZOS


def open_rgb(source):
    try:
        with Image.open(source) as image:
            image.load()
            return image.convert("RGB")
    except (FileNotFoundError, UnidentifiedImageError, OSError) as exc:
        print(f"warning: cannot decode {source}: {exc}", file=sys.stderr)
        return None


def convert(source, target, size, mode="RGB", fallback_to_icon=False):
    image = open_rgb(source)
    if image is not None:
        image = ImageOps.fit(image, size, method=resampling, centering=(0.5, 0.5))
    else:
        # The old boot-tv.jpg / boot-drc.jpg files in this project are not
        # decodable JPEG streams. WUP metadata still requires valid TGA splash
        # images, so generate a deterministic fallback instead of aborting the
        # whole Channel package.
        image = Image.new("RGB", size, (0, 0, 0))
        if fallback_to_icon:
            icon = open_rgb("icon.jpg")
            if icon is not None:
                max_side = min(size) // 3
                icon.thumbnail((max_side, max_side), resampling)
                x = (size[0] - icon.width) // 2
                y = (size[1] - icon.height) // 2
                image.paste(icon, (x, y))

    # Wii U menu metadata is strict: iconTex.tga is 128x128x32 while TV/DRC
    # splash TGAs are 24-bit.  Pillow's default RGB TGA made the old icon 24-bit.
    image = image.convert(mode)
    image.save(meta / target, format="TGA")


convert("icon.jpg", "iconTex.tga", (128, 128), mode="RGBA")
convert("boot-tv.jpg", "bootTvTex.tga", (1280, 720), mode="RGB", fallback_to_icon=True)
convert("boot-drc.jpg", "bootDrcTex.tga", (854, 480), mode="RGB", fallback_to_icon=True)


def verify_tga(name, width, height, depth):
    path = meta / name
    data = path.read_bytes()
    if len(data) < 44:
        raise SystemExit(f"error: {name} is too small")
    if data[2] != 2:
        raise SystemExit(f"error: {name} must be an uncompressed true-color TGA")
    actual_width, actual_height = struct.unpack_from("<HH", data, 12)
    actual_depth = data[16]
    if (actual_width, actual_height, actual_depth) != (width, height, depth):
        raise SystemExit(
            f"error: {name} is {actual_width}x{actual_height}x{actual_depth}; "
            f"expected {width}x{height}x{depth}"
        )
    if not data.endswith(b"TRUEVISION-XFILE.\x00"):
        raise SystemExit(f"error: {name} is missing the TGA 2.0 footer")
    print(f"WUP TGA OK: {name} {width}x{height}x{depth}")


verify_tga("iconTex.tga", 128, 128, 32)
verify_tga("bootTvTex.tga", 1280, 720, 24)
verify_tga("bootDrcTex.tga", 854, 480, 24)
print("WUP metadata schema OK: meta=33 app=16 cos=20")
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
