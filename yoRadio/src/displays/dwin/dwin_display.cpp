#include "dwin_display.h"
#include "dwin_vp.h"
#include "../../core/options.h"
#include "../../core/config.h"
#include "../../core/network.h"
#include "../../core/player.h"
#include "../../core/timekeeper.h"
#include "../../core/display.h"
#include "../../core/telnet.h"
#include "../../core/alarms.h"
#include "../tools/l10n.h"
#include <WiFi.h>

/** Human-readable codec label for DWIN Text Display. */
static const char *dwinFormatLabel(BitrateFormat fmt) {
  switch (fmt) {
    case BF_MP3:  return "MP3";
    case BF_AAC:  return "AAC";
    case BF_FLAC: return "FLC";
    case BF_OGG:  return "OGG";
    case BF_WAV:  return "WAV";
    default:      return "-";
  }
}

DwinDisplay dwin;

void DwinDisplay::writeRaw(const uint8_t *data, uint8_t len) {
  _serial.write(data, len);
  _serial.flush();
}

/** Convert UTF-8 (yoRadio) to Windows-1251 for DGUS 8-bit scroll fonts. */
static size_t utf8ToCp1251(const char *src, uint8_t *dst, size_t dstMax) {
  if (!src || !dst || dstMax == 0) return 0;
  size_t o = 0;
  const uint8_t *p = (const uint8_t *)src;
  while (*p && o + 1 < dstMax) {
    uint8_t c = *p++;
    if (c < 0x80) {
      dst[o++] = c;
      continue;
    }
    if ((c & 0xE0) == 0xC0 && *p) {
      uint16_t u = ((c & 0x1F) << 6) | (*p++ & 0x3F);
      uint8_t out = '?';
      if (u == 0x401) out = 0xA8;
      else if (u == 0x451) out = 0xB8;
      else if (u >= 0x410 && u <= 0x44F) out = (uint8_t)(u - 0x350);
      else if (u < 0x100) out = (uint8_t)u;
      dst[o++] = out;
      continue;
    }
    if ((c & 0xF0) == 0xE0 && p[0] && p[1]) {
      p += 2;
      dst[o++] = '?';
      continue;
    }
    if ((c & 0xF8) == 0xF0 && p[0] && p[1] && p[2]) {
      p += 3;
      dst[o++] = '?';
      continue;
    }
    dst[o++] = '?';
  }
  dst[o] = 0;
  return o;
}

void DwinDisplay::writeText(uint16_t vp, const char *text) {
  // Plain Text Display: payload starts at VP (clock/date/rssi).
  writeTextFixed(vp, text, 0);
}

void DwinDisplay::writeTextFixed(uint16_t vp, const char *text, uint8_t width) {
  // width=0 → tight string+NUL.
  // width>0 → fill entire DGUS Text Length with spaces, then overlay new text.
  // (DGUS keeps old bytes if the new payload is shorter — 0x00 alone is not enough.)
  if (width == 0) {
    uint8_t raw[40];
    size_t n = utf8ToCp1251(text ? text : "", raw, sizeof(raw));
    uint8_t p[kPacketMax];
    memset(p, 0, sizeof(p));
    p[0] = 0x5A;
    p[1] = 0xA5;
    p[2] = (uint8_t)(3 + n + 1);
    p[3] = 0x82;
    p[4] = (uint8_t)(vp >> 8);
    p[5] = (uint8_t)vp;
    memcpy(p + 6, raw, n);
    p[6 + n] = 0;
    enqueue(p, (uint8_t)(7 + n));
    return;
  }
  if (width > kPacketMax - 6) width = kPacketMax - 6;
  uint8_t conv[64];
  memset(conv, 0, sizeof(conv));
  size_t n = utf8ToCp1251(text ? text : "", conv, sizeof(conv));
  if (n > width) n = width;

  uint8_t p[kPacketMax];
  p[0] = 0x5A;
  p[1] = 0xA5;
  p[2] = (uint8_t)(3 + width);
  p[3] = 0x82;
  p[4] = (uint8_t)(vp >> 8);
  p[5] = (uint8_t)vp;
  memset(p + 6, 0x20, width); // wipe old glyphs
  if (n) memcpy(p + 6, conv, n);
  enqueue(p, (uint8_t)(6 + width));
}

void DwinDisplay::writeScrollText(uint16_t widgetVp, const char *text) {
  // Same as src/audio_player.cpp scroll(): widget VP 0x67B4, data written to 0x67B7.
  constexpr uint8_t kScrollLen = dwin_vp::SCROLL_TEXT_LEN;
  uint16_t dataVp = (uint16_t)(widgetVp + 3);
  uint8_t raw[kScrollLen + 1];
  size_t n = utf8ToCp1251(text ? text : "", raw, sizeof(raw));
  if (n > kScrollLen) n = kScrollLen;

  // Clear previous scroll content (zeros), then write new string once.
  uint8_t clr[kPacketMax];
  memset(clr, 0, sizeof(clr));
  clr[0] = 0x5A;
  clr[1] = 0xA5;
  clr[2] = (uint8_t)(3 + kScrollLen);
  clr[3] = 0x82;
  clr[4] = (uint8_t)(dataVp >> 8);
  clr[5] = (uint8_t)dataVp;
  enqueue(clr, (uint8_t)(6 + kScrollLen));

  uint8_t p[kPacketMax];
  memset(p, 0, sizeof(p));
  p[0] = 0x5A;
  p[1] = 0xA5;
  p[2] = (uint8_t)(3 + n + 1);
  p[3] = 0x82;
  p[4] = (uint8_t)(dataVp >> 8);
  p[5] = (uint8_t)dataVp;
  memcpy(p + 6, raw, n);
  p[6 + n] = 0;
  enqueue(p, (uint8_t)(7 + n));
#if DWIN_DEBUG
  Serial.printf("[DWIN] scroll widget=0x%04X data=0x%04X len=%u\n", widgetVp, dataVp, (unsigned)n);
#endif
}

uint16_t DwinDisplay::probeOnce(uint32_t waitMs) {
  const uint8_t rd[] = {0x5A, 0xA5, 0x04, 0x83, 0x00, 0x82, 0x01};
  while (_serial.available()) (void)_serial.read();
  writeRaw(rd, sizeof(rd));

  uint32_t start = millis();
  uint16_t got = 0;
  while (millis() - start < waitMs) {
    while (_serial.available()) {
      (void)_serial.read();
      got++;
    }
    delay(1);
  }
  return got;
}

