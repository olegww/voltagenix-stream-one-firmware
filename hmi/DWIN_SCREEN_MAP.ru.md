# Карта экранов, VP и подключения DGUS II

[English](DWIN_SCREEN_MAP.md) · **Русский**

Контракт интерфейса для дисплея `DMG10600C070_03WTC` (1024×600). Значения DGUS — 16-битные слова, старший байт первым.

Сборка минимального Player: [MINIMAL_PLAYER.ru.md](MINIMAL_PLAYER.ru.md).

## Правило адресов

Все прикладные переменные yoRadio начинаются с `0x5000`. Исключения: `0x0082` (яркость DGUS) и `0x0084` (страница) — системные регистры панели.

**Не** переиспользуйте VP из проекта пульсоксиметра в `src/` (`0x6800`, `0x6600`, …). Временно play/pause на `0x6800` key `0x09` принимается, если в `myoptions.h` включён `DWIN_LEGACY_PLAYER_VP`.

## Страницы

| Page | Назначение |
|---:|---|
| 0 | Загрузка |
| 1 | Player |
| 2 | Плейлист |
| 3 | Настройки DWIN (хаб) |
| 4 | Wi‑Fi |
| 5 | Аудио / EQ |
| 6 | Часовой пояс |
| 7 | Информация |
| 8 | Заставка |
| 9 | Диалог / резерв (прошивка **не** уходит сюда на VOL/LOST) |
| 10 | Будильники (список) |
| 11 | Будильник (редактор) |
| 21…27 | Английские аналоги страниц 1…7 |
| 30 / 31 | EN: Alarms / Alarm edit (опц.) |

## ESP32 → DWIN (статус)

| VP | Тип | Смысл |
|---:|---|---|
| `0x5000` | Scroll Text (данные в `0x5003`), Length **56** | Имя станции |
| `0x5040` | Scroll Text (данные в `0x5043`), Length **56** | Трек (только scroll) |
| `0x5600` | text, ≤ 64 | Статичная копия трека (опц.) |
| `0x5080` | text, Length ≥ 8 | Время `HH:MM:SS` (с секундами) |
| `0x5090` | text, Length ≥ 10 | Дата `DD.MM.YYYY` |
| `0x50A0` | text | RSSI (`-67 dBm`) |
| `0x50B0` | text, Length ≥ 16 | IP (`192.168.x.x` / SoftAP) |
| `0x50C0` | text | Bitrate (кбит/с) |
| `0x50D0` / `0x50D2` | word | VU left / right |
| `0x50D4` / `0x50D6` | word | Громкость 0…254 / **play: 0=stop, 1=play** (Variable Icon) |
| `0x50D8` | word | Номер станции (с 1) |
| `0x50E0` | text, Length ≥ 8 | Формат потока: `MP3` / `AAC` / `FLC` / `OGG` / `WAV` / `-` |
| `0x50F0` | text, Length ≥ 16 | День недели (полное имя, CP1251) |
| `0x5100`…`0x5280` | 7 text, шаг `0x40` | Имена в окне плейлиста |
| `0x52D0`…`0x52D6` | word | Номера строк плейлиста |
| `0x52C0` / `0x52C1` | word | Выбранная станция # / подсветка строки 0…6 |
| `0x5300` | word | Яркость 0…100 (контракт; на хаб не выносится) |
| `0x5302/04/06` | signed word | Bass / middle / treble (−40…+6) |
| `0x5308` | signed word | Balance (−16…+16) |
| `0x530A/0x530C` | signed word | TZ hour / minute |
| `0x530E` | word | Язык (резерв) |
| `0x5320` | word 0/1 | Telnet CLI вкл/выкл (не веб) |
| `0x5310` | word | Иконка погоды 0…9 (OWM) |
| `0x5312` / `0x5314` | signed | Темп. / ощущается °C |
| `0x5316` / `0x5318` / `0x531A` | word | Давление мм рт. ст. / влажность % / ветер м/с |
| `0x531C` | word | Давление гПа (опц.) |
| `0x5620` | text, ≤ 48 | Описание погоды |
| `0x5640` | text, Length ≥ 8 | Режим `WEB` / `SD` |
| `0x5660` | text, Length ≥ 16 | Версия прошивки (`YOVERSION`) |
| `0x56A0` | text, Length ≥ 32 | Текущий SSID Wi‑Fi (только показ) |
| `0x5700`+`i*0x20` | words | Будильник i: en/start/stop/sta/vol/days |
| `0x5800`+`i*0x40` | text, ≥ 40 | Сводка будильника i |
| `0x5760`…`0x576C` | words | Редактор: id/en/start/stop/sta/vol/days |
| `0x5770`…`0x5776` | word 0/1 | Иконки дней Вс…Сб |
| `0x5780` / `0x5782` | word | Активность 0/1/2 / № слота 1…3 |
| `0x58C0` | text, ≥ 32 | Конфликт / ошибка редактора |
| `0x58E0` | text, ≥ 32 | Баннер статуса на Player |

