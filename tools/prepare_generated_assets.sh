#!/bin/sh
set -eu

restore_xz_base64() {
    source_pattern="$1"
    output="$2"
    # shellcheck disable=SC2086
    cat $source_pattern | base64 -d | xz -dc > "$output"
}

restore_xz_base64 \
    "source/generated/camera_springtrap_textures.c.xz.b64" \
    "source/camera_springtrap_textures.c"
restore_xz_base64 \
    "source/generated/phantom_assets_min.c.xz.b64" \
    "source/phantom_assets.c"

if ! command -v ffmpeg >/dev/null 2>&1; then
    echo "ffmpeg is required to prepare audio assets" >&2
    exit 1
fi

mkdir -p assets data/audio
rm -rf assets/audio_low12 assets/audio_phantoms_low data/audio
mkdir -p data/audio

cat source/generated/audio_low12.tar.xz.b64.* \
    | base64 -d | tar -xJf - -C assets
cat source/generated/audio_phantoms.tar.xz.b64.part* \
    | base64 -d | tar -xJf - -C assets

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

convert_audio assets/audio_phantoms_low/garble1.mp3 data/audio/garble1.bin
convert_audio assets/audio_phantoms_low/mask.mp3 data/audio/mask.bin
convert_audio assets/audio_phantoms_low/echo1.mp3 data/audio/echo1.bin
convert_audio assets/audio_phantoms_low/echo3b.mp3 data/audio/echo3b.bin
convert_audio assets/audio_phantoms_low/echo4b.mp3 data/audio/echo4b.bin

rm -rf assets/audio_low12 assets/audio_phantoms_low
