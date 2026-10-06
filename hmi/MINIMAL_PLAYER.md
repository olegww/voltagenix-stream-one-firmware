# Минимальный экран Player для yoRadio

Экран **page id 1** — первый рабочий HMI после прошивки ESP32. Все VP из [`dwin_vp.h`](../yoRadio/src/displays/dwin/dwin_vp.h).

## Обязательные виджеты

| Виджет | VP | Тип DGUS | Направление |
|---|---:|---|---|
| Станция | `0x5000` | Scrolling text, coding **8bit**, шрифт с кириллицей | ESP → DWIN (CP1251) |
| Трек (scroll) | `0x5040` | **Scrolling text** only (данные в `5043`) | ESP → DWIN (CP1251) |
| Трек (статика, опц.) | `0x5600` | **Text Display**, Length ≤ 64 | ESP → DWIN (CP1251) |
| Часы | `0x5080` | Text display | ESP → DWIN |
| Дата | `0x5090` | Text display | ESP → DWIN |
| Громкость | `0x50D4` | Data variable (word) | ESP → DWIN |
| Play state | `0x50D6` | **Variable Icon** (word 0/1) | ESP → DWIN |
| VU left | `0x50D0` | Data variable (word) | ESP → DWIN |
| VU right | `0x50D2` | Data variable (word) | ESP → DWIN |
| Play/Pause | `0x5400` | Return Key Code, key **1** | DWIN → ESP |
| Previous | `0x5400` | Return Key Code, key **2** | DWIN → ESP |
| Next | `0x5400` | Return Key Code, key **3** | DWIN → ESP |
| Volume − / + | `0x5400` | Return Key Code, key **15** / **16** | DWIN → ESP |
| Volume number | `0x50D4` | Data variable (word 0…254) | ESP → DWIN |
| Volume slider (optional) | `0x5404` | Slider + auto-upload | DWIN → ESP |

## Текст станции (Scrolling text)

В DGUS Tool у виджета укажите **VP = `5000`** (как в старом проекте VP=`67B4`).

Прошивка пишет данные в **`5003`** (= VP+3), по тому же правилу, что `src` писал в `67B7` при виджете `67B4`.

| Поле | Значение |
|---|---|
| VP (0x) в DGUS | `5000` (станция) / `5040` (трек) |
| Coding mode | `0x00=8bit` |
| FONT0_ID | ASCII/BIN шрифт с нужными глифами |

**Нельзя** вешать обычный Text Display на VP скролла (`5040` / `5000`): первые слова — служебные для анимации, из‑за них «кракозябры» и дёрганье. Статичная копия трека → VP **`5600`**, Text Length **64**.

Обновление:
- станция — только при `NEWSTATION` (смена станции);
- трек — только при реальной смене строки title (`NEWTITLE`, без повторов).

Часы/RSSI/громкость больше **не** перезаписывают scroll и не сбрасывают анимацию.

## Две иконки Play / Stop (важно)

Нужны **два разных виджета** в одном месте экрана:

### 1) Картинка состояния — Variable Icon

1. Добавьте **Variable Icon** (иконка по переменной).
2. **VP (0x)** = `50D6`.
3. Загрузите **2 картинки**:
   - индекс **0** — Stopped (иконка «Play» / треугольник) — воспроизведение выключено;
   - индекс **1** — Playing (иконка «Pause» / две полоски) — идёт эфир.
4. Data Format = Int / word.

Прошивка сама пишет в `0x50D6`:
- `0` — остановлено;
- `1` — играет (в том числе после reboot, если smartstart возобновил эфир).

В Serial при смене состояния: `[DWIN] PLAY_STATE 0x50D6=0` или `=1`.

### 2) Кнопка нажатия — Return Key Code

1. Поверх той же области (или чуть больше) добавьте **Return Key Code**.
2. **VP (0x)** = `5400`.
3. **Key value (0x)** = `1`.
4. **Data auto-uploading** = ON.
5. Button effect = `-1` (без своей смены картинки — картинку меняет Variable Icon).

Неверно: вешать обе роли на один VP `5400` или менять картинку кнопки через Button effect вместо `50D6`.

## Громкость: две кнопки + число

| Виджет | VP | Key / значение |
|---|---:|---|
| Число уровня | **`50D4`** (не `54D4`) | Data Variable, Int, 0…254 (пишет ESP) |
| Кнопка Vol − | **`5400`** | Return Key Code, key = **`000F`** (15), auto-upload ON |
| Кнопка Vol + | **`5400`** | Return Key Code, key = **`0010`** (16), auto-upload ON |

Опционально слайдер громкости:

| Виджет | VP | Важно |
|---|---:|---|
| **Slider Display** (картинка ползунка) | **`50D4`** | Variable type = **Int** (не High byte!). Mode Horizontal. 0…254 |
| **Drag Adjustment** (касание) | **`5404`** | auto-upload ON, **Horizontal**, Int, 0…254 |

Если Variable type = High byte — ползунок всегда на 0 (ESP пишет громкость в младший байт).  
Если Drag = Vertical при широкой полоске — жест не двигает иконку по X.

## Сборка и прошивка дисплея

1. Сохраните проект DGUS Tool в `hmi/`.
2. Сгенерируйте `DWIN_SET/`.
3. Скопируйте содержимое на FAT32 microSD.
4. Выключите дисплей, вставьте SD, включите, дождитесь обновления.

## Проверка

После нажатия Play в Serial Monitor (115200):

