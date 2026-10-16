# Settings — page 3 (хаб) и связанные экраны

Яркость экрана **не выносится на ESP**: настраивается средствами самого DGUS / меню дисплея, без VP `5300` / `540E` на этой странице.

Page id: **3** (`dwin_vp::SETTINGS`). Вход с Player: VP `5400`, key **`0006`**.

Все кнопки: Return Key Code, VP **`5400`**, auto-upload ON, **Page switching = -1** (страницу переключает ESP).

## Page 3 — хаб «Настройки»

Статичный фон + навигация:

| Кнопка | Key (hex) | Действие |
|---|---:|---|
| Назад / Player | `0004` | page 1 |
| Звук / EQ | `0008` | page 5 (Audio) |
| Время | `0009` | page 6 (Time) |
| Сеть | `0007` | page 4 (Wi‑Fi статус) |
| О системе | `000A` | page 7 (Info) |
| Будильники / Alarms | `001B` | page 10 (Alarms) |
| Выкл. экран | `000B` | SCREEN_OFF |

Опционально на хабе (read-only): RSSI `50A0`, краткий IP/SSID если заполните Text Display на `50B0` / `5500` (пишет ESP при наличии данных).

**Не класть на хаб:** яркость, API погоды, IR, загрузку плейлиста.

## Page 5 — Audio (EQ)

Changes apply **live** (audio + EEPROM). No separate Save/OK button.

| Display (Data Variable Int, signed) | VP | Input (Drag / IncDec) | Range |
|---|---:|---|---|
| Bass | `5302` | `5406` | **−40…+6** dB |
| Middle | `5304` | `5408` | **−40…+6** dB |
| Treble | `5306` | `540A` | **−40…+6** dB |
| Balance | `5308` | `540C` | **−16…+16** |
| Volume (opt.) | `50D4` | `5404` or keys 15/16 | 0…254 |

DGUS: Variable type = **Int** (signed), auto-upload ON. Min/Max as above.

Open: key **`0008`** (`OPEN_AUDIO`) — firmware sets mode `AUDIO` and page 5 (not a bare `page()` without `_mode`). Back: **`0006`** / **`0004`**.

## Telnet (not the web UI)

Web «Telnet server» toggles the **port 23 CLI**, not HTTP. Web UI keeps working either way.

Optional checkbox on Settings hub / Info:

| Widget | VP | Direction |
|---|---:|---|
| State (Variable Icon 0/1) | `5320` | ESP → DWIN |
| Dual State Button | `5416` | DWIN → ESP (`0`/`1`) |
| or toggle key | `5400` key **`001A`** (26) | TOGGLE_TELNET |

Label on screen: «Telnet CLI», not «Web».

## Page 6 — Time

| Показ | VP | Ввод |
|---|---:|---|
| TZ hour | `530A` | `5410` (±) |
| TZ minute | `530C` | `5412` (±) |
| Часы / дата (read-only) | `5080` / `5090` | — |

Назад: `0006` / `0004`.

## Page 4 — Wi‑Fi (статус)

Без экранной клавиатуры в первой версии:

- Text: SSID `5500`, опционально IP на `50B0`
- RSSI `50A0`
- Подпись: смена сети — через веб `/settings.html`

Назад: `0006` / `0004`.

## Page 7 — Info (read-only)

Прошивка пишет все поля ниже — в DGUS положите Text Display / Data Variable:

| Показ | VP | Пример |
|---|---:|---|
| Версия | `5660` | `0.9.720` |
| IP | `50B0` | `192.168.1.10` |
| SSID | `56A0` | имя сети |
| RSSI | `50A0` | `-67 dBm` |
| Bitrate | `50C0` | `320` |
| Формат | `50E0` | `AAC` |
| Режим | `5640` | `WEB` / `SD` |
| № станции | `50D8` | word |
| Часы / дата / день | `5080` / `5090` / `50F0` | `HH:MM:SS` / `DD.MM.YYYY` / `понедельник` |

## Команды (напоминание)

| Key | Command |
|---:|---|
| 4 | OPEN_PLAYER |
| 5 | OPEN_PLAYLIST |
| 6 | OPEN_SETTINGS |
| 7 | OPEN_WIFI |
| 8 | OPEN_AUDIO |
| 9 | OPEN_TIME |
| 10 | OPEN_INFO |
| 11 | SCREEN_OFF |
| 27 | OPEN_ALARMS |

Alarms: [ALARMS_PAGE.md](ALARMS_PAGE.md) (3 slots, weekdays, visible conflict, 10 min snooze).

## HMI build order

1. Page 3 hub (buttons above).
2. Page 5 Audio.
3. Page 6 Time.
4. Page 10 / 11 Alarms.
5. Page 4 / 7 as needed.
6. Generate → SD → flash panel.