bool DwinDisplay::tryLink(int rxPin, int txPin, uint32_t baud) {
  _serial.end();
  delay(20);
  _serial.setRxBufferSize(256);
  _serial.begin(baud, SERIAL_8N1, rxPin, txPin);
  delay(80);

  // Visual TX test: page 1 + full brightness (HIGH byte = level for 0x0082).
  const uint8_t goPlayer[] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, 0x01};
  const uint8_t bright[] = {0x5A, 0xA5, 0x05, 0x82, 0x00, 0x82, 0x64, 0x00};
  writeRaw(goPlayer, sizeof(goPlayer));
  writeRaw(bright, sizeof(bright));
  delay(50);

  uint16_t got = probeOnce(350);
#if DWIN_DEBUG
  Serial.printf("[DWIN] try RX=GPIO%d TX=GPIO%d baud=%u -> %u bytes\n",
                rxPin, txPin, baud, got);
#endif
  return got > 0;
}

bool DwinDisplay::probeLink() {
  struct Attempt {
    int rx;
    int tx;
    uint32_t baud;
  };

  // Prefer configured pins/baud, then swapped TX/RX, then common bauds.
  const Attempt attempts[] = {
      {DWIN_RX, DWIN_TX, DWIN_BAUD},
      {DWIN_TX, DWIN_RX, DWIN_BAUD},
      {DWIN_RX, DWIN_TX, 9600},
      {DWIN_TX, DWIN_RX, 9600},
      {DWIN_RX, DWIN_TX, 57600},
      {DWIN_TX, DWIN_RX, 57600},
  };

  for (const Attempt &a : attempts) {
    if (tryLink(a.rx, a.tx, a.baud)) {
      _rxPin = a.rx;
      _txPin = a.tx;
      _baud = a.baud;
      Serial.printf("[DWIN] LINK OK: RX=GPIO%d TX=GPIO%d baud=%u\n", _rxPin, _txPin, _baud);
#if DWIN_DEBUG
      Serial.println("[DWIN] Put these values into yoRadio/myoptions.h if different from config");
#endif
      return true;
    }
  }

  // Keep configured UART open for later retry; do not claim ready.
  _rxPin = DWIN_RX;
  _txPin = DWIN_TX;
  _baud = DWIN_BAUD;
  _serial.end();
  delay(20);
  _serial.begin(_baud, SERIAL_8N1, _rxPin, _txPin);
  delay(50);

  Serial.println("[DWIN] LINK FAIL: no reply — will retry");
#if DWIN_DEBUG
  Serial.println("[DWIN] Hardware checklist:");
  Serial.println("[DWIN]  1) Common GND between ESP32-S3 and DWIN");
  Serial.println("[DWIN]  2) UART2 jumper on display = ON (TTL/CMOS), NOT RS232");
  Serial.println("[DWIN]  3) Use UART2 pins on 10-pin user header (not UART4)");
  Serial.println("[DWIN]  4) DWIN TX -> ESP RX, DWIN RX -> ESP TX");
  Serial.println("[DWIN]  5) If screen brightness/page did NOT change on boot, TX wire is wrong");
#endif
  return false;
}

void DwinDisplay::retryLinkIfNeeded() {
  if (_ready) return;
  uint32_t now = millis();
  if (_lastProbeMs != 0 && (now - _lastProbeMs) < 2000) return;
  _lastProbeMs = now;
  if (probeLink()) {
    _ready = true;
    page(dwin_vp::PLAYER);
    setBrightness();
    Serial.println("[DWIN] LINK OK after retry — display ready");
  }
}

void DwinDisplay::begin() {
  if (_reqQueue == nullptr) {
    _reqQueue = xQueueCreate(kReqQueueDepth, sizeof(requestParams_t));
  }
  // Ready only after a successful probe (see retryLinkIfNeeded in loop).
  _ready = false;
  _lastProbeMs = 0;
#if DWIN_DEBUG
  Serial.printf("[DWIN] probing link (cfg RX=GPIO%d TX=GPIO%d baud=%u)\n",
                DWIN_RX, DWIN_TX, DWIN_BAUD);
  Serial.println("[DWIN] Play button: VP 0x5400 key 1 (Data auto-upload ON)");
#endif

  if (probeLink()) {
    _ready = true;
    _lastProbeMs = millis();
    page(dwin_vp::PLAYER);
    setBrightness();
  } else {
    _lastProbeMs = millis();
  }

#if DWIN_DEBUG
  Serial.printf("[DWIN] UART active: RX=GPIO%d TX=GPIO%d baud=%u ready=%u\n",
                _rxPin, _txPin, _baud, (unsigned)_ready);
#endif
  Serial.printf("[DWIN] TX queue depth=%u flush/loop≤%u req queue=%u\n",
                (unsigned)kTxDepth, (unsigned)kTxFlushMax, (unsigned)kReqQueueDepth);
}

bool DwinDisplay::enqueue(const uint8_t *data, uint8_t len) {
  if (len > kPacketMax) return false;
  portENTER_CRITICAL(&_txMux);
  uint8_t next = (_txHead + 1) % kTxDepth;
  if (next == _txTail) {
    _txOverflow++;
    uint32_t n = _txOverflow;
    portEXIT_CRITICAL(&_txMux);
    // Rare log — avoid Serial flood when UI is under load.
    if ((n & 0x3F) == 1) {
      Serial.printf("[DWIN] TX overflow x%lu (depth=%u)\n", (unsigned long)n, (unsigned)kTxDepth);
    }
    return false;
  }
  memcpy(_tx[_txHead].data, data, len);
  _tx[_txHead].len = len;
  _txHead = next;
  portEXIT_CRITICAL(&_txMux);
  return true;
}

void DwinDisplay::flushTx() {
  if (_locked) return;

  for (uint8_t i = 0; i < kTxFlushMax; i++) {
    Packet p;
    portENTER_CRITICAL(&_txMux);
    if (_txHead == _txTail) {
      portEXIT_CRITICAL(&_txMux);
      return;
    }
    p = _tx[_txTail];
    if (_serial.availableForWrite() < (int)p.len) {
      portEXIT_CRITICAL(&_txMux);
      return;
    }
    _txTail = (_txTail + 1) % kTxDepth;
    portEXIT_CRITICAL(&_txMux);
    _serial.write(p.data, p.len);
  }
}

void DwinDisplay::writeWord(uint16_t vp, uint16_t value) {
  uint8_t p[] = {0x5A, 0xA5, 0x05, 0x82, (uint8_t)(vp >> 8), (uint8_t)vp,
                 (uint8_t)(value >> 8), (uint8_t)value};
  enqueue(p, sizeof(p));
}

