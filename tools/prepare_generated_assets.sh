#!/bin/sh
set -eu

restore_xz_base64() {
    source_pattern="$1"
    output="$2"
    # shellcheck disable=SC2086
    cat $source_pattern | base64 -d | xz -dc > "$output"
}

if ! command -v ffmpeg >/dev/null 2>&1; then
    echo "ffmpeg is required to prepare audio assets" >&2
    exit 1
fi
if ! command -v python3 >/dev/null 2>&1; then
    echo "python3 is required to prepare generated assets" >&2
    exit 1
fi
if ! command -v curl >/dev/null 2>&1 || ! command -v unzip >/dev/null 2>&1; then
    echo "curl and unzip are required to fetch the verified PC sound pack" >&2
    exit 1
fi

# This remaining generated visual is a user-supplied Wii U override, not a PSX
# asset. All former PSX camera/Phantom/screamer restoration has been removed.
restore_xz_base64 \
    "source/generated/phantom_mangle_user_texture.h.xz.b64" \
    "include/assets/phantom_mangle_user_texture.h"

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

# Supplied Wii U achievement package. This is project-specific content and has
# no PlayStation source dependency.
if ls source/generated/achievement_assets_micro.tar.xz.b64.* >/dev/null 2>&1; then
    temporary_achievements="${TMPDIR:-/tmp}/fnaf3-achievements-micro.tar.xz"
    # shellcheck disable=SC2086
    cat source/generated/achievement_assets_micro.tar.xz.b64.* \
        | base64 -d > "$temporary_achievements"
    tar -xJf "$temporary_achievements" -C .
    rm -f "$temporary_achievements"
else
    echo "Compact achievement asset bundle is missing" >&2
    exit 1
fi

test -s source/achievement_assets.c
test -s include/assets/achievement_assets.h
test -s assets/user_audio/achievement.ogg
test -s assets/user_audio/utine.ogg

python3 tools/convert_user_chica_png.py \
    assets/user_visuals/phantom_chica_jumpscare.png \
    source/phantom_chica_user_jumpscare.c \
    include/assets/phantom_chica_user_jumpscare.h

