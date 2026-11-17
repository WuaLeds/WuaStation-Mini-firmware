# Changelog

All notable changes to the WuaStation-mini firmware are listed here.
The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and
versions match the GitHub release tags (`WUA_FIRMWARE_VERSION` in
`src/config/Config.h`).

## [Unreleased]

### Added
- `.clang-format` (clang-format 23.1.5) and a formatting check in the CI workflow.
- MIT license (Wualabs LTD) and this changelog.

### Changed
- Sources formatted with clang-format (whitespace only, no behavior change).
- README redesigned with badges, icons and quick links.

## [0.3] - 2026-11-16

### Added
- Settings page with the Wualeds look (logo, colors, light and dark mode,
  Spanish and English), served by the board in the setup portal and on the LAN.
  Its source is `web/settings.html`, embedded gzipped at build time.
- Screen rotation set from the settings page: show or hide each screen, change
  its order and its time.
- City and timezone picked from 66 well-known cities around the world, or typed
  by hand.
- Automatic timezone (new default): the clock follows the weather city.
- Forecast screen: today's high and low, chance of rain, UV index and air
  quality (US or European index).
- Timer screen: countdowns from 1 to 90 min and pomodoro (25/5/15 min), driven
  with the DOWN button.
- Brightness slider applied live, and a temperature offset for the sensor.

### Changed
- Timezone names are matched loosely (case, spaces and accents do not matter):
  `Buenos Aires`, `Tokio`, `Europe/Madrid`.
- Interrupt priorities: alarm > alert > WiFi portal > running timer.
- The WiFiManager parameter form is replaced by the new settings page; WiFi is
  still chosen on its `/wifi` page.

### Fixed
- Release packaging: the embedded page header is generated inside the build
  directory, so a fresh checkout no longer loses `partitions.bin` between
  `pio run` and `pio project metadata`.

## [0.2] - 2026-11-13

### Changed
- The crypto ticker no longer scrolls: each symbol is shown as fixed pages,
  name, then price (green or red by trend), then 24 h change.
- Prices use as many decimals as fit the 18 px screen and are abbreviated with
  K and M when needed (at most 1 % rounding); pages that still do not fit pan
  slowly.
- Coin names come from Binance (`BTCUSDT` shows as `BTC`), and the compact font
  gained the uppercase letters A to Z.

### Added
- Release workflow: builds each GitHub release, attaches the web flasher files
  and rebuilds wualeds.com.

## [0.1] - 2026-11-13

### Added
- First release for the WuaStation-mini (ESP32-C3 + LMX1p 18x7 LED matrix).
- Clock and date over SNTP with timezone and daylight saving time.
- Weather from Open-Meteo, indoor readings from an I2C sensor (BME280, SHT4x,
  SHT3x or AHT2x, auto-detected), crypto ticker for up to 4 Binance symbols.
- Custom message, monthly anniversary message, visual alarm and on-screen
  alerts.
- WiFi setup portal and settings page, button controls and factory reset.

[Unreleased]: https://github.com/WuaLeds/WuaStation-Mini-firmware/compare/v0.3...HEAD
[0.3]: https://github.com/WuaLeds/WuaStation-Mini-firmware/compare/v0.2...v0.3
[0.2]: https://github.com/WuaLeds/WuaStation-Mini-firmware/compare/v0.1...v0.2
[0.1]: https://github.com/WuaLeds/WuaStation-Mini-firmware/releases/tag/v0.1
