<p align="center">
  <img src="docs/images/wualeds-logo.svg" alt="Wualeds" width="64">
</p>

<h1 align="center">WuaStation-mini firmware</h1>

<p align="center">
  <b>Clock, weather, crypto and sensor readings on an 18 x 7 LED matrix.</b><br>
  Works on its own, and joins Home Assistant through ESPHome when you want it to.
</p>

<p align="center">
  <a href="https://github.com/WuaLeds/WuaStation-Mini-firmware/actions/workflows/ci.yml"><img alt="CI" src="https://github.com/WuaLeds/WuaStation-Mini-firmware/actions/workflows/ci.yml/badge.svg"></a>
  <a href="https://github.com/WuaLeds/WuaStation-Mini-firmware/releases/latest"><img alt="Release" src="https://img.shields.io/github/v/release/WuaLeds/WuaStation-Mini-firmware?color=e8348c&label=release"></a>
  <a href="LICENSE"><img alt="License: MIT" src="https://img.shields.io/badge/license-MIT-f6604a"></a>
  <a href="https://wualeds.com/firmware"><img alt="Flash from the browser" src="https://img.shields.io/badge/flash-from%20the%20browser-fb8b24"></a>
  <br>
  <img alt="ESPHome" src="https://img.shields.io/badge/ESPHome-2026.10-000000?logo=esphome&logoColor=white">
  <img alt="Home Assistant" src="https://img.shields.io/badge/Home%20Assistant-ready-18BCF2?logo=homeassistant&logoColor=white">
  <img alt="ESP32-C3" src="https://img.shields.io/badge/MCU-ESP32--C3-f0466a?logo=espressif&logoColor=white">
  <img alt="Code style: clang-format" src="https://img.shields.io/badge/code%20style-clang--format-6b5d68">
</p>

<p align="center">
  <a href="#-features">Features</a> ·
  <a href="#%EF%B8%8F-screens">Screens</a> ·
  <a href="#-buttons">Buttons</a> ·
  <a href="#-setup">Setup</a> ·
  <a href="#-home-assistant">Home Assistant</a> ·
  <a href="#%EF%B8%8F-build--flash">Build</a> ·
  <a href="#-releases">Releases</a> ·
  <a href="CHANGELOG.md">Changelog</a>
</p>

---

