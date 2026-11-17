#!/usr/bin/env python3
"""Check that every place that carries the firmware version agrees.

    python tools/check_version.py [tag]

  * WUA_FIRMWARE_VERSION in lib/WuaStationCore/src/config/Config.h ("0.4")
  * esphome: project: version in wuastation-mini.yaml ("0.4")
  * the release tag in the default components_source of wuastation-mini.yaml
    (github://WuaLeds/WuaStation-Mini-firmware@v0.4), which Home Assistant
    users get when they adopt the station
  * version in lib/WuaStationCore/library.json ("0.4.0")
  * the tag, when given (v0.4)

Exits with an error listing every mismatch.
"""

import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def main() -> int:
    firmware = re.search(r'#define WUA_FIRMWARE_VERSION "([^"]+)"', read("lib/WuaStationCore/src/config/Config.h"))
    yaml_text = read("wuastation-mini.yaml")
    project = re.search(r'^\s+version:\s*"?([0-9][^"\s]*)"?\s*$', yaml_text, re.MULTILINE)
    source = re.search(r"components_source:\s*github://\S+@v(\S+)", yaml_text)
    library = json.loads(read("lib/WuaStationCore/library.json")).get("version", "")

    version = firmware.group(1) if firmware else None
    problems = []
    if not version:
        problems.append("WUA_FIRMWARE_VERSION not found in Config.h")
    if not project or project.group(1) != version:
        problems.append(f"wuastation-mini.yaml project version is {project and project.group(1)}, expected {version}")
    if not source or source.group(1) != version:
        problems.append(f"wuastation-mini.yaml components_source points at v{source and source.group(1)}, expected v{version}")
    if version and not (library == version or library.startswith(version + ".")):
        problems.append(f"library.json version is {library}, expected {version}.x")
    if len(sys.argv) > 1:
        tag = sys.argv[1]
        if tag.lstrip("v") != version:
            problems.append(f"tag {tag} does not match firmware version {version}")

    if problems:
        for p in problems:
            print(f"::error::{p}")
        return 1
    print(f"Version {version}: Config.h, wuastation-mini.yaml and library.json agree")
    return 0


if __name__ == "__main__":
    sys.exit(main())