void DwinDisplay::writeVuPair(uint8_t left, uint8_t right) {
  // DGUS word addresses step by 1: 50D0, 50D1, 50D2.
  // Contract keeps R at 50D2 (gap at 50D1) → pad one unused word in between.
  uint8_t p[] = {0x5A, 0xA5, 0x09, 0x82,
                 (uint8_t)(dwin_vp::VU_LEFT >> 8), (uint8_t)dwin_vp::VU_LEFT,
                 0x00, left, 0x00, 0x00, 0x00, right};
  enqueue(p, sizeof(p));
}

void DwinDisplay::page(uint16_t id) {
  uint8_t p[] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01,
                 (uint8_t)(id >> 8), (uint8_t)id};
  enqueue(p, sizeof(p));
}

void DwinDisplay::setBrightness() {
  uint8_t level = config.store.brightness;
  if (level > 100) level = 100;
  // DGUS system 0x0082: brightness is the HIGH byte (0x00..0x64), low is 0.
  // Working pulseoximeter HMI uses the same layout: 64 00 = 100%, 01 00 = 1%.
  uint8_t p[] = {0x5A, 0xA5, 0x05, 0x82, 0x00, 0x82, level, 0x00};
  enqueue(p, sizeof(p));
}

void DwinDisplay::sleep() {
  const uint8_t p[] = {0x5A, 0xA5, 0x05, 0x82, 0x00, 0x82, 0x00, 0x00};
  enqueue(p, sizeof(p));
}

void DwinDisplay::wake() {
  setBrightness();
  page((_mode == STATIONS) ? dwin_vp::PLAYLIST : dwin_vp::PLAYER);
}

void DwinDisplay::start() {
  _mode = PLAYER;
  _lastPlayState = -1;
  _lastStation[0] = 0;
  _lastTitle[0] = 0;
  // get_VUlevel() returns 0 while store.vumeter is off (web default).
  if (!config.store.vumeter) config.store.vumeter = true;
  page(dwin_vp::PLAYER);
  refresh();
  syncPlayState();
}

void DwinDisplay::setMode(displayMode_e value) {
  _mode = value;
  switch (value) {
    case PLAYER: page(dwin_vp::PLAYER); break;
    case STATIONS: {
      page(dwin_vp::PLAYLIST);
      uint16_t cur = config.lastStation();
      if (cur < 1) cur = 1;
      uint16_t len = config.playlistLength();
      if (len > 0 && cur > len) cur = len;
      display.currentPlItem = cur;
      refreshPlaylist();
      break;
    }
    case SETTINGS: page(dwin_vp::SETTINGS); break;
    case WIFI: page(dwin_vp::WIFI); break;
    case TIMEZONE: page(dwin_vp::TIME); break;
    case INFO: page(dwin_vp::INFO); break;
    case ALARMS:
      page(dwin_vp::ALARMS);
      pushAlarms();
      break;
    case ALARMEDIT:
      page(dwin_vp::ALARM_EDIT);
      pushAlarmEditor();
      break;
    case AUDIO:
      page(dwin_vp::AUDIO);
      pushEq();
      writeWord(dwin_vp::VOLUME, config.store.volume);
      break;
    case SCREENSAVER:
    case SCREENBLANK: page(dwin_vp::SCREENSAVER); break;
    case VOL:
      // Volume lives on Player (VP 0x50D4) — do not leave the current page.
      writeWord(dwin_vp::VOLUME, config.store.volume);
      break;
    case LOST:
      // Keep Player; show reconnect hint (no dedicated Lost page in HMI yet).
      page(dwin_vp::PLAYER);
      writeScrollText(dwin_vp::STATION, "Нет Wi-Fi");
      writeTextFixed(dwin_vp::TITLE_STATIC, "Переподключение…", dwin_vp::TITLE_STATIC_LEN);
      writeTextFixed(dwin_vp::NETWORK, "reconnect", dwin_vp::NETWORK_LEN);
      strlcpy(_lastStation, "Нет Wi-Fi", sizeof(_lastStation));
      strlcpy(_lastTitle, "Переподключение…", sizeof(_lastTitle));
      strlcpy(_lastIp, "reconnect", sizeof(_lastIp));
      break;
    case CLEAR:
    case NUMBERS:
    case UPDATING:
    case SLEEPING:
    case SDCHANGE:
      // Internal / transient modes — do not jump to empty DIALOG page 9.
      break;
    default:
      break;
  }
  if (value != LOST) refreshStatus();
}

uint16_t DwinDisplay::playlistWindowBase() const {
  // Sliding window of 7 rows; cursor prefers middle (index 3) when possible.
  return (display.currentPlItem > 3) ? (uint16_t)(display.currentPlItem - 3) : 1;
}

void DwinDisplay::refreshPlaylist() {
  const uint16_t len = config.playlistLength();
  const uint16_t base = playlistWindowBase();
  // 1) Wipe all name slots (spaces), so shorter names can't leave a tail.
  for (uint8_t i = 0; i < 7; i++) {
    writeTextFixed(dwin_vp::PLAYLIST_ITEM_0 + i * 0x40, "", dwin_vp::PLAYLIST_NAME_LEN);
  }
  // 2) Fill numbers + names for the current window.
  for (uint8_t i = 0; i < 7; i++) {
    uint16_t n = (uint16_t)(base + i);
    const bool ok = (len > 0 && n <= len);
    writeWord(dwin_vp::PLAYLIST_NUM_0 + i, ok ? n : 0);
    if (ok) {
      writeTextFixed(dwin_vp::PLAYLIST_ITEM_0 + i * 0x40, config.stationByNum(n),
                     dwin_vp::PLAYLIST_NAME_LEN);
    }
  }
  writeWord(dwin_vp::PLAYLIST_SELECTED, display.currentPlItem);
  uint16_t row = (display.currentPlItem >= base) ? (uint16_t)(display.currentPlItem - base) : 0;
  if (row > 6) row = 6;
  writeWord(dwin_vp::PLAYLIST_ROW, row);
#if DWIN_DEBUG
  Serial.printf("[DWIN] playlist window base=%u sel=%u row=%u len=%u\n", base,
                display.currentPlItem, row, len);
#endif
}

void DwinDisplay::movePlaylist(int16_t delta) {
  const uint16_t len = config.playlistLength();
  if (len == 0) return;
  int32_t p = (int32_t)display.currentPlItem + delta;
  while (p < 1) p += len;
  while (p > (int32_t)len) p -= len;
  display.currentPlItem = (uint16_t)p;
  refreshPlaylist();
}

