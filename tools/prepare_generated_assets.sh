#!/bin/sh
set -eu

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"

sh "$ROOT/tools/prepare_generated_assets_core.sh"
sh "$ROOT/tools/prepare_startup_intro_assets.sh"
