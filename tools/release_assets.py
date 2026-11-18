#!/usr/bin/env python3
"""Collect a PlatformIO build into the files the web flasher installs.

    python tools/release_assets.py <tag> [env=wuastation-mini] [out=dist] [metadata=.esphome/idedata/wuastation-mini.json]

Run it from the repository root after
`esphome -s components_source components compile wuastation-mini.yaml`.
<metadata> is the build description ESPHome writes (idedata: application,
flash images and offsets); a PlatformIO project folder also works, then
`pio project metadata -e <env>` is asked instead.
It copies the bootloader, partition table, boot_app0 and the application to
<out>/ and writes <out>/manifest.json with the flash offset, size and SHA-256
of each part. ESPHome's merged image (flash at 0x0, erases the settings) is
attached too, as <board>-<tag>.factory.bin, for esptool users, and
<out>/esphome-update.json: the manifest the station's update entity reads
(ESPHome http_request update: chip family, application URL and MD5), so Home
Assistant offers each new release.

The release workflow attaches these files to the GitHub release; wualeds.com
downloads them when it is built, and its /firmware page checks each part
against the manifest and writes it at its offset (NVS is left alone, so WiFi
and settings survive an update). Set PIO to use a specific `pio` executable.
"""

import hashlib
import json
import os
import shutil
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

BOARD = "wuastation-mini"  # product slug on wualeds.com
PRODUCT = "WuaStation mini"
CHIP_FAMILY = "ESP32-C3"
REPO = os.environ.get("GITHUB_REPOSITORY", "WuaLeds/WuaStation-Mini-firmware")


def app_offset(meta, images):
    """First app partition in partitions.bin (32-byte entries, magic AA 50)."""
    if meta["extra"].get("application_offset"):
        return int(meta["extra"]["application_offset"], 16)
    table = next((path for _, path in images if path.name == "partitions.bin"), None)
    if table:
        data = table.read_bytes()
        for i in range(0, len(data) - 31, 32):
            if data[i : i + 2] != b"\xaa\x50":
                break  # end of table
            if data[i + 2] == 0x00:  # type 0 = app
                return int.from_bytes(data[i + 4 : i + 8], "little")
    return 0x10000


def git_commit():
    try:
        commit = subprocess.check_output(["git", "rev-parse", "--short", "HEAD"], text=True).strip()
        dirty = subprocess.check_output(["git", "status", "--porcelain", "--untracked-files=no"], text=True)
        return commit + ("-dirty" if dirty.strip() else "")
    except (OSError, subprocess.CalledProcessError):
        return ""


def main():
    if len(sys.argv) < 2:
        sys.exit("usage: python tools/release_assets.py <tag> [env] [out] [project]")
    tag = sys.argv[1]
    env = sys.argv[2] if len(sys.argv) > 2 else "wuastation-mini"
    out = Path(sys.argv[3] if len(sys.argv) > 3 else "dist").resolve()
    source = Path(sys.argv[4] if len(sys.argv) > 4 else ".esphome/idedata/wuastation-mini.json").resolve()
    if source.is_file():
        meta = json.loads(source.read_text(encoding="utf-8"))
    else:
        pio = os.environ.get("PIO", "pio")
        output = subprocess.check_output(
            [pio, "project", "metadata", "-e", env, "--json-output"], text=True, cwd=source
        )
        meta = json.loads(output[output.index("{"):])[env]  # skip build chatter
    images = [(int(img["offset"], 16), Path(img["path"])) for img in meta["extra"]["flash_images"]]
    app = Path(meta["prog_path"]).with_suffix(".bin")
    images.append((app_offset(meta, images), app))

    out.mkdir(parents=True, exist_ok=True)
    parts = []
    for offset, path in sorted(images):
        data = path.read_bytes()
        shutil.copyfile(path, out / path.name)
        parts.append(
            {"file": path.name, "offset": offset, "size": len(data), "sha256": hashlib.sha256(data).hexdigest()}
        )

    factory = Path(meta["prog_path"]).with_name("firmware.factory.bin")
    if factory.is_file():
        shutil.copyfile(factory, out / f"{BOARD}-{tag}.factory.bin")

    manifest = {
        "board": BOARD,
        "version": tag,
        "commit": git_commit(),
        "env": env,
        "builtAt": datetime.now(timezone.utc).strftime("%Y-%m-%d"),
        "parts": parts,
    }
    (out / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")

    # The station reads releases/latest/download/esphome-update.json and offers
    # the update when its version differs from the running one, so the version
    # is the project version: "0.4", not "v0.4".
    version = tag[1:] if tag.startswith("v") else tag
    update = {
        "name": PRODUCT,
        "version": version,
        "builds": [
            {
                "chipFamily": CHIP_FAMILY,
                "ota": {
                    "path": f"https://github.com/{REPO}/releases/download/{tag}/{app.name}",
                    "md5": hashlib.md5(app.read_bytes()).hexdigest(),
                    "summary": f"{PRODUCT} firmware {version}",
                    "release_url": f"https://github.com/{REPO}/releases/tag/{tag}",
                },
            }
        ],
    }
    (out / "esphome-update.json").write_text(json.dumps(update, indent=2) + "\n", encoding="utf-8")

    for p in parts:
        print(f"0x{p['offset']:05x}  {p['file']}  ({p['size']} bytes)")
    print(f"manifest.json, esphome-update.json -> {out}/ ({tag}, commit {manifest['commit'] or '?'})")


if __name__ == "__main__":
    main()
