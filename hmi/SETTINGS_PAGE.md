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
| Выкл. экран | `000B` | SCREEN_OFF |

Опционально на хабе (read-only): RSSI `50A0`, краткий IP/SSID если заполните Text Display на `50B0` / `5500` (пишет ESP при наличии данных).

**Не класть на хаб:** яркость, API погоды, IR, загрузку плейлиста.

## Page 5 — Audio

| Показ (Data Variable) | VP | Ввод (Drag / Inc) | Диапазон |
|---|---:|---|---|
| Bass | `5302` | `5406` | signed (как в yoRadio) |
| Middle | `5304` | `5408` | signed |
| Treble | `5306` | `540A` | signed |
| Balance | `5308` | `540C` | signed |
| Volume (опц.) | `50D4` | `5404` или keys 15/16 | 0…254 |

Назад: key **`0006`** (хаб) или **`0004`** (Player).

Прошивка уже обрабатывает `SET_BASS`…`SET_BALANCE` в `dwin_display.cpp`.

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

Версия, IP/RSSI/bitrate, режим WEB/SD — по мере добавления текстов в прошивку. Сейчас достаточно RSSI `50A0`, bitrate `50C0`, clock/date.

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

## Порядок сборки HMI

1. Page 3 хаб (кнопки выше).
2. Page 5 Audio.
3. Page 6 Time.
4. Page 4 / 7 по желанию.
5. Generate → SD → прошивка дисплея.
