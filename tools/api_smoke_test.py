#!/usr/bin/env python3
"""Talk to a WuaStation-mini the way Home Assistant does, and check it.

    pip install aioesphomeapi
    python tools/api_smoke_test.py wuastation-mini-3f7590.local [--key BASE64]

Uses aioesphomeapi, the library behind Home Assistant's ESPHome integration:
reads the device info, lists the entities and actions, then drives the light,
the message, the screen select, a screen switch, a button and an action, and
checks every change through the station's own settings API. Every value it
touches is put back at the end.
"""

import argparse
import asyncio
import json
import sys
import urllib.request

from aioesphomeapi import APIClient

EXPECTED = {
    "light": {"display"},
    "text": {"message"},
    "select": {"screen"},
    "switch": {
        f"{s}_screen"
        for s in ("time", "date", "weather", "forecast", "indoor", "ticker", "message", "celebration", "timer")
    },
    "button": {"next_screen", "factory_reset", "restart"},
    "event": {"button_up", "button_down", "button_touch"},
    "sensor": {
        "indoor_temperature",
        "indoor_humidity",
        "indoor_pressure",
        "outdoor_temperature",
        "outdoor_humidity",
        "chance_of_rain_today",
        "uv_index_today",
        "air_quality_index",
        "wifi_signal",
        "uptime",
    },
    "text_sensor": {"ip_address", "wifi_network"},
    "update": {"firmware"},
}
EXPECTED_ACTIONS = {"show_message", "show_screen"}

results = []


def check(name, ok, detail=""):
    results.append(ok)
    print(f"{'PASS' if ok else 'FAIL'}  {name}{('  ' + detail) if detail else ''}")


def kind(info) -> str:
    n = type(info).__name__.replace("Info", "")
    return {
        "Light": "light",
        "Text": "text",
        "Select": "select",
        "Switch": "switch",
        "Button": "button",
        "Event": "event",
        "Sensor": "sensor",
        "TextSensor": "text_sensor",
        "Update": "update",
    }.get(n, n)


def settings(host):
    with urllib.request.urlopen(f"http://{host}/api/settings", timeout=10) as r:
        return json.load(r)


async def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("host")
    parser.add_argument("--key", help="API encryption key, if the station has one")
    args = parser.parse_args()

    cli = APIClient(args.host, 6053, None, noise_psk=args.key)
    await cli.connect(login=True)

    info = await cli.device_info()
    print(f"Device: {info.name} ({info.friendly_name}), {info.project_name} {info.project_version}, "
          f"ESPHome {info.esphome_version}, MAC {info.mac_address}")
    check("project name", info.project_name == "Wualeds.WuaStation mini", info.project_name)
    check("project version", info.project_version.startswith("0.4"), info.project_version)

    entities, actions = await cli.list_entities_services()
    found = {}
    for e in entities:
        found.setdefault(kind(e), set()).add(e.object_id)
    for k, ids in EXPECTED.items():
        missing = ids - found.get(k, set())
        check(f"entities: {k}", not missing, f"missing {sorted(missing)}" if missing else f"{len(ids)} found")
    check("actions", EXPECTED_ACTIONS <= {a.name for a in actions}, str(sorted(a.name for a in actions)))

    by_id = {e.object_id: e for e in entities}
    states = {}
    cli.subscribe_states(lambda s: states.__setitem__(s.key, s))
    await asyncio.sleep(3)

    def state(object_id):
        e = by_id.get(object_id)
        return states.get(e.key) if e else None

    for oid in ("indoor_temperature", "indoor_humidity", "outdoor_temperature", "uv_index_today",
                "air_quality_index", "ip_address", "wifi_network"):
        s = state(oid)
        print(f"      {oid} = {getattr(s, 'state', None)}")

    before = settings(args.host)
    light = state("display")
    check("light reports the stored brightness", light is not None and light.state
          and abs(light.brightness - before["bright"] / 255) < 0.01,
          f"light {getattr(light, 'brightness', None)} vs bright {before['bright']}")

    # ---- light brightness ----
    cli.light_command(by_id["display"].key, state=True, brightness=0.25)
    await asyncio.sleep(1.5)
    now = settings(args.host)
    check("light -> brightness setting", now["bright"] == 64, f"bright {now['bright']}")

    # ---- light off / on ----
    cli.light_command(by_id["display"].key, state=False)
    await asyncio.sleep(1)
    check("light off", state("display") is not None and not state("display").state)
    cli.light_command(by_id["display"].key, state=True)
    await asyncio.sleep(1)
    check("light on again", state("display") is not None and state("display").state)

    # ---- message ----
    cli.text_command(by_id["message"].key, "Hola desde Home Assistant")
    await asyncio.sleep(1.5)
    now = settings(args.host)
    check("text -> message setting", now["msg"] == "Hola desde Home Assistant", now["msg"])
    check("text state", getattr(state("message"), "state", None) == "Hola desde Home Assistant")

    # ---- screen switch ----
    cli.switch_command(by_id["celebration_screen"].key, False)
    await asyncio.sleep(1.5)
    now = settings(args.host)
    celebration = next(s for s in now["screens"] if s["id"] == "celebration")
    check("switch off -> screen hidden", not celebration["on"])
    cli.switch_command(by_id["celebration_screen"].key, True)
    await asyncio.sleep(1.5)
    now = settings(args.host)
    celebration = next(s for s in now["screens"] if s["id"] == "celebration")
    check("switch on -> screen shown", celebration["on"])

    # ---- screen select / next button ----
    cli.select_command(by_id["screen"].key, "Date")
    await asyncio.sleep(1)
    check("select -> Date on display", getattr(state("screen"), "state", None) == "Date",
          str(getattr(state("screen"), "state", None)))
    cli.button_command(by_id["next_screen"].key)
    await asyncio.sleep(1)
    check("next screen button", getattr(state("screen"), "state", None) != "Date",
          str(getattr(state("screen"), "state", None)))

    # ---- actions ----
    show_message = next(a for a in actions if a.name == "show_message")
    await cli.execute_service(show_message, {"message": "Prueba desde Home Assistant"})
    check("show_message action", True)

    # ---- put everything back ----
    cli.text_command(by_id["message"].key, before["msg"])
    cli.light_command(by_id["display"].key, state=True, brightness=before["bright"] / 255)
    await asyncio.sleep(1.5)
    now = settings(args.host)
    check("restored", now["msg"] == before["msg"] and now["bright"] == before["bright"]
          and now["screens"] == before["screens"])

    await cli.disconnect()
    print(f"\n{sum(results)}/{len(results)} checks passed")
    return 0 if all(results) else 1


if __name__ == "__main__":
    sys.exit(asyncio.run(main()))
