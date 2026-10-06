# HMI project for DGUS Tool

This folder is for the DGUS Tool project for `DMG10600C070_03WTC` at 1024x600. It contains no finished graphics: create the pages, fonts, images and controls in DGUS Tool.

Documentation:

- [DWIN_SCREEN_MAP.md](DWIN_SCREEN_MAP.md) — full VP contract, wiring, serial diagnostic checklist
- [MINIMAL_PLAYER.md](MINIMAL_PLAYER.md) — step-by-step first Player screen (play/prev/next/volume)
- [PLAYLIST_PAGE.md](PLAYLIST_PAGE.md) — playlist page
- [WEATHER.md](WEATHER.md) — OpenWeatherMap → DWIN
- [SETTINGS_PAGE.md](SETTINGS_PAGE.md) — settings hub (no ESP brightness)
- [FONT_GUIDE.md](FONT_GUIDE.md) — how to make a working classic ASCII `.bin` font (not DGUS_2)

All yoRadio application VPs begin at `0x5000`. Do not use former `0x2000`/`0x3000` addresses or pulseoximeter addresses from `src/` (`0x6800`, …) in a new yoRadio HMI. The only lower addresses used by firmware are the fixed DGUS system registers: `0x0082` for brightness and `0x0084` for page switching.

Place source images in `assets/`, fonts in `fonts/`, and the generated DWIN package in `DWIN_SET/`. Copy the generated `DWIN_SET` contents to a FAT32 microSD card, power the display off, insert the card, then power it on. Wait for the DWIN update to finish before removing the card.
