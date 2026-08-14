#!/usr/bin/env python3
"""Fail the build if a PSX visual path or runtime sprite fallback returns.

This is deliberately a provenance/route audit rather than an image similarity
check. The CI build regenerates the PC compatibility banks immediately before
this script runs, so validating their aliases and final renderer wiring proves
that historical Wii U symbol names resolve to the verified PC asset bank.
"""

from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]


def fail(message: str) -> None:
    print(f"PC-only visual audit FAILED: {message}", file=sys.stderr)
    raise SystemExit(1)


# Raw PlayStation texture files must never be present in the repository again.
for path in ROOT.rglob("*"):
    if not path.is_file():
        continue
    relative = path.relative_to(ROOT)
    if path.suffix.lower() == ".tim":
        fail(f"PlayStation TIM file present: {relative}")
    if "psx" in path.name.lower():
        fail(f"PSX-labelled file present: {relative}")

# These were old standalone/custom fallback payloads. The PC renderer owns all
# of these routes now, so restoring any one is an explicit regression.
for relative in (
    "source/phantom_assets.c",
    "source/camera_springtrap_assets.c",
    "source/jumpscare_assets.c",
    "tools/convert_user_chica_png.py",
    "assets/user_visuals/phantom_chica_jumpscare.png.b64.part1",
    "assets/user_visuals/phantom_chica_jumpscare.png.b64.part2",
    "assets/user_visuals/phantom_chica_jumpscare.png.b64.part3",
    "assets/user_visuals/phantom_chica_jumpscare.png.b64.part4",
):
    if (ROOT / relative).exists():
        fail(f"obsolete visual fallback restored: {relative}")

compat_header = ROOT / "include/assets/pc_compat_visuals.h"
if not compat_header.is_file():
    fail("generated PC compatibility header is missing")
compat = compat_header.read_text(encoding="utf-8")

# Historical symbol names are still referenced by some stable Wii U render
# paths. Every one below must resolve to a PC-generated object.
expected_aliases = {
    "gWarningTexture": "gPcCompatWarningTexture",
    "gMenuSpringtrapTexture": "gPcCompatTitleSpringtrap1Texture",
    "gMenuSpringtrapTexture2": "gPcCompatTitleSpringtrap2Texture",
    "gMenuSpringtrapTexture3": "gPcCompatTitleSpringtrap3Texture",
    "gMenuSpringtrapTexture4": "gPcCompatTitleSpringtrap4Texture",
    "gMenuSpringtrapTexture5": "gPcCompatTitleSpringtrap5Texture",
    "gOfficeTexture": "gPcCompatOfficeTexture",
    "gOriginalCameraSpringtrapTextures": "gPcCompatSpringtrapCameraTextures",
    "gPhantomFoxyOfficeTexture": "gPcCompatPhantomFoxyOfficeTexture",
    "gPhantomFreddyOfficeTexture": "gPcCompatPhantomFreddyOfficeTexture",
    "gPhantomChicaOfficeTexture": "gPcCompatPhantomChicaOfficeTexture",
    "gPhantomMangleOfficeTexture": "gPcCompatPhantomMangleOfficeTexture",
    "gPhantomPuppetOfficeTexture": "gPcCompatPhantomPuppetTexture",
    "gPhantomBBCameraTexture": "gPcCompatPhantomBBTexture",
    "gPhantomBBOfficeTexture": "gPcCompatPhantomBBTexture",
    "gPhantomChicaCameraTexture": "gPcCompatPhantomChicaCameraTexture",
    "gPhantomMangleCameraTexture": "gPcCompatPhantomMangleCameraTexture",
    "gPhantomPuppetCameraTexture": "gPcCompatPhantomPuppetCameraTexture",
    "gPhantomPuppetRealAnimation": "gPcCompatPhantomPuppetAnimation",
}
for index in range(1, 11):
    expected_aliases[f"gCamera{index:02d}Texture"] = (
        f"gPcCompatCamera{index:02d}Texture"
    )

for old, pc in expected_aliases.items():
    directive = f"#define {old} {pc}"
    if directive not in compat:
        fail(f"missing PC alias: {directive}")

main_c = (ROOT / "source/main.c").read_text(encoding="utf-8")
required_runtime_routes = (
    '#include "assets/pc_core_visuals.h"',
    '#include "assets/pc_compat_visuals.h"',
    '#include "assets/pc_character_visuals.h"',
    '#include "assets/pc_finishing_visuals.h"',
    '#include "main_v3_parts/main_pc_visual_override.inc"',
    '#include "main_v3_parts/main_pc_camera_fidelity.inc"',
    '#include "main_v3_parts/main_pc_character_override.inc"',
    '#include "main_v3_parts/main_pc_finishing_override.inc"',
    "#define gSpringtrapJumpscareLeft gPcSpringtrapJumpscare",
    "#define gSpringtrapJumpscareRight gPcSpringtrapJumpscare",
    "#define gPhantomFoxyRealJumpscare gPcPhantomFoxyJumpscare",
    "#define gPhantomBBRealJumpscare gPcPhantomBBJumpscare",
    "#define gPhantomFreddyRealJumpscare gPcPhantomFreddyJumpscare",
    "#define gPhantomChicaRealJumpscare gPcPhantomChicaJumpscare",
)
for route in required_runtime_routes:
    if route not in main_c:
        fail(f"final runtime PC route is missing: {route}")

if "phantom_chica_user_jumpscare" in main_c:
    fail("obsolete custom Phantom Chica include returned to the final runtime")

for relative in (
    "source/pc_core_visuals.c",
    "source/pc_compat_visuals.c",
    "source/pc_hud_visuals.c",
    "source/pc_character_visuals.c",
    "source/pc_finishing_visuals.c",
    "source/pc_night_intro_visuals.c",
):
    path = ROOT / relative
    if not path.is_file() or path.stat().st_size == 0:
        fail(f"generated PC visual bank is missing/empty: {relative}")

print("PC-only visual audit passed: no TIM/PSX/custom fallback paths and all legacy runtime sprite routes resolve to PC assets")
