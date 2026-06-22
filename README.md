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
- **Web monitor** — live status page over WiFi, backed by a `/data.json` endpoint.
- Hysteresis and boil threshold persist in EEPROM.

## Hardware

- WeMos D1 Mini Pro (ESP8266)
- DS18B20 temperature sensor (OneWire)
- 20x4 character LCD over I2C (address `0x3f`)
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
pio run            # build
pio run -t upload  # build and flash
```

## WiFi & web access

The device runs as a WiFi access point:

- SSID: `BrauKnecht`
- Password: `brauknecht`

Connect, then open **http://bk.local/** for the live status page.
