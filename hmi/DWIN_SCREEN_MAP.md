# DGUSII screen, VP, and wiring map

**English** · [Русский](DWIN_SCREEN_MAP.ru.md)

This is the definitive interface contract for the `DMG10600C070_03WTC` (1024x600) display. DGUS values are 16-bit words, high byte first.

Minimal Player screen build guide: [MINIMAL_PLAYER.md](MINIMAL_PLAYER.md) ([RU](MINIMAL_PLAYER.ru.md)).

## Address rule

All yoRadio application variables start at `0x5000`. The only exceptions are `0x0082` (DGUS brightness) and `0x0084` (DGUS page), which are fixed system registers of the display.

Do **not** reuse VP addresses from the pulseoximeter project in `src/` (`0x6800`, `0x6600`, …). That HMI is a different firmware. Legacy play/pause on `0x6800` key `0x09` is accepted temporarily when `DWIN_LEGACY_PLAYER_VP` is enabled in `myoptions.h`.

## Pages

| Page | Purpose |
|---:|---|
| 0 | Boot/loading |
| 1 | Player |
| 2 | Playlist |
| 3 | DWIN settings |
| 4 | Wi-Fi |
| 5 | Audio |
| 6 | Time zone |
| 7 | Information |
| 8 | Screensaver |
| 9 | Dialog/reserved (firmware does **not** open this for VOL/LOST) |
| 10 | Alarms (list) |
| 11 | Alarm (editor) |
| 21...27 | English equivalents of pages 1...7 |
| 30 / 31 | EN: Alarms / Alarm edit (optional) |

## ESP32 to DWIN status fields

