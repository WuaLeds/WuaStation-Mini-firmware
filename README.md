<p align="center">
  <img src="docs/images/wualeds-logo.svg" alt="Wualeds" width="64">
</p>

<h1 align="center">WuaStation-mini firmware</h1>

<p align="center">
  <b>Clock, weather, crypto and sensor readings on an 18 x 7 LED matrix.</b><br>
  PlatformIO firmware for the WuaStation-mini (ESP32-C3) with the LMX1p panoramic matrix.
</p>

<p align="center">
  <a href="https://github.com/WuaLeds/WuaStation-Mini-firmware/actions/workflows/ci.yml"><img alt="CI" src="https://github.com/WuaLeds/WuaStation-Mini-firmware/actions/workflows/ci.yml/badge.svg"></a>
  <a href="https://github.com/WuaLeds/WuaStation-Mini-firmware/releases/latest"><img alt="Release" src="https://img.shields.io/github/v/release/WuaLeds/WuaStation-Mini-firmware?color=e8348c&label=release"></a>
  <a href="LICENSE"><img alt="License: MIT" src="https://img.shields.io/badge/license-MIT-f6604a"></a>
  <a href="https://wualeds.com/firmware"><img alt="Flash from the browser" src="https://img.shields.io/badge/flash-from%20the%20browser-fb8b24"></a>
  <br>
  <img alt="ESP32-C3" src="https://img.shields.io/badge/MCU-ESP32--C3-f0466a?logo=espressif&logoColor=white">
  <img alt="PlatformIO" src="https://img.shields.io/badge/PlatformIO-pioarduino%2055.03.37-ffa830?logo=platformio&logoColor=white">
  <img alt="Arduino-ESP32 3.3.7" src="https://img.shields.io/badge/Arduino--ESP32-3.3.7-00979D?logo=arduino&logoColor=white">
  <img alt="Code style: clang-format" src="https://img.shields.io/badge/code%20style-clang--format-6b5d68">
</p>

<p align="center">
  <a href="#-features">Features</a> ·
  <a href="#%EF%B8%8F-screens">Screens</a> ·
  <a href="#-buttons">Buttons</a> ·
  <a href="#-setup">Setup</a> ·
  <a href="#%EF%B8%8F-build--flash">Build</a> ·
  <a href="#-releases">Releases</a> ·
  <a href="CHANGELOG.md">Changelog</a>
</p>

---

