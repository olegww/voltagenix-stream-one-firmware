# HMI for DGUS Tool

**English** · [Русский](README.ru.md)

Folder for the DGUS Tool project for `DMG10600C070_03WTC` (1024×600). There is no finished artwork in the repo — build pages, fonts and images in the Tool.

## Documents

| Doc | About |
|---|---|
| [DWIN_SCREEN_MAP.md](DWIN_SCREEN_MAP.md) | Pages, VPs, wiring, common mistakes |
| [MINIMAL_PLAYER.md](MINIMAL_PLAYER.md) | First Player screen |
| [PLAYLIST_PAGE.md](PLAYLIST_PAGE.md) | Playlist |
| [WEATHER.md](WEATHER.md) | OpenWeatherMap |
| [SETTINGS_PAGE.md](SETTINGS_PAGE.md) | Settings hub and related screens |
| [ALARMS_PAGE.md](ALARMS_PAGE.md) | Alarms |
| [FONT_GUIDE.md](FONT_GUIDE.md) | `.bin` fonts for text |

## Addresses

Application variables start at `0x5000`. Panel system registers: `0x0082` (brightness), `0x0084` (page).

Pages: 1 Player … 7 About, 10–11 alarms, 8 screensaver.

Put images in `assets/`, fonts in `fonts/`, the generated package in `DWIN_SET/`. Copy `DWIN_SET` to a FAT32 microSD, power the display off, insert the card, power on and wait for the update to finish.
