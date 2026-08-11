#!/usr/bin/env python3
"""Convert the original FNaF 3 PC Sound Effects ZIP to Wii U PCM overrides.

The script does not redistribute the WAV files.  It consumes a user-supplied
archive and writes 16 kHz mono signed-16-bit big-endian PCM files matching the
runtime paths used by the Wii U port.
"""

from __future__ import annotations

import argparse
import shutil
import subprocess
import tempfile
import zipfile
from pathlib import Path


RENAME = {
    "Clocks_Chimes_Cl_02480702.wav": "clock_chimes.bin",
    "CROWD_SMALL_CHIL_EC049202.wav": "crowd_children.bin",
    "Desolate_Underworld2.wav": "desolate_underworld.bin",
    "PartyFavorraspyPart_AC01__3.wav": "party_favor.bin",
    "crazy garble.wav": "crazy_garble.bin",
    "long glitched2.wav": "long_glitched2.bin",
    "night1final.wav": "phone_night1.bin",
    "night2final2.wav": "phone_night2.bin",
    "night3final.wav": "phone_night3.bin",
    "night4final.wav": "phone_night4.bin",
    "night5final.wav": "phone_night5.bin",
    "night6final.wav": "phone_night6.bin",
}


def output_name(name: str) -> str:
    base = Path(name).name
    if base in RENAME:
        return RENAME[base]
    stem = Path(base).stem.lower().replace(" ", "_")
    return f"{stem}.bin"


def convert(ffmpeg: str, source: Path, destination: Path) -> None:
    destination.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run(
        [
            ffmpeg,
            "-hide_banner",
            "-loglevel",
            "error",
            "-y",
            "-i",
            str(source),
            "-ar",
            "16000",
            "-ac",
            "1",
            "-f",
            "s16be",
            str(destination),
        ],
        check=True,
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("archive", type=Path, help="Sound Effects ZIP from the PC game")
    parser.add_argument("output", type=Path, help="output directory (usually SD:/wiiu/apps/fnaf3-wiiu/audio)")
    parser.add_argument("--ffmpeg", default="ffmpeg")
    args = parser.parse_args()

    if shutil.which(args.ffmpeg) is None:
        raise SystemExit("ffmpeg is required")

    written: list[str] = []
    with tempfile.TemporaryDirectory(prefix="fnaf3-pc-sfx-") as tmp_name:
        tmp = Path(tmp_name)
        with zipfile.ZipFile(args.archive) as archive:
            wav_members = [n for n in archive.namelist() if n.lower().endswith(".wav")]
            for member in wav_members:
                source = tmp / Path(member).name
                with archive.open(member) as src, source.open("wb") as dst:
                    shutil.copyfileobj(src, dst)
                destination = args.output / output_name(member)
                convert(args.ffmpeg, source, destination)
                written.append(destination.name)

    manifest = args.output / "pc_sound_pack_manifest.txt"
    manifest.write_text("\n".join(sorted(written)) + "\n", encoding="utf-8")
    print(f"Converted {len(written)} PC WAV files to {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