> [!TIP]
> **No tools needed:** install or update the firmware straight from the browser at
> [wualeds.com/firmware](https://wualeds.com/firmware). Your WiFi and settings are kept,
> also when updating from 0.3.

## ✨ Features

| | |
| --- | --- |
| 🕒 **Clock and date** | Synced over the internet, with timezone and daylight saving time |
| 🌤️ **Weather and forecast** | Open-Meteo: now, today's high/low, rain, UV index and air quality |
| 🏠 **Indoor sensor** | BME280, SHT4x, SHT3x or AHT2x on I2C, detected automatically |
| 💰 **Crypto ticker** | Up to 4 Binance pairs: name, price and 24 h change in green or red |
| ⏳ **Timer and pomodoro** | Countdowns from 1 to 90 min and 25/5/15 pomodoro, driven with a button |
| 💬 **Messages and alarm** | Your own scrolling text, a daily (visual) alarm and on-screen alerts |
| ⚙️ **Settings page** | Wualeds-branded page served by the board: screens, order, times, city, everything |
| 🌍 **66 preset cities** | City and timezone picked from a world list, or automatic from the city |
| 🏡 **Home Assistant** | Found on the network, added in one click: display, message, screens, sensors, buttons |

The firmware is an [ESPHome](https://esphome.io) build: ESPHome brings WiFi,
the setup portal, the Home Assistant API, OTA updates and logs, and the
station itself (screens, data, settings page) runs inside it as the
`wuastation` component. Without Home Assistant nothing changes: the station
keeps working on its own. Text is drawn with the
[WuaDisplay-LMX](https://github.com/WuaLeds/WuaDisplay-LMX) library
(`WUA_BOARD_LMX1P` backend).

## 🖥️ Screens

Which screens rotate, in which order and for how long is set from the
[settings page](#%EF%B8%8F-settings-page) or from Home Assistant; screens
without data are skipped automatically. Always present: **Splash**
("WuaStation" once at boot) and **Status** (WiFi icon and progress while
connecting, red cross when offline).

| | Id | Content | Default time |
| :---: | --- | --- | --- |
| 🕒 | `time` | `HH:MM` with blinking colon (SNTP, timezone + DST) | 10 s |
| 📅 | `date` | `DD.MM` | 4 s |
| 🌤️ | `weather` | Condition icon (day/night) + temperature, then humidity | 8 s |
| ☂️ | `forecast` | Today: ↑ max, ↓ min, ☂ chance of rain, `UV` index, `AQ` air quality, colored by band | 10 s, shared |
| 🏠 | `indoor` | I2C sensor: temperature, humidity, pressure (BME280) | 9 s |
| 💰 | `ticker` | Fixed pages per pair: name `BTC` → price `97.2K` → 24 h change `+2.3%` | 6.5 s per pair |
| 💬 | `message` | Custom text (settings page or Home Assistant) | one pass |
| 🎉 | `celebration` | Monthly anniversary message (from WuaDisplay-r1) | one pass |
| ⏳ | `timer` | Countdown / pomodoro (see [Timer](#%EF%B8%8F-timer)) | 4 s while idle |

**Interrupt screens** take over the rotation, highest first:
🚨 Alarm › 🔔 Alert and notifications › 📶 WiFi setup › ⏳ running Timer.

<details>
<summary>🎨 <b>How text fits 18 pixels</b></summary>

- Static screens use a compact 3x7 font (`display/TinyFont`) so `12:34` fits,
  and crossfade into each other with the library effects engine.
- Ticker prices get as many decimals as fit (`3457`, `145.7`, `0.457`) and are
  abbreviated when needed (`97.2K`, at most 1 % rounding). The rare page that
  still does not fit (`MATIC`, `0.00001234`) pans slowly instead of scrolling
  away. Page timings are in `Config::kTicker*`.
- Scrolling text uses the classic 5x7 GFX font with UTF-8 to CP437 conversion
  (accents, ñ, inverted marks, degree sign).
- Forecast values are colored by their own scale: temperature, UV band and
  air-quality band (US AQI or European index, chosen on the settings page).

</details>

## 🎮 Buttons

| Input | Click | Long press |
| --- | --- | --- |
| 🔼 **UP** | Next screen | WiFi info: network and address (or the setup WiFi while offline) |
| 🔽 **DOWN** | Brightness step (saved) | Device info (IP, host name, version, Home Assistant) |
| 👆 **TOUCH** | Next screen | Device info |

- Any click dismisses an alert; any button stops the alarm.
- On the timer screen DOWN drives the timer instead (below).
- Every press also reaches Home Assistant as an event, for automations.
- 🧹 **Factory reset**: hold DOWN while powering on (3 s, shows `RST` then `OK`).
  It erases the settings and the WiFi network.

### ⏱️ Timer

Go to the timer screen with UP, then:

| State | DOWN click | DOWN long | UP / TOUCH click |
| --- | --- | --- | --- |
| ⏳ Idle (hourglass or 🍅 + minutes) | Next preset: 1, 3, 5, 10, 15, 20, 30, 45, 60, 90 min, pomodoro | ▶️ Start | Next screen |
| ▶️ Running (`MM:SS`, stays on screen) | ⏸️ Pause / resume (blinks while paused) | ⏹️ Stop | Hide it (keeps running) |
| 🔔 Finished (flashing `00:00`) | Any button acknowledges | | |

The pomodoro runs 25 min of work (red) and 5 min breaks (green), with a 15 min
break after every 4th block, and goes on by itself until stopped. Times are in
`Config::kPomodoro*` / `Config::kTimerPresetsMin`.

## 📶 Setup

1. On first boot the board opens the WiFi network **`WuaStation-XXXX`**
   (password `WuaLeds2025`) and scrolls the instructions.
2. Join it from your phone: the setup page opens by itself (or open
   `http://192.168.4.1`). Pick your network and type its password.
3. Once connected, the settings page is served on your network at
   **`http://wuastation-mini-xxxxxx.local`** (or the IP shown with a DOWN long
   press).

Right after flashing from the browser, the network can also be set over USB
(Improv). If the saved network disappears (new router, moving house) the
station opens `WuaStation-XXXX` again by itself after a minute; *Change WiFi
network* on the settings page does it on purpose.

> [!NOTE]
> **Updating from 0.3** keeps everything: the WiFi network is moved into
> ESPHome on the first boot and the settings stay where they were (same
> partition table). The host name changes from `wuastation-xxxx.local` to
> `wuastation-mini-xxxxxx.local`.

### ⚙️ Settings page

A page with the Wualeds look (logo, colors of wualeds.com, light and dark
mode, Spanish and English) served by the board itself, with no internet
needed. Changes apply at once; only a new city or new ticker pairs restart the
board.

| Section | What you can set |
| --- | --- |
| 🖥️ **Screens** | Every screen with an on/off switch, ▲▼ arrows for the order and its time in seconds (empty = default; for `ticker`, per pair) |
| 📍 **Location and time** | City and timezone from 66 well-known cities grouped by continent (zones listed west to east with their UTC offset), or *Other…* to type any city (`Rosario,AR`) or zone. *Advanced*: latitude / longitude and the air-quality index (US / European) |
| 💰 **Ticker** | Up to 4 Binance pairs (`BTCUSDT, ETHUSDT`); empty hides the screen |
| 🔔 **Alarm and message** | Daily alarm (switch + time) and the custom scrolling text |
| ☀️ **Display and sensor** | Brightness (applied live while dragging) and a temperature offset for the sensor (±10 °C, enclosure heat) |
| 🔧 **Device** | Firmware version, host name, uptime, Home Assistant connection, change WiFi network, restart |

The header also shows the device clock, WiFi network, IP and the sensor
reading, refreshed every 10 s.

> [!NOTE]
> **Timezone** *Automatic* (`auto`, the default) follows the weather city, so
> choosing the city is enough. It also accepts an Olson name, city or country
> from [`Timezones.cpp`](lib/WuaStationCore/src/net/Timezones.cpp)
> (`Europe/Madrid`, `Buenos Aires`, `Tokio`, `Argentina`; case, spaces and
> accents do not matter) or any POSIX TZ string (`<-03>3`).

<details>
<summary>🧩 <b>How the page is built</b></summary>

- Source: [`web/settings.html`](web/settings.html), plain HTML, CSS and JS with
  no build step. The preset cities live in its `CITIES` table; each geocoding
  query (`Name,CC`) and timezone was checked against the Open-Meteo geocoding
  API.
- The `wuastation` component embeds it gzipped (~12 KB) at build time and
  serves it on ESPHome's web server
  ([`settings_web.cpp`](components/wuastation/settings_web.cpp)), plus a small
  JSON API: `/api/settings`, `/api/status`, `/api/brightness`, `/api/restart`,
  `/api/wifi/forget`.
- The rotation is stored as `time:10,date:4,-weather,ticker:9,...` (display
  order, optional seconds, `-` = hidden).

</details>

## 🏡 Home Assistant

The station shows up by itself in **Settings › Devices & services** (ESPHome
discovery); click *Configure* and it is added. Nothing else to install. Its
device page reads **Wualeds · WuaStation mini** with the firmware version.

Not listed? Add the **ESPHome** integration by hand with the station's IP
(DOWN long press), port `6053` and no encryption key. Discovery needs Home
Assistant on the same network as the station: in a virtual machine, use a
bridged network adapter rather than NAT.

| Entity | Type | What it does |
| --- | --- | --- |
| Display | 💡 light | On/off and brightness of the matrix |
| Message | ✏️ text | The custom scrolling message (empty hides it) |
| Screen | 📋 select | Screen on display; choosing one jumps to it |
| *Time screen* … *Timer screen* | 🔀 switch ×9 | Whether each screen is part of the rotation |
| Next screen, Restart, Factory reset | 🔘 button | Factory reset is disabled by default |
| Button UP / DOWN / TOUCH | ⚡ event | `click` and `long_press` of the physical buttons |
| Indoor temperature, humidity, pressure | 🌡️ sensor | From the I2C sensor |
| Outdoor temperature, humidity, chance of rain, UV index, air quality | 🌤️ sensor | From the weather service |
| WiFi signal, uptime, IP address, WiFi network | 🩺 diagnostic | |

Every change is mirrored both ways: what you change on the settings page or
with the buttons shows up in Home Assistant, and the other way round.

**Actions** (Developer tools › Actions):

| Action | Data | |
| --- | --- | --- |
| `esphome.wuastation_mini_xxxxxx_show_message` | `message: "Doorbell"` | Scrolls a notification over any screen |
| `esphome.wuastation_mini_xxxxxx_show_screen` | `screen: "weather"` | Jumps to a screen |

<details>
<summary>🤖 <b>Automation examples</b></summary>

```yaml
# Doorbell -> message on the station
automation:
  - alias: Doorbell on the WuaStation
    triggers:
      - trigger: state
        entity_id: binary_sensor.doorbell
        to: "on"
    actions:
      - action: esphome.wuastation_mini_3f7590_show_message
        data:
          message: "Ring ring!"

  # Dim the display at night
  - alias: WuaStation night mode
    triggers:
      - trigger: time
        at: "23:00:00"
    actions:
      - action: light.turn_on
        target:
          entity_id: light.wuastation_mini_3f7590_display
        data:
          brightness_pct: 5

  # Long press on TOUCH toggles the living room lamp
  - alias: WuaStation TOUCH button
    triggers:
      - trigger: state
        entity_id: event.wuastation_mini_3f7590_button_touch
        attribute: event_type
        to: long_press
    actions:
      - action: light.toggle
        target:
          entity_id: light.living_room
```

</details>

**ESPHome dashboard**: the station advertises its configuration
(`dashboard_import`), so the ESPHome add-on offers to *adopt* it. Adopting
creates a small YAML that pulls [`wuastation-mini.yaml`](wuastation-mini.yaml)
from this repository; from there you can add an API encryption key or your own
entities and update the station over the air.

## 🛠️ Build & flash

The firmware is built with ESPHome (Python 3.11+):

```sh
pip install esphome==2026.10.2
esphome -s components_source components run wuastation-mini.yaml       # build + flash (USB or OTA)
esphome -s components_source components logs wuastation-mini.yaml      # logs
```

`-s components_source components` builds the `wuastation` component from this
folder; without it ESPHome fetches the released one from GitHub (that is what
Home Assistant users get). Add `-s log_level DEBUG` for detailed logs.

| Folder | What |
| --- | --- |
| [`wuastation-mini.yaml`](wuastation-mini.yaml) | ESPHome configuration of the product (WiFi, API, entities) |
| [`components/wuastation/`](components/wuastation) | ESPHome component: hosts the core, settings page, Home Assistant entities |
| [`lib/WuaStationCore/`](lib/WuaStationCore) | Firmware core (PlatformIO library): screens, data, sensors, settings |
| [`src/`](src) + [`platformio.ini`](platformio.ini) | `c3_bench`: FPS benchmark of the LED pipeline with plain PlatformIO |

> [!WARNING]
> **Windows**: run ESPHome from PowerShell or cmd (ESP-IDF tools refuse Git
> Bash / MSYS), and keep ESPHome's PlatformIO apart from any other PlatformIO
> install with `PLATFORMIO_CORE_DIR` (it installs its own platform versions):
>
> ```powershell
> $env:PLATFORMIO_CORE_DIR = "$HOME\.platformio-esphome"
> ```

<details>
<summary>🧪 <b>Testing without Home Assistant</b></summary>

[`tools/api_smoke_test.py`](tools/api_smoke_test.py) talks to a station with
`aioesphomeapi`, the library behind Home Assistant's ESPHome integration: it
checks the device info, every entity and action, drives the light, message,
screen select, switches and buttons, verifies each change through the
settings API and puts everything back.

```sh
pip install aioesphomeapi
python tools/api_smoke_test.py wuastation-mini-xxxxxx.local
```

</details>

### 🎨 Code style

The sources follow [`.clang-format`](.clang-format) (clang-format 23.1.5), and
the [CI](.github/workflows/ci.yml) fails when a file does not match. To format
before committing:

```sh
pip install clang-format==23.1.5
git ls-files 'lib/*.h' 'lib/*.cpp' 'components/*.h' 'components/*.cpp' 'src/*.h' 'src/*.cpp' | xargs clang-format -i
```

Hand-aligned tables are kept between `// clang-format off` and
`// clang-format on`.

## 🚀 Releases

Each GitHub release is also published on
[wualeds.com/firmware](https://wualeds.com/firmware), the page that installs the
firmware from the browser. To publish a version:

1. Go through the [release test checklist](docs/TESTING.md) on real hardware.
2. Set the version everywhere it appears and push:
   `WUA_FIRMWARE_VERSION` in `lib/WuaStationCore/src/config/Config.h`,
   `project: version` and the tag in `components_source` in
   `wuastation-mini.yaml`, `version` in `lib/WuaStationCore/library.json`, and
   a new entry in [`CHANGELOG.md`](CHANGELOG.md).
   [`tools/check_version.py`](tools/check_version.py) checks they agree (CI runs it).
3. Create a release with the tag (`v0.4`). Write the notes as two bullet lists
   under the headings `## English` and `## Español`: the website shows each
   one in its language.
4. The [Release](.github/workflows/release.yml) workflow builds the firmware
   with ESPHome, attaches the files the web flasher installs (`bootloader.bin`,
   `partitions.bin`, `boot_app0.bin`, `firmware.bin` and a `manifest.json`
   with each part's offset, size and SHA-256, made by
   [`tools/release_assets.py`](tools/release_assets.py)) plus a merged
   `.factory.bin`, and calls the `VERCEL_DEPLOY_HOOK` secret so the website is
   rebuilt with the new version.

The workflow stops if the versions do not match the tag. To rebuild an
existing tag, run it by hand from the Actions tab (*Run workflow*).

## 🔌 Hardware

<details>
<summary>📌 <b>Pinout</b> (from <code>PCB/WuaStation-Mini/WuaStation-Mini.kicad_sch</code>, see <a href="lib/WuaStationCore/src/board/Pins.h"><code>Pins.h</code></a>)</summary>

| Net | GPIO | Use |
| --- | --- | --- |
| DIN | IO5 | LMX1p data (126 x SK6805-EC15) |
| UP | IO9 | Button to GND. **BOOT strap**: held at reset = download mode |
| DOWN | IO4 | Button to GND |
| TOUCH | IO7 | USB-C SBU1/SBU2, 2k pull-down, active HIGH (e.g. TTP223) |
| SDA / SCL | IO3 / IO6 | I2C expansion connector J1 (pull-ups on the breakout) |
| MCC1 | IO0 + IO20 | USB-C CC1 via JP1 (bridged), kept high-impedance |
| MCC2 | IO1 + IO21 | USB-C CC2 via JP2 (bridged), kept high-impedance |
| D- / D+ | IO18 / IO19 | Native USB (logs and Improv over USB Serial/JTAG) |
| RST | EN | SW1 |

The LMX1p chain order was checked against the LMX1p KiCad files: the board
used in landscape (rotated 90° clockwise) matches the library mapping 1:1 for
all 126 LEDs.

</details>

> [!IMPORTANT]
> - **Alarm is visual only**: the board has no buzzer or I2S amplifier, so the
>   MP3 siren of WuaDisplay-r1 is replaced by a flashing alarm screen
>   (`Config::kAlarmRingMs`).
> - **I2C pull-ups**: the board has none; use a sensor breakout that includes them.
> - **USB-C CC jumpers**: with JP1/JP2 bridged, IO0/IO20 and IO1/IO21 sit on the
>   CC lines. The firmware parks them as inputs at boot, but the ROM bootloader
>   drives IO21 (UART0 TX) high for a moment on every reset, which may confuse
>   some USB-C chargers. Cut the jumpers if those pins are not needed.

## 🗂️ Project structure

<details>
<summary>📁 <b>Source tree</b></summary>

```
wuastation-mini.yaml         ESPHome configuration of the product
components/wuastation/       ESPHome component
├── __init__.py              configuration, settings page embedding, libraries
├── wuastation.*             WuaStation: hosts the core (Host), mirrors it to Home Assistant
├── settings_web.*           settings page + JSON API on ESPHome's web server
├── entities.h               light, text, select, switch, button classes
├── legacy_wifi.*            moves the WiFi network saved by 0.3 into ESPHome
├── sensor.py light.py text.py select.py switch.py button.py event.py
└── partitions.csv           same table as 0.3 (settings survive an update)
lib/WuaStationCore/src/      firmware core (no ESPHome inside)
├── app/                     Station (composition root, buttons), Host (what the host provides)
├── board/                   Pins.h (schematic pinout), Board.* (bring-up, device id)
├── config/                  Config.h (defaults/tuning), Theme.h, Strings.h, ScreenConfig
├── core/                    Logger, Settings (NVS), SharedState (task-safe snapshots)
├── display/                 Display.h (WuaDisplay-LMX), Compositor, TinyFont, Icons, Widgets,
│                            ColorScale, TextUtil (UTF-8 -> CP437)
├── input/                   Button (debounce, click / long press)
├── net/                     TimeService (SNTP), Timezones, NetWorker (background HTTP task),
│                            HttpJson (esp_http_client, verified TLS)
├── providers/               ticker (Binance), weather (Open-Meteo), sensors (SHT4x, SHT3x, AHT2x, BME280)
├── states/                  DisplayController, base classes + one file per screen
└── web/                     SettingsJson (settings page API, transport-free)
web/settings.html            settings page source (embedded at build time)
src/bench/                   FPS benchmark (PlatformIO env c3_bench)
tools/                       check_version.py, release_assets.py, api_smoke_test.py
docs/images/                 README images
```

</details>

<details>
<summary>⚙️ <b>How it fits together</b></summary>

- **ESPHome** runs WiFi (station, setup access point, captive portal, Improv),
  mDNS, the Home Assistant API, OTA and the logger.
- **`WuaStation`** (the component) implements the core's `Host` interface on
  top of ESPHome, serves the settings page and turns Home Assistant commands
  into calls on the core, and the core's changes into entity states.
- **Main loop** (never blocks): buttons, sensor sampling, `DisplayController`
  (screen rotation + library scroll / crossfade).
- **Network task** (`NetWorker`): runs every internet job one after another
  while WiFi is up and publishes results through `SharedState<T>`, so HTTPS
  requests never freeze the display.
- **Web server task**: the settings page reads a snapshot of the settings and
  hands every change to the main loop.

</details>

### 🧩 Extending

- 🖼️ **New screen**: derive from `StaticStateBase` (implement `render()`),
  `PagedStateBase` (fixed pages: `beginPages()` / `renderPage()`) or
  `ScrollTextStateBase` (implement `nextPass()`). Give it an id in
  `config/ScreenConfig` (name, title and default time), map it in
  `Station::screen()`, add its name and description to the `I18N` tables of
  `web/settings.html`, and its switch and select option in
  `components/wuastation/switch.py` / `select.py`. Return `priority() > 0` for
  an interrupt screen.
- 🌐 **New internet source**: implement `INetJob` (blocking `run()` returning
  the next delay), publish a plain struct with `SharedState<T>`, add it to the
  `NetWorker` in `Station::setup()`.
- 🌡️ **New I2C sensor**: implement `ISensor` (probe / start / read) and add it
  to `SensorManager::_candidates`.
- 🏡 **New Home Assistant entity**: add a platform file in
  `components/wuastation/` (see `sensor.py`) and publish it from
  `WuaStation`.

> [!NOTE]
> HTTPS servers are verified against the ESP-IDF certificate bundle. Binance
> falls back to `data-api.binance.vision` if `api.binance.com` is blocked in
> your region.

## 📄 License

[MIT](LICENSE) © 2026 Wualabs LTD
