#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CHANNEL="${1:-$ROOT/build-wup/channel}"
OUT="${2:-$ROOT/build-wup/install}"
NUSPACKER_URL='https://raw.githubusercontent.com/ihaveamac/nuspacker/797b84af32e5bb0c353577a7a3f67e236f7b7ee3/NUSPacker.jar'
NUSPACKER_GIT_BLOB='8418addc529f63a2df855a49b7201ec27e637a18'
KEY="${WIIU_COMMON_KEY:-}"

[[ "$KEY" =~ ^[0-9A-Fa-f]{32}$ ]] || {
  echo 'WIIU_COMMON_KEY must contain 32 hexadecimal characters.' >&2
  exit 1
}

[[ -d "$CHANNEL/code" && -d "$CHANNEL/content" && -d "$CHANNEL/meta" ]] || {
  echo 'Invalid Wii U channel tree.' >&2
  exit 1
}

for required in code/app.xml code/cos.xml code/fnaf3-wiiu.rpx meta/meta.xml meta/iconTex.tga meta/bootTvTex.tga meta/bootDrcTex.tga; do
  [[ -s "$CHANNEL/$required" ]] || {
    echo "Missing channel file: $required" >&2
    exit 1
  }
done

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
umask 077
printf '%s' "$KEY" > "$TMP/encryptKeyWith"
unset KEY WIIU_COMMON_KEY

curl -fsSL "$NUSPACKER_URL" -o "$TMP/NUSPacker.jar"
[[ "$(git hash-object "$TMP/NUSPacker.jar")" == "$NUSPACKER_GIT_BLOB" ]] || {
  echo 'NUSPacker integrity check failed.' >&2
  exit 1
}

rm -rf "$OUT"
mkdir -p "$OUT"
CHANNEL="$(cd "$CHANNEL" && pwd)"
OUT="$(cd "$OUT" && pwd)"

(
  cd "$TMP"
  java -jar NUSPacker.jar -in "$CHANNEL" -out "$OUT" 2>&1 | \
    sed -E 's/^((Encryption key|Encrypt key with)[[:space:]]*:[[:space:]]*)[0-9A-Fa-f]{32}/\1[REDACTED]/'
)

test -n "$(find "$OUT" -maxdepth 1 -name '*.app' -print -quit)"
test -n "$(find "$OUT" -maxdepth 1 -name '*.h3' -print -quit)"
for file in title.tik title.tmd title.cert; do test -f "$OUT/$file"; done

cat > "$OUT/WUP_PACKAGE_INFO.txt" <<'EOF'
Five Nights at Freddy's 3 - Wii U Edition
Title ID: 00050000464E3355
Product code: WUP-N-FN3U
The Wii U common key is not included in this package.
EOF

echo "WUP package created in $OUT"
