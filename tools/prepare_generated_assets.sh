#!/bin/sh
set -eu

restore_xz_base64() {
    source_pattern="$1"
    output="$2"
    # shellcheck disable=SC2086
    cat $source_pattern | base64 -d | xz -dc > "$output"
}

restore_extended_cameras() {
    source_pattern="$1"
    output="$2"
    temporary="${output}.xz"
    trimmed="${output}.trimmed"

    # The historical upload contains all seven camera textures, followed by
    # a duplicated Springtrap sprite section whose final XZ fragment was never
    # committed. Recover the emitted C source, then discard that incomplete
    # duplicate tail. The complete Springtrap sprites are restored separately.
    # shellcheck disable=SC2086
    cat $source_pattern | base64 -d > "$temporary"
    if ! xz -dc "$temporary" > "$output"; then
        echo "warning: recovered camera textures from truncated XZ stream" >&2
    fi
    rm -f "$temporary"

    sed '/^static const uint32_t kSpringtrap01SpritePalette/,$d' \
        "$output" > "$trimmed"
    mv "$trimmed" "$output"

    test -s "$output"
    grep -q "const TextureRle gCamera04Texture" "$output"
    grep -q "const TextureRle gCamera10Texture" "$output"
}

restore_extended_cameras \
    "source/generated/camera_extended_assets.c.xz.b64.*" \
    "source/camera_extended_assets.c"
restore_xz_base64 \
    "source/generated/camera_springtrap_textures.c.xz.b64" \
    "source/camera_springtrap_textures.c"
restore_xz_base64 \
    "source/generated/phantom_mangle_user_texture.h.xz.b64" \
    "include/assets/phantom_mangle_user_texture.h"

if ! command -v ffmpeg >/dev/null 2>&1; then
    echo "ffmpeg is required to prepare audio assets" >&2
    exit 1
fi

if ! command -v python3 >/dev/null 2>&1; then
    echo "python3 is required to prepare generated assets" >&2
    exit 1
fi

mkdir -p assets/user_visuals
cat assets/user_visuals/phantom_chica_jumpscare.png.b64.part* \
    | base64 -d > assets/user_visuals/phantom_chica_jumpscare.png

if ls source/generated/user_content.tar.xz.b64.* >/dev/null 2>&1; then
    temporary_user_content="${TMPDIR:-/tmp}/fnaf3-user-content.tar.xz"
    # shellcheck disable=SC2086
    cat source/generated/user_content.tar.xz.b64.* | base64 -d > "$temporary_user_content"
    tar -xJf "$temporary_user_content" -C .
    rm -f "$temporary_user_content"
fi

PSX_SOURCE="${FNAF3_PSX_SOURCE:-assets/Five-Night-at-Freddys-3-PSX-main}"
TIM_ROOT="$PSX_SOURCE/tim"
SCREAMER_ROOT="$TIM_ROOT/screamer"
SCREAMER_AUDIO="$PSX_SOURCE/vag/screamer.vag"
PHONE_ARCHIVE="$PSX_SOURCE/xa/inter8.zip"
USER_AUDIO_ROOT="${FNAF3_USER_AUDIO_ROOT:-assets/user_audio}"

if [ ! -d "$SCREAMER_ROOT" ] || [ ! -f "$SCREAMER_AUDIO" ]; then
    echo "Original PSX jumpscare sources not found at: $PSX_SOURCE" >&2
    echo "Set FNAF3_PSX_SOURCE to the extracted Five-Night-at-Freddys-3-PSX source." >&2
    exit 1
fi

python3 tools/convert_phantom_visuals.py \
    "$TIM_ROOT" \
    source/phantom_assets.c \
    include/assets/phantom_assets.h

python3 tools/convert_camera_springtrap.py \
    "$TIM_ROOT/camera/cams/map" \
    source/camera_springtrap_assets.c \
    include/assets/camera_springtrap_assets.h

