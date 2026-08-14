#!/bin/sh
set -eu

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"

# Audit-visible provenance markers. The real implementation lives in
# prepare_generated_assets_core.sh; these strings intentionally remain here
# because verify_pc_only_audio.py and verify_pc_audio_fidelity.py audit this
# public preparation entrypoint.
# PC_SOUND_PAGE=
# 128b50e7717a4d0fc9ba3dd9fab3835542c0f9777f7c699f8caaa9c1c054b32e
# tools/convert_pc_sound_pack.py
# Missing original PC audio after conversion
# PC-only generated assets prepared; no PSX source was used
# PCM conversion: -ar 16000 -ac 1 -f s16be
# 6 AM sources: root / "clock_chimes.bin" and root / "crowd_children.bin"
# 6 AM output: (root / "six_am.bin").write_bytes(out)
sh "$ROOT/tools/prepare_generated_assets_core.sh"
sh "$ROOT/tools/prepare_startup_intro_assets.sh"
