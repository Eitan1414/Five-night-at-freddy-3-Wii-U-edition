#!/usr/bin/env python3
"""Lock decoded PC/MFA geometry, hotspots, draw order and character routes."""

from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


def fail(message: str) -> None:
    print(f"PC visual fidelity audit FAILED: {message}", file=sys.stderr)
    raise SystemExit(1)


def require(text: str, marker: str, context: str) -> None:
    if marker not in text:
        fail(f"{context}: missing {marker!r}")


main = read("source/main.c")
camera = read("source/main_v3_parts/main_pc_camera_fidelity.inc")
characters = read("source/main_v3_parts/main_pc_character_override.inc")
title = read("source/main_v3_parts/main_complete_title.inc")

# Final camera viewport keeps the 825x650 retail-PC source aspect on a compact,
# real-Wii-U-safe 432x340 RLE feed.
for marker in (
    "#define PC_MONITOR_PANEL_X 174",
    "#define PC_MONITOR_PANEL_Y 38",
    "#define PC_MONITOR_PANEL_W 674",
    "#define PC_MONITOR_PANEL_H 423",
    "#define PC_CAMERA_FEED_X 196",
    "#define PC_CAMERA_FEED_Y 70",
    "#define PC_CAMERA_FEED_W 432",
    "#define PC_CAMERA_FEED_H 340",
    "#define PC_CAMERA_SOURCE_W 825",
    "#define PC_CAMERA_SOURCE_H 650",
):
    require(camera, marker, "camera viewport geometry")

# Exact Night-3 Shadow Cupcake Create coordinates, Clickteam hotspots and source
# dimensions decoded from the MFA. These are separate Active objects, not baked
# into camera backgrounds.
for marker in (
    "{1, 0x01u, 528, 426, 15, 30, 31, 51,",
    "{2, 0x02u, 654, 360, 30, 59, 61, 99,",
    "{3, 0x04u, 432, 430, 20, 39, 41, 67,",
    "{5, 0x08u, 416, 314, 30, 59, 61, 99,",
):
    require(camera, marker, "Shadow Cupcake MFA geometry")

# Full-camera Phantom composites and Springtrap must render inside the same
# source-space viewport. The cupcake layer is intentionally drawn before the
# Phantom composite, matching the object layering used by the converted path.
for marker in (
    "&gPhantomBBCameraTexture",
    "&gPhantomChicaCameraTexture",
    "&gPhantomMangleCameraTexture",
    "&gPhantomPuppetCameraTexture",
    "gOriginalCameraSpringtrapTextures[camera]",
    "pc_camera_draw_shadow_cupcake(game);",
    "pc_camera_draw_compact_phantom(game, phantom);",
):
    require(camera, marker, "camera character route")
if camera.index("pc_camera_draw_shadow_cupcake(game);") > camera.index("pc_camera_draw_compact_phantom(game, phantom);"):
    fail("camera draw order changed: cupcake must precede Phantom composite")

# Static is clipped to the video feed and the PC map is drawn afterwards so its
# labels/buttons remain readable during camera switches.
if camera.index("pc_camera_draw_static(game);") > camera.index("pc_camera_draw_map(game);"):
    fail("camera-switch static draw order changed: map must remain above static")
require(camera, "original_ui_draw_texture(map_id, sOriginalUiPanelTargets,\n                             591, 177, 252, 235);", "PC camera-map geometry")

# Office character placement is sourced from decoded MFA hotspots/path vectors.
for marker in (
    "646, 77, 164, 362",
    "22, 72, 170, 374",
    "texture_x + 304, 58,\n                         175, 397",
    "760 - path_x, 126 - path_y,\n                             174, 231",
    "texture_x + 115, 89,\n                         362, 366",
    "texture_x + 580, 253,\n                         94, 115",
    "102, 5, 650, 465",
    "#define PC_FREDDY_SOURCE_PATH_LENGTH 1061u",
    "#define PC_FREDDY_SOURCE_PATH_DY 12u",
    "#define PC_OFFICE_SCALE_NUM 14u",
    "#define PC_OFFICE_SCALE_DEN 25u",
):
    require(characters, marker, "office character MFA geometry")

# Final jumpscare routes are PC-generated sequences, including Phantom Chica.
for marker in (
    "#define gSpringtrapJumpscareLeft gPcSpringtrapJumpscare",
    "#define gSpringtrapJumpscareRight gPcSpringtrapJumpscare",
    "#define gPhantomFoxyRealJumpscare gPcPhantomFoxyJumpscare",
    "#define gPhantomBBRealJumpscare gPcPhantomBBJumpscare",
    "#define gPhantomFreddyRealJumpscare gPcPhantomFreddyJumpscare",
    "#define gPhantomChicaRealJumpscare gPcPhantomChicaJumpscare",
):
    require(main, marker, "PC jumpscare route")

# The obsolete Wii-U custom Chica strip must not return anywhere in source or
# the asset-preparation pipeline.
for relative in (
    "tools/convert_user_chica_png.py",
    "assets/user_visuals/phantom_chica_jumpscare.png.b64.part1",
    "assets/user_visuals/phantom_chica_jumpscare.png.b64.part2",
    "assets/user_visuals/phantom_chica_jumpscare.png.b64.part3",
    "assets/user_visuals/phantom_chica_jumpscare.png.b64.part4",
):
    if (ROOT / relative).exists():
        fail(f"obsolete custom Phantom Chica visual returned: {relative}")
if "phantom_chica_user_jumpscare" in main:
    fail("main runtime still includes the obsolete custom Phantom Chica route")

# Title remains the full 4:3 PC frame plus exact PC UI sprites; Credits is the
# only intentionally Wii-U-specific row.
for marker in (
    "content_x, 0, 640, 480",
    "&gPcCompatTitleLogoTexture",
    "&gPcCompatTitleNewGameTexture",
    "&gPcCompatTitleLoadGameTexture",
    "&gPcCompatTitleNightmareTexture",
    "&gPcCompatTitleExtraTexture",
    "&gPcCompatTitleCursorTexture",
    "&gPcCompatTitleStarTexture",
    '"CREDITS"',
):
    require(title, marker, "PC title composition")

# The final source composition must retain PC camera, character and ending layers.
for marker in (
    '#include "main_v3_parts/main_pc_camera_fidelity.inc"',
    '#include "main_v3_parts/main_pc_character_override.inc"',
    '#include "main_v3_parts/main_pc_finishing_override.inc"',
):
    require(main, marker, "final PC visual layer")

print("PC visual fidelity audit passed: camera/office geometry, hotspots, draw order, title and jumpscare routes are locked")
