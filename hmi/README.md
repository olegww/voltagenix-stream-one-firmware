# HMI project for DGUS Tool

**English** · [Русский](README.ru.md)

This folder is for the DGUS Tool project for `DMG10600C070_03WTC` at 1024x600. It contains no finished graphics: create the pages, fonts, images and controls in DGUS Tool.

Documentation:

- [DWIN_SCREEN_MAP.md](DWIN_SCREEN_MAP.md) ([RU](DWIN_SCREEN_MAP.ru.md)) — full VP contract, wiring, serial diagnostic checklist
- [MINIMAL_PLAYER.md](MINIMAL_PLAYER.md) ([RU](MINIMAL_PLAYER.ru.md)) — step-by-step first Player screen
- [PLAYLIST_PAGE.md](PLAYLIST_PAGE.md) ([RU](PLAYLIST_PAGE.ru.md)) — playlist page
- [WEATHER.md](WEATHER.md) ([RU](WEATHER.ru.md)) — OpenWeatherMap → DWIN
- [SETTINGS_PAGE.md](SETTINGS_PAGE.md) ([RU](SETTINGS_PAGE.ru.md)) — settings hub (no ESP brightness)
- [FONT_GUIDE.md](FONT_GUIDE.md) ([RU](FONT_GUIDE.ru.md)) — classic ASCII `.bin` fonts (not DGUS_2)

All yoRadio application VPs begin at `0x5000`. Do not use former `0x2000`/`0x3000` addresses or pulseoximeter addresses from `src/` (`0x6800`, …) in a new yoRadio HMI. The only lower addresses used by firmware are the fixed DGUS system registers: `0x0082` for brightness and `0x0084` for page switching.

Place source images in `assets/`, fonts in `fonts/`, and the generated DWIN package in `DWIN_SET/`. Copy the generated `DWIN_SET` contents to a FAT32 microSD card, power the display off, insert the card, then power it on. Wait for the DWIN update to finish before removing the card.