void DwinDisplay::playStation(uint16_t num) {
  const uint16_t len = config.playlistLength();
  if (num < 1 || num > len) return;
  display.currentPlItem = num;
  setMode(::PLAYER);
  player.sendCommand({PR_PLAY, num});
}

void DwinDisplay::updatePlayState() {
  uint16_t state = player.status() == PLAYING ? 1 : 0;
  writeWord(dwin_vp::PLAY_STATE, state);
  _lastPlayState = (int8_t)state;
#if DWIN_DEBUG
  Serial.printf("[DWIN] PLAY_STATE 0x50D6=%u\n", state);
#endif
}

void DwinDisplay::syncPlayState() {
  int8_t state = player.status() == PLAYING ? 1 : 0;
  if (state == _lastPlayState) return;
  writeWord(dwin_vp::PLAY_STATE, (uint16_t)state);
  _lastPlayState = state;
#if DWIN_DEBUG
  Serial.printf("[DWIN] PLAY_STATE sync 0x50D6=%u\n", state);
#endif
}

void DwinDisplay::ackCommand() { writeWord(dwin_vp::COMMAND, 0); }

void DwinDisplay::pushStation() {
  if (strncmp(_lastStation, config.station.name, sizeof(_lastStation)) == 0) return;
  strlcpy(_lastStation, config.station.name, sizeof(_lastStation));
  writeScrollText(dwin_vp::STATION, config.station.name);
}

void DwinDisplay::pushTitle() {
  if (strncmp(_lastTitle, config.station.title, sizeof(_lastTitle)) == 0) return;
  strlcpy(_lastTitle, config.station.title, sizeof(_lastTitle));
  // Scroll widget VP=5040, data at 5043. Do NOT bind Text Display to 5040 —
  // first words are scroll control and flicker as garbage.
  writeScrollText(dwin_vp::TITLE, config.station.title);
  writeTextFixed(dwin_vp::TITLE_STATIC, config.station.title, dwin_vp::TITLE_STATIC_LEN);
}

int8_t DwinDisplay::clampTone(int16_t v) {
  if (v < dwin_vp::EQ_TONE_MIN) return dwin_vp::EQ_TONE_MIN;
  if (v > dwin_vp::EQ_TONE_MAX) return dwin_vp::EQ_TONE_MAX;
  return (int8_t)v;
}

int8_t DwinDisplay::clampBalance(int16_t v) {
  if (v < dwin_vp::EQ_BALANCE_MIN) return dwin_vp::EQ_BALANCE_MIN;
  if (v > dwin_vp::EQ_BALANCE_MAX) return dwin_vp::EQ_BALANCE_MAX;
  return (int8_t)v;
}

void DwinDisplay::pushEq() {
  writeWord(dwin_vp::BASS, (uint16_t)(int16_t)config.store.bass);
  writeWord(dwin_vp::MIDDLE, (uint16_t)(int16_t)config.store.middle);
  writeWord(dwin_vp::TREBLE, (uint16_t)(int16_t)config.store.trebble);
  writeWord(dwin_vp::BALANCE, (uint16_t)(int16_t)config.store.balance);
}

void DwinDisplay::pushTelnet() {
  writeWord(dwin_vp::TELNET_STATE, config.store.telnet ? 1 : 0);
}

void DwinDisplay::showAlarmError(const char *msg) {
  writeTextFixed(dwin_vp::ALARM_ERR, msg ? msg : "", dwin_vp::ALARM_ERR_LEN);
#if DWIN_DEBUG
  if (msg && msg[0]) Serial.printf("[ALARM] UI: %s\n", msg);
#endif
}

void DwinDisplay::openAlarmEditor(uint8_t id) {
  if (id >= ALARM_COUNT) id = 0;
  _editAlarm = id;
  showAlarmError("");
  setMode(::ALARMEDIT);
}

void DwinDisplay::pushAlarmEditor() {
  using namespace dwin_vp;
  const alarm_t &a = alarms.get(_editAlarm);
  writeWord(ALARM_EDIT_ID, _editAlarm);
  writeWord(ALARM_EDIT_ENABLED, a.enabled ? 1 : 0);
  writeWord(ALARM_EDIT_START, a.startMin);
  writeWord(ALARM_EDIT_STOP, a.stopMin);
  writeWord(ALARM_EDIT_STATION, a.station);
  writeWord(ALARM_EDIT_VOLUME, a.volume);
  writeWord(ALARM_EDIT_DAYS, a.days);
  for (uint8_t d = 0; d < 7; d++) {
    writeWord((uint16_t)(ALARM_DAY_ICON0 + d), (a.days & (1u << d)) ? 1 : 0);
  }
}

void DwinDisplay::pushAlarmStatus() {
  using namespace dwin_vp;
  char status[40];
  uint16_t active = 0;
  uint16_t activeId = 0;
  status[0] = 0;
  if (alarms.snoozePending()) {
    active = 2;
    int8_t from = alarms.snoozeFromId();
    activeId = from >= 0 ? (uint16_t)(from + 1) : 0;
    strlcpy(status, "Отложено 10 мин", sizeof(status));
  } else if (alarms.sessionActive()) {
    active = 1;
    activeId = (uint16_t)(alarms.activeId() + 1);
    snprintf(status, sizeof(status), "Будильник №%u", (unsigned)activeId);
  }
  writeWord(ALARM_ACTIVE, active);
  writeWord(ALARM_ACTIVE_ID, activeId);
  if ((int8_t)active != _lastAlarmActive) {
    _lastAlarmActive = (int8_t)active;
    writeTextFixed(ALARM_STATUS, status, ALARM_STATUS_LEN);
  }
}

void DwinDisplay::pushAlarms() {
  using namespace dwin_vp;
  char line[48];
  char range[16];
  char days[24];
  for (uint8_t i = 0; i < ALARM_COUNT; i++) {
    const alarm_t &a = alarms.get(i);
    uint16_t base = (uint16_t)(ALARM0 + i * ALARM_STRIDE);
    writeWord(base + 0x00, a.enabled ? 1 : 0);
    writeWord(base + 0x02, a.startMin);
    writeWord(base + 0x04, a.stopMin);
    writeWord(base + 0x06, a.station);
    writeWord(base + 0x08, a.volume);
    writeWord(base + 0x0A, a.days);
    AlarmManager::formatRange(a.startMin, a.stopMin, range, sizeof(range));
    AlarmManager::formatDays(a.days, days, sizeof(days));
    snprintf(line, sizeof(line), "%u %s %s #%u %s",
             (unsigned)(i + 1), range, days, a.station, a.enabled ? "ON" : "OFF");
    writeTextFixed((uint16_t)(ALARM_LINE0 + i * 0x40), line, ALARM_LINE_LEN);
  }
  if (_mode == ::ALARMEDIT) pushAlarmEditor();
  pushAlarmStatus();
}

