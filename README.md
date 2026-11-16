# WuaStation-mini firmware

PlatformIO firmware for the **WuaStation-mini** board (ESP32-C3-WROOM-02-N4)
with the **LMX1p** panoramic LED matrix (18 x 7 SK6805). It shows internet data
(time, weather, crypto tickers) and readings from an I2C sensor, all driven
through the [WuaDisplay-LMX](https://github.com/WuaLeds/WuaDisplay-LMX) library
(`WUA_BOARD_LMX1P` backend).

It carries over every feature of `WuaDisplay-r1-firmware` (time, weather,
ticker, alerts, calendar / monthly celebration, alarm, FPS benchmark) and
adapts them to the 18 x 7 canvas.

## Hardware

Pinout taken from `PCB/WuaStation-Mini/WuaStation-Mini.kicad_sch`
(see [`src/board/Pins.h`](src/board/Pins.h)):

| Net | GPIO | Use |
| --- | --- | --- |
| DIN | IO5 | LMX1p data (126 x SK6805-EC15) |
| UP | IO9 | Button to GND. **BOOT strap**: held at reset = download mode |
| DOWN | IO4 | Button to GND |
| TOUCH | IO7 | USB-C SBU1/SBU2, 2k pull-down, active HIGH (e.g. TTP223) |
| SDA / SCL | IO3 / IO6 | I2C expansion connector J1 (pull-ups on the breakout) |
| MCC1 | IO0 + IO20 | USB-C CC1 via JP1 (bridged) - kept high-impedance |
| MCC2 | IO1 + IO21 | USB-C CC2 via JP2 (bridged) - kept high-impedance |
| D- / D+ | IO18 / IO19 | Native USB (Serial = USB-CDC) |
| RST | EN | SW1 |

The LMX1p chain order was checked against the LMX1p KiCad files: the board
used in landscape (rotated 90 degrees clockwise) matches the library mapping
1:1 for all 126 LEDs.

## Screens

Which screens rotate, in which order and for how long is set from the
[settings page](#settings-page); screens without data are skipped
automatically. Always present: **Splash**
("WuaStation" once at boot) and **Status** (WiFi icon + progress while
connecting / syncing, red cross when offline).

| Id | Content | Default time |
| --- | --- | --- |
| `time` | `HH:MM` with blinking colon (SNTP, timezone + DST) | 10 s |
| `date` | `DD.MM` | 4 s |
| `weather` | Open-Meteo: condition icon (day/night) + temperature, then humidity | 8 s |
| `forecast` | Today: ↑ max, ↓ min, ☂ chance of rain, `UV` index, `AQ` air quality (colored by band) | 10 s, shared |
| `indoor` | I2C sensor: temperature, humidity, pressure (BME280) | 9 s |
| `ticker` | Binance, fixed pages per symbol: name `BTC` → price `97.2K` (green/red by trend) → 24 h change `+2.3%` | 6.5 s per symbol |
| `message` | Custom text from the settings page | one pass |
| `celebration` | Monthly anniversary message (ported from WuaDisplay-r1) | one pass |
| `timer` | Countdown / pomodoro (see [Timer](#timer)) | 4 s while idle |

Air quality uses the US AQI or the European index, as chosen on the settings
page.

Interrupt screens (preempt the rotation, highest first): **Alarm** >
**Alert** (queued notifications) > **WiFi portal** instructions > a running
**Timer**.

Static screens use a compact 3x7 font (`display/TinyFont`) so `12:34` fits the
18 px width, and crossfade between each other with the library effects engine.
Ticker prices get as many decimals as fit (`3457`, `145.7`, `0.457`) and are
abbreviated when needed (`97.2K`, at most 1 % rounding). The rare page that
still does not fit (e.g. `MATIC`, `0.00001234`) pans slowly instead of
scrolling away. Page timings are in `Config::kTicker*`.
Scrolling text uses the classic 5x7 GFX font with UTF-8 to CP437 conversion
(accents, n-tilde, inverted marks, degree sign).

## Buttons

| Input | Click | Long press |
| --- | --- | --- |
| UP | Next screen | Open / close the WiFi setup portal |
| DOWN | Brightness step (saved) | Device info (IP, hostname, version) |
| TOUCH | Next screen | Device info |

- Any click dismisses an alert; any button stops the alarm.
- On the timer screen DOWN drives the timer instead (below).
- **Factory reset**: hold DOWN while powering on (3 s, shows `RST` then `OK`).

### Timer

Go to the timer screen with UP, then:

| State | DOWN click | DOWN long | UP / TOUCH click |
| --- | --- | --- | --- |
| Idle (hourglass or tomato + minutes) | Next preset: 1, 3, 5, 10, 15, 20, 30, 45, 60, 90 min, pomodoro | Start | Next screen |
| Running (`MM:SS`, stays on screen) | Pause / resume (blinks while paused) | Stop | Hide it (keeps running in the rotation) |
| Finished (flashing `00:00`) | Any button acknowledges | | |

The pomodoro runs 25 min work (red) and 5 min breaks (green), with a 15 min
break after every 4th block, and goes on by itself until stopped. Times are in
`Config::kPomodoro*` / `Config::kTimerPresetsMin`. Visual only: there is no
buzzer.

## Setup

1. On first boot (no WiFi saved) the board opens the access point
   **`WuaStation-XXXX`** (password `WuaLeds2025`, see `Config::kPortalPassword`)
   and scrolls the instructions.
2. Join it and open `http://192.168.4.1`: the [settings page](#settings-page)
   opens; **WiFi setup** picks the network.
3. Once connected, the same page is served on the LAN at
   `http://wuastation-xxxx.local` (or the IP shown with a DOWN long press), so
   the settings can be changed any time without the access point.

### Settings page

A Wualeds-branded page (logo, colors of wualeds.com, light/dark, Spanish or
English) served by the board itself, with no internet needed. Saving stores
everything in NVS and restarts the board.

| Section | Settings |
| --- | --- |
| Screens | Every screen with an on/off switch, up/down arrows for the order and its time in seconds (empty = default; for `ticker`, per symbol) |
| Location and time | City and timezone picked from 66 well-known cities grouped by continent (zones listed west to east with their current UTC offset), or *Other…* to type any city (`Rosario,AR`) or zone; under *Advanced*, latitude/longitude and the air-quality index (US / European) |
| Ticker | Up to 4 Binance pairs (`BTCUSDT, ETHUSDT`); empty hides the screen |
| Alarm and message | Daily alarm (switch + time) and the custom scrolling text |
| Display and sensor | Brightness (applied live while dragging) and a temperature offset for the sensor (±10 °C, enclosure heat) |
| Device | Firmware version, host name, uptime, WiFi setup, restart |

The header also shows the device clock, WiFi network, IP and the sensor
reading, refreshed every 10 s.

**Timezone**: *Automatic* (`auto`, the default, also when empty) follows the
weather city, so choosing the city is enough. Otherwise an Olson name, city or
country from the built-in table in [`src/net/Timezones.cpp`](src/net/Timezones.cpp)
(`Europe/Madrid`, `Buenos Aires`, `Tokio`, `Argentina`; case, spaces and
accents do not matter) or any POSIX TZ string (`<-03>3`). The preset cities
live in the `CITIES` table of `web/settings.html`; each one's geocoding query
(`Name,CC`) and timezone were checked against the Open-Meteo geocoding API.

The page source is [`web/settings.html`](web/settings.html) (plain HTML, CSS
and JS, no build step). [`tools/embed_web.py`](tools/embed_web.py) runs before
every build and embeds it gzipped (~12 KB) as `SettingsPageData.h`, generated
inside the build directory (`.pio/build/<env>/generated/`, never in `src/`);
[`src/web/SettingsServer`](src/web/SettingsServer.h) serves it plus a small
JSON API (`/api/settings`, `/api/status`, `/api/brightness`, `/api/restart`).
The rotation is stored as `time:10,date:4,-weather,ticker:9,...` (display
order, optional seconds, `-` = hidden).

## Build & flash

```sh
pio run -e c3_release                 # build
pio run -e c3_release -t upload       # flash over USB
pio device monitor                    # serial monitor (c3_debug for logs)
```

| Environment | Purpose |
| --- | --- |
| `c3_release` | Production build (default) |
| `c3_debug` | Logs over USB-CDC (`WUA_ENABLE_DEBUG`) + built-in USB-JTAG debugging |
| `c3_bench` | FPS benchmark of the LED pipeline, WiFi off |

> If `pio` from the system PATH fails with a `littlefs` import error, use the
> PlatformIO Core bundled with the VS Code extension
> (`~/.platformio/penv/Scripts/pio`), or build from the VS Code PlatformIO UI.

Toolchain: pioarduino `55.03.37` (Arduino-ESP32 core 3.3.7). The
WuaDisplay-LMX library is pinned to a commit in `platformio.ini`; to develop
the library side by side, point `lib_deps` to a local `symlink://` path.

## Releases

Each GitHub release is also published on
[wualeds.com/firmware](https://wualeds.com/firmware), the page that installs the
firmware straight from the browser. To publish a version:

1. Set `WUA_FIRMWARE_VERSION` in `src/config/Config.h` (e.g. `"0.3"`) and push.
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

## Project structure

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
```

### How it fits together

- **Main loop** (never blocks): WiFi state machine, buttons, sensor sampling,
  `DisplayController` (screen rotation + library scroll/crossfade).
- **Network task** (`NetWorker`): runs every internet job one after another
  while WiFi is up and publishes results through `SharedState<T>`, so HTTPS
  requests never freeze the display.

### Extending

- **New screen**: derive from `StaticStateBase` (implement `render()`),
  `PagedStateBase` (fixed pages: `beginPages()` / `renderPage()`) or
  `ScrollTextStateBase` (implement `nextPass()`). Give it an id in
  `config/ScreenConfig` (name and default time), map it in `App::screen()` and
  add its name and description to the `I18N` tables of `web/settings.html`, so
  it can be placed from the settings page. Return `priority() > 0` for an
  interrupt screen.
- **New internet source**: implement `INetJob` (blocking `run()` returning the
  next delay), publish a plain struct with `SharedState<T>`, add it to the
  `NetWorker` in `App::setup()`.
- **New I2C sensor**: implement `ISensor` (probe / start / read) and add it to
  `SensorManager::_candidates`.

## Notes

- **Alarm is visual only**: the WuaStation-mini has no buzzer or I2S amplifier,
  so the MP3 siren of WuaDisplay-r1 is replaced by a flashing alarm screen
  (`Config::kAlarmRingMs`).
- **USB-C CC jumpers**: with JP1/JP2 bridged, IO0/IO20 and IO1/IO21 sit on the
  CC lines. The firmware parks them as inputs at boot, but the ROM bootloader
  drives IO21 (UART0 TX) high for a moment on every reset, which may confuse
  some USB-C chargers. Cut the jumpers if those pins are not needed.
- **I2C pull-ups**: the board has none; use a breakout that includes them.
- **TLS**: HTTPS is used without certificate validation (public, read-only
  APIs). Do not reuse `HttpJson` for anything that sends secrets.
- Binance falls back to `data-api.binance.vision` if `api.binance.com` is
  blocked in your region.
# WuaStation-Mini-firmware
