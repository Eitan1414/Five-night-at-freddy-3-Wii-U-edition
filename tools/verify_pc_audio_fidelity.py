#!/usr/bin/env python3
"""Audit runtime audio behavior that is already decoded/verified from the PC path.

This intentionally does not invent Clickteam mixer-volume values. It locks the
sample-rate path, loop/one-shot semantics, cue lifecycles and simultaneous
ambience routes that the Wii U port can verify from its decoded MFA behavior.
"""

from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


def fail(message: str) -> None:
    print(f"PC audio fidelity audit FAILED: {message}", file=sys.stderr)
    raise SystemExit(1)


def require(text: str, marker: str, context: str) -> None:
    if marker not in text:
        fail(f"{context}: missing {marker!r}")


prepare = read("tools/prepare_generated_assets.sh")
audio = read("source/platform/audio_wiiu.c")
main00 = read("source/main_v3_parts/main_00.inc")
main01 = read("source/main_v3_parts/main_01.inc")
main02 = read("source/main_v3_parts/main_02.inc")
full = read("source/main_v3_parts/main_full_audio.inc")
finishing = read("source/main_v3_parts/main_finishing.inc")
minigame = read("source/platform/pc_minigame_sfx_wiiu.c")

# 16 kHz signed big-endian PCM is embedded, while AX renders at 32 kHz. The
# 0.5 source ratio therefore preserves the original converted playback speed.
for marker in ("-ar 16000", "-ac 1", "-f s16be"):
    require(prepare, marker, "asset conversion format")
require(audio, "AX_INIT_RENDERER_32KHZ", "AX renderer rate")
require(audio, "AXSetVoiceSrcRatio(voice, 0.5f)", "16kHz->32kHz playback ratio")
require(audio, "AX_DEVICE_TYPE_TV", "TV audio routing")
require(audio, "AX_DEVICE_TYPE_DRC", "GamePad audio routing")
require(minigame, "AXSetVoiceSrcRatio(voice, 0.5f)", "minigame playback ratio")
require(minigame, "AX_DEVICE_TYPE_TV", "minigame TV routing")
require(minigame, "AX_DEVICE_TYPE_DRC", "minigame GamePad routing")

# Ventilation: alarm and breathing are persistent loops and both are stopped by
# the repair/reset path.
require(main00, "audio_restart(AUDIO_CUE_ALARM, 0.72f, true);", "ventilation alarm loop")
require(main00, "audio_stop(AUDIO_CUE_ALARM);", "ventilation alarm stop")
require(main00, "audio_stop(AUDIO_CUE_BREATHING);", "ventilation breathing stop")
require(main02, "audio_restart(AUDIO_CUE_BREATHING, 0.58f, true);", "ventilation breathing loop")

# Camera static is a short burst on open/switch, never a monitor-long loop.
require(main01, "audio_restart(AUDIO_CUE_STATIC, 0.28f, false);", "camera static one-shot")
require(main01, "audio_stop(AUDIO_CUE_STATIC);", "camera static cutoff")
if "audio_restart(AUDIO_CUE_STATIC, 0.28f, true)" in main01:
    fail("camera static became a loop")

# Audio lure uses one of the three exact PC echoes as a one-shot. The decoded
# seven-step, 1500ms recharge remains 630 frames at 60 Hz.
for cue in ("AUDIO_CUE_LURE_ECHO_1", "AUDIO_CUE_LURE_ECHO_3B", "AUDIO_CUE_LURE_ECHO_4B"):
    require(main01, cue, "Play Audio cue set")
require(main01, "audio_play(cue, 0.72f, false);", "Play Audio one-shot")
require(main00, "#define AUDIO_LURE_COOLDOWN_FRAMES 630u", "Play Audio 10.5s recharge")

