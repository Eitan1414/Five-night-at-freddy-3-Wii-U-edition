#!/usr/bin/env python3
"""Audit rare/random FNaF 3 events decoded from fivenights3-94.mfa."""

from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


def fail(message: str) -> None:
    print(f"PC rare-event audit FAILED: {message}", file=sys.stderr)
    raise SystemExit(1)


def require(text: str, marker: str, context: str) -> None:
    if marker not in text:
        fail(f"{context}: missing {marker!r}")


phantom = read("source/game/phantom_ai_parts/part00.inc")
systems = read("source/main_v3_parts/main_pc_system_fidelity.inc")
camera = read("source/main_v3_parts/main_pc_camera_fidelity.inc")

# Random Phantom clocks and the source rule excluding 12 AM.
require(phantom, "#define EVERY_20_SECONDS (20u * FRAMES_PER_SECOND)", "20-second Phantom clock")
require(phantom, "#define EVERY_60_SECONDS (60u * FRAMES_PER_SECOND)", "60-second Freddy clock")
require(phantom, "if (hour <= 0) {\n        return;\n    }", "12 AM random-spawn exclusion")
require(phantom, "#define SCARE_COOLDOWN_SECONDS 10u", "shared Phantom scare cooldown")

# Random roll denominators from the decoded MFA event groups.
for marker in (
    "ai_roll(system, 10u)",  # BB / Puppet / Chica
    "ai_roll(system, 7u)",   # Mangle
    "ai_roll(system, 12u)",  # Freddy
):
    require(phantom, marker, "Phantom random roll")

# Forced hours: BB at 3 AM, Freddy at 4 AM, Mangle/Puppet/Chica at 5 AM.
for marker in (
    "if (hour == 3 && !system->forced_bb",
    "if (hour == 4 && !system->forced_freddy",
    "if (hour == 5) {",
    "!system->forced_mangle",
    "!system->forced_puppet",
    "!system->forced_chica",
):
    require(phantom, marker, "forced Phantom event")

# Phantom Foxy is rolled only after the monitor opening animation finishes and
# uses the exact night-dependent denominators.
for marker in (
    "system->foxy_roll_pending = true;",
    "if (system->night == 2) denominator = 1000u;",
    "else if (system->night == 3) denominator = 50u;",
    "else if (system->night == 4) denominator = 25u;",
    "else denominator = 10u;",
    "system->foxy_present = (next_random(system) % denominator) == 1u;",
):
    require(phantom, marker, "Phantom Foxy rare roll")

# Hallucinationtrap: ventilation failure threshold, source duration formula and
# one-in-three fake-Springtrap roll per eligible camera. The real Springtrap
# camera must never be marked as a hallucination.
for marker in (
    "(uint32_t)(ai * 200) + (pc_systems_random() % 200u)",
    "if (camera == real_camera) continue;",
    "if ((pc_systems_random() % 3u) == 1u)",
    "const uint32_t threshold = (uint32_t)(1000 - ai * 100);",
    "if (game->ventilation_error_frames > threshold)",
):
    require(systems, marker, "Hallucinationtrap MFA rule")
require(camera, "sPcHallucinationCameraMask", "hallucination camera mask")
require(camera, "sPcHallucinationFrames", "hallucination lifetime")
require(camera, "pc_camera_fidelity_has_hallucination(camera)", "hallucination renderer route")

# Night AI and exposure thresholds are source-decoded and must not drift.
for marker in (
    "if (night <= 2) return 2;",
    "if (night == 3) return 3;",
    "if (night == 4) return 4;",
    "if (night == 5) return 5;",
    "return 7;",
    "if (night <= 1) return 100u;",
    "if (night == 2) return 90u;",
    "if (night == 3) return 80u;",
    "if (night == 4) return 70u;",
    "if (night == 5) return 60u;",
    "return 50u;",
):
    require(phantom, marker, "Phantom AI/time-limit table")

print("PC rare-event audit passed: Phantom clocks/forced events/Foxy odds and Hallucinationtrap rules match the decoded MFA routes")
