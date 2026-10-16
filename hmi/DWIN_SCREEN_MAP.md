# DWIN screen and variable map

**English** · [Русский](DWIN_SCREEN_MAP.ru.md)

Reference for the `DMG10600C070_03WTC` display (1024×600): page numbers, variable addresses (VP), and common HMI mistakes.

First Player screen: [MINIMAL_PLAYER.md](MINIMAL_PLAYER.md).

## Variable addresses

Application variables start at `0x5000` and above.

Two system registers belong to the panel (do not reuse them for widgets):

| Address | Purpose |
|---:|---|
| `0x0082` | Screen brightness |
| `0x0084` | Current page number |

Use only addresses from this document (and from `dwin_vp.h`) for buttons and fields.

## Pages

| # | Purpose |
|---:|---|
| 0 | Boot |
| 1 | Player |
| 2 | Playlist |
| 3 | Settings hub |
| 4 | Wi‑Fi |
| 5 | Audio / EQ |
| 6 | Time zone |
| 7 | About |
| 8 | Screensaver |
| 9 | Reserved |
| 10 | Alarms (list) |
| 11 | Alarm (editor) |
| 21…27 | Optional English copies of pages 1…7 |
| 30 / 31 | Optional English alarm pages |

On Wi‑Fi loss and while adjusting volume the firmware **stays on Player** (it does not open page 9).

## ESP → display (status)

| VP | DGUS type | Meaning |
|---:|---|---|
| `0x5000` | Scrolling Text, length **56** (data at `0x5003`) | Station name |
| `0x5040` | Scrolling Text, length **56** (data at `0x5043`) | Track title |
| `0x5600` | Text Display, ≤ 64 | Static track title (optional) |
| `0x5080` | Text, length ≥ 8 | Time `HH:MM:SS` |
| `0x5090` | Text, length ≥ 10 | Date `DD.MM.YYYY` |
| `0x50A0` | Text | RSSI (`-67 dBm`) |
| `0x50B0` | Text, length ≥ 16 | IP address |
| `0x50C0` | Text | Bitrate (kbps) |
| `0x50D0` / `0x50D2` | Number | VU left / right |
| `0x50D4` | Number | Volume 0…254 |
| `0x50D6` | Variable Icon | 0 = stopped, 1 = playing |
| `0x50D8` | Number | Station number (1-based) |
| `0x50E0` | Text, length ≥ 8 | Format: `MP3`, `AAC`, `FLC`, `OGG`, `WAV` |
| `0x50F0` | Text, length ≥ 16 | Weekday name |
| `0x5100`…`0x5280` | 7 text fields | Playlist names in the window |
| `0x52D0`…`0x52D6` | Number | Playlist row numbers |
| `0x52C0` / `0x52C1` | Number | Selected station / highlight row |
| `0x5302` / `04` / `06` | Signed number | Bass / middle / treble (−40…+6) |
| `0x5308` | Signed number | Balance (−16…+16) |
| `0x530A` / `0x530C` | Signed number | Time zone hour / minute |
| `0x5320` | 0 / 1 | Telnet CLI (not the web UI) |
| `0x5310` | Number | Weather icon 0…9 |
| `0x5312` / `0x5314` | Signed number | Temp / feels-like °C |
| `0x5316` / `5318` / `531A` | Number | Pressure mmHg / humidity % / wind m/s |
| `0x531C` | Number | Pressure hPa (optional) |
| `0x5620` | Text, ≤ 48 | Weather description |
| `0x5640` | Text | Mode `WEB` or `SD` |
| `0x5660` | Text | Firmware version |
| `0x56A0` | Text, ≤ 32 | Wi‑Fi SSID |
| `0x5700` + `i×0x20` | Numbers | Alarm i: on, start, stop, station, volume, days |
| `0x5800` + `i×0x40` | Text, ≥ 40 | Alarm summary line |
| `0x5760`…`0x576C` | Numbers | Alarm editor fields |
| `0x5770`…`0x5776` | 0 / 1 | Weekday icons Sun…Sat |
| `0x5780` / `0x5782` | Number | Alarm activity / slot number |
| `0x58C0` | Text, ≥ 32 | Error / conflict message |
| `0x58E0` | Text, ≥ 32 | Alarm status on Player |

