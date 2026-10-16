# Alarms — page 10 (list) and page 11 (editor)

**English** · [Русский](ALARMS_PAGE.ru.md)

Three independent radio alarms. Storage: SPIFFS `/data/alarms.bin` (not EEPROM). Overlapping enabled windows on shared weekdays are **not applied silently** — UI shows `Конфликт с №N` / conflict text and keeps previous values.

Page id: **10** (`ALARMS`), **11** (`ALARM_EDIT`). Open from Settings hub: VP `5400`, key **`001B`** (27).

All buttons: Return Key Code, VP **`5400`**, auto-upload ON, **Page switching = -1**.

## Behaviour

| Parameter | Value |
|---|---|
| Slots | 3 |
| Days | `tm_wday` bitmask: bit0=Sun … bit6=Sat |
| Presets | daily `0x7F`, weekdays `0x3E`, weekend `0x41` |
| Start / stop | minutes of day 0…1439 (may wrap midnight) |
| Station | 1-based playlist index |
| Volume | 0…254 |
| Snooze | 10 minutes |
| Dismiss | stop alarm session |
| Auto-stop | at `stopMin` while session active |

## Page 10 — list

| Widget | VP | Type |
|---|---:|---|
| Slot 0…2 enabled | `5700` / `5720` / `5740` | word 0/1 |
| startMin / stopMin | `+0x02` / `+0x04` | word |
| station / volume / days | `+0x06` / `+0x08` / `+0x0A` | word |
| Summary line | `5800` / `5840` / `5880` | text, Length ≥ 40 |
| Status banner | `58E0` | text, Length ≥ 32 |
| Activity | `5780` | 0=idle, 1=playing, 2=snooze |
| Active # | `5782` | 1…3 (0 if idle) |

| Button | Key | Action |
|---|---:|---|
| Back / Settings | `0006` | page 3 |
| Player | `0004` | page 1 |
| Edit #1/2/3 | `001C` / `001D` / `001E` | page 11, slot 0/1/2 |
| Snooze / Dismiss | `0022` / `0023` | during session |

Or write `0…2` to VP **`542C`** to open the editor.

## Page 11 — editor

Changes apply immediately. On conflict: message at `58C0`, firmware rewrites fields to previous values.

| Display | VP | Input (DWIN→ESP) | Range |
|---|---:|---|---|
| Slot id (0…2) | `5760` | — | read-only |
| Enabled | `5762` | `5420` | 0/1 |
| Start (min) | `5764` | `5422` | 0…1439 |
| Stop (min) | `5766` | `5424` | 0…1439 |
| Station | `5768` | `5426` | 1…N |
| Volume | `576A` | `5428` | 0…254 |
| Days bitmask | `576C` | `542A` | 0…0x7F |
| Day icons Sun…Sat | `5770`…`5776` | keys `0024`…`002A` | toggle |
| Error / conflict | `58C0` | — | text, Length ≥ 32 |

| Button | Key | Action |
|---|---:|---|
| Back to list | `001B` | OPEN_ALARMS |
| Daily / Weekdays / Weekend | `001F` / `0020` / `0021` | day preset |
| Toggle Sun…Sat | `0024`…`002A` | XOR day bit |
| Snooze / Dismiss | `0022` / `0023` | |

Use Drag/IncDec on `5422`/`5424` (step 1 or 5). Optional local HH:MM display via DGUS math.

## Commands `0x5400`

| Key (dec) | Command |
|---:|---|
| 27 | OPEN_ALARMS |
| 28 / 29 / 30 | ALARM_EDIT_0 / 1 / 2 |
| 31 / 32 / 33 | DAYS daily / weekdays / weekend |
| 34 / 35 | SNOOZE / DISMISS |
| 36…42 | TOGGLE_DAY Sun…Sat |

## Settings hub

Add an «Alarms» button on page 3: VP `5400`, key **`001B`**.

## HMI build order

1. Page 10 list.
2. Page 11 editor.
3. Hub button on page 3.
4. Optional Player banner `58E0` / icon `5780` + snooze/dismiss.
5. Generate → SD → flash panel.
