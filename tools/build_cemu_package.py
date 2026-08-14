#!/usr/bin/env python3

from __future__ import annotations

import argparse
import shutil
import sys
import zipfile
from pathlib import Path

from PIL import Image, ImageOps, UnidentifiedImageError


def open_rgb(source: Path):
    try:
        with Image.open(source) as image:
            image.load()
            return image.convert("RGB")
    except (FileNotFoundError, UnidentifiedImageError, OSError) as exc:
        print(f"warning: cannot decode {source}: {exc}", file=sys.stderr)
        return None


def fit_or_fallback(source: Path, size: tuple[int, int], icon: Path | None = None):
    try:
        resampling = Image.Resampling.LANCZOS
    except AttributeError:
        resampling = Image.LANCZOS

    image = open_rgb(source)
    if image is not None:
        return ImageOps.fit(image, size, method=resampling, centering=(0.5, 0.5))

    image = Image.new("RGB", size, (0, 0, 0))
    if icon is not None:
        icon_image = open_rgb(icon)
        if icon_image is not None:
            max_side = min(size) // 3
            icon_image.thumbnail((max_side, max_side), resampling)
            x = (size[0] - icon_image.width) // 2
            y = (size[1] - icon_image.height) // 2
            image.paste(icon_image, (x, y))
    return image


def write_install_readme(target: Path):
    target.write_text(
        "Five Nights at Freddy's 3 - Wii U Edition | Cemu package\n\n"
        "Recommended installation:\n"
        "1. Extract this ZIP.\n"
        "2. Open Cemu.\n"
        "3. File -> Install game title, update or DLC.\n"
        "4. Select fnaf3-wiiu-cemu/meta/meta.xml.\n"
        "5. Launch Five Nights at Freddy's 3 - Wii U Edition from Cemu's game list.\n\n"
        "Alternative direct launch:\n"
        "File -> Load, then select fnaf3-wiiu-cemu/code/fnaf3-wiiu.rpx.\n\n"
        "The installed-title route is recommended because it gives the game a normal Wii U title context and native save path inside Cemu's MLC.\n"
        "Title ID: 000500001337F3A3\n",
        encoding="utf-8",
    )


def main() -> int:
    parser = argparse.ArgumentParser(description="Build a decrypted Cemu-ready Wii U title package")
    parser.add_argument("--root", default=".", help="repository root")
    parser.add_argument("--rpx", default="fnaf3-wiiu.rpx", help="built RPX path relative to root")
    parser.add_argument("--version", default="v1.0", help="package version used in ZIP filename")
    args = parser.parse_args()

    root = Path(args.root).resolve()
    rpx = (root / args.rpx).resolve()
    if not rpx.is_file():
        raise SystemExit(f"error: missing RPX: {rpx}")

    dist = root / "dist"
    package = dist / "fnaf3-wiiu-cemu"
    code = package / "code"
    content = package / "content"
    meta = package / "meta"

    if package.exists():
        shutil.rmtree(package)
    dist.mkdir(parents=True, exist_ok=True)
    code.mkdir(parents=True)
    content.mkdir(parents=True)
    meta.mkdir(parents=True)

    shutil.copy2(rpx, code / "fnaf3-wiiu.rpx")
    shutil.copy2(root / "wup" / "app.xml", code / "app.xml")
    shutil.copy2(root / "wup" / "cos.xml", code / "cos.xml")
    shutil.copy2(root / "wup" / "meta.xml", meta / "meta.xml")

    (content / "channel.txt").write_text(
        "Five Nights at Freddy's 3 - Wii U Edition\n",
        encoding="utf-8",
    )

    icon = root / "icon.jpg"
    fit_or_fallback(icon, (128, 128)).save(meta / "iconTex.tga", format="TGA")
    fit_or_fallback(root / "boot-tv.jpg", (1280, 720), icon).save(
        meta / "bootTvTex.tga", format="TGA"
    )
    fit_or_fallback(root / "boot-drc.jpg", (854, 480), icon).save(
        meta / "bootDrcTex.tga", format="TGA"
    )

    write_install_readme(package / "CEMU-INSTALL.txt")

    required = [
        code / "fnaf3-wiiu.rpx",
        code / "app.xml",
        code / "cos.xml",
        content / "channel.txt",
        meta / "meta.xml",
        meta / "iconTex.tga",
        meta / "bootTvTex.tga",
        meta / "bootDrcTex.tga",
        package / "CEMU-INSTALL.txt",
    ]
    missing = [str(path.relative_to(root)) for path in required if not path.is_file()]
    if missing:
        raise SystemExit("error: incomplete Cemu package: " + ", ".join(missing))

    app_text = (code / "app.xml").read_text(encoding="utf-8")
    meta_text = (meta / "meta.xml").read_text(encoding="utf-8")
    title_id = "000500001337F3A3"
    if title_id not in app_text or title_id not in meta_text:
        raise SystemExit("error: Cemu package title ID does not match the Wii U Channel")

    zip_path = dist / f"fnaf3-wiiu-{args.version}-cemu.zip"
    if zip_path.exists():
        zip_path.unlink()

    with zipfile.ZipFile(zip_path, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for path in sorted(package.rglob("*")):
            if path.is_file():
                arcname = Path(package.name) / path.relative_to(package)
                archive.write(path, arcname.as_posix())

    print(f"Cemu package ready: {zip_path}")
    print(f"Install metadata: {package / 'meta' / 'meta.xml'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
