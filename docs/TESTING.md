# Release test checklist

What to check on real hardware before tagging a release. Items marked 🤖 are
covered by [`tools/api_smoke_test.py`](../tools/api_smoke_test.py), which
drives the station through the ESPHome API the way Home Assistant does.

## 1. Update from the previous version

- [ ] Flash with the web flasher (parts at their offsets, not the factory
      image): the station joins the same WiFi network by itself.
- [ ] The settings survive: city, timezone, tickers, screens, message,
      brightness.
- [ ] `http://wuastation-mini-xxxxxx.local` opens the settings page.

## 2. On its own (no Home Assistant)

- [ ] Boot: splash, WiFi status icon, then the rotation.
- [ ] Every screen shows real data: time, date, weather, forecast, indoor
      sensor, ticker, message, timer.
- [ ] Buttons: UP click (next), UP long (WiFi info), DOWN click (brightness),
      DOWN long (device info), TOUCH click / long, timer controls.
- [ ] The station keeps working for hours with Home Assistant off (no reboot).
- [ ] Router off for a few minutes, then on: the station reconnects by itself.

## 3. Settings page

- [ ] Brightness slider applies at once.
- [ ] Screens: switch off, reorder and change a time; *Save* applies without a
      restart and the rotation follows.
- [ ] Message, alarm, timezone, sensor offset, air-quality index: applied
      without a restart.
- [ ] City or ticker change: saved, the station restarts and uses them.
- [ ] *Change WiFi network*: the station opens `WuaStation-XXXX`; joining it
      from a phone shows the WiFi setup page; after picking a network the
      settings page is back on the LAN.
- [ ] Spanish / English and light / dark mode.

## 4. First setup (factory reset)

- [ ] Hold DOWN while powering on: `RST`, then `OK`, reboot.
- [ ] The matrix scrolls the setup instructions; join `WuaStation-XXXX`
      (password `WuaLeds2025`): the captive portal opens and lists networks.
- [ ] After choosing the network the station joins it and the instructions
      stop.

## 5. Home Assistant

- [ ] The station appears in *Settings › Devices & services* as discovered
      (ESPHome); *Configure* adds it.
- [ ] Device page: model, firmware version (project 0.4), every entity.
- [ ] 🤖 Display light: on/off and brightness, mirrored on the settings page.
- [ ] 🤖 Message text: changes the scrolling message.
- [ ] 🤖 Screen select: shows the current screen, jumps to the chosen one.
- [ ] 🤖 Screen switches: hide / show screens; the settings page follows.
- [ ] 🤖 Next screen button; 🤖 `show_message` action.
- [ ] `show_screen` action with `screen: weather`.
- [ ] Button events: press UP / DOWN / TOUCH, the event entities update
      (`click`, `long_press`); an automation triggers on them.
- [ ] Sensors: indoor (with a sensor connected) and outdoor values, WiFi
      signal, uptime.
- [ ] Change brightness with the DOWN button and the message on the settings
      page: Home Assistant shows the new values.
- [ ] Restart Home Assistant: the station reconnects and keeps working while
      Home Assistant is down.
- [ ] ESPHome dashboard (add-on): the station is offered for adoption.
- [ ] Logs from Home Assistant / ESPHome dashboard show the station's logs.
