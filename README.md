# ESP_UniClock v3.1

**A universal Wi-Fi clock firmware for ESP8266 / ESP32 — one codebase, 60+ different physical builds.**

Drive Nixie, VFD (vacuum fluorescent), Numitron, LED-segment, LED-matrix or NeoPixel "word clock"
displays from a single Arduino sketch. Which physical hardware you're building is selected entirely
in one file (`clocks.h`) — no code changes needed for a new build, just a new configuration block.

License: GPL-3.0-or-later (see the header of `ESP32_UniClock2.ino`) · Platforms: ESP8266, ESP32 ·
Language: C++ (Arduino)

---

## Table of contents

- [Features](#features)
- [Supported display drivers](#supported-display-drivers)
- [Optional components](#optional-components)
- [Screenshots](#screenshots)
- [Requirements](#requirements)
- [Getting started](#getting-started)
- [Configuration](#configuration-clocksh)
- [Developer documentation](#developer-documentation)
- [Schematics & PCB designs](#schematics--pcb-designs)
- [Manuals](#manuals)
- [External libraries](#external-libraries-bundled-in-libraries)
- [Known issues](#known-issues)
- [License](#license)
- [Author & support](#author--support)

---

## Features

- Works with any ESP8266 or ESP32 based module, via the Arduino IDE
- 1, 4, 6, 8 or 12 clock digits (tubes), depending on the build
- Time sync via Wi-Fi (NTP), GPS, or a DS3231 RTC chip — with automatic fallback between sources
- Wi-Fi in both infrastructure (STA) and standalone access-point (AP) modes, configured through a
  built-in responsive web UI (see [Screenshots](#screenshots))
- Temperature / humidity / barometric-pressure / ambient-light sensor support (see
  [Optional components](#optional-components))
- Adjustable day/night brightness, with optional automatic dimming from an ambient light sensor
- Radar or PIR motion sensor support to power tubes off when no one's around
- 8 tube-transition animation modes (including a random-pick mode)
- Full-color WS2812 (NeoPixel) tube backlighting with multiple animation effects
- Alarm function with buzzer and light
- Cathode/tube-life protection ("anti-poisoning") cycling
- Home Assistant integration via MQTT, including a master/slave mode so several clocks can share one
  physical sensor
- OTA firmware updates from the web UI
- Single-file hardware setup via `clocks.h` — no other source changes needed for a new physical build

## Supported display drivers

| Driver chip / scheme | Display type | Driver file(s) |
|---|---|---|
| SN74141 / 7441 / K155ID1 (BCD decoder), multiplexed | Nixie | `multiplex74141.ino` (8266), `multiplex74141_ESP32.ino` |
| SN74141, non-multiplexed (one decoder per tube) | Nixie | `no_multiplex74141.ino` (8266), `no_multiplex_ESP32.ino` |
| SN74141 + PCF8574 I²C port expander | Nixie | `pcf_multiplex74141.ino` |
| MAX6921 (20-ch. HV shift register) | VFD | `MAX6921.ino` (8266), `MAX6921_ESP32.ino` |
| HV5122 (32-ch. HV shift register, cascaded ×2) | Nixie | `HV5122new.ino` |
| PT6311 / PT6355 (smart VFD controller, on-chip scan) | VFD | `PT6311.ino`, `PT6355.ino` |
| SN75512 / SN75518 (HV shift register) | VFD | `SN75512.ino`, `SN75518.ino` |
| MM5450 (34-ch. LED driver) | LED 7-segment | `MM5450.ino` |
| MAX7219 / compatible | LED matrix / 7-segment | `MAX7219CNG.ino` |
| CD4511 / 74HC4511 (BCD→7-segment) | Numitron | `Numitron_4511N.ino` |
| 4× 74HC595 + optocouplers | Custom segment displays | `ZM1500.ino` |
| CD4543 + CD4028 (BCD→7-segment / BCD→decimal) | LED 7-segment | `mulev2.ino` ("Mule V2" board) |
| VQC10 (5×7 dot-matrix element) | Dot-matrix, full ASCII | `VQC10.ino` |
| WS2812 LED matrix behind an acrylic panel | "Word clock" (spells the time; EN/DE/HU) | `wordclock.ino` |
| — | Single-tube variant | `1tube.ino` |
| — | Sensor-only "clock" (no display) | `dummy.ino` |

Only one driver is compiled into any given firmware image — see [Configuration](#configuration-clocksh).

## Optional components

- DS18B20 temperature sensor (1 or 2 sensors on one bus)
- DHT11 / DHT22 temperature+humidity sensor
- BME280 / BMP280 / AHT10-AHT20 / SHT21 temperature/humidity/pressure sensor on I²C
- BH1750 ambient-light sensor on I²C
- Home Assistant integration via an MQTT client (with master/slave sensor sharing across clocks)
- DS3231 RTC module, settable via Wi-Fi or manually with pushbuttons
- Radar or PIR motion sensor
- GPS module for time sync in standalone (no-internet) setups
- WS2812 addressable LEDs for tube backlighting — fixed color or multiple animated effects
- Plain 3-pin PWM RGB backlighting (alternative to addressable LEDs, on boards with limited GPIO)
- User-defined GPIO pin numbers for every digit/segment/sensor/button

## Screenshots

- Web configuration UI: [`Webpage.jpg`](Webpage.jpg), [`Uniclock_v2 .png`](<Uniclock_v2 .png>)
- Photos of physical builds: [`photos/`](photos)
- PCB renders: [`PCB_available/`](PCB_available)

## Requirements

| Tool | Tested version |
|---|---|
| [Arduino IDE](https://www.arduino.cc/en/software) | **1.8.19** |
| ESP32 board package (`esp32` by Espressif Systems, via Boards Manager) | **2.0.17** |
| ESP8266 board package (`esp8266` by ESP8266 Community, via Boards Manager) | any recent 2.6.x–3.x (both are detected and handled at compile time, see `ESP32_UniClock2.ino`) |

> ⚠️ **Board package version matters.** This firmware has only been verified against **Arduino IDE
> 1.8.19** with the **ESP32 board package v2.0.17**. Newer Arduino IDE releases (2.x) or newer/older
> ESP32 core versions may require code changes (the ESP32 Arduino core's API — WiFi, LEDC/PWM, timer,
> and SPIFFS/LittleFS calls in particular — has changed across major versions). If you hit compile
> errors on a different toolchain version, try matching the tested versions above first before filing
> an issue.

## Getting started

1. **Download/clone this repository.**
2. **Install the libraries.** Unzip every archive in [`libraries/`](libraries) into your Arduino
   `libraries` folder (usually `Documents\Arduino\libraries` on Windows), **and rename this project's
   own folder to `ESP32_UniClock2`** — the sketch expects that exact folder name.
3. **Install the Boards Manager packages** for ESP8266 and/or ESP32 (see
   [Requirements](#requirements) for the tested versions).
4. **Pick (or write) your clock configuration** in [`clocks.h`](clocks.h) — see
   [Configuration](#configuration-clocksh) below.
5. **Select your board** in the Arduino IDE (`Tools → Board`) and the matching serial port.
6. **Upload the sketch** (`ESP32_UniClock2.ino`) as usual.
7. **Upload the web UI files to flash storage.** The files in [`data/`](data) (the config web page's
   HTML/CSS/JS) must be uploaded separately to SPIFFS/LittleFS using the filesystem-upload plugin —
   installers are bundled in [`SPIFFS uploaders/`](<SPIFFS uploaders>), or see:
   - [ESP8266 filesystem uploader tutorial](https://randomnerdtutorials.com/install-esp8266-filesystem-uploader-arduino-ide)
   - [ESP32 filesystem uploader tutorial](https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide)
8. **First boot**: if no Wi-Fi is configured, the clock starts its own access point (`192.168.4.1`) —
   connect to it and set your Wi-Fi/clock options from the web UI.

## Configuration (`clocks.h`)

All hardware setup lives in one file. Near the top, uncomment exactly **one** `#define CLOCK_NN` line
to select your build; every other line in that list must stay commented out. Each `CLOCK_NN` block then
defines the driver chip, GPIO pin assignments, and which optional features (sensors, RTC, GPS,
backlight, alarm, etc.) that specific build uses. To add a new physical build, copy the block closest
to your hardware, give it an unused number, and adjust the pins/options.

See [`docs/DEVELOPER_GUIDE.md`](docs/DEVELOPER_GUIDE.md) for the full breakdown of how `clocks.h`
drives the rest of the firmware.

## Developer documentation

This repository includes a full developer-documentation set (added during a 2026 code review pass):

- [`docs/DEVELOPER_GUIDE.md`](docs/DEVELOPER_GUIDE.md) — full architecture write-up: build model, the
  common driver interface, the core program's structure, the web admin API, and a complete driver catalog.
- [`docs/CODE_REVIEW.md`](docs/CODE_REVIEW.md) — file-by-file findings: bugs (fixed and open),
  optimization suggestions, and known limitations.
- [`docs/PROGRESS.md`](docs/PROGRESS.md) — status log of the review/documentation effort itself.

Every `.ino`/`.h` file in the project also has inline comments explaining the non-obvious hardware
protocol details, timing behavior, and known caveats of that specific driver.

## Schematics & PCB designs

See [`schematics/`](schematics) for KiCad-exported PCB/schematic PDFs, including:
- ESP32 clock, 6× IV-11 VFD tubes
- ESP32 clock, 6× Z573M/Z574M Nixie tubes
- ESP8266 Numitron clock, 4× IV-16 tubes
- ...and many more (ESP32 mainboard + interchangeable tube-driver daughterboards, HV5122-based flat
  panel builds, ESP8266 variants, etc.)

## Manuals

- UniClock Manual — English: [DOCX](<UniClock Manual ENG v3.1.docx>) · [PDF](<UniClock Manual ENG v3.1.pdf>)
- UniClock kézikönyv — Magyar: [DOCX](<UniClock Manual HUN v3.1.docx>) · [PDF](<UniClock Manual HUN v3.1.pdf>)

## External libraries (bundled in `libraries/`)

Core: `Time`, `Timezone`, `NTPClient`, `ESPAsyncWebServer`, `ESPAsyncTCP` (8266) / `AsyncTCP-ESP32`
(ESP32), `ESPAsyncWiFiManager`, `ArduinoJson`, `ESP32httpUpdate`, `EspSoftwareSerial`.

Optional (only needed for the sensors/features you enable): `Rtc_by_Makuna` (DS3231), `TinyGPSPlus`,
`NeoPixelBus_by_Makuna` (WS2812), `OneWire` + `DallasTemperature` (DS18B20), `DHT_sensor_library`,
`Adafruit_BME280_Library`, `Adafruit_BMP280_Library`, `Adafruit_AHTX0`, `Adafruit_BusIO`,
`Adafruit_Unified_Sensor`, `SHT21`, `BH1750`, `PubSubClient` + `arduino-home-assistant` (MQTT/Home
Assistant), `Samsung_16LF01_VFD`.

## Known issues

See [`docs/CODE_REVIEW.md`](docs/CODE_REVIEW.md) for the complete, current list. Most notably:
`mqtt.ino` ships a hardcoded default MQTT broker address/username/password (set in `factoryReset()`)
and had a buffer-handling issue in incoming-message parsing — **change the MQTT credentials from the
web UI before exposing an MQTT-enabled build to an untrusted network**, and see the code review for
details before relying on it in a security-sensitive setup.

## License

Released under the **GNU General Public License v3 (or later)** — see the license header in
`ESP32_UniClock2.ino` for the full notice.

## Author & support

**Peter Gautier** — [gautier.p62@gmail.com](mailto:gautier.p62@gmail.com)

Before uploading to the ESP board, don't forget to unzip the library files into the Arduino IDE's
`libraries` folder and **rename this project's own folder to `ESP32_UniClock2`**.