void DwinDisplay::pushWeather() {
  using namespace dwin_vp;
  if (!config.store.showweather || !timekeeper.weather.valid) {
    writeWord(WEATHER_ICON, 9);
    writeWord(WEATHER_TEMP, 0);
    writeWord(WEATHER_FEELS, 0);
    writeWord(WEATHER_PRESS, 0);
    writeWord(WEATHER_PRESS_HPA, 0);
    writeWord(WEATHER_HUM, 0);
    writeWord(WEATHER_WIND, 0);
    writeTextFixed(WEATHER_DESC, "", WEATHER_DESC_LEN);
#if DWIN_DEBUG
    Serial.println("[DWIN] weather cleared / unavailable");
#endif
    return;
  }
  const WeatherData &w = timekeeper.weather;
  writeWord(WEATHER_ICON, w.icon);
  writeWord(WEATHER_TEMP, (uint16_t)w.temp_c);
  writeWord(WEATHER_FEELS, (uint16_t)w.feels_c);
  writeWord(WEATHER_PRESS, w.press);
  writeWord(WEATHER_PRESS_HPA, w.press_hpa);
  writeWord(WEATHER_HUM, w.hum);
  writeWord(WEATHER_WIND, w.wind_ms);
  writeTextFixed(WEATHER_DESC, w.desc, WEATHER_DESC_LEN);
#if DWIN_DEBUG
  Serial.printf("[DWIN] weather icon=%u temp=%d feels=%d press=%umm/%uhPa hum=%u wind=%u\n",
                w.icon, w.temp_c, w.feels_c, w.press, w.press_hpa, w.hum, w.wind_ms);
#endif
}

void DwinDisplay::refreshStatus() {
  char b[40];

  // Clock with seconds (HH:MM:SS) — Text Length ≥ CLOCK_LEN in DGUS.
  strftime(b, sizeof(b), "%H:%M:%S", &network.timeinfo);
  if (strncmp(_lastClock, b, sizeof(_lastClock)) != 0) {
    strlcpy(_lastClock, b, sizeof(_lastClock));
    writeTextFixed(dwin_vp::CLOCK, b, dwin_vp::CLOCK_LEN);
  }

  // Date DD.MM.YYYY
  strftime(b, sizeof(b), "%d.%m.%Y", &network.timeinfo);
  if (strncmp(_lastDate, b, sizeof(_lastDate)) != 0) {
    strlcpy(_lastDate, b, sizeof(_lastDate));
    writeTextFixed(dwin_vp::DATE, b, dwin_vp::DATE_LEN);
  }

  // Weekday (full localized name → CP1251), once per day change
  {
    int8_t w = (int8_t)network.timeinfo.tm_wday;
    if (w < 0 || w > 6) w = 0;
    if (w != _lastWday) {
      _lastWday = w;
      char dowBuf[24];
      strncpy_P(dowBuf, LANG::dowf[(uint8_t)w], sizeof(dowBuf) - 1);
      dowBuf[sizeof(dowBuf) - 1] = 0;
      strlcpy(_lastWeekday, dowBuf, sizeof(_lastWeekday));
      writeTextFixed(dwin_vp::WEEKDAY, dowBuf, dwin_vp::WEEKDAY_LEN);
    }
  }

  // IP (STA or SoftAP)
  {
    IPAddress ip = (WiFi.getMode() == WIFI_STA) ? WiFi.localIP() : WiFi.softAPIP();
    const char *ips = config.ipToStr(ip);
    if (strncmp(_lastIp, ips, sizeof(_lastIp)) != 0) {
      strlcpy(_lastIp, ips, sizeof(_lastIp));
      writeTextFixed(dwin_vp::NETWORK, ips, dwin_vp::NETWORK_LEN);
    }
  }

  // RSSI / bitrate always (change often); format only on change
  snprintf(b, sizeof(b), "%d dBm", WiFi.RSSI());
  writeText(dwin_vp::RSSI, b);
  snprintf(b, sizeof(b), "%u", config.station.bitrate);
  writeText(dwin_vp::BITRATE, b);
  {
    const char *fmt = dwinFormatLabel(config.configFmt);
    if (strncmp(_lastFormat, fmt, sizeof(_lastFormat)) != 0) {
      strlcpy(_lastFormat, fmt, sizeof(_lastFormat));
      writeTextFixed(dwin_vp::FORMAT, fmt, dwin_vp::FORMAT_LEN);
    }
  }

  // Station number, play mode, firmware version (once), current SSID
  {
    uint16_t sn = config.lastStation();
    if (sn != _lastStationNum) {
      _lastStationNum = sn;
      writeWord(dwin_vp::STATION_NUM, sn);
    }
  }
  {
    const char *mode = config.getMode() == PM_SDCARD ? "SD" : "WEB";
    if (strncmp(_lastMode, mode, sizeof(_lastMode)) != 0) {
      strlcpy(_lastMode, mode, sizeof(_lastMode));
      writeTextFixed(dwin_vp::MODE_TEXT, mode, dwin_vp::MODE_TEXT_LEN);
    }
  }
  if (!_metaPushed) {
    writeTextFixed(dwin_vp::VERSION, YOVERSION, dwin_vp::VERSION_LEN);
    _metaPushed = true;
  }
  {
    String ssid = WiFi.SSID();
    if (strncmp(_lastSsid, ssid.c_str(), sizeof(_lastSsid)) != 0) {
      strlcpy(_lastSsid, ssid.c_str(), sizeof(_lastSsid));
      writeTextFixed(dwin_vp::SSID_STATUS, ssid.c_str(), dwin_vp::SSID_STATUS_LEN);
    }
  }

  // During slider drag, store.volume lags (setVol is queued) — don't yank the knob back.
  if ((int32_t)(millis() - _sliderGuardUntil) >= 0) {
    writeWord(dwin_vp::VOLUME, config.store.volume);
  }
  syncPlayState();
  writeWord(dwin_vp::BRIGHTNESS, config.store.brightness);
  pushEq();
  pushTelnet();
  writeWord(dwin_vp::TZ_HOUR, (uint16_t)(int16_t)config.store.tzHour);
  writeWord(dwin_vp::TZ_MINUTE, (uint16_t)(int16_t)config.store.tzMin);
  pushAlarmStatus();
}