Плейлист: [PLAYLIST_PAGE.ru.md](PLAYLIST_PAGE.ru.md). Погода: [WEATHER.ru.md](WEATHER.ru.md). Настройки: [SETTINGS_PAGE.ru.md](SETTINGS_PAGE.ru.md). Будильники: [ALARMS_PAGE.ru.md](ALARMS_PAGE.ru.md).

Яркость экрана — **локально в DGUS**, не через ESP / page 3. VP `0x5300` / `0x540E` в контракте есть, на хаб не кладём.

## DWIN → ESP32 (управление)

| VP | Значение | Действие |
|---:|---:|---|
| `0x5400` | 1 | Play/stop |
| `0x5400` | 2 / 3 | Пред. / след. станция |
| `0x5400` | 4 / 5 | Открыть Player / Playlist |
| `0x5400` | 6 / 7 / 8 / 9 / 10 | Settings / Wi‑Fi / Audio / Time / Info |
| `0x5400` | 11 / 12 | Выкл. экран / wake |
| `0x5400` | 13 / 14 | Сохранить Wi‑Fi / заставка |
| `0x5400` | 15 / 16 | Громкость − / + |
| `0x5400` | 17…23 | Плейлист: play видимой строки 0…6 |
| `0x5400` | 24 / 25 | Плейлист: страница −7 / +7 |
| `0x5402` | 1…N | Play станции по номеру |
| `0x5404` | 0…254 | Громкость (абсолютная) |
| `0x5406/08/0A` | signed | Bass / middle / treble (live) |
| `0x540C` | signed | Balance (live) |
| `0x540E` | 0…100 | Яркость |
| `0x5410/12` | signed | TZ hour / minute |
| `0x5416` | 0/1 | Telnet CLI on/off |
| `0x5400` | 26 | TOGGLE_TELNET |
| `0x5400` | 27 | OPEN_ALARMS |
| `0x5400` | 28…30 | Редактор будильника 0…2 |
| `0x5400` | 31…33 | Пресет дней (ежедн / Пн–Пт / СбВс) |
| `0x5400` | 34 / 35 | Snooze / Dismiss |
| `0x5400` | 36…42 | Toggle дня Вс…Сб |
| `0x5420`…`0x542A` | words | SET alarm en/start/stop/sta/vol/days |
| `0x542C` | 0…2 | Открыть редактор слота |
| `0x5500` / `0x5540` | text | Wi‑Fi SSID / пароль |

На page 2 ключи **2 / 3 / 1** — курсор вверх / вниз / play выбранной (не prev/next станции / toggle).

После любой команды на `0x5400` прошивка пишет туда `0`, чтобы повторное нажатие с тем же key снова сработало.

## DGUS Tool: кнопка Return Key Code (Play)

