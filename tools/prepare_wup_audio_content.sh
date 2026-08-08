#!/bin/sh
set -eu

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
OUT="${1:-$ROOT/.wup-audio/audio}"
WORK="${TMPDIR:-/tmp}/fnaf3-wup-audio.$$"
ARCHIVE="$WORK/fnaf3-sounds.zip"
EXTRACTED="$WORK/extracted"
EXPECTED_ARCHIVE_SHA256="128b50e7717a4d0fc9ba3dd9fab3835542c0f9777f7c699f8caaa9c1c054b32e"
EXPECTED_SIX_AM_OPUS_SHA256="aa977226f8b91941f158d39f5712d8f03ce0a150cbaf95e6e6e91aaa37da79e6"

cleanup() { rm -rf "$WORK"; }
trap cleanup EXIT INT TERM
mkdir -p "$WORK" "$EXTRACTED"
rm -rf "$OUT"
mkdir -p "$OUT"

for tool in curl unzip ffmpeg sha256sum base64; do
    command -v "$tool" >/dev/null 2>&1 || {
        echo "Missing required tool: $tool" >&2
        exit 1
    }
done

verify_sha256() {
    file="$1"
    expected="$2"
    got="$(sha256sum "$file" | awk '{print $1}')"
    [ "$got" = "$expected" ] || {
        echo "SHA-256 mismatch for $(basename "$file"): $got" >&2
        return 1
    }
}

download_archive() {
    if [ -n "${FNAF3_SOUND_ARCHIVE:-}" ]; then
        cp "$FNAF3_SOUND_ARCHIVE" "$ARCHIVE"
        verify_sha256 "$ARCHIVE" "$EXPECTED_ARCHIVE_SHA256"
        return 0
    fi

    if [ -n "${FNAF3_SOUND_ARCHIVE_URL:-}" ]; then
        urls="$FNAF3_SOUND_ARCHIVE_URL"
    else
        urls="https://www.sounds-resource.com/download/398090/
https://sounds.spriters-resource.com/download/398090/"
    fi

    for url in $urls; do
        echo "Trying original FNaF 3 sound archive: $url"
        rm -f "$ARCHIVE"
        if curl -L --fail --retry 3 --retry-delay 2 \
            -A 'Mozilla/5.0 (compatible; FNaF3-WiiU-build/1.0)' \
            -e 'https://sounds.spriters-resource.com/pc_computer/fivenightsatfreddys3/asset/398090/' \
            "$url" -o "$ARCHIVE" && \
            verify_sha256 "$ARCHIVE" "$EXPECTED_ARCHIVE_SHA256"; then
            return 0
        fi
    done

    echo "Unable to fetch the verified 63-file FNaF 3 sound archive." >&2
    exit 1
}

find_sound() {
    name="$1"
    found="$(find "$EXTRACTED" -type f -name "$name" -print -quit)"
    [ -n "$found" ] || {
        echo "Missing source WAV: $name" >&2
        exit 1
    }
    printf '%s\n' "$found"
}

convert_sound() {
    output="$1"
    source_name="$2"
    source="$(find_sound "$source_name")"
    ffmpeg -hide_banner -loglevel error -y -i "$source" \
        -ar 16000 -ac 1 -f s16be "$OUT/$output.bin"
    [ -s "$OUT/$output.bin" ] || {
        echo "Empty converted audio: $output.bin" >&2
        exit 1
    }
}

download_archive
unzip -q "$ARCHIVE" -d "$EXTRACTED"

convert_sound alarm alarm.wav
convert_sound breathing breathing.wav
convert_sound clock_chimes Clocks_Chimes_Cl_02480702.wav
convert_sound collect collect.wav
convert_sound crank1 crank1.wav
convert_sound crank2 crank2.wav
convert_sound crowd_children CROWD_SMALL_CHIL_EC049202.wav
convert_sound crush crush.wav
convert_sound danger2b danger2b.wav
convert_sound desolate_underworld Desolate_Underworld2.wav
convert_sound done done.wav
convert_sound echo1 echo1.wav
convert_sound echo3b echo3b.wav
convert_sound echo4b echo4b.wav
convert_sound end end.wav
convert_sound feed feed.wav
convert_sound garble1 garble1.wav
convert_sound glitch2 glitch2.wav
convert_sound lever1 lever1.wav
convert_sound lever2 lever2.wav
convert_sound mask mask.wav
convert_sound mb1 mb1.wav
convert_sound mb2 mb2.wav
convert_sound mb4b mb4b.wav
convert_sound mb5 mb5.wav
convert_sound mb8 mb8.wav
convert_sound mb9 mb9.wav
convert_sound party_favor PartyFavorraspyPart_AC01__3.wav
convert_sound phone_night1 night1final.wav
convert_sound phone_night2 night2final2.wav
convert_sound phone_night3 night3final.wav
convert_sound phone_night4 night4final.wav
convert_sound phone_night5 night5final.wav
convert_sound phone_night6 night6final.wav
convert_sound rainstorm2 rainstorm2.wav
convert_sound scanner4 scanner4.wav
convert_sound scream3 scream3.wav
convert_sound select select.wav
convert_sound stare stare.wav
convert_sound startday startday.wav
convert_sound static_sound static_sound.wav
convert_sound tablefan tablefan.wav
convert_sound titlemusic titlemusic.wav
convert_sound vent_closer1 vent_closer1.wav
convert_sound vent_louder2 vent_louder2.wav
convert_sound vent_quiet1 vent_quiet1.wav
convert_sound vent_quiet2 vent_quiet2.wav
convert_sound wait wait.wav

# six_am was supplied separately during the original audio-restoration pass.
# It is stored in compact Opus form as text chunks so the WUP build remains
# reproducible without committing the large source MP3.
SIX_AM_OPUS="$WORK/six_am.opus"
cat "$ROOT"/assets/wup_audio/six_am.opus.b64.* | base64 -d > "$SIX_AM_OPUS"
verify_sha256 "$SIX_AM_OPUS" "$EXPECTED_SIX_AM_OPUS_SHA256"
ffmpeg -hide_banner -loglevel error -y -i "$SIX_AM_OPUS" \
    -ar 16000 -ac 1 -f s16be "$OUT/six_am.bin"

count="$(find "$OUT" -maxdepth 1 -type f -name '*.bin' | wc -l | tr -d ' ')"
[ "$count" = "49" ] || {
    echo "Expected 49 WUP audio files, got $count" >&2
    exit 1
}

for file in "$OUT"/*.bin; do
    [ -s "$file" ] || {
        echo "Empty WUP audio file: $file" >&2
        exit 1
    }
done

echo "Prepared $count original-game WUP audio cues in $OUT"
