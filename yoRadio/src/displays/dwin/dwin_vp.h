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
enum Page : uint16_t {
  BOOT=0, PLAYER=1, PLAYLIST=2, SETTINGS=3, WIFI=4, AUDIO=5, TIME=6, INFO=7,
  SCREENSAVER=8, DIALOG=9, ALARMS=10, ALARM_EDIT=11,
  PLAYER_EN=21, PLAYLIST_EN=22, SETTINGS_EN=23, WIFI_EN=24, AUDIO_EN=25, TIME_EN=26, INFO_EN=27,
  ALARMS_EN=30, ALARM_EDIT_EN=31
};
constexpr uint16_t STATION=0x5000, TITLE=0x5040, CLOCK=0x5080, DATE=0x5090;
constexpr uint16_t RSSI=0x50A0, NETWORK=0x50B0, BITRATE=0x50C0;
/**
 * Scrolling Text payload length (CP1251) for station `0x5000` / title `0x5040`.
 * Data at VP+3; must stay below next widget (title→clock gap ≈ 61 bytes).
 * Set the same «Text Length» in DGUS Tool.
 */
constexpr uint8_t SCROLL_TEXT_LEN=56;
/** Optional static Text Display copy of title (not the scroll widget VP). */
constexpr uint16_t TITLE_STATIC=0x5600;
constexpr uint8_t TITLE_STATIC_LEN=64;
constexpr uint16_t VU_LEFT=0x50D0, VU_RIGHT=0x50D2, VOLUME=0x50D4, PLAY_STATE=0x50D6;
/** Station # in playlist (1-based). */
constexpr uint16_t STATION_NUM=0x50D8;
/** Codec / stream format text: MP3, AAC, FLC, OGG, WAV (Text Length ≥ 8). */
constexpr uint16_t FORMAT=0x50E0;
constexpr uint8_t FORMAT_LEN=8;
/** Weekday full name (CP1251), Text Length ≥ 16. */
constexpr uint16_t WEEKDAY=0x50F0;
constexpr uint8_t WEEKDAY_LEN=16;
/** Play mode WEB/SD, firmware version, current SSID (read-only status). */
constexpr uint16_t MODE_TEXT=0x5640;
constexpr uint8_t MODE_TEXT_LEN=8;
constexpr uint16_t VERSION=0x5660;
constexpr uint8_t VERSION_LEN=16;
constexpr uint16_t SSID_STATUS=0x56A0;
constexpr uint8_t SSID_STATUS_LEN=32;
/** IP text at NETWORK: Text Length ≥ 16 (e.g. 192.168.1.100). */
constexpr uint8_t NETWORK_LEN=16;
/** Clock with seconds HH:MM:SS — Text Length ≥ 8. */
constexpr uint8_t CLOCK_LEN=8;
/** Date DD.MM.YYYY — Text Length ≥ 10. */
constexpr uint8_t DATE_LEN=10;
/** 7 name texts: 0x5100 + i*0x40.  7 row numbers (word): 0x52D0 + i. */
constexpr uint16_t PLAYLIST_ITEM_0=0x5100, PLAYLIST_NUM_0=0x52D0;
/** Must be ≥ DGUS «Text Length» for playlist names (space-padded wipe). */
constexpr uint8_t PLAYLIST_NAME_LEN = 64;
constexpr uint16_t PLAYLIST_SELECTED=0x52C0, PLAYLIST_ROW=0x52C1;
constexpr uint16_t BRIGHTNESS=0x5300, BASS=0x5302, MIDDLE=0x5304, TREBLE=0x5306, BALANCE=0x5308, TZ_HOUR=0x530A, TZ_MINUTE=0x530C, LANGUAGE=0x530E;
/** Telnet CLI server on/off (0/1) — NOT the web UI. Checkbox / Variable Icon. */
constexpr uint16_t TELNET_STATE=0x5320;
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
/** DWIN → ESP: write 0/1 to enable/disable telnet server (port 23 CLI). */
constexpr uint16_t SET_TELNET=0x5416;
/** Alarm editor inputs (apply to current edit slot). */
constexpr uint16_t SET_ALARM_ENABLED=0x5420;
constexpr uint16_t SET_ALARM_START=0x5422;   // minutes of day 0…1439
constexpr uint16_t SET_ALARM_STOP=0x5424;
constexpr uint16_t SET_ALARM_STATION=0x5426;
constexpr uint16_t SET_ALARM_VOLUME=0x5428;
constexpr uint16_t SET_ALARM_DAYS=0x542A;    // bitmask 0…0x7F
constexpr uint16_t SELECT_ALARM=0x542C;      // 0…2 → open editor for slot
constexpr uint16_t WIFI_SSID=0x5500, WIFI_PASSWORD=0x5540;

