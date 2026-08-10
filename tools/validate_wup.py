#!/usr/bin/env python3
"""Validate the WUP package produced by NUSPacker before publishing it.

This performs structural checks that catch the common "invalid title.tmd" class
of packaging mistakes without needing to decrypt or launch the title.
"""

from __future__ import annotations

import hashlib
import struct
import sys
from pathlib import Path
import xml.etree.ElementTree as ET

TMD_HEADER_SIZE = 0x204
CONTENT_INFO_COUNT = 0x40
CONTENT_INFO_SIZE = 0x24
CONTENT_INFO_TABLE_SIZE = CONTENT_INFO_COUNT * CONTENT_INFO_SIZE
CONTENT_TABLE_OFFSET = TMD_HEADER_SIZE + CONTENT_INFO_TABLE_SIZE
CONTENT_RECORD_SIZE = 0x30

TMD_SIGNATURE_TYPE_OFFSET = 0x000
TMD_SYSTEM_VERSION_OFFSET = 0x184
TMD_TITLE_ID_OFFSET = 0x18C
TMD_GROUP_ID_OFFSET = 0x198
TMD_APP_TYPE_OFFSET = 0x19A
TMD_TITLE_VERSION_OFFSET = 0x1DC
TMD_CONTENT_COUNT_OFFSET = 0x1DE
TMD_CONTENT_INFO_HASH_OFFSET = 0x1E4

TICKET_SIGNATURE_TYPE_OFFSET = 0x000
TICKET_TITLE_ID_OFFSET = 0x1DC

EXPECTED_SIGNATURE_TYPE = 0x00010004
# NUSPacker Content.TYPE_HASHED. Only contents with this TMD type bit have .h3 trees.
CONTENT_TYPE_HASHED = 0x0002


def fail(message: str) -> None:
    raise SystemExit(f"WUP validation failed: {message}")


def read_xml_value(path: Path, tag: str) -> str:
    try:
        root = ET.parse(path).getroot()
    except (OSError, ET.ParseError) as exc:
        fail(f"cannot parse {path}: {exc}")
    node = root.find(tag)
    if node is None or node.text is None or not node.text.strip():
        fail(f"missing <{tag}> in {path}")
    return node.text.strip()


def read_be16(data: bytes, offset: int) -> int:
    return struct.unpack_from(">H", data, offset)[0]


def read_be32(data: bytes, offset: int) -> int:
    return struct.unpack_from(">I", data, offset)[0]


def read_be64(data: bytes, offset: int) -> int:
    return struct.unpack_from(">Q", data, offset)[0]


