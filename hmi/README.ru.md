# Проект HMI для DGUS Tool

[English](README.md) · **Русский**

Папка под проект DGUS Tool для дисплея `DMG10600C070_03WTC` (1024×600). Готовой графики в репозитории нет: страницы, шрифты, картинки и виджеты создаются в DGUS Tool.

## Документация (RU)

| Документ | Содержание |
|---|---|
| [DWIN_SCREEN_MAP.ru.md](DWIN_SCREEN_MAP.ru.md) | контракт VP, wiring, Serial-чеклист |
| [MINIMAL_PLAYER.ru.md](MINIMAL_PLAYER.ru.md) | первый экран Player (play/prev/next/громкость) |
| [PLAYLIST_PAGE.ru.md](PLAYLIST_PAGE.ru.md) | экран плейлиста |
| [WEATHER.ru.md](WEATHER.ru.md) | OpenWeatherMap → DWIN |
| [SETTINGS_PAGE.ru.md](SETTINGS_PAGE.ru.md) | хаб настроек (без яркости через ESP) |
| [FONT_GUIDE.ru.md](FONT_GUIDE.ru.md) | классические ASCII `.bin` шрифты (не DGUS_2) |

## Правила адресов

Все прикладные VP yoRadio начинаются с `0x5000`. Не используйте адреса пульсоксиметра из `src/` (`0x6800` и т.п.). Системные регистры DGUS: `0x0082` (яркость), `0x0084` (смена страницы).

Исходники картинок — в `assets/`, шрифты — в `fonts/`, собранный пакет — в `DWIN_SET/`. Содержимое `DWIN_SET` копируйте на FAT32 microSD, дисплей выключите, вставьте карту, включите и дождитесь окончания обновления.