void DwinDisplay::refresh() {
  _lastStation[0] = 0;
  _lastTitle[0] = 0;
  _lastClock[0] = 0;
  _lastDate[0] = 0;
  _lastWeekday[0] = 0;
  _lastIp[0] = 0;
  _lastFormat[0] = 0;
  _lastMode[0] = 0;
  _lastSsid[0] = 0;
  _lastWday = -1;
  _lastStationNum = 0;
  _metaPushed = false;
  pushStation();
  pushTitle();
  refreshStatus();
  pushWeather();
}

void DwinDisplay::putRequest(requestParams_t r) {
  // Before queue exists (very early boot) or when already on DwinTask — handle inline.
  if (_reqQueue == nullptr ||
      (_ownerTask != nullptr && xTaskGetCurrentTaskHandle() == _ownerTask)) {
    handleRequest(r);
    return;
  }
  if (xQueueSend(_reqQueue, &r, 0) != pdTRUE) {
    _reqOverflow++;
    if ((_reqOverflow & 0x1F) == 1) {
      Serial.printf("[DWIN] request queue full x%lu type=%d\n",
                    (unsigned long)_reqOverflow, (int)r.type);
    }
  }
}

void DwinDisplay::drainRequests() {
  if (_reqQueue == nullptr) return;
  requestParams_t r;
  // Bound work per loop so UART RX/TX stay responsive.
  for (uint8_t i = 0; i < kReqQueueDepth && xQueueReceive(_reqQueue, &r, 0) == pdTRUE; i++) {
    handleRequest(r);
  }
}

void DwinDisplay::handleRequest(requestParams_t r) {
  if (_locked) return;
  switch (r.type) {
    case DSP_START:
      start();
      break;
    case NEWMODE:
      setMode((displayMode_e)r.payload);
      break;
    case NEWSTATION:
      pushStation();
      _lastStationNum = config.lastStation();
      writeWord(dwin_vp::STATION_NUM, _lastStationNum);
      break;
    case NEWTITLE:
      pushTitle();
      break;
    case CLOCK:
    case DRAWVOL:
    case DBITRATE:
    case DSPRSSI:
    case NEWIP:
      refreshStatus();
      break;
    case PSTART:
    case PSTOP:
      updatePlayState();
      setBrightness();
      break;
    case DRAWPLAYLIST:
      display.currentPlItem = (uint16_t)r.payload;
      refreshPlaylist();
      break;
    case CLOSEPLAYLIST:
      player.sendCommand({PR_PLAY, r.payload});
      break;
    case NEWWEATHER:
    case SHOWWEATHER:
      pushWeather();
      break;
    default:
      break;
  }
}

bool DwinDisplay::mapLegacyPlayerKey(uint16_t key, uint16_t *commandOut) {
#if DWIN_LEGACY_PLAYER_VP
  using namespace dwin_vp;
  switch (key) {
    case 0x09: *commandOut = PLAY_TOGGLE; return true;
    case 0x01: *commandOut = NEXT; return true;
    case 0x02: *commandOut = PREVIOUS; return true;
    case 0x03: *commandOut = OPEN_PLAYLIST; return true;
    default: return false;
  }
#else
  (void)key;
  (void)commandOut;
  return false;
#endif
}

void DwinDisplay::logDiagnosticHint(uint16_t vp, uint16_t raw) {
#if !DWIN_DEBUG
  (void)vp;
  (void)raw;
#else
  if (vp == dwin_vp::PLAY_STATE) {
    Serial.println("[DWIN] hint: 0x50D6 is visual state only; bind button to VP 0x5400 key 1");
    return;
  }
  if (vp == 0x6800) {
    Serial.printf("[DWIN] hint: legacy VP 0x6800 key 0x%02X", raw);
#if DWIN_LEGACY_PLAYER_VP
    Serial.println(" -> mapped when supported");
#else
    Serial.println(" -> enable DWIN_LEGACY_PLAYER_VP in myoptions.h");
#endif
    return;
  }
  if (vp == dwin_vp::COMMAND && raw == 0x5400) {
    Serial.println("[DWIN] hint: key value must be 1, not 0x5400 (21504)");
    return;
  }
  Serial.printf("[DWIN] unhandled VP=0x%04X value=%u\n", vp, raw);
#endif
}

