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

Rotation (screens without data are skipped automatically):

| Screen | Content |
| --- | --- |
| Splash | "WuaStation" once at boot |
| Status | WiFi icon + progress while connecting / syncing, red cross when offline |
| Time | `HH:MM` with blinking colon (SNTP, timezone + DST) |
| Date | `DD.MM` |
| Weather | Open-Meteo: condition icon (day/night) + temperature, then humidity |
| Indoor | I2C sensor: temperature, humidity, pressure (BME280) |
| Ticker | Binance, fixed pages per symbol: name `BTC` → price `97.2K` (green/red by trend) → 24 h change `+2.3%` |
| Message | Custom text from the settings page |
| Celebration | Monthly anniversary message (ported from WuaDisplay-r1) |

Interrupt screens (preempt the rotation, highest first): **Alarm** >
**Alert** (queued notifications) > **WiFi portal** instructions.

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
- **Factory reset**: hold DOWN while powering on (3 s, shows `RST` then `OK`).

## Setup

1. On first boot (no WiFi saved) the board opens the access point
   **`WuaStation-XXXX`** (password `WuaLeds2025`, see `Config::kPortalPassword`)
   and scrolls the instructions.
2. Join it and open `http://192.168.4.1`: **Configure WiFi** for the network,
   **Setup** for the station settings.
3. Once connected, the same pages are served on the LAN at
   `http://wuastation-xxxx.local` (or the IP shown with a DOWN long press), so
   the settings can be changed any time without the access point.

Settings (stored in NVS; saving restarts the board):

| Field | Example | Notes |
| --- | --- | --- |
| Timezone | `Europe/Madrid` | Olson name from the built-in table or any POSIX TZ string (`<-03>3`) |
| Weather city | `Madrid` / `Madrid,ES` | Geocoded once with Open-Meteo |
| Latitude / Longitude | `40.4168` / `-3.7038` | Optional, override the city |
| Tickers | `BTCUSDT,ETHUSDT` | Up to 4 Binance symbols, empty hides the screen |
| Alarm | `07:30` | Empty disables it |
| Message | `Hola!` | Empty hides the screen |
| Brightness | `32` | 1-255 |

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
├── config/                  Config.h (defaults/tuning), Theme.h (colors), Strings.h (UI texts)
├── core/                    Logger, Settings (NVS), SharedState (task-safe snapshots)
├── display/                 Display.h (WuaDisplay-LMX), Compositor (frames + crossfade),
│                            TinyFont, Icons, Widgets, TextUtil (UTF-8 -> CP437)
├── input/                   Button (debounce, click / long press)
├── net/                     WifiService (WiFiManager portal), TimeService (SNTP),
│                            Timezones, NetWorker (background HTTP task), HttpJson
├── providers/
│   ├── ticker/              BinanceTickerProvider
│   ├── weather/             OpenMeteoWeatherProvider
│   └── sensors/             SensorManager + SHT4x, SHT3x, AHT2x, BME280 drivers
├── states/                  DisplayController + one file per screen
└── bench/                   FPS benchmark
```

### How it fits together

- **Main loop** (never blocks): WiFi state machine, buttons, sensor sampling,
  `DisplayController` (screen rotation + library scroll/crossfade).
- **Network task** (`NetWorker`): runs every internet job one after another
  while WiFi is up and publishes results through `SharedState<T>`, so HTTPS
  requests never freeze the display.

### Extending

- **New screen**: derive from `StaticStateBase` (implement `render()`) or
  `ScrollTextStateBase` (implement `nextPass()`), then register it in
  `App::registerStates()`. Return `priority() > 0` for an interrupt screen.
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
