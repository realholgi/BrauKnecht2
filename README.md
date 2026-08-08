# BrauKnecht2

ESP8266 homebrew automation — temperature control for mashing and boiling, with a
local LCD/encoder UI and a WiFi web monitor.

Pictures and a full write-up (German) at
https://holgi.beer/tags/brau-ger%C3%A4tschaft/

## Project context

### Domain vocabulary

- **BrauKnecht2** — ESP8266-based homebrew controller for mash and boil automation.
- **Mash rest** — one recipe step with a target temperature, hold time, and optional alarm.
- **Mash-in** — initial temperature target before the mash rests start.
- **Mash-out** — final mash temperature target before lautering or boiling.
- **Boil mode** — timer-driven boil phase with hop-addition reminders.
- **Manual mashing** — operator-controlled target temperature mode, set from the encoder or web dashboard.
- **Recipe import** — conversion of Kleiner-Brauhelfer JSON or BeerXML into mash rests, boil duration, and hop additions.
- **Controller status** — live temperature, setpoint, heater state, mode, firmware metadata, and optional MQTT availability.

### Architecture notes

- Hardware-coupled behavior belongs behind small seams so host-side tests can exercise pure logic with `pio test -e native`.
- Mash heating uses a hardware-free adaptive controller so its relay decisions
  can be exercised with deterministic temperature and time inputs.
- Web routes must keep state-changing actions explicit and same-origin guarded.
- Build/version metadata flows from `VERSION` through `scripts/generate_build_info.py` into `src/build_info.h` fallbacks and generated build files.

## Features

- **Automatic mashing** — up to 8 configurable rests (temperature + hold time),
  mash-in and mash-out temperatures, optional alarm per rest.
- **Manual mashing** — set the target temperature live with the rotary encoder
  or from the web dashboard (`10-100 °C`).
- **Boil mode** — boil timer with multiple hop-addition reminders.
- **Adaptive mash temperature control** — learns the positive heating rate and
  residual thermal lag to predict the cutoff point for different batch thermal
  masses. It keeps a five-second minimum predictive on-time, a 60-second normal
  relay-off dwell, immediate hard-overtemperature shutdown, and sensor-fault,
  abort, and inactive-regulation shutdown.
- **Local UI** — 20x4 LCD and rotary encoder (click to select, hold to abort);
  Setup contains only **Kochschwelle** and **AP ein/aus**.
- **Web app** — dashboard, history, recipe and settings pages over WiFi,
  backed by a `/data.json` endpoint. No external dependencies.
- **Browser-local history** (`/history`) — plots the active recipe setpoint
  curve and live temperature samples in RAM while the page is open.
- **Web config** (`/config`) — pick your home WiFi from a scanned list, set
  MQTT credentials and inspect firmware/build metadata; settings persist in flash.
- **Recipe import** (`/recipe`) — upload a Kleiner-Brauhelfer JSON or BeerXML
  file; the mash steps and hop additions are applied and stored in flash.
- **MQTT / Home Assistant** — publishes temperature, setpoint, heater and mode
  with Home Assistant auto-discovery (read-only), including an availability
  topic so HA shows the controller offline when it's powered down.
- **Versioned OTA updates** — firmware version comes from root `VERSION`;
  each PlatformIO build embeds Git hash, build time and environment.
- The boil threshold persists in EEPROM; adaptive mash-controller learning is
  intentionally reset for each heating session.

## Hardware

- WeMos D1 Mini Pro (ESP8266)
- DS18B20 temperature sensor (OneWire)
- 20x4 character LCD over I2C (address auto-detected, `0x27` or `0x3f`)
- Rotary encoder with push button
- Heating relay / SSR
- Buzzer

Pin assignments (`src/config.h`):

| Function           | Pin |
|--------------------|-----|
| Temp sensor (1-Wire) | D3  |
| Heater relay/SSR   | D4  |
| Encoder A          | D5  |
| Encoder B          | D6  |
| Encoder button     | D7  |
| Buzzer             | D8  |

## Build & flash

Built with [PlatformIO](https://platformio.org/) (board `d1_mini_pro`).
Libraries are pulled automatically from `platformio.ini`.

```sh
pio run                       # build (release: no serial logging)
pio run -t upload             # build and flash over USB
pio run -e d1_mini_debug -t upload   # flash with serial debug on 115200
pio run -e d1_mini_ota -t upload     # flash over WiFi (board reachable at bk.local)
pio test -e native            # run host-side unit tests
pio test -e native -f test_temperature_control  # run adaptive-controller tests
pio check                     # static analysis (cppcheck)
```

First flash must be over USB; once it's running, `d1_mini_ota` updates over WiFi.

Build metadata is generated at build time under `.pio/build/<env>/generated/`
and is not committed. `VERSION` in the repository root is the semantic version
source; `src/build_info.h` provides compile-time access and fallbacks.
Generated firmware shows `X.Y.Z` only when `HEAD` is exactly tagged `vX.Y.Z`;
otherwise it shows `X.Y.Z-dev`.

Use the Makefile helpers for release versioning:

```sh
make version          # prints vX.Y.Z
make bump-patch       # increments VERSION
make tag-version      # creates annotated git tag vX.Y.Z
make release-patch    # bump patch, commit VERSION if needed, tag
make binary           # writes dist/brauknecht-vX.Y.Z-d1_mini.bin
```

After an OTA upload, open **http://bk.local/config** and check **Geräteinfo**:
`Firmware`, `Build`, `Build-Zeit`, `IP`, `AP-IP` and optional `MQTT`. The same
fields are also exposed as `firmware_version`, `build_hash`, `build_time` and
`build_env` in `/data.json`.

## WiFi & web access

The device starts its own open access point for initial and recovery access, and
also joins your configured home network (`/config`):

- AP SSID: `BrauKnecht`

After the configured WLAN connection succeeds, the device AP closes and returns
when that connection is lost. LCD **Setup → AP ein/aus** temporarily changes the
current AP state until the next station connection transition or reboot. Use the
AP only while it is available, or use the configured LAN address / **http://bk.local/**
after the device joins your network. Navigation order is the same on desktop
and mobile:

- **Dashboard** (`/`) — active recipe timeline, live status and manual setpoint
  form in the app layout. `POST /manual` clamps the submitted setpoint to
  `10-100 °C`, switches to manual mode and keeps the heater as read-only status;
  the dashboard sends a same-origin action header for this state-changing call.
- **Verlauf** (`/history`) — recipe setpoint curve, live temperature samples,
  current step and recipe overview with hop additions and boil end. Samples
  stay only in browser RAM and reset when the page is closed/reloaded.
- **Rezept** (`/recipe`) — active recipe summary with mash/rest/hop/boil
  counters, full timeline through `Kochende`, and Kleiner-Brauhelfer JSON or
  BeerXML import.
- **Einstellungen** (`/config`) — WLAN, MQTT and Geräteinfo cards with human
  readable build time.