| Поле | Правильно | Частая ошибка |
|---|---|---|
| VP (0x) | `5400` | `50D6` (только иконка) |
| Key value (0x) | `1` | `5400` (command=21504) |
| Data auto-upload | ON | OFF → нет кадра UART |
| Button effect | только визуал | Путают с key value |

Ожидаемый кадр с дисплея:

```
5A A5 06 83 54 00 01 00 01
```

## Serial-чеклист

USB Serial 115200, в `myoptions.h`: `DWIN_DEBUG 1`.

При старте прошивка опрашивает дисплей (`read 0x0082`). **`ready` = true только после `LINK OK`**. При `LINK FAIL` — повтор каждые ~2 с (`LINK OK after retry`). UI-очередь до этого не дренируется.

`VOL` не меняет page (только VP громкости). `LOST` остаётся на Player с текстом «Нет Wi-Fi» / «Переподключение…» (page 9 не используется). `OPEN_AUDIO` синхронизирует `_mode=AUDIO`. `PLAY_STATE` обновляется только после реального play/stop.

| Симптом | Вероятная причина | Действие |
|---|---|---|
| Нет `[DWIN] UART2 started` | Не та прошивка / COM | Перепрошить; COM на 115200 |
| `LINK FAIL` + зависание boot | Нет ответа панели | DWIN **TX→GPIO4**, **RX→GPIO2**, GND; дождаться retry или починить UART |
| `LINK OK`, нет байт по кнопке | HMI не на панели | DGUS: **Generate** → `DWIN_SET` на FAT32 SD → цикл питания с SD |
| Есть `byte:` / нет `RX:` | Baud | UART панели и `DWIN_BAUD` = 115200 |
| `RX:` VP `0x6800` value `0x09` | Старый HMI из `src/` | Прошить yoRadio HMI или `DWIN_LEGACY_PLAYER_VP 1` |
| `command=21504` | Key = `5400` | Key value = `1` |
| `read VP=0x50D6` | Кнопка на иконке | Перевязать на VP `0x5400` |
| `command=1`, нет звука | Wi‑Fi / плейлист / DAC | Wi‑Fi; PCM5102 GPIO 5/6/7; SCK→GND |
| Toggle один раз | Уже исправлено | Свежая прошивка (ACK `0x5400→0`) |

Кнопка: **Return Key code**, VP=`5400`, key=`0001`, **Data auto-uploading ON**. Правки только в Tool недостаточно — панель обновляется с microSD.

Сырые байты: `#define DWIN_DEBUG_BYTES 1`.

## Сборка PIO (два env)

| Env | Чип | PSRAM |
|---|---|---|
| `stream-one-n16r8` (default) | N16R8 | 8 MB |
| `dev-n8` | N8 / без PSRAM | нет |

Подробнее: корневой [README](../README.md).

## Подключение (ESP32-S3 N16R8 + DMG10600C070)

| ESP32-S3 | DWIN | Сигнал |
|---|---|---|
| GPIO2 (TX2) | RX | ESP → DWIN |
| GPIO4 (RX2) | TX | DWIN → ESP |
| GND | GND | Общая земля |
| 5V | 5V | Питание панели |

UART: 115200 8N1, логика 3.3 V. Джампер UART2 = **TTL/CMOS**.

| ESP32-S3 | PCM5102 | Сигнал |
|---|---|---|
| GPIO5 | BCK / BCLK | I2S bit clock |
| GPIO6 | LCK / LRCK / WS | Word select |
| GPIO7 | DIN | I2S data |
| **GND** | **SCK / MCLK** | **на землю** (без внешнего MCLK) |
| GND | GND | Общая земля |
| 3V3/5V | VIN | Питание модуля |

Не оставляйте **SCK в воздухе**. На модуле часто уже разведено: **XSMT**→3.3 V, **FMT**→GND. Выход линейный.

В Serial при загрузке: `audio: I2S (7, 5, 6)` = DOUT, BCLK, LRC.

Пины: [`yoRadio/myoptions.h`](../yoRadio/myoptions.h).
