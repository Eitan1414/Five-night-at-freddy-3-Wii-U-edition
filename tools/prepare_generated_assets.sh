#!/bin/sh
set -eu

SOURCE_ARCHIVE="source/generated/camera_springtrap_textures.c.xz.b64"
OUTPUT_SOURCE="source/camera_springtrap_textures.c"

if command -v xz >/dev/null 2>&1; then
    base64 -d "$SOURCE_ARCHIVE" | xz -dc > "$OUTPUT_SOURCE"
elif command -v python3 >/dev/null 2>&1; then
    python3 - "$SOURCE_ARCHIVE" "$OUTPUT_SOURCE" <<'PY'
import base64
import lzma
import pathlib
import sys

source = pathlib.Path(sys.argv[1]).read_bytes()
pathlib.Path(sys.argv[2]).write_bytes(lzma.decompress(base64.b64decode(source)))
PY
else
    echo "xz or python3 is required to prepare generated assets" >&2
    exit 1
fi

if ! command -v ffmpeg >/dev/null 2>&1; then
    echo "ffmpeg is required to prepare audio assets" >&2
    exit 1
fi

mkdir -p assets data
rm -rf assets/audio_low12 data/audio
cat source/generated/audio_low12.tar.xz.b64.* | base64 -d | tar -xJf - -C assets
mkdir -p data/audio

convert_audio() {
    input="$1"
    output="$2"
    ffmpeg -hide_banner -loglevel error -y \
        -i "$input" -ar 16000 -ac 1 -f s16be "$output"
}

convert_audio assets/audio_low12/vent_quiet1.mp3 data/audio/vent_quiet1.bin
convert_audio assets/audio_low12/vent_quiet2.mp3 data/audio/vent_quiet2.bin
convert_audio assets/audio_low12/vent_closer1.mp3 data/audio/vent_closer1.bin
convert_audio assets/audio_low12/vent_louder2.mp3 data/audio/vent_louder2.bin
convert_audio assets/audio_low12/alarm.mp3 data/audio/alarm.bin
convert_audio assets/audio_low12/breathing.mp3 data/audio/breathing.bin
convert_audio assets/audio_low12/wait.mp3 data/audio/wait.bin
convert_audio assets/audio_low12/static_sound.mp3 data/audio/static_sound.bin
convert_audio assets/audio_low12/scream3.mp3 data/audio/scream3.bin