USER_AUDIO_ROOT="${FNAF3_USER_AUDIO_ROOT:-assets/user_audio}"
mkdir -p data/audio
rm -rf data/audio/*

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

# Fetch the verified original PC Sound Effects archive. The SHA-256 pin makes
# this reproducible and prevents an upstream replacement from silently changing
# the Wii U build.
PC_SOUND_PAGE="https://sounds.spriters-resource.com/pc_computer/fivenightsatfreddys3/asset/398090/"
PC_SOUND_TEMP="${TMPDIR:-/tmp}/fnaf3-pc-sounds-$$"
PC_SOUND_HTML="$PC_SOUND_TEMP/page.html"
PC_SOUND_ZIP="$PC_SOUND_TEMP/fnaf3-sounds.zip"
mkdir -p "$PC_SOUND_TEMP"

curl --http1.1 -L --fail --retry 5 --retry-all-errors --retry-delay 3 \
    -A "Mozilla/5.0" "$PC_SOUND_PAGE" -o "$PC_SOUND_HTML"

PC_SOUND_URL="$(python3 - "$PC_SOUND_HTML" "$PC_SOUND_PAGE" <<'PY'
import html
import re
import sys
from urllib.parse import urljoin

html_path, page_url = sys.argv[1:3]
text = html.unescape(open(html_path, encoding="utf-8", errors="ignore").read())
candidates = []
for pattern in (
    r'''href\s*=\s*["']([^"']+)["']''',
    r'''(?:data-url|data-download|action)\s*=\s*["']([^"']+)["']''',
    r'''https?://[^"'<>\\s]+''',
):
    for match in re.findall(pattern, text, flags=re.I):
        value = match if isinstance(match, str) else match[0]
        value = value.strip()
        low = value.lower()
        if ("download" in low or low.endswith(".zip") or "398090" in low) and value not in candidates:
            candidates.append(value)
for value in sorted(candidates, key=lambda v: (
        not v.lower().endswith(".zip"),
        "download" not in v.lower(),
        "398090" not in v,
        len(v))):
    absolute = urljoin(page_url, value)
    if absolute.rstrip("/") != page_url.rstrip("/"):
        print(absolute)
        break
PY
)"

if [ -z "$PC_SOUND_URL" ]; then
    echo "Could not resolve the verified PC Sound Effects ZIP URL" >&2
    exit 1
fi

curl --http1.1 -L --fail --retry 10 --retry-all-errors --retry-delay 5 \
    --connect-timeout 30 --max-time 1800 \
    -A "Mozilla/5.0" -H "Referer: $PC_SOUND_PAGE" \
    "$PC_SOUND_URL" -o "$PC_SOUND_ZIP"

echo "128b50e7717a4d0fc9ba3dd9fab3835542c0f9777f7c699f8caaa9c1c054b32e  $PC_SOUND_ZIP" \
    | sha256sum -c -

# Convert every WAV in the PC pack, not just the six Phantom sounds used by the
# previous pass. This supplies phone calls, UI, camera/maintenance, ambience,
# minigame music and the movement/glitch effects that were previously missing.
python3 tools/convert_pc_sound_pack.py "$PC_SOUND_ZIP" data/audio
rm -f data/audio/pc_sound_pack_manifest.txt

# The archive names the 6 AM sting "6AM"; the runtime has historically called
# that cue six_am. Resolve it from the converted PC pack instead of requiring a
# user file or falling back to PlayStation audio.
if [ ! -s data/audio/six_am.bin ]; then
    if [ -s data/audio/6am.bin ]; then
        mv data/audio/6am.bin data/audio/six_am.bin
    else
        six_am_candidate="$(find data/audio -maxdepth 1 -type f \
            \( -iname '*6*am*.bin' -o -iname '*six*am*.bin' \) \
            | sort | head -n 1)"
        if [ -n "$six_am_candidate" ] && [ -s "$six_am_candidate" ]; then
            mv "$six_am_candidate" data/audio/six_am.bin
        else
            echo "Could not resolve the original PC 6 AM cue from the verified sound pack" >&2
            echo "Available converted files:" >&2
            find data/audio -maxdepth 1 -type f -printf '%f\n' | sort >&2
            exit 1
        fi
    fi
fi

# Wii U-exclusive achievement sounds remain project-specific user assets.
if achievement_input="$(user_audio_path achievement)"; then
    convert_audio "$achievement_input" data/audio/achievement.bin
else
    echo "achievement notification audio is missing" >&2
    exit 1
fi
if utine_input="$(user_audio_path utine)"; then
    convert_audio "$utine_input" data/audio/utine.bin
else
    echo "Utine trophy audio is missing" >&2
    exit 1
fi

# Every runtime cue below must now have a real source. Failing here is
# intentional: a missing PC asset must be fixed instead of silently reintroducing
# a PlayStation clip, a synthetic tone or silence.
required_bins="
vent_quiet1 vent_quiet2 vent_closer1 vent_louder2 alarm breathing wait static_sound
scream3 garble1 mask echo1 echo3b echo4b
phone_night1 phone_night2 phone_night3 phone_night4 phone_night5 phone_night6
six_am select end crank1 crank2 lever1 lever2 stare titlemusic startday achievement utine
tablefan rainstorm2 danger2b scanner4 done collect feed glitch2 crowd_children clock_chimes party_favor desolate_underworld crush
mb1 mb2 mb4b mb5 mb8 mb9
get get2 jump jump2 jump3 jump4 land run long_glitched2 insuit laugh scare stop crazy_garble
"
for name in $required_bins; do
    if [ ! -s "data/audio/$name.bin" ]; then
        echo "Missing original PC audio after conversion: $name.bin" >&2
        echo "Available converted files:" >&2
        find data/audio -maxdepth 1 -type f -printf '%f\n' | sort >&2
        exit 1
    fi
done

rm -rf "$PC_SOUND_TEMP"
echo "PC-only generated assets prepared; no PSX source was used"
