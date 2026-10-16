# Settings hub and related screens

**English** · [Русский](SETTINGS_PAGE.ru.md)

Screen brightness is set **in the panel menu**, not via the ESP. Do not put brightness VPs on this page.

Page **3**. Open from Player: VP `5400`, key `0006`.

All buttons: Return Key Code, VP `5400`, auto-upload ON, Page switching = −1 (firmware changes pages).

## Page 3 — hub

| Button | Key | Goes to |
|---|---:|---|
| Back / Player | `0004` | Page 1 |
| Sound / EQ | `0008` | Page 5 |
| Time | `0009` | Page 6 |
| Network | `0007` | Page 4 |
| About | `000A` | Page 7 |
| Alarms | `001B` | Page 10 |
| Screen off | `000B` | Turns backlight off |

Optional on the hub: RSSI `50A0`, short IP/SSID (`50B0` / `56A0`).

Do not put here: brightness, weather API, IR, playlist upload.

## Page 5 — Audio (EQ)

Changes apply **immediately**; no separate Save button.

| Display | VP | Input | Range |
|---|---:|---|---|
| Bass | `5302` | `5406` | −40…+6 dB |
| Middle | `5304` | `5408` | −40…+6 dB |
| Treble | `5306` | `540A` | −40…+6 dB |
| Balance | `5308` | `540C` | −16…+16 |
| Volume (optional) | `50D4` | `5404` or keys 15/16 | 0…254 |

In DGUS: Int (signed), auto-upload ON, Min/Max as in the table.

Open: key `0008`. Back: `0006` or `0004`.

## Telnet CLI

The “Telnet server” flag in the web settings enables a CLI on port 23.

On the hub or About page you can add:

| Widget | VP |
|---|---:|
| State icon 0/1 | `5320` |
| Toggle | `5416` (writes 0/1) |
| or button | `5400` key `001A` (26) |

## Page 6 — Time

| Display | VP | Input |
|---|---:|---|
| TZ hours | `530A` | `5410` |
| TZ minutes | `530C` | `5412` |
| Clock / date (read-only) | `5080` / `5090` | — |

Back: `0006` / `0004`.

## Page 4 — Wi‑Fi

No on-screen keyboard in the first version:

- SSID `5500`, IP on `50B0`
- RSSI `50A0`
- Change network via web `/settings.html`

## Page 7 — About

| Field | VP | Example |
|---|---:|---|
| Version | `5660` | `0.9.720` |
| IP | `50B0` | `192.168.1.10` |
| SSID | `56A0` | network name |
| RSSI | `50A0` | `-67 dBm` |
| Bitrate | `50C0` | `320` |
| Format | `50E0` | `AAC` |
| Mode | `5640` | `WEB` / `SD` |
| Station # | `50D8` | number |
| Clock / date / weekday | `5080` / `5090` / `50F0` | |

## Commands (short)

| Key | Action |
|---:|---|
| 4 | Player |
| 5 | Playlist |
| 6 | Settings |
| 7 | Wi‑Fi |
| 8 | Audio |
| 9 | Time |
| 10 | About |
| 11 | Screen off |
| 27 | Alarms |

Alarms details: [ALARMS_PAGE.md](ALARMS_PAGE.md).

## Build order

1. Page 3 (hub).
2. Page 5 (audio).
3. Page 6 (time).
4. Pages 10 / 11 (alarms).
5. Pages 4 / 7 as needed.
6. Generate → microSD → flash the panel.
