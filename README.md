# BrauKnecht2

ESP8266 homebrew automation — temperature control for mashing and boiling, with a
local LCD/encoder UI and a WiFi web monitor.

Pictures and a full write-up (German) at
https://holgi.beer/tags/brau-ger%C3%A4tschaft/

## Features

- **Automatic mashing** — up to 8 configurable rests (temperature + hold time),
  mash-in and mash-out temperatures, optional alarm per rest.
- **Manual mashing** — set the target temperature live with the rotary encoder.
- **Boil mode** — boil timer with multiple hop-addition reminders.
- **Hysteresis temperature control** with safety features: sensor-error detection,
  overshoot cutoff, relay anti-chatter lockout, and a watchdog.
- **Local UI** — 20x4 LCD and rotary encoder (click to select, hold to abort).
- **Web monitor** — live, mobile-friendly status page over WiFi, backed by a
  `/data.json` endpoint. No external dependencies, auto light/dark.
- **Web config** (`/config`) — pick your home WiFi from a scanned list and set
  MQTT credentials; settings persist in flash.
- **Recipe import** (`/recipe`) — upload a Kleiner-Brauhelfer JSON or BeerXML
  file; the mash steps and hop additions are applied and stored in flash.
- **MQTT / Home Assistant** — publishes temperature, setpoint, heater and mode
  with Home Assistant auto-discovery (read-only), including an availability
  topic so HA shows the controller offline when it's powered down.
- **OTA updates** — flash new firmware over WiFi, no USB cable needed.
- Hysteresis and boil threshold persist in EEPROM.

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
pio check                     # static analysis (cppcheck)
```

First flash must be over USB; once it's running, `d1_mini_ota` updates over WiFi.

## WiFi & web access

The device always runs its own access point, and additionally joins your home
network once configured (`/config`):

- AP SSID: `BrauKnecht`
- AP password: `brauknecht`

Connect to the AP (or reach it on your LAN once joined) and open
**http://bk.local/** for the live status page. From there:

- **`/config`** — select your WiFi from a scanned list and set MQTT host/port/user.
- **`/recipe`** — upload a Kleiner-Brauhelfer JSON or BeerXML recipe.