> [!TIP]
> **No tools needed:** install or update the firmware straight from the browser at
> [wualeds.com/firmware](https://wualeds.com/firmware). Your WiFi and settings are kept.

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

It carries over every feature of `WuaDisplay-r1-firmware` and adapts it to the
18 x 7 canvas, drawn through the
[WuaDisplay-LMX](https://github.com/WuaLeds/WuaDisplay-LMX) library
(`WUA_BOARD_LMX1P` backend).

## 🖥️ Screens

Which screens rotate, in which order and for how long is set from the
[settings page](#%EF%B8%8F-settings-page); screens without data are skipped
automatically. Always present: **Splash** ("WuaStation" once at boot) and
**Status** (WiFi icon and progress while connecting, red cross when offline).

| | Id | Content | Default time |
| :---: | --- | --- | --- |
| 🕒 | `time` | `HH:MM` with blinking colon (SNTP, timezone + DST) | 10 s |
| 📅 | `date` | `DD.MM` | 4 s |
| 🌤️ | `weather` | Condition icon (day/night) + temperature, then humidity | 8 s |
| ☂️ | `forecast` | Today: ↑ max, ↓ min, ☂ chance of rain, `UV` index, `AQ` air quality, colored by band | 10 s, shared |
| 🏠 | `indoor` | I2C sensor: temperature, humidity, pressure (BME280) | 9 s |
| 💰 | `ticker` | Fixed pages per pair: name `BTC` → price `97.2K` → 24 h change `+2.3%` | 6.5 s per pair |
| 💬 | `message` | Custom text from the settings page | one pass |
| 🎉 | `celebration` | Monthly anniversary message (from WuaDisplay-r1) | one pass |
| ⏳ | `timer` | Countdown / pomodoro (see [Timer](#%EF%B8%8F-timer)) | 4 s while idle |

**Interrupt screens** take over the rotation, highest first:
🚨 Alarm › 🔔 Alert › 📶 WiFi portal › ⏳ running Timer.

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
| 🔼 **UP** | Next screen | Open / close the WiFi setup portal |
| 🔽 **DOWN** | Brightness step (saved) | Device info (IP, host name, version) |
| 👆 **TOUCH** | Next screen | Device info |

- Any click dismisses an alert; any button stops the alarm.
- On the timer screen DOWN drives the timer instead (below).
- 🧹 **Factory reset**: hold DOWN while powering on (3 s, shows `RST` then `OK`).

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

1. On first boot (no WiFi saved) the board opens the access point
   **`WuaStation-XXXX`** (password `WuaLeds2025`, see `Config::kPortalPassword`)
   and scrolls the instructions.
2. Join it and open **`http://192.168.4.1`**: the settings page opens;
   **WiFi setup** picks your network.
3. Once connected, the same page is served on your network at
   **`http://wuastation-xxxx.local`** (or the IP shown with a DOWN long press),
   so the settings can be changed any time.

### ⚙️ Settings page

A page with the Wualeds look (logo, colors of wualeds.com, light and dark
mode, Spanish and English) served by the board itself, with no internet
needed. Saving stores everything in NVS and restarts the board.

| Section | What you can set |
| --- | --- |
| 🖥️ **Screens** | Every screen with an on/off switch, ▲▼ arrows for the order and its time in seconds (empty = default; for `ticker`, per pair) |
| 📍 **Location and time** | City and timezone from 66 well-known cities grouped by continent (zones listed west to east with their UTC offset), or *Other…* to type any city (`Rosario,AR`) or zone. *Advanced*: latitude / longitude and the air-quality index (US / European) |
| 💰 **Ticker** | Up to 4 Binance pairs (`BTCUSDT, ETHUSDT`); empty hides the screen |
| 🔔 **Alarm and message** | Daily alarm (switch + time) and the custom scrolling text |
| ☀️ **Display and sensor** | Brightness (applied live while dragging) and a temperature offset for the sensor (±10 °C, enclosure heat) |
| 🔧 **Device** | Firmware version, host name, uptime, WiFi setup, restart |

The header also shows the device clock, WiFi network, IP and the sensor
reading, refreshed every 10 s.

> [!NOTE]
> **Timezone** *Automatic* (`auto`, the default) follows the weather city, so
> choosing the city is enough. It also accepts an Olson name, city or country
> from [`src/net/Timezones.cpp`](src/net/Timezones.cpp) (`Europe/Madrid`,
> `Buenos Aires`, `Tokio`, `Argentina`; case, spaces and accents do not
> matter) or any POSIX TZ string (`<-03>3`).

<details>
<summary>🧩 <b>How the page is built</b></summary>

- Source: [`web/settings.html`](web/settings.html), plain HTML, CSS and JS with
  no build step. The preset cities live in its `CITIES` table; each geocoding
  query (`Name,CC`) and timezone was checked against the Open-Meteo geocoding
  API.
- [`tools/embed_web.py`](tools/embed_web.py) runs before every build and embeds
  it gzipped (~12 KB) as `SettingsPageData.h`, generated inside the build
  directory (`.pio/build/<env>/generated/`, never in `src/`).
- [`src/web/SettingsServer`](src/web/SettingsServer.h) serves it on
  WiFiManager's web server, plus a small JSON API: `/api/settings`,
  `/api/status`, `/api/brightness`, `/api/restart`.
- The rotation is stored as `time:10,date:4,-weather,ticker:9,...` (display
  order, optional seconds, `-` = hidden).

</details>

## 🛠️ Build & flash

```sh
pio run -e c3_release                 # build
pio run -e c3_release -t upload       # flash over USB
pio device monitor                    # serial monitor (c3_debug for logs)
```

| Environment | Purpose |
| --- | --- |
| 🚀 `c3_release` | Production build (default) |
| 🐞 `c3_debug` | Logs over USB-CDC (`WUA_ENABLE_DEBUG`) + built-in USB-JTAG debugging |
| 📊 `c3_bench` | FPS benchmark of the LED pipeline, WiFi off |

Toolchain: pioarduino `55.03.37` (Arduino-ESP32 core 3.3.7). The
WuaDisplay-LMX library is pinned to a commit in `platformio.ini`; to develop
the library side by side, point `lib_deps` to a local `symlink://` path.

> [!WARNING]
> If `pio` from the system PATH fails with a `littlefs` import error, use the
> PlatformIO Core bundled with the VS Code extension
> (`~/.platformio/penv/Scripts/pio`), or build from the VS Code PlatformIO UI.

### 🎨 Code style

The sources follow [`.clang-format`](.clang-format) (clang-format 23.1.5), and
the [CI](.github/workflows/ci.yml) fails when a file does not match. To format
before committing:

```sh
pip install clang-format==23.1.5
git ls-files 'src/*.h' 'src/*.cpp' | xargs clang-format -i
```

Hand-aligned tables are kept between `// clang-format off` and
`// clang-format on`.

## 🚀 Releases

Each GitHub release is also published on
[wualeds.com/firmware](https://wualeds.com/firmware), the page that installs the
firmware from the browser. To publish a version:

1. Set `WUA_FIRMWARE_VERSION` in `src/config/Config.h` (e.g. `"0.3"`), add the
   version to [`CHANGELOG.md`](CHANGELOG.md) and push.
2. Create a release with the tag `v0.3`. Write the notes as two bullet lists
   under the headings `## English` and `## Español`: the website shows each
   one in its language.
3. The [Release](.github/workflows/release.yml) workflow builds `c3_release`,
   attaches the files the web flasher installs (`bootloader.bin`,
   `partitions.bin`, `boot_app0.bin`, `firmware.bin` and a `manifest.json`
   with each part's offset, size and SHA-256, made by
   [`tools/release_assets.py`](tools/release_assets.py)) and calls the
   `VERCEL_DEPLOY_HOOK` secret so the website is rebuilt with the new version.

The workflow stops if `WUA_FIRMWARE_VERSION` does not match the tag. To rebuild
an existing tag, run it by hand from the Actions tab (*Run workflow*).

## 🔌 Hardware

<details>
<summary>📌 <b>Pinout</b> (from <code>PCB/WuaStation-Mini/WuaStation-Mini.kicad_sch</code>, see <a href="src/board/Pins.h"><code>src/board/Pins.h</code></a>)</summary>

| Net | GPIO | Use |
| --- | --- | --- |
| DIN | IO5 | LMX1p data (126 x SK6805-EC15) |
| UP | IO9 | Button to GND. **BOOT strap**: held at reset = download mode |
| DOWN | IO4 | Button to GND |
| TOUCH | IO7 | USB-C SBU1/SBU2, 2k pull-down, active HIGH (e.g. TTP223) |
| SDA / SCL | IO3 / IO6 | I2C expansion connector J1 (pull-ups on the breakout) |
| MCC1 | IO0 + IO20 | USB-C CC1 via JP1 (bridged), kept high-impedance |
| MCC2 | IO1 + IO21 | USB-C CC2 via JP2 (bridged), kept high-impedance |
| D- / D+ | IO18 / IO19 | Native USB (Serial = USB-CDC) |
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
src/
├── main.cpp                 entry point (App, or Benchmark in c3_bench)
├── app/App.*                composition root: services, screens, buttons
├── board/                   Pins.h (schematic pinout), Board.* (bring-up, device id)
├── config/                  Config.h (defaults/tuning), Theme.h (colors), Strings.h (UI texts),
│                            ScreenConfig (screen rotation setting)
├── core/                    Logger, Settings (NVS), SharedState (task-safe snapshots)
├── display/                 Display.h (WuaDisplay-LMX), Compositor (frames + crossfade),
│                            TinyFont, Icons, Widgets (layouts, labels),
│                            ColorScale, TextUtil (UTF-8 -> CP437)
├── input/                   Button (debounce, click / long press)
├── net/                     WifiService (WiFiManager portal, /wifi page), TimeService (SNTP),
│                            Timezones, NetWorker (background HTTP task), HttpJson
├── providers/
│   ├── ticker/              BinanceTickerProvider
│   ├── weather/             OpenMeteoWeatherProvider
│   └── sensors/             SensorManager + SHT4x, SHT3x, AHT2x, BME280 drivers
├── states/                  DisplayController, base classes (Static / Paged / ScrollText)
│                            + one file per screen
├── web/                     SettingsServer (settings page + JSON API)
└── bench/                   FPS benchmark
web/settings.html            settings page source (embedded at build time)
tools/                       embed_web.py (pre-build), release_assets.py (release workflow)
docs/images/                 README images
```

</details>

<details>
<summary>⚙️ <b>How it fits together</b></summary>

- **Main loop** (never blocks): WiFi state machine, buttons, sensor sampling,
  `DisplayController` (screen rotation + library scroll / crossfade) and the
  settings page.
- **Network task** (`NetWorker`): runs every internet job one after another
  while WiFi is up and publishes results through `SharedState<T>`, so HTTPS
  requests never freeze the display.

</details>

### 🧩 Extending

- 🖼️ **New screen**: derive from `StaticStateBase` (implement `render()`),
  `PagedStateBase` (fixed pages: `beginPages()` / `renderPage()`) or
  `ScrollTextStateBase` (implement `nextPass()`). Give it an id in
  `config/ScreenConfig` (name and default time), map it in `App::screen()` and
  add its name and description to the `I18N` tables of `web/settings.html`, so
  it can be placed from the settings page. Return `priority() > 0` for an
  interrupt screen.
- 🌐 **New internet source**: implement `INetJob` (blocking `run()` returning
  the next delay), publish a plain struct with `SharedState<T>`, add it to the
  `NetWorker` in `App::setup()`.
- 🌡️ **New I2C sensor**: implement `ISensor` (probe / start / read) and add it
  to `SensorManager::_candidates`.

> [!CAUTION]
> HTTPS is used without certificate validation (public, read-only APIs). Do not
> reuse `HttpJson` for anything that sends secrets. Binance falls back to
> `data-api.binance.vision` if `api.binance.com` is blocked in your region.

## 📄 License

[MIT](LICENSE) © 2026 Wualabs LTD