```
[DWIN] RX: 5A A5 06 83 54 00 01 00 01
[DWIN] command=1 (VP 0x5400)
[DWIN] PLAY_STATE 0x50D6=1
```

После Stop / reboot со smartstart:

```
[DWIN] PLAY_STATE sync 0x50D6=1
```
(или `=0`, если эфир не запущен)

Если на дисплее старый HMI из `src/` (пульсоксиметр), play/pause с VP `0x6800` key `0x09` тоже работает при `DWIN_LEGACY_PLAYER_VP 1`.

## Следующие виджеты на Player (приоритет)

Прошивка уже пишет/читает эти VP — в DGUS нужно только положить виджеты и прошить дисплей.

| # | Виджет | VP / Key | Тип DGUS | Зачем |
|---:|---|---|---|---|
| 1 | Prev / Next | `5400` key **2** / **3** | Return Key Code | смена станции |
| 2 | Play/Pause кнопка + иконка | `5400` key **1** + Variable Icon `50D6` | см. выше | если ещё нет на экране |
| 3 | Станция | scroll VP **`5000`** (данные в `5003`) | Scrolling text, 8bit | имя станции |
| 4 | Трек | scroll VP **`5040`** (данные в `5043`) | Scrolling text, 8bit | title |
| 5 | Часы / дата | `5080` / `5090` | Text display | время |
| 6 | Bitrate / RSSI | `50C0` / `50A0` | Text display | статус потока/Wi‑Fi |
| 7 | Слайдер громкости | `5404` | Slider 0…254, auto-upload | абсолютный уровень |
| 8 | VU L/R | `50D0` / `50D2` | Data variable / progress | уровень сигнала |
| 9 | Кнопка Playlist | `5400` key **5** | Return Key Code | экран списка (page 2) |

Список станций (page 2): см. [PLAYLIST_PAGE.md](PLAYLIST_PAGE.md) — 7× Text `5100`…`5280`, фон статика, пагинация = смена текста.

Дальше по экранам: Settings hub — [SETTINGS_PAGE.md](SETTINGS_PAGE.md). Яркость — локально в DGUS, не через ESP.

## Часы, битрейт, VU, Playlist — настройки DGUS

Страницы: **Player = page 1**, **Playlist = page 2** (создай page 2 даже как заглушку).

### Часы / dbm / kbps — только Text Display

Прошивка пишет **строки** (`writeText`), не числа. Если стоит **Data Variable Int**, экран показывает «мусор» из первых двух ASCII-байт:

| Поле | VP | Пример мусора | Что это на самом деле |
|---|---:|---:|---|
| Часы | `5080` | `12339` (`0x3033`) | `'0''3'` → начало `03:xx` |
| kbps | `50C0` | `12601` (`0x3139`) | `'1''9'` → начало битрейта |
| dbm | `50A0` | `11574` (`0x2D36`) | `'-''6'` → начало `-67 dBm` |

**Исправление в DGUS:** удали Data Variable → поставь **Text Display** (Data Variable display → нет; нужен именно Text / Text Display):

| Виджет | VP | Длина | Формат |
|---|---:|---:|---|
| Часы | **`5080`** | ≥ 6 | `HH:MM` |
| Дата (опц.) | **`5090`** | ≥ 11 | `DD.MM.YYYY` |
| RSSI/dbm | **`50A0`** | ≥ 10 | `-67 dBm` |
| Bitrate/kbps | **`50C0`** | ≥ 6 | `320` |

Coding **8bit**, шрифт с цифрами. Подписи `dbm` / `kbps` — картинкой рядом.

### VU left / right (Variable Icon)

Прошивка каждые 250 ms пишет **0…100** (громко = больше). Нужен включённый VU в yoRadio (прошивка DWIN включает сама при старте).

| Поле | Left | Right |
|---|---|---|
| Тип | **Variable Icon** | Variable Icon |
| VP (0x) | **`50D0`** | **`50D2`** |
| Icon file | `35_VUm.icl` (или свой) | то же |
| Min → Icon | **0 → 0** | то же |
| Max → Icon | **100 → 39** (или последний кадр ICL) | то же |

Воспроизведение должно идти (`play=1`), иначе уровень ≈ 0.

**Производительность VU (не Curve Display):** опрос ~80 ms, один пакет `0x82` на `50D0`…`50D2` (pad на `50D1`, правый канал остаётся **`50D2`**), TX только при Δ≥2. Curve Display не использовать.

### Кнопка Playlist → page 2

На **page 1**:

| Поле | Значение |
|---|---|
| Тип | Return Key Code |
| VP (0x) | **`5400`** |
| Key value (0x) | **`0005`** (OPEN_PLAYLIST) |
| Data auto-uploading | ON |
| Page switching | **`-1`** (не переключать локально) |

ESP сам пишет page id **2** в системный регистр `0x0084` и заполняет строки плейлиста.

На **page 2** сразу добавь кнопку «Назад / Player»:

| Поле | Значение |
|---|---|
| VP | `5400` |
| Key | **`0004`** (OPEN_PLAYER) |
| Page switching | `-1` |
| auto-upload | ON |

Минимум на page 2 (пока): фон + кнопка назад. Строки списка — позже: Text VP `5100`, `5140`, `5180`… (шаг `0x40`, до 7 строк).

### Проверка в Serial

```
[DWIN] command=5 (VP 0x5400)     ← Playlist
[DWIN] command=4 (VP 0x5400)     ← назад на Player
```

Часы/битрейт обновляются по событиям CLOCK/DBITRATE; VU — без отдельного лога (тихий цикл 250 ms).