def main() -> int:
    if len(sys.argv) != 4:
        print(
            "usage: validate_wup.py <wup-output-dir> <app.xml> <meta.xml>",
            file=sys.stderr,
        )
        return 2

    output_dir = Path(sys.argv[1])
    app_xml = Path(sys.argv[2])
    meta_xml = Path(sys.argv[3])

    if not output_dir.is_dir():
        fail(f"output directory does not exist: {output_dir}")

    expected_title_id = int(read_xml_value(app_xml, "title_id"), 16)
    meta_title_id = int(read_xml_value(meta_xml, "title_id"), 16)
    if expected_title_id != meta_title_id:
        fail(
            f"title ID differs between app.xml ({expected_title_id:016X}) "
            f"and meta.xml ({meta_title_id:016X})"
        )
    if (expected_title_id >> 32) != 0x00050000:
        fail(f"unexpected application title ID: {expected_title_id:016X}")

    expected_system_version = int(read_xml_value(app_xml, "os_version"), 16)
    expected_group_id = int(read_xml_value(app_xml, "group_id"), 16)
    meta_group_id = int(read_xml_value(meta_xml, "group_id"), 16)
    if expected_group_id != meta_group_id:
        fail(
            f"group ID differs between app.xml ({expected_group_id:08X}) "
            f"and meta.xml ({meta_group_id:08X})"
        )

    expected_app_type = int(read_xml_value(app_xml, "app_type"), 16)
    expected_title_version = int(read_xml_value(app_xml, "title_version"), 16)
    meta_title_version = int(read_xml_value(meta_xml, "title_version"), 10)
    if expected_title_version != meta_title_version:
        fail(
            f"title version differs between app.xml ({expected_title_version}) "
            f"and meta.xml ({meta_title_version})"
        )

    tmd_path = output_dir / "title.tmd"
    tik_path = output_dir / "title.tik"
    cert_path = output_dir / "title.cert"
    for path in (tmd_path, tik_path, cert_path):
        if not path.is_file() or path.stat().st_size == 0:
            fail(f"missing or empty {path.name}")

    tmd = tmd_path.read_bytes()
    ticket = tik_path.read_bytes()

    if len(tmd) < CONTENT_TABLE_OFFSET:
        fail(f"title.tmd is too small ({len(tmd)} bytes)")
    if len(ticket) <= TICKET_TITLE_ID_OFFSET + 8:
        fail(f"title.tik is too small ({len(ticket)} bytes)")

    if read_be32(tmd, TMD_SIGNATURE_TYPE_OFFSET) != EXPECTED_SIGNATURE_TYPE:
        fail("title.tmd has an unexpected signature type")
    if read_be32(ticket, TICKET_SIGNATURE_TYPE_OFFSET) != EXPECTED_SIGNATURE_TYPE:
        fail("title.tik has an unexpected signature type")

    tmd_title_id = read_be64(tmd, TMD_TITLE_ID_OFFSET)
    ticket_title_id = read_be64(ticket, TICKET_TITLE_ID_OFFSET)
    if tmd_title_id != expected_title_id:
        fail(
            f"TMD title ID {tmd_title_id:016X} does not match XML "
            f"{expected_title_id:016X}"
        )
    if ticket_title_id != expected_title_id:
        fail(
            f"ticket title ID {ticket_title_id:016X} does not match XML "
            f"{expected_title_id:016X}"
        )

    if read_be64(tmd, TMD_SYSTEM_VERSION_OFFSET) != expected_system_version:
        fail("TMD system version does not match app.xml")
    if read_be16(tmd, TMD_GROUP_ID_OFFSET) != (expected_group_id & 0xFFFF):
        fail("TMD group ID does not match app.xml")
    if read_be32(tmd, TMD_APP_TYPE_OFFSET) != expected_app_type:
        fail("TMD app type does not match app.xml")
    if read_be16(tmd, TMD_TITLE_VERSION_OFFSET) != expected_title_version:
        fail("TMD title version does not match app.xml")

    content_count = read_be16(tmd, TMD_CONTENT_COUNT_OFFSET)
    if content_count == 0:
        fail("title.tmd declares zero contents")

    expected_tmd_size = CONTENT_TABLE_OFFSET + content_count * CONTENT_RECORD_SIZE
    if len(tmd) != expected_tmd_size:
        fail(
            f"title.tmd size is inconsistent with its content count: "
            f"got {len(tmd)}, expected {expected_tmd_size} for {content_count} contents"
        )

    # NUSPacker stores SHA-256(content-info table) in the TMD header.
    content_info_table = tmd[TMD_HEADER_SIZE:CONTENT_TABLE_OFFSET]
    stored_info_hash = tmd[
        TMD_CONTENT_INFO_HASH_OFFSET : TMD_CONTENT_INFO_HASH_OFFSET + 0x20
    ]
    actual_info_hash = hashlib.sha256(content_info_table).digest()
    if stored_info_hash != actual_info_hash:
        fail("TMD content-info SHA-256 is invalid")

    content_table = tmd[
        CONTENT_TABLE_OFFSET : CONTENT_TABLE_OFFSET + content_count * CONTENT_RECORD_SIZE
    ]

    # NUSPacker's first ContentInfo entry covers the full content table.
    first_info_count = read_be16(tmd, TMD_HEADER_SIZE + 2)
    if first_info_count != content_count:
        fail(
            f"TMD ContentInfo count ({first_info_count}) does not match "
            f"content count ({content_count})"
        )
    stored_content_table_hash = tmd[TMD_HEADER_SIZE + 4 : TMD_HEADER_SIZE + 0x24]
    if stored_content_table_hash != hashlib.sha256(content_table).digest():
        fail("TMD content table SHA-256 is invalid")

    declared_ids: set[int] = set()
    declared_indexes: set[int] = set()
    expected_app_names: set[str] = set()
    expected_h3_names: set[str] = set()
    hashed_content_count = 0

    for index in range(content_count):
        offset = CONTENT_TABLE_OFFSET + index * CONTENT_RECORD_SIZE
        content_id = read_be32(tmd, offset)
        content_index = read_be16(tmd, offset + 4)
        content_type = read_be16(tmd, offset + 6)
        encrypted_size = read_be64(tmd, offset + 8)
        is_hashed = (content_type & CONTENT_TYPE_HASHED) != 0

        if content_id in declared_ids:
            fail(f"duplicate content ID {content_id:08X} in title.tmd")
        if content_index in declared_indexes:
            fail(f"duplicate content index {content_index} in title.tmd")
        if encrypted_size == 0:
            fail(f"content {content_id:08X} has a zero size in title.tmd")

        declared_ids.add(content_id)
        declared_indexes.add(content_index)

        app_name = f"{content_id:08X}.app"
        app_path = output_dir / app_name
        expected_app_names.add(app_name)

        if not app_path.is_file():
            fail(f"TMD references missing content file {app_name}")
        if app_path.stat().st_size != encrypted_size:
            fail(
                f"{app_name} size ({app_path.stat().st_size}) does not match "
                f"TMD ({encrypted_size})"
            )

        # NUSPacker emits an H3 tree only when Content.TYPE_HASHED (0x0002)
        # is set in the TMD content type. Non-hashed contents legitimately have
        # no .h3 file and are protected by the SHA-1 stored in the TMD record.
        if is_hashed:
            hashed_content_count += 1
            h3_name = f"{content_id:08X}.h3"
            h3_path = output_dir / h3_name
            expected_h3_names.add(h3_name)
            if not h3_path.is_file() or h3_path.stat().st_size == 0:
                fail(f"missing or empty hash tree {h3_name} for hashed content")

    actual_app_names = {path.name for path in output_dir.glob("*.app")}
    actual_h3_names = {path.name for path in output_dir.glob("*.h3")}
    if actual_app_names != expected_app_names:
        fail(".app files do not exactly match the contents declared by title.tmd")
    if actual_h3_names != expected_h3_names:
        fail(".h3 files do not exactly match the hashed contents declared by title.tmd")

    if cert_path.stat().st_size < 0x100:
        fail("title.cert is unexpectedly small")

    print("WUP validation OK")
    print(f"  Title ID : {expected_title_id:016X}")
    print(f"  Contents : {content_count} ({hashed_content_count} hashed)")
    print(f"  TMD size : {len(tmd)} bytes")
    print(f"  Ticket   : {len(ticket)} bytes")
    print(f"  Cert     : {cert_path.stat().st_size} bytes")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