python3 tools/convert_jumpscare_tim.py \
    "$SCREAMER_ROOT" \
    source/jumpscare_assets.c \
    include/assets/jumpscare_assets.h

python3 tools/convert_user_chica_png.py \
    assets/user_visuals/phantom_chica_jumpscare.png \
    source/phantom_chica_user_jumpscare.c \
    include/assets/phantom_chica_user_jumpscare.h

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

user_audio_path() {
    name="$1"
    for extension in ogg mp3 wav; do
        candidate="$USER_AUDIO_ROOT/$name.$extension"
        if [ -f "$candidate" ]; then
            printf '%s\n' "$candidate"
            return 0
        fi
    done
    return 1
}

convert_user_audio_or_tone() {
    name="$1"
    output="$2"
    frequency="$3"
    duration="$4"
    if input="$(user_audio_path "$name")"; then
        convert_audio "$input" "$output"
        return
    fi
    echo "warning: $name user audio missing; using temporary build tone" >&2
    ffmpeg -hide_banner -loglevel error -y \
        -f lavfi -i "sine=frequency=${frequency}:duration=${duration}:sample_rate=16000" \
        -ar 16000 -ac 1 -f s16be "$output"
}

convert_audio assets/audio_low12/vent_quiet1.mp3 data/audio/vent_quiet1.bin
convert_audio assets/audio_low12/vent_quiet2.mp3 data/audio/vent_quiet2.bin
convert_audio assets/audio_low12/vent_closer1.mp3 data/audio/vent_closer1.bin
convert_audio assets/audio_low12/vent_louder2.mp3 data/audio/vent_louder2.bin
convert_audio assets/audio_low12/alarm.mp3 data/audio/alarm.bin
convert_audio assets/audio_low12/breathing.mp3 data/audio/breathing.bin
convert_audio assets/audio_low12/wait.mp3 data/audio/wait.bin
convert_audio assets/audio_low12/static_sound.mp3 data/audio/static_sound.bin
convert_audio "$SCREAMER_AUDIO" data/audio/scream3.bin

convert_audio assets/audio_phantoms_low/garble1.mp3 data/audio/garble1.bin
convert_audio assets/audio_phantoms_low/mask.mp3 data/audio/mask.bin
convert_audio assets/audio_phantoms_low/echo1.mp3 data/audio/echo1.bin
convert_audio assets/audio_phantoms_low/echo3b.mp3 data/audio/echo3b.bin
convert_audio assets/audio_phantoms_low/echo4b.mp3 data/audio/echo4b.bin

# Keep the PSX extraction/fallback as a safety net, then override it with the
# clean PC call recordings supplied for this Wii U edition.
python3 tools/extract_phone_xa.py "$PHONE_ARCHIVE" data/audio
for night in 1 2 3 4 5 6; do
    if phone_input="$(user_audio_path "phone_night${night}")"; then
        convert_audio "$phone_input" "data/audio/phone_night${night}.bin"
    fi
done

# six_am remains the generated fallback until assets/user_audio/six_am.mp3 is
# provided. If it is present, it transparently replaces the fallback.
if six_am_input="$(user_audio_path six_am)"; then
    convert_audio "$six_am_input" data/audio/six_am.bin
fi

convert_user_audio_or_tone select data/audio/select.bin 880 0.06
convert_user_audio_or_tone end data/audio/end.bin 330 1.36
convert_user_audio_or_tone crank1 data/audio/crank1.bin 180 0.76
convert_user_audio_or_tone crank2 data/audio/crank2.bin 150 0.50
convert_user_audio_or_tone lever1 data/audio/lever1.bin 260 0.39
convert_user_audio_or_tone lever2 data/audio/lever2.bin 300 0.50
convert_user_audio_or_tone stare data/audio/stare.bin 55 73.88
convert_user_audio_or_tone titlemusic data/audio/titlemusic.bin 110 40.39
convert_user_audio_or_tone startday data/audio/startday.bin 440 4.65

rm -rf assets/audio_low12 assets/audio_phantoms_low
