#!/bin/sh
set -eu

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
OUT="${1:-$ROOT/.wup-intro}"
WORK="${TMPDIR:-/tmp}/fnaf3-intro.$$"
EXPECTED_VIDEO_SHA256="700da6cfaf73c76b140c13a372610f4479694978cd1832a547d50a6fcd72e075"
EXPECTED_AUDIO_OPUS_SHA256="dac8f09c1e9cc8fc155529eb846ec70a12502d19290577df7592f025b4bc5c10"

cleanup() { rm -rf "$WORK"; }
trap cleanup EXIT INT TERM
mkdir -p "$WORK"
rm -rf "$OUT"
mkdir -p "$OUT"

for tool in base64 ffmpeg sha256sum; do
    command -v "$tool" >/dev/null 2>&1 || {
        echo "Missing required intro tool: $tool" >&2
        exit 1
    }
done

cat "$ROOT"/assets/intro/intro.f3v.b64.* | base64 -d > "$OUT/intro.f3v"
cat "$ROOT"/assets/intro/intro_audio.opus.b64.* | base64 -d > "$WORK/intro_audio.opus"

video_sha="$(sha256sum "$OUT/intro.f3v" | awk '{print $1}')"
audio_sha="$(sha256sum "$WORK/intro_audio.opus" | awk '{print $1}')"
[ "$video_sha" = "$EXPECTED_VIDEO_SHA256" ] || {
    echo "Intro video SHA-256 mismatch: $video_sha" >&2
    exit 1
}
[ "$audio_sha" = "$EXPECTED_AUDIO_OPUS_SHA256" ] || {
    echo "Intro audio SHA-256 mismatch: $audio_sha" >&2
    exit 1
}

ffmpeg -hide_banner -loglevel error -y -i "$WORK/intro_audio.opus" \
    -ar 16000 -ac 1 -f s16be "$OUT/intro_audio.bin"

[ -s "$OUT/intro.f3v" ]
[ -s "$OUT/intro_audio.bin" ]

echo "Prepared Wii U startup intro: $(du -h "$OUT/intro.f3v" | awk '{print $1}') video + $(du -h "$OUT/intro_audio.bin" | awk '{print $1}') PCM audio"
