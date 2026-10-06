#pragma once
#include <stdint.h>
/**
 * DGUS VP contract. Configure every DGUS Tool widget to these addresses.
 *
 * Application VPs deliberately start at 0x5000.  0x0082 and 0x0084 are
 * fixed DGUS system registers, not application fields.
 */
namespace dwin_vp {
constexpr uint16_t SYSTEM_PAGE = 0x0084, SYSTEM_BRIGHTNESS = 0x0082;
enum Page : uint16_t { BOOT=0, PLAYER=1, PLAYLIST=2, SETTINGS=3, WIFI=4, AUDIO=5, TIME=6, INFO=7, SCREENSAVER=8, DIALOG=9, PLAYER_EN=21, PLAYLIST_EN=22, SETTINGS_EN=23, WIFI_EN=24, AUDIO_EN=25, TIME_EN=26, INFO_EN=27 };
constexpr uint16_t STATION=0x5000, TITLE=0x5040, CLOCK=0x5080, DATE=0x5090;
constexpr uint16_t RSSI=0x50A0, NETWORK=0x50B0, BITRATE=0x50C0;
/** Optional static Text Display copy of title (not the scroll widget VP). */
constexpr uint16_t TITLE_STATIC=0x5600;
constexpr uint8_t TITLE_STATIC_LEN=64;
constexpr uint16_t VU_LEFT=0x50D0, VU_RIGHT=0x50D2, VOLUME=0x50D4, PLAY_STATE=0x50D6;
/** 7 name texts: 0x5100 + i*0x40.  7 row numbers (word): 0x52D0 + i. */
constexpr uint16_t PLAYLIST_ITEM_0=0x5100, PLAYLIST_NUM_0=0x52D0;
/** Must be ≥ DGUS «Text Length» for playlist names (space-padded wipe). */
constexpr uint8_t PLAYLIST_NAME_LEN = 64;
constexpr uint16_t PLAYLIST_SELECTED=0x52C0, PLAYLIST_ROW=0x52C1;
constexpr uint16_t BRIGHTNESS=0x5300, BASS=0x5302, MIDDLE=0x5304, TREBLE=0x5306, BALANCE=0x5308, TZ_HOUR=0x530A, TZ_MINUTE=0x530C, LANGUAGE=0x530E;
/** OpenWeatherMap → DWIN. Icon index 0…9 (same mapping as Nextion). */
constexpr uint16_t WEATHER_ICON=0x5310;
constexpr uint16_t WEATHER_TEMP=0x5312;   // signed °C (integer)
constexpr uint16_t WEATHER_FEELS=0x5314;  // signed °C (integer)
constexpr uint16_t WEATHER_PRESS=0x5316;  // mmHg
constexpr uint16_t WEATHER_HUM=0x5318;    // %
constexpr uint16_t WEATHER_WIND=0x531A;   // m/s (integer)
constexpr uint16_t WEATHER_PRESS_HPA=0x531C; // hPa (optional, matches OWM site)
constexpr uint16_t WEATHER_DESC=0x5620;   // Text Display, CP1251
constexpr uint8_t WEATHER_DESC_LEN=48;
constexpr uint16_t COMMAND=0x5400, SELECT_STATION=0x5402, SET_VOLUME=0x5404, SET_BASS=0x5406, SET_MIDDLE=0x5408, SET_TREBLE=0x540A, SET_BALANCE=0x540C, SET_BRIGHTNESS=0x540E, SET_TZ_HOUR=0x5410, SET_TZ_MINUTE=0x5412, SET_LANGUAGE=0x5414;
constexpr uint16_t WIFI_SSID=0x5500, WIFI_PASSWORD=0x5540;
enum Command : uint16_t {
  PLAY_TOGGLE = 1,
  PREVIOUS,
  NEXT,
  OPEN_PLAYER,
  OPEN_PLAYLIST,
  OPEN_SETTINGS,
  OPEN_WIFI,
  OPEN_AUDIO,
  OPEN_TIME,
  OPEN_INFO,
  SCREEN_OFF,
  SCREEN_ON,
  SAVE_WIFI,
  SHOW_SCREENSAVER,
  VOL_DOWN, // 15
  VOL_UP,   // 16
  /** Playlist page: play visible row 0…6 (window of 7). */
  PL_ROW_0 = 17,
  PL_ROW_1,
  PL_ROW_2,
  PL_ROW_3,
  PL_ROW_4,
  PL_ROW_5,
  PL_ROW_6,
  /** Playlist page: jump window by ±7 stations. */
  PL_PAGE_UP = 24,
  PL_PAGE_DOWN = 25
};
} // namespace dwin_vp