void DwinDisplay::command(uint16_t c) {
  using namespace dwin_vp;
  bool playStateChanged = false;

  switch (c) {
    case PLAY_TOGGLE: {
      // On playlist page: confirm selection (same as encoder center).
      if (_mode == ::STATIONS) {
        playStation(display.currentPlItem);
        break;
      }
#if DWIN_DEBUG
      Serial.printf("[DWIN] player.toggle() status=%d\n", player.status());
#endif
      // Do not paint PLAY_STATE here — wait for PSTART/PSTOP / syncPlayState.
      player.toggle();
      playStateChanged = true;
      break;
    }
    case PREVIOUS:
      if (_mode == ::STATIONS) {
        movePlaylist(-1);
        break;
      }
      player.prev();
      break;
    case NEXT:
      if (_mode == ::STATIONS) {
        movePlaylist(+1);
        break;
      }
      player.next();
      break;
    case VOL_DOWN: {
      // setVol() is queued — compute next level and push it to VP 0x50D4 immediately.
      uint8_t v = config.store.volume;
      uint8_t step = config.store.volsteps;
      v = (v >= step) ? (uint8_t)(v - step) : 0;
      player.setVol(v);
      writeWord(VOLUME, v);
#if DWIN_DEBUG
      Serial.printf("[DWIN] VOLUME 0x50D4=%u\n", v);
#endif
      break;
    }
    case VOL_UP: {
      uint8_t v = config.store.volume;
      uint8_t step = config.store.volsteps;
      v = (v <= 254 - step) ? (uint8_t)(v + step) : 254;
      player.setVol(v);
      writeWord(VOLUME, v);
#if DWIN_DEBUG
      Serial.printf("[DWIN] VOLUME 0x50D4=%u\n", v);
#endif
      break;
    }
    case PL_ROW_0:
    case PL_ROW_1:
    case PL_ROW_2:
    case PL_ROW_3:
    case PL_ROW_4:
    case PL_ROW_5:
    case PL_ROW_6:
      _mode = ::STATIONS;
      playStation((uint16_t)(playlistWindowBase() + (c - PL_ROW_0)));
      break;
    case PL_PAGE_UP:
      _mode = ::STATIONS;
      movePlaylist(-7);
      break;
    case PL_PAGE_DOWN:
      _mode = ::STATIONS;
      movePlaylist(+7);
      break;
    case OPEN_PLAYER:
      setMode(::PLAYER);
      break;
    case OPEN_PLAYLIST:
      setMode(::STATIONS);
      break;
    case OPEN_SETTINGS:
      setMode(::SETTINGS);
      pushTelnet();
      break;
    case OPEN_WIFI:
      setMode(::WIFI);
      break;
    case OPEN_AUDIO:
      setMode(::AUDIO);
      break;
    case OPEN_TIME:
      setMode(::TIMEZONE);
      break;
    case OPEN_INFO:
      setMode(::INFO);
      break;
    case TOGGLE_TELNET:
      config.saveValue(&config.store.telnet, !config.store.telnet);
      telnet.toggle();
      pushTelnet();
      break;
    case OPEN_ALARMS:
      setMode(::ALARMS);
      break;
    case ALARM_EDIT_0:
    case ALARM_EDIT_1:
    case ALARM_EDIT_2:
      openAlarmEditor((uint8_t)(c - ALARM_EDIT_0));
      break;
    case ALARM_DAYS_DAILY:
    case ALARM_DAYS_WEEKDAYS:
    case ALARM_DAYS_WEEKEND: {
      char err[32];
      uint8_t preset = (uint8_t)(c - ALARM_DAYS_DAILY);
      if (!alarms.setDaysPreset(_editAlarm, preset, err, sizeof(err))) {
        showAlarmError(err);
      } else {
        showAlarmError("");
      }
      pushAlarms();
      break;
    }
    case ALARM_SNOOZE:
      alarms.snooze();
      pushAlarms();
      break;
    case ALARM_DISMISS:
      alarms.dismiss();
      pushAlarms();
      break;
    case ALARM_TOGGLE_DAY_0:
    case ALARM_TOGGLE_DAY_1:
    case ALARM_TOGGLE_DAY_2:
    case ALARM_TOGGLE_DAY_3:
    case ALARM_TOGGLE_DAY_4:
    case ALARM_TOGGLE_DAY_5:
    case ALARM_TOGGLE_DAY_6: {
      char err[32];
      if (!alarms.toggleDay(_editAlarm, (uint8_t)(c - ALARM_TOGGLE_DAY_0), err, sizeof(err))) {
        showAlarmError(err[0] ? err : "Нельзя выключить все дни");
      } else {
        showAlarmError("");
      }
      pushAlarms();
      break;
    }
    case SCREEN_OFF:
      sleep();
      break;
    case SCREEN_ON:
      wake();
      break;
    case SHOW_SCREENSAVER:
      setMode(::SCREENSAVER);
      break;
    case SAVE_WIFI:
      if (_wifiSsid[0]) {
        char data[73];
        snprintf(data, sizeof(data), "%s\t%s\n", _wifiSsid, _wifiPass);
        config.saveWifiFromNextion(data);
      }
      break;
    default:
#if DWIN_DEBUG
      Serial.printf("[DWIN] unknown command=%u\n", c);
#endif
      break;
  }

  ackCommand();
  if (playStateChanged) setBrightness();
}

void DwinDisplay::value(uint16_t vp, uint16_t raw) {
  int16_t s = (int16_t)raw;
  using namespace dwin_vp;
  bool handled = true;

#if DWIN_LEGACY_PLAYER_VP
  if (vp == 0x6800) {
    uint16_t mapped = 0;
    if (mapLegacyPlayerKey(raw, &mapped)) {
#if DWIN_DEBUG
      Serial.printf("[DWIN] legacy VP 0x6800 key 0x%02X -> command %u\n", raw, mapped);
#endif
      command(mapped);
      return;
    }
  }
#endif

  if (vp == COMMAND) {
    command(raw);
  } else if (vp == SELECT_STATION && raw > 0 && raw <= config.playlistLength()) {
    playStation(raw);
  } else if (vp == SET_VOLUME || vp == VOLUME) {
    // Drag on 50D4: do not echo writeWord — fights the finger (jerky knob).
    // Drag on 5404: echo to 50D4 so Slider Display follows.
    uint8_t v = (uint8_t)(raw > 254 ? 254 : raw);
    _sliderVol = v;
    _sliderGuardUntil = millis() + 450;
    if (millis() - _sliderLastSetMs >= 60) {
      player.setVol(v);
      _sliderLastSetMs = millis();
    }
    if (vp == SET_VOLUME) writeWord(VOLUME, v);
  } else if (vp == SET_BASS) {
    // Live apply + EEPROM — no separate «Confirm» needed.
    config.setTone(clampTone(s), config.store.middle, config.store.trebble);
    writeWord(BASS, (uint16_t)(int16_t)config.store.bass);
  } else if (vp == SET_MIDDLE) {
    config.setTone(config.store.bass, clampTone(s), config.store.trebble);
    writeWord(MIDDLE, (uint16_t)(int16_t)config.store.middle);
  } else if (vp == SET_TREBLE) {
    config.setTone(config.store.bass, config.store.middle, clampTone(s));
    writeWord(TREBLE, (uint16_t)(int16_t)config.store.trebble);
  } else if (vp == SET_BALANCE) {
    config.setBalance(clampBalance(s));
    writeWord(BALANCE, (uint16_t)(int16_t)config.store.balance);
  } else if (vp == SET_TELNET) {
    bool on = raw != 0;
    if (on != (bool)config.store.telnet) {
      config.saveValue(&config.store.telnet, on);
      telnet.toggle();
    }
    pushTelnet();
  } else if (vp == SET_BRIGHTNESS) {
    config.store.brightness = (uint8_t)(raw > 100 ? 100 : raw);
    config.setBrightness(true);
  } else if (vp == SET_TZ_HOUR) {
    config.setTimezone((int8_t)s, config.store.tzMin);
  } else if (vp == SET_TZ_MINUTE) {
    config.setTimezone(config.store.tzHour, (int8_t)s);
  } else if (vp == SELECT_ALARM) {
    openAlarmEditor((uint8_t)(raw > 2 ? 2 : raw));
  } else if (vp == SET_ALARM_ENABLED) {
    char err[32];
    if (!alarms.setEnabled(_editAlarm, raw != 0, err, sizeof(err))) {
      showAlarmError(err);
    } else {
      showAlarmError("");
    }
    pushAlarms();
  } else if (vp == SET_ALARM_START) {
    char err[32];
    if (!alarms.setStart(_editAlarm, AlarmManager::clampMinOfDay((int32_t)raw), err, sizeof(err))) {
      showAlarmError(err);
    } else {
      showAlarmError("");
    }
    pushAlarms();
  } else if (vp == SET_ALARM_STOP) {
    char err[32];
    if (!alarms.setStop(_editAlarm, AlarmManager::clampMinOfDay((int32_t)raw), err, sizeof(err))) {
      showAlarmError(err);
    } else {
      showAlarmError("");
    }
    pushAlarms();
  } else if (vp == SET_ALARM_STATION) {
    alarms.setStation(_editAlarm, raw);
    showAlarmError("");
    pushAlarms();
  } else if (vp == SET_ALARM_VOLUME) {
    alarms.setVolume(_editAlarm, (uint8_t)(raw > 254 ? 254 : raw));
    showAlarmError("");
    pushAlarms();
  } else if (vp == SET_ALARM_DAYS) {
    char err[32];
    if (!alarms.setDays(_editAlarm, (uint8_t)(raw & 0x7F), err, sizeof(err))) {
      showAlarmError(err);
    } else {
      showAlarmError("");
    }
    pushAlarms();
  } else {
    handled = false;
#if DWIN_DEBUG
    logDiagnosticHint(vp, raw);
#endif
  }

  (void)handled;
}