| VP | Type | Meaning |
|---:|---|---|
| `0x5000` | Scroll Text (data at `0x5003`), Length **56** | Station name |
| `0x5040` | Scroll Text (data at `0x5043`), Length **56** | Track title (scroll only) |
| `0x5600` | text, length ≤ 64 | Track title static copy (optional font) |
| `0x5080` | text, Length ≥ 8 | Time `HH:MM:SS` (with seconds) |
| `0x5090` | text, Length ≥ 10 | Date `DD.MM.YYYY` |
| `0x50A0` | text | RSSI (`-67 dBm`) |
| `0x50B0` | text, Length ≥ 16 | IP address (`192.168.x.x` / SoftAP) |
| `0x50C0` | text | Bitrate (kbps number) |
| `0x50D0` / `0x50D2` | word | VU left / right |
| `0x50D4` / `0x50D6` | word | Volume 0...254 / **play state 0=stop, 1=play** (Variable Icon) |
| `0x50D8` | word | Station number (1-based) |
| `0x50E0` | text, Length ≥ 8 | Stream format: `MP3` / `AAC` / `FLC` / `OGG` / `WAV` / `-` |
| `0x50F0` | text, Length ≥ 16 | Weekday full name (RU/EN locale, CP1251) |
| `0x5100`...`0x5280` | 7 text fields, stride `0x40` | Playlist names (sliding window) |
| `0x52D0`...`0x52D6` | word | Playlist row numbers (absolute #) |
| `0x52C0` / `0x52C1` | word | Selected station # / highlight row 0…6 |
| `0x5300` | word | Brightness 0...100 |
| `0x5302/0x5304/0x5306` | signed word | Bass/middle/treble (−40…+6) |
| `0x5308` | signed word | Balance (−16…+16) |
| `0x530A/0x530C` | signed word | Time zone hour/minute |
| `0x530E` | word | Language (reserved) |
| `0x5320` | word 0/1 | Telnet CLI on/off (not web UI) |
| `0x5310` | word | Weather icon 0…9 (OWM) |
| `0x5312` / `0x5314` | signed | Temp / feels-like °C |
| `0x5316` / `0x5318` / `0x531A` | word | Pressure mmHg / humidity % / wind m/s |
| `0x531C` | word | Pressure hPa (optional) |
| `0x5620` | text, length ≤ 48 | Weather description |
| `0x5640` | text, Length ≥ 8 | Play mode `WEB` / `SD` |
| `0x5660` | text, Length ≥ 16 | Firmware version (`YOVERSION`) |
| `0x56A0` | text, Length ≥ 32 | Current Wi‑Fi SSID (read-only status) |
| `0x5700`+`i*0x20` | words | Alarm i: en/start/stop/sta/vol/days |
| `0x5800`+`i*0x40` | text, ≥ 40 | Alarm i summary line |
| `0x5760`…`0x576C` | words | Editor: id/en/start/stop/sta/vol/days |
| `0x5770`…`0x5776` | word 0/1 | Day icons Sun…Sat |
| `0x5780` / `0x5782` | word | Activity 0/1/2 / slot # 1…3 |
| `0x58C0` | text, ≥ 32 | Conflict / editor error |
| `0x58E0` | text, ≥ 32 | Status banner on Player |

Playlist: [PLAYLIST_PAGE.md](PLAYLIST_PAGE.md) ([RU](PLAYLIST_PAGE.ru.md)). Weather: [WEATHER.md](WEATHER.md) ([RU](WEATHER.ru.md)). Settings: [SETTINGS_PAGE.md](SETTINGS_PAGE.md) ([RU](SETTINGS_PAGE.ru.md)). Alarms: [ALARMS_PAGE.md](ALARMS_PAGE.md) ([RU](ALARMS_PAGE.ru.md)).

Яркость дисплея настраивается **локально в DGUS** (не через ESP / page 3). VP `0x5300` / `0x540E` в контракте остаются, но на хаб настроек не выносятся.

## DWIN to ESP32 controls

| VP | Value | Action |
|---:|---:|---|
| `0x5400` | 1 | Play/stop toggle |
| `0x5400` | 2 / 3 | Previous / next station |
| `0x5400` | 4 / 5 | Open Player / Playlist |
| `0x5400` | 6 / 7 / 8 / 9 / 10 | Settings / Wi-Fi / Audio / Time / Info |
| `0x5400` | 11 / 12 | Screen off / wake |
| `0x5400` | 13 / 14 | Save Wi-Fi / open screensaver |
| `0x5400` | 15 / 16 | Volume − / Volume + |
| `0x5400` | 17…23 | Playlist: play visible row 0…6 |
| `0x5400` | 24 / 25 | Playlist: page −7 / +7 |
| `0x5402` | 1...playlist length | Play station number |

On page 2, keys **2 / 3 / 1** mean list up / down / play selected (not prev/next station / toggle).
| `0x5404` | 0...254 | Volume (absolute) |
| `0x5406/0x5408/0x540A` | signed word | Bass/middle/treble (live) |
| `0x540C` | signed word | Balance (live) |
| `0x540E` | 0...100 | Brightness |
| `0x5410/0x5412` | signed word | Time zone hour/minute |
| `0x5416` | 0/1 | Telnet CLI on/off |
| `0x5400` | 26 | TOGGLE_TELNET |
| `0x5400` | 27 | OPEN_ALARMS |
| `0x5400` | 28…30 | Alarm editor slots 0…2 |
| `0x5400` | 31…33 | Day presets (daily / weekdays / weekend) |
| `0x5400` | 34 / 35 | Snooze / Dismiss |
| `0x5400` | 36…42 | Toggle day Sun…Sat |
| `0x5420`…`0x542A` | words | SET alarm en/start/stop/sta/vol/days |
| `0x542C` | 0…2 | Open editor for slot |
| `0x5500` / `0x5540` | text | Wi-Fi SSID / password |

After any command on `0x5400`, firmware writes `0` back to that VP so repeated presses with the same key value keep working.

## DGUS Tool: Return Key Code button (Play)

| Field | Correct value | Common mistake |
|---|---|---|
| VP (0x) | `5400` | Using `50D6` (visual icon only) |
| Key value (0x) | `1` | Using `5400` (sends command 21504) |
| Data auto-upload | ON | OFF → no UART frame |
| Button effect | visual only | Confused with key value |

Expected UART frame from display:

```
5A A5 06 83 54 00 01 00 01
```

## Serial diagnostic checklist

Open USB Serial at 115200 with `DWIN_DEBUG 1` in `myoptions.h`.

On boot the firmware probes the display (`read 0x0082`). **`ready` is true only after `LINK OK`**. On `LINK FAIL` it retries every ~2 s (`LINK OK after retry`). The UI queue is not drained until then.

`VOL` does not change page (volume VP only). `LOST` stays on Player with a reconnect hint (page 9 unused). `OPEN_AUDIO` keeps `_mode=AUDIO` in sync. `PLAY_STATE` updates only after a real play/stop.

| Symptom | Likely cause | Action |
|---|---|---|
| No `[DWIN] UART2 started` | Wrong firmware / Serial port | Re-upload; open correct COM at 115200 |
| `LINK FAIL` + boot wait | No panel reply | DWIN **TX→GPIO4**, **RX→GPIO2**, GND; wait for retry or fix UART |
| `LINK OK`, but no bytes on button | HMI not on the panel | In DGUS: **Generate** → copy `DWIN_SET` to FAT32 SD → power-cycle display with SD inserted until update finishes |
| `[DWIN] byte:` / `probe byte:` but no `RX:` | Baud mismatch | Set display UART and `DWIN_BAUD` to 115200 |
| `RX:` VP `0x6800` value `0x09` | Old HMI from `src/` | Reflash yoRadio HMI or keep `DWIN_LEGACY_PLAYER_VP 1` |
| `command=21504` | Key value set to `5400` | Set key value to `1` |
| `read VP=0x50D6` | Button bound to icon VP | Rebind button to VP `0x5400` |
| `command=1` but no audio | No Wi-Fi / no playlist / DAC off | Connect Wi-Fi; PCM5102 on GPIO 5/6/7 |
| Toggle works once only | Fixed in firmware | Flash latest firmware (ACK `0x5400→0`) |

Your DGUS button must be: **Return Key code**, VP=`5400`, key value=`0001`, **Data auto-uploading ON**. Editing in DGUS Tool alone is not enough — the panel must be updated via microSD.

Enable raw byte trace: `#define DWIN_DEBUG_BYTES 1` (default while commissioning).

## PlatformIO envs

| Env | Chip | PSRAM |
|---|---|---|
| `stream-one-n16r8` (default) | N16R8 | 8 MB |
| `dev-n8` | N8 / no PSRAM | none |

See root [README](../README.md).

## Wiring (ESP32-S3 N16R8, proven with DMG10600C070)

| ESP32-S3 | DWIN | Signal |
|---|---|---|
| GPIO2 (TX2) | RX | ESP32 to DWIN |
| GPIO4 (RX2) | TX | DWIN to ESP32 |
| GND | GND | Common ground |
| 5V | 5V | Display power |

UART: 115200 8N1, 3.3 V logic.

| ESP32-S3 | PCM5102 | Signal |
|---|---|---|
| GPIO5 | BCK / BCLK | I2S bit clock |
| GPIO6 | LCK / LRCK / WS | Left/right clock |
| GPIO7 | DIN | I2S data |
| **GND** | **SCK / MCLK** | **обязательно на землю** (режим без внешнего MCLK) |
| GND | GND | Common ground |
| 3V3/5V | VIN | Module power |

Не оставляй **SCK в воздухе** — на типичных модулях GY-PCM5102 без SCK→GND выхода нет.

Также проверь на модуле (часто уже разведено):
- **XSMT** → 3.3 V (unmute);
- **FMT** → GND (формат I2S);
- выход — **линейный** (нужен усилитель / активная колонка), не наушники напрямую.

В Serial при загрузке должно быть: `audio: I2S (7, 5, 6)` = DOUT, BCLK, LRC.

Pin overrides: [`yoRadio/myoptions.h`](../yoRadio/myoptions.h).
