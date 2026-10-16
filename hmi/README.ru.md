# HMI для DGUS Tool

[English](README.md) · **Русский**

Папка для проекта DGUS Tool под дисплей `DMG10600C070_03WTC` (1024×600). Готовой графики в репозитории нет: страницы, шрифты и картинки собираются в Tool.

## Документы

| Документ | О чём |
|---|---|
| [DWIN_SCREEN_MAP.ru.md](DWIN_SCREEN_MAP.ru.md) | Страницы, VP, подключение, типичные ошибки |
| [MINIMAL_PLAYER.ru.md](MINIMAL_PLAYER.ru.md) | Первый экран Player |
| [PLAYLIST_PAGE.ru.md](PLAYLIST_PAGE.ru.md) | Плейлист |
| [WEATHER.ru.md](WEATHER.ru.md) | Погода OpenWeatherMap |
| [SETTINGS_PAGE.ru.md](SETTINGS_PAGE.ru.md) | Хаб настроек и связанные экраны |
| [ALARMS_PAGE.ru.md](ALARMS_PAGE.ru.md) | Будильники |
| [FONT_GUIDE.ru.md](FONT_GUIDE.ru.md) | Шрифты `.bin` для текста |

## Адреса

Переменные приложения — от `0x5000`. Системные регистры панели: `0x0082` (яркость), `0x0084` (страница).

Страницы: 1 Player … 7 О системе, 10–11 будильники, 8 заставка.

Картинки — в `assets/`, шрифты — в `fonts/`, готовый пакет — в `DWIN_SET/`. Скопируйте `DWIN_SET` на FAT32 microSD, выключите дисплей, вставьте карту, включите и дождитесь обновления.