/**
 * Alarm list / editor status (ESP → DWIN).
 * Slot i=0…2: base = ALARM0 + i*ALARM_STRIDE.
 * Words: +0 enabled, +2 startMin, +4 stopMin, +6 station, +8 volume, +A days.
 * Text row: ALARM_LINE0 + i*0x40 (Length ≥ 40).
 */
constexpr uint16_t ALARM0=0x5700;
constexpr uint16_t ALARM_STRIDE=0x20;
constexpr uint16_t ALARM_LINE0=0x5800;
constexpr uint8_t ALARM_LINE_LEN=40;
constexpr uint16_t ALARM_EDIT_ID=0x5760;       // 0…2
constexpr uint16_t ALARM_EDIT_ENABLED=0x5762;
constexpr uint16_t ALARM_EDIT_START=0x5764;
constexpr uint16_t ALARM_EDIT_STOP=0x5766;
constexpr uint16_t ALARM_EDIT_STATION=0x5768;
constexpr uint16_t ALARM_EDIT_VOLUME=0x576A;
constexpr uint16_t ALARM_EDIT_DAYS=0x576C;
constexpr uint16_t ALARM_DAY_ICON0=0x5770;     // 7 words Sun…Sat 0/1
constexpr uint16_t ALARM_ACTIVE=0x5780;        // 0=idle 1=playing 2=snooze
constexpr uint16_t ALARM_ACTIVE_ID=0x5782;     // 1…3 (0 if idle)
constexpr uint16_t ALARM_ERR=0x58C0;           // conflict / hint text
constexpr uint8_t ALARM_ERR_LEN=32;
constexpr uint16_t ALARM_STATUS=0x58E0;        // banner on player
constexpr uint8_t ALARM_STATUS_LEN=32;
/** I2S tone / balance ranges (ESP32-audioI2S). */
constexpr int8_t EQ_TONE_MIN = -40;
constexpr int8_t EQ_TONE_MAX = 6;
constexpr int8_t EQ_BALANCE_MIN = -16;
constexpr int8_t EQ_BALANCE_MAX = 16;
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
  PL_PAGE_DOWN = 25,
  /** Toggle telnet CLI (same as SET_TELNET flip). */
  TOGGLE_TELNET = 26,
  OPEN_ALARMS = 27,
  /** Open editor for slot 0 / 1 / 2. */
  ALARM_EDIT_0 = 28,
  ALARM_EDIT_1 = 29,
  ALARM_EDIT_2 = 30,
  /** Days presets for current edit slot. */
  ALARM_DAYS_DAILY = 31,
  ALARM_DAYS_WEEKDAYS = 32,
  ALARM_DAYS_WEEKEND = 33,
  ALARM_SNOOZE = 34,
  ALARM_DISMISS = 35,
  /** Toggle weekday bit 0=Sun … 6=Sat on current edit slot. */
  ALARM_TOGGLE_DAY_0 = 36,
  ALARM_TOGGLE_DAY_1 = 37,
  ALARM_TOGGLE_DAY_2 = 38,
  ALARM_TOGGLE_DAY_3 = 39,
  ALARM_TOGGLE_DAY_4 = 40,
  ALARM_TOGGLE_DAY_5 = 41,
  ALARM_TOGGLE_DAY_6 = 42
};
} // namespace dwin_vp
