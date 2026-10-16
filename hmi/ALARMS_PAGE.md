# Alarms

**English** · [Русский](ALARMS_PAGE.ru.md)

Three independent alarms. Settings are stored in SPIFFS (`/data/alarms.bin`).

If two enabled alarms overlap on the same weekdays, the change is **not applied silently**: the screen shows a conflict message and keeps the previous values.

Pages: **10** — list, **11** — editor. Open from Settings: VP `5400`, key `001B` (27).

All buttons: Return Key Code, VP `5400`, auto-upload ON, Page switching = −1.

## Behaviour

| Parameter | Value |
|---|---|
| Slots | 3 |
| Days | Sun…Sat (individual or presets) |
| Presets | Daily / Mon–Fri / Sat–Sun |
| Start / stop | Time of day; may wrap past midnight |
| Station | Playlist index (1-based) |
| Volume | 0…254 |
| Snooze | 10 minutes |
| Dismiss | Stop the alarm session |
| Auto-stop | At stop time while the session is active |

## Page 10 — list

| On screen | VP |
|---|---:|
| On/off for slots 1…3 | `5700` / `5720` / `5740` |
| Start / stop time | +2 / +4 from each slot base |
| Station, volume, days | +6 / +8 / +A |
| Summary text | `5800` / `5840` / `5880` (length ≥ 40) |
| Status on Player | `58E0` |
| Playing / snoozed | `5780` (0 / 1 / 2) |
| Active slot number | `5782` (1…3) |

| Button | Key | Action |
|---|---:|---|
| Back to Settings | `0006` | Page 3 |
| Player | `0004` | Page 1 |
| Edit 1 / 2 / 3 | `001C` / `001D` / `001E` | Page 11 |
| Snooze / Dismiss | `0022` / `0023` | While alarm is active |

You can also open the editor by writing `0…2` to VP `542C`.

## Page 11 — editor

Changes apply immediately. On conflict, text appears at `58C0` and previous values are restored on screen.

| Field | Display | Input | Range |
|---|---:|---:|---|
| Slot number | `5760` | — | read-only |
| Enabled | `5762` | `5420` | 0 / 1 |
| Start (minutes of day) | `5764` | `5422` | 0…1439 |
| Stop | `5766` | `5424` | 0…1439 |
| Station | `5768` | `5426` | 1…N |
| Volume | `576A` | `5428` | 0…254 |
| Days mask | `576C` | `542A` | Sun…Sat bits |
| Day icons | `5770`…`5776` | keys `0024`…`002A` | toggle day |
| Error | `58C0` | — | text ≥ 32 |

| Button | Key | Action |
|---|---:|---|
| Back to list | `001B` | Page 10 |
| Daily / Weekdays / Weekend | `001F` / `0020` / `0021` | Day preset |
| Sun…Sat | `0024`…`002A` | Toggle day |
| Snooze / Dismiss | `0022` / `0023` | Same as list |

Use IncDec on `5422` / `5424` (step 1 or 5). You may also show `HH:MM` next to the fields.

## Commands on `0x5400`

| Key | Action |
|---:|---|
| 27 | Alarms list |
| 28 / 29 / 30 | Editor slot 1 / 2 / 3 |
| 31 / 32 / 33 | Day presets |
| 34 / 35 | Snooze / dismiss |
| 36…42 | Day Sun…Sat |

## Settings hub

Add an «Alarms» button on page 3: VP `5400`, key `001B`.

## Build order

1. Page 10 (list).
2. Page 11 (editor).
3. Button on page 3.
4. Optional on Player: status `58E0`, icon `5780`, snooze/dismiss.
5. Generate → microSD → flash the panel.
