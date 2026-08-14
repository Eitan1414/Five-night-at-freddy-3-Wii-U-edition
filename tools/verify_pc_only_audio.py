#!/usr/bin/env python3
"""Fail the build if a standard runtime sound can be replaced by a non-PC fallback."""

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]


def fail(message: str) -> None:
    print(f"PC-only audio audit FAILED: {message}", file=sys.stderr)
    raise SystemExit(1)


audio_c = (ROOT / "source/platform/audio_wiiu.c").read_text(encoding="utf-8")
audio_h = (ROOT / "include/platform/audio.h").read_text(encoding="utf-8")
prepare = (ROOT / "tools/prepare_generated_assets.sh").read_text(encoding="utf-8")
minigame = (ROOT / "source/platform/pc_minigame_sfx_wiiu.c").read_text(encoding="utf-8")

# Every declared runtime cue must have an embedded clip route. The only two
# project-specific exceptions in provenance are Achievement and Utine, but they
# are embedded as well so a clean install never depends on SD content.
cues = []
for cue in re.findall(r"\b(AUDIO_CUE_[A-Z0-9_]+)\b", audio_h):
    if cue == "AUDIO_CUE_COUNT" or cue in cues:
        continue
    cues.append(cue)
for cue in cues:
    if f"[{cue}] = {{" not in audio_c:
        fail(f"runtime cue is not backed by an embedded clip: {cue}")

# Standard PC sounds must never be externally replaceable. Only the two Wii U
# notification sounds are intentionally customizable from SD.
external_paths = set(re.findall(r'"(audio/[^"\\]+\.bin)"', audio_c))
allowed_external = {"audio/achievement.bin", "audio/utine.bin"}
if external_paths != allowed_external:
    fail(
        "external audio override set is not exactly Achievement/Utine: "
        + ", ".join(sorted(external_paths))
    )

if "load_external_audio(AUDIO_CUE_ACHIEVEMENT);" not in audio_c:
    fail("Achievement override route is missing")
if "load_external_audio(AUDIO_CUE_UTINE);" not in audio_c:
    fail("Utine override route is missing")
if re.search(r"for\s*\([^)]*AUDIO_CUE_COUNT[^)]*\)\s*\n\s*load_external_audio", audio_c):
    fail("bulk external-audio override loop returned")

# Build provenance: the verified PC archive must be pinned and all WAVs must be
# converted before linking. Missing assets intentionally fail the build.
required_prepare_markers = (
    "PC_SOUND_PAGE=",
    "128b50e7717a4d0fc9ba3dd9fab3835542c0f9777f7c699f8caaa9c1c054b32e",
    "tools/convert_pc_sound_pack.py",
    "Missing original PC audio after conversion",
    "PC-only generated assets prepared; no PSX source was used",
)
for marker in required_prepare_markers:
    if marker not in prepare:
        fail(f"verified PC sound-pack build marker is missing: {marker}")

# The dedicated secret-minigame player must remain embedded-only as well.
if "platform/storage.h" in minigame or "storage_read(" in minigame or "storage_file_size(" in minigame:
    fail("secret-minigame SFX regained an external storage fallback")

print(
    "PC-only audio audit passed: all runtime cues are embedded, standard sounds "
    "cannot be overridden, and only Achievement/Utine remain customizable"
)