void DwinDisplay::text(uint16_t vp, const uint8_t *data, uint8_t len) {
  char *out = vp == dwin_vp::WIFI_SSID ? _wifiSsid
              : (vp == dwin_vp::WIFI_PASSWORD ? _wifiPass : nullptr);
  size_t cap = vp == dwin_vp::WIFI_SSID ? sizeof(_wifiSsid) : sizeof(_wifiPass);
  if (!out) return;
  size_t n = len < cap - 1 ? len : cap - 1;
  memcpy(out, data, n);
  out[n] = 0;
}

void DwinDisplay::frame(const uint8_t *p, uint8_t n) {
  if (n < 8 || p[3] != 0x83) {
#if DWIN_DEBUG
    Serial.printf("[DWIN] ignored frame: length=%u command=0x%02X\n", n, n > 3 ? p[3] : 0);
#endif
    return;
  }

  uint16_t vp = ((uint16_t)p[4] << 8) | p[5];
  uint8_t bytes = (uint8_t)(p[6] * 2);
  if (7 + bytes > n) {
#if DWIN_DEBUG
    Serial.printf("[DWIN] ignored VP 0x%04X: %u data bytes, frame length=%u\n", vp, bytes, n);
#endif
    return;
  }

#if DWIN_DEBUG
  Serial.printf("[DWIN] read VP=0x%04X words=%u", vp, p[6]);
  for (uint8_t i = 0; i < bytes; ++i) Serial.printf(" %02X", p[7 + i]);
  Serial.println();
#endif

  if (vp == dwin_vp::WIFI_SSID || vp == dwin_vp::WIFI_PASSWORD) {
    text(vp, p + 7, bytes);
    return;
  }

  if (bytes >= 2) {
    uint16_t raw = ((uint16_t)p[7] << 8) | p[8];
#if DWIN_DEBUG
    if (vp == dwin_vp::COMMAND) {
      Serial.printf("[DWIN] command=%u (VP 0x%04X)\n", raw, vp);
    }
#endif
    value(vp, raw);
  }
}

void DwinDisplay::pollRx() {
  while (_serial.available()) {
    uint8_t b = (uint8_t)_serial.read();
#if DWIN_DEBUG_BYTES
    Serial.printf("[DWIN] byte: %02X\n", b);
#endif
    if (_rxLen == 0 && b != 0x5A) continue;
    if (_rxLen == 1 && b != 0xA5) {
      _rxLen = 0;
      continue;
    }
    if (_rxLen >= kRxMax) _rxLen = 0;
    _rx[_rxLen++] = b;
    if (_rxLen < 3) continue;

    uint8_t total = _rx[2] + 3;
    if (total > kRxMax) {
      _rxLen = 0;
      continue;
    }
    if (_rxLen != total) continue;

#if DWIN_DEBUG
    Serial.print("[DWIN] RX:");
    for (uint8_t i = 0; i < _rxLen; ++i) Serial.printf(" %02X", _rx[i]);
    Serial.println();
#endif
    frame(_rx, _rxLen);
    _rxLen = 0;
  }
}

void DwinDisplay::loop() {
  if (_ownerTask == nullptr) {
    _ownerTask = xTaskGetCurrentTaskHandle();
  }

  // Do not drain UI until the panel answered probe — avoids DSP_START on dead link.
  if (!_ready) {
    retryLinkIfNeeded();
    return;
  }

  drainRequests();
  pollRx();
  flushTx();
  syncPlayState();

  // After drag stops: apply final volume once (throttled sets may have skipped last value).
  if (_sliderGuardUntil != 0 && (int32_t)(millis() - _sliderGuardUntil) >= 0) {
    player.setVol(_sliderVol);
    writeWord(dwin_vp::VOLUME, _sliderVol);
    _sliderGuardUntil = 0;
  }

  // VU: poll ~80 ms, send only on change (deadband 2). One UART frame for L+R.
  // Curve Display is worse here — stream of samples, not a level meter.
  static uint32_t last = 0;
  static uint32_t lastHint = 0;
  if (millis() - last >= 80) {
    last = millis();
    if (!_locked && config.store.vumeter) {
      uint16_t vu = player.get_VUlevel(100);
      uint8_t L = (uint8_t)(100 - ((vu >> 8) & 0xFF));
      uint8_t R = (uint8_t)(100 - (vu & 0xFF));
      const int16_t dL = (int16_t)L - _lastVuL;
      const int16_t dR = (int16_t)R - _lastVuR;
      if (_lastVuL < 0 || abs(dL) >= 2 || abs(dR) >= 2 ||
          ((L == 0 && R == 0) && (_lastVuL != 0 || _lastVuR != 0))) {
        writeVuPair(L, R);
        _lastVuL = L;
        _lastVuR = R;
      }
    }
  }
  // Drain again after UI work so CLOCK/NEWTITLE from other cores don't stall a tick.
  drainRequests();
  flushTx();
#if DWIN_DEBUG
  if (millis() - lastHint >= 10000) {
    lastHint = millis();
    Serial.printf("[DWIN] waiting touch on VP 0x5400 (RX GPIO%d). avail=%d play=%d txDrop=%lu reqDrop=%lu\n",
                  _rxPin, _serial.available(), _lastPlayState,
                  (unsigned long)_txOverflow, (unsigned long)_reqOverflow);
  }
#endif
}

void DwinDisplay::resetQueue() {
  portENTER_CRITICAL(&_txMux);
  _txHead = _txTail = 0;
  portEXIT_CRITICAL(&_txMux);
  if (_reqQueue != nullptr) {
    xQueueReset(_reqQueue);
  }
}
