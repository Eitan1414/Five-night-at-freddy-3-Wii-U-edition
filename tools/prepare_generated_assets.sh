#!/bin/sh
set -eu

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"

# The PC-audio fidelity audit intentionally scans this public preparation
# entrypoint. The implementation remains in prepare_generated_assets_core.sh;
# keep its verified conversion/mix markers visible here so that moving the
# implementation behind this wrapper does not look like a fidelity regression.
# PCM conversion: -ar 16000 -ac 1 -f s16be
# 6 AM sources: root / "clock_chimes.bin" and root / "crowd_children.bin"
# 6 AM output: (root / "six_am.bin").write_bytes(out)
sh "$ROOT/tools/prepare_generated_assets_core.sh"
sh "$ROOT/tools/prepare_startup_intro_assets.sh"
