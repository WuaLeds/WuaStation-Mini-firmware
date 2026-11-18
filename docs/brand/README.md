# Wualeds as a brand in ESPHome and Home Assistant

How the WuaStation mini gets listed as a **Wualeds** product: on
[devices.esphome.io](https://devices.esphome.io/made-for-esphome/) with the
*Made for ESPHome* badge, and in Home Assistant's *Add integration* list with
the Wualeds logo (choosing it opens the ESPHome integration).

Home Assistant already shows **Wualeds / WuaStation mini** on the device page
(`esphome: project: name: Wualeds.WuaStation mini`); these steps add the
brand to the integration list and the ESPHome device catalogue.

| Step | Where | What | Depends on |
| --- | --- | --- | --- |
| 1 | this repository | v0.4 merged and released: the configuration is public on `main`, and the release carries `esphome-update.json` and `firmware.bin` | |
| 2 | [esphome/devices.esphome.io](https://github.com/esphome/devices.esphome.io) | Device page with `made-for-esphome: true`, plus an email asking to use the logo | 1 |
| 3 | [home-assistant/brands](https://github.com/home-assistant/brands) | Images in `core_integrations/wualeds/` | |
| 4 | [home-assistant/core](https://github.com/home-assistant/core) | Virtual integration `wualeds` supported by `esphome` | 2 approved |
| 5 | [home-assistant/home-assistant.io](https://github.com/home-assistant/home-assistant.io) | Integration page `wualeds` | 4 |

Every brand Home Assistant lists on top of ESPHome today (Apollo Automation,
Everything Smart Home, IoTorero, Konnected) is in *Made for ESPHome* or
*Works with Home Assistant*, so step 2 comes first. Home Assistant's
maintainers decide whether to accept steps 3 to 5.

## 1. Made for ESPHome requirements

Checked against the [checklist](https://github.com/esphome/devices.esphome.io/blob/main/.github/made-for-esphome-checklist.md)
and the automated review (`scripts/review-made-for-esphome.ts`):

- [x] ESP32-C3, ships with ESPHome, configuration public
      ([`wuastation-mini.yaml`](../../wuastation-mini.yaml)).
- [x] `improv_ble:` (WiFi from a phone) and `improv_serial:` (USB-C port).
- [x] `dashboard_import:`, `esphome: project:` name and version, `api:`,
      `ota: esphome`, serial flashing enabled.
- [x] No secrets, no passwords, no static IP in the configuration.
- [x] An `id:` on every entity and component.
- [x] Updates through `update: http_request`, reading
      `releases/latest/download/esphome-update.json` (written by
      [`tools/release_assets.py`](../../tools/release_assets.py)).

## 2. devices.esphome.io

New file `src/docs/devices/Wualeds-WuaStation-mini/index.md`, plus a product
photo (`wuastation-mini.webp`, needed: none in this repository yet):

````markdown
---
title: Wualeds WuaStation mini
date-published: 2026-11-18
type: misc
standard: global
board: esp32
project-url: https://github.com/WuaLeds/WuaStation-Mini-firmware/blob/main/wuastation-mini.yaml
difficulty: 1
made-for-esphome: true
---

![Wualeds WuaStation mini](wuastation-mini.webp "Wualeds WuaStation mini")

Maker: [https://wualeds.com](https://wualeds.com)

## Description

Desk station with an 18 x 7 RGB LED matrix (126 SK6805-EC15 LEDs) on an
ESP32-C3: clock, date, weather and forecast, indoor sensor (BME280, SHT4x,
SHT3x or AHT2x on I2C), crypto ticker, messages, timer and visual alarm. It
works on its own with its settings page, and in Home Assistant it exposes the
display as a light, the message, the screen rotation, the buttons as events
and every reading as a sensor.

WiFi is set from a phone over Bluetooth (Improv), over USB (Improv serial) or
from the setup access point. New firmware is offered by the update entity.

## GPIO Pinout

| Pin    | Function                                        |
| ------ | ----------------------------------------------- |
| GPIO5  | LED matrix data (126 x SK6805-EC15)             |
| GPIO9  | Button UP (also BOOT)                           |
| GPIO4  | Button DOWN                                     |
| GPIO7  | TOUCH input (USB-C SBU1/SBU2, active high)      |
| GPIO3  | I2C SDA (expansion connector)                   |
| GPIO6  | I2C SCL (expansion connector)                   |
| GPIO18 | USB D- (logs, Improv serial)                    |
| GPIO19 | USB D+                                          |

## Configuration

```yaml url=https://github.com/WuaLeds/WuaStation-Mini-firmware/blob/main/wuastation-mini.yaml
```
````

Then email esphome@openhomefoundation.org with the pull request link, asking
to use the Made for ESPHome logo. The pull request runs an automated review
(it downloads the configuration, builds it with the latest ESPHome and checks
the update manifest); approval is the `made-for-esphome-approved` label or the
merge.

## 3. home-assistant/brands

Copy [`home-assistant/`](home-assistant) to `core_integrations/wualeds/`:
`icon.png` (256 x 256), `icon@2x.png` (512 x 512), `logo.png` (128 px high),
`logo@2x.png` (256 px high), `dark_logo.png`, `dark_logo@2x.png`. Made by
[`tools/brand_images.py`](../../tools/brand_images.py) from the symbol in
[`wualeds-logo.svg`](../images/wualeds-logo.svg).

## 4. home-assistant/core

`homeassistant/components/wualeds/__init__.py`:

```python
"""Virtual integration: Wualeds."""
```

`homeassistant/components/wualeds/manifest.json`:

```json
{
  "domain": "wualeds",
  "name": "Wualeds",
  "integration_type": "virtual",
  "supported_by": "esphome"
}
```

Then `python -m script.hassfest` regenerates
`homeassistant/generated/integrations.json` (the `wualeds` entry next to the
other brands). Same shape as the Apollo Automation pull request
(home-assistant/core#139751).

## 5. home-assistant.io

`source/_integrations/wualeds.markdown`:

```markdown
---
title: Wualeds
description: Connect and control your Wualeds devices using the ESPHome integration
ha_release: 2026.12
ha_domain: wualeds
ha_integration_type: virtual
ha_supporting_domain: esphome
ha_supporting_integration: ESPHome
works_with:
  - local
---

[Wualeds](https://wualeds.com) is a member of the Made for ESPHome program.

Wualeds devices work locally and integrate seamlessly with the [ESPHome](/integrations/esphome/) {% term integration %} in Home Assistant. As all connectivity is done locally, status updates and device control from Home Assistant happen instantly.

{% include integrations/supported_brand.md %}
```
