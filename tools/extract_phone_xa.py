#!/usr/bin/env python3
"""Extract the original FNaF 3 PSX phone calls and 6 AM cue.

The upstream project stores its CD-XA sources in xa/inter8.zip. Depending on
how that archive was produced it can contain either the final interleaved
INTER8.XA file, the individual channel*.xa inputs, or already decoded audio.
This script accepts all three layouts and emits Wii U-ready 16 kHz mono
signed 16-bit big-endian PCM files.
"""
from __future__ import annotations

import argparse
import math
import struct
import subprocess
import tempfile
import zipfile
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class CueSpec:
    output: str
    channel: int
    start: int
    end: int
    minimum_pcm_bytes: int
    direct_names: tuple[str, ...]


CUES = (
    CueSpec("six_am.bin", 6, 0, 1664, 40000,
            ("6am.wav", "six_am.wav", "6am.mp3", "six_am.mp3")),
    CueSpec("phone_night1.bin", 6, 3160, 27560, 300000,
            ("phone dude night 1.wav", "phone_night1.wav", "night1.wav")),
    CueSpec("phone_night2.bin", 7, 0, 19240, 250000,
            ("phone dude night 2.wav", "phone_night2.wav", "night2.wav")),
    CueSpec("phone_night3.bin", 7, 20736, 32144, 150000,
            ("phone guy night 3.wav", "phone_night3.wav", "night3.wav")),
    CueSpec("phone_night4.bin", 7, 33640, 42824, 120000,
            ("phone guy night 4.wav", "phone_night4.wav", "night4.wav")),
    CueSpec("phone_night5.bin", 7, 44320, 52440, 100000,
            ("phone guy night 5.wav", "phone_night5.wav", "night5.wav")),
    CueSpec("phone_night6.bin", 8, 0, 6280, 80000,
            ("phone guy night 6.wav", "phone_night6.wav", "night6.wav")),
)


def normalise(name: str) -> str:
    return Path(name).name.lower().replace("-", "_")


def run_ffmpeg(source: Path, output: Path, minimum_pcm_bytes: int) -> bool:
    attempts = (
        ["ffmpeg", "-hide_banner", "-loglevel", "error", "-y",
         "-i", str(source), "-ar", "16000", "-ac", "1", "-f", "s16be",
         str(output)],
        ["ffmpeg", "-hide_banner", "-loglevel", "error", "-y",
         "-f", "psxstr", "-i", str(source), "-ar", "16000", "-ac", "1",
         "-f", "s16be", str(output)],
    )
    for command in attempts:
        output.unlink(missing_ok=True)
        completed = subprocess.run(command, stdout=subprocess.DEVNULL,
                                   stderr=subprocess.PIPE, check=False)
        if (completed.returncode == 0 and output.is_file() and
                output.stat().st_size >= minimum_pcm_bytes):
            return True
    output.unlink(missing_ok=True)
    return False


def sector_layout(data: bytes) -> tuple[int, int, int]:
    """Return sector size, channel byte offset and payload offset."""
    for size, channel_offset, payload_offset in (
        (2352, 17, 24),
        (2336, 1, 8),
    ):
        if len(data) >= size and len(data) % size == 0:
            return size, channel_offset, payload_offset
    raise ValueError(f"unsupported XA byte length {len(data)}")


def extract_interleaved(data: bytes, spec: CueSpec, destination: Path) -> None:
    sector_size, channel_offset, _ = sector_layout(data)
    total = len(data) // sector_size
    first = max(0, min(spec.start, total))
    last = max(first, min(spec.end, total))
    selected = bytearray()
    for sector_index in range(first, last):
        sector = data[sector_index * sector_size:(sector_index + 1) * sector_size]
        if channel_offset < len(sector) and sector[channel_offset] == spec.channel:
            selected.extend(sector)
    if not selected:
        raise ValueError(f"no channel {spec.channel} sectors in {spec.start}:{spec.end}")
    destination.write_bytes(selected)