See also: [PLAYLIST_PAGE.md](PLAYLIST_PAGE.md), [WEATHER.md](WEATHER.md), [SETTINGS_PAGE.md](SETTINGS_PAGE.md), [ALARMS_PAGE.md](ALARMS_PAGE.md).

Screen brightness is set **on the panel itself** (DGUS menu). Do not put it on the settings hub page.

## Display → ESP (controls)

Use **Return Key Code** buttons on VP `0x5400` with auto-upload ON. Do **not** switch pages from the button (Page switching = −1) — the firmware changes pages.

| Key value | Action |
|---:|---|
| 1 | Play / Stop |
| 2 / 3 | Previous / next station |
| 4 / 5 | Open Player / Playlist |
| 6…10 | Settings / Wi‑Fi / Audio / Time / About |
| 11 / 12 | Screen off / wake |
| 15 / 16 | Volume − / + |
| 17…23 | Playlist: play visible row 0…6 |
| 24 / 25 | Playlist: page −7 / +7 |
| 26 | Toggle Telnet CLI |
| 27 | Alarms list |
| 28…30 | Alarm editor slots 1…3 |
| 31…33 | Days: daily / weekdays / weekend |
| 34 / 35 | Snooze / dismiss alarm |
| 36…42 | Toggle day Sun…Sat |

Other input addresses:

| VP | Purpose |
|---:|---|
| `0x5402` | Station number to play |
| `0x5404` | Volume 0…254 |
| `0x5406` / `08` / `0A` / `0C` | EQ and balance (applied live) |
| `0x5410` / `12` | Time zone |
| `0x5416` | Telnet on/off |
| `0x5420`…`0x542A` | Alarm editor fields |
| `0x542C` | Open editor for slot 0…2 |
| `0x5500` / `0x5540` | Wi‑Fi SSID / password |

On the playlist page, keys 2 / 3 / 1 move the list and play the selection (not prev/next over the air).

After every command on `0x5400`, firmware writes `0` back so the same key works again.

## Play button in DGUS Tool

| Field | Correct | Common mistake |
|---|---|---|
| VP | `5400` | `50D6` (icon only) |
| Key value | `1` | `5400` (command becomes 21504) |
| Data auto-upload | ON | OFF — no UART frame |
| Button effect | visual only | Confused with key value |

Expected frame from the display:

```
5A A5 06 83 54 00 01 00 01
```

The Play/Pause icon (`0x50D6`) updates **after** a real start or stop, not at the moment of the press.

## Troubleshooting

Enable `DWIN_DEBUG 1` in `myoptions.h`, Serial 115200.

On boot you should see `LINK OK`. On `LINK FAIL` the firmware retries about every 2 seconds; the UI stays idle until the panel answers.

| Symptom | Check |
|---|---|
| No link | Wiring: DWIN TX→GPIO4, DWIN RX→GPIO2, common GND; UART2 jumper = TTL |
| `LINK OK` but no button frames | HMI not on the panel: Generate → SD → power-cycle with the card |
| `command=21504` | Key value must be `1`, not `5400` |
| Touch works, no sound | Wi‑Fi, playlist, PCM5102 (module SCK to GND) |
| Play works only once | Flash current firmware (ACK clears `0x5400`) |

Editing in DGUS Tool is not enough — update the panel from a microSD `DWIN_SET`.

## Firmware build

| PlatformIO env | Board |
|---|---|
| `stream-one-n16r8` (default) | ESP32-S3 N16R8, 8 MB PSRAM |
| `dev-n8` | Board without PSRAM (lab) |

See root [README](../README.md).

## Wiring

| ESP32-S3 | DWIN | Role |
|---|---|---|
| GPIO2 | RX | ESP → display |
| GPIO4 | TX | Display → ESP |
| GND | GND | Ground |
| 5V | 5V | Panel power |

UART 115200, 3.3 V logic. UART2 jumper = **TTL/CMOS**.

| ESP32-S3 | PCM5102 | Role |
|---|---|---|
| GPIO5 | BCK | Bit clock |
| GPIO6 | LCK / WS | Word select |
| GPIO7 | DIN | Data |
| GND | SCK | Must go to ground (no external MCLK) |
| GND | GND | Ground |

Line-level output — use an amp or active speakers.

Pins: [`yoRadio/myoptions.h`](../yoRadio/myoptions.h).