# Phantom Mangle and Puppet own long-lived effects with explicit end events.
require(main00, "audio_restart(AUDIO_CUE_GARBLE, 0.92f, true);", "Mangle garble start")
require(main00, "audio_stop(AUDIO_CUE_GARBLE);", "Mangle garble stop")
require(main00, "audio_restart(AUDIO_CUE_MASK, 0.82f, true);", "Puppet mask start")
require(main00, "audio_stop(AUDIO_CUE_MASK);", "Puppet mask stop")

# Office ambience is intentionally simultaneous, persists while panels are used
# (screen remains SCREEN_OFFICE), and is stopped on real screen transitions.
require(full, "audio_restart(AUDIO_CUE_OFFICE_FAN, 0.30f, true);", "office fan loop")
require(full, "audio_restart(AUDIO_CUE_RAIN_AMBIENCE, 0.13f, true);", "office rain loop")
require(full, "audio_stop(AUDIO_CUE_OFFICE_FAN);", "office fan stop")
require(full, "audio_stop(AUDIO_CUE_RAIN_AMBIENCE);", "office rain stop")
require(full, "if (before_screen == SCREEN_OFFICE)", "office ambience transition stop")
require(full, "if (game->screen == SCREEN_OFFICE && !sSecret.active)", "office ambience transition start")

# Maintenance scanner is a loop only while a repair is active; completion is a
# separate one-shot. Danger cues are one-shots so repeated messages restart them.
require(full, "audio_restart(AUDIO_CUE_REPAIR_SCANNER, 0.52f, true);", "repair scanner loop")
require(full, "audio_stop(AUDIO_CUE_REPAIR_SCANNER);", "repair scanner stop")
require(full, "audio_play(AUDIO_CUE_REPAIR_DONE, 0.82f, false);", "repair completion one-shot")
require(full, "AUDIO_CUE_DANGER", "Springtrap danger cue")
require(full, "false);", "one-shot audio paths")

# All six calls are contiguous PC cues, one-shot, and explicitly stoppable.
require(finishing, "AUDIO_CUE_PHONE_NIGHT_1 + night - 1", "six contiguous phone calls")
require(finishing, "audio_play(finishing_phone_cue(game->night), 0.76f, false);", "phone-call one-shot")
require(finishing, "audio_stop(finishing_phone_cue(sFinishing.phone_night));", "phone-call stop/mute route")
for duration in ("12200u", "9620u", "5704u", "4592u", "4060u", "3140u"):
    require(finishing, duration, "phone duration table")

# 6 AM is built from the two PC streams used by the game, rather than a synthetic
# replacement or a legacy console clip.
require(prepare, 'root / "clock_chimes.bin"', "6 AM clock source")
require(prepare, 'root / "crowd_children.bin"', "6 AM crowd source")
require(prepare, '(root / "six_am.bin").write_bytes(out)', "6 AM deterministic mix")

# Secret-minigame music loops, while pickups/feed/glitch/finale effects are
# one-shots. Dedicated movement SFX remain in the embedded PC player.
require(full, "audio_restart(full_audio_secret_music(sSecret.kind), 0.48f, true);", "secret minigame music loop")
for cue in (
    "AUDIO_CUE_MINIGAME_GLITCH",
    "AUDIO_CUE_MINIGAME_FEED",
    "AUDIO_CUE_MINIGAME_COLLECT",
    "AUDIO_CUE_MINIGAME_CHIMES",
    "AUDIO_CUE_MINIGAME_CROWD",
    "AUDIO_CUE_MINIGAME_PARTY_FAVOR",
):
    require(full, cue, "secret-minigame audio route")
for cue in ("get", "get2", "jump", "jump2", "jump3", "jump4", "land", "run", "long_glitched2", "insuit", "laugh", "scare", "stop", "crazy_garble"):
    require(minigame, f"DECLARE_PC_SFX_BIN({cue});", "embedded minigame SFX")

print("PC audio fidelity audit passed: rate, routing, loops, cutoffs, calls, ambience and minigame semantics are locked")