def extract_channel(data: bytes, spec: CueSpec, destination: Path) -> None:
    sector_size, _, _ = sector_layout(data)
    total = len(data) // sector_size
    # Upstream positions are expressed in the eight-way interleaved stream.
    first = max(0, min(spec.start // 8, total))
    last = max(first + 1, min((spec.end + 7) // 8, total))
    destination.write_bytes(data[first * sector_size:last * sector_size])


def write_fallback(output: Path, six_am: bool) -> None:
    """Keep CI/build usable while making missing original audio obvious."""
    rate = 16000
    duration = 2.4 if six_am else 1.2
    samples = int(rate * duration)
    pcm = bytearray()
    for index in range(samples):
        time = index / rate
        if six_am:
            notes = (523.25, 659.25, 783.99)
            note = notes[min(2, int(time / 0.8))]
            envelope = max(0.0, 1.0 - (time % 0.8) / 0.8)
            value = int(math.sin(2.0 * math.pi * note * time) * 9000 * envelope)
        else:
            # Quiet telephone line tone, deliberately not presented as a real call.
            value = int((math.sin(2.0 * math.pi * 350.0 * time) +
                         math.sin(2.0 * math.pi * 440.0 * time)) * 1700)
        pcm.extend(struct.pack(">h", max(-32768, min(32767, value))))
    output.write_bytes(pcm)


def find_member(names: list[str], candidates: tuple[str, ...]) -> str | None:
    candidate_set = {normalise(candidate) for candidate in candidates}
    for name in names:
        if normalise(name) in candidate_set:
            return name
    return None


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("archive", type=Path)
    parser.add_argument("output_dir", type=Path)
    args = parser.parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)

    if not args.archive.is_file():
        print(f"PHONE_XA_ARCHIVE_MISSING: {args.archive}")
        for spec in CUES:
            write_fallback(args.output_dir / spec.output, spec.output == "six_am.bin")
        return

    with tempfile.TemporaryDirectory(prefix="fnaf3-xa-") as temporary_name:
        temporary = Path(temporary_name)
        with zipfile.ZipFile(args.archive) as archive:
            names = [name for name in archive.namelist() if not name.endswith("/")]
            print("PHONE_XA_MEMBERS:")
            for name in names:
                print(f"  {name}")
            archive.extractall(temporary)

        files = {normalise(str(path.relative_to(temporary))): path
                 for path in temporary.rglob("*") if path.is_file()}
        interleaved = next((path for key, path in files.items()
                            if key in {"inter8.xa", "inter8.str", "inter8.bin"}), None)

        decoded = 0
        for spec in CUES:
            output = args.output_dir / spec.output
            source: Path | None = None
            mode = ""
            try:
                direct_member = find_member(names, spec.direct_names)
                if direct_member is not None:
                    source = temporary / direct_member
                    mode = "direct"
                elif interleaved is not None:
                    source = temporary / f"{spec.output}.xa"
                    extract_interleaved(interleaved.read_bytes(), spec, source)
                    mode = "interleaved"
                else:
                    channel_source = next((path for key, path in files.items()
                                           if key == f"channel{spec.channel}.xa"), None)
                    if channel_source is not None:
                        source = temporary / f"{spec.output}.xa"
                        extract_channel(channel_source.read_bytes(), spec, source)
                        mode = f"channel{spec.channel}"
            except (OSError, ValueError) as error:
                print(f"PHONE_XA_EXTRACT_ERROR: {spec.output}: {error}")
                source = None

            if (source is not None and
                    run_ffmpeg(source, output, spec.minimum_pcm_bytes)):
                decoded += 1
                print(f"PHONE_XA_DECODED: {spec.output} via {mode} "
                      f"({output.stat().st_size} bytes)")
            else:
                write_fallback(output, spec.output == "six_am.bin")
                print(f"PHONE_XA_FALLBACK: {spec.output}; provide a decoded WAV/MP3 source")

        print(f"PHONE_XA_RESULT: {decoded}/{len(CUES)} original cues decoded")


if __name__ == "__main__":
    main()
